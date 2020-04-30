//
// Created by lifujun on 2020/4/26.
//

#ifndef SOURCE_NEWINTARRAY_H
#define SOURCE_NEWINTARRAY_H


#include <utils/CicadaType.h>
#include <jni.h>

class CICADA_CPLUS_EXTERN NewIntArray {
public:
    NewIntArray(JNIEnv *pEnv, const int*source , int len);

    ~NewIntArray();

public:
    jintArray getArray();

private:
    JNIEnv *mEnv;
    jintArray mResult;


private:
    NewIntArray(NewIntArray &)
    {

    }

    const NewIntArray &operator=(const NewIntArray &);

};


#endif //SOURCE_NEWINTARRAY_H
