//
// Created by lifujun on 2020/4/29.
//

#ifndef SOURCE_OUTPUTBUFFERINFO_H
#define SOURCE_OUTPUTBUFFERINFO_H


#include <cstdint>
#include <jni.h>
#include <string>

class OutputBufferInfo {

public:
    static void init(JNIEnv* env);
    static void unInit(JNIEnv* env);

    static int convertTo(jobject jInfo, OutputBufferInfo* info);

    std::string toString();
public:

    int type;
    int index;
    int64_t pts;
    int flags;
    int bufferSize;
    int bufferOffset;

    int videoWidth;
    int videoHeight;
    int videoStride;
    int videoSliceHeight;
    int videoPixelFormat;
    int videoCropLeft;
    int videoCropRight;
    int videoCropTop;
    int videoCropBottom;

    int audioChannelCount;
    int audioChannelMask;
    int audioSampleRate;


};


#endif //SOURCE_OUTPUTBUFFERINFO_H
