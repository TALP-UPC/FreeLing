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
  //        Italian number recognizer
  //---------------------------------------------------------------------------

  //// State codes 
#define ST_B1 1  // initial state
#define ST_TUH1 1               //recognizes numbers until 999
#define ST_TUH2 2
#define ST_TUH3 3
#define ST_TUH4 4
#define ST_TUH5 5
#define ST_TUH6 6
#define ST_TUH7 7
#define ST_TUHTMlrd1 8  //recognizes "miliards" until 999 miliardi
#define ST_TUHTMlrd2 9
#define ST_TUHTMlrd3 10
#define ST_TUHTMlrd4 11
#define ST_TUHTMlrd5 12
#define ST_TUHTMlrd6 13
#define ST_TUHTMlrd7 14
#define ST_TUHTM1 15    //recognizes milions untul 999 milions
#define ST_TUHTM2 16
#define ST_TUHTM3 17
#define ST_TUHTM4 18
#define ST_TUHTM5 19
#define ST_TUHTM6 20
#define ST_TUHTM7 21
#define ST_TUHT1 22     //recognizes thousands until 999 thousands
#define ST_TUHT2 23
#define ST_TUHT3 24
#define ST_TUHT4 25
#define ST_TUHT5 26
#define ST_TUHT6 27
#define ST_TUHT7 28
#define ST_COD 49 // got pseudo-numerical code from initial state
#define ST_special 50

  // stop state
#define ST_STOP 51

  // Token codes
#define TK_c      1  // hundreds "cento" "duecento" 
#define TK_d      2  // decines undici, dodici, tredici (eleven,twelf)
#define TK_t      3  // teens deci, venti, trenta (twent,thirty)
#define TK_u      4  // units "tre" "quatro"
#define TK_we     5  // word "e"
#define TK_whalf  6  // word "medio"
#define TK_wquarter 7 // word "quarto"
#define TK_mil    8  // word "mille"   
#define TK_mill   9  // word "milioni" "milione"
#define TK_bill   10  // word "miliardo" "miliardi"
#define TK_num   11  // a number (in digits)
#define TK_milr  12  // word "miliardo/es"
#define TK_special 13   //this is the special token. We return a very special state when a number is well defined and there can not be appended anything on the tail of that number

#define TK_TUH 29
#define TK_TUHTMlrd 30 
#define TK_TUHTM 31
#define TK_TUHT 32

#define TK_code  15  // a code (ex. LX-345-2)

#define TK_other 16
 
 
  ///////////////////////////////////////////////////////////////
  ///  Create a numbers recognizer for Italian.
  ///////////////////////////////////////////////////////////////

  numbers_it::numbers_it(const std::wstring &dec, const std::wstring &thou): numbers_module(dec,thou)
  {  
    // Initializing value map
    value.insert(make_pair(L"uno",1.0f));     value.insert(make_pair(L"un",1.0f)); value.insert(make_pair(L"una",1.0f));
    value.insert(make_pair(L"due",2.0f));
    value.insert(make_pair(L"tre",3.0f));
    value.insert(make_pair(L"tré",3.0f));

    value.insert(make_pair(L"quattro",4.0f));
    value.insert(make_pair(L"quattr",4.0f));
    value.insert(make_pair(L"quatro",4.0f));

    value.insert(make_pair(L"cinque",5.0f));   
    value.insert(make_pair(L"sei",6.0f));
    value.insert(make_pair(L"sette",7.0f));   
    value.insert(make_pair(L"otto",8.0f));
    value.insert(make_pair(L"nove",9.0f));   
    value.insert(make_pair(L"dieci",10.0f));
    value.insert(make_pair(L"undici",11.0f));   
    value.insert(make_pair(L"dodici",12.0f));
    value.insert(make_pair(L"tredici",13.0f));  
    value.insert(make_pair(L"quattordici",14.0f));
    value.insert(make_pair(L"quindici",15.0f)); 
    value.insert(make_pair(L"sedici",16.0f));
    value.insert(make_pair(L"diciassette",17.0f)); 
    value.insert(make_pair(L"diciotto",18.0f));
    value.insert(make_pair(L"diciott",18.0f));  
    value.insert(make_pair(L"diciannove",19.0f));
    value.insert(make_pair(L"venti", 20.0f));
    value.insert(make_pair(L"trenta",30.0f));
    value.insert(make_pair(L"trent",30.0f)); 
         
    value.insert(make_pair(L"quaranta",40.0f));
    value.insert(make_pair(L"quarant",40.0f));
    value.insert(make_pair(L"cinquanta",50.0f));
    value.insert(make_pair(L"cinquant",50.0f));      
    value.insert(make_pair(L"sessanta",60.0f));
    value.insert(make_pair(L"sessant",60.0f));
    value.insert(make_pair(L"settanta",70.0f));
    value.insert(make_pair(L"settant",70.0f));
    value.insert(make_pair(L"ottanta",80.0f));
    value.insert(make_pair(L"ottant",80.0f));
    value.insert(make_pair(L"novanta",90.0f));
    value.insert(make_pair(L"novant",90.0f));

    value.insert(make_pair(L"cento",100.0f));

    value.insert(make_pair(L"mille",1000.0f));
    value.insert(make_pair(L"mila",1000.0f));
  
    value.insert(make_pair(L"milioni",1000000.0f));

    /*value.insert(make_pair("meta",0.5));    value.insert(make_pair("mezzo",0.5));  
      value.insert(make_pair("quarto",0.25));  value.insert(make_pair("quarti",0.25));*/ //still not ready to use. Handling for floating numbers.Processing tokens like Half, et al. 


    // Initializing token map

    tok.insert(make_pair(L"cento",TK_c));

    tok.insert(make_pair(L"dieci",TK_t));     tok.insert(make_pair(L"undici",TK_d));
    tok.insert(make_pair(L"dodici",TK_d));     tok.insert(make_pair(L"tredici",TK_d));
    tok.insert(make_pair(L"quattordici",TK_d));  tok.insert(make_pair(L"quindici",TK_d));
    tok.insert(make_pair(L"sedici",TK_d));  tok.insert(make_pair(L"diciassette",TK_d));
    tok.insert(make_pair(L"diciotto",TK_d));tok.insert(make_pair(L"diciott",TK_d));  
    tok.insert(make_pair(L"diciannove",TK_d));
    tok.insert(make_pair(L"venti",TK_t)); tok.insert(make_pair(L"vent",TK_t));
    tok.insert(make_pair(L"trenta",TK_t));tok.insert(make_pair(L"trent",TK_t));
    tok.insert(make_pair(L"quaranta",TK_t));   tok.insert(make_pair(L"quarant",TK_t)); 
    tok.insert(make_pair(L"cinquanta",TK_t)); tok.insert(make_pair(L"cinquant",TK_t));
    tok.insert(make_pair(L"sessanta",TK_t));  tok.insert(make_pair(L"sessanti",TK_t));tok.insert(make_pair(L"sessant",TK_t));
    tok.insert(make_pair(L"settanta",TK_t));tok.insert(make_pair(L"settant",TK_t));
    tok.insert(make_pair(L"ottanta",TK_t));tok.insert(make_pair(L"ottant",TK_t));
    tok.insert(make_pair(L"novanta",TK_t));tok.insert(make_pair(L"novant",TK_t));


    tok.insert(make_pair(L"uno",TK_u));tok.insert(make_pair(L"un",TK_u));tok.insert(make_pair(L"una",TK_u));
    tok.insert(make_pair(L"due",TK_u));      tok.insert(make_pair(L"tre",TK_u));
    tok.insert(make_pair(L"tré",3));
    tok.insert(make_pair(L"quattro",TK_u));tok.insert(make_pair(L"quattr",TK_u)); tok.insert(make_pair(L"quatro",TK_u));
    tok.insert(make_pair(L"cinque",TK_u));
    tok.insert(make_pair(L"sei",TK_u));   tok.insert(make_pair(L"sette",TK_u));
    tok.insert(make_pair(L"otto",TK_u));     tok.insert(make_pair(L"nove",TK_u));
  
    tok.insert(make_pair(L"e",TK_we));

    tok.insert(make_pair(L"mila",TK_mil));tok.insert(make_pair(L"mille",TK_mil)); //a rule in italian. mille for 1 mille and mila for 2,3,4 (plural) mila

    tok.insert(make_pair(L"milioni",TK_mill));tok.insert(make_pair(L"milione",TK_mill));tok.insert(make_pair(L"millioni",TK_mill));tok.insert(make_pair(L"millione",TK_mill));
    tok.insert(make_pair(L"bilione",TK_bill)); //this form is used rarely in italian language
    tok.insert(make_pair(L"miliardi",TK_milr));    tok.insert(make_pair(L"miliardo",TK_milr));

    tok.insert(make_pair(L"mezzo",TK_whalf)); tok.insert(make_pair(L"meta",TK_whalf));
    tok.insert(make_pair(L"quarto",TK_wquarter));tok.insert(make_pair(L"quarti",TK_wquarter));

    // Initializing power map
    power.insert(make_pair(TK_mil,  1000.0));
    power.insert(make_pair(TK_c,  100.0));
    power.insert(make_pair(TK_mill, 1000000.0));
    power.insert(make_pair(TK_bill, 1000000000000.0));
    power.insert(make_pair(TK_milr, 1000000000.0));

    Final.insert(ST_TUH1);  Final.insert(ST_TUH2);  Final.insert(ST_TUH3);

    //I have to redu this table, since it is not working propertly. 
    // Initialize special state attributes
    initialState=ST_B1; stopState=ST_STOP;

    Final.insert(ST_special);
    Final.insert(ST_COD);

    // Initialize transitions table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;
    //state B1 handle sepcial state from automaton. This is the right
    //place to do it. Inside our ComputeToken function we already
    //compute the value of the token, since in italian, alike in german,
    //numbers are compounded (embedded), one word, one number.

    trans[ST_B1][TK_special]=ST_special;
    trans[ST_B1][TK_num]=ST_COD;
    trans[ST_B1][TK_code]=ST_COD;
    /*
      #########################################################################################################
      ##################################### STATE TUH (tens,units, hundreds recognizer) ######################
      #########################################################################################################
    */  
    // STATE TU1
    trans[ST_TUH1][TK_u]=ST_TUH4;   trans[ST_TUH1][TK_t]=ST_TUH2;
    trans[ST_TUH1][TK_d]=ST_TUH3;   trans[ST_TUH1][TK_c]=ST_TUH5;

    //state TUH2
    trans[ST_TUH2][TK_u]=ST_TUH3;

    //state TUH4
    trans[ST_TUH4][TK_c]=ST_TUH5;
  
    //state TUH5
    trans[ST_TUH5][TK_u]=ST_TUH7;  trans[ST_TUH5][TK_d]=ST_TUH7;  trans[ST_TUH5][TK_t]=ST_TUH6;

    //state TUH7
    trans[ST_TUH6][TK_u]=ST_TUH7;
        
    //arcs that goes to TUHTMlrd automaton. That is billion recognizer
    trans[ST_TUH1][TK_milr]=ST_TUHTMlrd1;  trans[ST_TUH2][TK_milr]=ST_TUHTMlrd1;
    trans[ST_TUH3][TK_milr]=ST_TUHTMlrd1;  trans[ST_TUH4][TK_milr]=ST_TUHTMlrd1; 
    trans[ST_TUH5][TK_milr]=ST_TUHTMlrd1;  trans[ST_TUH6][TK_milr]=ST_TUHTMlrd1;
    trans[ST_TUH7][TK_milr]=ST_TUHTMlrd1;


    //arcs that goes to TUHTM automaton. That is milion recognizer numbers
    trans[ST_TUH1][TK_mill]=ST_TUHTM1;  trans[ST_TUH2][TK_mill]=ST_TUHTM1;
    trans[ST_TUH3][TK_mill]=ST_TUHTM1;  trans[ST_TUH4][TK_mill]=ST_TUHTM1;
    trans[ST_TUH5][TK_mill]=ST_TUHTM1;  trans[ST_TUH6][TK_mill]=ST_TUHTM1;
    trans[ST_TUH7][TK_mill]=ST_TUHTM1;

    //arcs that goes to TUHT automaton. That is thousands numbers
    trans[ST_TUH1][TK_mil]=ST_TUHT1;  trans[ST_TUH2][TK_mil]=ST_TUHT1;
    trans[ST_TUH3][TK_mil]=ST_TUHT1;  trans[ST_TUH4][TK_mil]=ST_TUHT1;
    trans[ST_TUH5][TK_mil]=ST_TUHT1;  trans[ST_TUH6][TK_mil]=ST_TUHT1;
    trans[ST_TUH7][TK_mil]=ST_TUHT1;

    //STATE TUHTMlrd = TUHT=TUHTM. We have the same rules after the words milion. We can use the same automaton for recognizing thousands
    // STATE TUHTMlrd1
    trans[ST_TUHTMlrd1][TK_u]=ST_TUHTMlrd4;  trans[ST_TUHTMlrd1][TK_t]=ST_TUHTMlrd2;
    trans[ST_TUHTMlrd1][TK_d]=ST_TUHTMlrd3;  trans[ST_TUHTMlrd1][TK_c]=ST_TUHTMlrd5;

    //state TUHTMlrd2
    trans[ST_TUHTMlrd2][TK_u]=ST_TUHTMlrd3;

    //state TUHTMlrd4
    trans[ST_TUHTMlrd4][TK_c]=ST_TUHTMlrd5;
  
    //state TUHTMlrd5
    trans[ST_TUHTMlrd5][TK_u]=ST_TUHTMlrd7;  trans[ST_TUHTMlrd5][TK_d]=ST_TUHTMlrd7;
    trans[ST_TUHTMlrd5][TK_t]=ST_TUHTMlrd6;

    //state TUHTMlrd6
    trans[ST_TUHTMlrd6][TK_u]=ST_TUHTMlrd7;

    //arcs that goes to TUHT automaton. That is thousands numbers
    trans[ST_TUHTMlrd1][TK_mil]=ST_TUHT1;  trans[ST_TUHTMlrd2][TK_mil]=ST_TUHT1;
    trans[ST_TUHTMlrd3][TK_mil]=ST_TUHT1;  trans[ST_TUHTMlrd4][TK_mil]=ST_TUHT1;
    trans[ST_TUHTMlrd5][TK_mil]=ST_TUHT1;  trans[ST_TUHTMlrd6][TK_mil]=ST_TUHT1;
    trans[ST_TUHTMlrd7][TK_mil]=ST_TUHT1;

    //arcs that goes to TUHTM automaton. That is milion recognizer numbers
    trans[ST_TUHTMlrd1][TK_mill]=ST_TUHTM1;  trans[ST_TUHTMlrd2][TK_mill]=ST_TUHTM1;
    trans[ST_TUHTMlrd3][TK_mill]=ST_TUHTM1;  trans[ST_TUHTMlrd4][TK_mill]=ST_TUHTM1;
    trans[ST_TUHTMlrd5][TK_mill]=ST_TUHTM1;  trans[ST_TUHTMlrd6][TK_mill]=ST_TUHTM1;
    trans[ST_TUHTMlrd7][TK_mill]=ST_TUHTM1;

    //STATE TUHTM1 = TUHT1. We have the same rules after the words milion. We can use the same automaton for recognizing thousands
    // STATE TUHTM1
    trans[ST_TUHTM1][TK_u]=ST_TUHTM4;  trans[ST_TUHTM1][TK_t]=ST_TUHTM2;
    trans[ST_TUHTM1][TK_d]=ST_TUHTM3;  trans[ST_TUHTM1][TK_c]=ST_TUHTM5;

    //state TUHTM2
    trans[ST_TUHTM2][TK_u]=ST_TUHTM3;

    //state TUHTM4
    trans[ST_TUHTM4][TK_c]=ST_TUHTM5;
  
    //state TUHTM5
    trans[ST_TUHTM5][TK_u]=ST_TUHTM7;  trans[ST_TUHTM5][TK_d]=ST_TUHTM7;  trans[ST_TUHTM5][TK_t]=ST_TUHTM6;

    //state TUHTM6
    trans[ST_TUHTM6][TK_u]=ST_TUHTM7;

    //arcs that goes to TUHT automaton. That is thousands numbers
    trans[ST_TUHTM1][TK_mil]=ST_TUHT1;  trans[ST_TUHTM2][TK_mil]=ST_TUHT1;
    trans[ST_TUHTM3][TK_mil]=ST_TUHT1;  trans[ST_TUHTM4][TK_mil]=ST_TUHT1;
    trans[ST_TUHTM5][TK_mil]=ST_TUHT1;  trans[ST_TUHTM6][TK_mil]=ST_TUHT1;
    trans[ST_TUHTM7][TK_mil]=ST_TUHT1;

    // STATE TUHT1
    trans[ST_TUHT1][TK_u]=ST_TUHT4;  trans[ST_TUHT1][TK_t]=ST_TUHT2;
    trans[ST_TUHT1][TK_d]=ST_TUHT3;  trans[ST_TUHT1][TK_c]=ST_TUHT5;

    //state TUHT2
    trans[ST_TUHT2][TK_u]=ST_TUHT3;

    //state TUHT4
    trans[ST_TUHT4][TK_c]=ST_TUHT5;
  
    //state TUHT5
    trans[ST_TUHT5][TK_u]=ST_TUHT7;  trans[ST_TUHT5][TK_d]=ST_TUHT7;  trans[ST_TUHT5][TK_t]=ST_TUHT6;

    //state TUHT6
    trans[ST_TUHT6][TK_u]=ST_TUHT7;
    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int numbers_it::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
    // in case ST_we have recognized a previous numbers, we do not want to
    // recognize the second number as being a part of it. For multiwords
    // processing, please work in this part
    if (state==ST_special) return TK_other;

    numbers_status *st = (numbers_status*) se.get_processing_status();

    bool didIRecognize=false;   // we have to know inside this funtion 
    // whether we have identified the number or not. 
    int token;
    map<wstring,int>::const_iterator im;
    map<wstring,int>::const_iterator prefixIm;
    map<wstring,int>::const_iterator suffixIm;
  
    int newstate=ST_STOP;
    wstring fullForm = j->get_lc_form();

    // when we search for a token, we will not find it since italian
    // numbers are compounded (multiwords). i.e. ("duecentotrentasete"="two hundred thirty-seven")
    // Therefore, a search by substrings is done .
    token = TK_other;
  
    if (fullForm.length()>4) {
      list<wstring> detectedNumbers;
      list<int> detectedNumbersCodes;
      list<std::wstring>::iterator dn;
      list<int>::iterator dnt;
      int numbersDetected=0;
      int fullLength=fullForm.length();
      int insertedNrsLength=0;
      int fromOffs=fullForm.length()-2;
      int toOffs=2;
      int tmpToken=TK_other;
    
      while(fromOffs>=0) {        //we assure the exit from this cycle when we fromOffs is zero
      
        std::wstring partForm=fullForm.substr(fromOffs,toOffs);
        im = tok.find(partForm);
      
        prefixIm=tok.find(fullForm.substr(0,fromOffs+toOffs));
      
        if (detectedNumbers.size()>0) {
          std::wstring suffix=fullForm.substr(fromOffs,toOffs)+detectedNumbers.front();
          suffixIm=tok.find(suffix);
          if(suffixIm!=tok.end()) {
            // Replace the last in the tail with the found suffix.
            // Suffix processing is very important, since in italian we
            // can have numbers like millediciotto. In this case this
            // processing function will identify otto as a token, and
            // milledici as another, and it will fail the
            // identification.  If we concatenate last inserted
            // identified number like otto with dici, we obtain
            // diciotto, and analyzer will remove the last inserted
            // number from the list and insert a new one concatenated
            if (detectedNumbers.size()>0) {
              partForm=suffix; //first of all, save the suffix
              im=suffixIm;     //save the suffix
            
              //update the length checker flag
              insertedNrsLength-=detectedNumbers.front().length();

              detectedNumbers.pop_front();     //remove the last token, since it's not the right one
              detectedNumbersCodes.pop_front();//remove the last code, since it's not the right one
              toOffs=1;                        //update the toOffset (superior limit of the offset)
            }
          }
        }
      
        if (prefixIm!=tok.end()) {        
          //this is only for optimization. We can have a full number
          // prefix.  for example, we can have diciotto mila, where
          // dicitotto is the prefix. One step and the job is done!!!
          // This part can be also removed, it will work.
          partForm=fullForm.substr(0,fromOffs+toOffs);
          im=prefixIm;
          fromOffs=-1;
          toOffs=1;
        }
                
        if (im!=tok.end()) {
          tmpToken = (*im).second;
          if (tmpToken!=TK_we) {
            toOffs=1;
            // insert in the list in inverse. We parse the string from tail to head. 
            numbersDetected++;
            insertedNrsLength+=partForm.length();
            detectedNumbers.push_front(partForm);
            detectedNumbersCodes.push_front(tmpToken);
          }
          else 
            toOffs++;
        }
        else
          toOffs++;
      
        fromOffs--; //At everycycle iterator is decreased. It assures the exit from loop
      }
    
      if ((numbersDetected>1)&&(insertedNrsLength==fullLength)) {
        //first check if it is really a full number. Can be a token
        //non-numeric which contains numeric tokens such as Trento.

        didIRecognize=true;
        std::wstring saveForm=j->get_form();
        sentence::iterator jReplacer=j;
        dnt=detectedNumbersCodes.begin();
        state=ST_B1;
      
        for (dn=detectedNumbers.begin();dn!=detectedNumbers.end()&&state!=ST_STOP;dn++) {
          
          token=*dnt;
        
          jReplacer->set_form(*dn);
          // here is the trick. We will move the states in the automaton
          // inside this function, then pass the last state to
          // automaton::annotate function
          newstate = trans[state][token];

          // let the child class perform any actions 
          // for the new state (e.g. computing date value...)
          StateActions(state, newstate, token, jReplacer, st);

          // change state
          state = newstate;
        
          if(dnt!=detectedNumbersCodes.end()) dnt++;      
        }

        if (dn!=detectedNumbers.end()&&state==ST_STOP) {
          st->iscode=true;
          didIRecognize=false;
        }
      
        j->set_form(saveForm);
      }
      else {
        im = tok.find(fullForm);
        if (im!=tok.end()) {
          token = (*im).second;
          newstate = trans[state][token];
          StateActions(state, newstate, token, j, st);
          didIRecognize=true;
        }
      }    
    }
    else {        
      im = tok.find(fullForm);
    
      if (im!=tok.end()) {        
        token = (*im).second;
        newstate = trans[state][token];
        StateActions(state, newstate, token, j, st);
        didIRecognize=true;
      }
    }
  
    TRACE(3,L"Next word form is: ["+fullForm+L"] token="+util::int2wstring(token));
  
    if(didIRecognize && newstate!=ST_STOP) token=TK_special;  
    else token=TK_other;
  
    // if the token was successfully decomposed and identified as a number, we're done
    if (token != TK_other) return (token);
  
    // Token not identified as number, let's have a closer look.
    // check to see if it is a number
    if (RE_number.search(fullForm)) token = TK_num;
    else if (RE_code.search(fullForm)) token = TK_code;
  
    TRACE(3,L"Leaving state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)); 
  
    return (token); //here we return the type of number we have 
  }


  ///////////////////////////////////////////////////////////////
  ///   Reset acumulators used by state actions:
  ///   bilion, milion, units.
  ///////////////////////////////////////////////////////////////

  void numbers_it::ResetActions(numbers_status *st) const {
    // default actions
    numbers_module::ResetActions(st); 

    // language specific actions
    st->floatUnits=0; //used for floating point values
    st->hundreds=0;   //hundreds keeper
    st->thousands=0;  //thousands keeper
  }

  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state, update current 
  ///  nummerical value.
  ///////////////////////////////////////////////////////////////

  void numbers_it::StateActions(int origin, int state, int token, sentence::const_iterator j, numbers_status *st) const {
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
    case ST_TUH1: case ST_TUH2:  case ST_TUH3: case ST_TUH4:  
    case ST_TUH6: case ST_TUH7: case ST_TUHT2:  case ST_TUHT3: 
    case ST_TUHT4: case ST_TUHT6: case ST_TUHT7: case ST_TUHTM2:  
    case ST_TUHTM3: case ST_TUHTM4: case ST_TUHTM6: case ST_TUHTM7: 
    case ST_TUHTMlrd2:  case ST_TUHTMlrd3: case ST_TUHTMlrd4: 
    case ST_TUHTMlrd6: case ST_TUHTMlrd7:
      //This is the addition operation
      st->units += value.find(form)->second;
      break;

    case ST_TUH5: case ST_TUHT1: case ST_TUHT5: 
    case ST_TUHTM1: case ST_TUHTM5: case ST_TUHTMlrd1: 
    case ST_TUHTMlrd5:

      switch(token) {
      case TK_c:
        if (st->units==0) st->units = 1;
        st->hundreds = st->units * power.find(token)->second;
        st->units = 0;
        break;

      case TK_mil: 
        if (st->units==0 && st->hundreds==0) st->units = 1;   //just in case we have the word "mille"s
        st->thousands = st->units * power.find(token)->second;
        st->thousands += st->hundreds * power.find(token)->second;          
        st->units = 0;          //release all the numbers after you have computed the new value
        st->hundreds = 0;
        break;

      case TK_mill:               //the same rules for milions and thousands.
        if (st->units==0 && st->hundreds==0) st->units = 1;  //just in case we have the word "mille"s
        st->milion = st->units * power.find(token)->second;
        st->milion += st->hundreds * power.find(token)->second;
        st->units = 0;          //release all the numbers after you have computed the new value
        st->hundreds = 0;
        break;

      case TK_milr:               //the same rules for milions and thousands.
        if (st->units==0 && st->hundreds==0) st->units = 1;      //just in case we have the word "mille"s
        st->bilion = st->units * power.find(token)->second;
        st->bilion += st->hundreds * power.find(token)->second;
          
        st->units = 0;          //release all the numbers after you have computed the new value
        st->hundreds = 0;
        break;    
      }
      break;

    case ST_COD:
      TRACE(3,L"Actions for COD state");
      if (token==TK_num)
        st->units += num;
      else
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

  void numbers_it::SetMultiwordAnalysis(sentence::iterator i, int fstate, const numbers_status *st) const {
    wstring lemma;

    // Setting the analysis for the nummerical expression
    if (st->iscode==CODE) {
      lemma=i->get_form();
    }
    else {
      // compute nummerical value as lemma
      lemma=util::longdouble2wstring(st->bilion + st->milion + st->thousands + st->hundreds + st->units);
    }

    if(fstate==ST_special) {
      i->set_analysis(analysis(lemma,L"Zd"));     //we have successfully recognized the word as number
      TRACE(3,L"Analysis set to: "+lemma+L" Zd");
    }
    else {
      i->set_analysis(analysis(lemma,L"Z"));
      TRACE(3,L"Analysis set to: "+lemma+L" Z");
    }
  }

} // namespace
