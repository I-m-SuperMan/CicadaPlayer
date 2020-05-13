//
// Created by moqi on 2019-08-14.
//

#include "AFMediaCodecFrame.h"

AFMediaCodecFrame::AFMediaCodecFrame(IAFFrame::FrameType type, int index, AFMediaCodecFrame::release f_release)
    : mType(type), mIndex(index), mRelease(std::move(f_release))
{
    if (type == FrameTypeVideo) {
        mInfo.format = AF_PIX_FMT_CICADA_MEDIA_CODEC;
    } else {
        mInfo.audio.format = AF_PIX_FMT_CICADA_MEDIA_CODEC;
    }
}


AFMediaCodecFrame::AFMediaCodecFrame(FrameType type, int index, const uint8_t *bufferPtr,  int64_t bufferSize, release f_release)
    : mType(type), mIndex(index), mRelease(std::move(f_release))
{
    if (type == FrameTypeVideo) {
        mInfo.format = AF_PIX_FMT_CICADA_MEDIA_CODEC;
    } else {
        mInfo.audio.format = AF_PIX_FMT_CICADA_MEDIA_CODEC;
    }

    mBufferPtr = bufferPtr;
    mBufferSize = bufferSize;
}

void AFMediaCodecFrame::setDiscard(bool discard)
{
    mDiscard = discard;
}

AFMediaCodecFrame::~AFMediaCodecFrame()
{
    mRelease(mIndex, !mDiscard);
}
