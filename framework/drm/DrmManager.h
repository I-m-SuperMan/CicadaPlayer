//
// Created by SuperMan on 11/27/20.
//

#ifndef SOURCE_DRMMANAGER_H
#define SOURCE_DRMMANAGER_H

#include <string>
#include <vector>
#include <mutex>
#include "IDrmHandler.h"

namespace Cicada {
    class DrmContext {

    public:
        DrmContext(const std::string &uri, const std::string &format);

        ~DrmContext();

        std::string getUri();

        std::string getFormat();

        IDrmHandler *getHandler();

    private:
        std::string uri;
        std::string format;
        std::unique_ptr<IDrmHandler> drmHandler{nullptr};
    };


    class DrmManager {
    public:

        DrmManager();

        ~DrmManager();

        void setDrmCallback(const DrmCallback &callback) {
            mDrmCallback = callback;
        }

        IDrmHandler *require(const std::string &uri, const std::string &format);

    private:
        std::mutex mDrmMutex{};
        std::vector<std::unique_ptr<DrmContext>> mDrmArray{};
        DrmCallback mDrmCallback{nullptr};
    };

}
#endif //SOURCE_DRMMANAGER_H
