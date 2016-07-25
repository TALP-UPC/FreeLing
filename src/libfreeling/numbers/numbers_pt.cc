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
  //        Portuguese number recognizer
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
#define ST_X1 36 // got "millar", "docenas" "centenares" "decenas" after a number
#define ST_X2 37 // got "y" after "millar/docenas/etc"
#define ST_X3 38 // got "millar y medio/cuarto",  "X docenas y media"
#define ST_X4 39 // got "medio" as 1st word
#define ST_X5 40 // got "medio centenar/millar/etc"
#define ST_B2b 42 // 
#define ST_B4b 43 // 
#define ST_Bu2 44 // 

  // stop state
#define ST_STOP 41

  // Token codes
#define TK_c      1  // hundreds "cien" "doscientos" 
#define TK_d      2  // tens "treinta" "cuarenta"
#define TK_u      3  // units "tres" "cuatro"
#define TK_wy     4  // word "y"
#define TK_wmedio 5  // word "medio"
#define TK_wcuarto 6 // word "cuarto"
#define TK_mil    7  // word "mil"   
#define TK_mill   8  // word "millon" "millones"
#define TK_bill   9  // word "billon" "billones"
#define TK_num   10  // a number (in digits)
#define TK_code  11  // a code (ex. LX-345-2)
#define TK_milr  12  // word "millar/es"
#define TK_cent  13  // word "centenar/es"
#define TK_dec   14  // word "decena/s"
#define TK_doc   15  // word "docena/s
#define TK_other 16
#define TK_du 17     // tens and units: "dez" - "cento e dez" (vs. vinte e dez)
 
 
  ///////////////////////////////////////////////////////////////
  ///  Create a numbers recognizer for Portuguese.
  ///////////////////////////////////////////////////////////////

  numbers_pt::numbers_pt(const std::wstring &dec, const std::wstring &thou): numbers_module(dec,thou)
  {  
    // Initializing value map
    value.insert(make_pair(L"cem",100.0f));          value.insert(make_pair(L"cento",100.0f));
    value.insert(make_pair(L"duzentos",200.0f));    value.insert(make_pair(L"duzentas",200.0f));
    value.insert(make_pair(L"trezentos",300.0f));   value.insert(make_pair(L"trezentas",300.0f));
    value.insert(make_pair(L"quatrocentos",400.0f)); value.insert(make_pair(L"quatrocentas",400.0f));
    value.insert(make_pair(L"quinhentos",500.0f));    value.insert(make_pair(L"quinhentas",500.0f));
    value.insert(make_pair(L"seiscentos",600.0f));   value.insert(make_pair(L"seiscentas",600.0f));
    value.insert(make_pair(L"setecentos",700.0f));   value.insert(make_pair(L"setecentas",700.0f));
    value.insert(make_pair(L"oitocentos",800.0f));   value.insert(make_pair(L"oitocentas",800.0f));
    value.insert(make_pair(L"novecentos",900.0f));   value.insert(make_pair(L"novecentas",900.0f));
    value.insert(make_pair(L"trinta",30.0f));     value.insert(make_pair(L"quarenta",40.0f));
    value.insert(make_pair(L"cinquenta",50.0f));   value.insert(make_pair(L"sessenta",60.0f));
    value.insert(make_pair(L"setenta",70.0f));     value.insert(make_pair(L"oitenta",80.0f));
    value.insert(make_pair(L"noventa",90.0f));     value.insert(make_pair(L"cinqüenta",50.0f));
    value.insert(make_pair(L"um",1.0f));           value.insert(make_pair(L"uma",1.0f));
    value.insert(make_pair(L"dois",2.0f));    value.insert(make_pair(L"duas",2.0f));
    value.insert(make_pair(L"três",3.0f));    value.insert(make_pair(L"quatro",4.0f));
    value.insert(make_pair(L"cinco",5.0f));   value.insert(make_pair(L"seis",6.0f));
    value.insert(make_pair(L"sete",7.0f));   value.insert(make_pair(L"oito",8.0f));
    value.insert(make_pair(L"nove",9.0f));   value.insert(make_pair(L"dez",10.0f));
    value.insert(make_pair(L"onze",11.0f));   value.insert(make_pair(L"doze",12.0f));
    value.insert(make_pair(L"treze",13.0f));  value.insert(make_pair(L"quatorze",14.0f));
    value.insert(make_pair(L"quinze",15.0f)); value.insert(make_pair(L"dezasseis",16.0f));
    value.insert(make_pair(L"dezassete",17.0f)); value.insert(make_pair(L"catorze",14.0f));
    value.insert(make_pair(L"dezoito",18.0f)); value.insert(make_pair(L"dezanove",19.0f));
    value.insert(make_pair(L"vinte",20.0f));    value.insert(make_pair(L"dous",2.0f));
    value.insert(make_pair(L"meio",0.5f));    value.insert(make_pair(L"meia",0.5f));  
    value.insert(make_pair(L"quarto",0.25f));

    // Initializing token map
    tok.insert(make_pair(L"cem",TK_c));        tok.insert(make_pair(L"cento",TK_c));
    tok.insert(make_pair(L"duzentos",TK_c));  tok.insert(make_pair(L"duzentas",TK_c));
    tok.insert(make_pair(L"trezentos",TK_c)); tok.insert(make_pair(L"trezentas",TK_c));
    tok.insert(make_pair(L"quatrocentos",TK_c)); tok.insert(make_pair(L"quatrocentas",TK_c));
    tok.insert(make_pair(L"quinhentos",TK_c));  tok.insert(make_pair(L"quinhentas",TK_c));
    tok.insert(make_pair(L"seiscentos",TK_c)); tok.insert(make_pair(L"seiscentas",TK_c));
    tok.insert(make_pair(L"setecentos",TK_c)); tok.insert(make_pair(L"setecentas",TK_c));
    tok.insert(make_pair(L"oitocentos",TK_c)); tok.insert(make_pair(L"oitocentas",TK_c));
    tok.insert(make_pair(L"novecentos",TK_c)); tok.insert(make_pair(L"novecentas",TK_c));
    tok.insert(make_pair(L"trinta",TK_d));     tok.insert(make_pair(L"quarenta",TK_d));
    tok.insert(make_pair(L"cinquenta",TK_d));   tok.insert(make_pair(L"sessenta",TK_d));
    tok.insert(make_pair(L"setenta",TK_d));     tok.insert(make_pair(L"oitenta",TK_d));
    tok.insert(make_pair(L"noventa",TK_d));  tok.insert(make_pair(L"vinte",TK_d));
    tok.insert(make_pair(L"um",TK_u));       tok.insert(make_pair(L"uma",TK_u));
    tok.insert(make_pair(L"dois",TK_u));      tok.insert(make_pair(L"duas",TK_u));      
    tok.insert(make_pair(L"três",TK_u));     tok.insert(make_pair(L"cinqüenta",TK_d));
    tok.insert(make_pair(L"quatro",TK_u));   tok.insert(make_pair(L"cinco",TK_u));
    tok.insert(make_pair(L"seis",TK_u));     tok.insert(make_pair(L"sete",TK_u));
    tok.insert(make_pair(L"oito",TK_u));     tok.insert(make_pair(L"nove",TK_u));
    //  tok.insert(make_pair(L"dez",TK_u));     tok.insert(make_pair(L"onze",TK_u));
    //  tok.insert(make_pair(L"doze",TK_u));     tok.insert(make_pair(L"treze",TK_u));
    //  tok.insert(make_pair(L"catorze",TK_u));  tok.insert(make_pair(L"quinze",TK_u));
    //  tok.insert(make_pair(L"dezasseis",TK_u));
    //  tok.insert(make_pair(L"dezassete",TK_u)); tok.insert(make_pair(L"dezoito",TK_u));
    //  tok.insert(make_pair(L"dezanove",TK_u)); tok.insert(make_pair(L"vinte",TK_u));
    tok.insert(make_pair(L"dez",TK_du));     tok.insert(make_pair(L"onze",TK_du));
    tok.insert(make_pair(L"doze",TK_du));     tok.insert(make_pair(L"treze",TK_du));
    tok.insert(make_pair(L"catorze",TK_du));  tok.insert(make_pair(L"quinze",TK_du));
    tok.insert(make_pair(L"dezasseis",TK_du));
    tok.insert(make_pair(L"dezassete",TK_du)); tok.insert(make_pair(L"dezoito",TK_du));
    tok.insert(make_pair(L"dezanove",TK_du)); tok.insert(make_pair(L"vinte",TK_du));
    //
    tok.insert(make_pair(L"mil",TK_mil));
    tok.insert(make_pair(L"milhão",TK_mill)); tok.insert(make_pair(L"milhões",TK_mill));
    tok.insert(make_pair(L"bilhão",TK_bill));    tok.insert(make_pair(L"bilhões",TK_bill));
    tok.insert(make_pair(L"bilião",TK_bill));    tok.insert(make_pair(L"biliões",TK_bill));

    tok.insert(make_pair(L"milhar",TK_milr));    tok.insert(make_pair(L"milhares",TK_milr));
    tok.insert(make_pair(L"milheiro",TK_milr));    tok.insert(make_pair(L"milheiros",TK_milr));
    tok.insert(make_pair(L"centenar",TK_cent));    tok.insert(make_pair(L"centenares",TK_cent));
    tok.insert(make_pair(L"dezena",TK_dec));    tok.insert(make_pair(L"dezenas",TK_dec));
    tok.insert(make_pair(L"dúzia",TK_doc));    tok.insert(make_pair(L"dúzias",TK_doc));

    tok.insert(make_pair(L"e",TK_wy));
    tok.insert(make_pair(L"meio",TK_wmedio));
    tok.insert(make_pair(L"meia",TK_wmedio));
    tok.insert(make_pair(L"quarto",TK_wcuarto));

    // Initializing power map
    power.insert(make_pair(TK_mil,  1000.0));
    power.insert(make_pair(TK_mill, 1000000.0));
    power.insert(make_pair(TK_bill, 1000000000000.0));
    power.insert(make_pair(TK_milr, 1000.0));
    power.insert(make_pair(TK_cent, 100.0));
    power.insert(make_pair(TK_doc,  12.0));
    power.insert(make_pair(TK_dec,  10.0));

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
    Final.insert(ST_X1);  Final.insert(ST_X3);  Final.insert(ST_X5);

    // Initialize transitions table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;
  
    // Initializing transitions table
    // State B1
    trans[ST_B1][TK_c]=ST_B2;   trans[ST_B1][TK_d]=ST_B3;   trans[ST_B1][TK_u]=ST_Bu;
    trans[ST_B1][TK_mil]=ST_B5; trans[ST_B1][TK_num]=ST_Bu; trans[ST_B1][TK_code]=ST_COD;
    trans[ST_B1][TK_wmedio]=ST_X4; trans[ST_B1][TK_du]=ST_Bu;
    // State B2
    trans[ST_B2][TK_wy]=ST_B4;
    trans[ST_B2][TK_mil]=ST_B5;
    trans[ST_B2][TK_bill]=ST_M1; trans[ST_B2][TK_mill]=ST_S1;
    // State B2b
    trans[ST_B2b][TK_u]=ST_Bu; trans[ST_B2b][TK_num]=ST_Bu;
    // State B3
    trans[ST_B3][TK_wy]=ST_B2b;   trans[ST_B3][TK_mil]=ST_B5; 
    trans[ST_B3][TK_bill]=ST_M1; trans[ST_B3][TK_mill]=ST_S1;
    // State B4
    trans[ST_B4][TK_u]=ST_Bu; trans[ST_B4][TK_num]=ST_Bu;
    trans[ST_B4][TK_du]=ST_Bu;
    trans[ST_B4][TK_d]=ST_B3;
    // State B4b
    trans[ST_B4b][TK_u]=ST_Bu; trans[ST_B4b][TK_num]=ST_Bu;
    trans[ST_B4b][TK_du]=ST_Bu;
    trans[ST_B4b][TK_d]=ST_Bu; trans[ST_B4b][TK_c]=ST_Bu;
    trans[ST_B4b][TK_d]=ST_B3;
    // State Bu
    trans[ST_Bu][TK_mil]=ST_B5; trans[ST_Bu][TK_bill]=ST_M1; trans[ST_Bu][TK_mill]=ST_S1;
    trans[ST_Bu][TK_milr]=ST_X1; trans[ST_Bu][TK_cent]=ST_X1;
    trans[ST_Bu][TK_dec]=ST_X1;  trans[ST_Bu][TK_doc]=ST_X1;
    // State B5
    trans[ST_B5][TK_wy]=ST_B4b;
    trans[ST_B5][TK_c]=ST_B2;
    trans[ST_B5][TK_bill]=ST_M1; trans[ST_B5][TK_mill]=ST_S1; trans[ST_B5][TK_num]=ST_Bk;
    // State B6 ???
    trans[ST_B6][TK_d]=ST_B7;    trans[ST_B6][TK_u]=ST_Bk;    trans[ST_B6][TK_num]=ST_Bk;
    trans[ST_B6][TK_bill]=ST_M1; trans[ST_B6][TK_mill]=ST_S1;
    // State B7 ???
    trans[ST_B7][TK_wy]=ST_B8; trans[ST_B7][TK_bill]=ST_M1; trans[ST_B7][TK_mill]=ST_S1;
    // State B8 ???
    trans[ST_B8][TK_u]=ST_Bk;  trans[ST_B8][TK_num]=ST_Bk; 
    // State Bk
    trans[ST_Bk][TK_bill]=ST_M1; trans[ST_Bk][TK_mill]=ST_S1;

    // State M1
    trans[ST_M1][TK_c]=ST_M2; trans[ST_M1][TK_wy]=ST_S1a;
    // State M1a
    trans[ST_M1a][TK_wmedio]=ST_M1b; trans[ST_M1a][TK_wcuarto]=ST_M1b;
    // State M1b
    // nothing else expected
    // State M2
    trans[ST_M2][TK_wy]=ST_B4;
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
    trans[ST_S1][TK_c]=ST_S2;
    trans[ST_S1][TK_wy]=ST_S1a;
    // State S1a
    trans[ST_S1a][TK_wmedio]=ST_S1b; trans[ST_S1a][TK_wcuarto]=ST_S1b;
    // From B4b
    trans[ST_S1a][TK_u]=ST_Bu; trans[ST_S1a][TK_num]=ST_Bu;
    trans[ST_S1a][TK_du]=ST_Bu;
    trans[ST_S1a][TK_d]=ST_Bu; trans[ST_S1a][TK_c]=ST_Bu;
    trans[ST_S1a][TK_d]=ST_B3;
    // State S1b
    // nothing else expected
    // State S2
    trans[ST_S2][TK_wy]=ST_B4;
    // State S3 ???
    trans[ST_S3][TK_wy]=ST_S4; trans[ST_S3][TK_mil]=ST_S5;
    // State S4 ???
    trans[ST_S4][TK_u]=ST_Su;  trans[ST_S4][TK_num]=ST_Su;
    // State Su ???
    trans[ST_Su][TK_mil]=ST_S5;
    // State S5 ???
    trans[ST_S5][TK_c]=ST_S6; trans[ST_S5][TK_d]=ST_S7;
    trans[ST_S5][TK_u]=ST_Sk; trans[ST_S5][TK_num]=ST_Sk; 
    // State S6 ???
    trans[ST_S6][TK_d]=ST_S7; trans[ST_S6][TK_u]=ST_Sk; trans[ST_S6][TK_num]=ST_Sk;  
    // State S7 ???
    trans[ST_S7][TK_wy]=ST_S8;
    // State S8 ???
    trans[ST_S8][TK_u]=ST_Sk; trans[ST_S8][TK_num]=ST_Sk; 
    // State Sk
    // nothing else is expected
    // State COD
    // nothing else is expected
    // State X1
    trans[ST_X1][TK_wy]=ST_X2;
    // State X2
    trans[ST_X2][TK_wmedio]=ST_X3;  trans[ST_X2][TK_wcuarto]=ST_X3;
    // State X3
    // nothing else is expected
    // State X4
    trans[ST_X4][TK_bill]=ST_M1;  trans[ST_X4][TK_mill]=ST_S1;   
    trans[ST_X4][TK_milr]=ST_X5;  trans[ST_X4][TK_cent]=ST_X5;
    trans[ST_X4][TK_dec]=ST_X5;   trans[ST_X4][TK_doc]=ST_X5;
    // State X5
    // nothing else is expected
 
    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int numbers_pt::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
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

  void numbers_pt::StateActions(int origin, int state, int token, sentence::const_iterator j, numbers_status *st) const {
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
      if (st->units==0) st->units=1; // not said: L"uno mil ciento treinta y dos"
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
    case ST_X1:
      TRACE(3,L"Actions for X1 state");
      // "Z millares/decenas/centenares/docenas"
      st->units *= power.find(token)->second;
      st->block = token;
      break;
      // ---------------------------------
    case ST_X3:
      TRACE(3,L"Actions for X3 state");
      // "Z millares/decenas/centenares/docenas y medio/cuarto"
      st->units += value.find(form)->second * power.find(st->block)->second;
      break;
      // ---------------------------------
    case ST_X4:
      TRACE(3,L"Actions for X4 state");
      // "medio" as 1st word
      st->units = value.find(form)->second;
      break;
      // ---------------------------------
    case ST_X5:
      TRACE(3,L"Actions for X5 state");
      // "medio millar/millon/centenar/docena/etc
      st->units *= power.find(token)->second;
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

  void numbers_pt::SetMultiwordAnalysis(sentence::iterator i, int fstate, const numbers_status *st) const {
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
