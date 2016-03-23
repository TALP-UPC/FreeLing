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

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/configfile.h"
#include "freeling/output/conll_handler.h"

using namespace std;
using namespace freeling;
using namespace freeling::io;

#define MOD_TRACECODE TRACE_OUTPUT
#define MOD_TRACENAME L"CONLL_HANDLER"


///---------------------------------------------
/// constructor: Create empty conll_sentence
///---------------------------------------------

conll_sentence::conll_sentence () {}

///---------------------------------------------
/// destructor
///---------------------------------------------

conll_sentence::~conll_sentence () {}

///---------------------------------------------
/// clear 
///---------------------------------------------

void conll_sentence::clear () {
  vector<vector<wstring> >::clear();
}


///----------------------------------------------------------
/// add a new row to the sentence
///---------------------------------------------------------

void conll_sentence::add_token (const vector<wstring> &token) {
  if (not this->empty() and token.size()!=(*this)[0].size()) 
    ERROR_CRASH(L"Lines with different length in CoNLL input sentence.");

  this->push_back(token);
}

///----------------------------------------------------------
/// get number of columns
///---------------------------------------------------------

size_t conll_sentence::get_n_columns () const {
  if (not this->empty()) return (*this)[0].size();
  else return 0;
}

///----------------------------------------------------------
/// get value for column col at word i
///---------------------------------------------------------

wstring conll_sentence::get_value(size_t i, size_t col) const {
  
  if (i >= this->size()) ERROR_CRASH(L"get_value. Requested word position out of sentence bounds.");
  if (col >= get_n_columns()) ERROR_CRASH(L"get_value. Requested column out of range.");

  return (*this)[i][col];
}


///----------------------------------------------------------
/// set value for column col at word i
///---------------------------------------------------------

void conll_sentence::set_value(size_t i, size_t col, const wstring &val) {
  
  if (i >= this->size()) ERROR_CRASH(L"set_value. Requested word position out of sentence bounds.");
  if (col >= get_n_columns()) ERROR_CRASH(L"set_value. Requested column is not available.");

  (*this)[i][col] = val;
}


///----------------------------------------------------------
/// actually output the conll format 
///---------------------------------------------------------

void conll_sentence::print_conll_sentence(wostream &sout) const {

  if (this->empty()) return;

  // compute required width for pretty printing each column
  vector<size_t> colsize((*this)[0].size(),0);
  for (size_t i=0; i<this->size(); i++) {
    for (size_t j=0; j<(*this)[i].size(); j++) 
      colsize[j] = max(colsize[j],(*this)[i][j].size());
  }

  // actually print sentence
  for (size_t i=0; i<this->size(); i++) {

    // print existing field with appropriate space filling
    size_t j;
    for (j=0; j<(*this)[i].size()-1; j++) 
      sout << (*this)[i][j] << wstring(colsize[j]-(*this)[i][j].size()+1, L' ');

    // last field requires no whitespace filling
    sout << (*this)[i][j] << endl;
  }
  
  sout<<endl;  // sentence delimiter
}

///////////////////////////////////////////////////////////
///
/// Class conll_handler:  Generic class for conll_input and
///                       conll_output handlers
///
///////////////////////////////////////////////////////////


const wstring conll_handler::UserPrefix = L"USER";
const map<wstring,conll_handler::ConllColumns> conll_handler::ValidFields = { {L"ID",ID}, 
                                                              {L"SPAN_BEGIN",SPAN_BEGIN}, 
                                                              {L"SPAN_END",SPAN_END},
                                                              {L"FORM",FORM},
                                                              {L"LEMMA",LEMMA},
                                                              {L"TAG",TAG},
                                                              {L"SHORT_TAG",SHORT_TAG},
                                                              {L"MSD",MSD},
                                                              {L"NEC",NEC},
                                                              {L"SENSE",SENSE},
                                                              {L"ALL_SENSES",ALL_SENSES},
                                                              {L"SYNTAX",SYNTAX},
                                                              {L"DEPHEAD",DEPHEAD},
                                                              {L"DEPREL",DEPREL},
                                                              {L"COREF",COREF},
                                                              {L"SRL",SRL} };

///----------------------------------------------------------
/// constructor (with default fields)
///---------------------------------------------------------

conll_handler::conll_handler() {
  init_default();
}

///----------------------------------------------------------
/// constructor (from config file)
///---------------------------------------------------------

conll_handler::conll_handler(const wstring &cfgFile) {
  
  enum sections {COLUMNS};
  
  config_file cfg(true);
  cfg.add_section(L"Columns",COLUMNS);
  
  if (not cfg.open(cfgFile))
    ERROR_CRASH(L"Error opening file "+cfgFile);
  
  wstring line; 
  while (cfg.get_content_line(line)) {
    
    // process each content line according to the section where it is found
    switch (cfg.get_section()) {
      
    case COLUMNS: {
      wistringstream sin; sin.str(line);
      wstring field;
      while (sin >> field) {
        // field name is in valid list, or it is 'UserPrefix' plus a number.
        if (ValidFields.find(field)!=ValidFields.end() or 
            (field.find(UserPrefix)==0 and util::RE_all_digits.match(field.substr(UserPrefix.size())))) {
          FieldName.push_back(field);
        }
        else {
          WARNING(L"Ignored invalid column name '"<<field<<L"' in config file "<<cfgFile);
        }
      }
      break;
    }

    default: break;
    }
  }
  
  cfg.close(); 

  if (not FieldName.empty()) {
    // if non-default columns specified, fill inverse index fieldName->position
    for (size_t i=0; i<FieldName.size(); ++i) 
      FieldPos.insert(make_pair(FieldName[i],i));    
  }
  else 
    // if no columns specified, use default
    init_default();
}

///----------------------------------------------------------
/// destructor
///----------------------------------------------------------

conll_handler::~conll_handler() {}
     

///----------------------------------------------------------
/// Constructor auxiliary: set default values
///----------------------------------------------------------

void conll_handler::init_default() {

  FieldName = {L"ID", L"FORM", L"LEMMA", L"TAG", L"SHORT_TAG", L"MSD", L"NEC",
               L"SENSE", L"SYNTAX", L"DEPHEAD", L"DEPREL", L"COREF", L"SRL"};

  for (size_t i=0; i<FieldName.size(); ++i) FieldPos.insert(make_pair(FieldName[i],i));
}

///----------------------------------------------------------
/// Obtain enum type from field name
///----------------------------------------------------------

conll_handler::ConllColumns conll_handler::field_code(const wstring &field) {

  map<wstring,ConllColumns>::const_iterator p = ValidFields.find(field);
  if (p==ValidFields.end()) return NO_FIELD;
  else return p->second;
}
     
