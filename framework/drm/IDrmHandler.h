//
// Created by SuperMan on 11/27/20.
//

#ifndef SOURCE_IDRMHANDLER_H
#define SOURCE_IDRMHANDLER_H

#include <cstdint>
#include <string>
#include <functional>
#include <map>
#include "DrmInfo.h"

typedef std::function<std::map<std::string, std::string>(
        const std::map<std::string, std::string>&)> DrmCallback;

namespace Cicada {
    class IDrmHandler {
    public:

        IDrmHandler(const DrmInfo &drmInfo);

        void setDrmCallback(const DrmCallback &callback) {
            drmCallback = callback;
        }

        virtual void
        convertData(int naluLengthSize, uint8_t **new_data, int *new_size, const uint8_t *data,
                    int size) {

        };

        virtual int initDecoder(void *pDecoder) {
            return 0;
        };

    protected:
        DrmInfo drmInfo;

        DrmCallback drmCallback{nullptr};

    };
}


#endif //SOURCE_IDRMHANDLER_H
