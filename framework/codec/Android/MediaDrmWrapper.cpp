//
// Created by lifujun on 2020/5/8.
//

#include <utils/Android/FindClass.h>
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/NewByteArray.h>
#include <utils/Android/Convertor.h>
#include "MediaDrmWrapper.h"

static const char *MediaDrmWrapperPath = "com/cicada/player/media/MediaDrmWrapper";

jclass gj_MediaDrmWrapper_class = nullptr;
jmethodID gj_MediaDrmWrapper_Construct = nullptr;
jmethodID gj_MediaDrmWrapper_GetMediaCrypto = nullptr;
jmethodID gj_MediaDrmWrapper_Destroy = nullptr;
jmethodID gj_MediaDrmWrapper_SetKeyRequestInfo = nullptr;

void MediaDrmWrapper::init(JNIEnv *pEnv)
{
    if (gj_MediaDrmWrapper_class == nullptr) {
        FindClass clzz(pEnv, MediaDrmWrapperPath);
        gj_MediaDrmWrapper_class = static_cast<jclass>(pEnv->NewGlobalRef(clzz.getClass()));
        gj_MediaDrmWrapper_Construct = pEnv->GetMethodID(gj_MediaDrmWrapper_class, "<init>",
                                       "(Ljava/lang/String;)V");
        gj_MediaDrmWrapper_GetMediaCrypto = pEnv->GetMethodID(gj_MediaDrmWrapper_class,
                                            "getMediaCrypto",
                                            "()Landroid/media/MediaCrypto;");
        gj_MediaDrmWrapper_Destroy = pEnv->GetMethodID(gj_MediaDrmWrapper_class, "destroy", "()V");
        gj_MediaDrmWrapper_SetKeyRequestInfo = pEnv->GetMethodID(gj_MediaDrmWrapper_class,
                                               "setKeyRequestInfo",
                                               "(Ljava/lang/String;[BLjava/lang/String;ILjava/lang/Object;)V");
    }
}

void MediaDrmWrapper::unInit(JNIEnv *pEnv)
{
    if (gj_MediaDrmWrapper_class != nullptr) {
        pEnv->DeleteGlobalRef(gj_MediaDrmWrapper_class);
        gj_MediaDrmWrapper_class = nullptr;
    }
}

MediaDrmWrapper::MediaDrmWrapper(std::string uuid)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    NewStringUTF uuidStr(pEnv, uuid.c_str());
    jobject obj = pEnv->NewObject(gj_MediaDrmWrapper_class, gj_MediaDrmWrapper_Construct,
                                  uuidStr.getString());
    mMediaDrm = pEnv->NewGlobalRef(obj);
    pEnv->DeleteLocalRef(obj);
}

MediaDrmWrapper::~MediaDrmWrapper()
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    pEnv->CallVoidMethod(mMediaDrm, gj_MediaDrmWrapper_Destroy);
    pEnv->DeleteGlobalRef(mMediaDrm);
}

jobject MediaDrmWrapper::getMediaCrypto()
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    jobject crypto = pEnv->CallObjectMethod(mMediaDrm, gj_MediaDrmWrapper_GetMediaCrypto);
    return crypto;
}

void MediaDrmWrapper::setKeyRequestInfo(std::string licenceUrl, char *initData, int len,
                                        std::string mimeType, int keyType,
                                        std::map<std::string, std::string> &params)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    NewStringUTF licence(pEnv, licenceUrl.c_str());
    NewByteArray initDataArray(pEnv, initData, len);
    NewStringUTF mime(pEnv, mimeType.c_str());
    jobject map = Convertor::cmap2Jmap(pEnv, params);
    pEnv->CallVoidMethod(mMediaDrm, gj_MediaDrmWrapper_SetKeyRequestInfo,
                         licence.getString(), initDataArray.getArray(), mime.getString(),
                         (jint) keyType, map);
    pEnv->DeleteLocalRef(map);
}



