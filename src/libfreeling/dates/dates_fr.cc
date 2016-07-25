/////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////

//-------------------------------------------------------//
//       French date/time recognizer                     //
//       Johannes Heinecke (johannes.heinecke@orange.fr) //
//-------------------------------------------------------//


#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/dates_modules.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"DATES"
#define MOD_TRACECODE DATES_TRACE
    

    // State codes 
#define ST_1 0
#define ST_2a 1
#define ST_3 2
#define ST_4 3
#define ST_5 4
#define ST_6 5
#define ST_7 6
#define ST_8 7
#define ST_9 8
#define ST_10 9
#define ST_11 10
#define ST_12 11
#define ST_13 12
#define ST_14 13
#define ST_15 14
#define ST_16 15
#define ST_17 16
#define ST_18 17
#define ST_19 18
#define ST_20 19
#define ST_21 20
#define ST_22 21
#define ST_23 22
#define ST_24 23
#define ST_25 24
#define ST_26 25
#define ST_STOP 49



    // Token codes
#define TK_w_1er 0
#define TK_w_a 1
#define TK_w_au 2
#define TK_w_en 3
#define TK_w_et 4
#define TK_w_fin 5
#define TK_w_h 6
#define TK_w_janvier 7
#define TK_w_le 8
#define TK_w_lundi 9
#define TK_w_matin 10
#define TK_w_matinee 11
#define TK_w_midi 12
#define TK_w_minuit 13
#define TK_w_minutes 14
#define TK_w_moins 15
#define TK_w_quart 16
#define TK_w_soir 17
#define TK_date 18
#define TK_day 19
#define TK_daymonth 20
#define TK_hour 21
#define TK_hourH 22
#define TK_minute 23
#define TK_month 24
#define TK_time 25
#define TK_year 26

#define TK_number 98
#define TK_other 99

    dates_fr::dates_fr(): dates_module(RE_DATE_FR, RE_TIME1_FR, RE_TIME2_FR, RE_ROMAN) {
#ifdef FRDEBUG
	// for tracing only
	stateNames.insert(make_pair(ST_1, L"ST_1"));
	stateNames.insert(make_pair(ST_2a, L"ST_2a"));
	stateNames.insert(make_pair(ST_3, L"ST_3"));
	stateNames.insert(make_pair(ST_4, L"ST_4"));
	stateNames.insert(make_pair(ST_5, L"ST_5"));
	stateNames.insert(make_pair(ST_6, L"ST_6"));
	stateNames.insert(make_pair(ST_7, L"ST_7"));
	stateNames.insert(make_pair(ST_8, L"ST_8"));
	stateNames.insert(make_pair(ST_9, L"ST_9"));
	stateNames.insert(make_pair(ST_10, L"ST_10"));
	stateNames.insert(make_pair(ST_11, L"ST_11"));
	stateNames.insert(make_pair(ST_12, L"ST_12"));
	stateNames.insert(make_pair(ST_13, L"ST_13"));
	stateNames.insert(make_pair(ST_14, L"ST_14"));
	stateNames.insert(make_pair(ST_15, L"ST_15"));
	stateNames.insert(make_pair(ST_16, L"ST_16"));
	stateNames.insert(make_pair(ST_17, L"ST_17"));
	stateNames.insert(make_pair(ST_18, L"ST_18"));
	stateNames.insert(make_pair(ST_19, L"ST_19"));
	stateNames.insert(make_pair(ST_20, L"ST_20"));
	stateNames.insert(make_pair(ST_21, L"ST_21"));
	stateNames.insert(make_pair(ST_22, L"ST_22"));
	stateNames.insert(make_pair(ST_23, L"ST_23"));
	stateNames.insert(make_pair(ST_24, L"ST_24"));
	stateNames.insert(make_pair(ST_25, L"ST_25"));
	stateNames.insert(make_pair(ST_26, L"ST_26"));
	stateNames.insert(make_pair(ST_STOP, L"ST_STOP"));

	tokenNames.insert(make_pair(TK_w_1er, L"TK_w_1er"));
	tokenNames.insert(make_pair(TK_w_a, L"TK_w_a"));
	tokenNames.insert(make_pair(TK_w_au, L"TK_w_au"));
	tokenNames.insert(make_pair(TK_w_en, L"TK_w_en"));
	tokenNames.insert(make_pair(TK_w_et, L"TK_w_et"));
	tokenNames.insert(make_pair(TK_w_fin, L"TK_w_fin"));
	tokenNames.insert(make_pair(TK_w_h, L"TK_w_h"));
	tokenNames.insert(make_pair(TK_w_janvier, L"TK_w_janvier"));
	tokenNames.insert(make_pair(TK_w_le, L"TK_w_le"));
	tokenNames.insert(make_pair(TK_w_lundi, L"TK_w_lundi"));
	tokenNames.insert(make_pair(TK_w_matin, L"TK_w_matin"));
	tokenNames.insert(make_pair(TK_w_matinee, L"TK_w_matinee"));
	tokenNames.insert(make_pair(TK_w_midi, L"TK_w_midi"));
	tokenNames.insert(make_pair(TK_w_minuit, L"TK_w_minuit"));
	tokenNames.insert(make_pair(TK_w_minutes, L"TK_w_minutes"));
	tokenNames.insert(make_pair(TK_w_moins, L"TK_w_moins"));
	tokenNames.insert(make_pair(TK_w_quart, L"TK_w_quart"));
	tokenNames.insert(make_pair(TK_w_soir, L"TK_w_soir"));
	tokenNames.insert(make_pair(TK_date, L"TK_date"));
	tokenNames.insert(make_pair(TK_day, L"TK_day"));
	tokenNames.insert(make_pair(TK_daymonth, L"TK_daymonth"));
	tokenNames.insert(make_pair(TK_hour, L"TK_hour"));
	tokenNames.insert(make_pair(TK_hourH, L"TK_hourH"));
	tokenNames.insert(make_pair(TK_minute, L"TK_minute"));
	tokenNames.insert(make_pair(TK_month, L"TK_month"));
	tokenNames.insert(make_pair(TK_time, L"TK_time"));
	tokenNames.insert(make_pair(TK_year, L"TK_year"));
	tokenNames.insert(make_pair(TK_number, L"TK_number"));
	tokenNames.insert(make_pair(TK_other, L"TK_other"));
#endif

	// Initialize special state attributes
	initialState=ST_1;
	stopState=ST_STOP;

	// Initialize token stranslation map (TODO: complete!
	tok.insert(make_pair(L"1er", TK_w_1er));
	tok.insert(make_pair(L"a", TK_w_a));
	tok.insert(make_pair(L"au", TK_w_au));
	tok.insert(make_pair(L"à", TK_w_a));
	tok.insert(make_pair(L"en", TK_w_en));
	tok.insert(make_pair(L"et", TK_w_et));
	tok.insert(make_pair(L"moins", TK_w_moins));
	tok.insert(make_pair(L"fin", TK_w_fin));
	tok.insert(make_pair(L"h", TK_w_h));
	tok.insert(make_pair(L":", TK_w_h));
	tok.insert(make_pair(L"heure", TK_w_h));
	tok.insert(make_pair(L"heures", TK_w_h));
	tok.insert(make_pair(L"janvier", TK_w_janvier));
	tok.insert(make_pair(L"janv", TK_w_janvier));
	tok.insert(make_pair(L"février", TK_w_janvier));
	tok.insert(make_pair(L"fevrier", TK_w_janvier));
	tok.insert(make_pair(L"fév", TK_w_janvier));
	tok.insert(make_pair(L"fev", TK_w_janvier));
	tok.insert(make_pair(L"mars", TK_w_janvier));
	tok.insert(make_pair(L"avril", TK_w_janvier));
	tok.insert(make_pair(L"mai", TK_w_janvier));
	tok.insert(make_pair(L"juin", TK_w_janvier));
	tok.insert(make_pair(L"juillet", TK_w_janvier));
	tok.insert(make_pair(L"juil", TK_w_janvier));
	tok.insert(make_pair(L"août", TK_w_janvier));
	tok.insert(make_pair(L"aout", TK_w_janvier));
	tok.insert(make_pair(L"septembre", TK_w_janvier));
	tok.insert(make_pair(L"octobre", TK_w_janvier));
	tok.insert(make_pair(L"novembre", TK_w_janvier));
	tok.insert(make_pair(L"décembre", TK_w_janvier));
	tok.insert(make_pair(L"decembre", TK_w_janvier));
	tok.insert(make_pair(L"sep", TK_w_janvier));
	tok.insert(make_pair(L"oct", TK_w_janvier));
	tok.insert(make_pair(L"nov", TK_w_janvier));
	tok.insert(make_pair(L"déc", TK_w_janvier));
	tok.insert(make_pair(L"dec", TK_w_janvier));
	tok.insert(make_pair(L"le", TK_w_le));
	tok.insert(make_pair(L"lundi", TK_w_lundi));
	tok.insert(make_pair(L"mardi", TK_w_lundi));
	tok.insert(make_pair(L"mercredi", TK_w_lundi));
	tok.insert(make_pair(L"jeudi", TK_w_lundi));
	tok.insert(make_pair(L"vendredi", TK_w_lundi));
	tok.insert(make_pair(L"samedi", TK_w_lundi));
	tok.insert(make_pair(L"dimanche", TK_w_lundi));
	tok.insert(make_pair(L"matin", TK_w_matin));
	tok.insert(make_pair(L"matinee", TK_w_matinee));
	tok.insert(make_pair(L"matinée", TK_w_matinee));
	tok.insert(make_pair(L"minutes", TK_w_minutes));
	tok.insert(make_pair(L"minute", TK_w_minutes));
	tok.insert(make_pair(L"min", TK_w_minutes));
	tok.insert(make_pair(L"m", TK_w_minutes));
	tok.insert(make_pair(L"quart", TK_w_quart));
	tok.insert(make_pair(L"demi", TK_w_quart));
	tok.insert(make_pair(L"soir", TK_w_soir));
	tok.insert(make_pair(L"matin", TK_w_matin));
	//tok.insert(make_pair(L"apres-midi", TK_w_apres_midi));
	tok.insert(make_pair(L"midi", TK_w_midi));
	tok.insert(make_pair(L"minuit", TK_w_minuit));

	// initialize map with month numeric values
	nMes.insert(make_pair(L"janvier",1));
	nMes.insert(make_pair(L"janv",1));
	nMes.insert(make_pair(L"févr",2));
	nMes.insert(make_pair(L"fevr",2));
	nMes.insert(make_pair(L"février",2));
	nMes.insert(make_pair(L"fevrier",2));
	nMes.insert(make_pair(L"mars",3));
	nMes.insert(make_pair(L"avril",4));
	nMes.insert(make_pair(L"mai",5));
	nMes.insert(make_pair(L"juin",6));
	nMes.insert(make_pair(L"juillet",7));
	nMes.insert(make_pair(L"juil",7));
	nMes.insert(make_pair(L"aout",8));
	nMes.insert(make_pair(L"août",8));
	nMes.insert(make_pair(L"septembre",9));
	nMes.insert(make_pair(L"octobre",10));
	nMes.insert(make_pair(L"novembre",11));
	nMes.insert(make_pair(L"décembre",12));
	nMes.insert(make_pair(L"decembre",12));
	nMes.insert(make_pair(L"sep",9));
	nMes.insert(make_pair(L"oct",10));
	nMes.insert(make_pair(L"nov",11));
	nMes.insert(make_pair(L"déc",12));
	nMes.insert(make_pair(L"dec",12));

	// initialize map with weekday numeric values
	nDia.insert(make_pair(L"lundi",L"L"));
	nDia.insert(make_pair(L"mardi",L"M"));
	nDia.insert(make_pair(L"mercredi",L"C"));
	nDia.insert(make_pair(L"jeudi",L"J"));
	nDia.insert(make_pair(L"vendredi",L"V"));
	nDia.insert(make_pair(L"samedi",L"S"));
	nDia.insert(make_pair(L"dimanche",L"D"));

	// Initialize Final state set
	Final.insert(ST_3);
	Final.insert(ST_4);
	Final.insert(ST_6);
	Final.insert(ST_7);
	Final.insert(ST_8);
	Final.insert(ST_10);
	Final.insert(ST_12);
	Final.insert(ST_13);
	Final.insert(ST_14);
	Final.insert(ST_16);
	Final.insert(ST_17);
	Final.insert(ST_20);
	Final.insert(ST_23);
	Final.insert(ST_24);
	Final.insert(ST_25);



	// Initialize transitions table. By default, stop state
	for (unsigned int s = 0; s < MAX_STATES; s++)
	    for (unsigned int t = 0; t < MAX_TOKENS; t++)
		trans[s][t] = ST_STOP;
	// state 1 (0)
	trans[ST_1][TK_w_1er] = ST_21;
	trans[ST_1][TK_w_a] = ST_9;
	trans[ST_1][TK_w_en] = ST_18;
	trans[ST_1][TK_w_le] = ST_2a;
	trans[ST_1][TK_w_lundi] = ST_3;
	trans[ST_1][TK_date] = ST_8;
	trans[ST_1][TK_day] = ST_5;
	trans[ST_1][TK_daymonth] = ST_25;
	trans[ST_1][TK_hour] = ST_11;

	// state 2a (1)
	trans[ST_2a][TK_w_lundi] = ST_3;
	trans[ST_2a][TK_date] = ST_8;

	// state 3 (2)
	trans[ST_3][TK_w_1er] = ST_21;
	trans[ST_3][TK_w_matin] = ST_4;
	trans[ST_3][TK_w_midi] = ST_4;
	trans[ST_3][TK_w_soir] = ST_4;
	trans[ST_3][TK_date] = ST_8;
	trans[ST_3][TK_day] = ST_5;

	// state 4 (3)
	trans[ST_4][TK_w_a] = ST_9;

	// state 5 (4)
	trans[ST_5][TK_w_janvier] = ST_6;
	trans[ST_5][TK_month] = ST_6;

	// state 6 (5)
	trans[ST_6][TK_w_a] = ST_9;
	trans[ST_6][TK_w_au] = ST_3;
	trans[ST_6][TK_w_en] = ST_18;
	trans[ST_6][TK_w_le] = ST_3;
	trans[ST_6][TK_year] = ST_7;

	// state 7 (6)
	trans[ST_7][TK_w_a] = ST_9;
	trans[ST_7][TK_w_au] = ST_3;
	trans[ST_7][TK_w_en] = ST_18;
	trans[ST_7][TK_w_le] = ST_3;

	// state 8 (7)
	trans[ST_8][TK_w_a] = ST_9;

	// state 9 (8)
	trans[ST_9][TK_w_midi] = ST_23;
	trans[ST_9][TK_w_minuit] = ST_24;
	trans[ST_9][TK_hour] = ST_11;
	trans[ST_9][TK_hourH] = ST_14;
	trans[ST_9][TK_time] = ST_10;

	// state 11 (10)
	trans[ST_11][TK_w_h] = ST_12;

	// state 12 (11)
	trans[ST_12][TK_w_et] = ST_15;
	trans[ST_12][TK_w_moins] = ST_22;
	trans[ST_12][TK_minute] = ST_13;

	// state 13 (12)
	trans[ST_13][TK_w_minutes] = ST_17;

	// state 14 (13)
	trans[ST_14][TK_w_et] = ST_15;
	trans[ST_14][TK_w_moins] = ST_22;
	trans[ST_14][TK_minute] = ST_13;

	// state 15 (14)
	trans[ST_15][TK_w_quart] = ST_16;
	trans[ST_15][TK_minute] = ST_13;

	// state 18 (17)
	trans[ST_18][TK_w_fin] = ST_19;

	// state 19 (18)
	trans[ST_19][TK_w_matinee] = ST_20;

	// state 21 (20)
	trans[ST_21][TK_w_janvier] = ST_6;

	// state 22 (21)
	trans[ST_22][TK_w_quart] = ST_16;
	trans[ST_22][TK_minute] = ST_13;

	// state 23 (22)
	trans[ST_23][TK_w_et] = ST_15;
	trans[ST_23][TK_w_moins] = ST_22;

	// state 24 (23)
	trans[ST_24][TK_w_et] = ST_15;
	trans[ST_24][TK_w_moins] = ST_22;

	// state 25 (24)
	trans[ST_25][TK_w_a] = ST_9;

	TRACE(3,L"analyzer succesfully created");
    }
    
#ifdef FRDEBUG
    wstring dates_fr::tokenName(const int token) const {
	wstring tn = tokenNames.find(token)->second;
	wstring res = tn + L" (" + util::int2wstring(token) + L")";
	return res;
    }
    wstring dates_fr::stateName(const int state) const {
	wstring tn = stateNames.find(state)->second;
	wstring res = tn + L" (" + util::int2wstring(state) + L")";
	return res;
    }
#endif

    // Implementation of virtual functions from class automat 
    // Compute the right token code for word j from given state.
    // called before leaving the state
    int dates_fr::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
	wstring form,formU;
	int token,value;
	map<wstring,int>::const_iterator im;

	dates_status *st = (dates_status *)se.get_processing_status();
	st->rem.clear(); // clear any previous RE matches

	formU = j->get_form();
	form = j->get_lc_form();

	token = TK_other;
	im = tok.find(form);
	if (im!=tok.end()) {
	    token = im->second;
	}

	tokenName(token);
#ifdef FRDEBUG
	TRACE(3,L"ANext word form is: ["+form+L"] token = " + tokenName(token));
#else
	TRACE(3,L"ANext word form is: ["+form+L"] token = " + util::int2wstring(token));
#endif

	// if the token was in the table, we're done
	if (token != TK_other) {
#ifdef FRDEBUG
	    TRACE(3,L"Leaving (a) state " + stateName(state)
		  + L" with token found in table "+ tokenName(token)); 
#else
	    TRACE(3,L"Leaving (a) state " + util::int2wstring(state)
		  + L" with token found in table " + util::int2wstring(token)); 
#endif
	    return (token);
	}

	// Token not found in translation table, let's have a closer look.

	// check to see if it is a number
	value=0;
	if (j->get_n_analysis() && j->get_tag()==L"Z") {
	    token = TK_number;
	    value = util::wstring2int(j->get_lemma());
	    TRACE(3,L"Numerical value of form '"+j->get_lemma()+L"': "+util::int2wstring(value));
	}
  
	// determine how to interpret that number, or if not number,
	// check for specific regexps.
	switch (state) {
        case ST_1:
	case ST_3:
	    if (RE_Date.search(form,st->rem)) {
		TRACE(3,L"Match DATE regex.");
		token = TK_date;
	    }

	    else if (token == TK_number && value <= 31) {
		token = TK_day;
	    }

	    break;
	case ST_5:
	    if (token == TK_number && value<=12) {
		token = TK_month;
	    }
	    break;
	case ST_6:
	    if (token == TK_number && value>1) {
		token = TK_year;
	    }
	    break;

	case ST_9:
          //	    wcerr << L"feeeeee " << form << L" " << st->rem.size() << endl;
	    if (RE_Time1.search(form,st->rem)) {
		if (st->rem.size() >= 3 && !st->rem[2].empty()) {
		    TRACE(3,L"Match TIME1 (h+m) regex.");
		    token = TK_time;
		} else {
		    TRACE(3,L"Match TIME1 (h) regex.");
		    token = TK_hourH;
		}
	    }
	    else if (token == TK_number && value <= 24) {
		token = TK_hour;
	    }

	    break;

	case ST_12:
	case ST_14:
	case ST_15:
	case ST_22:
	    if (token == TK_number && value <= 59) {
		token = TK_minute;
	    }
	    break;

	default:
	    break;
	}

#ifdef FRDEBUG
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
    ///  Basically, when reaching a state with an informative 
    ///  token (day, year, month, etc) store that part of the date.
    ///////////////////////////////////////////////////////////////

    void dates_fr::StateActions(int origin, int state, int token, sentence::const_iterator j, dates_status *st) const {
	wstring form;
	int value;
	map<wstring,int>::const_iterator im;

	form = j->get_lc_form();

#ifdef FRDEBUG
	TRACE(3,L"Reaching state " + stateName(state)
	      +L" with token " + tokenName(token)
	      +L" for word ["+form+L"]");
#else
	TRACE(3,L"Reaching state " + util::int2wstring(state)
	      +L" with token " + util::int2wstring(token)
	      +L" for word ["+form+L"]");
#endif

	if (state==ST_STOP) return;

	// get token numerical value, if any
	value=0;
	if ((token==TK_number || token==TK_day || token==TK_month || token==TK_year ||
	     token==TK_hour || token == TK_minute) &&
	    j->get_n_analysis() && j->get_tag()==L"Z") {
	    value = util::wstring2int(j->get_lemma());
	}

	// State actions
	switch (state) {
	case ST_3:
	    if (token==TK_w_lundi) {
		TRACE(3,L"Actions for state ST_3");
		st->weekday=nDia.find(form)->second;
	    }
	    break;
	case ST_4:
	    if (token==TK_w_matin) {
		TRACE(3,L"Actions for state ST_4");
		st->hour=L"9";
	    }
	    else if (token==TK_w_midi) {
		TRACE(3,L"Actions for state ST_4");
		st->hour=L"12";
	    }
	    else if (token==TK_w_soir) {
		TRACE(3,L"Actions for state ST_4");
		st->hour=L"18";
	    }
	    break;

	case ST_5:
	    if (token==TK_day) {
		TRACE(3,L"Actions for state ST_5");
		st->day=util::int2wstring(value);
	    }
	    break;
	case ST_6:
	    if (token==TK_w_janvier) {
		TRACE(3,L"Actions 1 for state ST_6");
		st->month = util::int2wstring(nMes.find(form)->second);
	    } else if (token==TK_month) {
		TRACE(3,L"Actions 2 for state ST_6");
		st->month = util::int2wstring(value);
	    }
	    break;
	case ST_7:
	    if (token==TK_year) {
		TRACE(3,L"Actions for state ST_7");
		st->year=util::int2wstring(value);
	    }

	    break;

	case ST_8:
	    if (token==TK_date) {
		TRACE(3,L"Actions for state ST_8");
		// rem contains matches for RE_Date
		st->day=st->rem[1];
		// to unify notation (01 -> 1)
		st->day=normalize(st->day);
		// month number (translating month name if necessary)
		im=nMes.find(util::lowercase(st->rem[2]));
		if (im != nMes.end()) 
		    st->month = util::int2wstring((*im).second);
		else {
		    st->month = st->rem[2];
		    // to unify notation (01 -> 1)
		    st->month=normalize(st->month);
		}
		// year number
		st->year = st->rem[3];
		// if year has only two digits, it can be 19xx or 20xx
		if (util::wstring2int(st->year)>=0 && util::wstring2int(st->year)<=20) st->year=L"20"+st->year;
		else if (util::wstring2int(st->year)>=50 && util::wstring2int(st->year)<=99) st->year=L"19"+st->year;
	    }

	    break;
	case ST_10:
	    if (token==TK_time) {
		TRACE(3,L"Actions for state ST_10");
		// rem contains matches for RE_Time1
		st->hour=st->rem[1];   // RE_Time1.Match(1);
		st->minute=st->rem[2];  // RE_Time1.Match(2);
	    }
	    break;

	case ST_11:
	    if (token==TK_hour) {
		TRACE(3,L"Actions for state ST_11");
		st->hour=util::int2wstring(value);
		// no minutes seen yet, but it is possible that the phrase was "à 10 heures" 
		// so here we set the minutes to 0, they might be reset by later states
		st->minute = L"0";
	    }

	    break;
	case ST_13:
	    if (token==TK_minute) {
		TRACE(3,L"Actions for state ST_13");
		if (origin==ST_15) {
		    st->minute=util::int2wstring(value);
		} else if (origin == ST_22) {
		    value = 60-value;
		    st->minute=util::int2wstring(value);
		    if (st->hour==L"1") st->hour=L"13"; // una menos veinte = 12:40
		    st->hour = normalize(st->hour, -1);
		}
	    }

	    break;
	case ST_14:
	    if (token==TK_hourH) {
		TRACE(3,L"Actions for state ST_14");
		// rem contains matches for RE_Time1
		st->hour=st->rem[1];   // RE_Time1.Match(1);
	    }
	    break;
	case ST_16:
	    if (token==TK_w_quart) {
		TRACE(3,L"Actions for state ST_16");
		if (origin==ST_15) {
		    if (form==L"demi") {
			st->minute = L"30";
		    } else if (form==L"quart") {
			st->minute = L"15";
		    }
		}
		else if (origin == ST_22) {
		    if (form==L"quart") {
			st->minute = L"45";
			st->hour = normalize(st->hour, -1);
		    }
		}
	    }
	    break;
	case ST_20:
	    // fin-matinee
	    TRACE(3,L"Actions for state ST_20");
	    st->hour= L"11";
	    st->minute= L"0";
	    break;

	case ST_21:
	    TRACE(3,L"Actions for state ST_21");
	    st->day = L"1";
	    break;
	case ST_24:
	    st->hour = L"0";
	    break;
	case ST_23:
	    st->hour = L"12";

	case ST_25:
	    if (token==TK_daymonth) {
		TRACE(3,L"Actions for state ST_25");
		// rem contains matches for RE_Date
		st->day=st->rem[1];
		// to unify notation (01 -> 1)
		st->day=normalize(st->day);
		// month number (translating month name if necessary)
		im=nMes.find(util::lowercase(st->rem[2]));
		if (im != nMes.end()) 
		    st->month = util::int2wstring((*im).second);
		else {
		    st->month = st->rem[2];
		    // to unify notation (01 -> 1)
		    st->month=normalize(st->month);
		}
	    }

	    break;

	default:
	    break;
	}
    }



    ///////////////////////////////////////////////////////////////
    ///   Set the appropriate lemma and tag for the
    ///   new multiword.
    ///////////////////////////////////////////////////////////////

    void dates_fr::SetMultiwordAnalysis(sentence::iterator i, int fstate, const dates_status *st) const {
	list<analysis> la;
	wstring lemma;

	// Setting the analysis for the date
	lemma=L"["+st->weekday+L":"+st->day+L"/"+st->month+L"/"+st->year+L":"+st->hour+L"."+st->minute+L":"+st->meridian+L"]";

	la.push_back(analysis(lemma,L"W"));
	i->set_analysis(la);
	TRACE(3,L"Analysis set to: ("+lemma+L" W)");
        // record this word was analyzed by this module
        i->set_analyzed_by(word::DATES);    
        // mark the word as analyzed
        i->lock_analysis();    
    }

} // namespace
