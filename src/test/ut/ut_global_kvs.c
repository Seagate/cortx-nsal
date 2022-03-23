/*
 * Filename: ut_global_kvs.c
 * Description: Implementation tests for global kvs support.
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
#include "debug.h"
#include "common/log.h"
#include "str.h"
#include "global_kvs.h"
#include "ut_nsal_ns.h"
#include "nsal.h"
#include <m0log.h>

#define DEFAULT_CONFIG "/etc/cortx/cortxfs.conf"

extern const int nsal_magic_symbol;

struct test_key {
	char type;
	str256_t name;
} test_key;

/* Set the global kvs */
static void test_nsal_set_global_kvs(void)
{
	struct collection_item *errors = NULL;
	struct collection_item *cfg_items = NULL;
	int rc = 0;

	rc = log_init("/var/log/cortx/fs/cortxfs.log", LEVEL_DEBUG);
	if (rc != 0) {
		rc = -EINVAL;
		printf("Log init failed, rc: %d\n", rc);
		goto out;
	}

	rc = config_from_file("libcortxfs", DEFAULT_CONFIG, &cfg_items,
			      INI_STOP_ON_ERROR, &errors);
	if (rc) {
		printf("Can't load config rc = %d", rc);
		rc = -rc;
		goto out;
	}

	rc = nsal_module_init(cfg_items);
	if (rc) {
		printf("nsal init failed rc = %d", rc);
		rc = -rc;
		goto out;
	}

	/* make use of kvs_fid as global kvs */
	rc = nsal_set_global_kvs("<0x780000000000000b:1>");
	if (rc) {
		printf("nsal_set_global_kvs failed rc = %d", rc);
		rc = -rc;
		goto out;
	}

out:
	if (rc) {
		free_ini_config_errors(errors);
	}

	ut_assert_int_equal(rc, 0);
}


/**
 * Test to add a KV pair in the global KVStore.
 */
static void test_nsal_global_set_kv(void)
{
	int rc = 0;
	char *name = "component";
	char *value = "cortxfs";
	struct test_key *key1;

	rc = nsal_global_alloc((void **)&key1, sizeof(struct test_key));
	ut_assert_int_equal(rc, 0);

	key1->type = 'T';
	str256_from_cstr(key1->name, name, strlen(name));

	rc = nsal_global_set_kv(key1, sizeof(struct test_key),
				 (void*)value, strlen(value)+1);
	ut_assert_int_equal(rc, 0);

	nsal_global_free(key1);
}

/**
 * Test to add a KV pair in the global KVStore.
 */
static void test_nsal_global_get_kv(void)
{
	int rc = 0;
	char *name = "component";
	char *value = "cortxfs";
	char *val = NULL;
	struct test_key *key1;
	size_t vlen=0;

	rc = nsal_global_alloc((void **)&key1, sizeof(struct test_key));
	ut_assert_int_equal(rc, 0);

	key1->type = 'T';
	str256_from_cstr(key1->name, name, strlen(name));

	rc = nsal_global_get_kv(key1, sizeof(struct test_key), (void**)&val,
			    &vlen);
	ut_assert_int_equal(rc, 0);

	rc = strcmp(value, val);
	ut_assert_int_equal(rc, 0);

	nsal_global_free(key1);
}

/**
 * Test to add a KV pair in the global KVStore.
 */
static void test_nsal_global_del_kv(void)
{
	int rc = 0;
	char *name = "component";
	char *val = NULL;
	size_t vlen=0;
	struct test_key *key1;

	rc = nsal_global_alloc((void **)&key1, sizeof(struct test_key));
	ut_assert_int_equal(rc, 0);

	key1->type = 'T';
	str256_from_cstr(key1->name, name, strlen(name));

	rc = nsal_global_del_kv(key1, sizeof(struct test_key));
	ut_assert_int_equal(rc, 0);

	/* try to retrive the deleted key */
	rc = nsal_global_get_kv(key1, sizeof(struct test_key), (void**)&val,
			    &vlen);
	ut_assert_int_equal(rc, -ENOENT);

	nsal_global_free(key1);
}


int main(int argc, char *argv[])
{
	int rc = 0;
	char *test_logs = "/var/log/cortx/test/ut/ut_nsal.logs";
	int test_count, test_failed;

	printf("Global KVS Tests\n");

	if (argc > 1 && strcmp(argv[1], "decode") == 0)
        {
                rc = decoder((const void*)&nsal_magic_symbol, argv[2], argv[3]);
                return rc;
        }

	rc = ut_load_config(CONF_FILE);
	if (rc != 0) {
		printf("ut_load_config: err = %d\n", rc);
		goto end;
	}

	test_logs = ut_get_config("nsal", "log_path", test_logs);

	rc = ut_init(test_logs);
	if (rc != 0) {
		printf("ut_init: err = %d\n", rc);
		goto out;
	}

	struct test_case test_list[] = {
		ut_test_case(test_nsal_set_global_kvs, NULL, NULL),
		ut_test_case(test_nsal_global_set_kv, NULL, NULL),
		ut_test_case(test_nsal_global_get_kv, NULL, NULL),
		ut_test_case(test_nsal_global_del_kv, NULL, NULL),
	};

	test_count = sizeof(test_list) / sizeof(test_list[0]);
	test_failed = ut_run(test_list, test_count, NULL, NULL);
	ut_fini();
	ut_summary(test_count, test_failed);

out:
	free(test_logs);

end:
	return rc;
}
