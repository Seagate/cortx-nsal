/*
 * Filename: global_kvs.h
 * Description: Declaration for NSAL global kvs api
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
 
/** Global KV-Store operations **/
/* The functionality provided by below APIs is dependent on basic
 * framework provided kvstore.h
 */ 

#ifndef GLOBAL_KVS_H
#define GLOBAL_KVS_H

#include "kvstore.h"

struct global_kvs_hdl {
	kvs_idx_fid_t *fid;
	struct kvstore *kvstore;
	struct kvs_idx index;
};

/* Set an index mentioned by fid_str as global kvstore. Here it is assumed
 * that the index has been created. Below call just opens the index. */
int nsal_set_global_kvs(const char *fid_str);

/* Allocate memory */
int nsal_global_alloc(void **ptr, size_t size);

/* Free the allocated memory */
void nsal_global_free(void *ptr);

/* Store KV pair in the global kvs */
int nsal_global_set_kv(void *key, const size_t klen,
		       void *value, const size_t vlen);

/* Get KV pair stored in the global kvs */
int nsal_global_get_kv(void *key, const size_t klen,
		       void **value, size_t *vlen);

/* Delete the KV pair stored in the global kvs */
int nsal_global_del_kv(const void *key, size_t klen);


#endif /*GLOBAL_KVS_H */
