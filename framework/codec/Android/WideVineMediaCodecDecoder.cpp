//
// Created by SuperMan on 11/27/20.
//

#define LOG_TAG "WideVineMediaCodecDecoder"

#include <utils/frame_work_log.h>
#include "WideVineMediaCodecDecoder.h"

extern "C" {
#include <libavutil/intreadwrite.h>
#include <utils/errors/framework_error.h>
}

#define  MAX_INPUT_SIZE 4
using namespace std;
using namespace Cicada;

WideVineMediaCodecDecoder WideVineMediaCodecDecoder::se(0);

WideVineMediaCodecDecoder::WideVineMediaCodecDecoder() {
    AF_LOGD("android decoder use jni");
}

WideVineMediaCodecDecoder::~WideVineMediaCodecDecoder() {

}

int WideVineMediaCodecDecoder::initDecoder(const Stream_meta *meta, void *wnd, uint64_t flags,
                std::map<std::string, std::string> drmInfo) {
    if (drmInfo.count("keyFormat") > 0 && drmInfo.count("keyUrl") > 0) {
        const std::string &format = drmInfo["keyFormat"];
        if (format != "urn:uuid:edef8ba9-79d6-4ace-a3c8-27dcd51d21ed") {
            return false;
        }

        mDrmUrl = drmInfo["keyUrl"];
        mDrmFormat = drmInfo["keyFormat"];
    }

}


int WideVineMediaCodecDecoder::configDecoder() {

    int ret = 0;

//config drm info
    if (mDrmSessionManager != nullptr) {

        bool forceInsecureDecoder = mDrmSessionManager->isForceInsecureDecoder();
        mDecoder->setForceInsecureDecoder(forceInsecureDecoder);

        int drmSessionSize = 0;
        void *drmSessionId = mDrmSessionManager->getSession(&drmSessionSize);
        assert(drmSessionId != nullptr);
        ret = mDecoder->setDrmInfo("edef8ba9-79d6-4ace-a3c8-27dcd51d21ed",
                                   drmSessionId, drmSessionSize);
    }

    if (ret < 0) {
        AF_LOGE("failed to config mDecoder drm info %d", ret);
        ret = gen_framework_errno(error_class_codec, codec_error_video_device_error);
        return ret;
    }

    return commonConfigDecoder();
}


int WideVineMediaCodecDecoder::dequeueInputBufferIndex(int64_t timeoutUs) {
    if (mDrmSessionManager != nullptr) {
        int state{};
        int code{};
        mDrmSessionManager->getSessionState(&state, &code);
        if (state == SESSION_STATE_IDLE) {
            AF_LOGD("drm not ready");
            return -EAGAIN;
        } else if (state == SESSION_STATE_ERROR) {
            AF_LOGE("drm error : %s", framework_err2_string(code));
            return code;
        } else if (state == SESSION_STATE_OPENED) {
            // drm has opened
            if (!mbInit) {
                int ret = configDecoder();
                if (ret < 0) {
                    return ret;
                }
            }
        }

    }

    int index = mDecoder->dequeueInputBufferIndex(timeoutUs);
    return index;
}


int WideVineMediaCodecDecoder::queueInputBuffer(int index, unique_ptr<IAFPacket> &pPacket) {
    int ret = 0;

    uint8_t *data = nullptr;
    int size = 0;
    int64_t pts = 0;

    if (pPacket != nullptr) {
        data = pPacket->getData();
        size = static_cast<int>(pPacket->getSize());
        pts = pPacket->getInfo().pts;
    } else {
        AF_LOGD("queue eos codecType = %d\n", codecType);
    }

    if (mDrmSessionManager != nullptr) {
        IAFPacket::EncryptionInfo encryptionInfo{};
        if (pPacket != nullptr) {
            pPacket->getEncryptionInfo(&encryptionInfo);
        }

        uint8_t *new_data = nullptr;
        int new_size = 0;
        convertToDrmDataIfNeed(&new_data, &new_size, data, size);

        if (new_data != nullptr) {
            data = new_data;
            size = new_size;
        }

        ret = mDecoder->queueSecureInputBuffer(index, data, static_cast<size_t>(size),
                                               &encryptionInfo, pts,
                                               false);

        if (new_data != nullptr) {
            free(new_data);
        }

    } else {
        ret = mDecoder->queueInputBuffer(index, data, static_cast<size_t>(size), pts,
                                         false);
    }

    return ret;
}


bool WideVineMediaCodecDecoder::checkSupport(const Stream_meta &meta, uint64_t flags, int maxSize,
                                             std::map<std::string, std::string> drmInfo) {

    if (!commonCheckSupport(meta, flags, maxSize)) {
        return false;
    }

    if (drmInfo.count("keyFormat") > 0 && drmInfo.count("keyUrl") > 0) {
        const std::string &format = drmInfo["keyFormat"];
        if (format != "urn:uuid:edef8ba9-79d6-4ace-a3c8-27dcd51d21ed") {
            return false;
        }

        mDrmUrl = drmInfo["keyUrl"];
        mDrmFormat = drmInfo["keyFormat"];
    }

    return true;
}

void
WideVineMediaCodecDecoder::convertToDrmDataIfNeed(uint8_t **new_data, int *new_size,
                                                  const uint8_t *data,
                                                  int size) {
    if (data == nullptr || size == 0) {
        return;
    }

    if (codecType == CODEC_VIDEO) {
        assert(naluLengthSize != 0);
    }

    if (naluLengthSize == 0) {
        return;
    }

    const int nalPrefixLen = 5;
    char nalPrefixData[nalPrefixLen];
    nalPrefixData[0] = 0;
    nalPrefixData[1] = 0;
    nalPrefixData[2] = 0;
    nalPrefixData[3] = 0;
    nalPrefixData[4] = 0;

    int sampleBytesWritten = 0;
    int sampleSize = size;

    std::vector<uint8_t> tmpData{};

    int nalUnitPrefixLength = naluLengthSize + 1;
    int nalUnitLengthFieldLengthDiff = 4 - naluLengthSize;
    int sampleCurrentNalBytesRemaining = 0;

    *new_size = size;
    *new_data = static_cast<uint8_t *>(malloc(*new_size));
    uint8_t *new_data_ptr = *new_data;
    uint8_t *ori_data_ptr = const_cast<uint8_t *>(data);

    while (sampleBytesWritten < sampleSize) {

        if (sampleCurrentNalBytesRemaining == 0) {
//new nal
            for (int i = 0; i < nalUnitPrefixLength; i++) {
                nalPrefixData[nalUnitLengthFieldLengthDiff + i] = *ori_data_ptr;
                ori_data_ptr++;
            }

            int nalLengthInt = AV_RB32(nalPrefixData);
            if (nalLengthInt < 1) {
                AF_LOGE("Invalid NAL length");
                return;
            }

            sampleCurrentNalBytesRemaining = nalLengthInt - 1;

            if (sampleBytesWritten + 5 > *new_size) {

                *new_size = sampleBytesWritten + 5;

                *new_data = static_cast<uint8_t *>(realloc(*new_data, *new_size));
                new_data_ptr = *new_data + sampleBytesWritten;
            }

            //put start code
            *new_data_ptr = 0;
            new_data_ptr++;
            *new_data_ptr = 0;
            new_data_ptr++;
            *new_data_ptr = 0;
            new_data_ptr++;
            *new_data_ptr = 1;
            new_data_ptr++;
            //nal type
            *new_data_ptr = nalPrefixData[4];
            new_data_ptr++;

            sampleBytesWritten += 5;
            sampleSize += nalUnitLengthFieldLengthDiff;

        } else {
            if (sampleBytesWritten + sampleCurrentNalBytesRemaining > *new_size) {
                *new_size = sampleBytesWritten + sampleCurrentNalBytesRemaining;
                *new_data = static_cast<uint8_t *>(realloc(*new_data, *new_size));

                new_data_ptr = *new_data + sampleBytesWritten;
            }
            memcpy(new_data_ptr, ori_data_ptr, sampleCurrentNalBytesRemaining);
            ori_data_ptr = ori_data_ptr + sampleCurrentNalBytesRemaining;
            new_data_ptr = new_data_ptr + sampleCurrentNalBytesRemaining;

            sampleBytesWritten += sampleCurrentNalBytesRemaining;
            sampleCurrentNalBytesRemaining -= sampleCurrentNalBytesRemaining;
        }
    }

    assert(*new_size == sampleBytesWritten);

    *new_size = sampleBytesWritten;
}


