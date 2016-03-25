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

#ifndef _OUTPUT_XML
#define _OUTPUT_XML

#include <iostream> 
#include "freeling/output/output_handler.h"

namespace freeling {

  namespace io {
    class WINDLL output_xml : public output_handler {

    public:   
      // constructor. 
      output_xml ();
      output_xml (const std::wstring &cfgFile);
      // destructor. 
      ~output_xml ();

      // print XML file header
      void PrintHeader(std::wostream &sout) const; 
      // print XML file footer
      void PrintFooter(std::wostream &sout) const; 
      // print given sentences to sout in appropriate format
      void PrintResults (std::wostream &sout, const std::list<freeling::sentence> &ls) const;
      // print given a document to sout in appropriate format
      void PrintResults(std::wostream &sout, const freeling::document &doc) const;
      /// inherit other methods
      using output_handler::PrintResults;

    private:
      bool AllSenses;
      bool AllAnalysis;

      void print_analysis(std::wostream &sout, const freeling::analysis &a, bool print_sel_status=false, bool print_probs=false) const;
      void PrintTreeXML (std::wostream &sout, const std::wstring &sid, freeling::parse_tree::const_iterator n, int depth) const;
      void PrintDepTreeXML (std::wostream &sout, const std::wstring &sid, freeling::dep_tree::const_iterator n, int depth) const;
      void PrintPredArgsXML(std::wostream &sout, const freeling::sentence &s) const;
      void PrintCorefs(std::wostream &sout, const freeling::document &doc) const;
      void PrintSemgraph(std::wostream &sout, const freeling::document &doc) const;
    };

  }

}

#endif
