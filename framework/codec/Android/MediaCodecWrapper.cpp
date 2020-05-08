//
// Created by lifujun on 2020/4/29.
//
#define LOG_TAG "MediaCodecWrapper"

#include <utils/Android/JniEnv.h>
#include <utils/Android/FindClass.h>
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/NewLinkedList.h>
#include <utils/Android/NewByteArray.h>
#include <utils/frame_work_log.h>
#include "MediaCodecWrapper.h"
#include "OutputBufferInfo.h"
#include "MediaCodecCryptoInfo.h"

using namespace Cicada;

static const char *MCWrapperPath = "com/cicada/player/media/MediaCodecWrapper";

static jclass gj_MCWrapper_class = nullptr;
static jmethodID gj_MCWrapper_construct = nullptr;
static jmethodID gj_MCWrapper_init = nullptr;
static jmethodID gj_MCWrapper_setMediaCrypto = nullptr;
static jmethodID gj_MCWrapper_setCodecSpecificData = nullptr;
static jmethodID gj_MCWrapper_configureVideo = nullptr;
static jmethodID gj_MCWrapper_configureAudio = nullptr;
static jmethodID gj_MCWrapper_start = nullptr;
static jmethodID gj_MCWrapper_stop = nullptr;
static jmethodID gj_MCWrapper_flush = nullptr;
static jmethodID gj_MCWrapper_dequeueInputBuffer = nullptr;
static jmethodID gj_MCWrapper_dequeueOutputBuffer = nullptr;
static jmethodID gj_MCWrapper_getInputBuffer = nullptr;
static jmethodID gj_MCWrapper_getOutputBuffer = nullptr;
static jmethodID gj_MCWrapper_queueInputBuffer = nullptr;
static jmethodID gj_MCWrapper_queueSecureInputBuffer = nullptr;
static jmethodID gj_MCWrapper_getOutputBufferInfo = nullptr;
static jmethodID gj_MCWrapper_releaseOutputBuffer = nullptr;


void MediaCodecWrapper::init(JNIEnv *pEnv)
{
    if (gj_MCWrapper_class == nullptr) {
        FindClass clazz(pEnv, MCWrapperPath);
        gj_MCWrapper_class = static_cast<jclass>(pEnv->NewGlobalRef(clazz.getClass()));
        gj_MCWrapper_construct = pEnv->GetMethodID(gj_MCWrapper_class, "<init>", "()V");
        gj_MCWrapper_init = pEnv->GetMethodID(gj_MCWrapper_class, "init",
                                              "(Ljava/lang/String;ILandroid/view/Surface;)I");
        gj_MCWrapper_setMediaCrypto = pEnv->GetMethodID(gj_MCWrapper_class, "setMediaCrypto",
                                      "(Landroid/media/MediaCrypto;)V");
        gj_MCWrapper_setCodecSpecificData = pEnv->GetMethodID(gj_MCWrapper_class,
                                            "setCodecSpecificData",
                                            "(Ljava/util/List;)V");
        gj_MCWrapper_configureVideo = pEnv->GetMethodID(gj_MCWrapper_class, "configureVideo",
                                      "(IIII)I");
        gj_MCWrapper_configureAudio = pEnv->GetMethodID(gj_MCWrapper_class, "configureAudio",
                                      "(II)I");
        gj_MCWrapper_start = pEnv->GetMethodID(gj_MCWrapper_class, "start", "()I");
        gj_MCWrapper_stop = pEnv->GetMethodID(gj_MCWrapper_class, "stop", "()I");
        gj_MCWrapper_flush = pEnv->GetMethodID(gj_MCWrapper_class, "flush", "()I");
        gj_MCWrapper_dequeueInputBuffer = pEnv->GetMethodID(gj_MCWrapper_class,
                                          "dequeueInputBuffer", "(J)I");
        gj_MCWrapper_dequeueOutputBuffer = pEnv->GetMethodID(gj_MCWrapper_class,
                                           "dequeueOutputBuffer", "(J)I");
        gj_MCWrapper_getInputBuffer = pEnv->GetMethodID(gj_MCWrapper_class,
                                      "getInputBuffer",
                                      "(I)Ljava/nio/ByteBuffer;");
        gj_MCWrapper_getOutputBuffer = pEnv->GetMethodID(gj_MCWrapper_class,
                                       "getOutputBuffer",
                                       "(I)Ljava/nio/ByteBuffer;");
        gj_MCWrapper_queueInputBuffer = pEnv->GetMethodID(gj_MCWrapper_class, "queueInputBuffer",
                                        "(IIIJI)I");
        gj_MCWrapper_releaseOutputBuffer = pEnv->GetMethodID(gj_MCWrapper_class,
                                           "releaseOutputBuffer", "(IZ)I");
        gj_MCWrapper_getOutputBufferInfo = pEnv->GetMethodID(gj_MCWrapper_class,
                                           "getOutputBufferInfo",
                                           "(I)Lcom/cicada/player/media/OutputBufferInfo;");
        gj_MCWrapper_queueSecureInputBuffer = pEnv->GetMethodID(gj_MCWrapper_class,
                                              "queueSecureInputBuffer",
                                              "(IILcom/cicada/player/media/MediaCodecCryptoInfo;JI)I");
    }
}

void MediaCodecWrapper::unInit(JNIEnv *pEnv)
{
    if (gj_MCWrapper_class != nullptr) {
        pEnv->DeleteGlobalRef(gj_MCWrapper_class);
    }

    gj_MCWrapper_class = nullptr;
}


MediaCodecWrapper::MediaCodecWrapper()
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    jobject obj = pEnv->NewObject(gj_MCWrapper_class, gj_MCWrapper_construct);
    mCodecWrapper = pEnv->NewGlobalRef(obj);
    pEnv->DeleteLocalRef(obj);
}


MediaCodecWrapper::~MediaCodecWrapper()
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    pEnv->DeleteGlobalRef(mCodecWrapper);
}

int MediaCodecWrapper::init(std::string mimeType, int category, void *surface)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    NewStringUTF mime(pEnv, mimeType.c_str());
    int ret = pEnv->CallIntMethod(mCodecWrapper, gj_MCWrapper_init, mime.getString(),
                                  (jint) category,
                                  (jobject) surface);
    AF_LOGD("init() mimeTyp(%s),category(%d),surface(%p) , ret = %d", mimeType.c_str(), category,
            surface, ret);
    return ret;
}

void MediaCodecWrapper::setCodecSpecificData(std::vector<char *> buffers, std::vector<int> lengths)
{
    int size = static_cast<int>(buffers.size());

    if (size > 0) {
        JniEnv jniEnv{};
        JNIEnv *pEnv = jniEnv.getEnv();
        NewLinkedList csdList(pEnv);

        for (int i = 0; i < size; i++) {
            NewByteArray bufferArray(pEnv, buffers[i], lengths[i]);
            csdList.add(bufferArray.getArray());
        }

        pEnv->CallVoidMethod(mCodecWrapper, gj_MCWrapper_setCodecSpecificData, csdList.getList());
    }
}

int MediaCodecWrapper::configureVideo(int h264Profile, int width, int height, int angle)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    int ret = pEnv->CallIntMethod(mCodecWrapper, gj_MCWrapper_configureVideo, h264Profile, width,
                                  height, angle);
    AF_LOGD("configureVideo() h264Profile(%d),width(%d),height(%d),angle(%d) , ret = %d",
            h264Profile, width, height, angle, ret);
    return ret;
}

int MediaCodecWrapper::configureAudio(int sampleRate, int channelCount)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    int ret = pEnv->CallIntMethod(mCodecWrapper, gj_MCWrapper_configureAudio, sampleRate,
                                  channelCount);
    AF_LOGD("configureAudio() sampleRate(%d),channelCount(%d) , ret = %d", sampleRate, channelCount,
            ret);
    return ret;
}

int MediaCodecWrapper::start()
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    int ret = pEnv->CallIntMethod(mCodecWrapper, gj_MCWrapper_start);
    AF_LOGD("start() ,ret = %d", ret);
    return ret;
}

int MediaCodecWrapper::stop()
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    int ret = pEnv->CallIntMethod(mCodecWrapper, gj_MCWrapper_stop);
    AF_LOGD("stop() ,ret = %d", ret);
    return ret;
}

int MediaCodecWrapper::flush()
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    int ret = pEnv->CallIntMethod(mCodecWrapper, gj_MCWrapper_flush);
    AF_LOGD("flush() ,ret = %d", ret);
    return ret;
}

int MediaCodecWrapper::dequeueInputBuffer(int64_t timeoutUs)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    int ret = pEnv->CallIntMethod(mCodecWrapper, gj_MCWrapper_dequeueInputBuffer,
                                  (jlong) timeoutUs);
    AF_LOGD("dequeueInputBuffer() timeoutUs(%lld) ,ret = %d", timeoutUs, ret);
    return ret;
}

int MediaCodecWrapper::dequeueOutputBuffer(int64_t timeoutUs)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    int ret = pEnv->CallIntMethod(mCodecWrapper, gj_MCWrapper_dequeueOutputBuffer,
                                  (jlong) timeoutUs);
    AF_LOGD("dequeueOutputBuffer() timeoutUs(%lld) ,ret = %d", timeoutUs, ret);
    return ret;
}

int MediaCodecWrapper::queueInputBuffer(int index, int offset, int size, int64_t presentationUs,
                                        int flags)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    int ret = pEnv->CallIntMethod(mCodecWrapper, gj_MCWrapper_queueInputBuffer, (jint) index,
                                  (jint) offset, (jint) size, (jlong) presentationUs, (jint) flags);
    AF_LOGD("queueInputBuffer() index(%d),offset(%d),size(%d),presentationUs(%lld),flags(%d) ,ret = %d",
            index, offset, size, presentationUs, flags, ret);
    return ret;
}

int MediaCodecWrapper::queueSecureInputBuffer(int index, int offset,
        std::unique_ptr<EncryptionInfo> encryptionInfo,
        int64_t presentationUs, int flags)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    jobject codecEncryptionInfo = MediaCodecCryptoInfo::convert(pEnv, move(encryptionInfo));
    int ret = pEnv->CallIntMethod(mCodecWrapper, gj_MCWrapper_queueSecureInputBuffer, (jint) index,
                                  (jint) offset, codecEncryptionInfo, (jlong) presentationUs,
                                  (jint) flags);
    pEnv->DeleteLocalRef(codecEncryptionInfo);
    AF_LOGD("queueSecureInputBuffer() index(%d),presentationUs(%lld),ret = %d", index, presentationUs, ret);
    return ret;
}

int MediaCodecWrapper::releaseOutputBuffer(int index, bool render)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    int ret = pEnv->CallIntMethod(mCodecWrapper, gj_MCWrapper_releaseOutputBuffer, (jint) index,
                                  (jboolean) render);
    AF_LOGD("releaseOutputBuffer() index(%d),render(%d),ret = %d", index, render, ret);
    return ret;
}

jobject MediaCodecWrapper::getInputBuffer(int index)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    jobject pJobject = pEnv->CallObjectMethod(mCodecWrapper, gj_MCWrapper_getInputBuffer,
                       (jint) index);
    AF_LOGD("getInputBuffer() index(%d), ret = %p", index, pJobject);
    return pJobject;
}

jobject MediaCodecWrapper::getOutputBuffer(int index)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    jobject pJobject = pEnv->CallObjectMethod(mCodecWrapper, gj_MCWrapper_getOutputBuffer, (jint) index);
    AF_LOGD("getOutputBuffer() index(%d), ret = %p", index, pJobject);
    return pJobject;
}

int MediaCodecWrapper::getOutputBufferInfo(int index, OutputBufferInfo *info)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    jobject jInfo = pEnv->CallObjectMethod(mCodecWrapper, gj_MCWrapper_getOutputBufferInfo,
                                           (jint) index);
    int ret = -1;

    if (jInfo != nullptr) {
        ret = OutputBufferInfo::convertTo(jInfo, info);
        pEnv->DeleteLocalRef(jInfo);
    }

    AF_LOGD("getOutputBufferInfo() index(%d), ret = %d", index, ret);
    return ret;
}

void MediaCodecWrapper::setMediaCrypto(jobject crypto)
{
    JniEnv jniEnv{};
    JNIEnv *pEnv = jniEnv.getEnv();
    pEnv->CallVoidMethod(mCodecWrapper, gj_MCWrapper_setMediaCrypto, crypto);
}
