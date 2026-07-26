#include <jni.h>
#include <string.h>

#include "zygisk.h"

static bool g_should_inject = false;
static char g_process_name[256] = {0};
static struct zygisk_api_table *g_api = NULL;
static JNIEnv *g_env = NULL;

static void spoof_field(JNIEnv *env, jclass clazz, const char *field_name, const char *value) {
    jfieldID field_id = (*env)->GetStaticFieldID(env, clazz, field_name, "Ljava/lang/String;");
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        return;
    }
    if (field_id == NULL) {
        return;
    }

    jstring jval = (*env)->NewStringUTF(env, value);
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        return;
    }
    if (jval == NULL) {
        return;
    }

    (*env)->SetStaticObjectField(env, clazz, field_id, jval);
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
    }
    (*env)->DeleteLocalRef(env, jval);
}

static void preAppSpecialize(void *impl, struct zygisk_app_specialize_args *args) {
    if (g_env == NULL || args == NULL) {
        return;
    }

    if (args->nice_name == NULL || *(args->nice_name) == NULL) {
        return;
    }

    jstring nice_name_jstr = *(args->nice_name);
    const char *nice_name = (*g_env)->GetStringUTFChars(g_env, nice_name_jstr, NULL);
    if (nice_name == NULL) {
        return;
    }

    size_t name_len = strlen(nice_name);
    if (name_len >= sizeof(g_process_name)) {
        name_len = sizeof(g_process_name) - 1;
    }
    memcpy(g_process_name, nice_name, name_len);
    g_process_name[name_len] = '\0';

    (*g_env)->ReleaseStringUTFChars(g_env, nice_name_jstr, nice_name);

    if (strstr(g_process_name, "com.google.android.apps.photos") != NULL) {
        g_should_inject = true;
    } else {
        g_should_inject = false;
    }
}

static void postAppSpecialize(void *impl, const struct zygisk_app_specialize_args *args) {
    if (g_should_inject) {
        if (g_env == NULL) {
            if (g_api != NULL) {
                g_api->setOption(g_api->impl, ZYGISK_DLCLOSE_MODULE_LIBRARY);
            }
            return;
        }

        jclass build_class = (*g_env)->FindClass(g_env, "android/os/Build");
        if (build_class == NULL) {
            (*g_env)->ExceptionClear(g_env);
            if (g_api != NULL) {
                g_api->setOption(g_api->impl, ZYGISK_DLCLOSE_MODULE_LIBRARY);
            }
            return;
        }

        spoof_field(g_env, build_class, "BRAND", "google");
        spoof_field(g_env, build_class, "MANUFACTURER", "Google");
        spoof_field(g_env, build_class, "MODEL", "Pixel XL");
        spoof_field(g_env, build_class, "FINGERPRINT", "google/marlin/marlin:10/QP1A.191005.007.A3/5972272:user/release-keys");

        if ((*g_env)->ExceptionCheck(g_env)) {
            (*g_env)->ExceptionClear(g_env);
        }

    }

    if (g_api != NULL) {
        g_api->setOption(g_api->impl, ZYGISK_DLCLOSE_MODULE_LIBRARY);
    }
}

static struct zygisk_module_abi g_abi = {
    .api_version = ZYGISK_API_VERSION,
    .impl = NULL,
    .preAppSpecialize = preAppSpecialize,
    .postAppSpecialize = postAppSpecialize,
    .preServerSpecialize = NULL,
    .postServerSpecialize = NULL
};

__attribute__((visibility("default")))
void zygisk_module_entry(struct zygisk_api_table *table, JNIEnv *env) {
    g_api = table;
    g_env = env;
    if (!table->registerModule(table, &g_abi)) {
        return;
    }
}
