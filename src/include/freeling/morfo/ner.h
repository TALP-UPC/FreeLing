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

#ifndef _NER
#define _NER

#include "freeling/windll.h"
#include "freeling/morfo/factory.h"
#include "freeling/morfo/ner_module.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///   Class ner implements a wrapper to transparently 
  ///   create and access a ner_module named entity recognizer
  ////////////////////////////////////////////////////////////////

  class WINDLL ner : public factory<ner_module> {

  public:
    /// Constructor
    ner(const std::wstring &);
    /// Destructor
    ~ner();
  };

} // namespace

#endif

