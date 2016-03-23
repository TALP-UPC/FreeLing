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

#ifndef _OUTPUT_TRAIN
#define _OUTPUT_TRAIN

#include <iostream> 
#include "freeling/output/output_handler.h"

namespace freeling {

  namespace io {

    class WINDLL output_train : public output_handler {

    private:
      bool OutputSenses;
      bool Only1stSense;
      bool OutputPhonetics;

      std::list<freeling::analysis> printRetokenizable(std::wostream &sout, const std::list<freeling::word> &rtk, 
                                                       std::list<freeling::word>::const_iterator w, const std::wstring &lem, 
                                                       const std::wstring &tag) const;
      void PrintWord (std::wostream &sout, const freeling::word &w, bool only_sel=true, bool probs=true) const;


    public:   
      // constructor. 
      output_train ();
      ~output_train ();

      // print given sentences to sout in appropriate format
      void PrintResults (std::wostream &sout, const std::list<freeling::sentence> &ls) const;
      // print given a document to sout in appropriate format
      void PrintResults(std::wostream &sout, const freeling::document &doc) const;
      /// inherit other methods
      using output_handler::PrintResults;

    };

  }

}
#endif
