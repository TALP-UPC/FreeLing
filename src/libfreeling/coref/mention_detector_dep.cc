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

///////////////////////////////////////////////
//
//   Author: Lluis Padro
//
///////////////////////////////////////////////


#include <fstream>
#include <sstream>

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/mention_detector_dep.h"


using namespace std;

namespace freeling {

#define MOD_TRACENAME L"MENTION_DETECTOR"
#define MOD_TRACECODE MENTIONS_TRACE


  ///////////////////////////////////////////////////////////////
  /// Create a mention detector module, loading appropriate files.
  ///////////////////////////////////////////////////////////////

  mention_detector_dep::mention_detector_dep(const wstring &filename) {

    // read configuration file and store information
    enum sections {TAGSET, MTAGS, EXCLUDED};
    config_file cfg(true,L"%");
    map<unsigned int, wstring> labels_section;

    wstring path=filename.substr(0,filename.find_last_of(L"/\\")+1);

    cfg.add_section(L"TagsetFile", TAGSET, true);
    cfg.add_section(L"MentionTags", MTAGS, true);
    cfg.add_section(L"Excluded", EXCLUDED);

    if (not cfg.open(filename))
      ERROR_CRASH(L"Error opening file "+filename);

    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;
      sin.str(line);

      switch (cfg.get_section()) {

      case TAGSET: {
        // reading Tagset description file name
        wstring ftags;
        sin>>ftags;
        // load tagset description
        TRACE(3,L"Loading tagset file "+ util::absolute(ftags,path));      
        Tags = new tagset(util::absolute(ftags,path));
        break;
      }

      case MTAGS: {
        // list of tags that are extracted as mentions
        wstring tag;
        sin >> tag;
	mention_tags.insert(tag);
        TRACE(6,L"Loaded mention tag "<<tag);
	break;
      }
      case EXCLUDED: {
        // list of lemmas excluded from mention extraction
	wstring lemma;
        sin >> lemma;
        excluded.insert(lemma);
        TRACE(6,L"Loaded mention excluded "<<lemma);
        break;
      }
      default: break;
      }
    }
    cfg.close();

    TRACE(3,L"mention detector succesfully created");
  }


  /////////////////////////////////////////////////
  /// Destructor
  /////////////////////////////////////////////////

  mention_detector_dep::~mention_detector_dep() {
    delete Tags;
  }


  /////////////////////////////////////////////////
  /// Detect entity mentions in a given document.
  /////////////////////////////////////////////////

  vector<mention> mention_detector_dep::detect(const document &doc) const {

    TRACE(3,L"Searching for mentions");

    vector<mention> mentions;

    int sentn=0;
    int mentn=0;

    if (not doc.is_dep_parsed())
      ERROR_CRASH(L"Error: document is not dependency-parsed");

    for (document::const_iterator par=doc.begin(); par!=doc.end(); ++par) {
      for (paragraph::const_iterator se=par->begin(); se!=par->end(); ++se) {
       
        const freeling::dep_tree &dt = se->get_dep_tree();
        for (dep_tree::const_iterator h = dt.begin(); h != dt.end(); ++h) {
          wstring tag = Tags->get_short_tag(h->get_word().get_tag());
          if (mention_tags.find(tag)!=mention_tags.end() and
              excluded.find(h->get_word().get_lemma())==excluded.end()) {
            // candidate mention, store it
            mention m(mentn, sentn, se, h);
            mentions.push_back(m);
            ++mentn;
          }          
        }
        
        ++sentn;
      }
    }

    TRACE(3,L"Mention detection done. Found " << mentions.size() << L" mentions.");
    #ifdef VERBOSE
    for (int k=0; k<mentions.size(); ++k) {
      TRACE(5, L"Mention " << k << L": " 
            << " id=" << mentions[k].get_id()
            << ", ns=" << mentions[k].get_n_sentence()
            << ", head=" << mentions[k].get_head().get_position() << L" ("<<mentions[k].get_head().get_lemma() << L")"
            << ", begin=" << mentions[k].get_pos_begin() << L" (" << (*mentions[k].get_sentence())[mentions[k].get_pos_begin()].get_form() << L")"
            << ", end=" << mentions[k].get_pos_end() << L" (" << (*mentions[k].get_sentence())[mentions[k].get_pos_end()].get_form() << L")");
    }
    #endif

    return mentions;
  }


} // namespace


