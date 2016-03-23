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


  //-----------------------------------------------//
  //        Spanish date recognizer                //
  //-----------------------------------------------//

  // State codes 
  // states for a date
#define ST_A  1     // A: initial state
#define ST_B  2     // B: got a week day                           (VALID DATE)
#define ST_C  3     // C: got a week day and a comma
#define ST_D  4     // D: got the word "dia" from A,B,C 
#define ST_E  5     // E: got day number from A,B,C,D              (VALID DATE)
#define ST_Eb 40    // Eb: as E but not final
#define ST_F  6     // F: got "de" or "del" from E (a month may come)
#define ST_G  7     // G: got word "mes" from A,F
#define ST_H  8     // H: got "de" from G (a month may come)
#define ST_I  9     // I: got a month name o number from A,F,G,H.  (VALID DATE)
#define ST_Ib 10    // Ib: got a short month name from A.
#define ST_J 11     // J: got "de" or "del" from J (a year may come)
#define ST_K 12     // K: got "año" from A,J (a year may come)
#define ST_L 13     // L: got a number from J,K or a date from A.  (VALID DATE)
#define ST_P 14     // P: got "a.c." or "d.c" from L,S2            (VALID DATE)
#define ST_S1 15    // S1: got "siglo" from A (a century may come)
#define ST_S2 16    // S2: got roman number from S1                (VALID DATE)
  // states for time, after finding a date (e.g. 22 de febrero a las cuatro)
#define ST_Ha 17    // Ha: got "hacia" "sobre" "a" after a valid date
#define ST_Hb 18    // Hb: got "eso" from Ha
#define ST_Hc 19    // Hc: got "de" from Hb
#define ST_Hd 20    // Hd: got "por" after a valid date
#define ST_He 21    // He: got "la" "las" "el" from Hd
#define ST_AH 22    // AH: got "a las" "sobre las" "hacia eso de las", etc. An hour is expected.
#define ST_BH 23    // BH: got a valid hour+min from AH,CH,DH or EH. (VALID DATE)
#define ST_CH 24    // CH: got a valid hour (no min) from AH         (VALID DATE)
#define ST_DH 25    // DH: got "en" from CH (expecting "punto")
#define ST_EH 26    // EH: got "y" from CH (expecting minutes)
#define ST_EHb 27    // EH: got "menos" from CH (expecting minutes)
#define ST_FH  28    // FH: got "de" "del" from BH,CH
#define ST_GH  29    // GH: got "tarde" "noche" "mediodia" etc from BH,CH,FH  (VALID DATE)
  // states for a time non preceded by a date
#define ST_BH1  30   // BH1: got valid hour+min from A,CH1 (could be anything "dos y veinte")
#define ST_BH2  31   // BH2: got valid hour+quartern from DH1,EH1,EH1b  (e.g "dos y media, dos en punto, dos y cuarto") (VALID DATE)
#define ST_CH1  32   // CH1: got valid hour (no min) from A (but could be a day number)
#define ST_DH1  33   // DH1: got "en" from CH1 (expecting "punto")
#define ST_EH1  34   // EH1: got "y" from CH1 (expecting minutes)
#define ST_EH1b 35   // EH1: got "menos" from CH1 (expecting minutes)
#define ST_FH1  36   // FH1B: got "de" "del" from BH1 (must be a time)
#define ST_FH1b 37  // FH1: got "de" "del" from CH1 (a month name may follow)
#define ST_GH1  38   // GH1: got "tarde" "noche" "mediodia" etc from BH1,CH1,FH1b  (VALID DATE)
  // stop state
#define ST_STOP 39

  // Token codes
#define TK_weekday   1      // weekday: "Lunes", "Martes", etc
#define TK_daynum    2      // daynum:  1-31
#define TK_month     3      // month: "enero", "febrero", "ene." "feb." etc,
#define TK_shmonth   4      // short month: "ene", "feb", "mar" etc,
#define TK_monthnum  5      // monthnum: 1-12
#define TK_number    6      // number: 23, 1976, 543, etc.
#define TK_comma     7      // comma: ","
#define TK_dot       8     // dot: "."
#define TK_colon     9     // colon:  ":"
#define TK_wday      10    // wdia: "día"
#define TK_wmonth    11    // wmonth: "mes"
#define TK_wyear     12    // wyear: "año"
#define TK_wpast     13    // wpast: "pasado", "presente", "proximo"
#define TK_wcentury  14    // wcentury:  "siglo", "sig", "sgl", etc.
#define TK_roman     15    // roman: XVI, VII, III, XX, etc.
#define TK_wacdc     16    // wacdc: "a.c.", "d.c."
#define TK_wampm     17    // wacdc: "a.m.", "p.m."
#define TK_hournum   18    // hournum: 0-24
#define TK_minnum    19    // minnum: 0-60
#define TK_wquart    20    // wquart: "cuarto" "media" "pico" "algo" "poco"
#define TK_wy        21    // wy: "y"
#define TK_wmenos    22    // wy: "wmenos"
#define TK_wmorning  23    // wmorning: "mañana", "tarde", "noche", "madrugada"
#define TK_wmidnight 24    // wmidnight: "mediodia", "medianoche"
#define TK_wen       25    // wen: "en"
#define TK_wa        26    // wa: "a" "al"
#define TK_wde       27    // wde: "de"
#define TK_wdel      28    // wdel "del
#define TK_wel       29    // wel: "el"
#define TK_wpor      30    // wpor: "por" 
#define TK_whacia    31    // wpor: "sobre" "hacia"
#define TK_weso      32    // weso: "eso"
#define TK_wla       33    // wla: "la" "las"
#define TK_wpunto    34    // wpunto: "punto"
#define TK_whour     35    // whour: "horas" "h."
#define TK_wmin      36    // wpico: "minutos" "min"
#define TK_hour      37    // hour: something matching just first part of RE_TIME1: 3h, 17h
#define TK_hhmm      38    // hhmm: something matching whole RE_TIME1: 3h25min, 3h25m, 3h25minutos,
#define TK_min       39    // min: something matching RE_TIME2: 25min, 3minutos
#define TK_date      40    // date: something matching RE_DATE: 3/ago/1992, 3/8/92, 3/agosto/1992, ...
#define TK_other     41    // other: any other token, not relevant in current state


  ///////////////////////////////////////////////////////////////
  ///  Create a dates recognizer for Spanish.
  ///////////////////////////////////////////////////////////////

  dates_es::dates_es(): dates_module(RE_DATE_ES, RE_TIME1_ES, RE_TIME2_ES, RE_ROMAN)
  {
    //Initialize token translation map
    tok.insert(make_pair(L"lunes",TK_weekday));     tok.insert(make_pair(L"martes",TK_weekday));
    tok.insert(make_pair(L"miércoles",TK_weekday)); tok.insert(make_pair(L"miercoles",TK_weekday));
    tok.insert(make_pair(L"jueves",TK_weekday));    tok.insert(make_pair(L"viernes",TK_weekday));
    tok.insert(make_pair(L"sábado",TK_weekday));    tok.insert(make_pair(L"sabado",TK_weekday));
    tok.insert(make_pair(L"domingo",TK_weekday));

    tok.insert(make_pair(L"por",TK_wpor));  tok.insert(make_pair(L"hacia",TK_whacia));
    tok.insert(make_pair(L"sobre",TK_whacia)); tok.insert(make_pair(L"eso",TK_weso));      
    tok.insert(make_pair(L"y",TK_wy));      tok.insert(make_pair(L"menos",TK_wmenos));
    tok.insert(make_pair(L"de",TK_wde));    tok.insert(make_pair(L"del",TK_wdel));
    tok.insert(make_pair(L"a",TK_wa));      tok.insert(make_pair(L"al",TK_wa));
    tok.insert(make_pair(L"en",TK_wen));    tok.insert(make_pair(L"punto",TK_wpunto));
    tok.insert(make_pair(L"la",TK_wla));    tok.insert(make_pair(L"las",TK_wla));
    tok.insert(make_pair(L"el",TK_wel));
    tok.insert(make_pair(L",",TK_comma));   tok.insert(make_pair(L".",TK_dot));
    tok.insert(make_pair(L":",TK_colon));   tok.insert(make_pair(L"día",TK_wday)); 
    tok.insert(make_pair(L"dia",TK_wday));  tok.insert(make_pair(L"mes",TK_wmonth)); 
    tok.insert(make_pair(L"año",TK_wyear)); tok.insert(make_pair(L"presente",TK_wpast));
    tok.insert(make_pair(L"pasado",TK_wpast)); tok.insert(make_pair(L"próximo",TK_wpast));
             
    tok.insert(make_pair(L"media",TK_wquart)); tok.insert(make_pair(L"cuarto",TK_wquart));
    tok.insert(make_pair(L"pico",TK_wquart));  tok.insert(make_pair(L"algo",TK_wquart));
    tok.insert(make_pair(L"poco",TK_wquart));   

    tok.insert(make_pair(L"siglo",TK_wcentury)); tok.insert(make_pair(L"s.",TK_wcentury));
    tok.insert(make_pair(L"sig.",TK_wcentury));  tok.insert(make_pair(L"sgl.",TK_wcentury));
    tok.insert(make_pair(L"sigl.",TK_wcentury));

    tok.insert(make_pair(L"a.c.",TK_wacdc));  tok.insert(make_pair(L"d.c.",TK_wacdc));
    tok.insert(make_pair(L"a.m.",TK_wampm));  tok.insert(make_pair(L"p.m.",TK_wampm));
                                                 
    tok.insert(make_pair(L"medianoche",TK_wmidnight)); tok.insert(make_pair(L"mediodía",TK_wmidnight));
    tok.insert(make_pair(L"mediodia",TK_wmidnight));   tok.insert(make_pair(L"mañana",TK_wmorning)); 
    tok.insert(make_pair(L"tarde",TK_wmorning));       tok.insert(make_pair(L"madrugada",TK_wmorning));
    tok.insert(make_pair(L"noche",TK_wmorning)); 
             
    tok.insert(make_pair(L"horas",TK_whour)); tok.insert(make_pair(L"hora",TK_whour));
    tok.insert(make_pair(L"h.",TK_whour));    tok.insert(make_pair(L"h",TK_whour));
    tok.insert(make_pair(L"m",TK_wmin));      tok.insert(make_pair(L"m.",TK_wmin));
    tok.insert(make_pair(L"min",TK_wmin));    tok.insert(make_pair(L"min.",TK_wmin));
    tok.insert(make_pair(L"minuto",TK_wmin)); tok.insert(make_pair(L"minutos",TK_wmin));

    tok.insert(make_pair(L"enero",TK_month));      tok.insert(make_pair(L"ene",TK_shmonth));
    tok.insert(make_pair(L"febrero",TK_month));    tok.insert(make_pair(L"feb",TK_shmonth));
    tok.insert(make_pair(L"marzo",TK_month));      tok.insert(make_pair(L"mar",TK_shmonth));
    tok.insert(make_pair(L"abril",TK_month));      tok.insert(make_pair(L"abr",TK_shmonth));
    tok.insert(make_pair(L"mayo",TK_month));       tok.insert(make_pair(L"may",TK_shmonth));
    tok.insert(make_pair(L"junio",TK_month));      tok.insert(make_pair(L"jun",TK_shmonth));
    tok.insert(make_pair(L"julio",TK_month));      tok.insert(make_pair(L"jul",TK_shmonth));
    tok.insert(make_pair(L"agosto",TK_month));     tok.insert(make_pair(L"ago",TK_shmonth));
    tok.insert(make_pair(L"septiembre",TK_month)); tok.insert(make_pair(L"sep",TK_shmonth));
    tok.insert(make_pair(L"octubre",TK_month));    tok.insert(make_pair(L"oct",TK_shmonth));
    tok.insert(make_pair(L"noviembre",TK_month));  tok.insert(make_pair(L"nov",TK_shmonth));
    tok.insert(make_pair(L"diciembre",TK_month));  tok.insert(make_pair(L"dic",TK_shmonth));
    tok.insert(make_pair(L"noviem",TK_month));     tok.insert(make_pair(L"sept",TK_shmonth));
    tok.insert(make_pair(L"diciem",TK_month));     tok.insert(make_pair(L"agos",TK_shmonth)); 
             
    // initialize map with month numeric values
    nMes.insert(make_pair(L"enero",1));      nMes.insert(make_pair(L"ene",1));
    nMes.insert(make_pair(L"febrero",2));    nMes.insert(make_pair(L"feb",2));
    nMes.insert(make_pair(L"marzo",3));      nMes.insert(make_pair(L"mar",3));
    nMes.insert(make_pair(L"abril",4));      nMes.insert(make_pair(L"abr",4));
    nMes.insert(make_pair(L"mayo",5));       nMes.insert(make_pair(L"may",5));
    nMes.insert(make_pair(L"junio",6));      nMes.insert(make_pair(L"jun",6));
    nMes.insert(make_pair(L"julio",7));      nMes.insert(make_pair(L"jul",7));
    nMes.insert(make_pair(L"agosto",8));     nMes.insert(make_pair(L"ago",8));
    nMes.insert(make_pair(L"septiembre",9)); nMes.insert(make_pair(L"sep",9));
    nMes.insert(make_pair(L"octubre",10));   nMes.insert(make_pair(L"oct",10));
    nMes.insert(make_pair(L"noviembre",11)); nMes.insert(make_pair(L"nov",11));
    nMes.insert(make_pair(L"diciembre",12)); nMes.insert(make_pair(L"dic",12));
    nMes.insert(make_pair(L"agos",8));     nMes.insert(make_pair(L"sept",9));
    nMes.insert(make_pair(L"noviem",11));  nMes.insert(make_pair(L"diciem",12));  

    // initialize map with weekday numeric values
    nDia.insert(make_pair(L"lunes",L"L"));       nDia.insert(make_pair(L"martes",L"M"));
    nDia.insert(make_pair(L"miércoles",L"X"));   nDia.insert(make_pair(L"miercoles",L"X"));
    nDia.insert(make_pair(L"jueves",L"J"));      nDia.insert(make_pair(L"viernes",L"V"));
    nDia.insert(make_pair(L"sábado",L"S"));      nDia.insert(make_pair(L"sabado",L"S"));
    nDia.insert(make_pair(L"domingo",L"G"));
                                              
    // Initialize special state attributes
    initialState=ST_A; stopState=ST_STOP;

    // Initialize Final state set 
    Final.insert(ST_B);   Final.insert(ST_E);     Final.insert(ST_I);   Final.insert(ST_L);
    Final.insert(ST_P);   Final.insert(ST_S2);    Final.insert(ST_BH);  Final.insert(ST_BH2);  
    Final.insert(ST_CH);  Final.insert(ST_GH);    Final.insert(ST_GH1); 

    // Initialize transitions table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;

    // State A
    trans[ST_A][TK_weekday]=ST_B;   trans[ST_A][TK_wday]=ST_D;   trans[ST_A][TK_wmonth]=ST_G;
    trans[ST_A][TK_wyear]=ST_K;     trans[ST_A][TK_date]=ST_L;     
    trans[ST_A][TK_month]=ST_I;     trans[ST_A][TK_shmonth]=ST_Ib;  
    trans[ST_A][TK_wcentury]=ST_S1; trans[ST_A][TK_wpast]=ST_A;  trans[ST_A][TK_daynum]=ST_CH1;
    trans[ST_A][TK_hour]=ST_CH1;    trans[ST_A][TK_hhmm]=ST_BH1; trans[ST_A][TK_wmidnight]=ST_GH1;
    // State B+
    trans[ST_B][TK_wpast]=ST_B;  trans[ST_B][TK_daynum]=ST_E; trans[ST_B][TK_comma]=ST_C; 
    trans[ST_B][TK_wday]=ST_D;    
    trans[ST_B][TK_wpor]=ST_Hd;  trans[ST_B][TK_wa]=ST_Ha;    trans[ST_B][TK_whacia]=ST_Ha;
    // State C
    trans[ST_C][TK_daynum]=ST_E; trans[ST_C][TK_wday]=ST_D;
    // State D
    trans[ST_D][TK_daynum]=ST_E;
    // State E+
    trans[ST_E][TK_wde]=ST_F;    trans[ST_E][TK_wdel]=ST_F;  
    trans[ST_E][TK_month]=ST_I;  trans[ST_E][TK_shmonth]=ST_I;
    trans[ST_E][TK_monthnum]=ST_I;
    trans[ST_E][TK_wpor]=ST_Hd;  trans[ST_E][TK_wa]=ST_Ha;   trans[ST_E][TK_whacia]=ST_Ha;
    // State Eb
    trans[ST_Eb][TK_wde]=ST_F;    trans[ST_Eb][TK_wdel]=ST_F;  
    trans[ST_Eb][TK_month]=ST_I;  trans[ST_Eb][TK_shmonth]=ST_I;
    trans[ST_Eb][TK_monthnum]=ST_I;
    trans[ST_Eb][TK_wpor]=ST_Hd;  trans[ST_Eb][TK_wa]=ST_Ha;   trans[ST_Eb][TK_whacia]=ST_Ha;
    // State F
    trans[ST_F][TK_wpast]=ST_F;  trans[ST_F][TK_month]=ST_I;   trans[ST_F][TK_shmonth]=ST_I; 
    trans[ST_F][TK_wmonth]=ST_G; trans[ST_F][TK_monthnum]=ST_I;
    // State G
    trans[ST_G][TK_wde]=ST_H;   trans[ST_G][TK_monthnum]=ST_I;   
    trans[ST_G][TK_month]=ST_I; trans[ST_G][TK_shmonth]=ST_I; 
    // State H
    trans[ST_H][TK_month]=ST_I; trans[ST_H][TK_shmonth]=ST_I; trans[ST_H][TK_monthnum]=ST_I;   
    // State I+
    trans[ST_I][TK_wde]=ST_J;   trans[ST_I][TK_wdel]=ST_J;
    trans[ST_I][TK_wpor]=ST_Hd; trans[ST_I][TK_wa]=ST_Ha;  trans[ST_I][TK_whacia]=ST_Ha;
    // State Ib
    trans[ST_Ib][TK_wde]=ST_J;   trans[ST_Ib][TK_wdel]=ST_J; 
    // State J
    trans[ST_J][TK_wpast]=ST_J; trans[ST_J][TK_number]=ST_L; trans[ST_J][TK_wyear]=ST_K;
    // State K
    trans[ST_K][TK_wpast]=ST_K; trans[ST_K][TK_number]=ST_L;
    // State L+
    trans[ST_L][TK_wacdc]=ST_P;
    trans[ST_L][TK_wpor]=ST_Hd; trans[ST_L][TK_wa]=ST_Ha;   trans[ST_L][TK_whacia]=ST_Ha;
    // State P+
    trans[ST_P][TK_wpor]=ST_Hd; trans[ST_P][TK_wa]=ST_Ha;   trans[ST_P][TK_whacia]=ST_Ha;
    // State S1
    trans[ST_S1][TK_roman]=ST_S2; trans[ST_S1][TK_dot]=ST_S1;
    // State S2+
    trans[ST_S2][TK_wacdc]=ST_S2;

    // State Ha
    trans[ST_Ha][TK_weso]=ST_Hb; trans[ST_Ha][TK_wel]=ST_AH; trans[ST_Ha][TK_wla]=ST_AH; 
    trans[ST_Ha][TK_wmidnight]=ST_GH; 
    // State Hb
    trans[ST_Hb][TK_wde]=ST_Hc;  trans[ST_Hb][TK_wdel]=ST_AH;
    // State Hc
    trans[ST_Hc][TK_wla]=ST_AH;
    // State Hd
    trans[ST_Hd][TK_wel]=ST_He; trans[ST_Hd][TK_wla]=ST_He;
    // State He
    trans[ST_He][TK_wmidnight]=ST_GH;   trans[ST_He][TK_wmorning]=ST_GH; 
    // State AH
    trans[ST_AH][TK_hhmm]=ST_BH; trans[ST_AH][TK_hour]=ST_CH; 
    trans[ST_AH][TK_hournum]=ST_CH; trans[ST_AH][TK_wmidnight]=ST_GH;
    // State BH+
    trans[ST_BH][TK_wmin]=ST_BH; trans[ST_BH][TK_wampm]=ST_GH;
    trans[ST_BH][TK_wdel]=ST_FH; trans[ST_BH][TK_wde]=ST_FH; 
    // State CH+
    trans[ST_CH][TK_whour]=ST_CH; trans[ST_CH][TK_minnum]=ST_BH; trans[ST_CH][TK_min]=ST_BH;
    trans[ST_CH][TK_wen]=ST_DH;   trans[ST_CH][TK_wy]=ST_EH;     trans[ST_CH][TK_wmenos]=ST_EHb;
    trans[ST_CH][TK_wde]=ST_FH;   trans[ST_CH][TK_wdel]=ST_FH;   trans[ST_CH][TK_wampm]=ST_GH; 
    // State DH
    trans[ST_DH][TK_wpunto]=ST_BH;
    // State EH
    trans[ST_EH][TK_wquart]=ST_BH; trans[ST_EH][TK_minnum]=ST_BH; trans[ST_EH][TK_min]=ST_BH;
    // State EHb
    trans[ST_EHb][TK_wquart]=ST_BH; trans[ST_EHb][TK_minnum]=ST_BH;  trans[ST_EHb][TK_min]=ST_BH;
    // State FH
    trans[ST_FH][TK_wla]=ST_FH; trans[ST_FH][TK_wmorning]=ST_GH; trans[ST_FH][TK_wmidnight]=ST_GH;
    // State GH+
    // nothing else expected. 

    // State BH1
    trans[ST_BH1][TK_wmin]=ST_BH1; trans[ST_BH1][TK_wampm]=ST_GH1; trans[ST_BH1][TK_wdel]=ST_FH1;
    trans[ST_BH1][TK_wde]=ST_FH1;  trans[ST_BH1][TK_wel]=ST_A;     trans[ST_BH1][TK_wen]=ST_DH1;
    // State BH2+
    trans[ST_BH2][TK_wampm]=ST_GH1; trans[ST_BH2][TK_wdel]=ST_FH1;
    trans[ST_BH2][TK_wde]=ST_FH1;   trans[ST_BH2][TK_wel]=ST_A;
    // State CH1
    trans[ST_CH1][TK_whour]=ST_CH1; trans[ST_CH1][TK_minnum]=ST_BH1;  trans[ST_CH1][TK_min]=ST_BH1;
    trans[ST_CH1][TK_wen]=ST_DH1;   trans[ST_CH1][TK_wmenos]=ST_EH1b; trans[ST_CH1][TK_wy]=ST_EH1;
    trans[ST_CH1][TK_wde]=ST_FH1b;  trans[ST_CH1][TK_wdel]=ST_FH1b;   trans[ST_CH1][TK_wampm]=ST_GH1;
    trans[ST_CH1][TK_month]=ST_I;   trans[ST_CH1][TK_shmonth]=ST_I;
    // State DH1
    trans[ST_DH1][TK_wpunto]=ST_BH2;
    // State EH1
    trans[ST_EH1][TK_wquart]=ST_BH2; trans[ST_EH1][TK_minnum]=ST_BH1;   trans[ST_EH1][TK_min]=ST_BH1;
    // State EH1b
    trans[ST_EH1b][TK_wquart]=ST_BH2; trans[ST_EH1b][TK_minnum]=ST_BH1; trans[ST_EH1b][TK_min]=ST_BH1;
    // State FH1
    trans[ST_FH1][TK_wla]=ST_FH1;       trans[ST_FH1][TK_wmorning]=ST_GH1; 
    trans[ST_FH1][TK_wmidnight]=ST_GH1; trans[ST_FH1][TK_weekday]=ST_B; 
    trans[ST_FH1][TK_wday]=ST_D;        trans[ST_FH1][TK_daynum]=ST_Eb; 
    // State FH1b
    trans[ST_FH1b][TK_wla]=ST_FH1;    trans[ST_FH1b][TK_wmonth]=ST_G;
    trans[ST_FH1b][TK_month]=ST_I;    trans[ST_FH1b][TK_shmonth]=ST_I;   
    trans[ST_FH1b][TK_wpast]=ST_FH1b; trans[ST_FH1b][TK_weekday]=ST_B; 
    trans[ST_FH1b][TK_wday]=ST_D;     trans[ST_FH1b][TK_wyear]=ST_K;
    trans[ST_FH1b][TK_daynum]=ST_Eb;
    trans[ST_FH1b][TK_wmorning]=ST_GH1;   trans[ST_FH1b][TK_wmidnight]=ST_GH1; 
    // State GH1+
    trans[ST_GH1][TK_wde]=ST_A;    trans[ST_GH1][TK_wdel]=ST_A;    trans[ST_GH1][TK_wel]=ST_A;

    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int dates_es::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
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
      token = (*im).second;
    }
  
    TRACE(3,L"Next word form is: ["+form+L"] token="+util::int2wstring(token));     
    // if the token was in the table, we're done
    if (token != TK_other) return (token);

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
      // --------------------------------
    case ST_A: 
      TRACE(3,L"In state A");     
      if (token==TK_number && value>=1 && value<=31 && form!=L"una" && form!=L"un") {
        token = TK_daynum;
      }
      else if (RE_Date.search(form,st->rem)) {
        TRACE(3,L"Match DATE regex.");
        token = TK_date;
      }
      else if (RE_Time1.search(form,st->rem)) {
        if (st->rem.size()>=3) {   // if (not rem[2].empty()) ??  // if (RE_Time1.Match(2)!="")
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
    case ST_B:
    case ST_C:
    case ST_D:
    case ST_FH1:
    case ST_FH1b:
      TRACE(3,L"In state B/C/D");     
      if (token==TK_number && value>=1 && value<=31 && form!=L"una" && form!=L"un") 
        // ("un/una de enero" is an invalid date)
        token = TK_daynum;
      break;
      // --------------------------------
    case ST_E:
    case ST_Eb:
    case ST_F:
    case ST_G:
    case ST_H:
      TRACE(3,L"In state E/F/G/H");     
      if (token==TK_number && value>=1 && value<=12) 
        token = TK_monthnum;
      break;
      // --------------------------------
    case ST_K:
      TRACE(3,L"In state K");     
      if (token==TK_number && (form==L"una" || form==L"un"))
        // ("año uno" is allright, but "año un/una" is an invalid date)
        token = TK_other;
      break;
      // --------------------------------
    case ST_S1:
      if (RE_Roman.search(formU,st->rem)) {
        TRACE(3,L"Match ROMAN regex. ");
        token=TK_roman;
      }
      break;
      // --------------------------------
    case ST_AH:
      TRACE(3,L"In state AH");     
      if (token==TK_number && value>=0 && value<=24) {
        token = TK_hournum;
      }
      else if (RE_Time1.search(form,st->rem)) {
        if (st->rem.size()>=3) {   // if (not rem[2].empty()) ??  // if (RE_Time1.Match(2)!="")
          TRACE(3,L"Match TIME1 regex (hour+min)");
          token = TK_hhmm;
        }
        else {
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
      // --------------------------------
    case ST_EH:
    case ST_EHb:
    case ST_EH1:
    case ST_EH1b:
      TRACE(3,L"In state EH/EH1");     
      if (token==TK_number && value>=0 && value<=60){
        token=TK_minnum;
      }
      else if (RE_Time2.search(form,st->rem)) {
        TRACE(3,L"Match TIME2 regex (minutes)");
        token = TK_min;
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

  void dates_es::StateActions(int origin, int state, int token, sentence::const_iterator j, dates_status *st) const {
    wstring form;
    int value;
    map<wstring,int>::const_iterator im;

    form = j->get_lc_form();
    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ["+form+L"]");

    if (state==ST_STOP) return;

    // get token numerical value, if any
    value=0;
    if ((token==TK_number || token==TK_daynum || token==TK_monthnum ||
         token==TK_hournum || token==TK_minnum ) &&
        j->get_n_analysis() && j->get_tag()==L"Z") {
      value = util::wstring2int(j->get_lemma());
    }

    // State actions
    switch (state) {
      // ---------------------------------
    case ST_B:
      if (token==TK_weekday) {
        TRACE(3,L"Actions for state B");
        st->weekday=nDia.find(form)->second;
        if (origin==ST_FH1b) {
          // what we found in CH1 was an hour. Day is coming now
          st->minute=L"00";
          if (st->temp>12) { 
            st->meridian=L"pm";
            st->hour=util::int2wstring(st->temp-12);
          }
          else if (st->temp!= -1) {
            st->hour=util::int2wstring(st->temp);
          }
        }
      }
      break;
      // ---------------------------------
    case ST_D:
      if (origin==ST_FH1b) {
        TRACE(3,L"Actions for state D");
        // what we found in CH1 was an hour. Day is coming now
        st->minute=L"00";
        if (st->temp>12) {
          st->meridian=L"pm";
          st->hour=util::int2wstring(st->temp-12);
        }
        else  if (st->temp!= -1) {
          st->hour=util::int2wstring(st->temp);
        }
      }
      break;
      // ---------------------------------
    case ST_E:
    case ST_Eb:
      if (token==TK_daynum) {
        TRACE(3,L"Actions for state E");
        st->day=util::int2wstring(value);
      }

      if (origin==ST_FH1b) {
        // what we found in CH1 was an hour. Day is coming now
        st->minute=L"00";
        if (st->temp>12) { 
          st->meridian=L"pm";
          st->hour=util::int2wstring(st->temp-12);
        }
        else if (st->temp!= -1) {
          st->hour=util::int2wstring(st->temp);
        }
      }
      break;
      // ---------------------------------
    case ST_G:
      if (origin==ST_FH1b) {
        TRACE(3,L"Actions for state G/K");
        // what we found in CH1 was a day, not an hour
        st->day=util::int2wstring(st->temp);
      }
      break;
      // ---------------------------------
    case ST_K:
      if (origin==ST_FH1b) {
        TRACE(3,L"Actions for state G/K");
        // what we found in CH1 was a month
        st->month=util::int2wstring(st->temp);
      }
      break;
      // ---------------------------------
    case ST_I:
      TRACE(3,L"Actions for state I");
      if (origin==ST_FH1b || origin==ST_CH1) {
        // what we found in CH1 was a day, not an hour
        st->day=util::int2wstring(st->temp);
      }
      if (token==TK_monthnum) {
        st->month = j->get_lemma();
      }
      else if (token==TK_month || token==TK_shmonth) {
        st->month = util::int2wstring(nMes.find(form)->second);
      }
      break;
      // ---------------------------------
    case ST_Ib:
      TRACE(3,L"Actions for state Ib");
      if (token==TK_shmonth)
        st->month = util::int2wstring(nMes.find(form)->second);
      break;
      // ---------------------------------
    case ST_L:
      TRACE(3,L"Actions for state L");
      if (token==TK_number) {
        st->year=util::int2wstring(value);
      }
      else if (token==TK_date) { // rem contains matches for RE_Date
        // day number
        st->day=st->rem[1];
        // to unify notation (01 -> 1)
        st->day=normalize(st->day);
        // month number (translating month name if necessary)
        im=nMes.find(util::lowercase(st->rem[2]));
        if (im!=nMes.end()) 
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
      // ---------------------------------
    case ST_P:
      TRACE(3,L"Actions for state P");
      if (form==L"a.c.") {
        if (st->century!=L"") st->century=L"-"+st->century;   
        if (st->year!=UNKNOWN_SYMB) st->year=L"-"+st->year;
      }
      break;
      // ---------------------------------
    case ST_S2:
      TRACE(3,L"Actions for state S2");
      if (token==TK_roman) st->century=form; 
      else if (form==L"a.c.") st->century = L"-"+st->century;
      break;
      // ---------------------------------
    case ST_BH:
    case ST_BH1:
    case ST_BH2:
      TRACE(3,L"Actions for state BH/BH1/BH2");
      // what we found in CH1 was an hour
      if (origin!=ST_A && (state==ST_BH1 || state==ST_BH2)) {
        if (st->temp>12) {
          st->meridian=L"pm";
          st->hour = util::int2wstring(st->temp-12);
        }
        else if (st->temp!= -1) {
          st->hour = util::int2wstring(st->temp);
        }
      }
      if (token==TK_minnum) {
        // "y veinte" vs "menos veinte"
        if (origin==ST_EHb || origin==ST_EH1b) {
          value = 60-value;
          if (st->hour==L"1") st->hour=L"13"; // una menos veinte = 12:40
          st->hour = normalize(st->hour, -1);
        }
        st->minute=util::int2wstring(value);
      }
      else if (token==TK_wquart) {
        if (form==L"media") {
          st->minute=L"30";
        }
        else if (form==L"cuarto") {
          // "y cuarto" vs "menos cuarto"
          if (origin==ST_EHb || origin==ST_EH1b) {
            st->minute=L"45";
            if (st->hour==L"1") st->hour=L"13"; // una menos veinte = 12:40
            st->hour = normalize(st->hour, -1);
          }
          else st->minute=L"15";
        }
      }
      else if (token==TK_wpunto && st->minute==UNKNOWN_SYMB) {
        st->minute=L"00";
      }
      else if (token==TK_hhmm) {  // rem contains matches for RE_Time1
        st->hour=st->rem[1];   // RE_Time1.Match(1);
        st->minute=st->rem[2];  // RE_Time1.Match(2);
      }
      else if (token==TK_min) {  // rem contains matches for RE_Time2
        value=util::wstring2int(st->rem[1]); // RE_Time2.Match(1)
        // "y veinte" vs "menos veinte"
        if (origin==ST_EHb || origin==ST_EH1b) {
          value = 60-value;
          if (st->hour==L"1") st->hour=L"13"; // una menos veinte = 12:40
          st->hour = normalize(st->hour, -1);
        }
        st->minute=util::int2wstring(value);
      }
      break;
      // ---------------------------------
    case ST_CH:
      TRACE(3,L"Actions for state CH");
      if (token==TK_hournum) {
        if (value>12) { 
          st->meridian=L"pm";
          st->hour=util::int2wstring(value-12);
        }
        else st->hour=util::int2wstring(value);
        //set the minute to 0, if it isn't so, latter it will be replaced.
        st->minute=L"00";
      }
      else if (token==TK_hour) { // rem contains matches for RE_Time1
        st->hour=st->rem[1];  // RE_Time1.Match(1);
      }
      break;
      // ---------------------------------
    case ST_CH1:
      TRACE(3,L"Actions for state CH1");
      if (token==TK_daynum) {
        // maybe day or hour.  Store for later decision in BH1/B/D/G/I/K
        st->temp=value;
      }
      else if (token==TK_hour) { // rem contains matches for RE_Time1
        st->hour=st->rem[1];  // RE_Time1.Match(1);
      }
      break;
      // ---------------------------------
    case ST_FH1:
      TRACE(3,L"Actions for state FH1");
      if (origin==ST_FH1b || origin==ST_CH1) {
        // what we found in CH1 was an hour, not a day
        // put minute to 00 and set hour. 
        st->minute=L"00";
        if (st->temp>12) {
          st->meridian=L"pm";
          st->hour = util::int2wstring(st->temp-12);
        }
        else if (st->temp!= -1) {
          st->hour = util::int2wstring(st->temp);
        }
      }
      break;
      // ---------------------------------
    case ST_GH:
    case ST_GH1:
      TRACE(3,L"Actions for state GH/GH1");

      if (origin==ST_FH1b) {
        // what we found in CH1 was an hour, set minute to 00
        st->minute=L"00";
        if (st->temp>12) { 
          st->meridian=L"pm";
          st->hour=util::int2wstring(st->temp-12);
        }
        else if (st->temp!= -1) {
          st->hour=util::int2wstring(st->temp);
        }
      }

      if (token==TK_wampm) {
        if (form==L"a.m.") {
          st->meridian=L"am"; }
        else if (form==L"p.m.") {
          st->meridian=L"pm"; }
      }
      else if (token==TK_wmorning) {
        if (form==L"mañana" || form==L"madrugada")  {
          st->meridian=L"am"; }
        else if (form==L"tarde" || form==L"noche") {
          st->meridian=L"pm"; }
      }
      //else if (token==TK_wmidnight && hour=="??") {
      else if (token==TK_wmidnight) {
        if (form==L"medianoche") {
          st->meridian=L"am";
          st->hour=L"00"; 
          st->minute=L"00";
        }
        else if (form==L"mediodia" || form==L"mediodía") {
          st->meridian=L"pm";
          if (st->hour==UNKNOWN_SYMB) {
            st->hour=L"12"; 
            st->minute=L"00";
          }
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

  void dates_es::SetMultiwordAnalysis(sentence::iterator i, int fstate, const dates_status *st) const {
    list<analysis> la;
    wstring lemma;

    // Setting the analysis for the date
    if (st->century!=UNKNOWN_SYMB) {
      lemma=wstring(L"[s:"+st->century+L"]"); }
    else {
      lemma=wstring(L"["+st->weekday+L":"+st->day+L"/"+st->month+L"/"+st->year+L":"+st->hour+L"."+st->minute+L":"+st->meridian+L"]"); }

    la.push_back(analysis(lemma,L"W"));   
    i->set_analysis(la);
    TRACE(3,L"Analysis set to: "+lemma+L" W");
  }

} // namespace
