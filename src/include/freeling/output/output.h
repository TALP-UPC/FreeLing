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

#ifndef _OUTPUT_FACT
#define _OUTPUT_FACT

#include "freeling/windll.h"
#include "freeling/output/output_handler.h"

namespace freeling {

  namespace io {

    ////////////////////////////////////////////////////////////////
    ///   Class output implements a wrapper to transparently 
    ///   create and access an output_handler
    ////////////////////////////////////////////////////////////////

    class WINDLL output  {
  
    private:
      output_handler *who;
  
    public:
      /// Constructor
      output(const std::wstring &);
      /// Destructor
      ~output();
  
      /// initialize underlying handler tagset
      void load_tagset(const std::wstring &ftag);
      /// initialize underlying handler language
      void set_language(const std::wstring &lg);

      // Print appropriate header for the output format (e.g. XML header or tag opening)
      void PrintHeader(std::wostream &sout) const;
      // print appropriate footer (e.g. close XML tags)
      void PrintFooter(std::wostream &sout) const; 
      /// print given sentences to sout in appropriate format (no headers)
      void PrintResults (std::wostream &sout, const std::list<freeling::sentence> &ls) const;
      std::wstring PrintResults (const std::list<freeling::sentence> &ls) const;
  
      /// print given document to sout in appropriate format, including headers.
      void PrintResults (std::wostream &sout, const freeling::document &doc);
      std::wstring PrintResults (const freeling::document &doc) const;
    };
  }
}

#endif

