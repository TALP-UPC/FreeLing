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

#include <fstream>
#include <sstream>

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/coref.h"


using namespace std;

namespace freeling {

#define MOD_TRACENAME L"COREF"
#define MOD_TRACECODE COREF_TRACE

  ///////////////////////////////////////////////////////////////
  /// Create an empty coreference module
  ///////////////////////////////////////////////////////////////

  coref::~coref() {
    delete extractor;
    delete classifier;
  }

  ///////////////////////////////////////////////////////////////
  /// Create a coreference module, loading appropriate files.
  ///////////////////////////////////////////////////////////////

  coref::coref(const wstring &filename) {
 
    freeling::regexp blankline(L"^[ \t]*$");
    wstring path=filename.substr(0,filename.find_last_of(L"/\\")+1);

    wstring sdbf;
    wstring abfile,abclasses;

    unsigned int features=COREFEX_ALL;

    // read configuration file and store information   
    enum sections {MODELFILE,MAX_DIST,SEMDB,CLASSES,FEATURES};
    config_file cfg(false,L"%");  
    cfg.add_section(L"Model",MODELFILE);
    cfg.add_section(L"MaxDistance",MAX_DIST);
    cfg.add_section(L"SEMDB",SEMDB);
    cfg.add_section(L"Classes",CLASSES);
    cfg.add_section(L"Features",FEATURES);

    if (not cfg.open(filename))
      ERROR_CRASH(L"Error opening file "+filename);

    wstring line;
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) { 

      case MODELFILE: {
        // get .abm file absolute name
        wistringstream sin;  sin.str(line);
        sin>>abfile;
        abfile = util::absolute(abfile,path);
        break;
      }

      case MAX_DIST: {
        // Read MaxDistance value
        wistringstream sin;  sin.str(line);
        sin>>MaxDistance ;
        break;
      }

      case SEMDB: {
        // load SEMDB section
        wstring fname;
        wistringstream sin;  sin.str(line);
        sin>>fname;
        sdbf= util::absolute(fname,path);
        break;
      }

      case CLASSES: {
        // get class codes for AdaBoost classifier
        abclasses=line;
        break;
      }

      case FEATURES: {
        // read active feature groups
        wistringstream sin;  sin.str(line);
        sin>>std::hex>>features;
        break;
      }

      default: break;
      }
    }
    cfg.close();

    // create feature extractor
    TRACE(3,L" Loading feature extractor");
    extractor = new coref_fex(features, sdbf);

    // create AdaBoost classifier
    TRACE(3,L" Loading adaboost model "+abfile);
    if (abfile.empty()) ERROR_CRASH(L"Missing <Model> section in config file "+filename);
    if (abclasses.empty()) ERROR_CRASH(L"Missing <Classes> section in config file "+filename);
    classifier = new adaboost(abfile,abclasses);

    TRACE(3,L"analyzer succesfully created");
  }

  ///////////////////////////////////////////////////////////////
  /// Fills a mention from a parse tree node
  ///////////////////////////////////////////////////////////////

  void coref::set_mention(parse_tree::iterator pt, int &wordn, mention_ab &men) const {
    parse_tree::sibling_iterator d;

    if (pt->num_children()==0) {
      const word & wo=pt->info.get_word();
      men.tokens.push_back(wo.get_lc_form());
      men.tags.push_back(wo.get_tag());
      TRACE(3,L"  mention word: "+util::int2wstring(wordn)+L" "+wo.get_form());
      wordn++;
    }
    else {
      for (d=pt->sibling_begin(); d!=pt->sibling_end(); ++d) {
        set_mention(d, wordn, men);
      }
    }
  }

  ///////////////////////////////////////////////////////////////
  /// Finds recursively all the SN and put in a list of samples
  ///////////////////////////////////////////////////////////////

  void coref::add_candidates(int sent, int &wordn, int &mentn, parse_tree::iterator pt, list<mention_ab> & candidates) const {
    parse_tree::sibling_iterator d;

    if (pt->num_children()==0) {
      TRACE(3,L"  out word: "+util::int2wstring(wordn)+L" "+pt->info.get_word().get_form());
      wordn++;
    }
    else if (pt->info.get_label() == L"sn") {
      mention_ab candidate;
      candidate.sent = sent;
      candidate.ptree = pt;
      candidate.posbegin = wordn;
      candidate.numde = mentn;
      set_mention(pt, wordn, candidate);
      candidate.posend = wordn-1;
    
      candidates.push_back(candidate);
      mentn++;
    }
    else {
      for (d=pt->sibling_begin(); d!=pt->sibling_end(); ++d)
        add_candidates(sent, wordn, mentn, d, candidates);
    }
  }

  ///////////////////////////////////////////////////////////////
  /// Check if the two mentions are coreferent. Uses the classifier.
  ///////////////////////////////////////////////////////////////

  bool coref::check_coref(const mention_ab & m1, const mention_ab & m2) const{

    TRACE(5,L"    -Encoding example");
    //m1.print()
    //m2.print();
    std::vector<int> encoded;
    extractor->extract(m1, m2, encoded);

    example exampl(classifier->get_nlabels());
    TRACE(5,L"   Encoded example:");
    for(std::vector<int>::iterator it=encoded.begin(); it!=encoded.end(); ++it) {
      exampl.add_feature((*it));
      TRACE(5,L"          "+util::int2wstring(*it));
    }

    TRACE(5,L"    -Classifying");
    // classify current example
    double *pred = new double[classifier->get_nlabels()];
    classifier->classify(exampl,pred);

    TRACE(4,L"    -Prediction for "+classifier->get_label(0)+L" = "+util::double2wstring(pred[0]));
    TRACE(4,L"    -Prediction for "+classifier->get_label(1)+L" = "+util::double2wstring(pred[1]));

    // return true if class 1 (corref) has positive prediction and
    // higher than class 0 (non-corref)
    bool b=(pred[1]>0 and pred[1]>pred[0]);
    delete []pred;

    return b;
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Find coreferences between NPs in given document.
  /////////////////////////////////////////////////////////////////////////////

  void coref::analyze(document &doc) const {

    list<mention_ab> candidates;

    TRACE(3,L"Searching for candidate noun phrases");
    int sentn=0;
    int wordn=0;
    int mentn=0;
    for (document::iterator par=doc.begin(); par!=doc.end(); ++par) {
      for (paragraph::iterator se=par->begin(); se!=par->end(); ++se) {
        add_candidates(sentn, wordn, mentn, se->get_parse_tree().begin(), candidates);
        sentn++;
      }
    }

    TRACE(3,L"Pairing "+util::int2wstring(candidates.size())+L" candidates");
    list<mention_ab>::const_iterator i = candidates.begin();
    for (i++; i!=candidates.end(); i++) {
      TRACE(4,L"   Pairing '"+util::vector2wstring(i->tokens,L" ")+L"' with all previous");
      bool found = false;
      int count = 0;
      list<mention_ab>::const_iterator j=i;
      while (j!=candidates.begin() and not found and count<MaxDistance) {
        j--;

        TRACE(4,L"   checking pair ("+util::vector2wstring(j->tokens,L" ")+L"<"
              +j->ptree->info.get_node_id()+L">,"
              +util::vector2wstring(i->tokens,L" ")+L"<"
              +i->ptree->info.get_node_id()+L">)");
        found = check_coref(*j, *i);
        if (found) 
          doc.add_positive(j->ptree->info.get_node_id(), i->ptree->info.get_node_id());

        TRACE(4,L"   -> "+wstring(found? L"COREFERENT" : L"not coreferent"));

        count++;
      }
    }
  }
} // namespace
