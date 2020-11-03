/*
 * Filename:         m0common.c
 * Description:      Contains motr helpers
 *
 * Copyright (c) 2020 Seagate Technology LLC and/or its Affiliates
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * For any questions about this software or licensing,
 * please email opensource@seagate.com or cortx-questions@seagate.com. 
 */

/* @todo Move this code to utils/src/cortx/helpers.c file, after
 iterators are generalized. (EOS-4351)
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syscall.h> /* for gettid */
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <dirent.h>

#include "motr/client.h"
#include "motr/client_internal.h"
#include "motr/idx.h"
#include "lib/thread.h"
#include "m0common.h"
#include <motr/helpers/helpers.h>
#include "operation.h"

/* Key Iterator */

struct m0_key_iter_priv {
	struct m0_bufvec key;
	struct m0_bufvec val;
	struct m0_op *op;
	int rcs[1];
	bool initialized;
};

_Static_assert(sizeof(struct m0_key_iter_priv) <=
	       sizeof(((struct kvstore_iter*) NULL)->priv),
	       "m0_key_iter_priv does not fit into 'priv'");

static inline
struct m0_key_iter_priv *m0_key_iter_priv(struct kvstore_iter *iter)
{
	return (void *) &iter->priv[0];
}

void m0_key_iter_fini(struct kvstore_iter *iter)
{
	struct m0_key_iter_priv *priv = m0_key_iter_priv(iter);

	perfc_trace_inii(PFT_M0_KEY_ITER_FINISH, PEM_NSAL_NFS_TO_MOTR);
	if (!priv->initialized)
		goto out;

	m0_bufvec_free(&priv->key);
	m0_bufvec_free(&priv->val);

	if (priv->op) {
		perfc_trace_attr(PEA_M0_OP_SM_ID, priv->op->op_sm.sm_id);
		perfc_trace_attr(PEA_M0_OP_SM_STATE, priv->op->op_sm.sm_state);

		perfc_trace_attr(PEA_TIME_ATTR_START_NSAL_M0_OP_FINISH);
		m0_op_fini(priv->op);
		perfc_trace_attr(PEA_TIME_ATTR_END_NSAL_M0_OP_FINISH);

		perfc_trace_attr(PEA_TIME_ATTR_START_NSAL_M0_OP_FREE);
		m0_op_free(priv->op);
		perfc_trace_attr(PEA_TIME_ATTR_END_NSAL_M0_OP_FREE);
	}

out:
	perfc_trace_finii(PERFC_TLS_POP_DONT_VERIFY);
	return;
}

bool m0_key_iter_find(struct kvstore_iter *iter, const void* prefix,
		      size_t prefix_len)
{
	struct m0_key_iter_priv *priv = m0_key_iter_priv(iter);
	struct m0_bufvec *key = &priv->key;
	struct m0_bufvec *val = &priv->val;
	struct m0_op **op = &priv->op;
	struct m0_idx *index = iter->idx.index_priv;
	int rc;

	perfc_trace_inii(PFT_M0_KEY_ITER_FIND, PEM_NSAL_NFS_TO_MOTR);
	if (prefix_len == 0)
		rc = m0_bufvec_empty_alloc(key, 1);
	else
		rc = m0_bufvec_alloc(key, 1, prefix_len);
	if (rc != 0) {
		goto out;
	}

	rc = m0_bufvec_empty_alloc(val, 1);
	if (rc != 0) {
		goto out_free_key;
	}

	memcpy(priv->key.ov_buf[0], prefix, prefix_len);

	perfc_trace_attr(PEA_TIME_ATTR_START_M0_IDX_OP);
	rc = m0_idx_op(index, M0_IC_NEXT, &priv->key, &priv->val,
		       priv->rcs, 0, op);
	perfc_trace_attr(PEA_TIME_ATTR_END_M0_IDX_OP);

	if (rc != 0) {
		goto out_free_val;
	}

	perfc_trace_attr(PEA_TIME_ATTR_START_NSAL_M0_OP_LAUNCH);
	m0_op_launch(op, 1);
	perfc_trace_attr(PEA_TIME_ATTR_END_NSAL_M0_OP_LAUNCH);

	perfc_trace_attr(PEA_TIME_ATTR_START_NSAL_M0_OP_WAIT);
	rc = m0_op_wait(*op, M0_BITS(M0_OS_STABLE),
			M0_TIME_NEVER);
	perfc_trace_attr(PEA_TIME_ATTR_END_NSAL_M0_OP_WAIT);

	perfc_trace_attr(PEA_M0_OP_SM_ID, (*op)->op_sm.sm_id);
	perfc_trace_attr(PEA_M0_OP_SM_STATE, (*op)->op_sm.sm_state);

	if (rc != 0) {
		goto out_free_op;
	}

	if (priv->rcs[0] != 0) {
		goto out_free_op;
	}

	/* release objects back to priv */
	key = NULL;
	val = NULL;
	op = NULL;
	priv->initialized = true;

out_free_op:
	if (op && *op) {
		perfc_trace_attr(PEA_TIME_ATTR_START_NSAL_M0_OP_FINISH);
		m0_op_fini(*op);
		perfc_trace_attr(PEA_TIME_ATTR_END_NSAL_M0_OP_FINISH);

		perfc_trace_attr(PEA_TIME_ATTR_START_NSAL_M0_OP_FREE);
		m0_op_free(*op);
		perfc_trace_attr(PEA_TIME_ATTR_END_NSAL_M0_OP_FREE);
	}

out_free_val:
	if (val)
		m0_bufvec_free(val);
out_free_key:
	if (key)
		m0_bufvec_free(key);
out:
	if (rc != 0) {
		memset(&priv, 0, sizeof(*priv));
	}

	iter->inner_rc = rc;
	perfc_trace_finii(PERFC_TLS_POP_DONT_VERIFY);
	return rc == 0;
}

/** Make a non-empty bufvec to be an empty bufvec.
 * Frees internal buffers (to data) inside the bufvec
 * without freeing m0_bufvec::ov_buf and m0_bufvec::ov_bec::v_count.
 */
static void m0_bufvec_free_data(struct m0_bufvec *bufvec)
{
	uint32_t i;

	assert(bufvec->ov_buf);

	for (i = 0; i < bufvec->ov_vec.v_nr; ++i) {
		m0_free(bufvec->ov_buf[i]);
		bufvec->ov_buf[i] = NULL;
	}
}

bool m0_key_iter_next(struct kvstore_iter *iter)
{
	struct m0_key_iter_priv *priv = m0_key_iter_priv(iter);
	struct m0_idx *index = iter->idx.index_priv;
	bool can_get_next = false;

	perfc_trace_inii(PFT_M0_KEY_ITER_NEXT, PEM_NSAL_NFS_TO_MOTR);
	assert(priv->initialized);

	/* Motr API: "'vals' vector ... should contain NULLs" */
	m0_bufvec_free_data(&priv->val);

	perfc_trace_attr(PEA_TIME_ATTR_START_M0_IDX_OP);
	iter->inner_rc = m0_idx_op(index, M0_IC_NEXT,
				   &priv->key, &priv->val, priv->rcs,
				   M0_OIF_EXCLUDE_START_KEY,  &priv->op);
	perfc_trace_attr(PEA_TIME_ATTR_END_M0_IDX_OP);

	if (iter->inner_rc != 0) {
		goto out;
	}

	perfc_trace_attr(PEA_TIME_ATTR_START_NSAL_M0_OP_LAUNCH);
	m0_op_launch(&priv->op, 1);
	perfc_trace_attr(PEA_TIME_ATTR_END_NSAL_M0_OP_LAUNCH);

	perfc_trace_attr(PEA_TIME_ATTR_START_M0_OP_WAIT);
	iter->inner_rc = m0_op_wait(priv->op, M0_BITS(M0_OS_STABLE),
				    M0_TIME_NEVER);
	perfc_trace_attr(PEA_TIME_ATTR_END_M0_OP_WAIT);

	perfc_trace_attr(PEA_M0_OP_SM_ID, priv->op->op_sm.sm_id);
	perfc_trace_attr(PEA_M0_OP_SM_STATE, priv->op->op_sm.sm_state);
	if (iter->inner_rc != 0) {
		goto out;
	}

	iter->inner_rc = priv->rcs[0];

	if (iter->inner_rc == 0)
		can_get_next = true;

out:
	perfc_trace_finii(PERFC_TLS_POP_DONT_VERIFY);
	return can_get_next;
}

size_t m0_key_iter_get_key(struct kvstore_iter *iter, void **buf)
{
	struct m0_key_iter_priv *priv = m0_key_iter_priv(iter);
	struct m0_bufvec *v = &priv->key;
	*buf = v->ov_buf[0];
	return v->ov_vec.v_count[0];
}

size_t m0_key_iter_get_value(struct kvstore_iter *iter, void **buf)
{
	struct m0_key_iter_priv *priv = m0_key_iter_priv(iter);
	struct m0_bufvec *v = &priv->val;
	*buf = v->ov_buf[0];
	return v->ov_vec.v_count[0];
}

