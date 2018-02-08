#pragma once

//#define RES_PATH "data/facecloud/"
#ifdef WIN32

#define RES_PATH std::string("data/")

#else
#define RES_PATH std::string("/root/jni/data/")

#endif // WIN32

