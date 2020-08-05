#ifndef DYNET_GLOBALS_H
#define DYNET_GLOBALS_H

#include <random>

#if defined WIN32 || defined WIN64
#  pragma warning(disable : 4251)
#  pragma warning(disable : 4275)
#endif

#if defined WIN32 || defined WIN64
#ifdef DYNET_EXPORTS
#  define DYN_WINDLL __declspec(dllexport)
#else
#  define DYN_WINDLL __declspec(dllimport)
#endif
#else
#define DYN_WINDLL
#endif


namespace dynet {

class Device;
class NamedTimer;

extern std::mt19937* rndeng;
DYN_WINDLL extern Device* default_device;
extern NamedTimer timer; // debug timing in executors.

} // namespace dynet

#endif
