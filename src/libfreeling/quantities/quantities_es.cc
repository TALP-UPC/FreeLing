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
  //   Spanish currency and ratio recognizer.      //
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
  ///  Create a quantities recognizer for Spanish.
  ///////////////////////////////////////////////////////////////

  quantities_es::quantities_es(const std::wstring &quantFile): quantities_module()
  {  
    // Initializing tokens hash
    tok.insert(make_pair(L"de",TK_wde));      
    tok.insert(make_pair(L"cada",TK_wcada));  
    tok.insert(make_pair(L"por",TK_wpor));    
    tok.insert(make_pair(L"sobre",TK_wsobre));
    tok.insert(make_pair(L"entre",TK_wsobre));
    tok.insert(make_pair(L"%",TK_pc));        
    tok.insert(make_pair(L"parte",TK_part));  
    tok.insert(make_pair(L"partes",TK_part)); 

    // es
    fract.insert(make_pair(L"medio",2));
    fract.insert(make_pair(L"tercio",3));
    fract.insert(make_pair(L"tercero",3));
    fract.insert(make_pair(L"cuarto",4));
    fract.insert(make_pair(L"quinto",5));
    fract.insert(make_pair(L"sexto",6));  fract.insert(make_pair(L"seisavo",6));
    fract.insert(make_pair(L"séptimo",7));
    fract.insert(make_pair(L"octavo",8));  fract.insert(make_pair(L"ochavo",8));
    fract.insert(make_pair(L"noveno",9));

    fract.insert(make_pair(L"décimo",10));         fract.insert(make_pair(L"veinteavo",20));
    fract.insert(make_pair(L"onceavo",11));        fract.insert(make_pair(L"veintiunavo",21));
    fract.insert(make_pair(L"doceavo",12));        fract.insert(make_pair(L"veintidosavo",22));
    fract.insert(make_pair(L"treceavo",13));       fract.insert(make_pair(L"veintitresavo",23));
    fract.insert(make_pair(L"catorceavo",14));     fract.insert(make_pair(L"veinticuatroavo",24));
    fract.insert(make_pair(L"quinceavo",15));      fract.insert(make_pair(L"veinticincoavo",25));
    fract.insert(make_pair(L"deciseisavo",16));    fract.insert(make_pair(L"veintiseisavo",26));
    fract.insert(make_pair(L"diecisieteavo",17));  fract.insert(make_pair(L"veintisieteavo",27));
    fract.insert(make_pair(L"dieciochoavo",18));   fract.insert(make_pair(L"veintiochoavo",28));
    fract.insert(make_pair(L"diecinueveavo",19));  fract.insert(make_pair(L"veintinueveavo",29));

    fract.insert(make_pair(L"treintavo",30));         fract.insert(make_pair(L"cuarentavo",40));
    fract.insert(make_pair(L"treintaiunavo",31));     fract.insert(make_pair(L"cuarentaiunavo",41));
    fract.insert(make_pair(L"treintaidosavo",32));    fract.insert(make_pair(L"cuarentaidosavo",42));
    fract.insert(make_pair(L"treintaitresavo",33));   fract.insert(make_pair(L"cuarentaitresavo",43));
    fract.insert(make_pair(L"treintaicuatroavo",34)); fract.insert(make_pair(L"cuarentaicuatroavo",44));
    fract.insert(make_pair(L"treintaicincoavo",35));  fract.insert(make_pair(L"cuarentaicincoavo",45));
    fract.insert(make_pair(L"treintaiseisavo",36));   fract.insert(make_pair(L"cuarentaiseisavo",46));
    fract.insert(make_pair(L"treintaisieteavo",37));  fract.insert(make_pair(L"cuarentaisieteavo",47));
    fract.insert(make_pair(L"treintaiochoavo",38));   fract.insert(make_pair(L"cuarentaiochoavo",48));
    fract.insert(make_pair(L"treintainueveavo",39));  fract.insert(make_pair(L"cuarentainueveavo",49));

    fract.insert(make_pair(L"cincuentavo",50));         fract.insert(make_pair(L"sesentavo",60));
    fract.insert(make_pair(L"cincuentaiunavo",51));     fract.insert(make_pair(L"sesentaiunavo",61));
    fract.insert(make_pair(L"cincuentaidosavo",52));    fract.insert(make_pair(L"sesentaidosavo",62));
    fract.insert(make_pair(L"cincuentaitresavo",53));   fract.insert(make_pair(L"sesentaitresavo",63));
    fract.insert(make_pair(L"cincuentaicuatroavo",54)); fract.insert(make_pair(L"sesentaicuatroavo",64));
    fract.insert(make_pair(L"cincuentaicincoavo",55));  fract.insert(make_pair(L"sesentaicincoavo",65));
    fract.insert(make_pair(L"cincuentaiseisavo",56));   fract.insert(make_pair(L"sesentaiseisavo",66));
    fract.insert(make_pair(L"cincuentaisieteavo",57));  fract.insert(make_pair(L"sesentaisieteavo",67));
    fract.insert(make_pair(L"cincuentaiochoavo",58));   fract.insert(make_pair(L"sesentaiochoavo",68));
    fract.insert(make_pair(L"cincuentainueveavo",59));  fract.insert(make_pair(L"sesentainueveavo",69));

    fract.insert(make_pair(L"setentavo",70));          fract.insert(make_pair(L"ochentavo",80));
    fract.insert(make_pair(L"setentaiunavo",71));      fract.insert(make_pair(L"ochentaiunavo",81));
    fract.insert(make_pair(L"setentaidosavo",72));     fract.insert(make_pair(L"ochentaidosavo",82));
    fract.insert(make_pair(L"setentaitresavo",73));    fract.insert(make_pair(L"ochentaitresavo",83));
    fract.insert(make_pair(L"setentaicuatroavo",74));  fract.insert(make_pair(L"ochentaicuatroavo",84));
    fract.insert(make_pair(L"setentaicincoavo",75));   fract.insert(make_pair(L"ochentaicincoavo",85));
    fract.insert(make_pair(L"setentaiseisavo",76));    fract.insert(make_pair(L"ochentaiseisavo",86));
    fract.insert(make_pair(L"setentaisieteavo",77));   fract.insert(make_pair(L"ochentaisieteavo",87));
    fract.insert(make_pair(L"setentaiochoavo",78));    fract.insert(make_pair(L"ochentaiochoavo",88));
    fract.insert(make_pair(L"setentainueveavo",79));   fract.insert(make_pair(L"ochentainueveavo",89));

    fract.insert(make_pair(L"noventavo",90));          fract.insert(make_pair(L"centésimo",100));
    fract.insert(make_pair(L"noventaiunavo",91));      fract.insert(make_pair(L"milésimo",1000));
    fract.insert(make_pair(L"noventaidosavo",92));     fract.insert(make_pair(L"diezmilésimo",10000));
    fract.insert(make_pair(L"noventaitresavo",93));    fract.insert(make_pair(L"cienmilésimo",100000));
    fract.insert(make_pair(L"noventaicuatroavo",94));  fract.insert(make_pair(L"millonésimo",1000000));
    fract.insert(make_pair(L"noventaicincoavo",95));   fract.insert(make_pair(L"diezmillonésimo",10000000));
    fract.insert(make_pair(L"noventaiseisavo",96));    fract.insert(make_pair(L"cienmillonésimo",100000000));
    fract.insert(make_pair(L"noventaisieteavo",97));   fract.insert(make_pair(L"milmillonésimo",1000000000));
    fract.insert(make_pair(L"noventaiochoavo",98));    
    fract.insert(make_pair(L"noventainueveavo",99));   
                                                    

    fract.insert(make_pair(L"doscientosavo",200));
    fract.insert(make_pair(L"trescientosavo",300));
    fract.insert(make_pair(L"cuatrocientosavo",400));
    fract.insert(make_pair(L"quinientosavo",500));
    fract.insert(make_pair(L"seiscientosavo",600));
    fract.insert(make_pair(L"sietecientosavo",700));
    fract.insert(make_pair(L"ochocientosavo",800));
    fract.insert(make_pair(L"novecientosavo",900));

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

  int quantities_es::ComputeToken(int state, sentence::iterator &j, sentence &se) const {

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

    TRACE(3,L"token computed: "+util::int2wstring(token));
    return(token);
  }

  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state with an informative 
  //   token (number, currency name) store the relevant info.
  ///////////////////////////////////////////////////////////////

  void quantities_es::StateActions(int origin, int state, int token, sentence::const_iterator j, quantities_status *st) const {

    wstring form = j->get_lc_form();
    wstring lema = j->get_lemma();
    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ("+form+L","+lema+L")");

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
