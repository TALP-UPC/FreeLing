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
  //        English number recognizer
  //---------------------------------------------------------------------------

  //// State codes 
#define ST_B1 1  // initial state
#define ST_B2 2  // got units  (VALID NUMBER)
#define ST_B3 3  // got "hundred" after units   (VALID NUMBER)
#define ST_B4 4  // got units after "hundred"   (VALID NUMBER)
#define ST_B5 5  // got "thousand" after valid number (VALID NUMBER)
#define ST_B6 6  // got units after "thousand" (VALID NUMBER)
#define ST_B7 7  // got "hundred" from B6  (VALID NUMBER)
#define ST_B8 8  // got units form B7   (VALID NUMBER)
#define ST_M1 9  // got "bilion/s" after a valid number  (VALID NUMBER)
#define ST_M2 10 // see B2 
#define ST_M3 11 // see B3
#define ST_M4 12 // see B4
#define ST_M5 13 // see B5
#define ST_M6 14 // see B6
#define ST_M7 15 // see B7
#define ST_M8 16 // see B8
#define ST_S1 17 // got "milion/s" after a valid number  (VALID NUMBER) 
#define ST_S2 18 // see B2
#define ST_S3 19 // see B3
#define ST_S4 20 // see B4
#define ST_S5 21 // see B5
#define ST_S6 22 // see B6
#define ST_S7 23 // see B7
#define ST_S8 24 // see B8
#define ST_COD 25 // got pseudo-numerical code from initial state
  // stop state
#define ST_STOP 26

  // Token codes
// #define TK_a      1  // word "a" or "an" (e.g. "a hundred").
                        // Not used any more, subsumed by TK_u
#define TK_u      1  // units "three" "seventeen"
#define TK_hundr  2  // word "hundred"   
#define TK_thous  3  // word "mil"
#define TK_mill   4  // word "million" "millions"
#define TK_bill   5  // word "billion" "billions"
#define TK_num    6  // a number (in digits)
#define TK_code   7  // a code (ex. LX-345-2)
#define TK_and    8  // word "and"
#define TK_ord    9  // ordinal code: "1st", "2nd", "43rd", etc
#define TK_other  10

  ///////////////////////////////////////////////////////////////
  ///  Create a numbers recognizer for English
  ///////////////////////////////////////////////////////////////

  numbers_en::numbers_en(const std::wstring &dec, const std::wstring &thou): numbers_module(dec,thou) {
    // Initializing value map
    value.insert(make_pair(L"a",1.0f));
    value.insert(make_pair(L"an",1.0f));
    value.insert(make_pair(L"one",1.0f));            value.insert(make_pair(L"two",2.0f)); 
    value.insert(make_pair(L"three",3.0f));          value.insert(make_pair(L"four",4.0f));
    value.insert(make_pair(L"five",5.0f));           value.insert(make_pair(L"six",6.0f));
    value.insert(make_pair(L"seven",7.0f));          value.insert(make_pair(L"eight",8.0f));
    value.insert(make_pair(L"nine",9.0f));           value.insert(make_pair(L"ten",10.0f));
    value.insert(make_pair(L"eleven",11.0f));        value.insert(make_pair(L"twelve",12.0f));
    value.insert(make_pair(L"thirteen",13.0f));      value.insert(make_pair(L"fourteen",14.0f));
    value.insert(make_pair(L"fifteen",15.0f));        value.insert(make_pair(L"sixteen",16.0f));
    value.insert(make_pair(L"seventeen",17.0f));     value.insert(make_pair(L"eighteen",18.0f)); 
    value.insert(make_pair(L"nineteen",19.0f));      
    value.insert(make_pair(L"twenty", 20.0f));       value.insert(make_pair(L"twenty-one",21.0f));
    value.insert(make_pair(L"twenty-two",22.0f));    value.insert(make_pair(L"twenty-three",23.0f));
    value.insert(make_pair(L"twenty-four",24.0f));   value.insert(make_pair(L"twenty-five",25.0f));  
    value.insert(make_pair(L"twenty-six",26.0f));    value.insert(make_pair(L"twenty-seven",27.0f));
    value.insert(make_pair(L"twenty-eight",28.0f));  value.insert(make_pair(L"twenty-nine",29.0f));
    value.insert(make_pair(L"thirty",30.0f));        value.insert(make_pair(L"thirty-one",31.0f)); 
    value.insert(make_pair(L"thirty-two",32.0f));    value.insert(make_pair(L"thirty-three",33.0f));
    value.insert(make_pair(L"thirty-four",34.0f));   value.insert(make_pair(L"thirty-five",35.0f));  
    value.insert(make_pair(L"thirty-six",36.0f));    value.insert(make_pair(L"thirty-seven",37.0f));
    value.insert(make_pair(L"thirty-eight",38.0f));  value.insert(make_pair(L"thirty-nine",39.0f));
    value.insert(make_pair(L"fourty",40.0f));        value.insert(make_pair(L"fourty-one",41.0f));
    value.insert(make_pair(L"fourty-two",42.0f));    value.insert(make_pair(L"fourty-three",43.0f));
    value.insert(make_pair(L"fourty-four",44.0f));   value.insert(make_pair(L"fourty-five",45.0f));  
    value.insert(make_pair(L"fourty-six",46.0f));    value.insert(make_pair(L"fourty-seven",47.0f));
    value.insert(make_pair(L"fourty-eight",48.0f));  value.insert(make_pair(L"fourty-nine",49.0f));
    value.insert(make_pair(L"forty",40.0f));         value.insert(make_pair(L"forty-one",41.0f));
    value.insert(make_pair(L"forty-two",42.0f));     value.insert(make_pair(L"forty-three",43.0f));
    value.insert(make_pair(L"forty-four",44.0f));    value.insert(make_pair(L"forty-five",45.0f));  
    value.insert(make_pair(L"forty-six",46.0f));     value.insert(make_pair(L"forty-seven",47.0f));
    value.insert(make_pair(L"forty-eight",48.0f));   value.insert(make_pair(L"forty-nine",49.0f));
    value.insert(make_pair(L"fifty",50.0f));         value.insert(make_pair(L"fifty-one",51.0f));
    value.insert(make_pair(L"fifty-two",52.0f));     value.insert(make_pair(L"fifty-three",53.0f));
    value.insert(make_pair(L"fifty-four",54.0f));    value.insert(make_pair(L"fifty-five",55.0f));  
    value.insert(make_pair(L"fifty-six",56.0f));     value.insert(make_pair(L"fifty-seven",57.0f));
    value.insert(make_pair(L"fifty-eight",58.0f));   value.insert(make_pair(L"fifty-nine",59.0f));
    value.insert(make_pair(L"sixty",60.0f));         value.insert(make_pair(L"sixty-one",61.0f));
    value.insert(make_pair(L"sixty-two",62.0f));     value.insert(make_pair(L"sixty-three",63.0f));
    value.insert(make_pair(L"sixty-four",64.0f));    value.insert(make_pair(L"sixty-five",65.0f));  
    value.insert(make_pair(L"sixty-six",66.0f));     value.insert(make_pair(L"sixty-seven",67.0f));
    value.insert(make_pair(L"sixty-eight",68.0f));   value.insert(make_pair(L"sixty-nine",69.0f));
    value.insert(make_pair(L"seventy",70.0f));       value.insert(make_pair(L"seventy-one",71.0f));
    value.insert(make_pair(L"seventy-two",72.0f));   value.insert(make_pair(L"seventy-three",73.0f));
    value.insert(make_pair(L"seventy-four",74.0f));  value.insert(make_pair(L"seventy-five",75.0f));  
    value.insert(make_pair(L"seventy-six",76.0f));   value.insert(make_pair(L"seventy-seven",77.0f));
    value.insert(make_pair(L"seventy-eight",78.0f)); value.insert(make_pair(L"seventy-nine",79.0f));
    value.insert(make_pair(L"eighty",80.0f));        value.insert(make_pair(L"eighty-one",81.0f));    
    value.insert(make_pair(L"eighty-two",82.0f));    value.insert(make_pair(L"eighty-three",83.0f));
    value.insert(make_pair(L"eighty-four",84.0f));   value.insert(make_pair(L"eighty-five",85.0f));  
    value.insert(make_pair(L"eighty-six",86.0f));    value.insert(make_pair(L"eighty-seven",87.0f));
    value.insert(make_pair(L"eighty-eight",88.0f));  value.insert(make_pair(L"eighty-nine",89.0f));
    value.insert(make_pair(L"ninety",90.0f));        value.insert(make_pair(L"ninety-one",91.0f)); 
    value.insert(make_pair(L"ninety-two",92.0f));    value.insert(make_pair(L"ninety-three",93.0f));
    value.insert(make_pair(L"ninety-four",94.0f));   value.insert(make_pair(L"ninety-five",95.0f));  
    value.insert(make_pair(L"ninety-six",96.0f));    value.insert(make_pair(L"ninety-seven",97.0f));
    value.insert(make_pair(L"ninety-eight",98.0f));  value.insert(make_pair(L"ninety-nine",99.0f));

    // Initializing token map
    tok.insert(make_pair(L"a",TK_u));
    tok.insert(make_pair(L"an",TK_u));
    tok.insert(make_pair(L"one",TK_u));             tok.insert(make_pair(L"two",TK_u));            
    tok.insert(make_pair(L"three",TK_u));          tok.insert(make_pair(L"four",TK_u));
    tok.insert(make_pair(L"five",TK_u));           tok.insert(make_pair(L"six",TK_u));
    tok.insert(make_pair(L"seven",TK_u));          tok.insert(make_pair(L"eight",TK_u));
    tok.insert(make_pair(L"nine",TK_u));           tok.insert(make_pair(L"ten",TK_u));
    tok.insert(make_pair(L"eleven",TK_u));        tok.insert(make_pair(L"twelve",TK_u));
    tok.insert(make_pair(L"thirteen",TK_u));      tok.insert(make_pair(L"fourteen",TK_u));
    tok.insert(make_pair(L"fifteen",TK_u));        tok.insert(make_pair(L"sixteen",TK_u));
    tok.insert(make_pair(L"seventeen",TK_u));     tok.insert(make_pair(L"eighteen",TK_u)); 
    tok.insert(make_pair(L"nineteen",TK_u));      
    tok.insert(make_pair(L"twenty", TK_u));       tok.insert(make_pair(L"twenty-one",TK_u));
    tok.insert(make_pair(L"twenty-two",TK_u));    tok.insert(make_pair(L"twenty-three",TK_u));
    tok.insert(make_pair(L"twenty-four",TK_u));   tok.insert(make_pair(L"twenty-five",TK_u));
    tok.insert(make_pair(L"twenty-six",TK_u));    tok.insert(make_pair(L"twenty-seven",TK_u));
    tok.insert(make_pair(L"twenty-eight",TK_u));  tok.insert(make_pair(L"twenty-nine",TK_u));
    tok.insert(make_pair(L"thirty",TK_u));        tok.insert(make_pair(L"thirty-one",TK_u)); 
    tok.insert(make_pair(L"thirty-two",TK_u));    tok.insert(make_pair(L"thirty-three",TK_u));
    tok.insert(make_pair(L"thirty-four",TK_u));   tok.insert(make_pair(L"thirty-five",TK_u));  
    tok.insert(make_pair(L"thirty-six",TK_u));    tok.insert(make_pair(L"thirty-seven",TK_u));
    tok.insert(make_pair(L"thirty-eight",TK_u));  tok.insert(make_pair(L"thirty-nine",TK_u));
    tok.insert(make_pair(L"fourty",TK_u));        tok.insert(make_pair(L"fourty-one",TK_u));
    tok.insert(make_pair(L"fourty-two",TK_u));    tok.insert(make_pair(L"fourty-three",TK_u));
    tok.insert(make_pair(L"fourty-four",TK_u));   tok.insert(make_pair(L"fourty-five",TK_u));  
    tok.insert(make_pair(L"fourty-six",TK_u));    tok.insert(make_pair(L"fourty-seven",TK_u));
    tok.insert(make_pair(L"fourty-eight",TK_u));  tok.insert(make_pair(L"fourty-nine",TK_u));
    tok.insert(make_pair(L"forty",TK_u));         tok.insert(make_pair(L"forty-one",TK_u));
    tok.insert(make_pair(L"forty-two",TK_u));     tok.insert(make_pair(L"forty-three",TK_u));
    tok.insert(make_pair(L"forty-four",TK_u));    tok.insert(make_pair(L"forty-five",TK_u));  
    tok.insert(make_pair(L"forty-six",TK_u));     tok.insert(make_pair(L"forty-seven",TK_u));
    tok.insert(make_pair(L"forty-eight",TK_u));   tok.insert(make_pair(L"forty-nine",TK_u));
    tok.insert(make_pair(L"fifty",TK_u));         tok.insert(make_pair(L"fifty-one",TK_u));
    tok.insert(make_pair(L"fifty-two",TK_u));     tok.insert(make_pair(L"fifty-three",TK_u));
    tok.insert(make_pair(L"fifty-four",TK_u));    tok.insert(make_pair(L"fifty-five",TK_u));  
    tok.insert(make_pair(L"fifty-six",TK_u));     tok.insert(make_pair(L"fifty-seven",TK_u));
    tok.insert(make_pair(L"fifty-eight",TK_u));   tok.insert(make_pair(L"fifty-nine",TK_u));
    tok.insert(make_pair(L"sixty",TK_u));         tok.insert(make_pair(L"sixty-one",TK_u));
    tok.insert(make_pair(L"sixty-two",TK_u));     tok.insert(make_pair(L"sixty-three",TK_u));
    tok.insert(make_pair(L"sixty-four",TK_u));    tok.insert(make_pair(L"sixty-five",TK_u));  
    tok.insert(make_pair(L"sixty-six",TK_u));     tok.insert(make_pair(L"sixty-seven",TK_u));
    tok.insert(make_pair(L"sixty-eight",TK_u));   tok.insert(make_pair(L"sixty-nine",TK_u));
    tok.insert(make_pair(L"seventy",TK_u));       tok.insert(make_pair(L"seventy-one",TK_u));
    tok.insert(make_pair(L"seventy-two",TK_u));   tok.insert(make_pair(L"seventy-three",TK_u));
    tok.insert(make_pair(L"seventy-four",TK_u));  tok.insert(make_pair(L"seventy-five",TK_u));  
    tok.insert(make_pair(L"seventy-six",TK_u));   tok.insert(make_pair(L"seventy-seven",TK_u));
    tok.insert(make_pair(L"seventy-eight",TK_u)); tok.insert(make_pair(L"seventy-nine",TK_u));
    tok.insert(make_pair(L"eighty",TK_u));        tok.insert(make_pair(L"eighty-one",TK_u));    
    tok.insert(make_pair(L"eighty-two",TK_u));    tok.insert(make_pair(L"eighty-three",TK_u));
    tok.insert(make_pair(L"eighty-four",TK_u));   tok.insert(make_pair(L"eighty-five",TK_u));  
    tok.insert(make_pair(L"eighty-six",TK_u));    tok.insert(make_pair(L"eighty-seven",TK_u));
    tok.insert(make_pair(L"eighty-eight",TK_u));  tok.insert(make_pair(L"eighty-nine",TK_u));
    tok.insert(make_pair(L"ninety",TK_u));        tok.insert(make_pair(L"ninety-one",TK_u)); 
    tok.insert(make_pair(L"ninety-two",TK_u));    tok.insert(make_pair(L"ninety-three",TK_u));
    tok.insert(make_pair(L"ninety-four",TK_u));   tok.insert(make_pair(L"ninety-five",TK_u));  
    tok.insert(make_pair(L"ninety-six",TK_u));    tok.insert(make_pair(L"ninety-seven",TK_u));
    tok.insert(make_pair(L"ninety-eight",TK_u));  tok.insert(make_pair(L"ninety-nine",TK_u));

    tok.insert(make_pair(L"hundred",TK_hundr));
    tok.insert(make_pair(L"thousand",TK_thous));
    tok.insert(make_pair(L"milion",TK_mill));  tok.insert(make_pair(L"milions",TK_mill));
    tok.insert(make_pair(L"bilion",TK_bill));  tok.insert(make_pair(L"bilions",TK_bill));
    tok.insert(make_pair(L"million",TK_mill));  tok.insert(make_pair(L"millions",TK_mill));
    tok.insert(make_pair(L"billion",TK_bill));  tok.insert(make_pair(L"billions",TK_bill));

    tok.insert(make_pair(L"and",TK_and));

    // Initializing power map
    power.insert(make_pair(TK_hundr, 100.0));
    power.insert(make_pair(TK_thous, 1000.0));
    power.insert(make_pair(TK_mill,  1000000.0));
    power.insert(make_pair(TK_bill,  1000000000.0));

    // Initialize special state attributes
    initialState=ST_B1; stopState=ST_STOP;

    // Initialize Final state set 
    Final.insert(ST_B2);  Final.insert(ST_B3);  Final.insert(ST_B4);  Final.insert(ST_B5);
    Final.insert(ST_B6);  Final.insert(ST_B7);  Final.insert(ST_B8); 
    Final.insert(ST_M1);  Final.insert(ST_M2);  Final.insert(ST_M3);  Final.insert(ST_M4);
    Final.insert(ST_M5);  Final.insert(ST_M6);  Final.insert(ST_M7);  Final.insert(ST_M8);
    Final.insert(ST_S1);  Final.insert(ST_S2);  Final.insert(ST_S3);  Final.insert(ST_S4);
    Final.insert(ST_S5);  Final.insert(ST_S6);  Final.insert(ST_S7);  Final.insert(ST_S8);
    Final.insert(ST_COD);  

    // Initialize transitions table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;
  
    // Initializing transitions table
    // State B1
    trans[ST_B1][TK_u]=ST_B2; 
    trans[ST_B1][TK_num]=ST_B2;   trans[ST_B1][TK_code]=ST_COD;   trans[ST_B1][TK_ord]=ST_COD; 
    // State B2
    trans[ST_B2][TK_hundr]=ST_B3; trans[ST_B2][TK_thous]=ST_B5; 
    trans[ST_B2][TK_bill]=ST_M1;  trans[ST_B2][TK_mill]=ST_S1;
    // State B3
    trans[ST_B3][TK_u]=ST_B4;     trans[ST_B3][TK_num]=ST_B4;     trans[ST_B3][TK_thous]=ST_B5; 
    trans[ST_B3][TK_bill]=ST_M1;  trans[ST_B3][TK_mill]=ST_S1;    trans[ST_B3][TK_and]=ST_B3;
    // State B4
    trans[ST_B4][TK_thous]=ST_B5; trans[ST_B4][TK_bill]=ST_M1;   trans[ST_B4][TK_mill]=ST_S1;
    // State B5
    trans[ST_B5][TK_u]=ST_B6;      trans[ST_B5][TK_num]=ST_B6;   trans[ST_B5][TK_bill]=ST_M1; trans[ST_B5][TK_mill]=ST_S1;
    trans[ST_B5][TK_and]=ST_B5; 
    // State B6
    trans[ST_B6][TK_hundr]=ST_B7; trans[ST_B6][TK_bill]=ST_M1;   trans[ST_B6][TK_mill]=ST_S1;
    // State B7
    trans[ST_B7][TK_u]=ST_B8;      trans[ST_B7][TK_num]=ST_B8;   trans[ST_B7][TK_bill]=ST_M1; 
    trans[ST_B7][TK_mill]=ST_S1;   trans[ST_B7][TK_and]=ST_B7;
    // State B8
    trans[ST_B8][TK_bill]=ST_M1;  trans[ST_B8][TK_mill]=ST_S1;
 
    // State M1
    trans[ST_M1][TK_u]=ST_M2;     trans[ST_M1][TK_num]=ST_M2;   trans[ST_M1][TK_bill]=ST_S1;
    // State M2
    trans[ST_M2][TK_hundr]=ST_M3; trans[ST_M2][TK_thous]=ST_M5;   trans[ST_M2][TK_bill]=ST_S1; 
    // State M3
    trans[ST_M3][TK_u]=ST_M4;     trans[ST_M3][TK_num]=ST_M4;     trans[ST_M3][TK_thous]=ST_M5; 
    trans[ST_M3][TK_bill]=ST_S1;  trans[ST_M3][TK_and]=ST_M3;
    // State M4
    trans[ST_M4][TK_thous]=ST_M5; trans[ST_M4][TK_bill]=ST_S1; 
    // State M5
    trans[ST_M5][TK_u]=ST_M6;     trans[ST_M5][TK_num]=ST_M6;     trans[ST_M5][TK_bill]=ST_S1; 
    trans[ST_M5][TK_and]=ST_M5; 
    // State M6
    trans[ST_M6][TK_hundr]=ST_M7; trans[ST_M6][TK_bill]=ST_S1; 
    // State M7
    trans[ST_M7][TK_u]=ST_M8;     trans[ST_M7][TK_num]=ST_M8;     trans[ST_M7][TK_bill]=ST_S1;
    trans[ST_M7][TK_and]=ST_M7;
    // State M8
    trans[ST_M8][TK_mill]=ST_S1; 

    // State S1
    trans[ST_S1][TK_u]=ST_S2;     trans[ST_S1][TK_num]=ST_S2;  
    // State S2
    trans[ST_S2][TK_hundr]=ST_S3; trans[ST_S2][TK_thous]=ST_S5; 
    // State S3
    trans[ST_S3][TK_u]=ST_S4;     trans[ST_S3][TK_num]=ST_S4;     trans[ST_S3][TK_thous]=ST_S5; 
    trans[ST_S3][TK_and]=ST_S3;
    // State S4
    trans[ST_S4][TK_thous]=ST_S5;
    // State S5
    trans[ST_S5][TK_u]=ST_S6;     trans[ST_S5][TK_num]=ST_S6; 
    trans[ST_S5][TK_and]=ST_S5; 
    // State S6
    trans[ST_S6][TK_hundr]=ST_S7;
    // State S7
    trans[ST_S7][TK_u]=ST_S8;     trans[ST_S7][TK_num]=ST_S8;     trans[ST_S7][TK_and]=ST_S7;
    // State S8
    // nothing else expected

    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int numbers_en::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
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

    TRACE(3,L"Leaving state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)); 
    return (token);
  }



  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state, update current 
  ///  nummerical value.
  ///////////////////////////////////////////////////////////////

  void numbers_en::StateActions(int origin, int state, int token, sentence::const_iterator j, numbers_status *st) const {
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
    case ST_B2:  case ST_M2:  case ST_S2:
    case ST_B4:  case ST_M4:  case ST_S4:
    case ST_B6:  case ST_M6:  case ST_S6:
    case ST_B8:  case ST_M8:  case ST_S8:
      TRACE(3,L"Actions for normal state");
      if (token==TK_num)
        st->units += num;
      else
        st->units += value.find(form)->second;
      break;
      // ---------------------------------
    case ST_B3:  case ST_M3:  case ST_S3:
    case ST_B5:  case ST_M5:  case ST_S5:
    case ST_B7:  case ST_M7:  case ST_S7:
      if (token != TK_and) {
        TRACE(3,L"Actions for 'thousands' and 'hundreds' state");
        st->units *= power.find(token)->second;
      }
      break;
      // ---------------------------------
    case ST_M1:
      TRACE(3,L"Actions for M1 (bilion) state");
      // store bilion value and get ready in case milions are caming
      st->bilion = st->units * power.find(token)->second;
      st->units=0;
      break;
    case ST_S1:
      TRACE(3,L"Actions for S1 (milion) state");
      // store milion value and get ready in case thousands are caming
      st->milion = st->units * power.find(token)->second;
      st->units=0;
      break;
      // ---------------------------------
    case ST_COD:
      TRACE(3,L"Actions for COD state");    
      if (token==TK_code) st->iscode=CODE;
      else if (token==TK_ord) st->iscode=ORD;
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

  void numbers_en::SetMultiwordAnalysis(sentence::iterator i, int fstate, const numbers_status *st) const {
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

    // record this word was analyzed by this module
    i->set_analyzed_by(word::NUMBERS);    
  }
} // namespace
