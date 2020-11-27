//
// Created by moqi on 2019/9/26.
//

#ifndef CICADA_PLAYER_CODECPROTOTYPE_H
#define CICADA_PLAYER_CODECPROTOTYPE_H
#include <memory>
#include <map>
#include "IDecoder.h"

class CICADA_CPLUS_EXTERN codecPrototype {
    //    static vector<codecPrototype *> codecQueue;
    static codecPrototype*  codecQueue [10];
    static int _nextSlot;
public:
    virtual ~codecPrototype() = default;

    virtual Cicada::IDecoder *clone() = 0;

    virtual bool is_supported(const Stream_meta &meta, uint64_t flags, int maxSize,std::map<std::string,std::string> drmInfo) = 0;

    static void addPrototype(codecPrototype *se);

    static Cicada::IDecoder *create(const Stream_meta &meta, uint64_t flags, int maxSize, std::map<std::string,std::string> drmInfo);
};


#endif //CICADA_PLAYER_CODECPROTOTYPE_H
