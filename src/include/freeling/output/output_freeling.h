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

#ifndef _OUTPUT_FREELING
#define _OUTPUT_FREELING

#include <iostream> 
#include "freeling/output/output_handler.h"

namespace freeling {

  namespace io {

    class WINDLL output_freeling : public output_handler {

    private:
      bool OutputSenses;
      bool AllSenses;
      bool OutputPhonetics;
      bool OutputDepTree;
      bool OutputCorefs;
      bool OutputSemgraph;

      std::wstring outputSenses (const freeling::analysis & a) const;
      std::list<freeling::analysis> printRetokenizable(std::wostream &sout, const std::list<freeling::word> &rtk, 
                                                       std::list<freeling::word>::const_iterator w, 
                                                       const std::wstring &lem, 
                                                       const std::wstring &tag) const;
    public:   
      // constructor. 
      output_freeling ();
      output_freeling(const std::wstring &cfgFile);
      ~output_freeling ();

      void PrintTree (std::wostream &sout, freeling::parse_tree::const_iterator n, int depth) const;
      void PrintDepTree (std::wostream &sout, freeling::dep_tree::const_iterator n, int depth) const;
      void PrintPredArgs (std::wostream &sout, const freeling::sentence &s) const;
      void PrintWord (std::wostream &sout, const freeling::word &w, bool only_sel=true, bool probs=true) const;
      void PrintCorefs(std::wostream &sout, const freeling::document &doc) const;
      void PrintSemgraph(std::wostream &sout, const freeling::document &doc) const;

      // print given sentences to sout in appropriate format
      void PrintResults (std::wostream &sout, const std::list<freeling::sentence> &ls) const;
      // print given a document to sout in appropriate format
      void PrintResults(std::wostream &sout, const freeling::document &doc) const;
      /// inherit other methods
      using output_handler::PrintResults;

      // activate/deactivate printing levels
      void output_senses(bool);
      void output_all_senses(bool);
      void output_phonetics(bool);
      void output_dep_tree(bool);
      void output_corefs(bool);  
      void output_semgraph(bool);
    };
  }
}
#endif
