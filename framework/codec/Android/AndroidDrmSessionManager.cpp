//
// Created by SuperMan on 2020/10/22.
//

#define  LOG_TAG "AndroidDrmSessionManager"
#include <utils/Android/GetStringUTFChars.h>
#include <utils/Android/JniUtils.h>
#include <utils/frame_work_log.h>
#include <utils/Android/FindClass.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/NewByteArray.h>
#include "AndroidDrmSessionManager.h"
#include "MediaCodec_Decoder.h"

extern "C" {
#include <utils/errors/framework_error.h>
}

using namespace Cicada;


static jclass jMediaDrmSessionClass = nullptr;
static jmethodID jMediaDrmSession_init = nullptr;
static jmethodID jMediaDrmSession_requireSession = nullptr;
static jmethodID jMediaDrmSession_releaseSession = nullptr;
static jmethodID jMediaDrmSession_isForceInsecureDecoder = nullptr;

static JNINativeMethod mediaCodec_method_table[] = {
        {"native_requestProvision", "(JLjava/lang/String;[B)[B", (void *) AndroidDrmSessionManager::requestProvision},
        {"native_requestKey",       "(JLjava/lang/String;[B)[B", (void *) AndroidDrmSessionManager::requestKey},
        {"native_changeState",      "(JII)V",                    (void *) AndroidDrmSessionManager::changeState},
        {"native_updateSessionId",  "(J[B)V",                    (void *) AndroidDrmSessionManager::updateSessionId},
};

int AndroidDrmSessionManager::registerMethod(JNIEnv *pEnv) {
    if (jMediaDrmSessionClass == nullptr) {
        return JNI_FALSE;
    }

    if (pEnv->RegisterNatives(jMediaDrmSessionClass, mediaCodec_method_table,
                              sizeof(mediaCodec_method_table) / sizeof(JNINativeMethod)) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

void AndroidDrmSessionManager::init(JNIEnv *env) {
    if (env == nullptr) {
        return;
    }

    if (jMediaDrmSessionClass == nullptr) {
        FindClass jClass(env, "com/cicada/player/utils/media/DrmSessionManager");
        jMediaDrmSessionClass = static_cast<jclass>(env->NewGlobalRef(jClass.getClass()));
        jMediaDrmSession_init = env->GetMethodID(jMediaDrmSessionClass, "<init>", "(J)V");
        jMediaDrmSession_requireSession = env->GetMethodID(jMediaDrmSessionClass,
                                                           "requireSession",
                                                           "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
        jMediaDrmSession_releaseSession = env->GetMethodID(jMediaDrmSessionClass, "releaseSession",
                                                           "()V");
        jMediaDrmSession_isForceInsecureDecoder = env->GetMethodID(jMediaDrmSessionClass, "isForceInsecureDecoder",
                                                           "()Z");
    }
}

void AndroidDrmSessionManager::unInit(JNIEnv *env) {
    if (env == nullptr) {
        return;
    }

    if (jMediaDrmSessionClass != nullptr) {
        env->DeleteGlobalRef(jMediaDrmSessionClass);
        jMediaDrmSessionClass = nullptr;
    }
}

bool AndroidDrmSessionManager::isForceInsecureDecoder() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return false;
    }

    jboolean  ret = env->CallBooleanMethod(mJDrmSessionManger, jMediaDrmSession_isForceInsecureDecoder);
    return (bool) ret;
}

AndroidDrmSessionManager::AndroidDrmSessionManager() {

    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }

    jobject pJobject = env->NewObject(jMediaDrmSessionClass, jMediaDrmSession_init, (jlong) this);
    mJDrmSessionManger = env->NewGlobalRef(pJobject);
    env->DeleteLocalRef(pJobject);
}

AndroidDrmSessionManager::~AndroidDrmSessionManager() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }

    if (mJDrmSessionManger != nullptr) {
        env->DeleteGlobalRef(mJDrmSessionManger);
    }

    if (mSessionId != nullptr) {
        free(mSessionId);
        mSessionId = nullptr;
    }

}

int AndroidDrmSessionManager::requireDrmSession(const std::string &keyUrl,
                                                const std::string &keyFormat,
                                                const std::string &mime,
                                                const std::string &licenseUrl) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return -1;
    }

    NewStringUTF jUrl(env, keyUrl.c_str());
    NewStringUTF jFormat(env, keyFormat.c_str());
    NewStringUTF jMime(env, mime.c_str());
    NewStringUTF jLicenseUrl(env, licenseUrl.c_str());

    AF_LOGI("drm requireDrmSession.,");
    env->CallVoidMethod(mJDrmSessionManger,
                                                              jMediaDrmSession_requireSession,
                                                              jUrl.getString(), jFormat.getString(),
                                                              jMime.getString(),
                                                              jLicenseUrl.getString());

    return 0;
}

void AndroidDrmSessionManager::releaseDrmSession() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }

    env->CallVoidMethod(mJDrmSessionManger, jMediaDrmSession_releaseSession);
}

void
AndroidDrmSessionManager::getSessionState(int *state, int *code) {

    std::unique_lock<std::mutex> lock(mStateMutex);
    *state = mState;
    *code = mErrorCode;
}

jbyteArray
AndroidDrmSessionManager::requestProvision(JNIEnv *env, jobject instance, jlong nativeIntance,
                                           jstring url, jbyteArray data) {

    AF_LOGI("drm requestProvision.,");
    if (nativeIntance == 0) {
        return nullptr;
    }

    auto *drmSessionManager = (AndroidDrmSessionManager *) (int64_t) nativeIntance;
    if (drmSessionManager->mRequestProvisionCallback != nullptr) {

        GetStringUTFChars cUrl(env, url);
        char *cData = JniUtils::jByteArrayToChars(env, data);
        int dataLen = env->GetArrayLength(data);

        void *responseData = nullptr;
        int responseDataSize = 0;

        drmSessionManager->mRequestProvisionCallback(&responseData, &responseDataSize,
                                                     cUrl.getChars(),
                                                     cData, dataLen,
                                                     drmSessionManager->mRequestUserData);

        free(cData);

        if (responseData == nullptr) {
            return nullptr;
        } else {
            jbyteArray mResult = env->NewByteArray(responseDataSize);
            env->SetByteArrayRegion(mResult, 0, responseDataSize, (jbyte *) (responseData));
            return mResult;
        }
    } else {
        return nullptr;
    }
}

jbyteArray
AndroidDrmSessionManager::requestKey(JNIEnv *env, jobject instance, jlong nativeIntance,
                                     jstring url,
                                     jbyteArray data) {
    AF_LOGI("drm requestKey.,");
    if (nativeIntance == 0) {
        return nullptr;
    }

    auto *drmSessionManager = (AndroidDrmSessionManager *) (int64_t) nativeIntance;
    if (drmSessionManager->mRequestLicenseCallback != nullptr) {

        GetStringUTFChars cUrl(env, url);
        char *cData = JniUtils::jByteArrayToChars(env, data);
        int dataLen = env->GetArrayLength(data);

        void *responseData = nullptr;
        int responseDataSize = 0;

        drmSessionManager->mRequestLicenseCallback(&responseData, &responseDataSize,
                                                   cUrl.getChars(),
                                                   cData, dataLen,
                                                   drmSessionManager->mRequestUserData);

        free(cData);
        AF_LOGD("requestKey , response data = %p , size = %d", responseData, responseDataSize);
        if (responseData == nullptr) {
            return nullptr;
        } else {
            jbyteArray mResult = env->NewByteArray(responseDataSize);
            env->SetByteArrayRegion(mResult, 0, responseDataSize, (jbyte *) (responseData));
            return mResult;
        }
    } else {
        return nullptr;
    }
}


void AndroidDrmSessionManager::updateSessionId(JNIEnv *env, jobject instance, jlong nativeInstance,
                            jbyteArray jSessionId) {


    if (jSessionId == nullptr) {
        return ;
    }

    auto *drmSessionManager = (AndroidDrmSessionManager *) (int64_t) nativeInstance;
    if(drmSessionManager == nullptr){
        return;
    }
    
    drmSessionManager->mSize = env->GetArrayLength(jSessionId);;
    drmSessionManager->mSessionId = JniUtils::jByteArrayToChars(env, jSessionId);
}

void AndroidDrmSessionManager::changeState(JNIEnv *env, jobject instance, jlong nativeIntance,
                                           jint state, jint errorCode) {

    auto *drmSessionManager = (AndroidDrmSessionManager *) (int64_t) nativeIntance;
    if (drmSessionManager == nullptr) {
        return;
    }

    int sessionState = SESSION_STATE_ERROR;
    if (state == 0) {
        sessionState = SESSION_STATE_OPENED;
        AF_LOGI("drm prepared OK");
    } else if (state == -1) {
        sessionState = SESSION_STATE_ERROR;
    } else if (state == -2) {
        sessionState = SESSION_STATE_IDLE;
    }

    drmSessionManager->changeStateInner(sessionState, (int) errorCode);

}

void AndroidDrmSessionManager::changeStateInner(int state,
                                                int errorCode) {
    std::unique_lock<std::mutex> lock(mStateMutex);

    AF_LOGI("drm change state %d , code = %d" , state , errorCode);

    mState = state;
    mErrorCode = gen_framework_errno(error_class_drm, errorCode);
}

void *AndroidDrmSessionManager::getSession(int *sessionSize) {
    if (mSessionId == nullptr) {
        return nullptr;
    }

    *sessionSize = mSize;
    return mSessionId;
}
