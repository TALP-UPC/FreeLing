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

#include <fstream>
#include <sstream>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/quantities_modules.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"QUANTITIES"
#define MOD_TRACECODE QUANT_TRACE

  //-----------------------------------------------------------//
  //   Russian currency and ratio recognizer.                  //
  //   The same structure is used, different lexical units     //
  //   obtained from configuration files, or mixed in the data //
  //-----------------------------------------------------------//

  // State codes 
  // states for a currency amount, percentage or ratio expression
#define ST_A 1
#define ST_B 2
#define ST_C 3
  // stop state
#define ST_STOP 8

  // Token codes
#define TK_number   1 // number
#define TK_pc       3 // sign "%"
#define TK_unit    11 // measure unit or currency name

#define TK_other   20


  ///////////////////////////////////////////////////////////////
  ///  Create a quantities recognizer for Russian.
  ///////////////////////////////////////////////////////////////

  quantities_ru::quantities_ru(const std::wstring &quantFile): quantities_module()
  {  
    // Initialize special state attributes
    initialState=ST_A; stopState=ST_STOP;

    // Initialize Final state set 
    Final.insert(ST_C);

    // Initialize transitions table. By default, stop state
    for(int s=0; s<MAX_STATES; ++s) 
      for(int t=0; t<MAX_TOKENS; ++t) 
        trans[s][t]=ST_STOP;
  
    // State A
    trans[ST_A][TK_number]=ST_B;
    // State B
    trans[ST_B][TK_unit]=ST_C;

    // Load configuration file
    readConfig(quantFile);

    TRACE(3, L"analyzer succesfully created");
  }

  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int quantities_ru::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
    int token = TK_other;
    wstring form = j->get_lc_form();
  
    map<wstring, int>::const_iterator im = tok.find(form);
    if (im!=tok.end()) {
      token = (*im).second;
      TRACE(3, L"Next word form is: ["+form+L"] token="+util::int2wstring(token)); 
    }
    else {
      // Token not found in translation table, let's have a closer look.
      // check to see if it is a number
    
      if (j->get_n_analysis() && j->get_tag()[0]==L'Z') 
        token = TK_number;
      TRACE(3, L"checked");
    
      switch (state) 
        {
        case ST_B:
          if (measures.matching(se, j))
            token=TK_unit;
          break;
      
        default: break;
        }
    }
    return token;
  }

  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state with an informative 
  //   token (number, currency name) store the relevant info.
  ///////////////////////////////////////////////////////////////

  void quantities_ru::StateActions(int origin, int state, int token, sentence::const_iterator j, quantities_status *st) const {

    wstring form = j->get_lc_form();
    wstring lema = j->get_lemma();
    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ["+form+L"]");
  
    // get token numerical value, if any
    wstring value=L"";
    if ((token==TK_number) && j->get_n_analysis() && j->get_tag()[0]==L'Z') 
      value = lema;

    // State actions
    switch (state) {
    case ST_B:
      TRACE(3,L"Actions for state B");
      st->value1=value;
      break;
       
    case ST_C:
      TRACE(3,L"Actions for state C");
      if (token==TK_unit) {
        TRACE(3,L"Actions for state I");
        st->unitCode=units.find(lema)->second+L"_"+lema;
        st->unitType=units.find(lema)->second;
      }
      break;

    default: break;
    }
  }
} // namespace
