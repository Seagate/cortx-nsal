SET(base_kvstore_LIB_SRCS
    kvstore_base.c
    global_kvs.c
)

add_library(base_kvstore OBJECT ${base_kvstore_LIB_SRCS})

add_subdirectory(plugins/cortx)

if(USE_KVS_REDIS)
    add_subdirectory(plugins/redis)
	add_definitions(-DWITH_REDIS)
endif(USE_KVS_REDIS)
