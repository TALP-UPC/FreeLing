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

#ifndef _ACCENTS
#define _ACCENTS

#include "freeling/morfo/accents_modules.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///
  ///  The class accents provides a wrapper to transparently
  ///  create and access an accents_module to handle accentuation
  ///  for the appropriate language.
  ///
  ////////////////////////////////////////////////////////////////

  class accents {

  private:
    /// remember which module is doing the real work.
    accents_module * who;

  public:
    /// Constructor
    accents(const std::wstring &);
    /// Destructor
    ~accents();

    /// fix accentuation patterns of a list of root
    /// candidates, according to a suffix rule
    void fix_accentuation(std::set<std::wstring> &, const sufrule &) const;
  };

} // namespace

#endif
