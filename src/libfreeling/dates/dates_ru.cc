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

  //---------------------------------------------------------------------------
  //        English date recognizer
  //---------------------------------------------------------------------------

  //// State codes 
  // states for a date
#define ST_A   1     // A: initial state
#define ST_B   2     // B: got a week day
#define ST_C   3     // C: got a week day and year
#define ST_D   4
#define ST_E   5
#define ST_F   6

#define ST_T0  10    // ST_T0: 12 часов
#define ST_T1  11    // ST_T1: 12 часов 17
#define ST_T2  12    // ST_T2: 12 часов 17 минут
#define ST_T3  13    // ST_T2: 17минут

  // stop state
#define ST_STOP 41

  ////////
  // Token codes
#define TK_weekday   1      // weekday: "Monday", "Tuesday", etc
#define TK_month     3 
#define TK_shmonth   5      // month abbr
#define TK_monthnum  6 
#define TK_number    7 
#define TK_morning   8 
#define TK_dot       9 
#define TK_colon     10
#define TK_in        11     // в 12 часов 34 минуты
#define TK_wmonth    12
#define TK_wpast     14
#define TK_wcentury  15
#define TK_roman     16
#define TK_wacdc     17
#define TK_hournum   18
#define TK_minnum    19
#define TK_date      22
#define TK_hhmm      23
#define TK_hour      24
#define TK_min       25
#define TK_day_am    26
#define TK_day_pm    27

#define TK_wyear     30
#define TK_wyear1    31

#define TK_whalf     40
#define TK_oclock    41 
#define TK_wquarter  42

#define TK_centnum   45
#define TK_yearnum   46

#define TK_other 100

  ///////////////////////////////////////////////////////////////
  ///  Create a dates recognizer for Russian.
  ///////////////////////////////////////////////////////////////

  dates_ru::dates_ru() : dates_module(RE_DATE_RU, RE_TIME_RU, RE_MINUTES_RU, RE_ROMAN)
  {
    //Initialize token translation map
#define ADD_TOK(m, w) tok.insert(make_pair(m, w))

    ADD_TOK(L"понедельник", TK_weekday);
    ADD_TOK(L"вторник", TK_weekday);
    ADD_TOK(L"среда", TK_weekday);  ADD_TOK(L"среду", TK_weekday);
    ADD_TOK(L"четверг", TK_weekday);
    ADD_TOK(L"пятница", TK_weekday); ADD_TOK(L"пятницу", TK_weekday);
    ADD_TOK(L"суббота", TK_weekday); ADD_TOK(L"субботу", TK_weekday);
    ADD_TOK(L"воскресенье", TK_weekday);    

    ADD_TOK(L"год", TK_wyear); ADD_TOK(L"года", TK_wyear); ADD_TOK(L"году", TK_wyear);
    ADD_TOK(L"час", TK_oclock); ADD_TOK(L"часа", TK_oclock); ADD_TOK(L"часов", TK_oclock); ADD_TOK(L"часам", TK_oclock); ADD_TOK(L"ч", TK_oclock); ADD_TOK(L"ч.", TK_oclock);
    ADD_TOK(L"в", TK_in);
    ADD_TOK(L"мин", TK_min); ADD_TOK(L"мин.", TK_min);   ADD_TOK(L"минута", TK_min); ADD_TOK(L"минут", TK_min); ADD_TOK(L"минуты", TK_min);
    ADD_TOK(L"дня", TK_day_am); ADD_TOK(L"вечера", TK_day_pm); ADD_TOK(L"ночи", TK_day_pm);

    // initialize map with month numeric values
#define ADD_MES(m, w, tk) nMes.insert(make_pair(m, w)); tok.insert(make_pair(m, tk));

    ADD_MES(L"январь", 1, TK_month);   ADD_MES(L"янв", 1, TK_shmonth);  ADD_MES(L"январем", 1, TK_month);   ADD_MES(L"января", 1, TK_month);    ADD_MES(L"январём", 1, TK_month); ADD_MES(L"январе", 1, TK_month); ADD_MES(L"январю", 1, TK_month);         
    ADD_MES(L"февраль", 2, TK_month);  ADD_MES(L"фев", 2, TK_shmonth);  ADD_MES(L"февралем", 2, TK_month);  ADD_MES(L"февраля", 2, TK_month);   ADD_MES(L"февралём", 2, TK_month); ADD_MES(L"феврале", 2, TK_month); ADD_MES(L"февралю", 2, TK_month);     
    ADD_MES(L"март", 3, TK_month);                                      ADD_MES(L"мартом", 3, TK_month);    ADD_MES(L"марта", 3, TK_month);     ADD_MES(L"марте", 3, TK_month); ADD_MES(L"марту", 3, TK_month);                                           
    ADD_MES(L"апрель", 4, TK_month);   ADD_MES(L"апр", 4, TK_shmonth);  ADD_MES(L"апрелем", 4, TK_month);   ADD_MES(L"апреля", 4, TK_month);    ADD_MES(L"апреле", 4, TK_month); ADD_MES(L"апрелю", 4, TK_month);                                        
    ADD_MES(L"май", 5, TK_month);                                       ADD_MES(L"маем", 5, TK_month);      ADD_MES(L"мая", 5, TK_month);       ADD_MES(L"мае", 5, TK_month); ADD_MES(L"маю", 5, TK_month);                                                 
    ADD_MES(L"июнь", 6, TK_month);                                      ADD_MES(L"июнем", 6, TK_month);     ADD_MES(L"июня", 6, TK_month);      ADD_MES(L"июне", 6, TK_month); ADD_MES(L"июню", 6, TK_month);                                              
    ADD_MES(L"июль", 7, TK_month);                                      ADD_MES(L"июлем", 7, TK_month);     ADD_MES(L"июля", 7, TK_month);      ADD_MES(L"июле", 7, TK_month); ADD_MES(L"июлю", 7, TK_month);                                              
    ADD_MES(L"август", 8, TK_month);   ADD_MES(L"авг", 8, TK_shmonth);  ADD_MES(L"августом", 8, TK_month);  ADD_MES(L"августа", 8, TK_month);   ADD_MES(L"августе", 8, TK_month); ADD_MES(L"августу", 8, TK_month);                                     
    ADD_MES(L"сентябрь", 9, TK_month); ADD_MES(L"сен", 9, TK_shmonth);  ADD_MES(L"сентябрем", 9, TK_month); ADD_MES(L"сентября", 9, TK_month);  ADD_MES(L"сентябрём", 9, TK_month); ADD_MES(L"сентябре", 9, TK_month); ADD_MES(L"сентябрю", 9, TK_month); 
    ADD_MES(L"октябрь", 10, TK_month); ADD_MES(L"окт", 10, TK_shmonth); ADD_MES(L"октябрем", 10, TK_month); ADD_MES(L"октября", 10, TK_month);  ADD_MES(L"октябрём", 10, TK_month); ADD_MES(L"октябре", 10, TK_month); ADD_MES(L"октябрю", 10, TK_month);     
    ADD_MES(L"ноябрь", 11, TK_month);  ADD_MES(L"ноя", 11, TK_shmonth); ADD_MES(L"ноябрем", 11, TK_month);  ADD_MES(L"ноября", 11, TK_month);   ADD_MES(L"ноябрём", 11, TK_month); ADD_MES(L"ноябре", 11, TK_month); ADD_MES(L"ноябрю", 11, TK_month);         
    ADD_MES(L"декабрь", 12, TK_month); ADD_MES(L"дек", 12, TK_shmonth); ADD_MES(L"декабрем", 12, TK_month); ADD_MES(L"декабря", 12, TK_month);  ADD_MES(L"декабрём", 12, TK_month); ADD_MES(L"декабре", 12, TK_month); ADD_MES(L"декабрю", 12, TK_month);     

    // initialize map with weekday values 
    nDia.insert(make_pair(L"понедельник", L"G"));
    nDia.insert(make_pair(L"вторник", L"L"));
    nDia.insert(make_pair(L"среда", L"M"));  
    nDia.insert(make_pair(L"четверг", L"X"));
    nDia.insert(make_pair(L"пятница", L"J"));     
    nDia.insert(make_pair(L"суббота", L"V"));
    nDia.insert(make_pair(L"воскресенье", L"S"));    

    // Initialize special state attributes
    initialState=ST_A; stopState=ST_STOP;

    // Initialize Final state set 
    Final.insert(ST_C); Final.insert(ST_D); Final.insert(ST_E); Final.insert(ST_F); Final.insert(ST_T0); Final.insert(ST_T2); Final.insert(ST_T3);

    // Initialize transition table. By default, stop state
    for(int s=0; s<MAX_STATES; ++s) 
      for(int t=0; t<MAX_TOKENS; ++t) 
        trans[s][t]=ST_STOP;

    // --------- ST_A -----------
    trans[ST_A][TK_date]=ST_D;
    trans[ST_A][TK_in]=ST_T1;    
    trans[ST_A][TK_minnum]=ST_T1;

    trans[ST_A][TK_min]=ST_T3;
    trans[ST_A][TK_hhmm]=ST_T3;
    trans[ST_A][TK_hour]=ST_T3;

    // --------- ST_B -----------
    trans[ST_B][TK_month]=ST_C;
    trans[ST_B][TK_shmonth]=ST_C;

    trans[ST_B][TK_wyear]=ST_E;

    trans[ST_B][TK_oclock]=ST_T0;

    // --------- ST_C -----------
    trans[ST_C][TK_number]=ST_F;

    // --------- ST_D -----------
    trans[ST_D][TK_in]=ST_E;

    // --------- ST_E -----------
    trans[ST_E][TK_wyear]=ST_E;
    trans[ST_E][TK_wyear1]=ST_E;
    trans[ST_E][TK_in]=ST_E;
    trans[ST_E][TK_number]=ST_B;

    // --------- ST_F -----------
    trans[ST_F][TK_number]=ST_B;
    trans[ST_F][TK_in]=ST_T0;

    // --------- ST_T0 ----------
    trans[ST_T0][TK_minnum]=ST_T1;
    trans[ST_T0][TK_day_pm]=ST_T0;
    trans[ST_T0][TK_day_am]=ST_T0;

    // --------- ST_T1 ----------
    trans[ST_T1][TK_min]=ST_T2;
    trans[ST_T1][TK_hhmm]=ST_T2;
    trans[ST_T1][TK_hour]=ST_T2;
    trans[ST_T1][TK_number]=ST_B;
    trans[ST_T1][TK_oclock]=ST_T0;
    trans[ST_T1][TK_month]=ST_E;
    trans[ST_T1][TK_wyear]=ST_E;
    TRACE(3,L"analyzer succesfully created");
  }


  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////
  int dates_ru::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
    wstring form;
    int token= TK_other, value=0;
    map<wstring, int>::const_iterator im;

    dates_status *st = (dates_status *)se.get_processing_status();
  
    form = j->get_lc_form();

    im = tok.find(form);
    if (im!=tok.end())
      token = (*im).second;
    
    TRACE(3,L"Next word form is: ["+ form +L"] token="+util::int2wstring(token));     

    // if the token was in the table, we're done
    if (token != TK_other) 
      return token;

    // check to see if it is a number
    if (j->get_n_analysis() && j->get_tag()==L"Z00000") 
      {
        token = TK_number;
        value = util::wstring2int(j->get_lemma());
        TRACE(3,L"Numerical value of form: "+util::int2wstring(value));
      }

    vector<wstring> rem;  // store regex matches

    // determine how to interpret that number, or if not number, check for specific regexps.
    switch (state) 
      {
      case ST_A: 
        TRACE(3,L"In state A");
        if (token == TK_number)
          {
            token = TK_minnum; // it can also be an hournum
          }
        else if (RE_Date.search(form)) 
          {
            TRACE(3,L"Match DATE regex. ");
            token = TK_date;
          }
        else if (RE_Time1.search(form, rem)) 
          {
            if (rem.size()>=3) 
              {
                st->hour = rem[1];
                st->minute= rem[2];
                TRACE(3,L"Match TIME1 regex (hour+min)");
                token = TK_hhmm;
              }
            else 
              { 
                st->hour = rem[1];
                TRACE(3,L"Partial match TIME1 regex (hour)");
                token = TK_hour;
              }  
          }
        else if (RE_Time2.search(form, rem))
          {
            token = TK_min;
            st->minute = rem[1];
          }
        break;
      case ST_B:
      case ST_C:
      case ST_T0:
        if (token==TK_number)
          token = TK_minnum;;
        break;
        break;
        // --------------------------------  
      default: break;
      }
    
    TRACE(3,L"Leaving state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)); 
    return token;
  }

  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state with an informative 
  ///  token (day, year, month, etc) store that part of the date.
  ///////////////////////////////////////////////////////////////

  void dates_ru::StateActions(int origin, int state, int token, sentence::const_iterator j, dates_status *st) const {

    if (state==ST_STOP) return;

    std::wstring form = j->get_lc_form();
    int value;
    map<wstring,int>::const_iterator im;
    vector<wstring> rem;  // store regex matches

    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ["+form+L"]");

    // get token numerical value, if any
    value=0;
    if ((token==TK_number || token==TK_monthnum ||
         token==TK_hournum || token==TK_minnum || token==TK_centnum || token==TK_yearnum) &&
        j->get_n_analysis() && j->get_tag()==L"Z00000")
      value = util::wstring2int(j->get_lemma());

    // State actions
    switch (state) 
      {
      case ST_B:
        if (token==TK_number)
          SetPrevStateValue(value,st); // can be a minute, hour, day or just number
        break;

      case ST_C:       
        if (token==TK_month || token==TK_shmonth)
          {
            st->month=util::int2wstring(nMes.find(form)->second);
            st->day = util::int2wstring(GetPrevStateValue(st));
          }
        break; 

      case ST_D:
        if (token==TK_date) 
          {
            std::wstring form = j->get_form();
            RE_Date.search(form, rem);
            // day number
            st->day=rem[1];
            // month number (translating month name if necessary) 
            im=nMes.find(util::lowercase(rem[2]));  // RE_Date.Match(2)
            if (im!=nMes.end())
              st->month = util::int2wstring((*im).second);
            else
              st->month = rem[2];
            st->year = rem[3];
            // if year has only two digits, it can be 19xx or 20xx 
            if (util::wstring2int(st->year)>=0 && util::wstring2int(st->year)<=20) 
              st->year=L"20"+st->year;
            else if (util::wstring2int(st->year)>=50 && util::wstring2int(st->year)<=99) 
              st->year=L"19"+st->year;
          }
        break;

      case ST_E:
        if (token==TK_wyear)
          st->year=util::int2wstring(GetPrevStateValue(st));
        else if (token==TK_month)
          {
            st->day = util::int2wstring(GetPrevStateValue(st));
            st->month = util::int2wstring(nMes.find(form)->second);
          }
        break;

      case ST_F:
        SetPrevStateValue(value,st);
        if (token==TK_number)
          st->year=util::int2wstring(GetPrevStateValue(st));
        break;

      case ST_T0:
        if (token==TK_day_pm || token==TK_day_am)
          {   
            if (st->hour==UNKNOWN_SYMB)
              {
                WARNING(L"case ST_T0 day not set");
                break;
              }
            int tmp = util::wstring2int(st->hour);
            if (tmp>=1 && tmp<12)
              st->hour = util::int2wstring(tmp + 12);
          }
        if (token==TK_oclock)
          st->hour = util::int2wstring(GetPrevStateValue(st));
        break;
      case ST_T1:
        if (token==TK_number || token==TK_minnum)
          SetPrevStateValue(value,st);
        break;
      case ST_T2:
        if (token==TK_min)
          st->minute = util::int2wstring(GetPrevStateValue(st));
        break;
      case ST_T3:
        break;

      default: break;
      }

    TRACE(3,L"State actions finished. ["+st->weekday+L":"+st->day+L":"+st->month+L":"+st->year+L":"+st->hour+L":"+st->minute+L":"+st->meridian+L"]");
  }

  ///////////////////////////////////////////////////////////////
  ///   Set the appropriate lemma and tag for the 
  ///   new multiword.
  ///////////////////////////////////////////////////////////////

  void dates_ru::SetMultiwordAnalysis(sentence::iterator i, int fstate, const dates_status *st)  const {
    list<analysis> la;
    wstring lemma;

    // Setting the analysis for the date     
    if (st->century!=L"??")
      lemma = L"[s:" + st->century + L"]";
    else
      lemma = L"["+normalize(st->weekday)+L":"+normalize(st->day)+L"/"+normalize(st->month)+L"/"+st->year+L":"+normalize(st->hour)+L"."+normalize(st->minute)+L":"+st->meridian+L"]";

    la.push_back(analysis(lemma,L"W"));
    i->set_analysis(la);
    TRACE(3,L"Analysis set to: "+lemma+L" W");
  }

  /* helpful functions */
  int dates_ru::GetPrevStateValue(dates_status *st) const {
    if (st->temp == -1)
      {
        WARNING(L"wrong automat logic! check trans-states map! temp == -1");
        return st->temp;
      }
    else
      {
        int t = st->temp;
        st->temp = -1;
        return t;
      }
  }

  void dates_ru::SetPrevStateValue(int value,dates_status *st) const {
    if (st->temp != -1)
      WARNING(L"wrong automat logic! check trans-states map! temp != -1");
    st->temp = value;
  }

} // namespace
