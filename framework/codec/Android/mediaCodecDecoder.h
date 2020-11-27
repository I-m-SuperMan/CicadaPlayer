#ifndef QU_ANDROID_H264_DECODER_HH
#define QU_ANDROID_H264_DECODER_HH

#include "../codecPrototype.h"
#include "AbsMediaCodecDecoder.h"

namespace Cicada {
    class mediaCodecDecoder : public AbsMediaCodecDecoder, private codecPrototype {
    public:
        mediaCodecDecoder();

        ~mediaCodecDecoder() override;

    private:
        bool checkSupport(const Stream_meta &meta, uint64_t flags, int maxSize,
                                  std::map<std::string, std::string> drmInfo) override ;

        int dequeueInputBufferIndex(int64_t timeoutUs) override;

        int initDecoder(const Stream_meta *meta , void *wnd, uint64_t flags,
                std::map<std::string, std::string> drmInfo) override;

        int queueInputBuffer(int index, std::unique_ptr<IAFPacket> &pPacket) override;

    private:
        explicit mediaCodecDecoder(int dummy) {
            addPrototype(this);
        };

        mediaCodecDecoder *clone() override {
            return new mediaCodecDecoder();
        };

        bool is_supported(const Stream_meta &meta, uint64_t flags, int maxSize,
                          std::map<std::string, std::string> drmInfo) override {
            if (flags & DECFLAG_HW)
                return checkSupport(meta, flags, maxSize, drmInfo);
            return false;
        };

        static mediaCodecDecoder se;

    };
}

#endif // QU_ANDROID_H264_DECODER_HH
