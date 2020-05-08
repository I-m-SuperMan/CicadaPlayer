//
// Created by lifujun on 2020/5/8.
//

#ifndef SOURCE_MEDIACODECCRYPTOINFO_H
#define SOURCE_MEDIACODECCRYPTOINFO_H


#include <jni.h>
#include <base/media/EncryptionInfo.h>

class MediaCodecCryptoInfo {
public:

    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

public:

    static jobject convert(JNIEnv *mEnv, std::unique_ptr<EncryptionInfo> encryptionInfo);
};


#endif //SOURCE_MEDIACODECCRYPTOINFO_H
