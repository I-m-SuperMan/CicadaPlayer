//
// Created by SuperMan on 11/27/20.
//

#ifndef SOURCE_IDRMHANDLER_H
#define SOURCE_IDRMHANDLER_H

#include <cstdint>
#include <string>
#include <functional>
#include <map>

typedef std::function<std::map<std::string, std::string>(
        std::map<std::string, std::string>)> DrmCallback;

namespace Cicada {
    class IDrmHandler {
    public:
        enum class State {
            Error = -1,
            Idle = 0,
            Ready = 1
        };

        IDrmHandler(const std::string &uri, const std::string &format);

        void setDrmCallback(const DrmCallback &callback) {
            drmCallback = callback;
        }

        virtual State getState() = 0;

        virtual int getResult(char **dst, int64_t *dstSize) = 0;

        virtual void open() = 0;

    protected:
        std::string uri;
        std::string format;

        DrmCallback drmCallback{nullptr};

    };
}


#endif //SOURCE_IDRMHANDLER_H
