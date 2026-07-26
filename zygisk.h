#ifndef ZYGISK_H
#define ZYGISK_H

#include <jni.h>
#include <stdbool.h>
#include <sys/types.h>

#define ZYGISK_API_VERSION 4

typedef enum {
    ZYGISK_FORCE_DENYLIST_UNMOUNT = 0,
    ZYGISK_DLCLOSE_MODULE_LIBRARY = 1,
} zygisk_option;

typedef enum {
    ZYGISK_PROCESS_GRANTED_ROOT = (1u << 0),
    ZYGISK_PROCESS_ON_DENYLIST = (1u << 1),
} zygisk_state_flag;

struct zygisk_app_specialize_args {
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jobjectArray *rlimits;
    jint *mount_external;
    jstring *se_info;
    jstring *nice_name;
    jstring *instruction_set;
    jstring *app_data_dir;

    jintArray *fds_to_ignore;
    jboolean *is_child_zygote;
    jboolean *is_top_app;
    jobjectArray *pkg_data_info_list;
    jobjectArray *whitelisted_data_info_list;
    jboolean *mount_data_dirs;
    jboolean *mount_storage_dirs;
};

struct zygisk_server_specialize_args {
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

struct zygisk_module_abi;
struct zygisk_api_table;

struct zygisk_module_abi {
    long api_version;
    void *impl;
    void (*preAppSpecialize)(void *impl, struct zygisk_app_specialize_args *args);
    void (*postAppSpecialize)(void *impl, const struct zygisk_app_specialize_args *args);
    void (*preServerSpecialize)(void *impl, struct zygisk_server_specialize_args *args);
    void (*postServerSpecialize)(void *impl, const struct zygisk_server_specialize_args *args);
};

struct zygisk_api_table {
    void *impl;
    bool (*registerModule)(struct zygisk_api_table *table, struct zygisk_module_abi *abi);

    void (*hookJniNativeMethods)(JNIEnv *env, const char *className, JNINativeMethod *methods, int numMethods);
    void (*pltHookRegister)(dev_t dev, ino_t inode, const char *symbol, void *newFunc, void **oldFunc);
    bool (*exemptFd)(int fd);
    bool (*pltHookCommit)();
    int  (*connectCompanion)(void *impl);
    void (*setOption)(void *impl, zygisk_option opt);
    int  (*getModuleDir)(void *impl);
    uint32_t (*getFlags)(void *impl);
};

#endif
