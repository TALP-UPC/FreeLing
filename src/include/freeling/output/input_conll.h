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

#ifndef _INPUT_CONLL
#define _INPUT_CONLL

#include <iostream> 
#include "freeling/output/input_handler.h"
#include "freeling/output/conll_handler.h"

namespace freeling {

  namespace io {
    class WINDLL input_conll : public input_handler, public conll_handler {

    public:   
      // default constructor. 
      input_conll();
      // constructor from config file
      input_conll(const std::wstring &fcfg);
      // destructor. 
      ~input_conll();

      void input_sentences(const std::wstring &lines, std::list<freeling::sentence> &ls) const;
      void input_document(const std::wstring &lines, freeling::document &doc) const;

    private:
      // whether to infer TAG from MSD column or not.
      bool use_msd;

      void conll2freeling(const conll_sentence &cs, freeling::sentence &s) const;
      void conll2freeling(const conll_sentence &cs, freeling::sentence &s, freeling::document &doc, int &nment) const;

      void load_corefs(const conll_sentence &cs,  
                       std::list<freeling::sentence>::const_iterator s,
                       int &nment, 
                       freeling::document &doc) const;

      void add_field_content(const std::wstring &field, 
                             const std::wstring &val, 
                             int i,
                             freeling::word &w, 
                             freeling::parse_tree::iterator &ptr, 
                             std::vector<freeling::dep_tree*> &dtrees, 
                             std::vector<std::wstring> &drels) const;
    };
  }
}

#endif
