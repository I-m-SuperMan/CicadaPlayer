//
// Created by lifujun on 2020/5/8.
//

#ifndef SOURCE_MEDIADRMWRAPPER_H
#define SOURCE_MEDIADRMWRAPPER_H


#include <string>
#include <jni.h>
#include <map>

class MediaDrmWrapper {
public:

    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

public:
    MediaDrmWrapper(std::string uuid);

    ~MediaDrmWrapper();

    jobject getMediaCrypto();

    void setKeyRequestInfo(std::string licenceUrl, char *initData, int len,
                           std::string mimeType, int keyType,
                           std::map<std::string, std::string> &params);

private:
    jobject mMediaDrm = nullptr;
};


#endif //SOURCE_MEDIADRMWRAPPER_H
