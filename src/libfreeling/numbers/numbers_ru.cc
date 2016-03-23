
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/numbers_modules.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"NUMBERS"
#define MOD_TRACECODE NUMBERS_TRACE

#define ORD_NUM // поддержка порядковых числительных

  //---------------------------------------------------------------------------
  //        Russian number recognizer
  //---------------------------------------------------------------------------

  //// State codes 
#define ST_B1 1  // initial state
#define ST_B2 2  // got units  (VALID NUMBER)
#define ST_B3 3  // двадцать три (десятки)
#define ST_B4 4  // сто двадцать (сотни)
#define ST_B5 5  // ???
#define ST_B6 6  // четверть миллиона, просто - "четверть" недопустимо

#define ST_M1 9  
#define ST_M2 10 
#define ST_M3 11 
#define ST_M4 12 
#define ST_M5 13

#define ST_COD 25 // got pseudo-numerical code from initial state
  // stop state
#define ST_STOP 26

  // Token codes
#define TK_hundr   2  // word "hundred"
#define TK_thous   3  // word "thousand"
#define TK_mill    4  // word "million"
#define TK_millrd  5  // word "milliard"
#define TK_num     6  // a number (in digits)
#define TK_code    7  // a code (ex. LX-345-2)
#define TK_with    8  // word "c"
#define TK_ord     9  // "1-ый", "2-ой"
#define TK_other   10
#define TK_half    11 // word "половина"
#define TK_quarter 12 // word "четверть"

#define TK_u       20  // units less than 20
#define TK_u_dec   21  // decimal units 20, 30 ... 90
#define TK_u_hun   22  // hundred units 100, 200 ... 900

  ////// TODO: 
#define TK_u1      30 // порядковое числительное ... less than 20
#define TK_u1_dec   31
#define TK_u1_hun   32

  ///////////////////////////////////////////////////////////////
  ///  Create a numbers recognizer for Russian
  ///////////////////////////////////////////////////////////////

  const wstring RE_NUM_RU=L"^[+-]?(\\d+[.,])?\\d+$";

  numbers_ru::numbers_ru(const std::wstring &dec, const std::wstring &thou): numbers_module(dec,thou) {

    RE_number = freeling::regexp(RE_NUM_RU);

#ifdef ORD_NUM
    value.insert(make_pair(L"первый", 1.0f));          tok.insert(make_pair(L"первый", TK_u1));
    value.insert(make_pair(L"второй", 2.0f));          tok.insert(make_pair(L"второй", TK_u1)); 
    value.insert(make_pair(L"третий", 3.0f));          tok.insert(make_pair(L"третий", TK_u1));
    value.insert(make_pair(L"четвертый", 4.0f));       tok.insert(make_pair(L"четвертый", TK_u1));
    value.insert(make_pair(L"пятый", 5.0f));           tok.insert(make_pair(L"пятый", TK_u1));
    value.insert(make_pair(L"шестой", 6.0f));          tok.insert(make_pair(L"шестой", TK_u1));
    value.insert(make_pair(L"седьмой", 7.0f));         tok.insert(make_pair(L"седьмой", TK_u1));
    value.insert(make_pair(L"восьмой", 8.0f));         tok.insert(make_pair(L"восьмой", TK_u1));
    value.insert(make_pair(L"девятый", 9.0f));         tok.insert(make_pair(L"девятый", TK_u1));
    value.insert(make_pair(L"десятый", 10.0f));        tok.insert(make_pair(L"десятый", TK_u1));
    value.insert(make_pair(L"одиннадцатый", 11.0f));   tok.insert(make_pair(L"одиннадцатый", TK_u1));
    value.insert(make_pair(L"двенадцатый", 12.0f));    tok.insert(make_pair(L"двенадцатый", TK_u1));
    value.insert(make_pair(L"тринадцатый", 13.0f));    tok.insert(make_pair(L"тринадцатый", TK_u1));
    value.insert(make_pair(L"четырнадцатый", 14.0f));  tok.insert(make_pair(L"четырнадцатый", TK_u1));
    value.insert(make_pair(L"пятнадцатый", 15.0f));    tok.insert(make_pair(L"пятнадцатый", TK_u1));
    value.insert(make_pair(L"шестнадцатый", 16.0f));   tok.insert(make_pair(L"шестнадцатый", TK_u1));
    value.insert(make_pair(L"семнадцатый", 17.0f));    tok.insert(make_pair(L"семнадцатый", TK_u1));
    value.insert(make_pair(L"восемнадцатый", 18.0f));  tok.insert(make_pair(L"восемнадцатый", TK_u1));
    value.insert(make_pair(L"девятнадцатый", 19.0f));  tok.insert(make_pair(L"девятнадцатый", TK_u1));
    
    value.insert(make_pair(L"двадцатый", 20.0f));     tok.insert(make_pair(L"двадцатый", TK_u1_dec));
    value.insert(make_pair(L"тридцатый", 30.0f));     tok.insert(make_pair(L"тридцатый", TK_u1_dec));
    value.insert(make_pair(L"сороковой", 40.0f));     tok.insert(make_pair(L"сороковой", TK_u1_dec));
    value.insert(make_pair(L"пятидесятый", 50.0f));   tok.insert(make_pair(L"пятидесятый", TK_u1_dec));
    value.insert(make_pair(L"шестидесятый", 60.0f));  tok.insert(make_pair(L"шестидесятый", TK_u1_dec));
    value.insert(make_pair(L"семидесятый", 70.0f));   tok.insert(make_pair(L"семидесятый", TK_u1_dec));
    value.insert(make_pair(L"восьмидесятый", 80.0f)); tok.insert(make_pair(L"восьмидесятый", TK_u1_dec));
    value.insert(make_pair(L"девяностый", 90.0f));    tok.insert(make_pair(L"девяностый", TK_u1_dec)); 
    
    value.insert(make_pair(L"сотый", 100.0f));         tok.insert(make_pair(L"сотый", TK_u1_hun));
    value.insert(make_pair(L"двухсотый", 200.0f));     tok.insert(make_pair(L"двухсотый", TK_u1_hun));
    value.insert(make_pair(L"трехсотый", 300.0f));     tok.insert(make_pair(L"трехсотый", TK_u1_hun));
    value.insert(make_pair(L"трёхсотый", 300.0f));     tok.insert(make_pair(L"трёхсотый", TK_u1_hun));
    value.insert(make_pair(L"четырехсотый", 400.0f));  tok.insert(make_pair(L"четырехсотый", TK_u1_hun));
    value.insert(make_pair(L"четырёхсотый", 400.0f));  tok.insert(make_pair(L"четырёхсотый", TK_u1_hun));
    value.insert(make_pair(L"пятисотый", 500.0f));     tok.insert(make_pair(L"пятисотый", TK_u1_hun));
    value.insert(make_pair(L"шестисотый", 600.0f));    tok.insert(make_pair(L"шестисотый", TK_u1_hun)); 
    value.insert(make_pair(L"семисотый", 700.0f));     tok.insert(make_pair(L"семисотый", TK_u1_hun));
    value.insert(make_pair(L"восьмисотый", 800.0f));   tok.insert(make_pair(L"восьмисотый", TK_u1_hun));
    value.insert(make_pair(L"девятисотый", 900.0f));   tok.insert(make_pair(L"девятисотый", TK_u1_hun));
#endif
    
    value.insert(make_pair(L"один", 1.0f));           tok.insert(make_pair(L"один", TK_u));
    value.insert(make_pair(L"два", 2.0f));            tok.insert(make_pair(L"два", TK_u));
    value.insert(make_pair(L"две", 2.0f));            tok.insert(make_pair(L"две", TK_u));
    value.insert(make_pair(L"три", 3.0f));            tok.insert(make_pair(L"три", TK_u));
    value.insert(make_pair(L"четыре", 4.0f));         tok.insert(make_pair(L"четыре", TK_u));
    value.insert(make_pair(L"пять", 5.0f));           tok.insert(make_pair(L"пять", TK_u));
    value.insert(make_pair(L"шесть", 6.0f));          tok.insert(make_pair(L"шесть", TK_u));
    value.insert(make_pair(L"семь", 7.0f));           tok.insert(make_pair(L"семь", TK_u));
    value.insert(make_pair(L"восемь", 8.0f));         tok.insert(make_pair(L"восемь", TK_u));
    value.insert(make_pair(L"девять", 9.0f));         tok.insert(make_pair(L"девять", TK_u));
    value.insert(make_pair(L"десять", 10.0f));        tok.insert(make_pair(L"десять", TK_u));
    value.insert(make_pair(L"одиннадцать", 11.0f));   tok.insert(make_pair(L"одиннадцать", TK_u));
    value.insert(make_pair(L"двенадцать", 12.0f));    tok.insert(make_pair(L"двенадцать", TK_u));
    value.insert(make_pair(L"тринадцать", 13.0f));    tok.insert(make_pair(L"тринадцать", TK_u));
    value.insert(make_pair(L"четырнадцать", 14.0f));  tok.insert(make_pair(L"четырнадцать", TK_u));
    value.insert(make_pair(L"пятнадцать", 15.0f));    tok.insert(make_pair(L"пятнадцать", TK_u));
    value.insert(make_pair(L"шестнадцать", 16.0f));   tok.insert(make_pair(L"шестнадцать", TK_u));
    value.insert(make_pair(L"семнадцать", 17.0f));    tok.insert(make_pair(L"семнадцать", TK_u));
    value.insert(make_pair(L"восемнадцать", 18.0f));  tok.insert(make_pair(L"восемнадцать", TK_u));
    value.insert(make_pair(L"девятнадцать", 19.0f));  tok.insert(make_pair(L"девятнадцать", TK_u));

    value.insert(make_pair(L"двадцать", 20.0f));      tok.insert(make_pair(L"двадцать", TK_u_dec));
    value.insert(make_pair(L"тридцать", 30.0f));    tok.insert(make_pair(L"тридцать", TK_u_dec));
    value.insert(make_pair(L"сорок", 40.0f));       tok.insert(make_pair(L"сорок", TK_u_dec));
    value.insert(make_pair(L"пятьдесят", 50.0f));   tok.insert(make_pair(L"пятьдесят", TK_u_dec));
    value.insert(make_pair(L"шестьдесят", 60.0f));  tok.insert(make_pair(L"шестьдесят", TK_u_dec));
    value.insert(make_pair(L"семьдеся", 70.0f));    tok.insert(make_pair(L"семьдеся", TK_u_dec));
    value.insert(make_pair(L"восемьдесят", 80.0f)); tok.insert(make_pair(L"восемьдесят", TK_u_dec));
    value.insert(make_pair(L"девяносто", 90.0f));   tok.insert(make_pair(L"девяносто", TK_u_dec));

    value.insert(make_pair(L"сто", 100.0f));        tok.insert(make_pair(L"сто", TK_u_hun));
    value.insert(make_pair(L"двести", 200.0f));     tok.insert(make_pair(L"двести", TK_u_hun));
    value.insert(make_pair(L"триста", 300.0f));     tok.insert(make_pair(L"триста", TK_u_hun));
    value.insert(make_pair(L"четыреста", 400.0f));  tok.insert(make_pair(L"четыреста", TK_u_hun));
    value.insert(make_pair(L"пятьсот", 500.0f));    tok.insert(make_pair(L"пятьсот", TK_u_hun));
    value.insert(make_pair(L"шестьсот", 600.0f));   tok.insert(make_pair(L"шестьсот", TK_u_hun));
    value.insert(make_pair(L"семьсот", 700.0f));    tok.insert(make_pair(L"семьсот", TK_u_hun));
    value.insert(make_pair(L"восемьсот", 800.0f));  tok.insert(make_pair(L"восемьсот", TK_u_hun));
    value.insert(make_pair(L"девятьсот", 900.0f));  tok.insert(make_pair(L"девятьсот", TK_u_hun));

    value.insert(make_pair(L"половиной", 0.5f));  tok.insert(make_pair(L"половиной", TK_half));
    value.insert(make_pair(L"четверть", 0.25f));  tok.insert(make_pair(L"четверть", TK_quarter));

    value.insert(make_pair(L"полсотни", 50.0f));  tok.insert(make_pair(L"полсотни", TK_u));
    value.insert(make_pair(L"полтысячи", 500.0f));  tok.insert(make_pair(L"полтысячи", TK_u));
    value.insert(make_pair(L"полмиллиона", 500000.0f));  tok.insert(make_pair(L"полмиллиона", TK_u));
    value.insert(make_pair(L"полмиллиарда", 500000000.0f));  tok.insert(make_pair(L"полмиллиарда", TK_u));

    tok.insert(make_pair(L"сотня", TK_hundr));  tok.insert(make_pair(L"сотен", TK_hundr));   tok.insert(make_pair(L"сотни", TK_hundr));
    tok.insert(make_pair(L"тысяча", TK_thous)); tok.insert(make_pair(L"тысячи", TK_thous));  tok.insert(make_pair(L"тысяч", TK_thous));    tok.insert(make_pair(L"тыс", TK_thous));
    tok.insert(make_pair(L"миллион", TK_mill)); tok.insert(make_pair(L"миллиона", TK_mill)); tok.insert(make_pair(L"миллионов", TK_mill)); tok.insert(make_pair(L"млн", TK_mill));
    tok.insert(make_pair(L"миллиард", TK_millrd)); tok.insert(make_pair(L"миллиарда", TK_millrd)); tok.insert(make_pair(L"млрд", TK_millrd));

    tok.insert(make_pair(L"с", TK_with));

    // Initializing power map
    power.insert(make_pair(TK_hundr,   100.0));
    power.insert(make_pair(TK_thous,   1000.0));
    power.insert(make_pair(TK_mill,    1000000.0));
    power.insert(make_pair(TK_millrd,  1000000000.0));

    // Initialize special state attributes
    initialState=ST_B1; stopState=ST_STOP;

    // Initialize Final state set 
    Final.insert(ST_B2);  Final.insert(ST_B3);  Final.insert(ST_B4);  Final.insert(ST_B5);
    Final.insert(ST_M1);  Final.insert(ST_M2);  Final.insert(ST_M3);  Final.insert(ST_M4);
    Final.insert(ST_M5);
    Final.insert(ST_COD);  

    // Initialize transitions table. By default, stop state
    for(int s=0; s<MAX_STATES; ++s) 
      for(int t=0; t<MAX_TOKENS; ++t) 
        trans[s][t] = ST_STOP;

    // Initializing transitions table
    // ************ State B1 ************
    trans[ST_B1][TK_num]=ST_B2;
    trans[ST_B1][TK_u]=ST_B2;
    trans[ST_B1][TK_u_dec]=ST_B3;
    trans[ST_B1][TK_u_hun]=ST_B4;

    trans[ST_B1][TK_u1]=ST_B2;
    trans[ST_B1][TK_u1_dec]=ST_B2;
    trans[ST_B1][TK_u1_hun]=ST_B2;

    trans[ST_B1][TK_hundr]=ST_M1;
    trans[ST_B1][TK_thous]=ST_M1;     
    trans[ST_B1][TK_mill] =ST_M1;
    trans[ST_B1][TK_millrd]=ST_M1;

    trans[ST_B1][TK_quarter]=ST_B6; // четверть миллиона, четверть миллиарда, просто - "четверть" недопустимо

    trans[ST_B1][TK_ord]=ST_COD; // 2-го 5-м 

    // ************ State B2 ************
    trans[ST_B2][TK_hundr]=ST_M1;
    trans[ST_B2][TK_thous]=ST_M2;     
    trans[ST_B2][TK_mill] =ST_M3;
    trans[ST_B2][TK_millrd]=ST_M4;

    trans[ST_B2][TK_with]=ST_B2;
    trans[ST_B2][TK_half]=ST_B2;
    trans[ST_B2][TK_quarter]=ST_B2;

    // ************ State B3 ************
    trans[ST_B3][TK_u]=ST_B2;
    trans[ST_B3][TK_u1]=ST_B2;

    // ************ State B4 ************
    trans[ST_B4][TK_u]=ST_B2;
    trans[ST_B4][TK_u1]=ST_B2;
    trans[ST_B4][TK_u_dec]=ST_B3;
    trans[ST_B4][TK_u1_dec]=ST_B3;

    // ************ State B5 ************

    // ************ State B6 ************
    trans[ST_B6][TK_hundr]=ST_M5;
    trans[ST_B6][TK_thous]=ST_M5;
    trans[ST_B6][TK_mill]=ST_M5;
    trans[ST_B6][TK_millrd]=ST_M5;

    // ************ State M1 ************
    trans[ST_M1][TK_u]=ST_B2;     //двух сотен двадцати двух
    trans[ST_M1][TK_u_dec]=ST_B3; // TODO: двух сотен тысяч
    trans[ST_M1][TK_u_hun]=ST_B4;

    // ************ State M2 ************
    trans[ST_M2][TK_u]=ST_B2;     //двух тысяч двухста двадцати двух
    trans[ST_M2][TK_u_dec]=ST_B3;
    trans[ST_M2][TK_u_hun]=ST_B4; // TODO: двух тысяч миллионов

    // ************ State M3 ************
    trans[ST_M3][TK_u]=ST_B2;     //двух миллионов двадцати тысяч двухста двадцати двух
    trans[ST_M3][TK_u_dec]=ST_B3;
    trans[ST_M3][TK_u_hun]=ST_B4;
    trans[ST_M3][TK_thous]=ST_M2;

    // ************ State M4 ************
    trans[ST_M4][TK_u]=ST_B2;     //двух миллиардов двадцати миллионов двадцати тысяч двухста двадцати двух
    trans[ST_M4][TK_u_dec]=ST_B3;
    trans[ST_M4][TK_u_hun]=ST_B4;
    trans[ST_M4][TK_thous]=ST_M2;
    trans[ST_M4][TK_mill]=ST_M3;

    // ************ State M5 ************

    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int numbers_ru::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
    wstring form;
    int token = TK_other;

    form = j->get_lc_form();
    map<wstring, int>::const_iterator im = tok.find(form);

    if (im != tok.end()) {
      token = (*im).second;
      TRACE(3,L"Next word form is: ["+form+L"] token="+util::int2wstring(token));
    }
    else {
      // Token not found in translation table, let's have a closer look.
      // split two last letters of the form
    
      wstring sfx, pref;
      wstring::size_type len = form.length();
      wstring::size_type dashPos = form.find(L"-");
    
      // Наращение состоит из одной буквы, если предпоследняя буква гласная и двух, 
      // если предпоследняя буква согласная.
      // ниже - упрощенная логика
      if ((len > 2) && (dashPos != form.npos)) {
        sfx = form.substr(dashPos + 1); 
        pref = form.substr(0, dashPos);
        if ((RE_number.search(pref))/* && (RE_alhpa.search(pref))*/)
          token=TK_ord;
      }
      //  TODO : дописать поддержку случаев : 1-ый, 2-ой и т.д
      // check to see if it is a number or an alphanumeric code
      else if (RE_number.search(form)) 
        token = TK_num;
      else if (RE_code.search(form)) 
        token = TK_code;

      TRACE(3,L"Leaving state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)); 
    }
  
    return token;
  }

  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state, update current 
  ///  nummerical value.
  ///////////////////////////////////////////////////////////////

  void numbers_ru::StateActions(int origin, int state, int token, sentence::const_iterator j, numbers_status *st) const {
    wstring::size_type i;
    long double num = 0;

    wstring form = j->get_lc_form();
    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ["+form+L"]");

    // get token numerical value, if any
    if (token==TK_num) {
      if (((i=form.find_last_of(L".")) != wstring::npos) || ((i=form.find_last_of(L",")) != wstring::npos)) {
        form[i]=L'.';
        TRACE(3, L"after replacing decimal "+form);
      }
      num = util::wstring2longdouble(form);
    }
  
    // State actions
    switch (state) {
      // ---------------------------------
    case ST_B2: case ST_B3: case ST_B4: case ST_B6:
      TRACE(3,L"Actions for normal state");
      st->units += (token==TK_num) ? num : value.find(form)->second;
      break;
      // ---------------------------------
    case ST_M1: case ST_M2: case ST_M3: case ST_M4: case ST_M5:
      if (token != TK_with) {
        TRACE(3,L"Actions for 'thousands' and 'hundreds' state");
        st->units = st->units ? st->units * power.find(token)->second: power.find(token)->second;
      }
      break;
      // ---------------------------------
    case ST_COD:
      if (token==TK_code) 
        st->iscode=CODE;
      else if (token==TK_ord) 
        st->iscode=ORD;
      break;
    default: break;
    }

    TRACE(3,L"State actions completed. bilion="+util::longdouble2wstring(st->bilion)+L" milion="+util::longdouble2wstring(st->milion)+L" units="+util::longdouble2wstring(st->units));
  }


  ///////////////////////////////////////////////////////////////
  ///   Set the appropriate lemma and tag for the 
  ///   new multiword.
  ///////////////////////////////////////////////////////////////

  void numbers_ru::SetMultiwordAnalysis(sentence::iterator i, int fstate, const numbers_status *st) const {
    wstring lemma, tag;

    tag=L"Z00000";

    // Setting the analysis for the nummerical expression
    if (st->iscode==CODE) 
      lemma=i->get_form();
    else 
      if (st->iscode==ORD) 
        {
          wstring form=i->get_form();
          wstring::size_type dashPos = form.find(L"-");
          lemma = form.substr(0, dashPos); 
          tag=L"JJ";  
        }
      else
        lemma=util::longdouble2wstring(st->bilion + st->milion + st->units);

    i->set_analysis(analysis(lemma, tag));
    TRACE(3, L"Analysis set to: " + lemma + L" " + tag);
  }

} // namespace
