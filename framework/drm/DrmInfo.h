//
// Created by SuperMan on 11/27/20.
//

#ifndef SOURCE_DRMINFO_H
#define SOURCE_DRMINFO_H

#include <string>

namespace Cicada {
    class DrmInfo {
    public:
        std::string uri;
        std::string format;

        bool operator==(const DrmInfo &drmInfo) {
            return uri == drmInfo.uri &&
                   format == drmInfo.format;
        }

        bool empty() {
            return uri.empty() &&
                   format.empty();
        }
    };
}


#endif //SOURCE_DRMINFO_H
