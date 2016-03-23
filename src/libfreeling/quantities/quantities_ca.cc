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
  //   Catalan currency and ratio recognizer.      //
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
#define ST_F 6 // got "de cada" after a number
#define ST_G 7 // got a measure unit or a currency name after a number
  // stop state
#define ST_STOP 8

  // Token codes
#define TK_number   1 // number
#define TK_n100     2 // number with value 100 or 1000 
#define TK_pc       3 // sign "%"
#define TK_wde      4 // word "de"
#define TK_wcada    5 // word "cada"
#define TK_wpor     6 // word "por"
#define TK_wsobre   7 // word "sobre" "entre"
#define TK_avo      8 // fraction word (tercio, veinteavo, milesima)
#define TK_part     9 // word "parte" or "partes"
#define TK_unit    10 // measure unit or currency name
#define TK_other   11

  ///////////////////////////////////////////////////////////////
  ///  Create a quantities recognizer for Catalan.
  ///////////////////////////////////////////////////////////////

  quantities_ca::quantities_ca(const std::wstring &quantFile): quantities_module()
  {  
    // Initializing tokens hash
    tok.insert(make_pair(L"de",TK_wde));     
    tok.insert(make_pair(L"cada",TK_wcada)); 
    tok.insert(make_pair(L"per",TK_wpor));   
    tok.insert(make_pair(L"sobre",TK_wsobre));
    tok.insert(make_pair(L"entre",TK_wsobre));
    tok.insert(make_pair(L"%",TK_pc));        
    tok.insert(make_pair(L"part",TK_part));   
    tok.insert(make_pair(L"parts",TK_part));  

    // ca
    fract.insert(make_pair(L"mig",2));
    fract.insert(make_pair(L"terç",3));
    fract.insert(make_pair(L"quart",4));
    fract.insert(make_pair(L"cinquè",5));
    fract.insert(make_pair(L"sisè",6));
    fract.insert(make_pair(L"setè",7));
    fract.insert(make_pair(L"vuitè",8));
    fract.insert(make_pair(L"novè",9));

    fract.insert(make_pair(L"desè",10));      fract.insert(make_pair(L"vintè",20));
    fract.insert(make_pair(L"onzè",11));      fract.insert(make_pair(L"vint-i-unè",21));
    fract.insert(make_pair(L"dotzè",12));     fract.insert(make_pair(L"vint-i-dosè",22));
    fract.insert(make_pair(L"tretzè",13));    fract.insert(make_pair(L"vint-i-tresè",23));
    fract.insert(make_pair(L"catorzè",14));   fract.insert(make_pair(L"vint-i-quatrè",24));
    fract.insert(make_pair(L"quinzè",15));    fract.insert(make_pair(L"vint-i-cinquè",25));
    fract.insert(make_pair(L"setzè",15));     fract.insert(make_pair(L"vint-i-sisè",26));
    fract.insert(make_pair(L"dissetè",17));   fract.insert(make_pair(L"vint-i-setè",27));
    fract.insert(make_pair(L"divuitè",18));   fract.insert(make_pair(L"vint-i-vuitè",28));
    fract.insert(make_pair(L"dinovè",19));    fract.insert(make_pair(L"vint-i-novè",29));

    fract.insert(make_pair(L"trentè",30));         fract.insert(make_pair(L"quarantè",40));
    fract.insert(make_pair(L"trenta-unè",31));     fract.insert(make_pair(L"quaranta-unè",41));
    fract.insert(make_pair(L"trenta-dosè",32));    fract.insert(make_pair(L"quaranta-dosè",42));
    fract.insert(make_pair(L"trenta-tresè",33));   fract.insert(make_pair(L"quaranta-tresè",43));
    fract.insert(make_pair(L"trenta-cuatrè",34));  fract.insert(make_pair(L"quaranta-quatrè",44));
    fract.insert(make_pair(L"trenta-cinquèo",35)); fract.insert(make_pair(L"quaranta-cinquè",45));
    fract.insert(make_pair(L"trenta-sisè",36));    fract.insert(make_pair(L"quaranta-sisè",46));
    fract.insert(make_pair(L"trenta-setè",37));    fract.insert(make_pair(L"quaranta-setè",47));
    fract.insert(make_pair(L"trenta-vuitè",38));   fract.insert(make_pair(L"quaranta-vuitè",48));
    fract.insert(make_pair(L"trenta-novè",39));    fract.insert(make_pair(L"quaranta-novè",49));

    fract.insert(make_pair(L"cinquantè",50));         fract.insert(make_pair(L"seixantè",60));
    fract.insert(make_pair(L"cinquanta-unè",51));     fract.insert(make_pair(L"seixanta-unè",61));
    fract.insert(make_pair(L"cinquanta-dosè",52));    fract.insert(make_pair(L"seixanta-dosè",62));
    fract.insert(make_pair(L"cinquanta-tresè",53));   fract.insert(make_pair(L"seixanta-tresè",63));
    fract.insert(make_pair(L"cinquanta-cuatrè",54));  fract.insert(make_pair(L"seixanta-quatrè",64));
    fract.insert(make_pair(L"cinquanta-cinquèo",55)); fract.insert(make_pair(L"seixanta-cinquè",65));
    fract.insert(make_pair(L"cinquanta-sisè",56));    fract.insert(make_pair(L"seixanta-sisè",66));
    fract.insert(make_pair(L"cinquanta-setè",57));    fract.insert(make_pair(L"seixanta-setè",67));
    fract.insert(make_pair(L"cinquanta-vuitè",58));   fract.insert(make_pair(L"seixanta-vuitè",68));
    fract.insert(make_pair(L"cinquanta-novè",59));    fract.insert(make_pair(L"seixanta-novè",69));
 
    fract.insert(make_pair(L"setantè",70));         fract.insert(make_pair(L"vuitantè",80));
    fract.insert(make_pair(L"setanta-unè",71));     fract.insert(make_pair(L"vuitanta-unè",81));
    fract.insert(make_pair(L"setanta-dosè",72));    fract.insert(make_pair(L"vuitanta-dosè",82));
    fract.insert(make_pair(L"setanta-tresè",73));   fract.insert(make_pair(L"vuitanta-tresè",83));
    fract.insert(make_pair(L"setanta-cuatrè",74));  fract.insert(make_pair(L"vuitanta-quatrè",84));
    fract.insert(make_pair(L"setanta-cinquèo",75)); fract.insert(make_pair(L"vuitanta-cinquè",85));
    fract.insert(make_pair(L"setanta-sisè",76));    fract.insert(make_pair(L"vuitanta-sisè",86));
    fract.insert(make_pair(L"setanta-setè",77));    fract.insert(make_pair(L"vuitanta-setè",87));
    fract.insert(make_pair(L"setanta-vuitè",78));   fract.insert(make_pair(L"vuitanta-vuitè",88));
    fract.insert(make_pair(L"setanta-novè",79));    fract.insert(make_pair(L"vuitanta-novè",89));

    fract.insert(make_pair(L"norantè",90));          fract.insert(make_pair(L"centèssim",100));
    fract.insert(make_pair(L"noranta-unè",91));      fract.insert(make_pair(L"milèssim",1000));
    fract.insert(make_pair(L"noranta-dosè",92));     fract.insert(make_pair(L"deumilèssim",10000));
    fract.insert(make_pair(L"noranta-tresè",93));    fract.insert(make_pair(L"centmilèssim",100000));
    fract.insert(make_pair(L"noranta-quatrè",94));  fract.insert(make_pair(L"mil·lionèssim",1000000));
    fract.insert(make_pair(L"noranta-cinquè",95));   fract.insert(make_pair(L"deumil·lionèssim",10000000));
    fract.insert(make_pair(L"noranta-sisè",96));    fract.insert(make_pair(L"centmil·lionèssim",100000000));
    fract.insert(make_pair(L"noranta-setè",97));   fract.insert(make_pair(L"milmil·lionèssim",1000000000));
    fract.insert(make_pair(L"noranta-vuitè",98));
    fract.insert(make_pair(L"noranta-novè",99));

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
    trans[ST_B][TK_pc]=ST_C;      trans[ST_B][TK_wpor]=ST_D;  trans[ST_B][TK_wde]=ST_E;
    trans[ST_B][TK_wsobre]=ST_F;  trans[ST_B][TK_avo]=ST_C;   trans[ST_B][TK_unit]=ST_G;
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

    // Load configuration file
    readConfig(quantFile);

    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int quantities_ca::ComputeToken(int state, sentence::iterator &j, sentence &se) const {

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

  void quantities_ca::StateActions(int origin, int state, int token, sentence::const_iterator j, quantities_status *st) const {

    wstring form = j->get_lc_form();
    wstring lema = j->get_lemma();
    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ["+form+L"]");

    // get token numerical value, if any
    wstring value=L"";
    if ((token==TK_number || token==TK_n100) && j->get_n_analysis() && j->get_tag()[0]==L'Z') 
      value = lema;

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
    case ST_G:
      // number + measure unit (or currency name) found, store magnitude and unit
      TRACE(3,L"Actions for state I");
      st->unitCode=units.find(lema)->second+L"_"+lema;
      st->unitType=units.find(lema)->second;
      break;
      // ---------------------------------
    default: break;
    }
  }

} // namespace
