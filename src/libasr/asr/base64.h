#ifndef _BASE64_H_
#define _BASE64_H_

#include <vector>
#include <string>

std::string base64_encode(unsigned char const* buf, unsigned int bufLen);
std::string base64_decode(const std::string &encoded);

#endif
