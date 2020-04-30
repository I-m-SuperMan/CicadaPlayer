//
// Created by lifujun on 2020/4/29.
//

#include <utils/Android/FindClass.h>
#include <utils/Android/JniEnv.h>
#include "OutputBufferInfo.h"

static const char *OutputBufferInfoPath = "com/cicada/player/media/OutputBufferInfo";

static jclass gj_OutputBufferInfo_class = nullptr;
static jfieldID gj_OutputBufferInfo_type = nullptr;
static jfieldID gj_OutputBufferInfo_index = nullptr;
static jfieldID gj_OutputBufferInfo_pts = nullptr;
static jfieldID gj_OutputBufferInfo_flags = nullptr;
static jfieldID gj_OutputBufferInfo_bufferSize = nullptr;
static jfieldID gj_OutputBufferInfo_bufferOffset = nullptr;

static jfieldID gj_OutputBufferInfo_videoWidth = nullptr;
static jfieldID gj_OutputBufferInfo_videoHeight = nullptr;
static jfieldID gj_OutputBufferInfo_videoStride = nullptr;
static jfieldID gj_OutputBufferInfo_videoSliceHeight = nullptr;
static jfieldID gj_OutputBufferInfo_videoPixelFormat = nullptr;
static jfieldID gj_OutputBufferInfo_videoCropLeft = nullptr;
static jfieldID gj_OutputBufferInfo_videoCropRight = nullptr;
static jfieldID gj_OutputBufferInfo_videoCropTop = nullptr;
static jfieldID gj_OutputBufferInfo_videoCropBottom = nullptr;

static jfieldID gj_OutputBufferInfo_audioChannelCount = nullptr;
static jfieldID gj_OutputBufferInfo_audioChannelMask = nullptr;
static jfieldID gj_OutputBufferInfo_audioSampleRate = nullptr;

void OutputBufferInfo::init(JNIEnv *env)
{
    if (gj_OutputBufferInfo_class == nullptr) {
        FindClass clazz(env, OutputBufferInfoPath);
        gj_OutputBufferInfo_class = static_cast<jclass>(env->NewGlobalRef(clazz.getClass()));
        gj_OutputBufferInfo_type = env->GetFieldID(gj_OutputBufferInfo_class, "type", "I");
        gj_OutputBufferInfo_index = env->GetFieldID(gj_OutputBufferInfo_class, "index", "I");
        gj_OutputBufferInfo_pts = env->GetFieldID(gj_OutputBufferInfo_class, "pts", "J");
        gj_OutputBufferInfo_flags = env->GetFieldID(gj_OutputBufferInfo_class, "flags", "I");
        gj_OutputBufferInfo_bufferSize = env->GetFieldID(gj_OutputBufferInfo_class, "bufferSize",
                                         "I");
        gj_OutputBufferInfo_bufferOffset = env->GetFieldID(gj_OutputBufferInfo_class,
                                           "bufferOffset", "I");
        gj_OutputBufferInfo_videoWidth = env->GetFieldID(gj_OutputBufferInfo_class, "videoWidth",
                                         "I");
        gj_OutputBufferInfo_videoHeight = env->GetFieldID(gj_OutputBufferInfo_class, "videoHeight",
                                          "I");
        gj_OutputBufferInfo_videoStride = env->GetFieldID(gj_OutputBufferInfo_class, "videoStride",
                                          "I");
        gj_OutputBufferInfo_videoSliceHeight = env->GetFieldID(gj_OutputBufferInfo_class,
                                               "videoSliceHeight", "I");
        gj_OutputBufferInfo_videoPixelFormat = env->GetFieldID(gj_OutputBufferInfo_class,
                                               "videoPixelFormat", "I");
        gj_OutputBufferInfo_videoCropLeft = env->GetFieldID(gj_OutputBufferInfo_class,
                                            "videoCropLeft", "I");
        gj_OutputBufferInfo_videoCropRight = env->GetFieldID(gj_OutputBufferInfo_class,
                                             "videoCropRight", "I");
        gj_OutputBufferInfo_videoCropTop = env->GetFieldID(gj_OutputBufferInfo_class,
                                           "videoCropTop", "I");
        gj_OutputBufferInfo_videoCropBottom = env->GetFieldID(gj_OutputBufferInfo_class,
                                              "videoCropBottom", "I");
        gj_OutputBufferInfo_audioChannelCount = env->GetFieldID(gj_OutputBufferInfo_class,
                                                "audioChannelCount", "I");
        gj_OutputBufferInfo_audioChannelMask = env->GetFieldID(gj_OutputBufferInfo_class,
                                               "audioChannelMask", "I");
        gj_OutputBufferInfo_audioSampleRate = env->GetFieldID(gj_OutputBufferInfo_class,
                                              "audioSampleRate", "I");
    }
}

void OutputBufferInfo::unInit(JNIEnv *env)
{
    if (gj_OutputBufferInfo_class != nullptr) {
        env->DeleteGlobalRef(gj_OutputBufferInfo_class);
    }
}


int OutputBufferInfo::convertTo(jobject jInfo, OutputBufferInfo *info)
{
    JniEnv jniEnv{};
    JNIEnv *env = jniEnv.getEnv();
    info->type = env->GetIntField(jInfo, gj_OutputBufferInfo_type);
    info->index = env->GetIntField(jInfo, gj_OutputBufferInfo_index);
    info->pts = env->GetLongField(jInfo, gj_OutputBufferInfo_pts);
    info->flags = env->GetIntField(jInfo, gj_OutputBufferInfo_flags);
    info->bufferSize = env->GetIntField(jInfo, gj_OutputBufferInfo_bufferSize);
    info->bufferOffset = env->GetIntField(jInfo, gj_OutputBufferInfo_bufferOffset);
    info->videoWidth = env->GetIntField(jInfo, gj_OutputBufferInfo_videoWidth);
    info->videoHeight = env->GetIntField(jInfo, gj_OutputBufferInfo_videoHeight);
    info->videoStride = env->GetIntField(jInfo, gj_OutputBufferInfo_videoStride);
    info->videoSliceHeight = env->GetIntField(jInfo, gj_OutputBufferInfo_videoSliceHeight);
    info->videoPixelFormat = env->GetIntField(jInfo, gj_OutputBufferInfo_videoPixelFormat);
    info->videoCropLeft = env->GetIntField(jInfo, gj_OutputBufferInfo_videoCropLeft);
    info->videoCropRight = env->GetIntField(jInfo, gj_OutputBufferInfo_videoCropRight);
    info->videoCropTop = env->GetIntField(jInfo, gj_OutputBufferInfo_videoCropTop);
    info->videoCropBottom = env->GetIntField(jInfo, gj_OutputBufferInfo_videoCropBottom);
    info->audioChannelCount = env->GetIntField(jInfo, gj_OutputBufferInfo_audioChannelCount);
    info->audioChannelMask = env->GetIntField(jInfo, gj_OutputBufferInfo_audioChannelMask);
    info->audioSampleRate = env->GetIntField(jInfo, gj_OutputBufferInfo_audioSampleRate);
    return 0;
}

std::string OutputBufferInfo::toString()
{
    return std::string();
}

