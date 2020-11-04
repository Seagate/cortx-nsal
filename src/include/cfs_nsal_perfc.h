/*
 * Filename:	cfs_nsal_perfc.h
 * Description:	This module defines performance counters and helpers.
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

#ifndef __CFS_NSAL_PERF_COUNTERS_H_
#define __CFS_NSAL_PERF_COUNTERS_H_
/******************************************************************************/
#include "perf/tsdb.h" /* ACTION_ID_BASE */
#include "operation.h"
#include <pthread.h>
#include <string.h>
#include "debug.h"
#include "perf/perf-counters.h"

enum perfc_nsal_function_tags {
	PFT_KVS_START = PFTR_RANGE_4_START,

	PFT_KVS_INIT,
	PFT_KVS_FINI,
	PFT_KVS_ALLOC,
	PFT_KVS_FREE,
	PFT_KVS_GET,
	PFT_KVS_SET,
	PFT_KVTREE_ITER_CH,

	PFT_CORTX_KVS_INIT,
	PFT_CORTX_KVS_FINISH,
	PFT_CORTX_KVS_ALLOC,
	PFT_CORTX_KVS_FREE,
	PFT_CORTX_KVS_INDEX_CREATE,
	PFT_CORTX_KVS_INDEX_DELETE,
	PFT_CORTX_KVS_INDEX_OPEN,
	PFT_CORTX_KVS_INDEX_CLOSE,
	PFT_CORTX_KVS_GET_BIN,
	PFT_CORTX_KVS4_GET_BIN,
	PFT_CORTX_KVS_SET_BIN,
	PFT_CORTX_KVS4_SET_BIN,
	PFT_CORTX_KVS_DELETE_BIN,
	PFT_CORTX_KVS_GEN_FID,
	PFT_CORTX_KVS_PREFIX_ITER_FIND,
	PFT_CORTX_KVS_PREFIX_ITER_NEXT,
	PFT_CORTX_KVS_PREFIX_ITER_FINISH,
	PFT_CORTX_KVS_ITER_GET_KV,
	PFT_CORTX_KVS_GET_LIST_SIZE,

	PFT_M0_KEY_ITER_FINISH,
	PFT_M0_KEY_ITER_FIND,
	PFT_M0_KEY_ITER_NEXT,

	PFT_KVS_END = PFTR_RANGE_4_END,
};

enum perfc_nsal_entity_attrs {
	PEA_KVS_START = PEAR_RANGE_4_START,

	PEA_KVS_ALLOC_SIZE,
	PEA_KVS_ALLOC_RES_RC,
	PEA_KVS_KLEN,
	PEA_KVS_VLEN,
	PEA_KVS_RES_RC,

	PEA_TIME_ATTR_START_NSAL_M0_OP_FINISH,
	PEA_TIME_ATTR_END_NSAL_M0_OP_FINISH,
	PEA_TIME_ATTR_START_NSAL_M0_OP_FREE,
	PEA_TIME_ATTR_END_NSAL_M0_OP_FREE,
	PEA_TIME_ATTR_START_NSAL_M0_OP_LAUNCH,
	PEA_TIME_ATTR_END_NSAL_M0_OP_LAUNCH,
	PEA_TIME_ATTR_START_NSAL_M0_OP_WAIT,
	PEA_TIME_ATTR_END_NSAL_M0_OP_WAIT,

	PEA_NS_RES_RC,
	PEA_KVS_LIST_SIZE,

	PEA_KVS_END = PEAR_RANGE_4_END
};

enum perfc_nsal_entity_maps {
	PEM_KVS_START = PEMR_RANGE_4_START,

	PEM_NSAL_NFS_TO_MOTR,
	PEM_NSAL_TO_MOTR,
	PEM_KVS_TO_NFS,

	PEM_KVS_END = PEMR_RANGE_4_END
};

/******************************************************************************/
#endif /* __CFS_NSAL_PERF_COUNTERS_H_ */
