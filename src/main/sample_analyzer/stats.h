//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#include <sstream>

#if defined WIN32 || defined WIN64
  #include <windows.h>
  #include <time.h>
  typedef LARGE_INTEGER TIME;
  typedef LARGE_INTEGER FREQ;
  #define get_clock_frequency(x) QueryPerformanceFrequency(&x)
  #define get_current_time(x) QueryPerformanceCounter(&x)
  #define get_seconds_passed(start,end,tps) (((double)(end.QuadPart)-(double)(start.QuadPart))/tps.QuadPart)
#else
  #include <sys/times.h>
  typedef struct tms TIME;
  typedef double FREQ;
  #define get_clock_frequency(x) {x=sysconf(_SC_CLK_TCK);}
  #define get_current_time(x) times(&x)
  #define get_seconds_passed(start,end,tps) (((double)(end.tms_utime+end.tms_stime)-(double)(start.tms_utime+start.tms_stime))/tps)
#endif

#include "freeling.h"


/// Server performance statistics

class ServerStats {
 private:
   double cpuTime_total;
   int sentences;
   int words;
   TIME start;
   FREQ tickspersec;

 public:
   /// Constructor
   ServerStats() 
   { 
       ResetStats(); 
       get_clock_frequency(tickspersec);
   }

   /// Destructor
   ~ServerStats() {}

   /// return report on current stats.
   std::wstring GetStats() {
     TIME end;
     get_current_time(end);
     cpuTime_total = get_seconds_passed(start,end,tickspersec);

     std::wostringstream sout;
     sout << L"Words: "<<words<<L", sentences: "<<sentences<<L", cpuTime_total: "<<cpuTime_total<<std::endl;
     sout << L"Words/sentence: "<<(sentences>0?words/sentences:0)<<L", words/second: "<<(cpuTime_total>0?words/cpuTime_total:0)<<L", sentences/second: "<<(cpuTime_total>0?sentences/cpuTime_total:0)<<std::endl;
     return sout.str();
   }

   /// restart counting
   void ResetStats() {
     get_current_time(start);
     words=0;
     sentences=0;
     cpuTime_total=0.0; 
   }

   /// update current stats with a new sentence.
   void UpdateStats(const std::list<sentence> &ls) {
     sentences += ls.size();
     for (std::list<sentence>::const_iterator is=ls.begin(); is!=ls.end(); is++) 
       words+=is->size();
   }     

   /// update current stats with a new sentence.
   void UpdateStats(const document &doc) {
     for (document::const_iterator par=doc.begin(); par!=doc.end(); par++) 
       UpdateStats(*par);
   }
};
