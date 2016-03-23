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

#include <sstream>
#include "freeling/omlet/classifier.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

#define MOD_TRACENAME L"CLASSIFIER"
#define MOD_TRACECODE OMLET_TRACE

using namespace std;

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Constructor: Load the label names
  ////////////////////////////////////////////////////////////////

  classifier::classifier(const wstring &cod) {

    wistringstream sin;
    sin.str(cod);

    label_others=L"";

    wstring key,name;
    int nlabels=0;
    while(sin>>key>>name) {
      if (key==L"<others>")
        label_others=name;
      else {
        int k = util::wstring2int(key);
        if (k!=nlabels) 
          ERROR_CRASH(L"Unordered label names in class definition '"+cod+L"'");

        labels.push_back(name);
        nlabels++; 
      }
    }
  }

  ////////////////////////////////////////////////////////////////
  /// Return the name for class k
  ////////////////////////////////////////////////////////////////

  wstring classifier::get_label(int k) const {
    return labels[k];
  }

  ////////////////////////////////////////////////////////////////
  /// Return the index for class with name n
  ////////////////////////////////////////////////////////////////

  int classifier::get_index(const wstring &n) const {
    // search for class name
    for (size_t k=0; k<labels.size(); k++)
      if (labels[k]==n) return k;
    // class name not found
    return -1;
  }

  ////////////////////////////////////////////////////////////////
  /// Return the name for default class (emtpy string if no default class used)
  ////////////////////////////////////////////////////////////////

  wstring classifier::default_class() const {
    return label_others;
  }

  ///////////////////////////////////////////////////////////////
  ///  Get number of labels of the classifier
  ///////////////////////////////////////////////////////////////

  int classifier::get_nlabels() const {
    return labels.size();
  }

} // namespace
