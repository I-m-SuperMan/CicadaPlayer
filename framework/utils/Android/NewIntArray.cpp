//
// Created by lifujun on 2020/4/26.
//

#include "NewIntArray.h"
#include "JniException.h"

NewIntArray::NewIntArray(JNIEnv *pEnv, const int *source, int len)
{
    if (source == nullptr || pEnv == nullptr) {
        mResult = nullptr;
        mEnv = nullptr;
    } else {
        mEnv = pEnv;
        mResult = pEnv->NewIntArray(len);
        pEnv->SetIntArrayRegion(mResult, 0, len, (jint *) (source));
        JniException::clearException(mEnv);
    }
}

NewIntArray::~NewIntArray()
{
    if (mResult != nullptr) {
        mEnv->DeleteLocalRef(mResult);
        JniException::clearException(mEnv);
    }

    mResult = nullptr;
}

jintArray NewIntArray::getArray()
{
    return mResult;
}
