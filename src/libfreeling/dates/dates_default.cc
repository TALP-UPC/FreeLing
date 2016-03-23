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

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/dates_modules.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"DATES"
#define MOD_TRACECODE DATES_TRACE

  ///////////////////////////////////////////////////////////////
  ///  Abstract class constructor.
  ///////////////////////////////////////////////////////////////

  dates_module::dates_module(const std::wstring &rd, const std::wstring &rt1, const std::wstring &rt2, const std::wstring &rtrom): automat<dates_status>(), RE_Date(rd), RE_Time1(rt1), RE_Time2(rt2), RE_Roman(rtrom) {}

  ///////////////////////////////////////////////////////////////
  ///  Normalize a number, removing non-significant zeros
  ///////////////////////////////////////////////////////////////

  std::wstring dates_module::normalize(const std::wstring &in, int offs) const {
    return (in!=UNKNOWN_SYMB ? util::int2wstring(util::wstring2int(in)+offs) : in);
  }


  ///////////////////////////////////////////////////////////////
  ///   Reset acumulators used by state actions:
  ///   day, year, month, hour, minute, etc.
  ///////////////////////////////////////////////////////////////

  void dates_module::ResetActions(dates_status *st) const {

    st->century = st->weekday = st->day = st->month = st->year = UNKNOWN_SYMB;
    st->hour = st->minute = st->meridian = UNKNOWN_SYMB;
    st->temp = -1;
    st->sign=0;
    st->inGbb=false;
    st->daytemp=-1;
  }



  //-----------------------------------------------//
  //        Default date/time recognizer           //
  //        Only recognize simple patterns         //
  //-----------------------------------------------//

  // State codes 
#define ST_A  1     // A: initial state
#define ST_B  2     // B: Date pattern found
#define ST_C  3     // C: hour pattern found after date
#define ST_D  4     // D: hh+mm patter found after date
  // stop state
#define ST_STOP 5

  // Token codes
#define TK_hour    1  // hour: something matching just first part of RE_TIME1: 3h, 17h
#define TK_hhmm    2  // hhmm: something matching whole RE_TIME1: 3h25min, 3h25m, 3h25minutos,
#define TK_min     3  // min: something matching RE_TIME2: 25min, 3minutos
#define TK_date    4  // date: something matching RE_DATE: 3/ago/1992, 3/8/92, 3/agosto/1992, ...
#define TK_other   5  // other: any other token, not relevant in current state


  ///////////////////////////////////////////////////////////////
  ///  Create a default dates recognizer.
  ///////////////////////////////////////////////////////////////

  dates_default::dates_default(): dates_module(RE_DATE_DF, RE_TIME1_DF, RE_TIME2_DF, RE_ROMAN)
  {
    // Initialize special state attributes
    initialState=ST_A; stopState=ST_STOP;

    // Initialize Final state set 
    Final.insert(ST_B); Final.insert(ST_C); Final.insert(ST_D);

    // Initialize transitions table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;

    // State A
    trans[ST_A][TK_date]=ST_B;
    // State B
    trans[ST_B][TK_hour]=ST_C; trans[ST_B][TK_hhmm]=ST_D; 
    // State C
    trans[ST_C][TK_min]=ST_D;
    // State D
    // nothing else expected. 

    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int dates_default::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
    wstring form,formU;
    int token;
    vector<wstring> rem;  // to store r.e. match results

    formU = j->get_form();
    form = j->get_lc_form();

    token = TK_other;
    if (RE_Date.search(form)) {
      TRACE(3,L"Match DATE regex.");  
      token = TK_date;
    }
    else if (RE_Time1.search(form,rem)) {
      if (rem.size()>=3) {   // if (not rem[2].str().empty()) ??  // if (RE_Time1.Match(2)!="")
        TRACE(3,L"Match TIME1 regex (hour+min)");
        token = TK_hhmm;
      }
      else{
        TRACE(3,L"Partial match TIME1 regex (hour)");
        token = TK_hour;
      }
    }
    else if (RE_Time2.search(form)) {
      TRACE(3,L"Match TIME2 regex (minutes)");
      token = TK_min;
    }

    TRACE(3,L"Leaving state "+util::int2wstring(state)+L" with token "+util::int2wstring(token));
    return (token);
  }


  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state with an informative 
  ///  token (day, year, month, etc) store that part of the date.
  ///////////////////////////////////////////////////////////////

  void dates_default::StateActions(int origin, int state, int token, sentence::const_iterator j, dates_status *st) const {
    wstring form;

    form = j->get_lc_form();
    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ["+form+L"]");

    // switch (state)

    TRACE(3,L"State actions finished. ["+st->day+L":"+st->month+L":"+st->year+L":"+st->hour+L":"+st->minute+L"]");
  }



  ///////////////////////////////////////////////////////////////
  ///   Set the appropriate lemma and tag for the
  ///   new multiword.
  ///////////////////////////////////////////////////////////////

  void dates_default::SetMultiwordAnalysis(sentence::iterator i, int fstate, const dates_status *st) const {
    list<analysis> la;
    wstring lemma;

    // Setting the analysis for the date
    lemma=L"["+st->weekday+L":"+st->day+L"/"+st->month+L"/"+st->year+L":"+st->hour+L"."+st->minute+L":"+st->meridian+L"]";

    la.push_back(analysis(lemma,L"W"));
    i->set_analysis(la);
    TRACE(3,L"Analysis set to: ("+lemma+L" W)");
  }

} // namespace
