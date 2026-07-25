#include <stdio.h>
#include <string.h>
#include <android/log.h>

#include "zygisk.h"
#include "gpx.h"

static const char PIXEL_XL_FP[] = "google/marlin/marlin:10/QP1A.191005.007.A3/5972272:user/release-keys";

static struct {
    char name[256];
    int inject;
} g_app;

static ZygiskApi g_api_val;
static ZygiskApi *g_api = &g_api_val;
static JNIEnv *g_env;

static void set_static_string_field(JNIEnv *env, jclass clazz, const char *fieldName, jstring value) {
    jfieldID fieldId = (*env)->GetStaticFieldID(env, clazz, fieldName, "Ljava/lang/String;");
    if (fieldId != NULL) {
        (*env)->SetStaticObjectField(env, clazz, fieldId, value);
    }
}

static void inject_build(JNIEnv *env, const char *pkg_name, const char *model, const char *product, const char *finger) {
    if (env == NULL) {
        LOGW("env null, failed to inject build for %s", pkg_name);
        return;
    }

    jclass build_class = (*env)->FindClass(env, "android/os/Build");
    if (build_class == NULL) {
        LOGW("build_class null, failed to inject build for %s", pkg_name);
        return;
    }

    LOGI("inject build for %s with \nPRODUCT:%s \nMODEL:%s \nFINGERPRINT:%s", pkg_name, product, model, finger);

    jstring google_str = (*env)->NewStringUTF(env, "Google");
    jstring product_str = (*env)->NewStringUTF(env, product);
    jstring model_str = (*env)->NewStringUTF(env, model);
    jstring finger_str = (*env)->NewStringUTF(env, finger);

    set_static_string_field(env, build_class, "BRAND", google_str);
    set_static_string_field(env, build_class, "MANUFACTURER", google_str);
    set_static_string_field(env, build_class, "PRODUCT", product_str);
    set_static_string_field(env, build_class, "DEVICE", product_str);
    set_static_string_field(env, build_class, "MODEL", model_str);
    set_static_string_field(env, build_class, "FINGERPRINT", finger_str);

    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
    }

    (*env)->DeleteLocalRef(env, google_str);
    (*env)->DeleteLocalRef(env, product_str);
    (*env)->DeleteLocalRef(env, model_str);
    (*env)->DeleteLocalRef(env, finger_str);
}

static void pre_app_specialize(ModuleBase *self, AppSpecializeArgs *args) {
    JNIEnv *env = g_env;
    if (env == NULL || args == NULL || args->nice_name == NULL || *args->nice_name == NULL) {
        return;
    }

    const char *process = (*env)->GetStringUTFChars(env, *args->nice_name, NULL);
    if (process == NULL) {
        return;
    }
    snprintf(g_app.name, sizeof(g_app.name), "%s", process);

    g_app.inject = (strstr(process, "com.google.android.apps.photos") != NULL) ||
                    (strstr(process, "com.google.android.gms") != NULL);

    (*env)->ReleaseStringUTFChars(env, *args->nice_name, process);
}

static void post_app_specialize(ModuleBase *self, const AppSpecializeArgs *args) {
    if (g_app.inject) {
        inject_build(g_env, g_app.name, "Pixel XL", "marlin", PIXEL_XL_FP);
    }

    zygisk_set_option(g_api, DLCLOSE_MODULE_LIBRARY);
}

typedef struct {
    long api_version;
    ModuleBase *impl;
    void (*preAppSpecialize)(ModuleBase *, AppSpecializeArgs *);
    void (*postAppSpecialize)(ModuleBase *, const AppSpecializeArgs *);
    void (*preServerSpecialize)(ModuleBase *, ServerSpecializeArgs *);
    void (*postServerSpecialize)(ModuleBase *, const ServerSpecializeArgs *);
} module_abi;

static ModuleBase g_base;
static module_abi g_abi;

void zygisk_module_entry(api_table *table, JNIEnv *env) {
    g_api->tbl = table;
    g_env = env;

    g_base.preAppSpecialize = pre_app_specialize;
    g_base.postAppSpecialize = post_app_specialize;
    g_base.preServerSpecialize = NULL;
    g_base.postServerSpecialize = NULL;
    g_base.user_data = NULL;

    g_abi.api_version = ZYGISK_API_VERSION;
    g_abi.impl = &g_base;
    g_abi.preAppSpecialize = pre_app_specialize;
    g_abi.postAppSpecialize = post_app_specialize;
    g_abi.preServerSpecialize = NULL;
    g_abi.postServerSpecialize = NULL;

    table->registerModule(table, &g_abi);
}
