//
// Created by lifujun on 2020/4/26.
//

#ifndef SOURCE_ENCRYPTIONINFO_H
#define SOURCE_ENCRYPTIONINFO_H


#include <cstdint>
#include <string>
#include "SubsampleEncryptionInfo.h"

class EncryptionInfo {

public:

    void setKey(const uint8_t* key , uint32_t key_size);

    void setIv(const uint8_t* iv , uint32_t iv_size);

    EncryptionInfo() = default;
    ~EncryptionInfo();

    std::string scheme{};

    uint32_t crypt_byte_block{0};
    uint32_t skip_byte_block{0};

    uint8_t *key_id{nullptr};
    uint32_t key_id_size{0};

    uint8_t *iv{nullptr};
    uint32_t iv_size{0};

    std::unique_ptr<SubsampleEncryptionInfo> subsamples{nullptr};
    uint32_t subsample_count{0};
};


#endif //SOURCE_ENCRYPTIONINFO_H
