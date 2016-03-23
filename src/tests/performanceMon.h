
#if defined WIN32 || defined WIN64
#include <windows.h>

#define beginCheckTime(str_in) LARGE_INTEGER lstart##str_in, lend##str_in, lfreq##str_in; \
int __tp = GetThreadPriority(GetCurrentThread()); SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL); \
QueryPerformanceFrequency(&lfreq##str_in); QueryPerformanceCounter(&lstart##str_in);

#define endCheckTime(str_in) QueryPerformanceCounter(&lend##str_in); SetThreadPriority(GetCurrentThread(), __tp); \
std::wcout << (double)(lend##str_in.QuadPart - lstart##str_in.QuadPart) * 1000.0 / lfreq##str_in.QuadPart << "  in:  " << (int)##str_in << std::endl;

long long _benchmark(void(*pfunc)())
{
     LARGE_INTEGER freq,start,stop;
     int tp;
    
     QueryPerformanceFrequency(&freq);
    
     if (freq.QuadPart == 0)
     {
       return 0;
     }
    
     tp = GetThreadPriority(GetCurrentThread());
    
     if (tp == THREAD_PRIORITY_ERROR_RETURN)
     {
       return 0;
     }
    
     SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
     QueryPerformanceCounter(&start);
     (*pfunc)();
     QueryPerformanceCounter(&stop);
     SetThreadPriority(GetCurrentThread(),tp);
    
     return (long long)((stop.QuadPart - start.QuadPart) * 1000 /(double)(freq.QuadPart));
}

#define CHECKSPENDTIME(pfunc, funkName) std::cout << ##funkName << _benchmark(pfunc);

#else
#include <sys/time.h>
#include <iostream>
#include <sys/resource.h>

    #define beginCheckTime(str_in) struct timeval lstart##str_in, lend##str_in; pid_t pid = getpid();\
    int prior = getpriority(PRIO_PROCESS, pid); setpriority(PRIO_PROCESS, pid, -20); gettimeofday(&lstart##str_in, NULL);
    
    #define endCheckTime(str_in) gettimeofday(&lend##str_in, NULL); setpriority(PRIO_PROCESS, pid, prior);\
    std::cout << (lend##str_in.tv_sec - lstart##str_in.tv_sec)*1000000 - (lend##str_in.tv_usec - lstart##str_in.tv_usec)  << std::endl;
    #define CHECKSPENDTIME(pfunc, funkName) (*pfunc)();
    
    
    long long _benchmark(void(*pfunc)())                                                                                                                                      
    {
	struct timeval start, stop;
	pid_t pid = getpid();

	int prior = getpriority(PRIO_PROCESS, pid);
	setpriority(PRIO_PROCESS, pid, -20);
        gettimeofday(&start, NULL);       
        (*pfunc)();
        gettimeofday(&stop, NULL);
        setpriority(PRIO_PROCESS, pid, prior);
        return ((stop.tv_sec - start.tv_sec)*1000000 - (stop.tv_usec - start.tv_usec));
    }
#endif
