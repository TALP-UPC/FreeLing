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

#include <fstream>
#include <sstream>
#include <ctime>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/ukb.h"
#include "freeling/morfo/configfile.h"

using namespace std;

#define MOD_TRACENAME L"UKB_WSD"
#define MOD_TRACECODE WSD_TRACE

namespace freeling {

  //////////////////////////////////////////////////////////////////
  ///  Create UKB word sense disambiguator reading config from given file
  
  ukb::ukb(const std::wstring &wsdFile) : RE_wnpos(RE_WNP) {

    // default path to included files
    wstring path=wsdFile.substr(0,wsdFile.find_last_of(L"/\\")+1);
    wstring relFile;

    // default values for pageRank parameters
    double thr=0.000001;
    int nit=30;
    double damp=0.85;

    // configuration file
    enum sections {RELATION_FILE,REX_WNPOS,PR_PARAMS};
    config_file cfg;  
    cfg.add_section(L"RelationFile",RELATION_FILE);
    cfg.add_section(L"RE_Wordnet_PoS",REX_WNPOS);
    cfg.add_section(L"PageRankParameters",PR_PARAMS);
    
    if (not cfg.open(wsdFile))
      ERROR_CRASH(L"Error opening file "+wsdFile);

    // read ukb configuration file 
    wstring line; 
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) {

      case RELATION_FILE: { // reading relation file name
        wistringstream sin;
        sin.str(line);
        wstring fname;
        sin>>fname;
        relFile=freeling::util::absolute(fname,path); 
        break;
      }

      case REX_WNPOS: { // reading regex for WN PoS tags
        RE_wnpos = freeling::regexp(line);
        break;
      }

      case PR_PARAMS: { // reading page rank parameters
        wistringstream sin;
        sin.str(line);
        wstring key; 
        sin>>key;
        if (key==L"Threshold") sin>>thr;
        else if (key==L"MaxIterations") sin>>nit;
        else if (key==L"Damping") sin>>damp;
        else 
          WARNING(L"Error: Unknown parameter "+key+L" in PageRankParameters section of file "+wsdFile+L"."); 
        break;
      }

      default: break;
      }
    }
    cfg.close();
    
    if (relFile.empty()) 
      ERROR_CRASH(L"No relation file provided in UKB configuration file "+wsdFile+L".");
    
    // load relation graph
    wn = new freeling::csr_kb(relFile,nit,thr,damp);
        
    TRACE(1,L"UKB module successfully loaded");
  }

  
  //////////////////////////////////////////////////////////////////
  ///  Destructor

  ukb::~ukb() {
    delete wn;
  }
 
  //////////////////////////////////////////////////////////////////
  ///  Init a weight vector for synsets in the sentences 

  void ukb::init_synset_vector(const list<freeling::sentence> &ls, vector<double> &pv) const {
    /// get synsets for words in sentence, checking for duplicates
    int nw=0;

    map<wstring,pair<const freeling::word*,int> > uniq;  // unique words in the sentence

    for (list<freeling::sentence>::const_iterator s=ls.begin(); s!=ls.end(); s++) {
      int best = s->get_best_seq();
      for (sentence::const_iterator w=s->begin(); w!=s->end(); w++) {
        // count the word as relevant if it has a WN tag (N,A,R,V), even if it has no synsets
        if (RE_wnpos.search(w->get_tag(best))) {
          // check if first occurrence of word+PoS in the sentence list
          wstring key=w->get_lc_form()+L"#"+freeling::util::lowercase(w->get_tag(best).substr(0,1));
          
          if (uniq.find(key)==uniq.end()) {  // new word, add it 
            nw++;
            uniq.insert(make_pair(key,make_pair(&(*w),best)));
          }
        }
      }
    }

    // for each unique word-PoS, compute PV for its synsets
    vector<double>(wn->size(), 0.0).swap(pv);
    for (map<wstring,pair<const freeling::word*,int>>::const_iterator u=uniq.begin(); u!=uniq.end(); u++) {
      const freeling::word *w = u->second.first;
      int best = u->second.second;
      const list<pair<wstring,double> > & lsen = w->get_senses(best);
      int nsyn = lsen.size();
      for (list<pair<wstring,double> >::const_iterator s=lsen.begin(); s!=lsen.end(); s++) {
        size_t syn = wn->get_vertex(s->first);
        if (syn==csr_kb::VERTEX_NOT_FOUND)
          WARNING(L"Unknown synset "<<s->first<<L" ignored. Please check consistency between sense dictionary and KB.");
        else 
          pv[syn] += (1.0/(double)nw)*(1.0/(double)nsyn);
      } 
    }
  }

  //////////////////////////////////////////////////////////////////
  /// Rank synsets in each sentence word according to given rank vector

  void ukb::extract_ranks_to_sentences(list<freeling::sentence> &ls, const vector<double> &pv) const {
    /// move obtained ranks to word synsets, sorting sense list by rank
    for (list<freeling::sentence>::iterator s=ls.begin(); s!=ls.end(); s++) {
      int best = s->get_best_seq();
      for (sentence::iterator w=s->begin(); w!=s->end(); w++) {
        // get reference to list of senses for the word
        list<pair<wstring,double> > & lsen = w->get_senses(best);
        // store obtained ranks in the list of senses
        for (list<pair<wstring,double> >::iterator p=lsen.begin(); p!=lsen.end(); p++) {
          // get rank for the synset
          size_t syn = wn->get_vertex(p->first);          
          // update list of senses with new ranks
          if (syn!=csr_kb::VERTEX_NOT_FOUND) p->second=pv[syn];
        }
        // sort list by descending rank
        lsen.sort(util::descending_second<wstring,double>);
      }
    }
  }

  //////////////////////////////////////////////////////////////////
  ///  Disambiguate given sentence (alone, no extra context)
  ///  Provided for completitude, but not really useful.

  void ukb::analyze(freeling::sentence &s) const {
    list<freeling::sentence> ls;
    ls.push_back(s);
    analyze(ls);
    s=ls.front();
  }

  //////////////////////////////////////////////////////////////////
  ///  Disambiguate given sentences (all toghether, this is the expected use)

  void ukb::analyze(std::list<freeling::sentence> &ls) const {

    //  Init a weight vector for synsets in the sentences 
    vector<double> pv;
    init_synset_vector(ls,pv);

    // page rank the vector, using loaded kb
    wn->pagerank(pv);

    // Extract ranks from the vector, and move results to sentence
    extract_ranks_to_sentences(ls,pv);
  }

}
