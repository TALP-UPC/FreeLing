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
#include "freeling/morfo/numbers_modules.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"NUMBERS"
#define MOD_TRACECODE NUMBERS_TRACE


  //---------------------------------------------------------------------------
  //        Catalan number recognizer
  //---------------------------------------------------------------------------

  //// State codes 
#define ST_B1 1  // initial state
#define ST_B2 2  // got hundreds "doscientos"  (VALID NUMBER)
#define ST_B3 3  // got tens "treinta" "docientos treinta"   (VALID NUMBER)
#define ST_B4 4  // got "y" after tens
#define ST_Bu 5  // got units after "y": "doscientos treinta y cuatro"   (VALID NUMBER)
#define ST_B5 6  // got "mil" after unit "doscientos treinta y cuatro mil"   (VALID NUMBER)
#define ST_B6 7  // got hundreds after "mil"   (VALID NUMBER)
#define ST_B7 8  // got tens after "mil"   (VALID NUMBER)
#define ST_B8 9  // got "y" after tens   (VALID NUMBER)
#define ST_Bk 10 // got units after "y"   (VALID NUMBER)
#define ST_M1 11 // got "billones" after a valid number  (VALID NUMBER)
#define ST_M1a 12 // got "y" after "billones"
#define ST_M1b 13 // got "... millones y medio/cuarto" 
#define ST_M2 14 // got hundreds "doscientos" after billions  (VALID NUMBER)
#define ST_M3 15 // got tens "treinta" "docientos treinta"   (VALID NUMBER)
#define ST_M4 16 // got "y" after tens
#define ST_Mu 17 // got units after "y": "doscientos treinta y cuatro"   (VALID NUMBER)
#define ST_M5 18 // got "mil" after unit "doscientos treinta y cuatro mil"   (VALID NUMBER)
#define ST_M6 19 // got hundreds after "mil"   (VALID NUMBER)
#define ST_M7 20 // got tens after "mil"   (VALID NUMBER)
#define ST_M8 21 // got "y" after tens   (VALID NUMBER)
#define ST_Mk 22 // got units after "y"   (VALID NUMBER)
#define ST_S1 23 // got "millones" after a valid number  (VALID NUMBER) 
#define ST_S1a 24 // got "y" after "millones"
#define ST_S1b 25 // got "... millones y medio/cuarto" 
#define ST_S2 26 // got hundreds "doscientos" after millions  (VALID NUMBER)
#define ST_S3 27 // got tens "treinta" "docientos treinta"   (VALID NUMBER)
#define ST_S4 28 // got "y" after tens
#define ST_Su 29 // got units after "y": "doscientos treinta y cuatro"   (VALID NUMBER)
#define ST_S5 30 // got "mil" after unit "doscientos treinta y cuatro mil"   (VALID NUMBER)
#define ST_S6 31 // got hundreds after "mil"   (VALID NUMBER)
#define ST_S7 32 // got tens after "mil"   (VALID NUMBER)
#define ST_S8 33 // got "y" after tens   (VALID NUMBER)
#define ST_Sk 34 // got units after "y"   (VALID NUMBER)
#define ST_COD 35 // got pseudo-numerical code from initial state
  // stop state
#define ST_STOP 36

  // Token codes
#define TK_c      1  // hundreds "cien" "doscientos" 
#define TK_d      2  // tens "treinta" "cuarenta"
#define TK_u      3  // units "tres" "cuatro"
#define TK_z      4  // zero "zero"
#define TK_wy     5  // word "y"
#define TK_wmedio 6  // word "medio"
#define TK_wcuarto 7 // word "cuarto"
#define TK_mil    8  // word "mil"   
#define TK_mill   9  // word "millon" "millones"
#define TK_bill  10  // word "billon" "billones"
#define TK_num   11  // a number (in digits)
#define TK_code  12  // a code (ex. LX-345-2)
#define TK_other 13
 
 
  ///////////////////////////////////////////////////////////////
  ///  Create a numbers recognizer for Catalan.
  ///////////////////////////////////////////////////////////////

  numbers_ca::numbers_ca(const std::wstring &dec, const std::wstring &thou): numbers_module(dec,thou)
  {  
    // Initializing value map
    value.insert(make_pair(L"cent",100.0f));         value.insert(make_pair(L"dos-cents",200.0f));
    value.insert(make_pair(L"dos-centes",200.0f));   value.insert(make_pair(L"dues-centes",200.0f));
    value.insert(make_pair(L"tres-cents",300.0f));   value.insert(make_pair(L"tres-centes",300.0f));
    value.insert(make_pair(L"quatre-cents",400.0f)); value.insert(make_pair(L"quatre-centes",400.0f));
    value.insert(make_pair(L"cinc-cents",500.0f));   value.insert(make_pair(L"cinc-centes",500.0f));
    value.insert(make_pair(L"sis-cents",600.0f));    value.insert(make_pair(L"sis-centes",600.0f));
    value.insert(make_pair(L"set-cents",700.0f));    value.insert(make_pair(L"set-centes",700.0f));
    value.insert(make_pair(L"vuit-cents",800.0f));   value.insert(make_pair(L"vuit-centes",800.0f));
    value.insert(make_pair(L"huit-cents",800.0f));   value.insert(make_pair(L"huit-centes",800.0f));
    value.insert(make_pair(L"nou-cents",900.0f));    value.insert(make_pair(L"nou-centes",900.0f));
    value.insert(make_pair(L"trenta",30.0f));        value.insert(make_pair(L"quaranta",40.0f));
    value.insert(make_pair(L"cinquanta",50.0f));     value.insert(make_pair(L"seixanta",60.0f));
    value.insert(make_pair(L"setanta",70.0f));       value.insert(make_pair(L"vuitanta",80.0f));
    value.insert(make_pair(L"huitanta",80.0f));      value.insert(make_pair(L"noranta",90.0f));     
    value.insert(make_pair(L"zero",0.0f));
    value.insert(make_pair(L"un",1.0f));             value.insert(make_pair(L"una",1.0f));
    value.insert(make_pair(L"dos",2.0f));            value.insert(make_pair(L"dues",2.0f));
    value.insert(make_pair(L"tres",3.0f));           value.insert(make_pair(L"quatre",4.0f));
    value.insert(make_pair(L"cinc",5.0f));           value.insert(make_pair(L"sis",6.0f));
    value.insert(make_pair(L"set",7.0f));            value.insert(make_pair(L"vuit",8.0f));
    value.insert(make_pair(L"nou",9.0f));            value.insert(make_pair(L"deu",10.0f));
    value.insert(make_pair(L"onze",11.0f));          value.insert(make_pair(L"dotze",12.0f));
    value.insert(make_pair(L"tretze",13.0f));        value.insert(make_pair(L"catorze",14.0f));
    value.insert(make_pair(L"quinze",15.0f));        value.insert(make_pair(L"setze",16.0f));
    value.insert(make_pair(L"disset",17.0f));        value.insert(make_pair(L"desset",17.0f));        
    value.insert(make_pair(L"dèsset",17.0f));        
    value.insert(make_pair(L"divuit",18.0f));        value.insert(make_pair(L"devuit",18.0f)); 
    value.insert(make_pair(L"dinou",19.0f));         value.insert(make_pair(L"denou",19.0f)); 
    value.insert(make_pair(L"vint", 20.0f));  
    value.insert(make_pair(L"vint-i-u",21.0f));      value.insert(make_pair(L"vint-i-una",21.0f));  
    value.insert(make_pair(L"vint-i-un",21.0f));     value.insert(make_pair(L"vint-i-dos",22.0f));
    value.insert(make_pair(L"vint-i-dues",22.0f));   value.insert(make_pair(L"vint-i-tres",23.0f));
    value.insert(make_pair(L"vint-i-quatre",24.0f)); value.insert(make_pair(L"vint-i-cinc",25.0f));  
    value.insert(make_pair(L"vint-i-sis",26.0f));    value.insert(make_pair(L"vint-i-set",27.0f));
    value.insert(make_pair(L"vint-i-vuit",28.0f));   value.insert(make_pair(L"vint-i-nou",29.0f));
    value.insert(make_pair(L"trenta-u",31.0f));      value.insert(make_pair(L"trenta-una",31.0f));  
    value.insert(make_pair(L"trenta-un",31.0f));     value.insert(make_pair(L"trenta-dos",32.0f));
    value.insert(make_pair(L"trenta-dues",32.0f));   value.insert(make_pair(L"trenta-tres",33.0f));
    value.insert(make_pair(L"trenta-quatre",34.0f)); value.insert(make_pair(L"trenta-cinc",35.0f));  
    value.insert(make_pair(L"trenta-sis",36.0f));    value.insert(make_pair(L"trenta-set",37.0f));
    value.insert(make_pair(L"trenta-vuit",38.0f));   value.insert(make_pair(L"trenta-nou",39.0f));
    value.insert(make_pair(L"quaranta-u",41.0f));      value.insert(make_pair(L"quaranta-una",41.0f));  
    value.insert(make_pair(L"quaranta-un",41.0f));     value.insert(make_pair(L"quaranta-dos",42.0f));
    value.insert(make_pair(L"quaranta-dues",42.0f));   value.insert(make_pair(L"quaranta-tres",43.0f));
    value.insert(make_pair(L"quaranta-quatre",44.0f)); value.insert(make_pair(L"quaranta-cinc",45.0f));  
    value.insert(make_pair(L"quaranta-sis",46.0f));    value.insert(make_pair(L"quaranta-set",47.0f));
    value.insert(make_pair(L"quaranta-vuit",48.0f));   value.insert(make_pair(L"quaranta-nou",49.0f));
    value.insert(make_pair(L"cinquanta-u",51.0f));      value.insert(make_pair(L"cinquanta-una",51.0f));  
    value.insert(make_pair(L"cinquanta-un",51.0f));     value.insert(make_pair(L"cinquanta-dos",52.0f));
    value.insert(make_pair(L"cinquanta-dues",52.0f));   value.insert(make_pair(L"cinquanta-tres",53.0f));
    value.insert(make_pair(L"cinquanta-quatre",54.0f)); value.insert(make_pair(L"cinquanta-cinc",55.0f));  
    value.insert(make_pair(L"cinquanta-sis",56.0f));    value.insert(make_pair(L"cinquanta-set",57.0f));
    value.insert(make_pair(L"cinquanta-vuit",58.0f));   value.insert(make_pair(L"cinquanta-nou",59.0f));
    value.insert(make_pair(L"seixanta-u",61.0f));      value.insert(make_pair(L"seixanta-una",61.0f));  
    value.insert(make_pair(L"seixanta-un",61.0f));     value.insert(make_pair(L"seixanta-dos",62.0f));
    value.insert(make_pair(L"seixanta-dues",62.0f));   value.insert(make_pair(L"seixanta-tres",63.0f));
    value.insert(make_pair(L"seixanta-quatre",64.0f)); value.insert(make_pair(L"seixanta-cinc",65.0f));  
    value.insert(make_pair(L"seixanta-sis",66.0f));    value.insert(make_pair(L"seixanta-set",67.0f));
    value.insert(make_pair(L"seixanta-vuit",68.0f));   value.insert(make_pair(L"seixanta-nou",69.0f));
    value.insert(make_pair(L"setanta-u",71.0f));      value.insert(make_pair(L"setanta-una",71.0f));  
    value.insert(make_pair(L"setanta-un",71.0f));     value.insert(make_pair(L"setanta-dos",72.0f));
    value.insert(make_pair(L"setanta-dues",72.0f));   value.insert(make_pair(L"setanta-tres",73.0f));
    value.insert(make_pair(L"setanta-quatre",74.0f)); value.insert(make_pair(L"setanta-cinc",75.0f));  
    value.insert(make_pair(L"setanta-sis",76.0f));    value.insert(make_pair(L"setanta-set",77.0f));
    value.insert(make_pair(L"setanta-vuit",78.0f));   value.insert(make_pair(L"setanta-nou",79.0f));
    value.insert(make_pair(L"vuitanta-u",81.0f));      value.insert(make_pair(L"vuitanta-una",81.0f));  
    value.insert(make_pair(L"vuitanta-un",81.0f));     value.insert(make_pair(L"vuitanta-dos",82.0f));
    value.insert(make_pair(L"vuitanta-dues",82.0f));   value.insert(make_pair(L"vuitanta-tres",83.0f));
    value.insert(make_pair(L"vuitanta-quatre",84.0f)); value.insert(make_pair(L"vuitanta-cinc",85.0f));  
    value.insert(make_pair(L"vuitanta-sis",86.0f));    value.insert(make_pair(L"vuitanta-set",87.0f));
    value.insert(make_pair(L"vuitanta-vuit",88.0f));   value.insert(make_pair(L"vuitanta-nou",89.0f));
    value.insert(make_pair(L"noranta-u",91.0f));      value.insert(make_pair(L"noranta-una",91.0f));  
    value.insert(make_pair(L"noranta-un",91.0f));     value.insert(make_pair(L"noranta-dos",92.0f));
    value.insert(make_pair(L"noranta-dues",92.0f));   value.insert(make_pair(L"noranta-tres",93.0f));
    value.insert(make_pair(L"noranta-quatre",94.0f)); value.insert(make_pair(L"noranta-cinc",95.0f));  
    value.insert(make_pair(L"noranta-sis",96.0f));    value.insert(make_pair(L"noranta-set",97.0f));
    value.insert(make_pair(L"noranta-vuit",98.0f));   value.insert(make_pair(L"noranta-nou",99.0f));

    value.insert(make_pair(L"mig",0.5f));  value.insert(make_pair(L"quart",0.25f));

    // Initializing token map
    tok.insert(make_pair(L"cent",TK_c));         tok.insert(make_pair(L"dos-cents",TK_c));
    tok.insert(make_pair(L"dos-centes",TK_c));   tok.insert(make_pair(L"dues-centes",TK_c));
    tok.insert(make_pair(L"tres-cents",TK_c));   tok.insert(make_pair(L"tres-centes",TK_c));
    tok.insert(make_pair(L"quatre-cents",TK_c)); tok.insert(make_pair(L"quatre-centes",TK_c));
    tok.insert(make_pair(L"cinc-cents",TK_c));   tok.insert(make_pair(L"cinc-centes",TK_c));
    tok.insert(make_pair(L"sis-cents",TK_c));    tok.insert(make_pair(L"sis-centes",TK_c));
    tok.insert(make_pair(L"set-cents",TK_c));    tok.insert(make_pair(L"set-centes",TK_c));
    tok.insert(make_pair(L"vuit-cents",TK_c));   tok.insert(make_pair(L"vuit-centes",TK_c));
    tok.insert(make_pair(L"huit-cents",TK_c));   tok.insert(make_pair(L"huit-centes",TK_c));
    tok.insert(make_pair(L"nou-cents",TK_c));    tok.insert(make_pair(L"nou-centes",TK_c));
    tok.insert(make_pair(L"trenta",TK_u));       tok.insert(make_pair(L"quaranta",TK_u));
    tok.insert(make_pair(L"cinquanta",TK_u));    tok.insert(make_pair(L"seixanta",TK_u));
    tok.insert(make_pair(L"setanta",TK_u));      tok.insert(make_pair(L"vuitanta",TK_u));
    tok.insert(make_pair(L"huitanta",TK_u));     tok.insert(make_pair(L"noranta",TK_u));     
    tok.insert(make_pair(L"zero",TK_z));
    tok.insert(make_pair(L"un",TK_u));             tok.insert(make_pair(L"una",TK_u));
    tok.insert(make_pair(L"dos",TK_u));            tok.insert(make_pair(L"dues",TK_u));
    tok.insert(make_pair(L"tres",TK_u));           tok.insert(make_pair(L"quatre",TK_u));
    tok.insert(make_pair(L"cinc",TK_u));           tok.insert(make_pair(L"sis",TK_u));
    tok.insert(make_pair(L"set",TK_u));            tok.insert(make_pair(L"vuit",TK_u));
    tok.insert(make_pair(L"nou",TK_u));            tok.insert(make_pair(L"deu",TK_u));
    tok.insert(make_pair(L"onze",TK_u));          tok.insert(make_pair(L"dotze",TK_u));
    tok.insert(make_pair(L"tretze",TK_u));        tok.insert(make_pair(L"catorze",TK_u));
    tok.insert(make_pair(L"quinze",TK_u));        tok.insert(make_pair(L"setze",TK_u));
    tok.insert(make_pair(L"disset",TK_u));        tok.insert(make_pair(L"desset",TK_u)); 
    tok.insert(make_pair(L"dèsset",TK_u)); 
    tok.insert(make_pair(L"divuit",TK_u));        tok.insert(make_pair(L"devuit",TK_u)); 
    tok.insert(make_pair(L"dinou",TK_u));         tok.insert(make_pair(L"denou",TK_u));        
    tok.insert(make_pair(L"vint", TK_u));  
    tok.insert(make_pair(L"vint-i-u",TK_u));      tok.insert(make_pair(L"vint-i-una",TK_u));  
    tok.insert(make_pair(L"vint-i-un",TK_u));     tok.insert(make_pair(L"vint-i-dos",TK_u));
    tok.insert(make_pair(L"vint-i-dues",TK_u));   tok.insert(make_pair(L"vint-i-tres",TK_u));
    tok.insert(make_pair(L"vint-i-quatre",TK_u)); tok.insert(make_pair(L"vint-i-cinc",TK_u));  
    tok.insert(make_pair(L"vint-i-sis",TK_u));    tok.insert(make_pair(L"vint-i-set",TK_u));
    tok.insert(make_pair(L"vint-i-vuit",TK_u));   tok.insert(make_pair(L"vint-i-nou",TK_u));
    tok.insert(make_pair(L"trenta-u",TK_u));      tok.insert(make_pair(L"trenta-una",TK_u));  
    tok.insert(make_pair(L"trenta-un",TK_u));     tok.insert(make_pair(L"trenta-dos",TK_u));
    tok.insert(make_pair(L"trenta-dues",TK_u));   tok.insert(make_pair(L"trenta-tres",TK_u));
    tok.insert(make_pair(L"trenta-quatre",TK_u)); tok.insert(make_pair(L"trenta-cinc",TK_u));  
    tok.insert(make_pair(L"trenta-sis",TK_u));    tok.insert(make_pair(L"trenta-set",TK_u));
    tok.insert(make_pair(L"trenta-vuit",TK_u));   tok.insert(make_pair(L"trenta-nou",TK_u));
    tok.insert(make_pair(L"quaranta-u",TK_u));      tok.insert(make_pair(L"quaranta-una",TK_u));  
    tok.insert(make_pair(L"quaranta-un",TK_u));     tok.insert(make_pair(L"quaranta-dos",TK_u));
    tok.insert(make_pair(L"quaranta-dues",TK_u));   tok.insert(make_pair(L"quaranta-tres",TK_u));
    tok.insert(make_pair(L"quaranta-quatre",TK_u)); tok.insert(make_pair(L"quaranta-cinc",TK_u));  
    tok.insert(make_pair(L"quaranta-sis",TK_u));    tok.insert(make_pair(L"quaranta-set",TK_u));
    tok.insert(make_pair(L"quaranta-vuit",TK_u));   tok.insert(make_pair(L"quaranta-nou",TK_u));
    tok.insert(make_pair(L"cinquanta-u",TK_u));      tok.insert(make_pair(L"cinquanta-una",TK_u));  
    tok.insert(make_pair(L"cinquanta-un",TK_u));     tok.insert(make_pair(L"cinquanta-dos",TK_u));
    tok.insert(make_pair(L"cinquanta-dues",TK_u));   tok.insert(make_pair(L"cinquanta-tres",TK_u));
    tok.insert(make_pair(L"cinquanta-quatre",TK_u)); tok.insert(make_pair(L"cinquanta-cinc",TK_u));  
    tok.insert(make_pair(L"cinquanta-sis",TK_u));    tok.insert(make_pair(L"cinquanta-set",TK_u));
    tok.insert(make_pair(L"cinquanta-vuit",TK_u));   tok.insert(make_pair(L"cinquanta-nou",TK_u));
    tok.insert(make_pair(L"seixanta-u",TK_u));      tok.insert(make_pair(L"seixanta-una",TK_u));  
    tok.insert(make_pair(L"seixanta-un",TK_u));     tok.insert(make_pair(L"seixanta-dos",TK_u));
    tok.insert(make_pair(L"seixanta-dues",TK_u));   tok.insert(make_pair(L"seixanta-tres",TK_u));
    tok.insert(make_pair(L"seixanta-quatre",TK_u)); tok.insert(make_pair(L"seixanta-cinc",TK_u));  
    tok.insert(make_pair(L"seixanta-sis",TK_u));    tok.insert(make_pair(L"seixanta-set",TK_u));
    tok.insert(make_pair(L"seixanta-vuit",TK_u));   tok.insert(make_pair(L"seixanta-nou",TK_u));
    tok.insert(make_pair(L"setanta-u",TK_u));      tok.insert(make_pair(L"setanta-una",TK_u));  
    tok.insert(make_pair(L"setanta-un",TK_u));     tok.insert(make_pair(L"setanta-dos",TK_u));
    tok.insert(make_pair(L"setanta-dues",TK_u));   tok.insert(make_pair(L"setanta-tres",TK_u));
    tok.insert(make_pair(L"setanta-quatre",TK_u)); tok.insert(make_pair(L"setanta-cinc",TK_u));  
    tok.insert(make_pair(L"setanta-sis",TK_u));    tok.insert(make_pair(L"setanta-set",TK_u));
    tok.insert(make_pair(L"setanta-vuit",TK_u));   tok.insert(make_pair(L"setanta-nou",TK_u));
    tok.insert(make_pair(L"vuitanta-u",TK_u));      tok.insert(make_pair(L"vuitanta-una",TK_u));  
    tok.insert(make_pair(L"vuitanta-un",TK_u));     tok.insert(make_pair(L"vuitanta-dos",TK_u));
    tok.insert(make_pair(L"vuitanta-dues",TK_u));   tok.insert(make_pair(L"vuitanta-tres",TK_u));
    tok.insert(make_pair(L"vuitanta-quatre",TK_u)); tok.insert(make_pair(L"vuitanta-cinc",TK_u));  
    tok.insert(make_pair(L"vuitanta-sis",TK_u));    tok.insert(make_pair(L"vuitanta-set",TK_u));
    tok.insert(make_pair(L"vuitanta-vuit",TK_u));   tok.insert(make_pair(L"vuitanta-nou",TK_u));
    tok.insert(make_pair(L"noranta-u",TK_u));      tok.insert(make_pair(L"noranta-una",TK_u));  
    tok.insert(make_pair(L"noranta-un",TK_u));     tok.insert(make_pair(L"noranta-dos",TK_u));
    tok.insert(make_pair(L"noranta-dues",TK_u));   tok.insert(make_pair(L"noranta-tres",TK_u));
    tok.insert(make_pair(L"noranta-quatre",TK_u)); tok.insert(make_pair(L"noranta-cinc",TK_u));  
    tok.insert(make_pair(L"noranta-sis",TK_u));    tok.insert(make_pair(L"noranta-set",TK_u));
    tok.insert(make_pair(L"noranta-vuit",TK_u));   tok.insert(make_pair(L"noranta-nou",TK_u));
    tok.insert(make_pair(L"mil",TK_mil));
    tok.insert(make_pair(L"milió",TK_mill));  tok.insert(make_pair(L"milions",TK_mill));
    tok.insert(make_pair(L"bilió",TK_bill));  tok.insert(make_pair(L"bilions",TK_bill));
    tok.insert(make_pair(L"i",TK_wy));
    tok.insert(make_pair(L"mig",TK_wmedio));  tok.insert(make_pair(L"quart",TK_wcuarto));

    // Initializing power map
    power.insert(make_pair(TK_mil,  1000.0));
    power.insert(make_pair(TK_mill, 1000000.0));
    power.insert(make_pair(TK_bill, 1000000000000.0));

    // Initialize special state attributes
    initialState=ST_B1; stopState=ST_STOP;

    // Initialize Final state set 
    Final.insert(ST_B2);  Final.insert(ST_B3);  Final.insert(ST_Bu);  Final.insert(ST_B5);
    Final.insert(ST_B6);  Final.insert(ST_B7);  Final.insert(ST_Bk); 
    Final.insert(ST_M1);  Final.insert(ST_M2);  Final.insert(ST_M3);  Final.insert(ST_Mu);
    Final.insert(ST_M5);  Final.insert(ST_M6);  Final.insert(ST_M7);  Final.insert(ST_Mk);
    Final.insert(ST_S1);  Final.insert(ST_S2);  Final.insert(ST_S3);  Final.insert(ST_Su);
    Final.insert(ST_S5);  Final.insert(ST_S6);  Final.insert(ST_S7);  Final.insert(ST_Sk);
    Final.insert(ST_M1b); Final.insert(ST_S1b); Final.insert(ST_COD);  

    // Initialize transitions table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;
  
    // Initializing transitions table
    // State B1
    trans[ST_B1][TK_c]=ST_B2;   trans[ST_B1][TK_d]=ST_B3;   trans[ST_B1][TK_u]=ST_Bu; 
    trans[ST_B1][TK_mil]=ST_B5; trans[ST_B1][TK_num]=ST_Bu; trans[ST_B1][TK_code]=ST_COD;
    trans[ST_B1][TK_z]=ST_Sk;
    // State B2
    trans[ST_B2][TK_d]=ST_B3;    trans[ST_B2][TK_u]=ST_Bu;    trans[ST_B2][TK_mil]=ST_B5; 
    trans[ST_B2][TK_bill]=ST_M1; trans[ST_B2][TK_mill]=ST_S1; trans[ST_B2][TK_num]=ST_Bu;
    // State B3
    trans[ST_B3][TK_wy]=ST_B4;   trans[ST_B3][TK_mil]=ST_B5; 
    trans[ST_B3][TK_bill]=ST_M1; trans[ST_B3][TK_mill]=ST_S1;
    // State B4
    trans[ST_B4][TK_u]=ST_Bu; trans[ST_B4][TK_num]=ST_Bu;
    // State Bu
    trans[ST_Bu][TK_mil]=ST_B5; trans[ST_Bu][TK_bill]=ST_M1; trans[ST_Bu][TK_mill]=ST_S1;
    // State B5
    trans[ST_B5][TK_c]=ST_B6;    trans[ST_B5][TK_d]=ST_B7;    trans[ST_B5][TK_u]=ST_Bk; 
    trans[ST_B5][TK_bill]=ST_M1; trans[ST_B5][TK_mill]=ST_S1; trans[ST_B5][TK_num]=ST_Bk;
    // State B6
    trans[ST_B6][TK_d]=ST_B7;    trans[ST_B6][TK_u]=ST_Bk;    trans[ST_B6][TK_num]=ST_Bk;
    trans[ST_B6][TK_bill]=ST_M1; trans[ST_B6][TK_mill]=ST_S1;
    // State B7
    trans[ST_B7][TK_wy]=ST_B8; trans[ST_B7][TK_bill]=ST_M1; trans[ST_B7][TK_mill]=ST_S1;
    // State B8
    trans[ST_B8][TK_u]=ST_Bk;  trans[ST_B8][TK_num]=ST_Bk; 
    // State Bk
    trans[ST_Bk][TK_bill]=ST_M1; trans[ST_Bk][TK_mill]=ST_S1;

    // State M1
    trans[ST_M1][TK_c]=ST_M2; trans[ST_M1][TK_d]=ST_M3;   trans[ST_M1][TK_num]=ST_Mu; 
    trans[ST_M1][TK_u]=ST_Mu; trans[ST_M1][TK_mil]=ST_M5; trans[ST_M1][TK_wy]=ST_M1a; 
    // State M1a
    trans[ST_M1a][TK_wmedio]=ST_M1b; trans[ST_M1a][TK_wcuarto]=ST_M1b;
    // State M1b
    // nothing else expected
    // State M2
    trans[ST_M2][TK_d]=ST_M3;    trans[ST_M2][TK_u]=ST_Mu;   trans[ST_M2][TK_num]=ST_Mu; 
    trans[ST_M2][TK_mil]=ST_M5;  trans[ST_M2][TK_mill]=ST_S1;
    // State M3
    trans[ST_M3][TK_wy]=ST_M4;   trans[ST_M3][TK_mil]=ST_M5;  trans[ST_M3][TK_mill]=ST_S1;
    // State M4
    trans[ST_M4][TK_u]=ST_Mu;  trans[ST_M4][TK_num]=ST_Mu;
    // State Mu
    trans[ST_Mu][TK_mil]=ST_M5; trans[ST_Mu][TK_mill]=ST_S1;
    // State M5
    trans[ST_M5][TK_c]=ST_M6;   trans[ST_M5][TK_d]=ST_M7;   trans[ST_M5][TK_num]=ST_Mk; 
    trans[ST_M5][TK_u]=ST_Mk;   trans[ST_M5][TK_mill]=ST_S1;
    // State M6
    trans[ST_M6][TK_d]=ST_M7;    trans[ST_M6][TK_u]=ST_Mk;
    trans[ST_M6][TK_mill]=ST_S1; trans[ST_M6][TK_num]=ST_Mk; 
    // State M7
    trans[ST_M7][TK_wy]=ST_M8; trans[ST_M7][TK_mill]=ST_S1;
    // State M8
    trans[ST_M8][TK_u]=ST_Mk;   trans[ST_M8][TK_num]=ST_Mk; 
    // State Mk
    trans[ST_Mk][TK_mill]=ST_S1;

    // State S1
    trans[ST_S1][TK_c]=ST_S2; trans[ST_S1][TK_d]=ST_S3;   trans[ST_S1][TK_num]=ST_Su; 
    trans[ST_S1][TK_u]=ST_Su; trans[ST_S1][TK_mil]=ST_S5; trans[ST_S1][TK_wy]=ST_S1a; 
    // State S1a
    trans[ST_S1a][TK_wmedio]=ST_S1b; trans[ST_S1a][TK_wcuarto]=ST_S1b;
    // State S1b
    // nothing else expected
    // State S2
    trans[ST_S2][TK_d]=ST_S3;   trans[ST_S2][TK_u]=ST_Su; 
    trans[ST_S2][TK_mil]=ST_S5; trans[ST_S2][TK_num]=ST_Su; 
    // State S3
    trans[ST_S3][TK_wy]=ST_S4; trans[ST_S3][TK_mil]=ST_S5;
    // State S4
    trans[ST_S4][TK_u]=ST_Su;  trans[ST_S4][TK_num]=ST_Su;
    // State Su
    trans[ST_Su][TK_mil]=ST_S5;
    // State S5
    trans[ST_S5][TK_c]=ST_S6; trans[ST_S5][TK_d]=ST_S7;
    trans[ST_S5][TK_u]=ST_Sk; trans[ST_S5][TK_num]=ST_Sk; 
    // State S6
    trans[ST_S6][TK_d]=ST_S7; trans[ST_S6][TK_u]=ST_Sk; trans[ST_S6][TK_num]=ST_Sk;  
    // State S7
    trans[ST_S7][TK_wy]=ST_S8;
    // State S8
    trans[ST_S8][TK_u]=ST_Sk; trans[ST_S8][TK_num]=ST_Sk; 
    // State Sk
    // nothing else is expected
    // State COD
    // nothing else is expected
 
    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int numbers_ca::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
    wstring form;
    int token;
    map<wstring,int>::const_iterator im;
 
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
    if (RE_number.search(form)) token = TK_num;
    else if (RE_code.search(form)) token = TK_code;

    TRACE(3,L"Leaving state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)); 
    return (token);
  }



  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state, update current 
  ///  nummerical value.
  ///////////////////////////////////////////////////////////////

  void numbers_ca::StateActions(int origin, int state, int token, sentence::const_iterator j, numbers_status *st) const {
    wstring form;
    size_t i;
    long double num;

    form = j->get_lc_form();
    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ["+form+L"]");

    // get token numerical value, if any
    num=0;
    if (token==TK_num) {
      // erase thousand points
      while ((i=form.find_first_of(MACO_Thousand))!=wstring::npos) {
        form.erase(i,1);
      }
      TRACE(3,L"after erasing thousand "+form);
      // make sure decimal point is "."
      if ((i=form.find_last_of(MACO_Decimal))!=wstring::npos) {
        form[i]=L'.';
        TRACE(3,L"after replacing decimal "+form);
      }

      num = util::wstring2longdouble(form);
    }

    // State actions
    switch (state) {
      // ---------------------------------
    case ST_B2:  case ST_M2:  case ST_S2:
    case ST_B3:  case ST_M3:  case ST_S3:
    case ST_Bu:  case ST_Mu:  case ST_Su:
    case ST_B6:  case ST_M6:  case ST_S6:
    case ST_B7:  case ST_M7:  case ST_S7:
    case ST_Bk:  case ST_Mk:  case ST_Sk:
      TRACE(3,L"Actions for normal state");
      if (token==TK_num)
        st->units += num;
      else 
        st->units += value.find(form)->second;
      break;
      // ---------------------------------
    case ST_B5:  case ST_M5:  case ST_S5:
      TRACE(3,L"Actions for 'thousands' state");
      if (st->units==0) st->units=1; // not said: "uno mil ciento treinta y dos"
      st->units *= power.find(token)->second;
      break;
      // ---------------------------------
    case ST_M1:
      TRACE(3,L"Actions for M1 (bilion) state");
      // store bilion value and get ready in case milions are caming
      st->bilion = st->units * power.find(token)->second;
      st->units=0;
      break;
      // ---------------------------------
    case ST_M1b:
      TRACE(3,L"Actions for M1b state");
      // "dos billones y medio/cuarto"
      st->bilion += value.find(form)->second * power.find(TK_bill)->second;
      break;
      // ---------------------------------
    case ST_S1:
      TRACE(3,L"Actions for S1 (milion) state");
      // store milion value and get ready in case thousands are caming
      st->milion = st->units * power.find(token)->second;
      st->units=0;
      break;
      // ---------------------------------
    case ST_S1b:
      TRACE(3,L"Actions for S1b state");
      // "dos millones y medio/cuarto"
      st->milion += value.find(form)->second * power.find(TK_mill)->second;
      break;
      // ---------------------------------
    case ST_COD:
      TRACE(3,L"Actions for COD state");
      st->iscode = CODE;
      break;
      // ---------------------------------
    default: break;
    }

    TRACE(3,L"State actions completed. bilion="+util::longdouble2wstring(st->bilion)+L" milion="+util::longdouble2wstring(st->milion)+L" units="+util::longdouble2wstring(st->units));
  }


  ///////////////////////////////////////////////////////////////
  ///   Set the appropriate lemma and tag for the 
  ///   new multiword.
  ///////////////////////////////////////////////////////////////

  void numbers_ca::SetMultiwordAnalysis(sentence::iterator i, int fstate, const numbers_status *st) const {

    wstring lemma;
    // Setting the analysis for the nummerical expression
    if (st->iscode==CODE) {
      lemma=i->get_form();
    }
    else {
      // compute nummerical value as lemma
      lemma=util::longdouble2wstring(st->bilion + st->milion + st->units);
    }

    i->set_analysis(analysis(lemma,L"Z"));
    TRACE(3,L"Analysis set to: "+lemma+L" Z");

    // record this word was analyzed by this module
    i->set_analyzed_by(word::NUMBERS);    
  }

} // namespace
