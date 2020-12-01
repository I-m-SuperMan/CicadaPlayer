//
// Created by SuperMan on 12/1/20.
//

#ifndef SOURCE_IWIDEVINEDECODER_H
#define SOURCE_IWIDEVINEDECODER_H


class IWideVineDecoder {
public:
    virtual void setWideVineSession(const std::string& uuid, char* sessionId , int sessionSize) = 0;

    virtual void setWideVineForceInSecureDecoder(bool insecure) = 0;
};


#endif //SOURCE_IWIDEVINEDECODER_H
