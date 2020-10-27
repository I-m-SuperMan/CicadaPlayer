//
// Created by pingkai on 2019/12/31.
//

#ifndef CICADAMEDIA_DEMUXERUTILS_H
#define CICADAMEDIA_DEMUXERUTILS_H

#include <string>
#include <framework/utils/AFMediaType.h>

void test_mergeHeader(std::string url, bool merge);

void test_mergeAudioHeader(const std::string& url , bool  merge);

void testFirstSeek(const std::string &url, int64_t time, int64_t abs_error);

void test_encryptionInfo(const std::string& url, Stream_type streamType, bool merge);

void test_metaKeyInfo(const std::string& url, Stream_type streamType);

void test_csd(const std::string& url , bool merge);

#endif //CICADAMEDIA_DEMUXERUTILS_H
