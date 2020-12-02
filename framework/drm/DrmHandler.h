//
// Created by SuperMan on 11/27/20.
//

#ifndef SOURCE_DRMHANDLER_H
#define SOURCE_DRMHANDLER_H

#include <cstdint>
#include <string>
#include <functional>
#include <map>
#include "DrmInfo.h"

namespace Cicada {

    class DrmRequestParam {
    public:
        DrmRequestParam() = default;

        ~DrmRequestParam() = default;

        std::string mDrmType{};
        void *mParam{nullptr};
    };

    class DrmResponseData {
    public:
        DrmResponseData() = default;

        ~DrmResponseData() {
            if (mData != nullptr) {
                free(mData);
            }
        };

        int mSize{0};
        char *mData{nullptr};
    };

    class DrmHandler {

    public:

        DrmHandler(const DrmInfo &drmInfo);

        virtual ~DrmHandler() = default;

        void setDrmCallback(const std::function<DrmResponseData *(
                const DrmRequestParam &drmRequestParam)> &callback) {
            drmCallback = callback;
        }

        virtual bool isErrorState() {
            return false;
        }

    protected:
        DrmInfo drmInfo;

        std::function<DrmResponseData *(const DrmRequestParam &drmRequestParam)> drmCallback{
                nullptr};

    };
}


#endif //SOURCE_DRMHANDLER_H
