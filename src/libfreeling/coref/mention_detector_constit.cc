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
//   Author: Jordi Turmo (turmo@lsi.upc.edu)
//
///////////////////////////////////////////////

// TODO :
// Reduccions: 
//  - eliminar de mencions els XPRO a principi d'oracio interrogativa. 
//    Eg: *[Qué]* quiere [él]? Depen del proposit?
//  - eliminar les Noun Phrases que no son alias de NEs
//    Depen del proposit (aquestes Noun Phrases fan baixar la precissio)
// Ampliacions:
//  - controlar sequencies de grups entrecomillats, p.e. mencions d'events sencers

#include <fstream>
#include <sstream>

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/mention_detector_constit.h"


using namespace std;

namespace freeling {

#define MOD_TRACENAME L"MENTION_DETECTOR"
#define MOD_TRACECODE MENTIONS_TRACE


  ///////////////////////////////////////////////////////////////
  /// Create a mention detector module, loading appropriate files.
  ///////////////////////////////////////////////////////////////

  mention_detector_constit::mention_detector_constit(const wstring &filename) {

    // read configuration file and store information
    enum sections {LANGUAGE, LABELS, PROHIBITED_HEADS, REDUCE_MENTIONS};
    config_file cfg(true,L"%");

    cfg.add_section(L"Language",LANGUAGE, true);
    cfg.add_section(L"Labels",LABELS,true);
    cfg.add_section(L"ProhibitedHeads",PROHIBITED_HEADS,true);
    cfg.add_section(L"ReduceMentions",REDUCE_MENTIONS, true);

    if (not cfg.open(filename))
      ERROR_CRASH(L"Error opening file "+filename);

    map<unsigned int, bool> exists_section;
    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;
      sin.str(line);

      switch (cfg.get_section()) {

      case LANGUAGE: {
	// Read the language
        sin>>_Language;
	break;
      }
      case LABELS: {
        // Read morpho/syntactic labels to identify mentions
	wstring name, val;
        sin>>name;
	sin>>val;
	freeling::regexp re(val);
	_Labels.insert(make_pair(name,re));
        break;
      }
      case PROHIBITED_HEADS: {
        // Read file of temporal nouns      
        wstring val;
        sin>>val;
        _No_heads.insert(val);
        break;
      }
      case REDUCE_MENTIONS: {
        // Read ReduceMentions value               
        wstring val;
        sin>>val;
        _Reduce_mentions = (val == L"yes") ? true : false;
        break;
      }
      default: break;
      }
    }
    cfg.close();

    TRACE(3,L"mention detector succesfully created");
  }

  ///////////////////////////////////////////////////////////////
  /// Destructor
  ///////////////////////////////////////////////////////////////

  mention_detector_constit::~mention_detector_constit() {}


  ///////////////////////////////////////////////////////////////  
  /// Get the labels of the configuration             
  ///////////////////////////////////////////////////////////////  
  const map<wstring, freeling::regexp>& mention_detector_constit::get_config_labels() const {
    return _Labels;
  }
    

  ///////////////////////////////////////////////////////////////
  /// Recursively finds all the noun phrases in a parse tree
  ///////////////////////////////////////////////////////////////

  void mention_detector_constit::candidates(int sentn, paragraph::const_iterator s, int &wordn, sentence::const_iterator &itword, int &mentn, parse_tree::const_iterator pt, vector<mention> &mentions, subordinateType &stype) const {
    
    wstring label = pt->get_label();
    subordinateType stype_node=NONE; 
    stype=NONE;

    if (pt.num_children()==0) {
      TRACE(6,L"   Checking word: nw="+util::int2wstring(wordn)+L" "+pt->get_word().get_form());
 
      ///////////////////////////
      // Pronouns and Proper Nouns
      ///////////////////////////
      if (_Labels.find(L"PRON")->second.search(label) or 
          _Labels.find(L"PN")->second.search(label) or
          (_Labels.find(L"NC")->second.search(label) and _Language==L"ENGLISH")) {

        mention candidate(mentn, sentn, s, pt, wordn, itword);

        if (_Labels.find(L"PN")->second.search(label)) {
          candidate.set_type(mention::PROPER_NOUN); 
	  TRACE(6,L"      it is a proper noun: "+label);
	}
	else if (_Labels.find(L"PRON")->second.search(label)) {
          candidate.set_type(mention::PRONOUN);
	  TRACE(6,L"      it is a pronoun: "+label);
	}
	else {
	  // eg: "[agreements] [that] will be wellcome"
	  candidate.set_type(mention::NOUN_PHRASE);
	  TRACE(6,L"      it is a noun phrase (num_ch=0): "+label);
	}
        add_mention(candidate, mentions, mentn);

      }  

      // Colon: means that a non essential subordinated sentence can occur inside an NP
      if (_Labels.find(L"PC")->second.search(label) and stype==NONE) {
        stype = PRE_NON_ESSENTIAL;
        TRACE(6,L"      it is PRE_NON_ESSENTIAL: "+label);
      }
      wordn++;
      itword++;
    }

    ///////////////////////////
    // Coordinated Noun phrases
    ///////////////////////////
    else if (_Labels.find(L"COOR_NP")->second.search(label)) {
      TRACE(6,L"   Checking coordinated NP, nw="+util::int2wstring(wordn)+L" "+label);
 
     // create mention
      int wordnBefore=wordn;
      sentence::const_iterator itwordBefore=itword;
      mention candidate(mentn, sentn, s, pt, wordn, itword);

      candidate.set_type(mention::COMPOSITE);
      wordn=candidate.get_pos_end()+1;
      itword = candidate.get_it_end();
 
      // add mention
      TRACE(6,L"      it is a coordinated noun phrase: "+label);
      add_mention(candidate, mentions, mentn); 
      // try her children
      for (parse_tree::const_sibling_iterator d=pt.sibling_begin(); d!=pt.sibling_end(); ++d) 
        candidates(sentn, s, wordnBefore, itwordBefore, mentn, d, mentions, stype);
    }

    ///////////////////////////
    // Noun phrases in parentheses
    ///////////////////////////
    else if (_Labels.find(L"NP_PARENTH")->second.search(label)) {
      TRACE(6,L"   Checking parenthesized NP nw="+util::int2wstring(wordn)+L" "+label);

      // try her children
      vector<mention> newMentions;

      for (parse_tree::const_sibling_iterator d=pt.sibling_begin(); d!=pt.sibling_end(); d++)
	candidates(sentn, s, wordn, itword, mentn, d, newMentions, stype);

      // add new mentions
      for (vector<mention>::iterator pms=newMentions.begin(); pms!=newMentions.end(); pms++) {
        TRACE(6,L"      parenthesized NP: "+label);
	add_mention(*pms, mentions, mentn);           
      }
    }

    ///////////////////////////
    // Noun phrases
    ///////////////////////////
    else if (_Labels.find(L"NP")->second.search(label)) {
      TRACE(6,L"   Checking NP nw="+util::int2wstring(wordn)+L" "+label);

      // create mention
      int wordnBefore=wordn;
      sentence::const_iterator itwordBefore=itword;
      mention candidate(mentn, sentn, s, pt, wordn, itword);

      candidate.set_type(mention::NOUN_PHRASE);
      wordn=candidate.get_pos_end()+1;
      itword=candidate.get_it_end(); 

      // try her children
      vector<mention> newMentions;
      for (parse_tree::const_sibling_iterator d=pt.sibling_begin(); d!=pt.sibling_end(); ++d){
        candidates(sentn, s, wordnBefore, itwordBefore, mentn, d, newMentions, stype);  
	if (_Language==L"SPANISH") {
	  if (stype==PRE_NON_ESSENTIAL and stype_node==NONE) stype_node = PRE_NON_ESSENTIAL;
	  else if (stype==REL_CLAUSE and stype_node==NONE) {stype_node = ESSENTIAL; stype=NONE;}
	  else if (stype==REL_CLAUSE and stype_node==PRE_NON_ESSENTIAL) {stype_node = NON_ESSENTIAL; stype=NONE;}
	  else if (stype_node==PRE_NON_ESSENTIAL) {stype_node = NONE; stype=NONE;}
	}
	else 
	  if (_Language==L"ENGLISH" and stype_node==NONE) stype_node=stype;
	
      }

      switch (newMentions.size()) {
      case 0: { 
	TRACE(7,L"      basic mention: "+label);
        // eg: "gnom(casa)" -> [gnom(casa)]
        add_mention(candidate, mentions, mentn);
        break;
      }
      case 1: {
        TRACE(7,L"      one child mention: "+label);
        if (newMentions.front().get_type()==mention::NOUN_PHRASE) {
	  // eg "sn(el, [gnom(hombre, [np(IBM)])])" ->"[sn(el, gnom(hombre, [np(IBM)]))]" 
          // eg: "sn(esa, [gnom(casa)])" ->"[sn(esa,gnom(casa))]"
          // precision can decrease. Eg: "el hombre que tiene [dinero]"
          add_mention(candidate, mentions, mentn);
	}
	else if (newMentions.front().get_type()==mention::PROPER_NOUN and (newMentions.front().get_head() != candidate.get_head())) {
	  // eg "gnom(hombre, [np(IBM)])" ->"[gnom(hombre, [np(IBM)])]"
	  // eg"gnom([np(IBM)], man)" ->"[gnom([np(IBM)], man)]"
	  add_mention(candidate, mentions, mentn);
	  newMentions[0].subsumed_with_no_verb(true); // they occur within a candidate in which no verb occurs
	  add_mention(newMentions.front(), mentions, mentn);
	}
	else if (newMentions.front().get_type()==mention::PROPER_NOUN and (candidate.get_pos_end()-candidate.get_pos_begin()==0)) {
	  // eg "gnom([np(IBM)])" ->"gnom([np(IBM)])"
	  add_mention(newMentions.front(), mentions, mentn);
	}
	else if (newMentions.front().get_type()==mention::PRONOUN)  {
	  // eg "gnom([PP(ella)])" -> gnom([PP(ella)])"
	  add_mention(newMentions.front(), mentions, mentn);
	}
	else {
	  // eg "sn(the, gnom([np(New_Millenium)])" ->"[sn(the, gnom(np(New_Millenium))]"
          // eg "gnom([su])" ->"[gnom(su)]"
	  add_mention(candidate, mentions, mentn);
	}
	break;
      }
      default: {
	TRACE(7,L"      more than one child mention: "+label);

	//if (_Language==L"SPANISH" or _Language==L"ENGLISH") // particular for relative clauses in Spanish
        stype=stype_node;

        switch (stype) {

	case NONE:
	case PRE_NON_ESSENTIAL: {
           TRACE(7,L"      case NONE or PRE_NON_ESSENTIAL: "+label);
	  // eg: "[su][camisa]" -> "[[su] camisa]"
          // eg: "[el botón] de [[su] camisa]" -> "[el botón de [[su] camisa]]"
          add_mention(candidate, mentions, mentn);
          for (vector<mention>::iterator pms=newMentions.begin(); pms!=newMentions.end(); pms++)
	    // If the first mention is a NOUN_PHRASE and starts at the candidate start position it will be removed.
	    // If a mention shares the same head as the candidate it will be removed-
	    // eg: "[[President] [Pepe Sanchez]]" -> "[President Pepe Sanchez]"
	    // If the first mention is a PROPER_NOUN it is accepted.
	    // eg: "[John], [the Kid], works hard".

	    if ((!(pms->get_type() == mention::NOUN_PHRASE and pms->get_pos_begin()==candidate.get_pos_begin()) and 
		 !(pms->get_head() == candidate.get_head())) or 
		(pms->get_type() == mention::PROPER_NOUN and pms->get_pos_begin()==candidate.get_pos_begin())) {

	      pms->subsumed_with_no_verb(true); // they occur within a candidate in which any verb occurs
              add_mention(*pms, mentions, mentn); 
	    }
          break;
        }
        case ESSENTIAL: {
           TRACE(7,L"      case ESSENTIAL: "+label);
         //eg: "el [hombre] [que] tiene [el dinero]" -> "[el [hombre] [que] tiene [el dinero]]"
          add_mention(candidate, mentions, mentn);
          for (vector<mention>::iterator pms=newMentions.begin(); pms!=newMentions.end(); pms++)
            add_mention(*pms, mentions, mentn);           
          stype=NONE;
          break;
        }
        case NON_ESSENTIAL: {
           TRACE(7,L"      case NON_ESSENTIAL: "+label);
	 //// WARNING!
	 //// CoNLL annotation includes this as a possible mention
	 //// the next line can be removed if this is not the case
	  add_mention(candidate, mentions, mentn);
         // eg: "[el hombre], [que] tiene [el dinero]," -> same
          for (vector<mention>::iterator pms=newMentions.begin(); pms!=newMentions.end(); pms++)
            add_mention(*pms, mentions, mentn);                   
          stype=NONE;
          break;
        }
        default: break;
        }
      }
      }
    }

    ///////////////////////////
    // other labels
    ///////////////////////////
    else {
      TRACE(7,L"   Checking other nw="+util::int2wstring(wordn) + L" " + label);

      // visit all children
      for (parse_tree::const_sibling_iterator d=pt.sibling_begin(); d!=pt.sibling_end(); ++d) {
        candidates(sentn, s, wordn, itword, mentn, d, mentions, stype);       

	// essential/non_essential relative clauses for the used English grammar
	// in non-essential, colons are within the relative clause (as opposite to the used Spanish grammar)
	if (_Language==L"ENGLISH" and _Labels.find(L"SUB")->second.search(label)) {
	  if (stype==PRE_NON_ESSENTIAL and d==pt.sibling_begin() and stype_node==NONE) stype_node = NON_ESSENTIAL;
	  else if (d==pt.sibling_begin() and stype_node==NONE) stype_node = ESSENTIAL;
	}
      }

      if (_Language==L"ENGLISH" and stype_node != NONE) 
        stype = stype_node; 
      else if (_Language==L"SPANISH" and _Labels.find(L"SUB")->second.search(label))
	stype = REL_CLAUSE;

    }
  }

  ///////////////////////////////////////////////////////////////////////
  /// private functions
  /// Add a new mention in a vector if its head is not prohibited
  ///////////////////////////////////////////////////////////////////////

  void mention_detector_constit::add_mention(const mention &m, vector<mention> &mentions, int &n) const {
    if (_No_heads.find(m.get_head().get_lemma())==_No_heads.end()) {
      mentions.push_back(m);
      n++;
      TRACE(7,L"      Added mention ["<<m.value()<<L"], type="<<m.get_type());
    }
    else { 
      TRACE(7,L"      Skipping fobidden head mention ["+m.value()+L"], type="+util::int2wstring(m.get_type()));
    }
  }



  ///////////////////////////////////////////////////////////////////////
  /// Mark those mentions which are the initial ones in their sentence 
  ///////////////////////////////////////////////////////////////////////

  void mention_detector_constit::mark_initial_mentions_from(int i, vector<mention> &mentions) const{

    mentions[i].set_initial(true);

    int pos_begin = mentions[i].get_pos_begin();
    
    for (unsigned int j=i+1; j<mentions.size(); j++)
      mentions[j].set_initial(mentions[j].get_pos_begin() == pos_begin);
  }

  ///////////////////////////////////////////////////////////////
  /// Discard some mentions with the same head
  ///////////////////////////////////////////////////////////////

  void mention_detector_constit::discard_mentions(std::vector<mention> &mentions) const {
    vector<mention>::const_iterator pm;    

    // TO DO if necessary
  }


  /////////////////////////////////////////////////
  /// Detect entity mentions in a given document.
  /////////////////////////////////////////////////

  vector<mention> mention_detector_constit::detect(const document &doc) const {
    TRACE(3,L"Searching for mentions");
    int sentn=0;
    int mentn=0;
    unsigned int first_mention_pos=0;

    vector<mention> mentions;

    if (!doc.is_parsed() or !doc.is_dep_parsed())
      ERROR_CRASH(L"Error: document is not parsed");

    for (document::const_iterator par=doc.begin(); par!=doc.end(); ++par) {
      for (paragraph::const_iterator se=par->begin(); se!=par->end(); ++se) {

        if (se->is_parsed()) {
          subordinateType stype=NONE;
	  int wordn=0;
	  sentence::const_iterator itword=se->begin();
	 
          candidates(sentn, se, wordn, itword, mentn, se->get_parse_tree().begin(), mentions, stype);

	  if (mentions.size() > first_mention_pos) {
	    mark_initial_mentions_from(first_mention_pos,mentions);
	    first_mention_pos = mentions.size();
	  }
        }
        else
          ERROR_CRASH(L"Error searching mentions in a document without syntactic information. Although this is not mandatory to detect mentions, syntax is required for the next step of coreference resolution");
        sentn++;
      }
    }

    TRACE(4,L"Mention detection done. Found "+util::int2wstring(mentions.size())+L" mentions.");
    // renumbering identifiers
    int i=0;
    for (vector<mention>::iterator pv = mentions.begin(); pv != mentions.end(); pv++, i++) {
      pv->set_id(i);
      TRACE(5, L"mention " + util::int2wstring(pv->get_id()) + L" [" + pv->value() + L"] " + pv->get_head().get_form() + L" type=" + util::int2wstring(pv->get_type()));
    }

    return mentions;
  }

} // namespace


