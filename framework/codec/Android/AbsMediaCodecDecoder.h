//
// Created by SuperMan on 11/27/20.
//

#ifndef SOURCE_ABSMEDIACODECDECODER_H
#define SOURCE_ABSMEDIACODECDECODER_H

#define CODEC_VIDEO (0)
#define CODEC_AUDIO (1)

#include "../ActiveDecoder.h"
#include "codec/Android/jni/MediaCodec_Decoder.h"
#include <set>

namespace Cicada {

    class AbsMediaCodecDecoder : public ActiveDecoder {
    public:
        AbsMediaCodecDecoder();

        ~AbsMediaCodecDecoder() override;

    private:

        int init_decoder(const Stream_meta *meta, void *wnd, uint64_t flags,
                         std::map<std::string, std::string> drmInfo) override;

        void close_decoder() override;

        int enqueue_decoder(std::unique_ptr<IAFPacket> &pPacket) override;

        int dequeue_decoder(std::unique_ptr<IAFFrame> &pFrame) override;

        void flush_decoder() override;

        int get_decoder_recover_size() override {
            return 0;
        };

    protected:

        virtual bool checkSupport(const Stream_meta &meta, uint64_t flags, int maxSize,
                                  std::map<std::string, std::string> drmInfo) = 0;

        virtual int dequeueInputBufferIndex(int64_t timeoutUs) = 0;

        virtual int initDecoder(const Stream_meta *meta, void *wnd, uint64_t flags,
                                std::map<std::string, std::string> drmInfo) = 0;

        virtual int queueInputBuffer(int index, std::unique_ptr<IAFPacket> &pPacket) = 0;

    protected:

        int commonConfigDecoder();

        static bool commonCheckSupport(const Stream_meta &meta, uint64_t flags, int maxSize);

    private:
        int setCSD(const Stream_meta *meta);

        void releaseDecoder();

    protected:

        MediaCodec_Decoder *mDecoder{nullptr};
        int naluLengthSize = 0;
        int codecType = CODEC_VIDEO;
        bool mbInit{false};

    private:
        int mVideoWidth{0};
        int mVideoHeight{0};

        int channel_count{0};
        int sample_rate{0};
        int format{0};

        std::string mMime{};

        std::recursive_mutex mFuncEntryMutex;

        int mInputFrameCount{0};
        int mOutputFrameCount{0};
        bool mThrowFrame{false};
        bool mUseNdk{false};

        std::mutex mFlushInterruptMuex;
        int mFlushInterrupt{false};

        // mDecoder->flush() state  0: stop  1: flushed 2:Running 3: end-of-stream
        volatile int mFlushState{0};

        std::set<int64_t> mDiscardPTSSet;

        int mMetaVideoWidth{0};
        int mMetaVideoHeight{0};
        void *mVideoOutObser = nullptr;
        int mMetaAudioSampleRate{0};
        int mMetaAudioChannels{0};
        int mMetaAudioIsADTS{0};


    };
}

#endif //SOURCE_ABSMEDIACODECDECODER_H
