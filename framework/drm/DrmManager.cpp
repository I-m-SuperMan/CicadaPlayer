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
    mDrmArray.clear();
}

IDrmHandler *DrmManager::require(const std::string &uri, const std::string &format) {

    std::unique_lock<std::mutex> drmLock(mDrmMutex);

    if (!mDrmArray.empty()) {
        for (auto &item : mDrmArray) {
            DrmContext *drmContext = item.get();
            if (drmContext->getUri() == uri && drmContext->getFormat() == format) {
                return drmContext->getHandler();
            }
        }
    }

    auto *newDrmContext = new DrmContext(uri, format);
    IDrmHandler *pDrmHandler = newDrmContext->getHandler();

    assert(pDrmHandler != nullptr);

    if (pDrmHandler == nullptr) {
        return nullptr;
    }

    pDrmHandler->setDrmCallback(mDrmCallback);
    mDrmArray.push_back(std::unique_ptr<DrmContext>(newDrmContext));

    return pDrmHandler;
}


DrmContext::DrmContext(const std::string &uri, const std::string &format) {
    this->uri = uri;
    this->format = format;

    IDrmHandler *drm = DrmHandlerPrototype::create(uri, format);
    drmHandler = std::unique_ptr<IDrmHandler>(drm);
    drm->open();
}

DrmContext::~DrmContext() {

}

std::string DrmContext::getUri() {
    return uri;
}

std::string DrmContext::getFormat() {
    return format;
}

IDrmHandler *DrmContext::getHandler() {
    return drmHandler.get();
}
