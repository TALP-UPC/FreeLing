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

#ifndef _OUTPUT_NAF
#define _OUTPUT_NAF

#include <iostream> 
#include <set> 
#include "freeling/output/output_handler.h"

namespace freeling {

  namespace io {

    class WINDLL output_naf : public output_handler {

    private:
      // NAF layers to print
      std::set<std::wstring> layers;

      std::wstring naf_pos_tag(const std::wstring &tag) const;

      void PrintDepTreeNAF (std::wostream &sout, freeling::dep_tree::const_iterator n, const std::wstring &sid) const;

      void PrintTextLayer (std::wostream &sout, const freeling::document &doc) const;
      void PrintTermsLayer (std::wostream &sout, const freeling::document &doc) const;
      void PrintEntitiesLayer(std::wostream &sout, const freeling::document &doc) const;
      void PrintChunksLayer(std::wostream &sout, const freeling::document &doc) const;
      void PrintConstituencyLayer(std::wostream &sout, const freeling::document &doc) const;
      void PrintDepsLayer(std::wostream &sout, const freeling::document &doc) const;
      void PrintSRLLayer(std::wostream &sout, const freeling::document &doc) const;
      void PrintCoreferencesLayer(std::wostream &sout, const freeling::document &doc) const;

      static std::wstring get_term_id(const std::wstring &sid, const freeling::word &w, const std::wstring &pref=L"");
      static void print_tokens(std::wostream &sout, const freeling::word &w, bool wf, const std::wstring &nsent, int &ntok);
      static void print_span(std::wostream &sout, const freeling::sentence &s, int from, int to);
      void print_external_refs(std::wostream &sout, const freeling::word &w) const;

    public:   
      // constructor. 
      output_naf ();
      output_naf (const std::wstring &cfgFile);
      // destructor. 
      ~output_naf ();

      // print given sentences to sout in appropriate format
      void PrintResults (std::wostream &sout, const std::list<freeling::sentence> &ls) const;
      // print given a document to sout in appropriate format
      void PrintResults(std::wostream &sout, const freeling::document &doc) const;
      /// inherit other methods
      using output_handler::PrintResults;

      // print NAF header
      void PrintHeader(std::wostream &sout) const;
      // print NAF footer
      void PrintFooter(std::wostream &sout) const; 

      // activate/deactivate layer for next printings
      void ActivateLayer(const std::wstring &ly, bool b);

    };

  }
}

#endif
