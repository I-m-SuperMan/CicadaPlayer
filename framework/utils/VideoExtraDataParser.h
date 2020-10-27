//
// Created by SuperMan on 2020/10/26.
//

#ifndef CICADAMEDIA_VIDEOEXTRADATAPARSER_H
#define CICADAMEDIA_VIDEOEXTRADATAPARSER_H

extern "C" {
#include <libavcodec/avcodec.h>
};

namespace Cicada {
    class VideoExtraDataParser {

    public:
        VideoExtraDataParser(AVCodecID codecId, uint8_t * extraData, int extraData_size) {
            this->codecId = codecId;
            this->extraData = extraData;
            this->extraData_size = extraData_size;
        };

        ~VideoExtraDataParser() {
            if(sps_data){
                av_freep(&sps_data);
            }

            if(pps_data){
                av_freep(&pps_data);
            }

            if(vps_data){
                av_freep(&vps_data);
            }
        }

        int parser();

    public:
        uint8_t *vps_data = nullptr;
        uint8_t *sps_data = nullptr;
        uint8_t *pps_data = nullptr;
        int vps_data_size = 0;
        int sps_data_size = 0;
        int pps_data_size = 0;

    private:
        AVCodecID codecId = AVCodecID::AV_CODEC_ID_NONE;
        uint8_t* extraData = nullptr;
        int extraData_size = 0;

    };
}


#endif//CICADAMEDIA_VIDEOEXTRADATAPARSER_H
