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

        static void changeState(JNIEnv *env, jobject instance, jlong nativeIntance, jbyteArray data,
                                jint state, jint errorCode);

    public:

        AndroidDrmSessionManager();

        ~AndroidDrmSessionManager();

        int
        requireDrmSession(void **dstSessionId, int *dstSessionSize, const std::string &keyUrl,
                          const std::string &keyFormat, const std::string &mime,
                          const std::string &licenseUrl) override;

        void releaseDrmSession(void *sessionId, int dstSessionSize) override;

        virtual void getSessionState(void* sessionId , int sessionSize , int* state, int* code)  override;

    public:
        class SessionState {
        public:
            SessionState(void *sessionId, int sessionSize, int state , int errorCode ) {
                mId = malloc(sessionSize);
                memcpy(mId, sessionId, sessionSize);
                mSize = sessionSize;
                mState = state;
                mErrorCode = errorCode;
            }

            ~SessionState() {
                if (mId != nullptr) {
                    free(mId);
                }
            }

        public:
            void *mId = nullptr;
            int mSize;
            int mState;
            int mErrorCode;
        };

    private:

        void changeStateInner(char *sessionId, int size, int state , int errorCode);

    private:
        jobject mDrmSessionManger = nullptr;

        std::mutex mStateMutex{};
        std::list<SessionState> mSessionStates{};

    };

}

#endif //SOURCE_ANDROIDDRMSESSIONMANAGER_H
