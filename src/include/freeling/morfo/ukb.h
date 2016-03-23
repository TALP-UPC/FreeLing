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
//    contact: Lluis Padro (padro@lsi.upc.edu)
//             TALP Research Center
//             despatx Omega-S112 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#ifndef _FL_UKB_H
#define _FL_UKB_H

#include <string>
#include <list>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/processor.h"
#include "freeling/morfo/csr_kb.h"

namespace freeling {

  const std::wstring RE_WNP=L"^[NARV]";

  class WINDLL ukb : public processor {

  private:  
    freeling::csr_kb *wn;
    freeling::regexp RE_wnpos;
 
    void init_synset_vector(const std::list<freeling::sentence> &, std::vector<double> &) const;
    void extract_ranks_to_sentences(std::list<freeling::sentence> &, const std::vector<double> &) const;

  public:
    ukb(const std::wstring &);
    ~ukb();
 
    /// analyze given sentence
    void analyze(freeling::sentence &) const;
    /// analyze given sentences
    void analyze(std::list<freeling::sentence> &) const;

    /// inherit other methods
    using processor::analyze;
  };

}

#endif
