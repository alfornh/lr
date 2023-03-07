#ifndef _DIS_INC_H__
#define _DIS_INC_H__
//#ifdef _WIN32

#define FILE_PATH_SEPERATOR "\\"

extern "C" {

bool zfexist(const char *path);

int zmkdir(const char *path);

}

//#endif//_WIN32

#endif//_DIS_INC_H__