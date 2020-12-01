//
// Created by SuperMan on 11/27/20.
//

#ifndef SOURCE_DRMMANAGER_H
#define SOURCE_DRMMANAGER_H

#include <string>
#include <vector>
#include <mutex>
#include "IDrmHandler.h"
#include "DrmInfo.h"

namespace Cicada {

    class DrmManager {
    public:

        DrmManager();

        ~DrmManager();

        void setDrmCallback(const DrmCallback &callback) {
            mDrmCallback = callback;
        }

        IDrmHandler *require(const DrmInfo &drmInfo);

    private:
        std::mutex mDrmMutex{};
        std::map<DrmInfo, std::unique_ptr<IDrmHandler>> mDrmMap{};
        DrmCallback mDrmCallback{nullptr};
    };

}
#endif //SOURCE_DRMMANAGER_H
