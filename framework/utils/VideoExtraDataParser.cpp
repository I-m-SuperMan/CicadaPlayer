//
// Created by SuperMan on 2020/10/26.
//

extern "C" {
#include "ffmpeg_utils.h"
}

#include "VideoExtraDataParser.h"

using namespace Cicada;

int VideoExtraDataParser::parser() {
    if (codecId != AV_CODEC_ID_H264 && codecId != AV_CODEC_ID_HEVC) {
        return -1;
    }

    AVCodec *codec = avcodec_find_decoder(codecId);
    if (codec == nullptr) {
        return -1;
    }

    AVCodecContext *avctx = avcodec_alloc_context3((const AVCodec *) codec);
    if (avctx == nullptr) {
        return -1;
    }

    int ret = -1;
    if (codecId == AVCodecID::AV_CODEC_ID_H264) {
        ret = parse_h264_extraData(avctx, extraData, extraData_size,
                                   &sps_data, &sps_data_size,
                                   &pps_data, &pps_data_size ,
                                   &nal_length_size);
    } else if (codecId == AV_CODEC_ID_HEVC) {
        ret = parse_h265_extraData(avctx, extraData, extraData_size,
                                   &vps_data, &vps_data_size,
                                   &sps_data, &sps_data_size,
                                   &pps_data, &pps_data_size,
                                   &nal_length_size);
    }

    avcodec_free_context(&avctx);

    return ret;
}
