
#include "DrmHandlerPrototype.h"

using namespace std;
using namespace Cicada;

DrmHandlerPrototype *DrmHandlerPrototype::drmHandlerQueue[];
int DrmHandlerPrototype::_nextSlot;

void DrmHandlerPrototype::addPrototype(DrmHandlerPrototype *se) {
    drmHandlerQueue[_nextSlot++] = se;
}

IDrmHandler *DrmHandlerPrototype::create(const std::string &uri, const std::string &format) {

    for (int i = 0; i < _nextSlot; ++i) {
        if (drmHandlerQueue[i]->is_supported(uri)) {
            return drmHandlerQueue[i]->clone(uri , format);
        }
    }

    return nullptr;
}
