//
// Created by SuperMan on 2020/10/22.
//

#ifndef SOURCE_IDRMSESSIONMANAGER_H
#define SOURCE_IDRMSESSIONMANAGER_H

#include <string>
#include <utils/AFMediaType.h>

#define SESSION_STATE_ERROR (-1)
#define SESSION_STATE_IDLE (-2)
#define SESSION_STATE_OPENED (0)

namespace Cicada {
    class IDrmSessionManager {

    public:

        static IDrmSessionManager *create();

    public:
        IDrmSessionManager() = default;

        ~IDrmSessionManager() = default;

        void setDrmRequestCallback(drmRequestCb provisionCb ,drmRequestCb keyCb , void* userData ){
            mRequestProvisionCallback = provisionCb;
            mRequestLicenseCallback = keyCb;
            mRequestUserData = userData;
        }

        virtual int
        requireDrmSession(const std::string &keyUrl,
                          const std::string &keyFormat, const std::string &mime,
                          const std::string &licenseUrl) = 0;

        virtual void releaseDrmSession() = 0;

        virtual void getSessionState( int* state, int* code) = 0;

        virtual void* getSession(int *sessionSize) = 0;

        virtual bool isForceInsecureDecoder() {
            return false;
        }

    protected:

        void * mRequestUserData = nullptr;

        drmRequestCb mRequestProvisionCallback = nullptr;

        drmRequestCb mRequestLicenseCallback = nullptr;
    };
}


#endif //SOURCE_IDRMSESSIONMANAGER_H
