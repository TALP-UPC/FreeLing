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

#ifndef _OUTPUT_JSON
#define _OUTPUT_JSON

#include <iostream> 
#include "freeling/output/output_handler.h"
#include "freeling/output/json.hpp"

namespace jsn = nlohmann;

namespace freeling {

  namespace io {

    class WINDLL output_json : public output_handler {

    public:   
      // constructor. 
      output_json ();
      output_json (const std::wstring &cfgFile);
      // destructor. 
      ~output_json ();

      // get json object for a sentence list
      jsn::ordered_json json_Sentences (const std::list<freeling::sentence> &ls) const;
      // get json object for a document
      jsn::ordered_json json_Document(const document &doc) const;
      // print given sentences to sout in appropriate format
      void PrintResults (std::wostream &sout, const std::list<freeling::sentence> &ls) const;
      // print given a document to sout in appropriate format
      void PrintResults(std::wostream &sout, const freeling::document &doc) const;
      /// inherit other methods
      using output_handler::PrintResults;

    private:
      bool AllSenses;
      bool AllAnalysis;

      jsn::ordered_json json_analysis(const freeling::analysis &a, bool print_sel_status, bool print_probs) const;
      jsn::ordered_json json_Tree (const std::wstring &sid,
				   freeling::parse_tree::const_iterator n) const;
      jsn::ordered_json json_DepTree (const std::wstring &sid,
				      freeling::dep_tree::const_iterator n) const;
      jsn::ordered_json json_PredArgs(const freeling::sentence &s) const;
      jsn::ordered_json json_Corefs(const freeling::document &doc) const;
      jsn::ordered_json json_Semgraph(const freeling::document &doc) const;
    };
  }
}

#endif
