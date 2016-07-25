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
#define ST_B   2     // B: got a week day                           (VALID DATE)
#define ST_C   3     // C: got a week day and a comma
#define ST_D   4     // D: got the word "day" from A,B,C 
#define ST_E   5     // E: got day number from B,C,D     
#define ST_Eb  6     // Eb: got a month from A                    (VALID DATE)
#define ST_Ebb 7     // Ebb: got a month from C
#define ST_Ec  8     // Ec: got a day number from A 
#define ST_F   9     // F: got "of" from E (a month may come)
#define ST_Fb  10 
#define ST_G   11    // G: got word "month" from A,F
#define ST_Gb  12    // Gb: got day number from Fb                 (VALID DATE)
#define ST_Gbb 42    // as Gb but not a final state
#define ST_H   13    // H: got "of" from G (a month may come)
#define ST_I   14    // I: got a month name o number from A,F,G,H.  (VALID DATE)
#define ST_Ib  15    // Ib: got a short month name from A.
#define ST_Ic  43    // Ic: got a monthnum: as state I but not a final state
#define ST_J   16    // J: got "of" from J (a year may come)
#define ST_Jb  17    // Jb: got a centnum (num between 13 and 19) from J, K, I, Gb (first two numbers of the year)
#define ST_K   18    // K: got "year" from A,J (a year may come)
#define ST_L   19    // L: got a number from J,K or a date from A.  (VALID DATE)
#define ST_M   20 
#define ST_N   21 
#define ST_O   22 
#define ST_P   23    // P: got "b.c." or "a.c" from L               (VALID DATE)


  //#define S1 15    // S1: got "siglo" from A (a century may come)
  //#define S2 16    // S2: got roman number from S1                (VALID DATE)


  // states for time, after finding a date (e.g. August 23th at a quarter past three)
#define ST_Ha 24    // Ha: got "at" "about" after a valid date
#define ST_AH 25    // AH: got hhmm.                                 (VALID DATE)
#define ST_BH 26    // BH:                                        
#define ST_CH 27    // CH:                                         
#define ST_DH 28    // DH: 
#define ST_EH 29    // EH:                                           (VALID DATE)
#define ST_EHb 30   // EHb:                                          (VALID DATE)
#define ST_FH  31   // FH: 
#define ST_GH  32   // GH:                                           (VALID DATE)

  // states for a time non preceded by a date
#define ST_AH1   33   // AH1                                          (VALID DATE)
#define ST_BH1   34   // BH1: 
#define ST_CH1   35   // CH1: 
#define ST_DH1   36   // DH1: 
#define ST_DH1b  45   
#define ST_EH1   37   // EH1:                                         (VALID DATE)  
#define ST_EH1b  38  
#define ST_EH1c  46  
#define ST_FH1   39   // FH1: 
#define ST_GH1   40   // GH1:                                         (VALID DATE)

  // auxiliar state
#define ST_aux 44

  // stop state
#define ST_STOP 41

  ////////
  // Token codes
#define TK_weekday   1      // weekday: "Monday", "Tuesday", etc
#define TK_daynum    2      // daynum:  1st-31st
#define TK_month     3      // month: "enero", "febrero", "ene." "feb." etc,
#define TK_shmonth   4      // short month: "ene", "feb", "mar" etc,
#define TK_monthnum  5      // monthnum: 1-12
#define TK_number    6      // number: 23, 1976, 543, etc.
#define TK_comma     7      // comma: ","
#define TK_dot       8     // dot: "." 
#define TK_colon     9     // colon:  ":"                  ***not in use***
#define TK_wday      10    // wdia: "día"
#define TK_wmonth    11    // wmonth: "mes"
#define TK_wyear     12    // wyear: "año"
#define TK_wpast     13    // wpast: "pasado", "presente", "proximo"
#define TK_wcentury  14    // wcentury:  "siglo", "sig", "sgl", etc.       ***not in use***
#define TK_roman     15    // roman: XVI, VII, III, XX, etc.              ***not in use***
#define TK_wacdc     16    // wacdc: "b.c.", "a.d."
#define TK_wampm     17    // wacdc: "a.m.", "p.m."
#define TK_hournum   18    // hournum: 0-24
#define TK_minnum    19    // minnum: 0-60
#define TK_wquarter  20    // wquarter: "quarter"
#define TK_whalf     21    // 
#define TK_wto       22    // wto: "to"
#define TK_wmorning  23    // wmorning: 
#define TK_wmidnight 24    // wmidnight: 
#define TK_wat       25    // wat: "at"
#define TK_wof       26    // wof: "of"
#define TK_win       43    // win: "in", "on"
#define TK_wa        27    // wa: "a"
#define TK_wabout    28    // wabout: "about"
#define TK_wthe      29    // wthe: "the"
#define TK_wand      30    // wand: "and", "oh" (for the year)
#define TK_woclock   31    // woclock: "o'clock"
#define TK_wmin      32    // wmin: "minute(s)" "min" "hour" "hours"
#define TK_hour      33    // hour: something matching just first part of RE_TIME1: 3h, 17h
#define TK_hhmm      34    // hhmm: something matching whole RE_TIME1: 3h25min, 3h25m, 3h25minutes,
#define TK_min       35    // min: something matching RE_TIME2: 25min, 3minutes
#define TK_date      36    // date: something matching RE_DATE: 3/ago/1992, 3/8/92, 3/agosto/1992, ...
#define TK_other     37    // other: any other token, not relevant in current state
#define TK_kyearnum  38
#define TK_centnum   39
#define TK_yearnum   40
#define TK_whundred  41


  ///////////////////////////////////////////////////////////////
  ///  Create a dates recognizer for English.
  ///////////////////////////////////////////////////////////////

  dates_en::dates_en(): dates_module(RE_DATE_EN, RE_TIME1_EN, RE_TIME2_EN, RE_ROMAN)
  {
    //Initialize token translation map
    tok.insert(make_pair(L"monday",TK_weekday));     tok.insert(make_pair(L"thursday",TK_weekday));
    tok.insert(make_pair(L"wednesday",TK_weekday));  tok.insert(make_pair(L"tuesday",TK_weekday));
    tok.insert(make_pair(L"friday",TK_weekday));     tok.insert(make_pair(L"saturday",TK_weekday));
    tok.insert(make_pair(L"sunday",TK_weekday));    

    tok.insert(make_pair(L"the",TK_wthe));    
    tok.insert(make_pair(L"of",TK_wof));      tok.insert(make_pair(L"on",TK_wof));     
    tok.insert(make_pair(L"a",TK_wa));        tok.insert(make_pair(L"to",TK_wto));      
    tok.insert(make_pair(L"at",TK_wat));      tok.insert(make_pair(L"about",TK_wabout));
    tok.insert(make_pair(L"in",TK_win));
    tok.insert(make_pair(L"and",TK_wand));    tok.insert(make_pair(L"oh",TK_wand));

    tok.insert(make_pair(L",",TK_comma));        tok.insert(make_pair(L".",TK_dot));
    tok.insert(make_pair(L":",TK_colon));        tok.insert(make_pair(L"day",TK_wday)); 
    tok.insert(make_pair(L"month",TK_wmonth));   tok.insert(make_pair(L"year",TK_wyear)); 
    tok.insert(make_pair(L"current",TK_wpast));  tok.insert(make_pair(L"last",TK_wpast)); 
    tok.insert(make_pair(L"next",TK_wpast));     tok.insert(make_pair(L"coming",TK_wpast));
    tok.insert(make_pair(L"past",TK_wpast));
     
    tok.insert(make_pair(L"half",TK_whalf));    tok.insert(make_pair(L"quarter",TK_wquarter));
    tok.insert(make_pair(L"o'clock",TK_woclock));    
    //tok.insert(make_pair(L"siglo",TK_wcentury)); tok.insert(make_pair(L"s.",TK_wcentury));
    //tok.insert(make_pair(L"sig.",TK_wcentury));  tok.insert(make_pair(L"sgl.",TK_wcentury));
    //tok.insert(make_pair(L"sigl.",TK_wcentury));

    tok.insert(make_pair(L"hundred",TK_whundred));

    tok.insert(make_pair(L"b.c.",TK_wacdc));  tok.insert(make_pair(L"a.d.",TK_wacdc));
    tok.insert(make_pair(L"a.m.",TK_wampm));  tok.insert(make_pair(L"p.m.",TK_wampm));
    tok.insert(make_pair(L"am.",TK_wampm));  tok.insert(make_pair(L"pm.",TK_wampm));
                                                 
    tok.insert(make_pair(L"midnight",TK_wmidnight));   tok.insert(make_pair(L"midday",TK_wmidnight));
    tok.insert(make_pair(L"noon",TK_wmidnight));
    tok.insert(make_pair(L"morning",TK_wmorning));     tok.insert(make_pair(L"afternoon",TK_wmorning));     
    tok.insert(make_pair(L"evening",TK_wmorning));     tok.insert(make_pair(L"night",TK_wmorning));  
             
    tok.insert(make_pair(L"m",TK_wmin));      tok.insert(make_pair(L"m.",TK_wmin));
    tok.insert(make_pair(L"min",TK_wmin));    tok.insert(make_pair(L"min.",TK_wmin));
    tok.insert(make_pair(L"minute",TK_wmin)); tok.insert(make_pair(L"minutes",TK_wmin));
    tok.insert(make_pair(L"hour",TK_wmin)); tok.insert(make_pair(L"hours",TK_wmin));

    tok.insert(make_pair(L"january",TK_month));    tok.insert(make_pair(L"jan",TK_shmonth)); 
    tok.insert(make_pair(L"february",TK_month));   tok.insert(make_pair(L"feb",TK_shmonth));
    tok.insert(make_pair(L"march",TK_month));      tok.insert(make_pair(L"mar",TK_shmonth));
    tok.insert(make_pair(L"april",TK_month));      tok.insert(make_pair(L"apr",TK_shmonth));
    tok.insert(make_pair(L"may",TK_shmonth));       
    tok.insert(make_pair(L"june",TK_month));       tok.insert(make_pair(L"jun",TK_shmonth));
    tok.insert(make_pair(L"july",TK_month));       tok.insert(make_pair(L"jul",TK_shmonth));
    tok.insert(make_pair(L"august",TK_month));     tok.insert(make_pair(L"aug",TK_shmonth));
    tok.insert(make_pair(L"september",TK_month));  tok.insert(make_pair(L"sep",TK_shmonth));
    tok.insert(make_pair(L"october",TK_month));    tok.insert(make_pair(L"oct",TK_shmonth));
    tok.insert(make_pair(L"november",TK_month));   tok.insert(make_pair(L"nov",TK_shmonth));
    tok.insert(make_pair(L"december",TK_month));   tok.insert(make_pair(L"dec",TK_shmonth));
    tok.insert(make_pair(L"novemb",TK_shmonth));   tok.insert(make_pair(L"sept",TK_shmonth));
    tok.insert(make_pair(L"decemb",TK_shmonth));   tok.insert(make_pair(L"septemb",TK_shmonth)); 

    // day numbers
    tok.insert(make_pair(L"first",TK_daynum));       tok.insert(make_pair(L"second",TK_daynum));
    tok.insert(make_pair(L"third",TK_daynum));       tok.insert(make_pair(L"fourth",TK_daynum)); 
    tok.insert(make_pair(L"fifth",TK_daynum));       tok.insert(make_pair(L"sixth",TK_daynum));
    tok.insert(make_pair(L"seventh",TK_daynum));     tok.insert(make_pair(L"eighth",TK_daynum)); 
    tok.insert(make_pair(L"ninth",TK_daynum));       tok.insert(make_pair(L"tenth",TK_daynum));
    tok.insert(make_pair(L"eleventh",TK_daynum));   tok.insert(make_pair(L"twelfth",TK_daynum)); 
    tok.insert(make_pair(L"thirteenth",TK_daynum));  tok.insert(make_pair(L"fourteenth",TK_daynum));
    tok.insert(make_pair(L"fifteenth",TK_daynum));   tok.insert(make_pair(L"sixteenth",TK_daynum)); 
    tok.insert(make_pair(L"seventeenth",TK_daynum)); tok.insert(make_pair(L"eighteenth",TK_daynum));
    tok.insert(make_pair(L"nineteenth",TK_daynum));  tok.insert(make_pair(L"twentieth",TK_daynum)); 
    tok.insert(make_pair(L"twenty-first",TK_daynum));  tok.insert(make_pair(L"twenty-second",TK_daynum)); 
    tok.insert(make_pair(L"twenty-third",TK_daynum));  tok.insert(make_pair(L"twenty-fourth",TK_daynum)); 
    tok.insert(make_pair(L"twenty-fifth",TK_daynum));  tok.insert(make_pair(L"twenty-sixth",TK_daynum)); 
    tok.insert(make_pair(L"twenty-seventh",TK_daynum)); tok.insert(make_pair(L"twenty-eighth",TK_daynum)); 
    tok.insert(make_pair(L"twenty-ninth",TK_daynum));   
    tok.insert(make_pair(L"thirtieth",TK_daynum));     tok.insert(make_pair(L"thirty-first",TK_daynum)); 
             
    // initialize map with month numeric values
    nMes.insert(make_pair(L"january",1));    nMes.insert(make_pair(L"jan",1));
    nMes.insert(make_pair(L"february",2));   nMes.insert(make_pair(L"feb",2));
    nMes.insert(make_pair(L"march",3));      nMes.insert(make_pair(L"mar",3));
    nMes.insert(make_pair(L"april",4));      nMes.insert(make_pair(L"apr",4));
    nMes.insert(make_pair(L"may",5));       
    nMes.insert(make_pair(L"june",6));       nMes.insert(make_pair(L"jun",6));
    nMes.insert(make_pair(L"july",7));       nMes.insert(make_pair(L"jul",7));
    nMes.insert(make_pair(L"august",8));     nMes.insert(make_pair(L"aug",8));
    nMes.insert(make_pair(L"september",9));  nMes.insert(make_pair(L"sep",9));
    nMes.insert(make_pair(L"october",10));   nMes.insert(make_pair(L"oct",10));
    nMes.insert(make_pair(L"november",11));  nMes.insert(make_pair(L"nov",11));
    nMes.insert(make_pair(L"december",12));  nMes.insert(make_pair(L"dec",12));
    nMes.insert(make_pair(L"novemb",11));    nMes.insert(make_pair(L"sept",9));
    nMes.insert(make_pair(L"decemb",12));    nMes.insert(make_pair(L"septemb",9));  
      
    // initialize map with weekday values 
    nDia.insert(make_pair(L"sunday",L"G"));     nDia.insert(make_pair(L"monday",L"L"));
    nDia.insert(make_pair(L"tuesday",L"M"));    nDia.insert(make_pair(L"wednesday",L"X"));
    nDia.insert(make_pair(L"thursday",L"J"));   nDia.insert(make_pair(L"friday",L"V"));
    nDia.insert(make_pair(L"saturday",L"S"));

    // initialize map with day numeric values 
    numDay.insert(make_pair(L"first",1));       numDay.insert(make_pair(L"second",2));
    numDay.insert(make_pair(L"third",3));       numDay.insert(make_pair(L"fourth",4)); 
    numDay.insert(make_pair(L"fifth",5));       numDay.insert(make_pair(L"sixth",6));
    numDay.insert(make_pair(L"seventh",7));     numDay.insert(make_pair(L"eighth",8)); 
    numDay.insert(make_pair(L"ninth",9));       numDay.insert(make_pair(L"tenth",10));
    numDay.insert(make_pair(L"eleventh",11));   numDay.insert(make_pair(L"twelfth",12)); 
    numDay.insert(make_pair(L"thirteenth",13));  numDay.insert(make_pair(L"fourteenth",14));
    numDay.insert(make_pair(L"fifteenth",15));   numDay.insert(make_pair(L"sixteenth",16)); 
    numDay.insert(make_pair(L"seventeenth",17)); numDay.insert(make_pair(L"eighteenth",18));
    numDay.insert(make_pair(L"nineteenth",19));  numDay.insert(make_pair(L"twentieth",20)); 
    numDay.insert(make_pair(L"twenty-first",21));  numDay.insert(make_pair(L"twenty-second",22)); 
    numDay.insert(make_pair(L"twenty-third",23));  numDay.insert(make_pair(L"twenty-fourth",24)); 
    numDay.insert(make_pair(L"twenty-fifth",25));  numDay.insert(make_pair(L"twenty-sixth",26)); 
    numDay.insert(make_pair(L"twenty-seventh",27)); numDay.insert(make_pair(L"twenty-eighth",28)); 
    numDay.insert(make_pair(L"twenty-ninth",29));   
    numDay.insert(make_pair(L"thirtieth",30));     numDay.insert(make_pair(L"thirty-first",31)); 

    // Initialize special state attributes
    initialState=ST_A; stopState=ST_STOP;

    // Initialize Final state set 
    Final.insert(ST_B);   Final.insert(ST_I);     Final.insert(ST_L);     Final.insert(ST_E);
    Final.insert(ST_P);   Final.insert(ST_Eb);    Final.insert(ST_Gb);    Final.insert(ST_Gbb);  
    Final.insert(ST_AH);  Final.insert(ST_CH);    Final.insert(ST_GH);    Final.insert(ST_EH); 
    Final.insert(ST_EHb); Final.insert(ST_AH1);   Final.insert(ST_GH1);   Final.insert(ST_EH1);

    // Initialize transition table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;

    // State A
    trans[ST_A][TK_weekday]=ST_B;   trans[ST_A][TK_wday]=ST_D;   trans[ST_A][TK_wmonth]=ST_G;
    trans[ST_A][TK_wyear]=ST_K;     trans[ST_A][TK_win]=ST_J;    trans[ST_A][TK_date]=ST_L;     
    trans[ST_A][TK_month]=ST_Eb;    trans[ST_A][TK_shmonth]=ST_Ib;  //trans[ST_A][TK_wcentury]=ST_S1; 
    trans[ST_A][TK_wpast]=ST_A;     trans[ST_A][TK_daynum]=ST_Ec;
    trans[ST_A][TK_hhmm]=ST_AH1;    trans[ST_A][TK_wmidnight]=ST_GH1;  
    trans[ST_A][TK_hour]=ST_CH1;    trans[ST_A][TK_hournum]=ST_CH1; 
    trans[ST_A][TK_min]=ST_CH1;     trans[ST_A][TK_minnum]=ST_CH1; 
    trans[ST_A][TK_whalf]=ST_BH1;    trans[ST_A][TK_wquarter]=ST_BH1; 
    // State B+
    trans[ST_B][TK_daynum]=ST_E;      trans[ST_B][TK_comma]=ST_C; 
    trans[ST_B][TK_wday]=ST_D;    
    trans[ST_B][TK_wmidnight]=ST_GH;  trans[ST_B][TK_wmorning]=ST_GH;  
    trans[ST_B][TK_wat]=ST_Ha;        trans[ST_B][TK_wabout]=ST_Ha;    trans[ST_B][TK_win]=ST_FH;
    trans[ST_B][TK_wthe]=ST_B;        trans[ST_B][TK_hhmm]=ST_AH1;     trans[ST_B][TK_date]=ST_L;     
    // State C
    trans[ST_C][TK_daynum]=ST_E;      trans[ST_C][TK_wday]=ST_D; 
    trans[ST_C][TK_month]=ST_Ebb;     trans[ST_C][TK_shmonth]=ST_Ebb;   
    trans[ST_C][TK_hhmm]=ST_AH1;      trans[ST_C][TK_date]=ST_L;     
    // State D
    trans[ST_D][TK_daynum]=ST_E;
    trans[ST_D][TK_hhmm]=ST_AH1;      trans[ST_D][TK_date]=ST_L;     
    // State E
    trans[ST_E][TK_wof]=ST_F;  
    trans[ST_E][TK_month]=ST_I;       trans[ST_E][TK_shmonth]=ST_I;
    trans[ST_E][TK_monthnum]=ST_Ic;
    trans[ST_E][TK_wmidnight]=ST_GH;  trans[ST_E][TK_wmorning]=ST_GH;  
    trans[ST_E][TK_wat]=ST_Ha;        trans[ST_E][TK_wabout]=ST_Ha;    trans[ST_E][TK_win]=ST_FH;
    // State Eb+
    trans[ST_Eb][TK_wthe]=ST_Fb;  
    trans[ST_Eb][TK_wof]=ST_J;         trans[ST_Eb][TK_comma]=ST_J; 
    trans[ST_Eb][TK_kyearnum]=ST_L;    trans[ST_Eb][TK_centnum]=ST_Jb;
    trans[ST_Eb][TK_wmidnight]=ST_GH;  trans[ST_Eb][TK_wmorning]=ST_GH;  
    trans[ST_Eb][TK_wat]=ST_Ha;        trans[ST_Eb][TK_wabout]=ST_Ha;     trans[ST_Eb][TK_win]=ST_FH;
    trans[ST_Eb][TK_daynum]=ST_Gbb;
    // State Ebb
    trans[ST_Ebb][TK_wthe]=ST_Fb; 
    trans[ST_Ebb][TK_daynum]=ST_Gb;
    // State Ec
    trans[ST_Ec][TK_wof]=ST_F;  
    trans[ST_Ec][TK_month]=ST_I;  trans[ST_Ec][TK_shmonth]=ST_I; trans[ST_Ec][TK_monthnum]=ST_Ic;
    // State F
    trans[ST_F][TK_wpast]=ST_F;  trans[ST_F][TK_wthe]=ST_F;
    trans[ST_F][TK_month]=ST_I;   trans[ST_F][TK_shmonth]=ST_I; 
    trans[ST_F][TK_wmonth]=ST_G; trans[ST_F][TK_monthnum]=ST_Ic;
    // State Fb
    trans[ST_Fb][TK_daynum]=ST_Gb; 
    // State G
    trans[ST_G][TK_wof]=ST_H;   trans[ST_G][TK_monthnum]=ST_Ic;   
    trans[ST_G][TK_month]=ST_I; trans[ST_G][TK_shmonth]=ST_I;
    // State Gb+
    trans[ST_Gb][TK_comma]=ST_J;       trans[ST_Gb][TK_wof]=ST_J;
    trans[ST_Gb][TK_kyearnum]=ST_L;    trans[ST_Gb][TK_centnum]=ST_Jb;
    trans[ST_Gb][TK_wmidnight]=ST_GH;  trans[ST_Gb][TK_wmorning]=ST_GH;  
    trans[ST_Gb][TK_wat]=ST_Ha;        trans[ST_Gb][TK_wabout]=ST_Ha;    trans[ST_Gb][TK_win]=ST_FH;
    // State Gbb
    trans[ST_Gbb][TK_comma]=ST_J;       trans[ST_Gbb][TK_wof]=ST_J;
    trans[ST_Gbb][TK_kyearnum]=ST_L;    trans[ST_Gbb][TK_centnum]=ST_Jb;
    trans[ST_Gbb][TK_wmidnight]=ST_GH;  trans[ST_Gbb][TK_wmorning]=ST_GH;  
    trans[ST_Gbb][TK_wat]=ST_Ha;        trans[ST_Gbb][TK_wabout]=ST_Ha;    trans[ST_Gbb][TK_win]=ST_FH;
    // State H
    trans[ST_H][TK_month]=ST_I; trans[ST_H][TK_shmonth]=ST_I; trans[ST_H][TK_monthnum]=ST_Ic;   
    // State I+
    trans[ST_I][TK_wof]=ST_J;         trans[ST_I][TK_comma]=ST_J;
    trans[ST_I][TK_kyearnum]=ST_L;    trans[ST_I][TK_centnum]=ST_Jb;
    trans[ST_I][TK_wmidnight]=ST_GH;  trans[ST_I][TK_wmorning]=ST_GH;  
    trans[ST_I][TK_wat]=ST_Ha;        trans[ST_I][TK_wabout]=ST_Ha;    trans[ST_I][TK_win]=ST_FH;
    // State Ib
    trans[ST_Ib][TK_wof]=ST_J;   trans[ST_Ib][TK_comma]=ST_J; 
    trans[ST_Ib][TK_wthe]=ST_Fb;  
    trans[ST_Ib][TK_kyearnum]=ST_L;    trans[ST_Ib][TK_centnum]=ST_Jb;
    trans[ST_Ib][TK_wmidnight]=ST_GH;  trans[ST_Ib][TK_wmorning]=ST_GH;  
    trans[ST_Ib][TK_wat]=ST_Ha;        trans[ST_Ib][TK_wabout]=ST_Ha;  
    trans[ST_Ib][TK_daynum]=ST_Gbb;    trans[ST_Ib][TK_win]=ST_FH;
    // State Ic
    trans[ST_Ic][TK_wof]=ST_J;         trans[ST_Ic][TK_comma]=ST_J;
    trans[ST_Ic][TK_kyearnum]=ST_L;    trans[ST_Ic][TK_centnum]=ST_Jb;
    trans[ST_Ic][TK_wmidnight]=ST_GH;  trans[ST_Ic][TK_wmorning]=ST_GH;  
    trans[ST_Ic][TK_wat]=ST_Ha;        trans[ST_Ic][TK_wabout]=ST_Ha;     trans[ST_Ic][TK_win]=ST_FH;

    // State J
    trans[ST_J][TK_wpast]=ST_J;     trans[ST_J][TK_wthe]=ST_J;
    trans[ST_J][TK_kyearnum]=ST_L;  trans[ST_J][TK_centnum]=ST_Jb;  trans[ST_J][TK_wyear]=ST_K;
    // State Jb
    trans[ST_Jb][TK_yearnum]=ST_L;     trans[ST_Jb][TK_wand]=ST_M;     trans[ST_Jb][TK_whundred]=ST_N;
    trans[ST_Jb][TK_comma]=ST_J;       trans[ST_Jb][TK_wof]=ST_J;
    trans[ST_Jb][TK_centnum]=ST_Jb;
    trans[ST_Jb][TK_wmidnight]=ST_GH;  trans[ST_Jb][TK_wmorning]=ST_GH;    
    trans[ST_Jb][TK_wat]=ST_Ha;        trans[ST_Jb][TK_wabout]=ST_Ha;    trans[ST_Jb][TK_win]=ST_FH;
    trans[ST_Jb][TK_kyearnum]=ST_L;  
    trans[ST_Jb][TK_other]=ST_aux;    trans[ST_Jb][TK_dot]=ST_aux;  
    // State K
    trans[ST_K][TK_centnum]=ST_Jb; trans[ST_K][TK_kyearnum]=ST_L;
    // State L+
    trans[ST_L][TK_wacdc]=ST_P;
    trans[ST_L][TK_wmidnight]=ST_GH;  trans[ST_L][TK_wmorning]=ST_GH;  
    trans[ST_L][TK_wat]=ST_Ha;        trans[ST_L][TK_wabout]=ST_Ha;   trans[ST_L][TK_win]=ST_FH;
    // State M
    trans[ST_M][TK_wand]=ST_L;  trans[ST_M][TK_yearnum]=ST_L;
    // State N
    trans[ST_N][TK_wand]=ST_O;
    // State O
    trans[ST_O][TK_yearnum]=ST_L;
    // State P+
    trans[ST_P][TK_wmidnight]=ST_GH;  trans[ST_P][TK_wmorning]=ST_GH;  
    trans[ST_P][TK_wat]=ST_Ha;        trans[ST_P][TK_wabout]=ST_Ha;    trans[ST_P][TK_win]=ST_FH;

    // State S1
    //trans[ST_S1][TK_roman]=ST_S2; trans[ST_S1][TK_dot]=ST_S1;
    // State S2+
    //trans[ST_S2][TK_wacdc]=ST_S2;

    // State Ha
    trans[ST_Ha][TK_wa]=ST_Ha; trans[ST_Ha][TK_wabout]=ST_Ha; trans[ST_Ha][TK_wthe]=ST_Ha; 
    trans[ST_Ha][TK_whalf]=ST_BH; trans[ST_Ha][TK_wquarter]=ST_BH;  
    trans[ST_Ha][TK_hour]=ST_CH;  trans[ST_Ha][TK_minnum]=ST_CH; trans[ST_Ha][TK_min]=ST_CH;
    trans[ST_Ha][TK_hhmm]=ST_AH; 
    trans[ST_Ha][TK_wmidnight]=ST_GH; trans[ST_Ha][TK_wmorning]=ST_GH;  
    // State AH+
    trans[ST_AH][TK_wampm]=ST_GH; 
    trans[ST_AH][TK_wof]=ST_FH; trans[ST_AH][TK_wat]=ST_FH; trans[ST_AH][TK_win]=ST_FH;   
    // State BH
    trans[ST_BH][TK_wpast]=ST_DH; trans[ST_BH][TK_wto]=ST_DH; 
    // State CH
    trans[ST_CH][TK_wpast]=ST_DH;     trans[ST_CH][TK_wto]=ST_DH;
    trans[ST_CH][TK_minnum]=ST_EHb;   trans[ST_CH][TK_min]=ST_EHb; 
    trans[ST_CH][TK_woclock]=ST_AH; 
    trans[ST_CH][TK_wof]=ST_FH;       trans[ST_CH][TK_wat]=ST_FH;   trans[ST_CH][TK_win]=ST_FH;
    trans[ST_CH][TK_wmin]=ST_CH;      trans[ST_CH][TK_wand]=ST_CH; 
    trans[ST_CH][TK_other]=ST_aux;    trans[ST_CH][TK_dot]=ST_aux;  
    // State DH
    trans[ST_DH][TK_hour]=ST_EH;  trans[ST_DH][TK_hournum]=ST_EH;
    // State EH+
    trans[ST_EH][TK_wampm]=ST_GH; 
    trans[ST_EH][TK_wof]=ST_FH;  trans[ST_EH][TK_wat]=ST_FH;  trans[ST_EH][TK_win]=ST_FH;  
    trans[ST_EH][TK_wmidnight]=ST_GH; trans[ST_EH][TK_wmorning]=ST_GH;  
    trans[ST_EH][TK_wmin]=ST_EH; 
    // State EHb+
    trans[ST_EHb][TK_wmin]=ST_EHb; 
    trans[ST_EHb][TK_wampm]=ST_GH; trans[ST_EHb][TK_wmidnight]=ST_GH; trans[ST_EHb][TK_wmorning]=ST_GH;  
    trans[ST_EHb][TK_wat]=ST_FH; trans[ST_EHb][TK_wof]=ST_FH; trans[ST_EHb][TK_win]=ST_FH;   
    // State FH
    trans[ST_FH][TK_wthe]=ST_FH; trans[ST_FH][TK_wmorning]=ST_GH; trans[ST_FH][TK_wmidnight]=ST_GH;
    // State GH+
    trans[ST_GH][TK_wat]=ST_Ha; trans[ST_GH][TK_wabout]=ST_Ha;

    // State AH1
    trans[ST_AH1][TK_wof]=ST_A;
    trans[ST_AH1][TK_wampm]=ST_GH1; 
    trans[ST_AH1][TK_wat]=ST_FH1; trans[ST_AH1][TK_win]=ST_FH1;
    // State BH1
    trans[ST_BH1][TK_wpast]=ST_DH1; trans[ST_BH1][TK_wto]=ST_DH1;
    // State CH1
    trans[ST_CH1][TK_wpast]=ST_DH1b; trans[ST_CH1][TK_wto]=ST_DH1b;
    trans[ST_CH1][TK_minnum]=ST_EH1b;  trans[ST_CH1][TK_min]=ST_EH1b;
    trans[ST_CH1][TK_woclock]=ST_AH1;
    trans[ST_CH1][TK_wat]=ST_FH1;
    trans[ST_CH1][TK_wmin]=ST_CH1;  trans[ST_CH1][TK_wand]=ST_CH1;
    trans[ST_CH1][TK_wof]=ST_F;
    trans[ST_CH1][TK_month]=ST_I;  trans[ST_CH1][TK_shmonth]=ST_I;
    // State DH1
    trans[ST_DH1][TK_hournum]=ST_EH1;  trans[ST_DH1][TK_hour]=ST_EH1;
    // State DH1b
    trans[ST_DH1b][TK_hournum]=ST_EH1c;  trans[ST_DH1b][TK_hour]=ST_EH1c;
    // State EH1+
    trans[ST_EH1][TK_wampm]=ST_GH1;
    trans[ST_EH1][TK_wof]=ST_A;  trans[ST_EH1][TK_wat]=ST_FH1; trans[ST_EH1][TK_win]=ST_FH1;
    // State EH1b
    trans[ST_EH1b][TK_wmin]=ST_EH1b;  trans[ST_EH1b][TK_wampm]=ST_GH1;
    trans[ST_EH1b][TK_wof]=ST_A;  trans[ST_EH1b][TK_wat]=ST_FH1; trans[ST_EH1b][TK_win]=ST_FH1;
    trans[ST_EH1b][TK_wmin]=ST_EH1b; 
    // State EH1c
    trans[ST_EH1c][TK_wampm]=ST_GH1;
    trans[ST_EH1c][TK_wof]=ST_A;  trans[ST_EH1c][TK_wat]=ST_FH1; trans[ST_EH1c][TK_win]=ST_FH1;
    // State FH1
    trans[ST_FH1][TK_wthe]=ST_FH1;      
    trans[ST_FH1][TK_wmorning]=ST_GH1; trans[ST_FH1][TK_wmidnight]=ST_GH1;
    // State GH1+
    trans[ST_GH1][TK_wof]=ST_A;  trans[ST_GH1][TK_win]=ST_A;

    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////
  int dates_en::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
    wstring form;
    int token,value;
    map<wstring,int>::const_iterator im;

    dates_status *st = (dates_status *)se.get_processing_status();
    st->rem.clear(); // clear any previous RE matches

    form = j->get_lc_form();

    token = TK_other;  
    // check if it is a known token
    im = tok.find(form);
    if (im!=tok.end()) {
      token = (*im).second;
    }
  
    TRACE(3,L"Next word form is: ["+form+L"] token="+util::int2wstring(token));     

    // if the token was in the table, we're done
    if (token != TK_other) return (token);

    // Token not found in translation table, let's have a closer look.

    // look if any lemma is an ordinal adjective
    bool ordinal=false;
    word::iterator a;
    for (a=j->begin(); a!=j->end() && !ordinal; a++) {
      ordinal = (a->get_tag()==L"JJ" && util::RE_all_digits.search(a->get_lemma()) &&
                 util::wstring2int(a->get_lemma())>0 && util::wstring2int(a->get_lemma())<=31);
      TRACE(3, L"studying possible analysis of word ["+form+L"]:"+a->get_tag()+L" "+a->get_lemma());
    }
    // if any, consider this a fraction word (fifth, eleventh, ...) 
    if (ordinal) {
      a--;
      j->unselect_all_analysis();
      j->select_analysis(a);
      token=TK_daynum;
      value=util::wstring2int(j->get_lemma());
      TRACE(3,L"Ordinal value of form: "+util::int2wstring(value)+L". New token: "+util::int2wstring(token));
      return (token);
    } 

    // check to see if it is a number
    value=0;
    if (j->get_n_analysis() && j->get_tag()==L"Z") {
      token = TK_number;
      value = util::wstring2int(j->get_lemma());
      TRACE(3,L"Numerical value of form: "+util::int2wstring(value));
    }

    // determine how to interpret that number, or if not number,
    // check for specific regexps.
    switch (state) {
      // --------------------------------
    case ST_A: 
      TRACE(3,L"In state A");     
      if (token==TK_number && value>=0 && value<=59) {
        token = TK_minnum; // it can also be an hournum
      }
      else if (RE_Date.search(form,st->rem)) {
        TRACE(3,L"Match DATE regex. ");
        token = TK_date;
      }
      else if (RE_Time1.search(form,st->rem)) {
        if (st->rem.size()>=3) {   //  if (RE_Time1.Match(2)!="")
          TRACE(3,L"Match TIME1 regex (hour+min)");
          token = TK_hhmm;
        }
        else { 
          TRACE(3,L"Partial match TIME1 regex (hour)");
          token = TK_hour;
        }  
      }
      else if (RE_Time2.search(form,st->rem)) {
        token = TK_min;
      }
    
      break;
      // --------------------------------
    case ST_B:
    case ST_C:
    case ST_D:
      TRACE(3,L"In state B/C/D");
      if (token==TK_number && value>=0 && value<=31){
        token = TK_daynum;
      }
      else if (RE_Date.search(form,st->rem)) {
        TRACE(3,L"Match DATE regex. ");
        token = TK_date;
      }
      else if (RE_Time1.search(form,st->rem)) {
        if (st->rem.size()>=3) {   //  if (RE_Time1.Match(2)!="")
          TRACE(3,L"Match TIME1 regex (hour+min)");
          token = TK_hhmm;
        }
        else { 
          TRACE(3,L"Partial match TIME1 regex (hour)");
          token = TK_hour;
        }  
      }
      else if (RE_Time2.search(form,st->rem)) {
        token = TK_min;
      }
      
      break;
      // --------------------------------
    case ST_Ebb:
    case ST_Fb:
      TRACE(3,L"In state B/C/D");
      if (token==TK_number && value>=0 && value<=31){
        token = TK_daynum;
      }
      break;
      // --------------------------------    
    case ST_E:
    case ST_Ec:
    case ST_F:
    case ST_G:
    case ST_H:
      TRACE(3,L"In state E/Ec/F/G/H");     
      if (token==TK_number && value>=1 && value<=12) 
        token = TK_monthnum;
      break;
    
      // --------------------------------
    case ST_Eb:
      TRACE(3,L"In state Eb");     
      if (token==TK_number && (value>=10 && value<=99))  // years til 9999, could also be a daynumber
        token = TK_centnum;       
      else if (token==TK_number && value>=100)
        token = TK_kyearnum;
      else if (token==TK_number && value<10 && value>0)
        token = TK_daynum;
      break;      

      // -----------------------------------
    case ST_Gb:
    case ST_Gbb:
    case ST_K:
    case ST_J:
    case ST_I:
    case ST_Ic:
      TRACE(3,L"In state Gb/Gbb/K/J/I");     
      if (token==TK_number && (value>=10 && value<=99))  // years til 9999
        token = TK_centnum;       
      else if (token==TK_number && value>=100)
        token = TK_kyearnum;
      break;      

      // --------------------------------
    case ST_Jb:
      TRACE(3,L"In state Jb");     
      if (token==TK_number && (value>=1 && value<=99))  // decada
        token = TK_yearnum;       
      else if (token==TK_number && value>=100)
        token = TK_kyearnum;
      break;      
        
      // --------------------------------
    case ST_M:
      TRACE(3,L"In state M");     
      if (token==TK_number && value>=1 && value<=99)  // unit
        token = TK_yearnum;       

      break;      

      // --------------------------------
    case ST_O:
      TRACE(3,L"In state O");     
      if (token==TK_number && (value>=1 && value<=99))  // decada
        token = TK_yearnum;       
        
      break;      

      // --------------------------------
    case ST_Ha:
      TRACE(3,L"In state Ha");     
      if (token==TK_number && value>=0 && value<=60) {
        token = TK_minnum;
      }
      else if (RE_Time1.search(form,st->rem)) {
        if (st->rem.size()>=3) {   //  if (RE_Time1.Match(2)!="")
          TRACE(3,L"Match TIME1 regex (hour+min)");
          token = TK_hhmm;
        }
        else{
          TRACE(3,L"Partial match TIME1 regex (hour)");
          token = TK_hour;
        }       
      }
      break;
      // --------------------------------
    case ST_CH:
    case ST_CH1:
      TRACE(3,L"In state CH/CH1");
      if (token==TK_number && value>=0 && value<=60) {
        token=TK_minnum;
      }
      else if (RE_Time2.search(form,st->rem)) {
        TRACE(3,L"Match TIME2 regex (minutes)");
        token = TK_min;
      }    
      break;

      // -------------------------------   
    case ST_DH:
    case ST_DH1:
    case ST_DH1b:
      TRACE(3,L"In state DH/DH1/DH1b");
      if (token==TK_number && value>=0 && value<=24) {
        token = TK_hournum;
      }
      else if (RE_Time1.search(form,st->rem)) {
        if (st->rem.size()>=3) {   //  if (RE_Time1.Match(2)!="")
          TRACE(3,L"Match TIME1 regex (hour+min)");
          token = TK_hhmm;
        }
        else{
          TRACE(3,L"Partial match TIME1 regex (hour)");
          token = TK_hour;
        }
      }
      break;
      // --------------------------------  
    default: break;
    }
  
    TRACE(3,L"Leaving state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)); 
    return (token);
  }


  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state with an informative 
  ///  token (day, year, month, etc) store that part of the date.
  ///////////////////////////////////////////////////////////////

  void dates_en::StateActions(int origin, int state, int token, sentence::const_iterator j, dates_status *st) const {
    wstring form;
    int value;
    map<wstring,int>::const_iterator im;

    form = j->get_lc_form();
    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ["+form+L"]");

    // get token numerical value, if any
    value=0;
    if ((token==TK_number || token==TK_daynum || token==TK_monthnum ||
         token==TK_hournum || token==TK_minnum || token==TK_kyearnum ||
         token==TK_centnum || token==TK_yearnum) &&
        j->get_n_analysis() && j->get_tag()==L"Z") {
      value = util::wstring2int(j->get_lemma());
    }
    else if (token==TK_daynum){
      // if its in the nDay map get value from there
      map<wstring,int>::const_iterator k=numDay.find(form);
      if (k!=numDay.end())
        value=numDay.find(form)->second;
      else
        value = util::wstring2int(j->get_lemma());
    }

    // State actions
    switch (state) {
      // ---------------------------------
    case ST_B:
      TRACE(3,L"Actions for state B");
      if (token==TK_weekday)
        st->weekday=nDia.find(form)->second;
      break;
      // ---------------------------------
    case ST_E:
      TRACE(3,L"Actions for state E");
      if (token==TK_daynum) {
        st->day=util::int2wstring(value);
      }
      break;
      // ---------------------------------
    case ST_Ec:
    case ST_Gb:
      TRACE(3,L"Actions for state Ec/Gb");
      if (token==TK_daynum) {
        st->day=util::int2wstring(value);
      }
      break;
      // ---------------------------------
    case ST_Gbb:
      TRACE(3,L"Actions for state Gbb");
      if (token==TK_daynum) {
        st->daytemp=value;
        st->inGbb=true;
        TRACE(3, L"in Gbb "+util::int2wstring(st->inGbb)+L" daytemp "+util::int2wstring(st->daytemp));
        st->day=util::int2wstring(value);
      }
      break;
      // ---------------------------------
    case ST_Eb:
    case ST_Ebb:
      TRACE(3,L"Actions for state Eb/Ebb");
      if (token==TK_month || token==TK_shmonth)
        st->month = util::int2wstring(nMes.find(form)->second);
      break;
      // ---------------------------------
    case ST_F:
      TRACE(3,L"Actions for state F");
      if (origin==ST_CH1) { //what we found in CH1 was a day
        st->day=st->minute;
        st->minute=UNKNOWN_SYMB;
      }
      else if (origin==ST_E)
        st->day= util::int2wstring(st->temp);
      break;
      // ---------------------------------   
    case ST_I:
    case ST_Ib:
    case ST_Ic:
      TRACE(3,L"Actions for state I/Ib/Ic");
      if (origin==ST_CH1) { //what we found in CH1 was a day
        st->day=util::int2wstring(st->temp);
      }
      else if (origin==ST_E)
        st->day= util::int2wstring(st->temp);

      if (token==TK_monthnum)
        st->month = j->get_lemma();
      else if (token==TK_month || token==TK_shmonth)
        st->month = util::int2wstring(nMes.find(form)->second);
      break; 

      // ---------------------------------
    case ST_Jb:
      TRACE(3,L"Actions for state Jb");
      if (token==TK_centnum) {
        // received first part of a year (century number between 10 and 99)
        //  or a day number
        st->temp=value;
      }
      else 
        st->temp=0;
      break;
      // ---------------------------------
    case ST_J:
      TRACE(3,L"Actions for state J");
      if (origin==ST_Jb) {
        // the centnum received in Jb (temp) was the daynumber
        st->daytemp=st->temp;
        st->inGbb=true;
        TRACE(3, L"in Gbb "+util::int2wstring(st->inGbb)+L" daytemp "+util::int2wstring(st->daytemp));
      }
      if (origin==ST_A and token==TK_win)  // in_1982 --> 1982
        st->shiftbegin=1;    
      break;
      // ---------------------------------
    case ST_L:
      TRACE(3,L"Actions for state L");
      if (token==TK_yearnum) {
        if (origin==ST_Jb || origin==ST_M || origin==ST_O) {
          // received second part of the year (decade number between 1 and 99)
          //  year is centnum*100+value
          st->temp=st->temp*100+value;
          st->year=util::int2wstring(st->temp);
        }
      }
      else if (token==TK_kyearnum) { 
        // received year 2000 or bigger (also accepted 100-1100)
        st->year=util::int2wstring(value);
        // temp is the day seen from Gb to Jb
        if (origin==ST_Gb || origin==ST_Gbb || origin==ST_Jb)
          st->day=util::int2wstring(st->temp);
      }
      //if we passed trough Gbb daytemp is the day
      if (st->inGbb)
        st->day=util::int2wstring(st->daytemp);

      if (token==TK_date) { // rem contains matches for RE_Date
        // day number
        st->day=st->rem[1]; // RE_Date.Match(1);
        // to unify notation (01 -> 1)
        st->day=normalize(st->day);
        // month number (translating month name if necessary) 
        im = nMes.find(util::lowercase(st->rem[2]));  // RE_Date.Match(2)
        if (im!=nMes.end())
          st->month = util::int2wstring((*im).second);
        else {
          st->month = st->rem[2];  // RE_Date.Match(2);
          // to unify notation (01 -> 1) 
          st->month = normalize(st->month);
        }
        // year number
        st->year = st->rem[3];  // RE_Date.Match(3);
        // if year has only two digits, it can be 19xx or 20xx 
        if (util::wstring2int(st->year)>=0 && util::wstring2int(st->year)<=20) st->year=L"20"+st->year;
        else if (util::wstring2int(st->year)>=50 && util::wstring2int(st->year)<=99) st->year=L"19"+st->year;
      }
      break;
      // ---------------------------------
    case ST_P:
      TRACE(3,L"Actions for state P");
      if (form==L"b.c."){
        //if (century!="") century="-"+century;   
        if (st->year!=UNKNOWN_SYMB) st->year=L"-"+st->year;
      }
      break;

      // ---------------------------------
      // time
      // ---------------------------------
    case ST_BH:
    case ST_BH1:
      TRACE(3,L"Actions for state BH/BH1");
    
      if (token==TK_whalf) 
        st->minute=util::int2wstring(30);
      else if (token==TK_wquarter)
        st->minute=util::int2wstring(15);

      break;
      // ---------------------------------
    case ST_CH:
    case ST_CH1:
      TRACE(3,L"Actions for state CH/CH1"); 
      if (token==TK_minnum) {
        st->temp=value; // can be a minute, hour or day
      }
      else if (token==TK_min) { // rem contains matches for RE_Time2
        st->minute = st->rem[1];  // Time2.Match(1);
        st->temp=util::wstring2int(st->minute); // to be used in DH, DH1 or DH1b if it is 15min *to* 12.
      }
      else if (token==TK_hour) { // rem contains matches for RE_Time1
        st->hour=st->rem[1];  // RE_Time1.Match(1);
      }
      break;
      // ---------------------------------
    case ST_DH:
    case ST_DH1:
    case ST_DH1b:
      TRACE(3,L"Actions for state DH/DH1/DH1b");

      if (origin==ST_CH1 || origin==ST_CH) //what we stored in CH1 was a minute
        st->minute=util::int2wstring(st->temp);

      if (token==TK_wpast)
        st->sign=1;
      else if (token==TK_wto)
        st->sign=-1;

      break;

      // ---------------------------------
    case ST_EH:
    case ST_EH1:
    case ST_EH1c:
      TRACE(3,L"Actions for state EH/EH1/EH1c");
      if (token==TK_hournum){
        if (value==24 || value==0) {
          st->meridian=L"am";
          st->hour=L"0";
        }
        else if (value>12) { 
          st->meridian=L"pm";
          st->hour=util::int2wstring(value-12);
        }
        else st->hour=util::int2wstring(value);
      }
      else if (token==TK_hour) { // rem contains matches for RE_Time1
        st->hour=st->rem[1];  // RE_Time1.Match(1);
      }

      // correct hour if necessary (quarter to two is 1:45) 
      //  and minute (20 to --> min=40)
      if (st->sign==-1) {
        if (st->hour==L"1") st->hour=L"13"; // una menos veinte = 12:40
        else if (st->hour==L"0") st->hour=L"12";
        st->hour = normalize(st->hour, -1);
    
        // set minute
        st->minute = util::int2wstring(60-util::wstring2int(st->minute));
      }
      break;  
  
      // ---------------------------------
    case ST_EHb:
    case ST_EH1b:
      TRACE(3,L"Actions for state EHb/EH1b");

      // if it comes from CH, what we stored in temp was an hour
      if (origin==ST_CH || (origin==ST_CH1 && st->temp<=24 && st->temp>=0)) {
        st->hour=util::int2wstring(st->temp);
        if (st->temp==24) {
          st->meridian=L"am";
          st->hour=L"0";
        }
        else if (st->temp>12) { 
          st->meridian=L"pm";
          st->hour=util::int2wstring(st->temp-12);
        }
        st->minute =L"00";
      }

      if (token==TK_minnum) 
        st->minute= util::int2wstring(value); 

      else if (token==TK_min) {  // rem contains matches for RE_Time2
        st->minute = st->rem[1];  // RE_Time2.Match(1);
      }

      break;

      // ---------------------------------
    case ST_AH:
    case ST_AH1:
      TRACE(3,L"Actions for state AH/AH1");

      // if it comes from CH, what we stored in temp was an hour
      if (origin==ST_CH || (origin==ST_CH1 && st->temp<=24 && st->temp>=0)) {
        st->hour=util::int2wstring(st->temp);
        if (st->temp==24) {
          st->meridian=L"am";
          st->hour=L"0";
        }
        else if (st->temp>12) { 
          st->meridian=L"pm";
          st->hour=util::int2wstring(st->temp-12);
        }
      }

      if (token==TK_woclock)
        st->minute=L"00";

      else if (token==TK_hhmm) { // rem contains matches for RE_hhmm
        st->hour= st->rem[1];  // RE_Time1.Match(1);
        st->minute= st->rem[2];  // RE_Time1.Match(2);
      }

      break;  

      // ---------------------------------
    case ST_GH:
    case ST_GH1:
      TRACE(3,L"Actions for state GH/GH1");
      if (token==TK_wampm) {
        if (form==L"a.m.")
          st->meridian=L"am";
        else if (form==L"p.m.") 
          st->meridian=L"pm";
      }
      else if (token==TK_wmorning) {
        if (form==L"morning") 
          st->meridian=L"am";
        else if (form==L"afternoon" || form==L"evening" || form==L"night") 
          st->meridian=L"pm";
      }
      else if (token==TK_wmidnight) {
        if (st->hour==UNKNOWN_SYMB) {
          if (form==L"midnight") {
            st->meridian=L"am";
            st->hour=L"00"; 
            st->minute=L"00";
          }
          else if (form==L"midday" || form==L"noon") {
            st->meridian=L"pm";
            st->hour=L"12"; 
            st->minute=L"00";
          }
        }
        else {
          if (form==L"midnight") 
            st->meridian=L"am";
          else if (form==L"midday" || form==L"noon")
            st->meridian=L"pm";
        }
      }

      break;  

      // ---------------------------------
    case ST_FH:
      TRACE(3,L"Actions for state FH");
      // if it comes from CH, what we stored in temp was an hour
      if (origin==ST_CH && st->temp<=24  && st->temp>=0) {
        st->hour=util::int2wstring(st->temp);
        st->minute=L"00";
        if (st->temp==24) {
          st->meridian=L"am";
          st->hour=L"0";
        }
        else if (st->temp>12) { 
          st->meridian=L"pm";
          st->hour=util::int2wstring(st->temp-12);
        }
      }
      break;  

      // ---------------------------------
    case ST_aux:
      TRACE(3,L"Actions for state aux");
      // if it comes from CH, what we stored in temp was an hour
      // and this is the end of the date
      if (origin==ST_CH && st->temp<=24  && st->temp>=0) {
        st->hour=util::int2wstring(st->temp);
        st->minute=L"00";
        if (st->temp==24) {
          st->meridian=L"am";
          st->hour=L"0";
        }
        else if (st->temp>12) { 
          st->meridian=L"pm";
          st->hour=util::int2wstring(st->temp-12);
        }
      }

      break;  
    

      // ---------------------------------
    default: break;
    }

    TRACE(3,L"State actions finished. ["+st->weekday+L":"+st->day+L":"+st->month+L":"+st->year+L":"+st->hour+L":"+st->minute+L":"+st->meridian+L"]");
  }

  ///////////////////////////////////////////////////////////////
  ///   Set the appropriate lemma and tag for the 
  ///   new multiword.
  ///////////////////////////////////////////////////////////////

  void dates_en::SetMultiwordAnalysis(sentence::iterator i, int fstate, const dates_status *st) const {
    list<analysis> la;
    wstring lemma;

    // Setting the analysis for the date     
    if (st->century!=UNKNOWN_SYMB)
      lemma = L"[s:" + st->century + L"]";
    else
      lemma = L"[" + st->weekday + L":" + st->day + L"/" + st->month + L"/" + st->year 
        + L":" + st->hour + L"." + st->minute + L":" + st->meridian + L"]";

    la.push_back(analysis(lemma,L"W"));
    i->set_analysis(la);
    TRACE(3,L"Analysis set to: "+lemma+L" W");

    // record this word was analyzed by this module
    i->set_analyzed_by(word::DATES);     
    // mark the word as analyzed
    i->lock_analysis();    
 }

} // namespace
