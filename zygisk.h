#ifndef ZYGISK_H
#define ZYGISK_H

#include <jni.h>
#include <stdint.h>
#include <sys/types.h>

#define ZYGISK_API_VERSION 4

typedef struct ZygiskApi ZygiskApi;
typedef struct AppSpecializeArgs AppSpecializeArgs;
typedef struct ServerSpecializeArgs ServerSpecializeArgs;
typedef struct ModuleBase ModuleBase;

struct AppSpecializeArgs {
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

struct ServerSpecializeArgs {
    jint *uid;
    jint *gid;
    jintArray *gids;
    jint *runtime_flags;
    jlong *permitted_capabilities;
    jlong *effective_capabilities;
};

struct ModuleBase {
    void (*onLoad)(ModuleBase *self, ZygiskApi *api, JNIEnv *env);
    void (*preAppSpecialize)(ModuleBase *self, AppSpecializeArgs *args);
    void (*postAppSpecialize)(ModuleBase *self, const AppSpecializeArgs *args);
    void (*preServerSpecialize)(ModuleBase *self, ServerSpecializeArgs *args);
    void (*postServerSpecialize)(ModuleBase *self, const ServerSpecializeArgs *args);
    void *user_data;
};

typedef enum {
    FORCE_DENYLIST_UNMOUNT = 0,
    DLCLOSE_MODULE_LIBRARY = 1
} ZygiskOption;

typedef enum {
    PROCESS_GRANTED_ROOT = (1u << 0),
    PROCESS_ON_DENYLIST = (1u << 1)
} ZygiskStateFlag;

typedef struct api_table api_table;

struct api_table {
    void *impl;
    int (*registerModule)(api_table *, void *);
    void (*hookJniNativeMethods)(JNIEnv *, const char *, JNINativeMethod *, int);
    void (*pltHookRegister)(dev_t, ino_t, const char *, void *, void **);
    int (*exemptFd)(int);
    int (*pltHookCommit)(void);
    int (*connectCompanion)(void *);
    void (*setOption)(void *, ZygiskOption);
    int (*getModuleDir)(void *);
    uint32_t (*getFlags)(void *);
};

struct ZygiskApi {
    api_table *tbl;
};

static inline int zygisk_connect_companion(ZygiskApi *api) {
    return api->tbl->connectCompanion ? api->tbl->connectCompanion(api->tbl->impl) : -1;
}

static inline int zygisk_get_module_dir(ZygiskApi *api) {
    return api->tbl->getModuleDir ? api->tbl->getModuleDir(api->tbl->impl) : -1;
}

static inline void zygisk_set_option(ZygiskApi *api, ZygiskOption opt) {
    if (api->tbl->setOption) api->tbl->setOption(api->tbl->impl, opt);
}

static inline uint32_t zygisk_get_flags(ZygiskApi *api) {
    return api->tbl->getFlags ? api->tbl->getFlags(api->tbl->impl) : 0;
}

static inline int zygisk_exempt_fd(ZygiskApi *api, int fd) {
    return api->tbl->exemptFd && api->tbl->exemptFd(fd);
}

static inline void zygisk_hook_jni_native_methods(ZygiskApi *api, JNIEnv *env, const char *className, JNINativeMethod *methods, int numMethods) {
    if (api->tbl->hookJniNativeMethods) api->tbl->hookJniNativeMethods(env, className, methods, numMethods);
}

static inline void zygisk_plt_hook_register(ZygiskApi *api, dev_t dev, ino_t inode, const char *symbol, void *newFunc, void **oldFunc) {
    if (api->tbl->pltHookRegister) api->tbl->pltHookRegister(dev, inode, symbol, newFunc, oldFunc);
}

static inline int zygisk_plt_hook_commit(ZygiskApi *api) {
    return api->tbl->pltHookCommit && api->tbl->pltHookCommit();
}

void zygisk_module_entry(api_table *table, JNIEnv *env) __attribute__((visibility("default")));
void zygisk_companion_entry(int client) __attribute__((visibility("default")));

#endif
