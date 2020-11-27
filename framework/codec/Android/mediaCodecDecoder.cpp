#define LOG_TAG "mediaCodecDecoder"

#include "mediaCodecDecoder.h"


#define  MAX_INPUT_SIZE 4
using namespace std;
namespace Cicada {

    mediaCodecDecoder mediaCodecDecoder::se(0);

    mediaCodecDecoder::mediaCodecDecoder() {
    }

    mediaCodecDecoder::~mediaCodecDecoder() {
    }

    bool mediaCodecDecoder::checkSupport(const Stream_meta &meta, uint64_t flags, int maxSize,
                                         std::map<std::string, std::string> drmInfo) {
        return commonCheckSupport(meta, flags, maxSize);
    }

    int mediaCodecDecoder::dequeueInputBufferIndex(int64_t timeoutUs) {
        int index = mDecoder->dequeueInputBufferIndex(timeoutUs);
        return index;
    }

    int mediaCodecDecoder::initDecoder(const Stream_meta *meta, void *wnd, uint64_t flags,
                                       std::map<std::string, std::string> drmInfo) {
        return commonConfigDecoder();
    }

    int mediaCodecDecoder::queueInputBuffer(int index, std::unique_ptr<IAFPacket> &pPacket) {
        uint8_t *data = nullptr;
        int size = 0;
        int64_t pts = 0;

        if (pPacket != nullptr) {
            data = pPacket->getData();
            size = static_cast<int>(pPacket->getSize());
            pts = pPacket->getInfo().pts;
        }

        return mDecoder->queueInputBuffer(index, data, static_cast<size_t>(size), pts,
                                          false);
    }

}
