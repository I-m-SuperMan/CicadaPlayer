
#ifndef CICADA_PLAYER_DRMHANDLERPROTOTYPE_H
#define CICADA_PLAYER_DRMHANDLERPROTOTYPE_H

#include <memory>

#include <utils/CicadaType.h>
#include "IDrmHandler.h"
#include "DrmInfo.h"

namespace Cicada {

    class CICADA_CPLUS_EXTERN DrmHandlerPrototype {
        static DrmHandlerPrototype *drmHandlerQueue[10];
        static int _nextSlot;
    public:
        virtual ~DrmHandlerPrototype() = default;

        virtual IDrmHandler *clone(const DrmInfo &drmInfo) = 0;

        virtual bool is_supported(const DrmInfo &drmInfo) = 0;

        static void addPrototype(DrmHandlerPrototype *se);

        static Cicada::IDrmHandler *create(const DrmInfo &drmInfo);
    };
}


#endif //CICADA_PLAYER_DRMHANDLERPROTOTYPE_H
