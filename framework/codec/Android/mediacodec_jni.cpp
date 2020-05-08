#define LOG_TAG "mediaCodecDecoder"

#include <cstdlib>
#include "mediacodec_jni.h"
#include "mediaCodec.h"
#include <utils/frame_work_log.h>
#include <utils/Android/systemUtils.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/JniException.h>

using namespace std;
namespace Cicada {

    int MediaCodec_JNI::init(const char *mime, int category, jobject surface)
    {
        category_codec = category;
        mediaCodec = std::unique_ptr<MediaCodecWrapper>(new MediaCodecWrapper());
        int ret = mediaCodec->init(mime, category, surface);

        if (ret == 0) {
            //TODO 修改uuid
            mediaDrm  = std::unique_ptr<MediaDrmWrapper>(new MediaDrmWrapper("edef8ba9-79d6-4ace-a3c8-27dcd51d21ed"));
            jobject crypto = mediaDrm->getMediaCrypto();
            mediaCodec->setMediaCrypto(crypto);
            JniEnv jniEnv;
            JNIEnv *handle = jniEnv.getEnv();
            handle->DeleteLocalRef(crypto);
        }

        return ret;
    }

    int MediaCodec_JNI::start()
    {
        return mediaCodec->start();
    }

    int MediaCodec_JNI::stop()
    {
        return mediaCodec->stop();
    }

    int MediaCodec_JNI::flush()
    {
        return mediaCodec->flush();
    }

    int MediaCodec_JNI::dequeue_in(int64_t timeout)
    {
        int index = mediaCodec->dequeueInputBuffer(timeout);

        if (index >= 0) {
            return index;
        } else {
            return MC_INFO_TRYAGAIN;
        }
    }

    int MediaCodec_JNI::queue_in(int index, const void *p_buf, size_t size, int64_t pts, bool config,
                                 std::unique_ptr<EncryptionInfo> encryptionInfo)
    {
        if (index < 0) {
            AF_LOGE("queue_in fail index = %d", index);
            return MC_ERROR;
        }

        JniEnv jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return MC_ERROR;
        }

        AndroidJniHandle<jobject> inputBuffer(mediaCodec->getInputBuffer(index));

        if ((jobject) inputBuffer == nullptr) {
            AF_LOGE("mediaCodec->getInputBuffer fail . index = %d", index);
            return MC_ERROR;
        }

        int64_t j_mc_size = handle->GetDirectBufferCapacity(inputBuffer);
        auto *p_mc_buf = (uint8_t *) handle->GetDirectBufferAddress(inputBuffer);

        if (j_mc_size < 0) {
            AF_LOGE("Java buffer has invalid size");
            return MC_ERROR;
        }

        if (j_mc_size > size) {
            j_mc_size = size;
        }

        if (p_buf != nullptr) {
            memcpy(p_mc_buf, p_buf, j_mc_size);
        }

        int flags = 0;

        if (config) {
            flags |= BUFFER_FLAG_CODEC_CONFIG;
        }

        if (p_buf == nullptr) {
            flags |= BUFFER_FLAG_END_OF_STREAM;
        }

        if (encryptionInfo != nullptr) {
            return mediaCodec->queueSecureInputBuffer(index, j_mc_size, move(encryptionInfo), pts,
                    flags);
        } else {
            return mediaCodec->queueInputBuffer(index, 0, j_mc_size, pts, flags);
        }
    }

    int MediaCodec_JNI::dequeue_out(int64_t timeout)
    {
        int index = mediaCodec->dequeueOutputBuffer(timeout);

        if (index >= 0) {
            return index;
        } else if (index == INFO_OUTPUT_FORMAT_CHANGED) {
            return MC_INFO_OUTPUT_FORMAT_CHANGED;
        } else if (index == INFO_OUTPUT_BUFFERS_CHANGED) {
            return MC_INFO_OUTPUT_BUFFERS_CHANGED;
        } else {
            return MC_INFO_TRYAGAIN;
        }
    }

    int MediaCodec_JNI::get_out(int index, mc_out *out, bool readBuffer)
    {
        JniEnv jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return MC_ERROR;
        }

        int targetIndex = index;

        if (index == MC_INFO_OUTPUT_FORMAT_CHANGED) {
            targetIndex = INFO_OUTPUT_FORMAT_CHANGED;
        } else if (index == MC_INFO_OUTPUT_BUFFERS_CHANGED) {
            targetIndex = INFO_OUTPUT_BUFFERS_CHANGED;
        }

        OutputBufferInfo info{};
        int ret = mediaCodec->getOutputBufferInfo(targetIndex, &info);

        if (ret == 0) {
            if (info.index >= 0) {
                out->type = MC_OUT_TYPE_BUF;
                out->buf.index = info.index;
                out->buf.pts = info.pts;
                int flags = info.flags;

                if ((flags & BUFFER_FLAG_END_OF_STREAM) != 0) {
                    out->b_eos = true;
                } else {
                    out->b_eos = false;
                }

                if (readBuffer) {
                    AndroidJniHandle<jobject> buf(mediaCodec->getOutputBuffer(info.index));
                    uint8_t *ptr = (uint8_t *) handle->GetDirectBufferAddress(buf);
                    int offset = info.bufferOffset;
                    out->buf.p_ptr = ptr + offset;
                    out->buf.size = info.bufferSize;
                } else {
                    out->buf.p_ptr = nullptr;
                    out->buf.size = 0;
                }

                return 1;
            } else if (info.index == INFO_OUTPUT_FORMAT_CHANGED) {
                out->type = MC_OUT_TYPE_CONF;
                out->b_eos = false;

                if (category_codec == CATEGORY_VIDEO) {
                    out->conf.video.width = info.videoWidth;
                    out->conf.video.height = info.videoHeight;
                    out->conf.video.stride = info.videoStride;
                    out->conf.video.slice_height = info.videoSliceHeight;
                    out->conf.video.pixel_format = info.videoPixelFormat;
                    out->conf.video.crop_left = info.videoCropLeft;
                    out->conf.video.crop_top = info.videoCropTop;
                    out->conf.video.crop_right = info.videoCropRight;
                    out->conf.video.crop_bottom = info.videoCropBottom;
                    AF_LOGI("width %d height %d stride %d slice_height %d crop right %d",
                            out->conf.video.width, out->conf.video.height, out->conf.video.stride,
                            out->conf.video.slice_height, out->conf.video.crop_right);
                } else {
                    out->conf.audio.channel_count = info.audioChannelCount;
                    out->conf.audio.channel_mask = info.audioChannelMask;
                    out->conf.audio.sample_rate = info.audioSampleRate;
                }

                return 1;
            }
        }

        return 0;
    }

    int MediaCodec_JNI::configure(size_t i_h264_profile, const mc_args &args)
    {
//        {
//            NewStringUTF key(handle, "csd-0");
//            handle->CallVoidMethod((jobject) jformat, jfields.set_bytebuffer, key.getString(),
//                                   JavaWVDrm::getPPS());
//        }
//
//        {
//            NewStringUTF key(handle, "csd-1");
//            handle->CallVoidMethod((jobject) jformat, jfields.set_bytebuffer, key.getString(),
//                                   JavaWVDrm::getSPS());
//        }
//
//        wvDrm = JavaWVDrm::create(handle);
//        JavaWVDrm::init(handle, wvDrm,
//                        "AAAARHBzc2gAAAAA7e+LqXnWSs6jyCfc1R0h7QAAACQIARIBNRoNd2lkZXZpbmVfdGVzdCIKMjAxNV90ZWFycyoCU0Q=",
//                        "https://proxy.uat.widevine.com/proxy?provider=widevine_test");
//        wvCrypto = JavaWVDrm::getMediaCrypto(handle, wvDrm);

        //TODO  1.设置csd
        // 2.设置MeidaCrypto
        if (category_codec == CATEGORY_AUDIO) {
            return mediaCodec->configureAudio(args.audio.sample_rate, args.audio.channel_count);
        } else if (category_codec == CATEGORY_VIDEO) {
            return mediaCodec->configureVideo(i_h264_profile, args.video.width, args.video.height,
                                              args.video.angle);
        }

        return MC_ERROR;
    }


    void MediaCodec_JNI::unInit()
    {
    }

    int MediaCodec_JNI::setOutputSurface(jobject surface)
    {
//        JniEnv jniEnv;
//        JNIEnv *handle = jniEnv.getEnv();
//
//        if (!handle) {
//            AF_LOGE("env is nullptr.");
//            return MC_ERROR;
//        }
//
//        handle->CallVoidMethod(codec, jfields.set_output_surface, surface);
//
//        if (JniException::clearException(handle)) {
//            AF_LOGE("Exception in MediaCodec.setOutputSurface");
//            return MC_ERROR;
//        }
//TODO ??
        return 0;
    }

    int MediaCodec_JNI::release_out(int index, bool render)
    {
        if (index < 0) {
            return MC_ERROR;
        }

        return mediaCodec->releaseOutputBuffer(index, render);
    }
}
