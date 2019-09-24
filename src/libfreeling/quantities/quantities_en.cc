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
  //   English currency and ratio recognizer.      //
  //   The same structure is used, different lexical units     //
  //   obtained from configuration files, or mixed in the data //
  //-----------------------------------------------------------//

  // State codes 
  // states for a currency amount, percentage or ratio expression
#define ST_A 1 // initial state
#define ST_B 2 // got a number (a quantity)
#define ST_C 3 // got "20 %", "20 por ciento", "2 de cada 3", "4 sobre 5", "2 tercios"  (VALID PERCENTAGE)
#define ST_D 4 // got "por" after a number 
#define ST_E 5 // got "de" after a number
#define ST_F 6 // got "of each" or "out of" after a number
#define ST_G 7 // got a measure unit or a currency name after a number
#define ST_H 8 // got word "out" after a number
  // stop state
#define ST_STOP 9

  // Token codes
#define TK_number   1 // number
#define TK_n100     2 // number with value 100 or 1000 
#define TK_pc       3 // sign "%"
#define TK_wout     4 // word "out"
#define TK_wde      5 // word "of"
#define TK_wcada    6 // word "each"
#define TK_wpor     7 // word "per"
#define TK_wsobre   8 // word "between"
#define TK_avo      9 // fraction word (tenth, half)
#define TK_part    10 // word "part" or "parts"
#define TK_unit    11 // measure unit or currency name
#define TK_curr    12 // complete currency amount (e.g. $10,000)
#define TK_other   13

  ///////////////////////////////////////////////////////////////
  ///  Create a quantities recognizer for Spanish.
  ///////////////////////////////////////////////////////////////

  quantities_en::quantities_en(const std::wstring &quantFile): quantities_module()
  {  
    // Initializing tokens hash
    tok.insert(make_pair(L"out",TK_wout));      
    tok.insert(make_pair(L"of",TK_wde));      
    tok.insert(make_pair(L"each",TK_wcada));  
    tok.insert(make_pair(L"per",TK_wpor));    
    tok.insert(make_pair(L"between",TK_wsobre));
    tok.insert(make_pair(L"%",TK_pc));        
    tok.insert(make_pair(L"part",TK_part));  
    tok.insert(make_pair(L"parts",TK_part)); 

    // es
    fract.insert(make_pair(L"half",2));
    fract.insert(make_pair(L"third",3));
    fract.insert(make_pair(L"fourth",4));
    fract.insert(make_pair(L"quarter",4));
    fract.insert(make_pair(L"fifth",5));
    fract.insert(make_pair(L"sixth",6));
    fract.insert(make_pair(L"seventh",7));
    fract.insert(make_pair(L"eighth",8));
    fract.insert(make_pair(L"ninth",9));

    fract.insert(make_pair(L"tenth",10));       fract.insert(make_pair(L"twentieth",20));
    fract.insert(make_pair(L"eleventh",11));    fract.insert(make_pair(L"twenty-first",21));
    fract.insert(make_pair(L"twelveth",12));    fract.insert(make_pair(L"twenty-second",22));
    fract.insert(make_pair(L"thirteenth",13));  fract.insert(make_pair(L"twenty-third",23));
    fract.insert(make_pair(L"fourteenth",14));  fract.insert(make_pair(L"twenty-fourth",24));
    fract.insert(make_pair(L"fifteenth",15));   fract.insert(make_pair(L"twenty-fifth",25));
    fract.insert(make_pair(L"sixteenth",16));   fract.insert(make_pair(L"twenty-sixth",26));
    fract.insert(make_pair(L"seventeenth",17)); fract.insert(make_pair(L"twenty-seventh",27));
    fract.insert(make_pair(L"eighteenth",18));  fract.insert(make_pair(L"twenty-eighth",28));
    fract.insert(make_pair(L"nineteenth",19));  fract.insert(make_pair(L"twenty-nineth",29));

    fract.insert(make_pair(L"thirtieth",30));      fract.insert(make_pair(L"fortieth",40));
    fract.insert(make_pair(L"thirty-first",31));   fract.insert(make_pair(L"forty-first",41));
    fract.insert(make_pair(L"thirty-second",32));  fract.insert(make_pair(L"forty-second",42));
    fract.insert(make_pair(L"thirty-third",33));   fract.insert(make_pair(L"forty-third",43));
    fract.insert(make_pair(L"thirty-fourth",34));  fract.insert(make_pair(L"forty-fourth",44));
    fract.insert(make_pair(L"thirty-fifth",35));   fract.insert(make_pair(L"forty-fifth",45));
    fract.insert(make_pair(L"thirty-sixth",36));   fract.insert(make_pair(L"forty-sixth",46));
    fract.insert(make_pair(L"thirty-seventh",37)); fract.insert(make_pair(L"forty-seventh",47));
    fract.insert(make_pair(L"thirty-eighth",38));  fract.insert(make_pair(L"forty-eighth",48));
    fract.insert(make_pair(L"thirty-nineth",39));  fract.insert(make_pair(L"forty-nineth",49));

    fract.insert(make_pair(L"fiftieth",50));      fract.insert(make_pair(L"sixtieth",60));
    fract.insert(make_pair(L"fifty-first",51));   fract.insert(make_pair(L"sixty-first",61));
    fract.insert(make_pair(L"fifty-second",52));  fract.insert(make_pair(L"sixty-second",62));
    fract.insert(make_pair(L"fifty-third",53));   fract.insert(make_pair(L"sixty-third",63));
    fract.insert(make_pair(L"fifty-fourth",54));  fract.insert(make_pair(L"sixty-fourth",64));
    fract.insert(make_pair(L"fifty-fifth",55));   fract.insert(make_pair(L"sixty-fifth",65));
    fract.insert(make_pair(L"fifty-sixth",56));   fract.insert(make_pair(L"sixty-sixth",66));
    fract.insert(make_pair(L"fifty-seventh",57)); fract.insert(make_pair(L"sixty-seventh",67));
    fract.insert(make_pair(L"fifty-eighth",58));  fract.insert(make_pair(L"sixty-eighth",68));
    fract.insert(make_pair(L"fifty-nineth",59));  fract.insert(make_pair(L"sixty-nineth",69));

    fract.insert(make_pair(L"seventieth",70));      fract.insert(make_pair(L"eightieth",80));
    fract.insert(make_pair(L"seventy-first",71));   fract.insert(make_pair(L"eighty-first",81));
    fract.insert(make_pair(L"seventy-second",72));  fract.insert(make_pair(L"eighty-second",82));
    fract.insert(make_pair(L"seventy-third",73));   fract.insert(make_pair(L"eighty-third",83));
    fract.insert(make_pair(L"seventy-fourth",74));  fract.insert(make_pair(L"eighty-fourth",84));
    fract.insert(make_pair(L"seventy-fifth",75));   fract.insert(make_pair(L"eighty-fifth",85));
    fract.insert(make_pair(L"seventy-sixth",76));   fract.insert(make_pair(L"eighty-sixth",86));
    fract.insert(make_pair(L"seventy-seventh",77)); fract.insert(make_pair(L"eighty-seventh",87));
    fract.insert(make_pair(L"seventy-eighth",78));  fract.insert(make_pair(L"eighty-eighth",88));
    fract.insert(make_pair(L"seventy-nineth",79));  fract.insert(make_pair(L"eighty-nineth",89));

    fract.insert(make_pair(L"ninetieth",90));       fract.insert(make_pair(L"hundredth",100));
    fract.insert(make_pair(L"ninety-first",91));    fract.insert(make_pair(L"thousandth",1000));
    fract.insert(make_pair(L"ninety-second",92));   fract.insert(make_pair(L"ten-thousandth",10000));
    fract.insert(make_pair(L"ninety-third",93));    fract.insert(make_pair(L"one-hundred-thousandth",100000));
    fract.insert(make_pair(L"ninety-fourth",94));   fract.insert(make_pair(L"one-millionth",1000000));
    fract.insert(make_pair(L"ninety-fifth",95));    fract.insert(make_pair(L"ten-millionth",10000000));
    fract.insert(make_pair(L"ninety-sixth",96));    fract.insert(make_pair(L"one-hundred-millionth",100000000));
    fract.insert(make_pair(L"ninety-seventh",97));  fract.insert(make_pair(L"one-billionth",1000000000));
    fract.insert(make_pair(L"ninety-eighth",98));   
    fract.insert(make_pair(L"ninety-nineth",99));   

    // Initialize special state attributes
    initialState=ST_A; stopState=ST_STOP;

    // Initialize Final state set 
    Final.insert(ST_C);  Final.insert(ST_G); 

    // Initialize transitions table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;
  
    // State A
    trans[ST_A][TK_number]=ST_B;
    trans[ST_A][TK_curr]=ST_G;
    // State B
    trans[ST_B][TK_pc]=ST_C;      trans[ST_B][TK_wpor]=ST_D;  trans[ST_B][TK_wde]=ST_E;
    trans[ST_B][TK_wsobre]=ST_F;  trans[ST_B][TK_avo]=ST_C;   trans[ST_B][TK_unit]=ST_G;
    trans[ST_B][TK_wout]=ST_H;
    // State C
    trans[ST_C][TK_part]=ST_C;
    // State D
    trans[ST_D][TK_n100]=ST_C;
    // State E
    trans[ST_E][TK_wcada]=ST_F;  trans[ST_E][TK_unit]=ST_G;
    // State F
    trans[ST_F][TK_number]=ST_C;
    // State G
    // nothing else expected from here.
    // State H
    trans[ST_H][TK_wde]=ST_F;

    // Load configuration file
    readConfig(quantFile);

    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int quantities_en::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
 
    // look for token in table
    wstring form = j->get_lc_form();  
    int token = TK_other;
    map<wstring,int>::const_iterator im = tok.find(form);
    if (im!=tok.end()) token = (*im).second;  
    TRACE(3,L"Next word form is: ["+form+L"] token="+util::int2wstring(token)); 

    // if the token was in the table, we're done
    if (token != TK_other) return (token);

    // Token not found in translation table, let's have a closer look.

    // check to see if it is a currency (e.g $10,000)
    if (j->get_n_analysis() && j->get_tag()[0]==L'Z' &&
	CurrencySymbols.find(j->get_form().substr(0,1))!=CurrencySymbols.end()) {
      token = TK_curr;
    }
    // check to see if it is a number
    else if (j->get_n_analysis() && j->get_tag()[0]==L'Z') {
      token = TK_number;
    }
    TRACE(3,L"checked");

    switch (state) {
      // ---------------------------------
    case ST_B:
      // in state B a currency name, a measure unit, or a fraction may come
      if (measures.matching(se,j))
        token=TK_unit;
      else {
        // look if any lemma is found in "fract" map
        word::iterator a;
        for (a=j->begin(); a!=j->end() && fract.find(a->get_lemma())==fract.end(); a++);
        // if any, consider this a fraction word (fifth, eleventh, ...) 
        if (a!=j->end()) {
          j->unselect_all_analysis();
          j->select_analysis(a);
          token=TK_avo;
        }
      }
      break;
    case ST_E: 
      // in state E a currency name or a measure unit may come
      if (measures.matching(se,j))
        token=TK_unit;
      break;
      // ---------------------------------
    case ST_D:
      // in state D only 100 and 1000 are allowed
      if (token==TK_number) {
        wstring value = j->get_lemma();
        if (value==L"100" || value==L"1000")
          token=TK_n100;
      }
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

  void quantities_en::StateActions(int origin, int state, int token, sentence::const_iterator j, quantities_status *st) const {

    wstring form = j->get_lc_form();
    wstring lema = j->get_lemma();
    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ["+form+L"]");

    // get token numerical value, if any
    wstring value=L"";
    if ((token==TK_number || token==TK_n100) && j->get_n_analysis() && j->get_tag()[0]==L'Z') {
      value = lema;
    }
    else if (token==TK_curr) {
      lema = CurrencySymbols.find(j->get_form().substr(0,1))->second;
      st->value1 = j->get_form().substr(1);
      // remove thousand points
      size_t i;
      while ((i=st->value1.find_first_of(L","))!=wstring::npos) {
        st->value1.erase(i,1);
      }
    }
    
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
      TRACE(3,L"more Actions for state C. token="+util::int2wstring(token));

      if (token==TK_pc)
        // percent sign "%", second value of ratio is 100
        st->value2=L"100";
      else if (token==TK_avo)
        // fraction word (e.g. "two thirds", "dos tercios"), second value is value of second word
        st->value2=util::longdouble2wstring(fract.find(lema)->second);
      else if (token!=TK_part)
        // normal ratio "3 de cada 4" "5 por mil", store second value.
        st->value2=value;
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
      TRACE(3,L"Final state. lemma="<<lema<<L" value="<<st->value1<<" type="<<st->unitType);
      break;
    }
      // ---------------------------------
    default: break;
    }
  }


} // namespace
