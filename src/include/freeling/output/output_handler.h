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

#ifndef _OUTPUT_HANDLER
#define _OUTPUT_HANDLER

#include <iostream> 
#include "freeling/morfo/language.h"
#include "freeling/output/io_handler.h"

namespace freeling {

  namespace io {

    class WINDLL output_handler : public io_handler {

    public:   
      /// empty constructor
      output_handler();
      /// destructor
      ~output_handler();

      // Print appropriate header for the ourput format (e.g. XML header or tag opening)
      virtual void PrintHeader(std::wostream &sout) const;
      // print appropriate footer (e.g. close XML tags)
      virtual void PrintFooter(std::wostream &sout) const; 
      /// print given sentences to sout in appropriate format (no headers)
      virtual void PrintResults (std::wostream &sout, const std::list<freeling::sentence> &ls) const = 0;
      virtual std::wstring PrintResults (const std::list<freeling::sentence> &ls) const;

      /// print given document to sout in appropriate format, including headers.
      virtual void PrintResults (std::wostream &sout, const freeling::document &doc) const = 0;
      virtual std::wstring PrintResults (const freeling::document &doc) const;

    protected:
      /// Criteria to sort dependency children by word position 
      static bool ascending_position(const freeling::dep_tree::const_sibling_iterator &n1, 
                                     const freeling::dep_tree::const_sibling_iterator &n2);
      /// replace a XML reserved token with its right escape code
      static std::wstring escapeXML(const std::wstring &s);
      /// replace quotes and backslashes for JSON
      static std::wstring escapeJSON(const std::wstring &s);

      // auxiliary to compute term/token id 
      static std::wstring get_token_id(const std::wstring &sid, int tk, const std::wstring &pref=L"t");

      // compute retokenization combinations for a word
      static std::list<freeling::analysis> compute_retokenization(const std::list<freeling::word> &rtk, 
                                                                  std::list<freeling::word>::const_iterator w, 
                                                                  const std::wstring &lem, 
                                                                  const std::wstring &tag);
    };

  }
}

#endif
