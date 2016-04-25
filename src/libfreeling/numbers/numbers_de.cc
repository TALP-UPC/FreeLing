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

//-------------------------------------------------------//
//       German number recognizer                        //
//       Johannes Heinecke (johannes.heinecke@orange.fr) //
//-------------------------------------------------------//


#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/numbers_modules.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"NUMBERS"
#define MOD_TRACECODE NUMBERS_TRACE
#define ST_START 0
#define ST_read_eins2 1
#define ST_read_hundert 2
#define ST_read_unit2 3
#define ST_read_und 4
#define ST_read_vierzig 5
#define ST_read_vierzehn 6
#define ST_read_tausend 7
#define ST_read_unit3 8
#define ST_read_hundert2 9
#define ST_read_unit4 10
#define ST_read_vierzehn2 11
#define ST_read_und2 12
#define ST_read_vierzig2 13
#define ST_read_eins 14
#define ST_read_digits 15
#define ST_STOP 49

#define TK_w_eins 0
#define TK_w_hundert 1
#define TK_w_tausend 2
#define TK_w_teens 3
#define TK_w_tens 4
#define TK_w_und 5
#define TK_w_unit 6
#define TK_num 7

#define TK_number 98
#define TK_other 99

  numbers_de::numbers_de(const std::wstring &dec, const std::wstring &thou): numbers_module(dec,thou) {
#ifdef DEDEBUG
  // for tracing only
  stateNames.insert(make_pair(ST_START, L"ST_START"));
  stateNames.insert(make_pair(ST_read_eins2, L"ST_read_eins2"));
  stateNames.insert(make_pair(ST_read_hundert, L"ST_read_hundert"));
  stateNames.insert(make_pair(ST_read_unit2, L"ST_read_unit2"));
  stateNames.insert(make_pair(ST_read_und, L"ST_read_und"));
  stateNames.insert(make_pair(ST_read_vierzig, L"ST_read_vierzig"));
  stateNames.insert(make_pair(ST_read_vierzehn, L"ST_read_vierzehn"));
  stateNames.insert(make_pair(ST_read_tausend, L"ST_read_tausend"));
  stateNames.insert(make_pair(ST_read_unit3, L"ST_read_unit3"));
  stateNames.insert(make_pair(ST_read_hundert2, L"ST_read_hundert2"));
  stateNames.insert(make_pair(ST_read_unit4, L"ST_read_unit4"));
  stateNames.insert(make_pair(ST_read_vierzehn2, L"ST_read_vierzehn2"));
  stateNames.insert(make_pair(ST_read_und2, L"ST_read_und2"));
  stateNames.insert(make_pair(ST_read_vierzig2, L"ST_read_vierzig2"));
  stateNames.insert(make_pair(ST_read_eins, L"ST_read_eins"));
  stateNames.insert(make_pair(ST_read_digits, L"ST_read_digits"));
  stateNames.insert(make_pair(ST_STOP, L"ST_STOP"));

  tokenNames.insert(make_pair(TK_w_eins, L"TK_w_eins"));
  tokenNames.insert(make_pair(TK_w_hundert, L"TK_w_hundert"));
  tokenNames.insert(make_pair(TK_w_tausend, L"TK_w_tausend"));
  tokenNames.insert(make_pair(TK_w_teens, L"TK_w_teens"));
  tokenNames.insert(make_pair(TK_w_tens, L"TK_w_tens"));
  tokenNames.insert(make_pair(TK_w_und, L"TK_w_und"));
  tokenNames.insert(make_pair(TK_w_unit, L"TK_w_unit"));
  tokenNames.insert(make_pair(TK_num, L"TK_num"));
  //  tokenNames.insert(make_pair(TK_number, L"TK_number"));
  tokenNames.insert(make_pair(TK_other, L"TK_other"));
#endif

  // Initialize special state attributes
  initialState=ST_START;
  stopState=ST_STOP;

  // Initialize token translation map (TODO: complete!)
  tok.insert(make_pair(L"eins", TK_w_eins));
  tok.insert(make_pair(L"hundert", TK_w_hundert));
  tok.insert(make_pair(L"einhundert", TK_w_hundert));
  tok.insert(make_pair(L"zweihundert", TK_w_hundert));
  tok.insert(make_pair(L"dreihundert", TK_w_hundert));
  tok.insert(make_pair(L"vierhundert", TK_w_hundert));
  tok.insert(make_pair(L"fünfhundert", TK_w_hundert));
  tok.insert(make_pair(L"sechshundert", TK_w_hundert));
  tok.insert(make_pair(L"siebenhundert", TK_w_hundert));
  tok.insert(make_pair(L"achthundert", TK_w_hundert));
  tok.insert(make_pair(L"neunhundert", TK_w_hundert));
  tok.insert(make_pair(L"tausend", TK_w_tausend));

  tok.insert(make_pair(L"elf", TK_w_teens));
  tok.insert(make_pair(L"zwölf", TK_w_teens));
  tok.insert(make_pair(L"dreizehn", TK_w_teens));
  tok.insert(make_pair(L"vierzehn", TK_w_teens));
  tok.insert(make_pair(L"fünfzehn", TK_w_teens));
  tok.insert(make_pair(L"sechzehn", TK_w_teens));
  tok.insert(make_pair(L"siebzehn", TK_w_teens));
  tok.insert(make_pair(L"achtzehn", TK_w_teens));
  tok.insert(make_pair(L"neunzehn", TK_w_teens));


  tok.insert(make_pair(L"zwanzig", TK_w_tens));
  tok.insert(make_pair(L"dreißig", TK_w_tens));
  tok.insert(make_pair(L"dreissig", TK_w_tens));
  tok.insert(make_pair(L"vierzig", TK_w_tens));
  tok.insert(make_pair(L"fünfzig", TK_w_tens));
  tok.insert(make_pair(L"sechzig", TK_w_tens));
  tok.insert(make_pair(L"siebzig", TK_w_tens));
  tok.insert(make_pair(L"achtzig", TK_w_tens));
  tok.insert(make_pair(L"neunnzig", TK_w_tens));


  tok.insert(make_pair(L"und", TK_w_und));

  tok.insert(make_pair(L"ein", TK_w_unit));
  tok.insert(make_pair(L"eine", TK_w_unit));
  tok.insert(make_pair(L"eins", TK_w_unit));
  tok.insert(make_pair(L"zwei", TK_w_unit));
  tok.insert(make_pair(L"drei", TK_w_unit));
  tok.insert(make_pair(L"vier", TK_w_unit));
  tok.insert(make_pair(L"fünf", TK_w_unit));
  tok.insert(make_pair(L"sechs", TK_w_unit));
  tok.insert(make_pair(L"sieben", TK_w_unit));
  tok.insert(make_pair(L"acht", TK_w_unit));
  tok.insert(make_pair(L"neun", TK_w_unit));
  tok.insert(make_pair(L"zehn", TK_w_unit));



  // initialize map with numeric values
  value.insert(make_pair(L"ein",1.0f));
  value.insert(make_pair(L"eine",1.0f));
  value.insert(make_pair(L"eins",1.0f));
  value.insert(make_pair(L"zwei",2.0f));
  value.insert(make_pair(L"drei", 3.0f));
  value.insert(make_pair(L"vier",  4.0f));
  value.insert(make_pair(L"fünf",  5.0f));
  value.insert(make_pair(L"sechs", 6.0f));
  value.insert(make_pair(L"sieben", 7.0f));
  value.insert(make_pair(L"acht", 8.0f));
  value.insert(make_pair(L"neun", 9.0f));
  value.insert(make_pair(L"zehn", 10.0f));
  value.insert(make_pair(L"elf", 11.0f));
  value.insert(make_pair(L"zwölf", 12.0f));
  value.insert(make_pair(L"dreizehn", 13.0f));
  value.insert(make_pair(L"vierzehn", 14.0f));
  value.insert(make_pair(L"fünfzehn", 15.0f));
  value.insert(make_pair(L"sechzehn", 16.0f));
  value.insert(make_pair(L"siebzehn", 17.0f));
  value.insert(make_pair(L"achtzehn", 18.0f));
  value.insert(make_pair(L"neunzehn", 19.0f));
  value.insert(make_pair(L"zwanzig", 20.0f));
  value.insert(make_pair(L"dreißig", 30.0f));
  value.insert(make_pair(L"dreissig", 30.0f));
  value.insert(make_pair(L"vierzig", 40.0f));
  value.insert(make_pair(L"fünfzig", 50.0f));
  value.insert(make_pair(L"sechzig", 60.0f));
  value.insert(make_pair(L"siebsig", 70.0f));
  value.insert(make_pair(L"achtzig", 80.0f));
  value.insert(make_pair(L"neunzig", 90.0f));
  value.insert(make_pair(L"hundert", 100.0f));
  value.insert(make_pair(L"einhundert", 100.0f));
  value.insert(make_pair(L"zweihundert", 200.0f));
  value.insert(make_pair(L"dreihundert", 300.0f));
  value.insert(make_pair(L"vierhundert", 400.0f));
  value.insert(make_pair(L"fünfhundert", 500.0f));
  value.insert(make_pair(L"sechshundert", 600.0f));
  value.insert(make_pair(L"siebenhundert", 700.0f));
  value.insert(make_pair(L"achthundert", 800.0f));
  value.insert(make_pair(L"neunhundert", 900.0f));
  value.insert(make_pair(L"tausend", 1000.0f));


  value.insert(make_pair(L"tausend", 1000.0f));

  // Initialize Final state set
  Final.insert(ST_read_eins2);
  Final.insert(ST_read_hundert);
  Final.insert(ST_read_unit2);
  Final.insert(ST_read_vierzig);
  Final.insert(ST_read_vierzehn);
  Final.insert(ST_read_tausend);
  Final.insert(ST_read_unit3);
  Final.insert(ST_read_hundert2);
  Final.insert(ST_read_unit4);
  Final.insert(ST_read_vierzehn2);
  Final.insert(ST_read_vierzig2);
  Final.insert(ST_read_eins);
  Final.insert(ST_read_digits);

  // Initialize transitions table. By default, stop state
  for (unsigned int s = 0; s < MAX_STATES; s++)
    for (unsigned int t = 0; t < MAX_TOKENS; t++)
      trans[s][t] = ST_STOP;

  // state START (0)
  trans[ST_START][TK_w_eins] = ST_read_eins2;
  trans[ST_START][TK_w_hundert] = ST_read_hundert;
  trans[ST_START][TK_w_tausend] = ST_read_tausend;
  trans[ST_START][TK_w_teens] = ST_read_vierzehn;
  trans[ST_START][TK_w_tens] = ST_read_vierzig;
  trans[ST_START][TK_w_unit] = ST_read_unit2;
  trans[ST_START][TK_num] = ST_read_digits;

  // state read_hundert (2)
  trans[ST_read_hundert][TK_w_eins] = ST_read_eins2;
  trans[ST_read_hundert][TK_w_teens] = ST_read_vierzehn;
  trans[ST_read_hundert][TK_w_tens] = ST_read_vierzig;
  trans[ST_read_hundert][TK_w_unit] = ST_read_unit2;

  // state read_unit2 (3)
  trans[ST_read_unit2][TK_w_tausend] = ST_read_tausend;
  trans[ST_read_unit2][TK_w_und] = ST_read_und;

  // state read_und (4)
  trans[ST_read_und][TK_w_tens] = ST_read_vierzig;

  // state read_vierzig (5)
  trans[ST_read_vierzig][TK_w_tausend] = ST_read_tausend;

  // state read_vierzehn (6)
  trans[ST_read_vierzehn][TK_w_tausend] = ST_read_tausend;

  // state read_tausend (7)
  trans[ST_read_tausend][TK_w_eins] = ST_read_eins;
  trans[ST_read_tausend][TK_w_hundert] = ST_read_hundert2;
  trans[ST_read_tausend][TK_w_teens] = ST_read_vierzehn2;
  trans[ST_read_tausend][TK_w_tens] = ST_read_vierzig2;
  trans[ST_read_tausend][TK_w_unit] = ST_read_unit4;

  // state read_unit3 (8)
  //  trans[ST_read_unit3][TK_w_hundert] = ST_read_hundert2;

  // state read_hundert2 (9)
  trans[ST_read_hundert2][TK_w_eins] = ST_read_eins;
  trans[ST_read_hundert2][TK_w_teens] = ST_read_vierzehn2;
  trans[ST_read_hundert2][TK_w_tens] = ST_read_vierzig2;
  trans[ST_read_hundert2][TK_w_unit] = ST_read_unit4;

  // state read_unit4 (10)
  trans[ST_read_unit4][TK_w_eins] = ST_read_eins;
  trans[ST_read_unit4][TK_w_teens] = ST_read_vierzehn2;
  trans[ST_read_unit4][TK_w_und] = ST_read_und2;

  // state read_und2 (12)
  trans[ST_read_und2][TK_w_tens] = ST_read_vierzig2;

  TRACE(3,L"number analyzer succesfully created");
 }
#ifdef DEDEBUG
    wstring numbers_de::tokenName(const int token) const {
	wstring tn = tokenNames.find(token)->second;
	wstring res = tn + L" (" + util::int2wstring(token) + L")";
	return res;
    }
    wstring numbers_de::stateName(const int state) const {
	wstring tn = stateNames.find(state)->second;
	wstring res = tn + L" (" + util::int2wstring(state) + L")";
	return res;
    }
#endif




  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int numbers_de::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
    wstring form;
    int token;
    map<wstring,int>::const_iterator im;
 
    form = j->get_lc_form();
    if (RE_number.search(form)) token = TK_num;    
    else if (form.back() == L'.'
	     && RE_number.search(form.substr(0, form.size()-1))) {
	token = TK_num;    
    }
    else token = TK_other;
    im = tok.find(form);
    if (im != tok.end()) {
      token = im->second;
    }

#ifdef DEDEBUG
    TRACE(3,L"State " + stateName(state) +
	  L" NEXT NUMBER word form is: [" + form + L"] token = " + tokenName(token));
#else
    TRACE(3,L"Next NUMBER word form is: ["+form+L"] token = " + util::int2wstring(token));
#endif


    // if the token was in the table, we're done
    if (token != TK_other) {
#ifdef DEDEBUG
	    TRACE(3,L"Leaving (a) state " + stateName(state)
		  + L" with token found in table "+ tokenName(token)); 
#else
	    TRACE(3,L"Leaving (a) state " + util::int2wstring(state)
		  + L" with token found in table " + util::int2wstring(token)); 
#endif
	return (token);
    }

    // Token not found in translation table, let's have a closer look.
    /*
    // split two last letters of the form
    wstring sfx,pref;
    if (form.length()>2) {
      sfx=form.substr(form.length()-2,2); // last two letters
      pref=form.substr(0,form.length()-2); // all but last two letters
    }
 
    // check to see if it is a numeric ordinal (1st, 42nd, 15th)
    if (form.length()>2 && (sfx==L"st" || sfx==L"nd" || sfx==L"rd" || sfx==L"th") 
        && RE_number.search(pref) )
      token=TK_ord;
    // check to see if it is a number or  an alphanumeric code
    else if (RE_number.search(form)) token = TK_num;
    else if (RE_code.search(form)) token = TK_code;
    */

#ifdef DEDEBUG
    TRACE(3,L"Leaving finally state " + stateName(state)
	  + L" with token "+ tokenName(token)); 
#else
    TRACE(3,L"Leaving finally state " + util::int2wstring(state)
	  + L" with token "+ util::int2wstring(token)); 
#endif

    return (token);
  }



  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state, update current 
  ///  nummerical value.
  ///////////////////////////////////////////////////////////////

  void numbers_de::StateActions(int origin, int state, int token, sentence::const_iterator j, numbers_status *st) const {
    wstring form;
    size_t i;
    long double num;

    form = j->get_lc_form();
#ifdef DEDEBUG
	TRACE(3,L"Reaching state " + stateName(state)
	      +L" with token " + tokenName(token)
	      +L" for word ["+form+L"]");
#else
	TRACE(3,L"Reaching state " + util::int2wstring(state)
	      +L" with token " + util::int2wstring(token)
	      +L" for word ["+form+L"]");
#endif

    // get token numerical value, if any
    num=0;
    if (token==TK_num) {
      // erase thousand points
      while ((i=form.find_first_of(MACO_Thousand))!=wstring::npos) {
        form.erase(i,1);
      }
      TRACE(3,L"after erasing thousand "+form);
      // make sure decimal point is L"."
      if ((i=form.find_last_of(MACO_Decimal))!=wstring::npos) {
        form[i]=L'.';
        TRACE(3,L"after replacing decimal "+form);
      }
      num = util::wstring2longdouble(form);
    }

    // State actions
    switch (state) {
      // ---------------------------------
    case ST_read_digits:
      TRACE(3,L"Actions for NUM state");
      if (token==TK_num)
        st->units += num;
      break;

    case ST_read_unit2:
      TRACE(3,L"Actions for normal state");
      if (token==TK_num)
	  st->units += num;//*1000;
      else
	  st->units += (value.find(form)->second);// * 1000;
      break;

      //    case ST_read_unit3:
	  //st->units += (value.find(form)->second * 100);
	  //lastValue = value.find(form)->second;
      //      break;
    case ST_read_unit4:
      TRACE(3,L"Actions for normal state");
      if (token==TK_num)
        st->units += num;
      else
        st->units += value.find(form)->second;
      break;

    case ST_read_vierzehn:
    case ST_read_vierzig:
    case ST_read_hundert:
      if (token==TK_num)
	  st->units += num;//*1000;
      else
	  st->units += (value.find(form)->second);// * 1000;
      break;
    case ST_read_vierzehn2:
    case ST_read_vierzig2:

      if (token==TK_num)
        st->units += num;
      else
	  st->units += (value.find(form)->second);
      break;
    case ST_read_hundert2:
	//if (lastValue != 0) {
	st->units +=  (value.find(form)->second); //lastValue * 100;
	//} else {
	//st->units += 100;
	//}
	//lastValue = 0;
	break;

    case ST_read_eins2:
    case ST_read_eins:
	st->units += 1;
	break;
    case ST_read_tausend:
	if (st->units == 0) st->units = 1;
	st->units *= 1000;
	
    default: break;
    }
  
    TRACE(3,L"Reach Actions finished, st->units = " << st->units);
	//TRACE(3,L"State actions completed. bilion="+util::longdouble2wstring(st->bilion)+L" milion="+util::longdouble2wstring(st->milion)+L" units="+util::longdouble2wstring(st->units));
  }


  ///////////////////////////////////////////////////////////////
  ///   Set the appropriate lemma and tag for the 
  ///   new multiword.
  ///////////////////////////////////////////////////////////////

  void numbers_de::SetMultiwordAnalysis(sentence::iterator i, int fstate, const numbers_status *st) const {
    wstring lemma, tag;

    tag=L"Z";

    // Setting the analysis for the nummerical expression
    if (st->iscode==CODE) {
      lemma=i->get_form();
    }
    else if (st->iscode==ORD) {
      // ordinal adjective.  4th --> (4th, 4, JJ)
      // L"fourth" gets the same analysis, but via dictionary.
      wstring fm=i->get_form();
      lemma = fm.substr(0,fm.length()-2); // all but last two letters
      tag=L"JJ";  
    }
    else {
      // compute nummerical value as lemma
      lemma=util::longdouble2wstring(st->bilion + st->milion + st->units);
    }

    i->set_analysis(analysis(lemma,tag));
    TRACE(3,L"Analysis set to: "+lemma+L" "+tag);
  }


} // namespace freeling 
