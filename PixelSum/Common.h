#pragma once

#ifdef  _WIN32
#ifdef __COMPILING_PIXEL_SUM
#define PIXEL_SUM_API __declspec(dllexport)
#else
#define PIXEL_SUM_API __declspec(dllimport)
#endif // __COMPILING_PIXEL_SUM
#else
#define PIXEL_SUM_API
#endif //  _WIN32
