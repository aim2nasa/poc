#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>

bool initSoftHSM();
void finalizeSoftHSM();
bool rm(std::string path);
bool rmdir(std::string path);
bool findTokenDirectory(std::string basedir, std::string& tokendir, char* serial, char* label);

#endif