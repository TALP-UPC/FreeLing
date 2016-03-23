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

#ifndef _INPUT_HANDLER
#define _INPUT_HANDLER

#include <iostream> 
#include "freeling/morfo/language.h"
#include "freeling/output/io_handler.h"

namespace freeling {

  namespace io {
    class input_handler : public io_handler {

    public:   
      /// constructor. 
      input_handler ();
      /// destructor
      ~input_handler();

      /// load partially analyzed sentences form 'lines' into a list of sentences
      virtual void input_sentences(const std::wstring &lines, std::list<freeling::sentence> &ls) const = 0;
      virtual void input_document(const std::wstring &lines, freeling::document &doc) const;
    };

  }
}
#endif
