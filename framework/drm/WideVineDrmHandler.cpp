//
// Created by SuperMan on 11/27/20.
//

#define  LOG_TAG "WideVineDrmHandler"

#include "WideVineDrmHandler.h"
#include <utils/Android/GetStringUTFChars.h>
#include <utils/Android/JniUtils.h>
#include <utils/frame_work_log.h>
#include <utils/Android/FindClass.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/NewByteArray.h>
#include <utils/CicadaUtils.h>

extern "C" {
#include <utils/errors/framework_error.h>
}


static jclass jMediaDrmSessionClass = nullptr;
static jmethodID jMediaDrmSession_init = nullptr;
static jmethodID jMediaDrmSession_requireSession = nullptr;
static jmethodID jMediaDrmSession_releaseSession = nullptr;
static jmethodID jMediaDrmSession_isForceInsecureDecoder = nullptr;
using namespace Cicada;

WideVineDrmHandler WideVineDrmHandler::dummyWideVineHandler(0);

WideVineDrmHandler::WideVineDrmHandler(const DrmInfo &drmInfo)
        : IDrmHandler(drmInfo) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }

    jobject pJobject = env->NewObject(jMediaDrmSessionClass, jMediaDrmSession_init, (jlong) this);
    mJDrmSessionManger = env->NewGlobalRef(pJobject);
    env->DeleteLocalRef(pJobject);
}

WideVineDrmHandler::~WideVineDrmHandler() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }


    if (mJDrmSessionManger != nullptr) {
        env->CallVoidMethod(mJDrmSessionManger, jMediaDrmSession_releaseSession);
        env->DeleteGlobalRef(mJDrmSessionManger);
    }

    if (mSessionId != nullptr) {
        free(mSessionId);
        mSessionId = nullptr;
    }

}

int WideVineDrmHandler::getResult(char **dst, int64_t *dstSize) {

    std::unique_lock<std::mutex> lock(mDrmMutex);

    if (mSessionId != nullptr && mSize > 0) {
        *dst = static_cast<char *>(malloc(mSize));
        memcpy(*dst, mSessionId, mSize);
    }

    return 0;
}

IDrmHandler::State WideVineDrmHandler::getState() {
    std::unique_lock<std::mutex> lock(mDrmMutex);
    return mState;
}

void WideVineDrmHandler::open() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }

    NewStringUTF jUrl(env, drmInfo.uri.c_str());
    NewStringUTF jFormat(env, drmInfo.format.c_str());

    env->CallVoidMethod(mJDrmSessionManger,
                        jMediaDrmSession_requireSession,
                        jUrl.getString(), jFormat.getString(),
                        nullptr, "");
}

IDrmHandler *
WideVineDrmHandler::clone(const DrmInfo &drmInfo) {
    return new WideVineDrmHandler(drmInfo);
}

bool WideVineDrmHandler::is_supported(const DrmInfo &drmInfo) {
    return drmInfo.format == "urn:uuid:edef8ba9-79d6-4ace-a3c8-27dcd51d21ed";
}

static JNINativeMethod mediaCodec_method_table[] = {
        {"native_requestProvision", "(JLjava/lang/String;[B)[B", (void *) WideVineDrmHandler::requestProvision},
        {"native_requestKey",       "(JLjava/lang/String;[B)[B", (void *) WideVineDrmHandler::requestKey},
        {"native_changeState",      "(JII)V",                    (void *) WideVineDrmHandler::changeState},
        {"native_updateSessionId",  "(J[B)V",                    (void *) WideVineDrmHandler::updateSessionId},
};

int WideVineDrmHandler::registerMethod(JNIEnv *pEnv) {
    if (jMediaDrmSessionClass == nullptr) {
        return JNI_FALSE;
    }

    if (pEnv->RegisterNatives(jMediaDrmSessionClass, mediaCodec_method_table,
                              sizeof(mediaCodec_method_table) / sizeof(JNINativeMethod)) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

void WideVineDrmHandler::init(JNIEnv *env) {
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
        jMediaDrmSession_isForceInsecureDecoder = env->GetMethodID(jMediaDrmSessionClass,
                                                                   "isForceInsecureDecoder",
                                                                   "()Z");
    }
}

void WideVineDrmHandler::unInit(JNIEnv *env) {
    if (env == nullptr) {
        return;
    }

    if (jMediaDrmSessionClass != nullptr) {
        env->DeleteGlobalRef(jMediaDrmSessionClass);
        jMediaDrmSessionClass = nullptr;
    }
}

bool WideVineDrmHandler::isForceInsecureDecoder() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return false;
    }

    jboolean ret = env->CallBooleanMethod(mJDrmSessionManger,
                                          jMediaDrmSession_isForceInsecureDecoder);
    return (bool) ret;
}

void WideVineDrmHandler::updateSessionId(JNIEnv *env, jobject instance, jlong nativeInstance,
                                         jbyteArray jSessionId) {

    if (jSessionId == nullptr) {
        return;
    }

    auto *drmSessionManager = (WideVineDrmHandler *) (int64_t) nativeInstance;
    if (drmSessionManager == nullptr) {
        return;
    }

    {
        std::unique_lock<std::mutex> lock(drmSessionManager->mDrmMutex);
        drmSessionManager->mSize = env->GetArrayLength(jSessionId);;
        drmSessionManager->mSessionId = JniUtils::jByteArrayToChars(env, jSessionId);
    }
}

void WideVineDrmHandler::changeState(JNIEnv *env, jobject instance, jlong nativeIntance,
                                     jint state, jint errorCode) {

    auto *drmSessionManager = (WideVineDrmHandler *) (int64_t) nativeIntance;
    if (drmSessionManager == nullptr) {
        return;
    }

    {
        std::unique_lock<std::mutex> lock(drmSessionManager->mDrmMutex);

        if (state == 0) {
            drmSessionManager->mState = State::Ready;
            AF_LOGI("drm prepared OK");
        } else if (state == -1) {
            drmSessionManager->mState = State::Error;
        } else if (state == -2) {
            drmSessionManager->mState = State::Idle;
        }

        drmSessionManager->mErrorCode = gen_framework_errno(error_class_drm, errorCode);
    }
}


jbyteArray
WideVineDrmHandler::requestProvision(JNIEnv *env, jobject instance, jlong nativeIntance,
                                     jstring url, jbyteArray data) {

    AF_LOGI("drm requestProvision.,");
    if (nativeIntance == 0) {
        return nullptr;
    }

    auto *drmSessionManager = (WideVineDrmHandler *) (int64_t) nativeIntance;
    if (drmSessionManager->drmCallback != nullptr) {

        GetStringUTFChars cUrl(env, url);
        char *cData = JniUtils::jByteArrayToChars(env, data);
        int dataLen = env->GetArrayLength(data);
        std::map<std::string, std::string> requestParam{};
        requestParam["type"] = "provision";
        requestParam["url"] = std::string(cUrl.getChars());
        requestParam["data"] = CicadaUtils::base64enc(cData, dataLen);
        std::map<std::string, std::string> result = drmSessionManager->drmCallback(requestParam);

        free(cData);

        char *responseData = nullptr;
        int responseDataSize = 0;

        if (result.count("responseData") != 0) {
            responseDataSize = CicadaUtils::base64dec(result["responseData"], &responseData);
        }

        if (responseData == nullptr) {
            return nullptr;
        } else {
            jbyteArray mResult = env->NewByteArray(responseDataSize);
            env->SetByteArrayRegion(mResult, 0, responseDataSize, (jbyte *) (responseData));

            free(responseData);

            return mResult;
        }
    } else {
        return nullptr;
    }
}

jbyteArray
WideVineDrmHandler::requestKey(JNIEnv *env, jobject instance, jlong nativeIntance,
                               jstring url,
                               jbyteArray data) {
    AF_LOGI("drm requestKey.,");
    if (nativeIntance == 0) {
        return nullptr;
    }

    auto *drmSessionManager = (WideVineDrmHandler *) (int64_t) nativeIntance;
    if (drmSessionManager->drmCallback != nullptr) {

        GetStringUTFChars cUrl(env, url);
        char *cData = JniUtils::jByteArrayToChars(env, data);
        int dataLen = env->GetArrayLength(data);
        std::map<std::string, std::string> requestParam{};
        requestParam["type"] = "key";
        requestParam["url"] = std::string(cUrl.getChars());
        requestParam["data"] = CicadaUtils::base64enc(cData, dataLen);
        std::map<std::string, std::string> result = drmSessionManager->drmCallback(requestParam);

        free(cData);

        char *responseData = nullptr;
        int responseDataSize = 0;

        if (result.count("responseData") != 0) {
            responseDataSize = CicadaUtils::base64dec(result["responseData"], &responseData);
        }

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


