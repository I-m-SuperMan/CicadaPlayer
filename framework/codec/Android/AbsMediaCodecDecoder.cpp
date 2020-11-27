//
// Created by SuperMan on 11/27/20.
//

#include "AbsMediaCodecDecoder.h"
#include "codec/Android/jni/MediaCodec_Decoder.h"
#include <utils/frame_work_log.h>
#include <utils/timer.h>
#include <utils/Android/systemUtils.h>
#include <cassert>
#include <map>
#include <utils/ffmpeg_utils.h>
#include <utils/VideoExtraDataParser.h>
#include <base/media/AFMediaCodecFrame.h>
#include <base/media/AVAFPacket.h>

extern "C" {
#include <libavutil/intreadwrite.h>
#include <utils/errors/framework_error.h>
}

#define  MAX_INPUT_SIZE 4
using namespace std;
using namespace Cicada;

typedef struct blackModelDevice {
    AFCodecID codec;
    string model;
} blackModelDevice;
blackModelDevice blackList[] = {
        {AF_CODEC_ID_H264, "2014501"},
        {AF_CODEC_ID_HEVC, "OPPO R9tm"},
        {AF_CODEC_ID_HEVC, "OPPO A59s"},
};

AbsMediaCodecDecoder::AbsMediaCodecDecoder() {
    AF_LOGD("android decoder use jni");
    mFlags |= DECFLAG_HW;
    mDecoder = new MediaCodec_Decoder();
}

AbsMediaCodecDecoder::~AbsMediaCodecDecoder() {
    lock_guard<recursive_mutex> func_entry_lock(mFuncEntryMutex);
    delete mDecoder;
}

bool AbsMediaCodecDecoder::commonCheckSupport(const Stream_meta &meta, uint64_t flags, int maxSize) {
    AFCodecID codec = meta.codec;
    if (codec != AF_CODEC_ID_H264 && codec != AF_CODEC_ID_HEVC
        && codec != AF_CODEC_ID_AAC) {
        return false;
    }

    string version = get_android_property("ro.build.version.sdk");

    if (atoi(version.c_str()) < 16) {
        return false;
    }

    if (atoi(version.c_str()) < 21) {
        if (flags & DECFLAG_ADAPTIVE || codec == AF_CODEC_ID_HEVC || maxSize > 1920) {
            return false;
        }
    }
    string model = get_android_property("ro.product.model");
    for (auto device : blackList) {
        if (device.codec == codec && device.model == model) {
            AF_LOGI("device %d@%s is in black list\n", device.codec, device.model.c_str());
            return false;
        }
    }

    return true;
}

int AbsMediaCodecDecoder::init_decoder(const Stream_meta *meta, void *voutObsr, uint64_t flags,
                                       std::map<std::string, std::string> drmInfo) {
    if (meta->pixel_fmt == AF_PIX_FMT_YUV422P || meta->pixel_fmt == AF_PIX_FMT_YUVJ422P) {
        return -ENOSPC;
    }

    if (!checkSupport(*meta, flags, max(meta->height, meta->width), drmInfo)) {
        return -ENOSPC;
    }

    if (flags & DECFLAG_DIRECT) {
        mFlags |= DECFLAG_OUT;
    }

    if (meta->codec == AF_CODEC_ID_H264) {
        codecType = CODEC_VIDEO;
        mMime = "video/avc";
    } else if (meta->codec == AF_CODEC_ID_HEVC) {
        codecType = CODEC_VIDEO;
        mMime = "video/hevc";
    } else if (meta->codec == AF_CODEC_ID_AAC) {
        codecType = CODEC_AUDIO;
        mMime = "audio/mp4a-latm";
    } else {
        AF_LOGE("codec is %d, not support", meta->codec);
        return -ENOSPC;
    }

    if (codecType == CODEC_VIDEO) {
        mMetaVideoWidth = meta->width;
        mMetaVideoHeight = meta->height;
        mVideoOutObser = voutObsr;
    } else if (codecType == CODEC_AUDIO) {
        mMetaAudioSampleRate = meta->samplerate;
        mMetaAudioChannels = meta->channels;
        mMetaAudioIsADTS = meta->isAdts;
    }

    lock_guard<recursive_mutex> func_entry_lock(mFuncEntryMutex);

    setCSD(meta);

    return initDecoder(meta , voutObsr, flags, drmInfo);

}

int AbsMediaCodecDecoder::setCSD(const Stream_meta *meta) {
    if (meta->codec == AF_CODEC_ID_HEVC) {

        if (meta->extradata == nullptr || meta->extradata_size == 0) {
            return -1;
        }

        VideoExtraDataParser parser(AV_CODEC_ID_HEVC, meta->extradata, meta->extradata_size);
        int ret = parser.parser();
        if (ret >= 0) {
            naluLengthSize = parser.nal_length_size;
            std::list<CodecSpecificData> csdList{};
            CodecSpecificData csd0{};

            const int data_size =
                    parser.vps_data_size + parser.sps_data_size + parser.pps_data_size;
            char data[data_size];

            memcpy(data, parser.vps_data, parser.vps_data_size);
            memcpy(data + parser.vps_data_size, parser.sps_data, parser.sps_data_size);
            memcpy(data + parser.vps_data_size + parser.sps_data_size, parser.pps_data,
                   parser.pps_data_size);

            csd0.setScd("csd-0", data, data_size);
            csdList.push_back(csd0);
            mDecoder->setCodecSpecificData(csdList);

            csdList.clear();
        }

        return ret;
    } else if (meta->codec == AF_CODEC_ID_H264) {

        if (meta->extradata == nullptr || meta->extradata_size == 0) {
            return -1;
        }

        VideoExtraDataParser parser(AV_CODEC_ID_H264, meta->extradata, meta->extradata_size);
        int ret = parser.parser();
        if (ret >= 0) {
            naluLengthSize = parser.nal_length_size;

            std::list<CodecSpecificData> csdList{};
            CodecSpecificData csd0{};
            csd0.setScd("csd-0", parser.sps_data, parser.sps_data_size);
            csdList.push_back(csd0);
            CodecSpecificData csd1{};
            csd1.setScd("csd-1", parser.pps_data, parser.pps_data_size);
            csdList.push_back(csd1);
            mDecoder->setCodecSpecificData(csdList);

            csdList.clear();
        }

        return ret;
    } else if (meta->codec == AF_CODEC_ID_AAC) {
        if (meta->extradata == nullptr || meta->extradata_size == 0) {
            //ADTS, has no extra data . MediaCodec MUST set csd when decode aac

            int samplingFreq[] = {
                    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
                    16000, 12000, 11025, 8000
            };

            // Search the Sampling Frequencies
            int sampleIndex = -1;
            for (int i = 0; i < 12; ++i) {
                if (samplingFreq[i] == mMetaAudioSampleRate) {
                    sampleIndex = i;
                    break;
                }
            }
            if (sampleIndex < 0) {
                return -1;
            }

            const size_t kCsdLength = 2;
            char csd[kCsdLength];
            csd[0] = (meta->profile + 1) << 3 | sampleIndex >> 1;
            csd[1] = (sampleIndex & 0x01) << 7 | meta->channels << 3;

            std::list<CodecSpecificData> csdList{};
            CodecSpecificData csd0{};
            csd0.setScd("csd-0", csd, kCsdLength);
            csdList.push_back(csd0);
            mDecoder->setCodecSpecificData(csdList);

            csdList.clear();
        } else {
            std::list<CodecSpecificData> csdList{};
            CodecSpecificData csd0{};
            csd0.setScd("csd-0", meta->extradata, meta->extradata_size);
            csdList.push_back(csd0);
            mDecoder->setCodecSpecificData(csdList);

            csdList.clear();
        }
        return 0;
    } else {
        return -1;
    }
}

void AbsMediaCodecDecoder::flush_decoder() {
    lock_guard<recursive_mutex> func_entry_lock(mFuncEntryMutex);
    mOutputFrameCount = 0;

    if (!mbInit) {
        return;
    }

    if (mInputFrameCount <= 0) {
        return;
    }

    {
        std::lock_guard<std::mutex> l(mFlushInterruptMuex);
        mFlushState = 1;
        int ret = mDecoder->flush();
        AF_LOGI("clearCache. ret %d, flush state %d", ret, mFlushState);
    }

    mDiscardPTSSet.clear();
    mInputFrameCount = 0;
}

void AbsMediaCodecDecoder::close_decoder() {
    lock_guard<recursive_mutex> func_entry_lock(mFuncEntryMutex);

    // stop decoder.
    // must before destructor producer because inner thread will use surface.
    if (mbInit) {
        mFlushState = 0;
        mDecoder->stop();
        releaseDecoder();
        mbInit = false;
    }

    mInputFrameCount = 0;
}

void AbsMediaCodecDecoder::releaseDecoder() {
    if (mDecoder != nullptr) {
        mDecoder->release();
    }

    if (mDrmSessionManager != nullptr) {
        mDrmSessionManager->releaseDrmSession();
    }
}

int AbsMediaCodecDecoder::enqueue_decoder(unique_ptr<IAFPacket> &pPacket) {
    int index = dequeueInputBufferIndex(1000);

    if (index == MC_ERROR) {
        AF_LOGE("dequeue_in error.");
        return -ENOSPC;
    } else if (index == MC_INFO_TRYAGAIN) {
        return -EAGAIN;
    } else if (index < 0) {
        return index;
    }

    int ret = 0;

    if (index >= 0) {

        if (pPacket != nullptr) {
            if (pPacket->getDiscard()) {
                mDiscardPTSSet.insert(pPacket->getInfo().pts);
            }
        } else {
            AF_LOGD("queue eos codecType = %d\n", codecType);
        }

        ret = queueInputBuffer(index, pPacket);

        if (ret < 0) {
            AF_LOGE(" mDecoder->queue_in error codecType = %d\n", codecType);
        }

        mInputFrameCount++;
    }

    if (mFlushState == 1) {
        std::lock_guard<std::mutex> l(mFlushInterruptMuex);

        if (pPacket != nullptr) {
            AF_LOGI("send Frame mFlushState = 2. pts %"
                            PRId64, pPacket->getInfo().pts);
        }

        mFlushState = 2;
    }

    if (ret == 0) {
        return 0;
    } else {
        AF_LOGE("queue_in error. ret %d", ret);
        return -ENOSPC;
    }

    return ret;
}

int AbsMediaCodecDecoder::dequeue_decoder(unique_ptr<IAFFrame> &pFrame) {
    int ret;
    int index;
    index = mDecoder->dequeueOutputBufferIndex(1000);

    if (index == MC_ERROR) {
        AF_LOGE("dequeue_out occur error. flush state %d", mFlushState);
        return MC_ERROR;
    } else if (index == MC_INFO_TRYAGAIN || index == MC_INFO_OUTPUT_BUFFERS_CHANGED) {
        return -EAGAIN;
    } else if (index == MC_INFO_OUTPUT_FORMAT_CHANGED) {
        mc_out out{};
        mDecoder->getOutput(index, &out, false);

        if (codecType == CODEC_VIDEO) {
            mVideoHeight = out.conf.video.height;

            if (out.conf.video.crop_bottom != MC_ERROR && out.conf.video.crop_top != MC_ERROR) {
                mVideoHeight = out.conf.video.crop_bottom + 1 - out.conf.video.crop_top;
            }

            mVideoWidth = out.conf.video.width;

            if (out.conf.video.crop_right != MC_ERROR && out.conf.video.crop_left != MC_ERROR) {
                mVideoWidth = out.conf.video.crop_right + 1 - out.conf.video.crop_left;
            }
        } else if (codecType == CODEC_AUDIO) {
            channel_count = out.conf.audio.channel_count;
            sample_rate = out.conf.audio.sample_rate;
            format = out.conf.audio.format;
        }

        return -EAGAIN;
    } else if (index >= 0) {
        mc_out out{};
        ret = mDecoder->getOutput(index, &out, codecType != CODEC_VIDEO);
        auto item = mDiscardPTSSet.find(out.buf.pts);

        if (item != mDiscardPTSSet.end()) {
            mDecoder->releaseOutputBuffer(index, false);
            mDiscardPTSSet.erase(item);
            return -EAGAIN;
        }

        if (out.b_eos) {
            return STATUS_EOS;
        }

        // AF_LOGD("mediacodec out pts %" PRId64, out.buf.pts);
        if (codecType == CODEC_VIDEO) {
            pFrame = unique_ptr<AFMediaCodecFrame>(
                    new AFMediaCodecFrame(IAFFrame::FrameTypeVideo, index,
                                          [this](int index, bool render) {
                                              mDecoder->releaseOutputBuffer(index, render);
                                          }));
            pFrame->getInfo().video.width = mVideoWidth;
            pFrame->getInfo().video.height = mVideoHeight;
        } else if (codecType == CODEC_AUDIO) {

            assert(out.buf.p_ptr != nullptr);

            if (out.buf.p_ptr == nullptr) {
                return -EAGAIN;
            }

            AFSampleFormat afFormat = AFSampleFormat::AF_SAMPLE_FMT_NONE;
            if (format < 0 || format == 2) {
                afFormat = AF_SAMPLE_FMT_S16;
            } else if (format == 3) {
                afFormat = AF_SAMPLE_FMT_U8;
            } else if (format == 4) {
                afFormat = AF_SAMPLE_FMT_S32;
            }

            assert(afFormat != AFSampleFormat::AF_SAMPLE_FMT_NONE);

            IAFFrame::AFFrameInfo frameInfo{};
            frameInfo.audio.format = afFormat;
            frameInfo.audio.sample_rate = sample_rate;
            frameInfo.audio.channels = channel_count;

            uint8_t *data[1] = {nullptr};
            data[0] = const_cast<uint8_t *>(out.buf.p_ptr);
            int lineSize[1] = {0};
            lineSize[0] = out.buf.size;

            pFrame = unique_ptr<AVAFFrame>(
                    new AVAFFrame(frameInfo, (const uint8_t **) data, (const int *) lineSize, 1,
                                  IAFFrame::FrameTypeAudio));
            mDecoder->releaseOutputBuffer(index, false);

            pFrame->getInfo().audio.sample_rate = sample_rate;
            pFrame->getInfo().audio.channels = channel_count;
            pFrame->getInfo().audio.format = afFormat;
        }

        pFrame->getInfo().pts = out.buf.pts != -1 ? out.buf.pts : INT64_MIN;

        return 0;
    } else {
        AF_LOGE("unknown error %d\n", index);
        return index;
    }
}

int AbsMediaCodecDecoder::commonConfigDecoder() {

    int ret = -1;
    if (codecType == CODEC_VIDEO) {
        ret = mDecoder->configureVideo(mMime, mMetaVideoWidth, mMetaVideoHeight, 0,
                                       static_cast<jobject>(mVideoOutObser));
    } else if (codecType == CODEC_AUDIO) {
        ret = mDecoder->configureAudio(mMime, mMetaAudioSampleRate, mMetaAudioChannels,
                                       mMetaAudioIsADTS);
    }

    if (ret >= 0) {
        ret = 0;
    } else {
        AF_LOGE("failed to config mDecoder rv %d", ret);
        releaseDecoder();
        ret = gen_framework_errno(error_class_codec, codec_error_video_device_error);
    }

    if (ret == 0) {
        if (mDecoder->start() == MC_ERROR) {
            AF_LOGE("mediacodec start failed.");
            return gen_framework_errno(error_class_codec, codec_error_video_device_error);
        }

        mbInit = true;
        mFlushState = 1;
    }

    return ret;
}