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
#include <cassert>
#include <codec/Android/IWideVineDecoder.h>

extern "C" {
#include <utils/errors/framework_error.h>
#include <libavutil/intreadwrite.h>
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

void WideVineDrmHandler::open() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }

    NewStringUTF jUrl(env, drmInfo.uri.c_str());
    NewStringUTF jFormat(env, drmInfo.format.c_str());
    bSessionRequested = true;
    jstring pJurl = jUrl.getString();
    jstring pJformat = jFormat.getString();
    env->CallVoidMethod(mJDrmSessionManger,
                        jMediaDrmSession_requireSession,
                        pJurl, pJformat,
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
        drmSessionManager->mSessionSize = env->GetArrayLength(jSessionId);;
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
            drmSessionManager->mState = SESSION_STATE_OPENED;
            AF_LOGI("drm prepared OK");
        } else if (state == -1) {
            drmSessionManager->mState = SESSION_STATE_ERROR;
        } else if (state == -2) {
            drmSessionManager->mState = SESSION_STATE_IDLE;
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
        requestParam["drmType"] = "WideVine";
        requestParam["requestType"] = "provision";
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
        requestParam["drmType"] = "WideVine";
        requestParam["requestType"] = "key";
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

int WideVineDrmHandler::initDecoder(void *pDecoder) {

    auto *wideVineDecoder = static_cast<IWideVineDecoder *>(pDecoder);
    if (wideVineDecoder == nullptr) {
        return -1;
    }

    std::unique_lock<std::mutex> lock(mDrmMutex);
    if (!bSessionRequested) {
        open();
    }

    if (mState == SESSION_STATE_OPENED) {
        bool insecure = isForceInsecureDecoder();
        wideVineDecoder->setWideVineForceInSecureDecoder(insecure);
        wideVineDecoder->setWideVineSession("edef8ba9-79d6-4ace-a3c8-27dcd51d21ed", mSessionId,
                                            mSessionSize);
        return 0;
    } else if (mState == SESSION_STATE_IDLE) {
        return -EAGAIN;
    } else if (mState == SESSION_STATE_ERROR) {
        return mErrorCode;
    }
    return -EAGAIN;
}

void WideVineDrmHandler::convertData(int naluLengthSize, uint8_t **new_data, int *new_size,
                                     const uint8_t *data,
                                     int size) {
    if (data == nullptr || size == 0) {
        return;
    }

    if (naluLengthSize == 0) {
        return;
    }

    const int nalPrefixLen = 5;
    char nalPrefixData[nalPrefixLen];
    nalPrefixData[0] = 0;
    nalPrefixData[1] = 0;
    nalPrefixData[2] = 0;
    nalPrefixData[3] = 0;
    nalPrefixData[4] = 0;

    int sampleBytesWritten = 0;
    int sampleSize = size;

    std::vector<uint8_t> tmpData{};

    int nalUnitPrefixLength = naluLengthSize + 1;
    int nalUnitLengthFieldLengthDiff = 4 - naluLengthSize;
    int sampleCurrentNalBytesRemaining = 0;

    *new_size = size;
    *new_data = static_cast<uint8_t *>(malloc(*new_size));
    uint8_t *new_data_ptr = *new_data;
    uint8_t *ori_data_ptr = const_cast<uint8_t *>(data);

    while (sampleBytesWritten < sampleSize) {

        if (sampleCurrentNalBytesRemaining == 0) {
//new nal
            for (int i = 0; i < nalUnitPrefixLength; i++) {
                nalPrefixData[nalUnitLengthFieldLengthDiff + i] = *ori_data_ptr;
                ori_data_ptr++;
            }

            int nalLengthInt = AV_RB32(nalPrefixData);
            if (nalLengthInt < 1) {
                AF_LOGE("Invalid NAL length");
                return;
            }

            sampleCurrentNalBytesRemaining = nalLengthInt - 1;

            if (sampleBytesWritten + 5 > *new_size) {

                *new_size = sampleBytesWritten + 5;

                *new_data = static_cast<uint8_t *>(realloc(*new_data, *new_size));
                new_data_ptr = *new_data + sampleBytesWritten;
            }

            //put start code
            *new_data_ptr = 0;
            new_data_ptr++;
            *new_data_ptr = 0;
            new_data_ptr++;
            *new_data_ptr = 0;
            new_data_ptr++;
            *new_data_ptr = 1;
            new_data_ptr++;
            //nal type
            *new_data_ptr = nalPrefixData[4];
            new_data_ptr++;

            sampleBytesWritten += 5;
            sampleSize += nalUnitLengthFieldLengthDiff;

        } else {
            if (sampleBytesWritten + sampleCurrentNalBytesRemaining > *new_size) {
                *new_size = sampleBytesWritten + sampleCurrentNalBytesRemaining;
                *new_data = static_cast<uint8_t *>(realloc(*new_data, *new_size));

                new_data_ptr = *new_data + sampleBytesWritten;
            }
            memcpy(new_data_ptr, ori_data_ptr, sampleCurrentNalBytesRemaining);
            ori_data_ptr = ori_data_ptr + sampleCurrentNalBytesRemaining;
            new_data_ptr = new_data_ptr + sampleCurrentNalBytesRemaining;

            sampleBytesWritten += sampleCurrentNalBytesRemaining;
            sampleCurrentNalBytesRemaining -= sampleCurrentNalBytesRemaining;
        }
    }

    assert(*new_size == sampleBytesWritten);

    *new_size = sampleBytesWritten;
}



