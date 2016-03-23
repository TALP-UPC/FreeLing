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

#ifndef _FACTORY_H
#define _FACTORY_H

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

namespace freeling {

  ///////////////////////////////////////////////////////////////
  ///  This template is used by numbers, quantities, dates, and ner
  /// classes to dinamycally create the appropriate subclass of
  /// numbers_module, quantities_module, dates_module, or ner_module
  /// (according to received options).
  //////////////////////////////////////////////////////////////

  template<class T>
    class factory {
  protected:
    /// remember which module is doing the real work.
    T* who;

  public:
    /// Constructor, the module is created by the class deriving the factory
    factory<T>() {}
    /// Destructor, delete the module
    ~factory<T>() { delete(who); }

    /// analyze given sentence, calling the created module
    void analyze(sentence &s) const { who->analyze(s); }
    /// analyze given sentences, calling the created module
    void analyze(std::list<sentence> &ls) const { who->analyze(ls); }
    /// analyze given sentence, calling the created module, return copy
    sentence analyze(const sentence &s) const { return who->analyze(s); }
    /// analyze given sentences, calling the created module, return copy
    std::list<sentence> analyze(const std::list<sentence> &ls) const { return who->analyze(ls); }
    /// analyze given document, calling the created module
    void analyze(document &d) const { who->analyze(d); }
    /// analyze given document, calling the created module, return copy
    document analyze(const document &d) const { return who->analyze(d); }
  };

} // namespace

#endif
