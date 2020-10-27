//
// Created by SuperMan on 2020/10/22.
//
#ifdef  ANDROID
#include <codec/Android/AndroidDrmSessionManager.h>
#endif

#include "IDrmSessionManager.h"

Cicada::IDrmSessionManager *Cicada::IDrmSessionManager::create() {
#ifdef  ANDROID
    return new AndroidDrmSessionManager();
#endif
    return nullptr;
}
