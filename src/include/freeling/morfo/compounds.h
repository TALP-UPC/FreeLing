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

#ifndef _COMPOUNDS
#define _COMPOUNDS

#include <string>
#include <list>

#include "freeling/morfo/language.h"
#include "freeling/morfo/foma_FSM.h"

namespace freeling {

  // just predeclaring
  class dictionary;

  ////////////////////////////////////////////////////////////////
  ///  Class compounds implements a compound checker 
  ////////////////////////////////////////////////////////////////

  class compounds {

    private:

       /// Auxiliary class to store a pattern
       class pattern {
         public:
           std::wstring patr;
           int head;
           std::wstring tag;

           pattern(const std::wstring &, int, const std::wstring &);
           ~pattern();
       };


       /// whether only unknown words (vs all words) are checked for compounds
       bool unknown_only;
       /// list of valid compound patterns (e.g. NC_NC, NC_SP_NC, etc),
       /// with head position for each. 
       std::list<pattern > patterns;

       /// FSM to check for compounds
       foma_FSM *fsm;

       // dictionary we are included in
       const dictionary &dic;

    public:
      /// Create a suffixed words analyzer. 
      compounds(const std::wstring &, const dictionary &);
      /// Destroy a suffixed words analyzer. 
      ~compounds();
      
      /// Check whether a word is a compound. Add analysis if it is.
      bool check_compound(word &) const;
  };

}

#endif

