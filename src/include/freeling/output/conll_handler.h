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

#ifndef _CONLL_HANDLER
#define _CONLL_HANDLER

#include <iostream> 
#include <vector> 
#include "freeling/morfo/language.h"

namespace freeling {

  namespace io {

    class conll_sentence : public std::vector<std::vector<std::wstring> > {

    public:
      /// constructor
      conll_sentence();
    
      // destructor
      ~conll_sentence();

      void clear ();

      void print_conll_sentence(std::wostream &sout) const;

      void add_token (const std::vector<std::wstring> &token);
      size_t get_n_columns () const;
      std::wstring get_value(size_t i, size_t col) const;
      void set_value(size_t i, size_t col, const std::wstring &val);    

    };


    // Abstract class with common infrastructure for conll_input and conll_output

    class conll_handler {

    protected:
      typedef enum {ID, SPAN_BEGIN, SPAN_END, FORM, LEMMA, TAG, SHORT_TAG, MSD, 
                    NEC, SENSE, ALL_SENSES, SYNTAX, DEPHEAD, DEPREL, COREF, SRL,
                    NO_FIELD} ConllColumns;

      // given a position, find out which field is stored there
      std::vector<std::wstring> FieldName;
      // given a field name, find out in which position it is
      std::map<std::wstring,size_t> FieldPos;

      // constructor (with default fields)
      conll_handler();
      // constructor (from config file)
      conll_handler(const std::wstring &cfgFile);
      // destructor
      ~conll_handler();

      void init_default();
      static ConllColumns field_code(const std::wstring &field);     
     
      static const std::wstring UserPrefix;
      static const std::map<std::wstring,ConllColumns> ValidFields;
    };
  }
}
#endif
