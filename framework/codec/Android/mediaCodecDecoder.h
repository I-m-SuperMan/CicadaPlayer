#ifndef QU_ANDROID_H264_DECODER_HH
#define QU_ANDROID_H264_DECODER_HH

#include <cstdio>
#include <list>
#include <jni.h>
#include <thread>
#include <condition_variable>
#include <codec/IDecoder.h>
#include <utils/afThread.h>
#include <base/media/AVAFPacket.h>
#include <queue>
#include <base/media/AFMediaCodecFrame.h>
#include "codec/ActiveDecoder.h"
#include "../codecPrototype.h"
#include "MediaCodec_Decoder.h"
#include <set>


#define CODEC_VIDEO (0)
#define CODEC_AUDIO (1)

namespace Cicada{
    class mediaCodecDecoder : public ActiveDecoder, private codecPrototype {
    public:
        mediaCodecDecoder();

        ~mediaCodecDecoder() override;

    private:

        int init_decoder(const Stream_meta *meta, void *wnd, uint64_t flags) override;

        void close_decoder() override;

        int enqueue_decoder(std::unique_ptr<IAFPacket> &pPacket) override;

        int dequeue_decoder(std::unique_ptr<IAFFrame> &pFrame) override;

        void flush_decoder() override;

        int get_decoder_recover_size() override
        {
            return 0;
        };

    private:
        static bool checkSupport(const Stream_meta &meta, uint64_t flags, int maxSize);

        int setSCD(const Stream_meta *meta);

        void releaseDecoder();

    private:
        explicit mediaCodecDecoder(int dummy)
        {
            addPrototype(this);
        };

        mediaCodecDecoder *clone() override
        {
            return new mediaCodecDecoder();
        };

        bool is_supported(const Stream_meta &meta, uint64_t flags, int maxSize) override
        {
            if (flags & DECFLAG_HW)
                return checkSupport(meta, flags, maxSize);
            return false;
        };
        static mediaCodecDecoder se;

    private:
        int mVideoWidth{0};
        int mVideoHeight{0};

        int channel_count{0};
        int sample_rate{0};
        int format{0};

        int codecType = CODEC_VIDEO;

        MediaCodec_Decoder *mDecoder{nullptr};

        std::recursive_mutex mFuncEntryMutex;
        bool mbInit{false};

        int mInputFrameCount{0};
        int mOutputFrameCount{0};
        bool mThrowFrame{false};
        bool mUseNdk{false};

        std::mutex mFlushInterruptMuex;
        int mFlushInterrupt{false};

        // mDecoder->flush() state  0: stop  1: flushed 2:Running 3: end-of-stream
        volatile int mFlushState{0};

        std::set<int64_t> mDiscardPTSSet;

    };
}

#endif // QU_ANDROID_H264_DECODER_HH
