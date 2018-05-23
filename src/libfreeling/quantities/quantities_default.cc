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
#include "freeling/morfo/configfile.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"QUANTITIES"
#define MOD_TRACECODE QUANT_TRACE

  ///////////////////////////////////////////////////////////////
  ///  Abstract class constructor.
  ///////////////////////////////////////////////////////////////

  quantities_module::quantities_module(): automat<quantities_status>(), measures(L"") {}

  ///////////////////////////////////////////////////////////////
  ///  Load a configuration file
  ///////////////////////////////////////////////////////////////

  void quantities_module::readConfig(const std::wstring &quantFile) {

    // opening quantities config file
    enum sections {CURRENCY, MEASURE, MEASURE_NAMES};
    config_file cfg;
    cfg.add_section(L"Currency",CURRENCY);
    cfg.add_section(L"Measure",MEASURE);
    cfg.add_section(L"MeasureNames",MEASURE_NAMES);

    if (not cfg.open(quantFile))
      ERROR_CRASH(L"Error opening file "+quantFile);

    // process each content line according to the section where it is found
    wstring line;
    while (cfg.get_content_line(line))  {
      wistringstream sin;
      sin.str(line);

      // process line depending on current section
      switch (cfg.get_section()) { 
      
      case CURRENCY: {    // reading currency symbol
        currency_key = line;
        break;
      }

      case MEASURE: {     // reading Measure units
        wstring key,unit;
        sin >> unit >> key;
        units.insert(make_pair(key,unit));
        break;
      }
     
      case MEASURE_NAMES: { // reading multiwords expressing measures
        measures.add_locution(line);
        break;
      }
      
      default: break; 
      }
    }

    cfg.close(); 
  }
  

  ///////////////////////////////////////////////////////////////
  ///   Reset acumulators used by state actions:
  ///   value1, value2 (for ratios).
  ///////////////////////////////////////////////////////////////

  void quantities_module::ResetActions(quantities_status *st) const {
    TRACE(3,L"Resetting state actions.");
    st->value1=L""; st->value2=L"";
    st->unitCode=L"";
    st->unitType=L"";
  }

  ///////////////////////////////////////////////////////////////
  ///   Set the appropriate lemma and tag for the new multiword.
  ///////////////////////////////////////////////////////////////

  void quantities_module::SetMultiwordAnalysis(sentence::iterator i, int fstate, const quantities_status *st) const {
    wstring lemma,tag;

    TRACE(3,L"Setting analysis for new MW");
    // Setting the lemma 
    if (currency_key!=L"" and st->unitType==currency_key) {
      lemma=st->unitCode+L":"+st->value1;
      tag=L"Zm";
    }
    else if (st->unitCode!=L"") {
      // it's a quantity + measure unit
      lemma=st->unitCode+L":"+st->value1;
      tag=L"Zu";
    }
    else {
      // it's a ratio
      lemma=st->value1+L"/"+st->value2;
      tag=L"Zp";
    }

    i->set_analysis(analysis(lemma,tag));   
    TRACE(3,L"Analysis set to: "+lemma+L" "+tag);

    // record this word was analyzed by this module
    i->set_analyzed_by(word::QUANTITIES);    
    // mark analysis as definitive
    i->lock_analysis();
  }


  ///////////////////////////////////////////////////
  //-----------------------------------------------//
  //   Default currency and ratio recognizer.      //
  //   Only recognize simple stuff such as "20%"   //
  //-----------------------------------------------//

  // State codes 
  // states for a currency amount, percentage or ratio expression
#define ST_A 1 // initial state
#define ST_B 2 // got a number (a quantity)
#define ST_C 3 // got "20 %"  (VALID PERCENTAGE)
#define ST_G 4 // got a measure unit or a currency name after a number
  // stop state
#define ST_STOP 5

  // Token codes
#define TK_number  1 // number
#define TK_pc      2 // sign "%"
#define TK_unit    3 // measure unit or currency name (km, g/m2, m/s, australian dollar, ...)
#define TK_other   4

  ///////////////////////////////////////////////////////////////
  ///  Create a default quantities recognizer.
  ///////////////////////////////////////////////////////////////

  quantities_default::quantities_default(const std::wstring &quantFile): quantities_module() {
    // Initializing tokens hash
    tok.insert(make_pair(L"%",TK_pc));

    // Initialize special state attributes
    initialState=ST_A; stopState=ST_STOP;

    // Initialize Final state set
    Final.insert(ST_C);  Final.insert(ST_G); 

    // Initialize transitions table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;
  
    // State A
    trans[ST_A][TK_number]=ST_B;
    // State B
    trans[ST_B][TK_pc]=ST_C;  trans[ST_B][TK_unit]=ST_G;
    // State C
    // nothing else expected from here.

    // Load measure units file, if any
    if (not quantFile.empty()) readConfig(quantFile);

    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int quantities_default::ComputeToken(int state, sentence::iterator &j, sentence &se) const {

    // look for token in table
    wstring form = j->get_lc_form();
    int token = TK_other;
    map<wstring,int>::const_iterator im = tok.find(form);
    if (im!=tok.end()) token = (*im).second;  
    TRACE(3,L"Next word form is: ["+form+L"] token="+util::int2wstring(token));     

    // if the token was in the table, we're done
    if (token != TK_other) return (token);

    // Token not found in translation table, let's have a closer look.

    // check to see if it is a number
    if (j->get_n_analysis() && j->get_tag()[0]==L'Z') {
      token = TK_number;
    }

    switch (state) {
    case ST_B:
      // in state B a currency name or a measure unit may come
      if (measures.matching(se,j))
        token=TK_unit;
      break;
      // ---------------------------------
    default: break;
    }


    return(token);
  }


  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state with an informative 
  //   token (number, currency name) store the relevant info.
  ///////////////////////////////////////////////////////////////

  void quantities_default::StateActions(int origin, int state, int token, sentence::const_iterator j, quantities_status *st) const {

    wstring form = j->get_lc_form();
    wstring lema = j->get_lemma();
    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ("+form+L","+lema+L")");

    // get token numerical value, if any
    wstring value=L"";
    if (token==TK_number &&  j->get_n_analysis() && j->get_tag()[0]==L'Z') 
      value = j->get_lemma();

    // State actions
    switch (state) {
      // ---------------------------------
    case ST_B:
      // a nummerical value has been found. Store it just in case.
      TRACE(3,L"Actions for state B");
      st->value1=value;
      break;
      // ---------------------------------
    case ST_C:
      // complete ratio/percentage found
      TRACE(3,L"Actions for state C");
      if (token==TK_pc) 
        st->value2=L"100";  // percent sign "%", second value of ratio is 100
      break;
      // ---------------------------------
    case ST_G: {
      // number + measure unit (or currency name) found, store magnitude and unit
      TRACE(3,L"Actions for state G");
      map<wstring,wstring>::const_iterator p = units.find(lema);
      if (p != units.end()) {
        st->unitCode = p->second+L"_"+lema;
        st->unitType = p->second;
      }
      else {
        st->unitCode = L"??_"+lema;
        st->unitType = L"??";
      }
      break;
    }

      // ---------------------------------
    default: break;
    }
  }


} // namespace
