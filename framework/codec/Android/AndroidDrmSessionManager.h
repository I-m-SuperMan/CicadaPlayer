//
// Created by SuperMan on 2020/10/22.
//

#ifndef SOURCE_ANDROIDDRMSESSIONMANAGER_H
#define SOURCE_ANDROIDDRMSESSIONMANAGER_H

#include <codec/IDrmSessionManager.h>
#include <jni.h>
#include <list>
#include <mutex>

namespace Cicada {
    class AndroidDrmSessionManager : public IDrmSessionManager {

    public:
        static void init(JNIEnv *env);

        static void unInit(JNIEnv *env);

        static int registerMethod(JNIEnv *env);

        static jbyteArray
        requestProvision(JNIEnv *env, jobject instance, jlong nativeIntance, jstring url,
                         jbyteArray data);

        static jbyteArray
        requestKey(JNIEnv *env, jobject instance, jlong nativeIntance, jstring url,
                   jbyteArray data);

        static void changeState(JNIEnv *env, jobject instance, jlong nativeIntance,
                                jint state, jint errorCode);

    public:

        AndroidDrmSessionManager();

        ~AndroidDrmSessionManager();

        int
        requireDrmSession(const std::string &keyUrl,
                          const std::string &keyFormat, const std::string &mime,
                          const std::string &licenseUrl) override;

        void releaseDrmSession() override;

         void getSessionState(  int* state, int* code)  override;

         void* getSession(int *sessionSize) override ;

    private:

        void changeStateInner( int state , int errorCode);

    private:
        jobject mJDrmSessionManger = nullptr;

        std::mutex mStateMutex{};

        void *mSessionId = nullptr;
        int mSize;
        int mState;
        int mErrorCode;

    };

}

#endif //SOURCE_ANDROIDDRMSESSIONMANAGER_H
