/*
 * Filename : global_kvs.c
 * Description : 
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

#include <errno.h>
#include <string.h>
#include "kvstore.h"
#include "global_kvs.h"

/* Global definitions */
struct global_kvs_hdl g_kvs_hdl;
extern bool kvs_init_done;

/* Set an index mentioned by fid_str as global kvstore. Here it is assumed that
 * the index is created */
int nsal_set_global_kvs(const char *fid_str)
{
	int rc = 0;

	if (g_kvs_hdl.fid != NULL) {
		log_err("Global KVS is already set \n");
		rc = -EEXIST;
		goto out;
	}
	if (!kvs_init_done) {
		log_err("kvs_init is not done \n");
		rc = -EINVAL;
		goto out;
	}
	g_kvs_hdl.fid = malloc(sizeof(kvs_idx_fid_t));
	if (g_kvs_hdl.fid == NULL) {
		log_err("Unable to allocate memory \n");
		goto out;
	}
	rc = kvs_fid_from_str(fid_str, g_kvs_hdl.fid);
	if (rc) {
		log_err("Failed to get fid \n");
		free(g_kvs_hdl.fid);
		goto out;
	}
	g_kvs_hdl.kvstore = kvstore_get();
	rc = kvs_index_open(g_kvs_hdl.kvstore, g_kvs_hdl.fid, &g_kvs_hdl.index);
	if (rc) {
		log_err("Failed to open the index \n");
		free(g_kvs_hdl.fid);
		goto out;
	}

	log_test("TEST TEST TEST Into NSAL module \n");
out:
	return rc;
}

/* Memory allocation for key */
int nsal_global_alloc(void **ptr, size_t size)
{
	return kvs_alloc(g_kvs_hdl.kvstore, ptr, size);
}

/* free up the allocated memory */
void nsal_global_free(void *ptr)
{
	kvs_free(g_kvs_hdl.kvstore, ptr);
}

/* Store KV pair in the global kvs */
int nsal_global_set_kv(void *key, const size_t klen,
                       void *value, const size_t vlen)
{
	int rc = 0;

	if (g_kvs_hdl.fid == NULL) {
		log_err("Global KVS is not set \n");
		rc = -EINVAL;
		goto out;
	}
	rc = kvs_set(g_kvs_hdl.kvstore, &g_kvs_hdl.index, key, klen,
		     value, vlen);

out:
	return rc;
}

/* Get KV pair stored in the global kvs */
int nsal_global_get_kv(void *key, const size_t klen,
                       void **value, size_t *vlen)
{
	int rc = 0;

	if (g_kvs_hdl.fid == NULL) {
		log_err("Global KVS is not set \n");
		rc = -EINVAL;
		goto out;
	}
	rc = kvs_get(g_kvs_hdl.kvstore, &g_kvs_hdl.index, key, klen,
		     value, vlen);

out:
	return rc;
}

/* Delete the KV pair stored in the global kvs */
int nsal_global_del_kv(const void *key, size_t klen)
{
	int rc = 0;

	if (g_kvs_hdl.fid == NULL) {
		log_err("Global KVS is not set \n");
		rc = -EINVAL;
		goto out;
	}
	rc = kvs_del(g_kvs_hdl.kvstore, &g_kvs_hdl.index, key, klen);

out:
	return rc;
}
