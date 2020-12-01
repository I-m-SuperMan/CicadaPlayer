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

typedef std::function<std::map<std::string, std::string>(
        const std::map<std::string, std::string>&)> DrmCallback;

namespace Cicada {
    class DrmHandler {
    public:

        DrmHandler(const DrmInfo &drmInfo);

        void setDrmCallback(const DrmCallback &callback) {
            drmCallback = callback;
        }

        virtual bool isErrorState() {
            return false;
        }

    protected:
        DrmInfo drmInfo;

        DrmCallback drmCallback{nullptr};

    };
}


#endif //SOURCE_DRMHANDLER_H
