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
#include <fstream>

#include "freeling/morfo/punts.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"PUNCTUATION"
#define MOD_TRACECODE PUNCT_TRACE

#define OTHER L"<Other>"

  ////////////////////////////////////////////////////////////////
  /// Create a punctuation sign recognizer.
  ////////////////////////////////////////////////////////////////

  punts::punts(const std::wstring &puntFile) : database(puntFile) {
    tagOthers=access_database(OTHER);
  }


  ////////////////////////////////////////////////////////////////
  /// Detect and annotate punctuation signs in given sentence,
  /// using given options.
  ////////////////////////////////////////////////////////////////

  void punts::analyze(sentence &se) const {
    sentence::iterator i;
    wstring form;

    for (i=se.begin(); i!=se.end(); ++i) {

      form=i->get_form();
      TRACE(3,L"checking "+form);

      // search for word in punctuation list
      wstring data=access_database(form);

      if (not data.empty()) {
        TRACE(3,L"     ["+form+L"] found in map: known punctuation");
        // extract lemma+tag from recovered data string
        wstring lemma=data.substr(0,data.find(L" "));
        wstring tag=data.substr(data.find(L" ")+1);            
        // punctuation sign found in the hash
        i->set_analysis(analysis(lemma,tag));
        // prevent any other module from reinterpreting this.
        i->lock_analysis();
      }
      else { 
        TRACE(3,L"     ["+form+L"] Not found in map: check if it is punctuation..");
        // Not found in list. If no alphanumeric chars, tag it as "others"
        if (not util::RE_has_alphanum.search(form)) {
          TRACE(3,L"   .. no alphanumeric char found. tag as "+tagOthers);
          i->set_analysis(analysis(form,tagOthers));
        }
      }
    }
    
    TRACE(3,L"done ");
    TRACE_SENTENCE(1,se);
  }

} // namespace
