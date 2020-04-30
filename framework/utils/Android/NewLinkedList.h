//
// Created by lifujun on 2020/4/29.
//

#ifndef SOURCE_NEWLINKEDLIST_H
#define SOURCE_NEWLINKEDLIST_H

#include <jni.h>
#include <utils/CicadaType.h>

class CICADA_CPLUS_EXTERN NewLinkedList {


public:
    NewLinkedList(JNIEnv *pEnv);

    ~NewLinkedList();

public:
    jobject getList();

    void add(jobject value);

private:
    JNIEnv *mEnv{nullptr};
    jobject mResult{nullptr};

private:
    NewLinkedList(NewLinkedList &) {

    }

    const NewLinkedList &operator=(const NewLinkedList &);

};


#endif //SOURCE_NEWLINKEDLIST_H
