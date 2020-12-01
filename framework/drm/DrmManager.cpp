//
// Created by SuperMan on 11/27/20.
//

#include <cassert>
#include "DrmManager.h"
#include "DrmHandlerPrototype.h"

using namespace Cicada;

DrmManager::DrmManager() {

}

DrmManager::~DrmManager() {
    mDrmMap.clear();
}

IDrmHandler *DrmManager::require(const DrmInfo &drmInfo) {

    std::unique_lock<std::mutex> drmLock(mDrmMutex);

    if (!mDrmMap.empty()) {
        for (auto &item : mDrmMap) {
            auto &drmItem = (DrmInfo &) item.first;
            if (drmItem == drmInfo) {
                return item.second.get();
            }
        }
    }

    IDrmHandler *pDrmHandler = DrmHandlerPrototype::create(drmInfo);

    assert(pDrmHandler != nullptr);

    if (pDrmHandler == nullptr) {
        return nullptr;
    }

    pDrmHandler->setDrmCallback(mDrmCallback);
    mDrmMap[drmInfo] = std::unique_ptr<IDrmHandler>(pDrmHandler);

    return pDrmHandler;
}
