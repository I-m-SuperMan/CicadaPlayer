//
// Created by lifujun on 2020/4/26.
//

#ifndef SOURCE_NEWBYTEARRAY_H
#define SOURCE_NEWBYTEARRAY_H


#include <jni.h>

class NewByteArray {
public:
    NewByteArray(JNIEnv *pEnv, const char*source , int len);

    ~NewByteArray();

public:
    jbyteArray getArray();

private:
    JNIEnv *mEnv;
    jbyteArray mResult;


private:
    NewByteArray(NewByteArray &)
    {

    }

    const NewByteArray &operator=(const NewByteArray &);

};


#endif //SOURCE_NEWBYTEARRAY_H
