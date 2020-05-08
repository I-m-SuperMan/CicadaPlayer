//
// Created by lifujun on 2020/4/29.
//

#ifndef SOURCE_MEIDACODECWRAPPER_H
#define SOURCE_MEIDACODECWRAPPER_H


#include <jni.h>
#include <string>
#include <vector>
#include "base/media/EncryptionInfo.h"
#include "OutputBufferInfo.h"

namespace Cicada {
    class MediaCodecWrapper {

    public:

        static void init(JNIEnv *pEnv);

        static void unInit(JNIEnv *pEnv);

    public:
        MediaCodecWrapper();

        ~MediaCodecWrapper();

        int init(std::string mimeType, int category, void *surface);

        void setCodecSpecificData(std::vector<char *> buffers, std::vector<int> size);

        int configureVideo(int h264Profile, int width, int height, int angle);

        int configureAudio(int sampleRate, int channelCount);

        int start();

        int stop();

        int flush();

        int dequeueInputBuffer(int64_t timeoutUs);

        int dequeueOutputBuffer(int64_t timeoutUs);

        int queueInputBuffer(int index, int offset, int size, int64_t presentationUs, int flags);

        int queueSecureInputBuffer(int index, int offset,
                                   std::unique_ptr<EncryptionInfo> encryptionInfo,
                                   int64_t presentationUs, int flags);

        int releaseOutputBuffer(int index, bool render);


        jobject getInputBuffer(int index);

        jobject getOutputBuffer(int index);

        int getOutputBufferInfo(int index, OutputBufferInfo* info);


    private:
        jobject mCodecWrapper = nullptr;


    };
}


#endif //SOURCE_MEIDACODECWRAPPER_H
