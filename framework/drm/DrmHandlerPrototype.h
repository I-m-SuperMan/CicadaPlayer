
#ifndef CICADA_PLAYER_DRMHANDLERPROTOTYPE_H
#define CICADA_PLAYER_DRMHANDLERPROTOTYPE_H

#include <memory>

#include <utils/CicadaType.h>
#include "IDrmHandler.h"

namespace Cicada {

    class CICADA_CPLUS_EXTERN DrmHandlerPrototype {
        static DrmHandlerPrototype *drmHandlerQueue[10];
        static int _nextSlot;
    public:
        virtual ~DrmHandlerPrototype() = default;

        virtual IDrmHandler *clone(const std::string &uri, const std::string &format) = 0;

        virtual bool is_supported(const std::string &format) = 0;

        static void addPrototype(DrmHandlerPrototype *se);

        static Cicada::IDrmHandler *create(const std::string &uri, const std::string &format);
    };
}


#endif //CICADA_PLAYER_DRMHANDLERPROTOTYPE_H
