
#if defined WIN32 || defined WIN64
#  pragma warning(disable : 4251)
#  pragma warning(disable : 4275)
#endif


#ifndef __WINDLL_H__
#define __WINDLL_H__

#if defined WIN32 || defined WIN64 
  #define getcwd _getcwd
  #include <iso646.h>

  #ifdef TREELER_EXPORTS
    #define WINDLL __declspec(dllexport)
    #define EXPIMP_TEMPLATE
  #else
    #define WINDLL __declspec(dllimport)
    #define EXPIMP_TEMPLATE extern
  #endif

#else
  #define WINDLL
#endif

#endif
