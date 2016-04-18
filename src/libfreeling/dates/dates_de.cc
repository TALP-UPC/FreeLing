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
//       German date/time recognizer                     //
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
#define ST_Start 0
#define ST_read_um 1
#define ST_read_hour 2
#define ST_read_time 3
#define ST_read_Uhr 4
#define ST_read_minute 5
#define ST_read_halb 6
#define ST_read_Viertel 7
#define ST_read_vor 8
#define ST_read_nach 9
#define ST_read_Uhr2 10
#define ST_read_elf 11
#define ST_read_am 12
#define ST_read_ord 13
#define ST_read_month 14
#define ST_read_year 15
#define ST_read_dow 16
#define ST_read_OrdPoint 17
#define ST_read_date 18
#define ST_read_comma 19
#define ST_STOP 49

#define TK_w_Monat 0
#define TK_w_Montag 1
#define TK_w_Uhr 2
#define TK_w_Viertel 3
#define TK_w_am_den 4
#define TK_w_comma 5
#define TK_w_den 6
#define TK_w_elf 7
#define TK_w_halb 8
#define TK_w_nach 9
#define TK_w_point 10
#define TK_w_um 11
#define TK_w_vor 12
#define TK_date 13
#define TK_day 14
#define TK_hour 15
#define TK_minute 16
#define TK_time 17
#define TK_year 18

#define TK_number 98
#define TK_other 99

    dates_de::dates_de(): dates_module(RE_DATE_DE, RE_TIME1_DE, RE_TIME2_DE, RE_ROMAN) {
#ifdef DEDEBUG
	// for tracing only
	stateNames.insert(make_pair(ST_Start, L"ST_Start"));
	stateNames.insert(make_pair(ST_read_um, L"ST_read_um"));
	stateNames.insert(make_pair(ST_read_hour, L"ST_read_hour"));
	stateNames.insert(make_pair(ST_read_time, L"ST_read_time"));
	stateNames.insert(make_pair(ST_read_Uhr, L"ST_read_Uhr"));
	stateNames.insert(make_pair(ST_read_minute, L"ST_read_minute"));
	stateNames.insert(make_pair(ST_read_halb, L"ST_read_halb"));
	stateNames.insert(make_pair(ST_read_Viertel, L"ST_read_Viertel"));
	stateNames.insert(make_pair(ST_read_vor, L"ST_read_vor"));
	stateNames.insert(make_pair(ST_read_nach, L"ST_read_nach"));
	stateNames.insert(make_pair(ST_read_Uhr2, L"ST_read_Uhr2"));
	stateNames.insert(make_pair(ST_read_elf, L"ST_read_elf"));
	stateNames.insert(make_pair(ST_read_am, L"ST_read_am"));
	stateNames.insert(make_pair(ST_read_ord, L"ST_read_ord"));
	stateNames.insert(make_pair(ST_read_month, L"ST_read_month"));
	stateNames.insert(make_pair(ST_read_year, L"ST_read_year"));
	stateNames.insert(make_pair(ST_read_dow, L"ST_read_dow"));
	stateNames.insert(make_pair(ST_read_OrdPoint, L"ST_read_OrdPoint"));
	stateNames.insert(make_pair(ST_read_date, L"ST_read_date"));
	stateNames.insert(make_pair(ST_read_comma, L"ST_read_comma"));
	stateNames.insert(make_pair(ST_STOP, L"ST_STOP"));

	tokenNames.insert(make_pair(TK_w_Monat, L"TK_w_Monat"));
	tokenNames.insert(make_pair(TK_w_Montag, L"TK_w_Montag"));
	tokenNames.insert(make_pair(TK_w_Uhr, L"TK_w_Uhr"));
	tokenNames.insert(make_pair(TK_w_Viertel, L"TK_w_Viertel"));
	tokenNames.insert(make_pair(TK_w_am_den, L"TK_w_am_den"));
	tokenNames.insert(make_pair(TK_w_comma, L"TK_w_comma"));
	tokenNames.insert(make_pair(TK_w_den, L"TK_w_den"));
	tokenNames.insert(make_pair(TK_w_elf, L"TK_w_elf"));
	tokenNames.insert(make_pair(TK_w_halb, L"TK_w_halb"));
	tokenNames.insert(make_pair(TK_w_nach, L"TK_w_nach"));
	tokenNames.insert(make_pair(TK_w_point, L"TK_w_point"));
	tokenNames.insert(make_pair(TK_w_um, L"TK_w_um"));
	tokenNames.insert(make_pair(TK_w_vor, L"TK_w_vor"));
	tokenNames.insert(make_pair(TK_date, L"TK_date"));
	tokenNames.insert(make_pair(TK_day, L"TK_day"));
	tokenNames.insert(make_pair(TK_hour, L"TK_hour"));
	tokenNames.insert(make_pair(TK_minute, L"TK_minute"));
	tokenNames.insert(make_pair(TK_time, L"TK_time"));
	tokenNames.insert(make_pair(TK_year, L"TK_year"));
	tokenNames.insert(make_pair(TK_number, L"TK_number"));
	tokenNames.insert(make_pair(TK_other, L"TK_other"));
#endif

	// Initialize special state attributes
	initialState=ST_Start;
	stopState=ST_STOP;

	// Initialize token stranslation map (TODO: complete!)
	tok.insert(make_pair(L"januar", TK_w_Monat));
	tok.insert(make_pair(L"februar", TK_w_Monat));
	tok.insert(make_pair(L"märz", TK_w_Monat));
	tok.insert(make_pair(L"april", TK_w_Monat));
	tok.insert(make_pair(L"mai", TK_w_Monat));
	tok.insert(make_pair(L"juni", TK_w_Monat));
	tok.insert(make_pair(L"juli", TK_w_Monat));
	tok.insert(make_pair(L"august", TK_w_Monat));
	tok.insert(make_pair(L"september", TK_w_Monat));
	tok.insert(make_pair(L"oktober", TK_w_Monat));
	tok.insert(make_pair(L"november", TK_w_Monat));
	tok.insert(make_pair(L"dezember", TK_w_Monat));

	tok.insert(make_pair(L"montag", TK_w_Montag));
	tok.insert(make_pair(L"dienstag", TK_w_Montag));
	tok.insert(make_pair(L"mittwoch", TK_w_Montag));
	tok.insert(make_pair(L"donnerstag", TK_w_Montag));
	tok.insert(make_pair(L"freitag", TK_w_Montag));
	tok.insert(make_pair(L"samstag", TK_w_Montag));
	tok.insert(make_pair(L"sonntag", TK_w_Montag));
	tok.insert(make_pair(L"uhr", TK_w_Uhr));
	tok.insert(make_pair(L"viertel", TK_w_Viertel));
	tok.insert(make_pair(L"am", TK_w_am_den));
	tok.insert(make_pair(L"den", TK_w_den));

	tok.insert(make_pair(L"ein", TK_w_elf));
	tok.insert(make_pair(L"eine", TK_w_elf));
	tok.insert(make_pair(L"zwei", TK_w_elf));
	tok.insert(make_pair(L"drei", TK_w_elf));
	tok.insert(make_pair(L"vier", TK_w_elf));
	tok.insert(make_pair(L"fünf", TK_w_elf));
	tok.insert(make_pair(L"sechs", TK_w_elf));
	tok.insert(make_pair(L"sieben", TK_w_elf));
	tok.insert(make_pair(L"acht", TK_w_elf));
	tok.insert(make_pair(L"neun", TK_w_elf));
	tok.insert(make_pair(L"zehn", TK_w_elf));
	tok.insert(make_pair(L"elf", TK_w_elf));
	tok.insert(make_pair(L"zwölf", TK_w_elf));
	tok.insert(make_pair(L"dreizehn", TK_w_elf));
	tok.insert(make_pair(L"vierzehn", TK_w_elf));
	tok.insert(make_pair(L"fünfzehn", TK_w_elf));
	tok.insert(make_pair(L"sechzehn", TK_w_elf));
	tok.insert(make_pair(L"siebzehn", TK_w_elf));
	tok.insert(make_pair(L"achtzehn", TK_w_elf));
	tok.insert(make_pair(L"neunzehn", TK_w_elf));
	tok.insert(make_pair(L"zwanzig", TK_w_elf));


	tok.insert(make_pair(L"halb", TK_w_halb));
	tok.insert(make_pair(L"nach", TK_w_nach));
	tok.insert(make_pair(L".", TK_w_point));
	tok.insert(make_pair(L",", TK_w_comma));
	tok.insert(make_pair(L"um", TK_w_um));
	tok.insert(make_pair(L"vor", TK_w_vor));

	// initialize map with number words
	nNumbers.insert(make_pair(L"ein", 1));
	nNumbers.insert(make_pair(L"eine", 1));
	nNumbers.insert(make_pair(L"zwei", 2));
	nNumbers.insert(make_pair(L"drei", 3));
	nNumbers.insert(make_pair(L"vier", 4));
	nNumbers.insert(make_pair(L"fünf", 5));
	nNumbers.insert(make_pair(L"sechs", 6));
	nNumbers.insert(make_pair(L"sieben", 7));
	nNumbers.insert(make_pair(L"acht", 8));
	nNumbers.insert(make_pair(L"neun", 9));
	nNumbers.insert(make_pair(L"zehn", 10));
	nNumbers.insert(make_pair(L"elf", 11));
	nNumbers.insert(make_pair(L"zwölf", 12));
	nNumbers.insert(make_pair(L"dreizehn", 13));
	nNumbers.insert(make_pair(L"vierzehn", 14));
	nNumbers.insert(make_pair(L"fünfzehn", 15));
	nNumbers.insert(make_pair(L"sechzehn", 16));
	nNumbers.insert(make_pair(L"siebzehn", 17));
	nNumbers.insert(make_pair(L"achtzehn", 18));
	nNumbers.insert(make_pair(L"neunzehn", 19));
	nNumbers.insert(make_pair(L"zwanzig", 20));

	// initialize map with month numeric values
	nMes.insert(make_pair(L"januar",1));
	nMes.insert(make_pair(L"februar",2));
	nMes.insert(make_pair(L"märz",3));
	nMes.insert(make_pair(L"april",4));
	nMes.insert(make_pair(L"mai",5));
	nMes.insert(make_pair(L"juni",6));
	nMes.insert(make_pair(L"juli",7));
	nMes.insert(make_pair(L"august",8));
	nMes.insert(make_pair(L"september",9));
	nMes.insert(make_pair(L"oktober",10));
	nMes.insert(make_pair(L"november",11));
	nMes.insert(make_pair(L"dezember",12));


	// initialize map with weekday numeric values
	nDia.insert(make_pair(L"montag",L"Mo"));
	nDia.insert(make_pair(L"dienstag",L"Di"));
	nDia.insert(make_pair(L"mittwoch",L"Mi"));
	nDia.insert(make_pair(L"donnerstag",L"Do"));
	nDia.insert(make_pair(L"freitag",L"Fr"));
	nDia.insert(make_pair(L"samstag",L"Sa"));
	nDia.insert(make_pair(L"sonnabend",L"Sa"));
	nDia.insert(make_pair(L"sonntag",L"So"));

	// Initialize Final state set
	Final.insert(ST_read_hour);
	Final.insert(ST_read_time);
	Final.insert(ST_read_Uhr);
	Final.insert(ST_read_minute);
	Final.insert(ST_read_Uhr2);
	Final.insert(ST_read_elf);
	Final.insert(ST_read_ord);
	Final.insert(ST_read_month);
	Final.insert(ST_read_year);
	Final.insert(ST_read_date);

	// Initialize transitions table. By default, stop state
	for (unsigned int s = 0; s < MAX_STATES; s++)
	    for (unsigned int t = 0; t < MAX_TOKENS; t++)
		trans[s][t] = ST_STOP;

	// state Start (0)
	trans[ST_Start][TK_w_Montag] = ST_read_dow;
	trans[ST_Start][TK_w_Viertel] = ST_read_Viertel;
	trans[ST_Start][TK_w_am_den] = ST_read_am;
	trans[ST_Start][TK_w_halb] = ST_read_halb;
	trans[ST_Start][TK_w_um] = ST_read_um;
	trans[ST_Start][TK_date] = ST_read_date;
	trans[ST_Start][TK_day] = ST_read_ord;
	trans[ST_Start][TK_hour] = ST_read_hour;
	trans[ST_Start][TK_time] = ST_read_time;

	// state read_um (1)
	trans[ST_read_um][TK_w_Viertel] = ST_read_Viertel;
	trans[ST_read_um][TK_w_elf] = ST_read_elf;
	trans[ST_read_um][TK_w_halb] = ST_read_halb;
	trans[ST_read_um][TK_hour] = ST_read_hour;
	trans[ST_read_um][TK_time] = ST_read_time;

	// state read_hour (2)
	trans[ST_read_hour][TK_w_Monat] = ST_read_month;
	trans[ST_read_hour][TK_w_Uhr] = ST_read_Uhr;
	trans[ST_read_hour][TK_w_nach] = ST_read_nach;
	trans[ST_read_hour][TK_w_point] = ST_read_OrdPoint; // this means we have read a day and not an hour
	trans[ST_read_hour][TK_w_vor] = ST_read_vor;

	// state read_Uhr (4)
	trans[ST_read_Uhr][TK_minute] = ST_read_minute;

	// state read_halb (6)
	trans[ST_read_halb][TK_w_elf] = ST_read_elf;

	// state read_Viertel (7)
	trans[ST_read_Viertel][TK_w_nach] = ST_read_nach;
	trans[ST_read_Viertel][TK_w_vor] = ST_read_vor;

	// state read_vor (8)
	trans[ST_read_vor][TK_w_elf] = ST_read_elf;
	trans[ST_read_vor][TK_hour] = ST_read_elf;

	// state read_nach (9)
	trans[ST_read_nach][TK_w_elf] = ST_read_elf;
	trans[ST_read_nach][TK_w_halb] = ST_read_halb;
	trans[ST_read_nach][TK_hour] = ST_read_elf;

	// state read_elf (11)
	trans[ST_read_elf][TK_w_Uhr] = ST_read_Uhr2;
	trans[ST_read_elf][TK_w_nach] = ST_read_nach;
	trans[ST_read_elf][TK_w_vor] = ST_read_vor;

	// state read_am (12)
	trans[ST_read_am][TK_date] = ST_read_date;
	trans[ST_read_am][TK_day] = ST_read_ord;

	// state read_ord (13)
	trans[ST_read_ord][TK_w_Monat] = ST_read_month;
	trans[ST_read_ord][TK_w_point] = ST_read_OrdPoint;

	// state read_month (14)
	trans[ST_read_month][TK_w_um] = ST_read_um;
	trans[ST_read_month][TK_year] = ST_read_year;

	// state read_year (15)
	trans[ST_read_year][TK_w_um] = ST_read_um;

	// state read_dow (16)
	trans[ST_read_dow][TK_w_den] = ST_read_am;

	// state read_OrdPoint (17)
	trans[ST_read_OrdPoint][TK_w_Monat] = ST_read_month;

	// state read_date (18)
	trans[ST_read_date][TK_w_comma] = ST_read_comma;
	trans[ST_read_date][TK_w_um] = ST_read_um;

	// state read_comma (19)
	trans[ST_read_comma][TK_time] = ST_read_time;

	TRACE(3,L"analyzer succesfully created");
    }
#ifdef DEDEBUG
    wstring dates_de::tokenName(const int token) const {
	wstring tn = tokenNames.find(token)->second;
	wstring res = tn + L" (" + util::int2wstring(token) + L")";
	return res;
    }
    wstring dates_de::stateName(const int state) const {
	wstring tn = stateNames.find(state)->second;
	wstring res = tn + L" (" + util::int2wstring(state) + L")";
	return res;
    }
#endif

    // Implementation of virtual functions from class automate
    // Compute the right token code for word j from given state.
    // called before leaving the state
    int dates_de::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
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

	//tokenName(token);
#ifdef DEDEBUG
	TRACE(3,L"State " + stateName(state) +
	      L" NEXT word form is: [" + form + L"] token = " + tokenName(token));
#else
	TRACE(3,L"Next word form is: ["+form+L"] token = " + util::int2wstring(token));
#endif
	// if the token was in the table, we are done
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

	case ST_Start:
	case ST_read_comma:
	    if (RE_Date.search(form,st->rem)) {		
		TRACE(3,L"Match DATE (h+m) regex.");
		//for (unsigned int i = 0; i < st->rem.size(); ++i) {
		//    //TRACE(3,L" RE " + i + L" " + st->rem[i]);
		//    wcerr << L" RE " <<i << L" " << st->rem[i] << endl;
		//}
		token = TK_date;
	    } 
	    else if (RE_Time1.search(form,st->rem)) {
		if (st->rem.size() >= 3 && !st->rem[2].empty()) {
		    TRACE(3,L"Match TIME1 (h+m) regex.");
		    token = TK_time;
		} else {
		    TRACE(3,L"Match TIME1 (h) regex.");
		    token = TK_hour;
		}
	    }
	    else if (token == TK_number) {
		if (value <= 24) {
		    token = TK_hour;
		} else 
		    token = TK_day;
	    }
	    break;

	case ST_read_um:
	    if (RE_Time1.search(form,st->rem)) {
		if (st->rem.size() >= 3 && !st->rem[2].empty()) {
		    TRACE(3,L"Match TIME1 (h+m) regex.");
		    token = TK_time;
		} else {
		    TRACE(3,L"Match TIME1 (h) regex.");
		    token = TK_hour;
		}
	    }
	    else if (token == TK_number && value <= 24) {
		token = TK_hour;
	    }
	    break;

	case ST_read_am:
	    TRACE(3,L"check Match DATE regex." + form);
	    if (RE_Date.search(form,st->rem)) {		
		TRACE(3,L"Match DATE regex.");
		//for (unsigned int i = 0; i < st->rem.size(); ++i) {
		//    //TRACE(3,L" RE " + i + L" " + st->rem[i]);
		//    wcerr << L" RE " <<i << L" " << st->rem[i] << endl;
		//}
		token = TK_date;
	    } else 
		token = TK_day;

	    break;

	case ST_read_vor:
	    if (token == TK_number && value <= 24) {
		token = TK_hour;
	    }
	    break;

	case ST_read_nach:
	    if (token == TK_number && value <= 24) {
		token = TK_hour;
	    }
	    break;

	case ST_read_Uhr:
	    break;

	case ST_read_month:
	    if (token == TK_number /*&& value <= 24*/) {
		token = TK_year;
	    }
	    break;

	case ST_read_OrdPoint:
	    TRACE(3,L"SET NEW TOKEN " + stateName(state));
	    token = TK_w_Monat;
	    break;

	default:
	    break;
	}
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
    ///  Basically, when reaching a state with an informative 
    ///  token (day, year, month, etc) store that part of the date.
    ///////////////////////////////////////////////////////////////
    void dates_de::StateActions(int origin, int state, int token, sentence::const_iterator j, dates_status *st) const {
	wstring form;
	int value;
	map<wstring,int>::const_iterator im;
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
	if (state==ST_STOP) return;
	// get token numerical value, if any
	value=0;
	if ((token==TK_number || token==TK_day || //token==TK_month ||
             token==TK_year ||
	     token==TK_hour || token == TK_minute) &&
	    j->get_n_analysis() && j->get_tag()==L"Z") {
	    value = util::wstring2int(j->get_lemma());
	}
	// State actions
	switch (state) {
        case ST_Start:
	    break;
        case ST_read_um:
	    break;
        case ST_read_hour:
	    if (token==TK_hour) {
		TRACE(3,L"Actions for state ST_read_hour");
		st->hour=util::int2wstring(value);
		lastValue = value;
		// no minutes seen yet, but it is possible that the phrase was "um 10 " 
		// so here we set the minutes to 0, they might be reset by later states
		st->minute = L"0";
	    }

	    break;
        case ST_read_time:
	    if (token==TK_time) {
		TRACE(3,L"Actions for state ST_read_time");
		// rem contains matches for RE_Time1
		st->hour=st->rem[1];   // RE_Time1.Match(1);
		st->minute=st->rem[2];  // RE_Time1.Match(2);
	    }

	    break;
        case ST_read_Uhr:
	    break;
        case ST_read_minute:
	    break;
        case ST_read_halb:
	    // heaving found "halb", we stock -30 in minutes to take into account when found the hour
	    st->minute=L"-30";
	    break;
        case ST_read_elf:
	    if (token==TK_w_elf) {
		if (origin == ST_read_vor) {
		    int h = nNumbers.find(form)->second;
		    h--;
		    if (h == 0) h = 12;
		    st->hour = util::int2wstring(h);    
		}
		else if (origin == ST_read_nach) {
		    int h = nNumbers.find(form)->second;
		    st->hour = util::int2wstring(h);    
		} 
		else {
		    // let's take it as hours, but we also put the
		    // value into lastValues to be able to detect "um 10 nach 4"
		    TRACE(3,L"Actions 1 for state ST_read_elf");
		    int h = nNumbers.find(form)->second;
		    lastValue = h;
		    if (st->minute ==L"-30") {
			h--;
			if (h == 0) h = 12;
			st->minute = L"30";
		    }
		    st->hour = util::int2wstring(h);
		}
	    } if (token == TK_hour) {
		if (origin == ST_read_vor) {
		    int h = value;
		    h--;
		    if (h == 0) h = 12;
		    st->hour = util::int2wstring(h);    
		}
		else if (origin == ST_read_nach) {
		    st->hour = util::int2wstring(value);    
		} 

	    }
	    break;
	case ST_read_Viertel:
	    //lastValue = 15;
	    break;
	case ST_read_vor:
	    if (origin == ST_read_elf) {
		st->minute = util::int2wstring(60-lastValue);
	    }
	    else if (origin == ST_read_hour) {
		st->minute = util::int2wstring(60-lastValue);
		st->hour = L"";
	    }
	    else if (origin == ST_read_Viertel) {
		st->minute = util::int2wstring(45);
	    }
	    break;
	case ST_read_nach:
	    if (origin == ST_read_elf) {
		st->minute = util::int2wstring(lastValue);
	    }
	    else if (origin == ST_read_hour) {
		st->minute = util::int2wstring(lastValue);
		st->hour = L"";
	    }
	    else if (origin == ST_read_Viertel) {
		st->minute = util::int2wstring(15);
	    }

	    break;
	case ST_read_Uhr2:
	    break;
	case ST_read_am:
	    break;
	case ST_read_OrdPoint:
	    TRACE(3,L"Actions for state ST_read_OrdPoint " + stateName(origin) );
	    if (origin == ST_read_hour) {
		// "3. " in fact 3 is a day and not an hour
		st->day = util::int2wstring(lastValue);
		
	    }
	    break;

	case ST_read_ord:
	    TRACE(3,L"Actions for state ST_read_ord");
	    st->day = util::int2wstring(value);
	    break;

	case ST_read_month:
	    if (token==TK_w_Monat) {
		TRACE(3,L"Actions for state ST_read_month");
		st->month = util::int2wstring(nMes.find(form)->second);
		// eventually correct preceding token which was possibly mal-interpreted as hour
		if (st->hour != L"" && st->hour != L"??") {
		    //wcerr << st->hour << L" " << st->day << endl;
		    st->day = st->hour;
		    st->hour = L"";
		}
	    }
	    break;
	case ST_read_date:
	    st->day=st->rem[1];   // RE_Date.Match(1);
	    st->month=st->rem[2];
	    st->year=st->rem[3];
	    break;
	case ST_read_year:
	    st->year = util::int2wstring(value);
	    break;
	case ST_read_dow:
	    if (token==TK_w_Montag) {
		TRACE(3,L"Actions for state ST_read_dow");
		st->weekday=nDia.find(form)->second;
	    }

	    st->month = util::int2wstring(nMes.find(form)->second);
	    break;
	}
	TRACE(3,L"Reach Actions finished");
    }

    ///////////////////////////////////////////////////////////////
    ///   Set the appropriate lemma and tag for the
    ///   new multiword.
    ///////////////////////////////////////////////////////////////

    void dates_de::SetMultiwordAnalysis(sentence::iterator i, int fstate, const dates_status *st) const {
	list<analysis> la;
	wstring lemma;

	// Setting the analysis for the date
	lemma=L"["+st->weekday+L":"+st->day+L"/"+st->month+L"/"+st->year+L":"+st->hour+L"."+st->minute+L":"+st->meridian+L"]";

	la.push_back(analysis(lemma,L"W"));
	i->set_analysis(la);
	TRACE(3,L"Analysis set to: ("+lemma+L" W)");
    }

} // namespace freeling 
