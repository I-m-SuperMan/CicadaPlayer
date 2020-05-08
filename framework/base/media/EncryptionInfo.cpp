//
// Created by lifujun on 2020/4/26.
//

#include <utils/frame_work_log.h>
#include "EncryptionInfo.h"

EncryptionInfo::~EncryptionInfo()
{
    if (key_id != nullptr) {
        free(key_id);
    }

    if (iv != nullptr) {
        free(iv);
    }
}

void EncryptionInfo::setKey(const uint8_t *key, uint32_t key_size)
{
    AF_LOGI("key = %p , size = %d", key, key_size);

    if (key != nullptr && key_size > 0) {
        key_id_size = key_size;
        key_id = static_cast<uint8_t *>(malloc(key_size));
        memcpy(key_id, key, key_size);
        _hex_dump(key_id, key_id_size);
    }
}

void EncryptionInfo::setIv(const uint8_t *_iv, uint32_t _iv_size)
{
    AF_LOGI("iv = %p , size = %d", _iv, _iv_size);

    if (_iv != nullptr && _iv_size > 0) {
        iv_size = _iv_size;
        iv = static_cast<uint8_t *>(malloc(_iv_size));
        memcpy(iv, _iv, _iv_size);
        _hex_dump(iv, iv_size);
    }
}
