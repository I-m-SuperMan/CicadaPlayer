//
// Created by SuperMan on 11/27/20.
//

#ifndef SOURCE_WIDEVINEDRMHANDLER_H
#define SOURCE_WIDEVINEDRMHANDLER_H

#include <jni.h>
#include <mutex>
#include "IDrmHandler.h"
#include "DrmHandlerPrototype.h"

#define SESSION_STATE_ERROR (-1)
#define SESSION_STATE_IDLE (-2)
#define SESSION_STATE_OPENED (0)

namespace Cicada {
    class WideVineDrmHandler : public IDrmHandler, private DrmHandlerPrototype {
    public:
        WideVineDrmHandler(const std::string &uri, const std::string &format);

        ~WideVineDrmHandler();

        void open() override;

        State getState() override;

        int getResult(char **dst, int64_t *dstSize) override;

        IDrmHandler *clone(const std::string &uri, const std::string &format) override;

        bool is_supported(const std::string &format) override;

        bool isForceInsecureDecoder();

    public:
        static void init(JNIEnv *env);

        static void unInit(JNIEnv *env);

        static int registerMethod(JNIEnv *env);

        static jbyteArray
        requestProvision(JNIEnv *env, jobject instance, jlong nativeInstance, jstring url,
                         jbyteArray data);

        static jbyteArray
        requestKey(JNIEnv *env, jobject instance, jlong nativeInstance, jstring url,
                   jbyteArray data);

        static void changeState(JNIEnv *env, jobject instance, jlong nativeInstance,
                                jint state, jint errorCode);

        static void updateSessionId(JNIEnv *env, jobject instance, jlong nativeInstance,
                                    jbyteArray jSessionId);

    protected:
        explicit WideVineDrmHandler(int dummy)
                : IDrmHandler("", "") {
            addPrototype(this);
        }

        static WideVineDrmHandler dummyWideVineHandler;

    private:
        jobject mJDrmSessionManger = nullptr;

        std::mutex mDrmMutex{};

        void *mSessionId = nullptr;
        int mSize{0};

        State mState{State::Idle};
        int mErrorCode{0};
    };
}


#endif //SOURCE_WIDEVINEDRMHANDLER_H
