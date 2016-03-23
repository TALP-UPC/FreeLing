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

#ifndef _OUTPUT_CONLL
#define _OUTPUT_CONLL

#include <iostream> 
#include "freeling/output/output_handler.h"
#include "freeling/output/conll_handler.h"

namespace freeling {

  namespace io {

    class WINDLL output_conll : public output_handler, public conll_handler {

    public:   
      // empty constructor. 
      output_conll ();
      // constructor from cfg file
      output_conll (const std::wstring &cfgFile);
      // destructor. 
      ~output_conll ();

      /// print given sentences to sout in appropriate format
      void PrintResults (std::wostream &sout, const std::list<freeling::sentence> &ls) const;
      /// print given a document to sout in appropriate format
      void PrintResults(std::wostream &sout, const freeling::document &doc) const;
      /// inherit other methods
      using output_handler::PrintResults;

    private:
      std::wstring compute_value(const freeling::sentence &s,
                                 const freeling::word &w, 
                                 const std::wstring &field, 
                                 const std::vector<std::wstring> &openchunk, 
                                 const std::vector<std::wstring> &closechunk,
                                 const std::map<std::wstring,std::wstring> &openmention,
                                 const std::map<std::wstring,std::wstring> &closemention) const;

      static void add_srl(std::vector<std::wstring> &token, 
                          const freeling::sentence &s, int id);

      static void openclosechunks(freeling::parse_tree::const_iterator n, 
                                  std::vector<std::wstring> &open,
                                  std::vector<std::wstring> &close);

      static void openclosementions(const freeling::document &doc, 
                                    std::map<std::wstring,std::wstring> &open, 
                                    std::map<std::wstring,std::wstring> &close);

      static void add_mention_oc(std::map<std::wstring,std::wstring> &table, 
                                 const std::wstring &key, 
                                 const std::wstring &value);

      /// Fill conll_sentence from freeling::sentence
      void freeling2conll(const freeling::sentence &s, 
                          conll_sentence &cs,
                          const std::map<std::wstring,std::wstring> &openmention=std::map<std::wstring,std::wstring>(),
                          const std::map<std::wstring,std::wstring> &closemention=std::map<std::wstring,std::wstring>()) const;


    };
  }
}
#endif
