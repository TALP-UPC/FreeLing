#include "dynet/globals.h"
#include "dynet/devices.h"
#include "dynet/timing.h"

#ifdef HAVE_CUDA
#include "dynet/cuda.h"
#endif

namespace dynet {

std::mt19937* rndeng = nullptr;
DYN_WINDLL Device* default_device = nullptr;
float default_weight_decay_lambda;
int autobatch_flag; 
int profiling_flag = 0;
NamedTimer timer;

}
