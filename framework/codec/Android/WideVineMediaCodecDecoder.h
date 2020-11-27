//
// Created by SuperMan on 11/27/20.
//

#ifndef SOURCE_WIDEVINEMEDIACODECDECODER_H
#define SOURCE_WIDEVINEMEDIACODECDECODER_H

#include <codec/codecPrototype.h>
#include <drm/IDrmHandler.h>
#include "AbsMediaCodecDecoder.h"

namespace Cicada {
    class WideVineMediaCodecDecoder : public AbsMediaCodecDecoder, private codecPrototype {
    public:
        WideVineMediaCodecDecoder();

        ~WideVineMediaCodecDecoder() override;

    private:

        bool checkSupport(const Stream_meta &meta, uint64_t flags, int maxSize,
                          std::map<std::string, std::string> drmInfo) override ;

        int dequeueInputBufferIndex(int64_t timeoutUs) override;

        int initDecoder(const Stream_meta *meta, void *wnd, uint64_t flags,
                        std::map<std::string, std::string> drmInfo) override;

        int queueInputBuffer(int index, std::unique_ptr<IAFPacket> &pPacket) override;


    private:
        explicit WideVineMediaCodecDecoder(int dummy) {
            addPrototype(this);
        };

        WideVineMediaCodecDecoder *clone() override {
            return new WideVineMediaCodecDecoder();
        };

        bool is_supported(const Stream_meta &meta, uint64_t flags, int maxSize,
                          std::map<std::string, std::string> drmInfo) override {
            if (flags & DECFLAG_HW)
                return checkSupport(meta, flags, maxSize, drmInfo);
            return false;
        };

        static WideVineMediaCodecDecoder se;

    private:

        int configDecoder();

        void
        convertToDrmDataIfNeed(uint8_t **new_data, int *new_size, const uint8_t *data, int size);

    private:

        std::string mDrmUrl{};
        std::string mDrmFormat{};

        IDrmHandler* mDrmHandler = nullptr;
    };
}


#endif //SOURCE_WIDEVINEMEDIACODECDECODER_H
