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
    enum sections {TAGSET, MTAGS, EXCLUDED, COORD};
    config_file cfg(true,L"%");
    map<unsigned int, wstring> labels_section;

    wstring path=filename.substr(0,filename.find_last_of(L"/\\")+1);

    cfg.add_section(L"TagsetFile", TAGSET, true);
    cfg.add_section(L"MentionTags", MTAGS, true);
    cfg.add_section(L"CoordLabel", COORD);
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
        Tags = new tagset(util::absolute(ftags,path));
        break;
      }

      case MTAGS: {
        // list of tags that are extracted as mentions
        wstring tag, fun, typ;
        sin >> tag >> fun >> typ;
        mention::mentionType t;
        if (typ==L"NounPhrase") t=mention::NOUN_PHRASE;
        else if (typ==L"ProperNoun") t=mention::PROPER_NOUN;
        else if (typ==L"Pronoun") t=mention::PRONOUN;
        else {
          WARNING(L"Invalid mention type "<<typ<<" in file "<<filename<<L". Using default (NounPhrase).");
          t=mention::NOUN_PHRASE;
        }
	mention_tags.insert(make_pair(tag,make_pair(fun,t)));
	break;
      }

      case COORD: {
        // dep function label for coordinations
        sin >> CoordLabel;
	break;
      }

      case EXCLUDED: {
        // list of lemmas excluded from mention extraction
	wstring lemma;
        sin >> lemma;
        excluded.insert(lemma);
        break;
      }
      default: break;
      }
    }
    cfg.close();

    TRACE(3,L"Module succesfully created");
  }


  /////////////////////////////////////////////////
  /// Destructor
  /////////////////////////////////////////////////

  mention_detector_dep::~mention_detector_dep() {
    delete Tags;
  }

  /////////////////////////////////////////////////
  /// Recursively navigate the tree, extracting mentions
  /////////////////////////////////////////////////
  
  void mention_detector_dep::detect_mentions(freeling::dep_tree::const_iterator h, freeling::paragraph::const_iterator se, int sentn, bool maximal, vector<mention> &mentions, int &mentn) const {

    bool found_mention = false;

    wstring tag = Tags->get_short_tag(h->get_word().get_tag());
    auto p = mention_tags.find(tag);

    // If the tag is in the list of mention tags, and the function is different from the 
    // forbiden value (if any), then it is a valid mention
    if (p!=mention_tags.end() and
        excluded.find(h->get_word().get_lemma())==excluded.end() and
        (p->second.first==L"-" or h->get_label()!=p->second.first)) {

      // candidate mention, store it
      mention m(mentn, sentn, se, h);      
      // if is is a coordination, set appropriate type, otherwise use type from config file
      if (is_coordination(h)) m.set_type(mention::COMPOSITE);
      else m.set_type(p->second.second);
      // store maximality 
      m.set_maximal(maximal);
      mentions.push_back(m);
      ++mentn;

      found_mention = true;
      
      // if the mention is a coordination, create also a mention for the head alone
      if (m.is_type(mention::COMPOSITE)) {
        
        // if it is composite, one of the children has CoordLabel. Locate it.
        dep_tree::const_sibling_iterator s; 
        for (s=h.sibling_begin(); 
             s->get_label()!=CoordLabel;
             ++s);
        
        // use word previous to coordination as mention end
        mention m(mentn, sentn, se, h, s->get_word().get_position()-1);
        // use type given by head tag
        m.set_type(p->second.second);

        mentions.push_back(m);
        ++mentn;
      }
    }

    // dive into children. If we are a mention or inside one, tell them they are not maximal
    for (dep_tree::const_sibling_iterator c=h.sibling_begin(); c!=h.sibling_end(); ++c)
      detect_mentions(c, se, sentn, not found_mention and maximal, mentions, mentn);
  }


  /////////////////////////////////////////////////
  /// check whether given depnode is heading a coordination
  /////////////////////////////////////////////////

  bool mention_detector_dep::is_coordination(dep_tree::const_iterator h) const {
    bool found = false;    
    for (dep_tree::const_sibling_iterator s=h.sibling_begin(); s!=h.sibling_end() and not found; ++s)
      found = (s->get_label() == CoordLabel);
    return found;
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
        detect_mentions (se->get_dep_tree().begin(), se, sentn, true, mentions, mentn);       
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
            << ", mention=(" << mentions[k].get_pos_begin() << L"," << mentions[k].get_pos_end() << L")"
            << " [" << mentions[k].value() << L"]");
    }
    #endif

    return mentions;
  }


} // namespace


