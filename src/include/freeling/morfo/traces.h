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

#ifndef _TRACES
#define _TRACES

#include <iostream>
#include <string>
#include <list>
#include <cstdlib>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"

/// possible values for MOD_TRACECODE
#define SPLIT_TRACE         0x00000001
#define TOKEN_TRACE         0x00000002
#define MACO_TRACE          0x00000004
#define LANGIDENT_TRACE     0x00000008
#define NUMBERS_TRACE       0x00000010
#define DATES_TRACE         0x00000020
#define PUNCT_TRACE         0x00000040
#define DICT_TRACE          0x00000080
#define AFF_TRACE           0x00000100
#define LOCUT_TRACE         0x00000200
#define NP_TRACE            0x00000400
#define PROB_TRACE          0x00000800
#define QUANT_TRACE         0x00001000
#define NEC_TRACE           0x00002000
#define AUTOMAT_TRACE       0x00004000
#define TAGGER_TRACE        0x00008000
#define SENSES_TRACE        0x00010000
#define CHART_TRACE         0x00020000
#define GRAMMAR_TRACE       0x00040000
#define DEP_TRACE           0x00080000
#define COREF_TRACE         0x00100000
#define UTIL_TRACE          0x00200000
#define WSD_TRACE           0x00400000
#define ALTERNATIVES_TRACE  0x00800000
#define DATABASE_TRACE      0x01000000
#define FEX_TRACE           0x02000000
#define OMLET_TRACE         0x04000000
#define PHONETICS_TRACE     0x08000000
#define MENTIONS_TRACE      0x10000000
#define OUTPUT_TRACE        0x20000000
#define SEMGRAPH_TRACE      0x40000000

#define SUMMARIZER_TRACE    0x80000000

// MOD_TRACECODE and MOD_TRACENAME are empty. The class 
// using the trace is expected to set them
#undef MOD_TRACECODE
#undef MOD_TRACENAME

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class traces implements trace and error handling utilities.
  ////////////////////////////////////////////////////////////////

  class WINDLL traces {
  public:
    // current trace level
    static int TraceLevel;
    // modules to trace
    static unsigned long TraceModule;
  };

} // namespace


/// Macros that must be used to put traces in the code.
/// They may be either defined or null, depending on -DVERBOSE compilation flag. 
#define ERROR_CRASH(msg) { std::wcerr<<MOD_TRACENAME<<L": "<<msg<<std::endl; \
                           exit(1); \
                         }
 
/// Warning macros. Compile without -DNO_WARNINGS (default) to get a code that warns about suspicious things.
/// Compile with -DNO_WARNINGS to get non-warning, exploitation version.
#ifdef NO_WARNINGS
#define WARNING(msg)
#else
#define WARNING(msg)  { std::wcerr<<MOD_TRACENAME<<L": "<<msg<<std::endl; }
#endif

/// Tracing macros. Compile with -DVERBOSE to get a traceable code.
/// Compile without -DVERBOSE (default) to get faster, non-traceable, exploitation version.
#ifdef VERBOSE   
/// ifdef VERBOSE --> TRACE macros exists

#define TRACE(lv,msg) { if (freeling::traces::TraceLevel>=lv && (freeling::traces::TraceModule&MOD_TRACECODE)) { \
                          std::wcerr << MOD_TRACENAME << L": " << msg << std::endl; \
                        } \
                      }
                      

#define TRACE_WORD(lv,wd) { if (freeling::traces::TraceLevel>=lv && (freeling::traces::TraceModule&MOD_TRACECODE)) { \
                              TRACE(lv, L"Word form ["<<wd.get_form()<<L"] ("<<wd.get_span_start()<<L","<<wd.get_span_finish()<<L")"); \
                              for (word::const_iterator an=wd.unselected_begin(); an!=wd.unselected_end(); an++) \
                                TRACE(lv, L"   analysis: <"<<an->get_lemma()<<L","<<an->get_tag()<<L","<<an->get_prob()<<L">"); \
                              for (word::const_iterator an=wd.selected_begin(); an!=wd.selected_end(); an++) \
                                TRACE(lv, L"   analysis: <"<<an->get_lemma()<<L","<<an->get_tag()<<L","<<an->get_prob()<<L"> **");  \
                              if (wd.is_multiword()) {   \
                                TRACE(lv, L"   is a multiword composed by:"); \
                                const std::list<word> & mw = wd.get_words_mw(); \
                                for (std::list<word>::const_iterator p=mw.begin(); p!=mw.end(); p++) \
                                  TRACE(lv, L"     ("<<p->get_form()<<L")"); \
                              } \
                            } \
                          }

#define TRACE_WORD_LIST(lv,wl) { if (freeling::traces::TraceLevel>=lv && (freeling::traces::TraceModule&MOD_TRACECODE)) { \
                                   for (std::list<word>::const_iterator wd=wl.begin(); wd!=wl.end(); wd++) { \
                                     TRACE_WORD(lv, (*wd));             \
                                   } \
                                 } \
                               }

#define TRACE_SENTENCE(lv,s)  { if (freeling::traces::TraceLevel>=lv && (freeling::traces::TraceModule&MOD_TRACECODE)) { \
                                   TRACE(lv,L"BEGIN sentence"); \
                                   TRACE_WORD_LIST(lv,s); \
                                   TRACE(lv,L"END sentence"); \
                                 } \
                              }
                  
#define TRACE_SENTENCE_LIST(lv,ls) { if (freeling::traces::TraceLevel>=lv && (freeling::traces::TraceModule&MOD_TRACECODE)) { \
                                       for (std::list<sentence>::const_iterator s=ls.begin(); s!=ls.end(); s++) \
                                         TRACE_SENTENCE(lv,(*s)); \
                                     } \
                                   }

#else
/// ifndef VERBOSE --> No messages displayed. Faster code.
#define TRACE(x,y)
#define TRACE_WORD(x,y)
#define TRACE_WORD_LIST(x,y)
#define TRACE_SENTENCE(x,y)
#define TRACE_SENTENCE_LIST(x,y)
#endif

#endif
