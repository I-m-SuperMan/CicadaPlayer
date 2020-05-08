//
// Created by lifujun on 2020/5/8.
//

#include <utils/Android/FindClass.h>
#include <utils/Android/NewByteArray.h>
#include <utils/Android/NewIntArray.h>
#include <utils/Android/NewStringUTF.h>
#include "MediaCodecCryptoInfo.h"

static const char *MediaCodecCryptoInfoPath = "com/cicada/player/media/MediaCodecCryptoInfo";

jclass gj_MediaCodecCryptoInfo_class = nullptr;
jmethodID gj_MediaCodecCryptoInfo_Construct = nullptr;
jmethodID gj_MediaCodecCryptoInfo_SetPattern = nullptr;
jmethodID gj_MediaCodecCryptoInfo_SetMode = nullptr;
jmethodID gj_MediaCodecCryptoInfo_SetSubSamplesInfo = nullptr;
jmethodID gj_MediaCodecCryptoInfo_SetKeyAndIv = nullptr;

void MediaCodecCryptoInfo::init(JNIEnv *env) {
    if (gj_MediaCodecCryptoInfo_class == nullptr) {
        FindClass cls(env, MediaCodecCryptoInfoPath);
        gj_MediaCodecCryptoInfo_class = (jclass) env->NewGlobalRef(cls.getClass());
        gj_MediaCodecCryptoInfo_Construct = env->GetMethodID(gj_MediaCodecCryptoInfo_class,
                                                             "<init>",
                                                             "()V");
        gj_MediaCodecCryptoInfo_SetPattern = env->GetMethodID(gj_MediaCodecCryptoInfo_class,
                                                              "setPattern",
                                                              "(II)V");
        gj_MediaCodecCryptoInfo_SetMode = env->GetMethodID(gj_MediaCodecCryptoInfo_class,
                                                           "setMode",
                                                           "(Ljava/lang/String;)V");
        gj_MediaCodecCryptoInfo_SetSubSamplesInfo = env->GetMethodID(
                gj_MediaCodecCryptoInfo_class,
                "setSubSamplesInfo",
                "(I[I[I)V");
        gj_MediaCodecCryptoInfo_SetKeyAndIv = env->GetMethodID(gj_MediaCodecCryptoInfo_class,
                                                               "setKeyAndIv",
                                                               "([B[B)V");
    }
}

void MediaCodecCryptoInfo::unInit(JNIEnv *pEnv) {
    if (gj_MediaCodecCryptoInfo_class != nullptr) {
        pEnv->DeleteGlobalRef(gj_MediaCodecCryptoInfo_class);
        gj_MediaCodecCryptoInfo_class = nullptr;
    }
}

jobject
MediaCodecCryptoInfo::convert(JNIEnv *mEnv, std::unique_ptr<EncryptionInfo> encryptionInfo) {
    if (encryptionInfo == nullptr) {
        return nullptr;
    }

    jobject javaInfo = mEnv->NewObject(gj_MediaCodecCryptoInfo_class,
                                       gj_MediaCodecCryptoInfo_Construct);
    auto pEncryptionInfo = encryptionInfo.get();

    mEnv->CallVoidMethod(javaInfo, gj_MediaCodecCryptoInfo_SetPattern,
                         pEncryptionInfo->crypt_byte_block, pEncryptionInfo->skip_byte_block);

    NewStringUTF schemeStr(mEnv, pEncryptionInfo->scheme.c_str());
    mEnv->CallVoidMethod(javaInfo, gj_MediaCodecCryptoInfo_SetMode, schemeStr.getString());

    NewByteArray keyArray(mEnv, reinterpret_cast<const char *>(pEncryptionInfo->key_id),
                          pEncryptionInfo->key_id_size);
    NewByteArray ivArray(mEnv, reinterpret_cast<const char *>(pEncryptionInfo->iv),
                         pEncryptionInfo->iv_size);
    mEnv->CallVoidMethod(javaInfo, gj_MediaCodecCryptoInfo_SetKeyAndIv, keyArray.getArray(),
                         ivArray.getArray());

    const int subsmaplesNum = pEncryptionInfo->subsample_count;
    if (subsmaplesNum > 0) {
        int clearData[subsmaplesNum];
        int encryptData[subsmaplesNum];

        clearData[0] = pEncryptionInfo->subsamples->bytes_of_clear_data;
        encryptData[0] = pEncryptionInfo->subsamples->bytes_of_protected_data;

        NewIntArray clearDataNum(mEnv, clearData, subsmaplesNum);
        NewIntArray encryptDataNum(mEnv, encryptData, subsmaplesNum);
        mEnv->CallVoidMethod(javaInfo, gj_MediaCodecCryptoInfo_SetSubSamplesInfo, subsmaplesNum,
                             clearDataNum.getArray(), encryptDataNum.getArray());
    }

    return javaInfo;
}


