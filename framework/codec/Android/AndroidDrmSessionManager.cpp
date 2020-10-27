//
// Created by SuperMan on 2020/10/22.
//

#include <utils/Android/GetStringUTFChars.h>
#include <utils/Android/JniUtils.h>
#include <utils/frame_work_log.h>
#include <utils/Android/FindClass.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/NewByteArray.h>
#include "AndroidDrmSessionManager.h"
#include "MediaCodec_Decoder.h"

using namespace Cicada;


static jclass jMediaDrmSessionClass = nullptr;
static jmethodID jMediaDrmSession_init = nullptr;
static jmethodID jMediaDrmSession_requireSession = nullptr;
static jmethodID jMediaDrmSession_releaseSession = nullptr;

static JNINativeMethod mediaCodec_method_table[] = {
        {"native_requestProvision", "(JLjava/lang/String;[B)[B", (void *) AndroidDrmSessionManager::requestProvision},
        {"native_requestKey",       "(JLjava/lang/String;[B)[B", (void *) AndroidDrmSessionManager::requestKey},
        {"native_changeState",      "(J[BI)V",                   (void *) AndroidDrmSessionManager::changeState},
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
                                                           "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)[B");
        jMediaDrmSession_releaseSession = env->GetMethodID(jMediaDrmSessionClass, "releaseSession",
                                                           "([B)V");
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

AndroidDrmSessionManager::AndroidDrmSessionManager() {

    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }

    jobject pJobject = env->NewObject(jMediaDrmSessionClass, jMediaDrmSession_init, (jlong) this);
    mDrmSessionManger = env->NewGlobalRef(pJobject);
    env->DeleteLocalRef(pJobject);
}

AndroidDrmSessionManager::~AndroidDrmSessionManager() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }

    if (mDrmSessionManger != nullptr) {
        env->DeleteGlobalRef(mDrmSessionManger);
    }

    mSessionStates.clear();

}

int AndroidDrmSessionManager::requireDrmSession(void **dstSessionId, int *dstSessionSize,
                                                const std::string &keyUrl,
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

    jbyteArray sessionId = (jbyteArray) env->CallObjectMethod(mDrmSessionManger,
                                                              jMediaDrmSession_requireSession,
                                                              jUrl.getString(), jFormat.getString(),
                                                              jMime.getString(),
                                                              jLicenseUrl.getString());

    if (sessionId == nullptr) {
        return -1;
    }

    *dstSessionSize = env->GetArrayLength(sessionId);;
    *dstSessionId = JniUtils::jByteArrayToChars(env, sessionId);

    env->DeleteLocalRef(sessionId);

    return 0;
}

void AndroidDrmSessionManager::releaseDrmSession(void *sessionId, int sessionSize) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }

    NewByteArray jSessionId(env, sessionId, sessionSize);
    env->CallVoidMethod(mDrmSessionManger, jMediaDrmSession_releaseSession, jSessionId.getArray());
}

int AndroidDrmSessionManager::getSessionState(void *sessionId, int sessionSize) {

    std::unique_lock<std::mutex> lock(mStateMutex);

    if (!mSessionStates.empty()) {
        for (SessionState &sessionState : mSessionStates) {
            if (sessionState.mSize == sessionSize && memcmp(sessionState.mId, sessionId, sessionSize) == 0) {
               return sessionState.mState;
            }
        }
    }

    return SESSION_STATE_ERROR;
}

jbyteArray
AndroidDrmSessionManager::requestProvision(JNIEnv *env, jobject instance, jlong nativeIntance,
                                           jstring url, jbyteArray data) {
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

void AndroidDrmSessionManager::changeState(JNIEnv *env, jobject instance, jlong nativeIntance,
                                           jbyteArray data, jint state) {

    auto *drmSessionManager = (AndroidDrmSessionManager *) (int64_t) nativeIntance;
    if (drmSessionManager == nullptr) {
        return;
    }

    int sessionSize = env->GetArrayLength(data);;
    char *sessionId = JniUtils::jByteArrayToChars(env, data);

    if (sessionId == nullptr) {
        return;
    }
    int sessionState = SESSION_STATE_ERROR;
    if (state == 0) {
        sessionState = SESSION_STATE_OPENED;
    } else if (state == -1) {
        sessionState = SESSION_STATE_RELEASED;
    } else if (state == -2) {
        sessionState = SESSION_STATE_ERROR;
    } else if (state == -3) {
        sessionState = SESSION_STATE_IDLE;
    }

    drmSessionManager->changeStateInner(sessionId, sessionSize, sessionState);

    free(sessionId);
}

void AndroidDrmSessionManager::changeStateInner(char *sessionId, int sessionSize, int state) {

    std::unique_lock<std::mutex> lock(mStateMutex);
    if (!mSessionStates.empty()) {
        for (SessionState &sessionState : mSessionStates) {
            if (sessionState.mSize == sessionSize && memcmp(sessionState.mId, sessionId, sessionSize) == 0) {
                sessionState.mState = state;
                return;
            }
        }
    }

    SessionState sessionState(sessionId, sessionSize, state);
    mSessionStates.push_back(sessionState);

}
