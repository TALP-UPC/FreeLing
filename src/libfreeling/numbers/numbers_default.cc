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
#include "freeling/morfo/numbers_modules.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"NUMBERS"
#define MOD_TRACECODE NUMBERS_TRACE

  ///////////////////////////////////////////////////////////////
  ///  Abstract class constructor.
  ///////////////////////////////////////////////////////////////

  numbers_module::numbers_module(const std::wstring &dec, const std::wstring &thou): automat<numbers_status>(), MACO_Decimal(dec), MACO_Thousand(thou), RE_code(RE_CODE), RE_number(RE_NUM), RE_number_neg(RE_NUM_NEG) {}

  ///////////////////////////////////////////////////////////////
  ///   Reset acumulators used by state actions:
  ///   bilion, milion, units.
  ///////////////////////////////////////////////////////////////

  void numbers_module::ResetActions(numbers_status *st) const {
    numbers_status *nst = (numbers_status*)st;
    nst->iscode=0;
    nst->milion=0; nst->bilion=0; nst->units=0;
    nst->block=0;
  }


  //-----------------------------------------------//
  //        Default number recognizer              //
  //        Only recognize digits and codes        //
  //-----------------------------------------------//

  // State codes
#define ST_A 1
#define ST_NUM 2 // got number from initial state
#define ST_COD 3 // got pseudo-numerical code from initial state
  // stop state
#define ST_STOP 4

  // Token codes
#define TK_num   1  // a number (in digits)
#define TK_code  2  // a code (ex. LX-345-2)
#define TK_other 3

  ///////////////////////////////////////////////////////////////
  ///  Create a default numbers recognizer.
  ///////////////////////////////////////////////////////////////

  numbers_default::numbers_default(const std::wstring &dec, const std::wstring &thou): numbers_module(dec,thou)
  {  
    // Initializing value map
    // Initialize special state attributes
    initialState=ST_A; stopState=ST_STOP;

    // Initialize Final state set 
    Final.insert(ST_NUM);  Final.insert(ST_COD);

    // Initialize transitions table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;
  
    // Initializing transitions table
    // State A
    trans[ST_A][TK_num]=ST_NUM;  trans[ST_A][TK_code]=ST_COD;
    // State NUM
    // nothing else is expected
    // State COD
    // nothing else is expected
 
    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Computenthe right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int numbers_default::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
    wstring form;
    int token;
 
    form = j->get_lc_form();
  
    // check to see if it is a number
    if (RE_number.search(form)) token = TK_num;
    else if (RE_code.search(form)) token = TK_code;
    else token = TK_other;

    TRACE(3,L"Next word form is: ["+form+L"] token="+util::int2wstring(token));     

    TRACE(3,L"Leaving state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)); 
    return (token);
  }


  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state, update current 
  ///  nummerical value.
  ///////////////////////////////////////////////////////////////

  void numbers_default::StateActions(int origin, int state, int token, sentence::const_iterator j, numbers_status *st) const {
    wstring form;
    size_t i;
    long double num;

    form = j->get_lc_form();
    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ["+form+L"]");

    // get token numerical value, if any
    num=0;
    if (token==TK_num) {
      // erase thousand points
      while ((i=form.find_first_of(MACO_Thousand))!=wstring::npos) {
        form.erase(i,1);
      }
      TRACE(3,L"after erasing thousand "+form);
      // make sure decimal point is "."
      if ((i=form.find_last_of(MACO_Decimal))!=wstring::npos) {
        form[i]=L'.';
        TRACE(3,L"after replacing decimal "+form);
      }

      num = util::wstring2longdouble(form);
    }

    // State actions
    switch (state) {
      // ---------------------------------
    case ST_NUM:
      TRACE(3,L"Actions for NUM state");
      if (token==TK_num)
        st->units += num;
      break;
      // ---------------------------------
    case ST_COD:
      TRACE(3,L"Actions for COD state");
      st->iscode = CODE;
      break;
      // ---------------------------------
    default: break;
    }
  
    TRACE(3,L"State actions completed. units="+util::longdouble2wstring(st->units));
  }


  ///////////////////////////////////////////////////////////////
  ///   Set the appropriate lemma and tag for the 
  ///   new multiword.
  ///////////////////////////////////////////////////////////////

  void numbers_default::SetMultiwordAnalysis(sentence::iterator i, int fstate, const numbers_status *st) const {

    wstring lemma;

    // Setting the analysis for the nummerical expression
    if (st->iscode==CODE) {
      lemma=i->get_form();
    }
    else {
      // compute nummerical value as lemma
      lemma=util::longdouble2wstring(st->units);
    }

    i->set_analysis(analysis(lemma,L"Z"));
    TRACE(3,L"Analysis set to: "+lemma+L" Z");

    // record this word was analyzed by this module
    i->set_analyzed_by(word::NUMBERS);    
  }

} // namespace
