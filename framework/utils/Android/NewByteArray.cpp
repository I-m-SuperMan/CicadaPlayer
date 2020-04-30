//
// Created by lifujun on 2020/4/26.
//

#include <utils/frame_work_log.h>
#include "NewByteArray.h"
#include "JniException.h"


NewByteArray::NewByteArray(JNIEnv *pEnv, const char *source, int len)
{
    if (source == nullptr || pEnv == nullptr) {
        mResult = nullptr;
        mEnv = nullptr;
    } else {
        mEnv = pEnv;
        mResult = pEnv->NewByteArray(len);
        pEnv->SetByteArrayRegion(mResult, 0, len, (jbyte *) (source));
        JniException::clearException(mEnv);
    }
}

NewByteArray::~NewByteArray()
{
    if (mResult != nullptr) {
        mEnv->DeleteLocalRef(mResult);
        JniException::clearException(mEnv);
    }

    mResult = nullptr;
}

jbyteArray NewByteArray::getArray()
{
    return mResult;
}