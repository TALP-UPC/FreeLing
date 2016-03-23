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

#include "freeling/morfo/fex.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"FEX"
#define MOD_TRACECODE FEX_TRACE

  ////////////////////////////////////////////////////////////////
  /// constructor, given rule file, lexicon file (may be empty), and custom functions
  ////////////////////////////////////////////////////////////////

  fex::fex(const wstring &rgfFile,
           const wstring &lexFile,
           const map<wstring,const feature_function *> &custom) : lex(lexFile), feat_functs(custom) {

    /// open and read feature extraction rule file.
    wstring path=rgfFile.substr(0,rgfFile.find_last_of(L"/\\")+1);

    wifstream fabr;
    util::open_utf8_file(fabr, rgfFile);
    if (fabr.fail()) ERROR_CRASH(L"Error opening file "+rgfFile);
    
    Tags = NULL;

    // loading rules
    fex_rulepack pk;

    enum sections {NONE,RULES,TAGSET};
    sections sect = NONE;

    int pkid=0;
    int ruleid=0;
    wstring line;
    while (getline(fabr,line)) {
      wstring token;
      wistringstream sin;
      sin.str(line);
      sin>>token;  // get first token in the line

      if (token==L"RULES") {  // rule pack begins
        // create new rule pack.
        pk.rules.clear(); pk.conds.clear();
        // assign pakage identifier
        pkid++;
        // special rule-id for package conditions
        wstring rid=util::int2wstring(pkid)+L"_0";
        // read target word condition
        read_condition(sin,rid,path,pk.conds,pk.operation);
        // interpret next lines as rules.
        sect=RULES; 
        TRACE(2,L"Package initialized");
      }

      else if (token==L"ENDRULES") {  // rule pack ends
        // store just finished rule pack in the set of rule packs.
        packs.push_back(pk);
        // next lines are not rules
        sect=NONE;
        TRACE(2,L"Package finished");
      }

      else if (sect==NONE and token==L"TAGSET") {
        sin>>token;  // get next token in the line, should be tagset file name
        Tags = new tagset(util::absolute(token,path));
      }

      else if (sect==RULES and not token.empty()) {  // non-blank rule line.
        wstring rang;
        int op;
        list<fex_condition> lcd;
        // read rang
        sin>>rang;
        // assign identifier to new rule
        ruleid++;
        wstring rid=util::int2wstring(pkid)+L"_"+util::int2wstring(ruleid);
        // read rule condition
        read_condition(sin,rid,path,lcd,op);
        // add rule to rule pack
        pk.rules.push_back(fex_rule(rid,token,rang,op,lcd,feat_functs));
        TRACE(2,L"  Added new rule "+rid+L" "+token+L" "+rang);
      }

      else if (token[0]!=L'#' and not token.empty()) 
        // if comment or blank, ignore line. Otherwise, complain.
        WARNING(L"Unexpected token '"+token+L"' in file "+rgfFile+L". Line ignored.");
    }
    fabr.close(); 

    if (Tags==NULL) {
      WARNING(L"**** Tagset definition file not specified.");
      WARNING(L"**** This will cause problems if your RGF rules use conditions on short tags ('t').");
    }

    /// packs.begin()->trace();
    TRACE(1,L"Feature extraction rules loaded");    
  }

  ////////////////////////////////////////////////////////////////
  /// Destructor
  ////////////////////////////////////////////////////////////////

  fex::~fex() {
    delete Tags;
  }

  ////////////////////////////////////////////////////////////////
  /// Extract *all possible* features (by name) for given sentence
  ////////////////////////////////////////////////////////////////

  void fex::get_features(sentence &sent, vector<set<wstring> > &resN, vector<set<int> > &resI, int encode) const {

    TRACE(3,L"get_features. encode="+util::int2wstring(encode));

    if (encode & ENCODE_NAME) 
      // prepare to extract features by name
      resN = vector<set<wstring> >(sent.size(),set<wstring>());

    if (encode & ENCODE_INT) { 
      // prepare to extract features by code
      if (lex.is_empty()) 
        WARNING(L"ENCODE_INT requested, but no lexicon was provided. Feature sets will be empty.");
      resI = vector<set<int> >(sent.size(),set<int>());
    }

    // starting new sentence, create a new status to store features, and initialize it for each rule.
    fex_status *st = new fex_status();
    sent.set_processing_status((processor_status *)st);
    for (list<fex_rulepack>::const_iterator pack=packs.begin(); pack!=packs.end(); pack++)
      for (list<fex_rule>::const_iterator r=pack->rules.begin(); r!=pack->rules.end(); r++) 
        st->features.insert(make_pair(r->get_id(),map<int,list<wstring> >()));

    // start applying rule packs
    list<fex_rulepack>::const_iterator pack;
    for (pack=packs.begin(); pack!=packs.end(); pack++) {
      TRACE(3,L"Processing rule pack");

      // precompute and store features, to avoid repetitions later.
      if (pack->conds.size()==1 and pack->conds.begin()->is_true())
        // Pack condition is ALL. Precompute each rule once per word.
        precompute_once(*pack,sent);
      else
        // Pack condition is not ALL. Precompute each rule only 
        // for required ranges around matching words.
        precompute_range(*pack,sent);

      // The pack is precomputed. Now we can generate actual features.
      for (int nw=0; nw<(int)sent.size(); nw++) {
        TRACE(3,L"  Extracting features for word "+sent[nw].get_form());

        if (fex_rule::check_conds(pack->conds, pack->operation, sent[nw], *Tags, st)) {
          for (list<fex_rule>::const_iterator r=pack->rules.begin(); r!=pack->rules.end(); r++) {
            int first = max((int)nw+r->get_left(), 0);
            int last = min((int)nw+r->get_right(), (int)sent.size()-1);

            TRACE(4,L"    Checking rule. range=["+util::int2wstring(first)+L","
                  +util::int2wstring(last)+L"]");

            for (int nw1=first; nw1<=last; nw1++) {
              TRACE(4,L"      Extracting for "+sent[nw1].get_form());
              // for each word in the range, extract the feature for this rule
              list<wstring> feat;
              r->extract(sent,nw1,nw,*Tags,feat);
              TRACE(4,L"      Features extracted="+util::int2wstring(feat.size()));
              for (list<wstring>::iterator s=feat.begin(); s!=feat.end(); s++) {
                if (not lex.is_empty()) {
                  // lexicon is available: Filter features.
                  unsigned int c=lex.get_code(*s); 
                  if (c>0) {  
                    // code=0 means feature not in lexicon, to be ignored
                    if (encode & ENCODE_NAME) resN[nw].insert(*s);
                    if (encode & ENCODE_INT) resI[nw].insert(c);
                  }           
                  TRACE(4,L"      Feature "+(*s)+wstring(c>0? L" passed":L" didn't pass")+
                        L" lexicon filter.");
                }
                else 
                  // no lexicon available, add all features (by name)
                  if (encode & ENCODE_NAME) resN[nw].insert(*s);
              }
            }
          }
        }
        else {TRACE(4,L"    Cond doesn't check. skip rule");}
      }

      // rule pack finished, go to next rule pack.
      TRACE(4,L"Rule pack finished.");
    }

    sent.clear_processing_status();
  }

  ////////////////////////////////////////////////////////////////
  /// Extract features (by name) for given sentence, filtering
  /// by current lexicon.
  ////////////////////////////////////////////////////////////////

  void fex::encode_name(sentence &sent, vector<set<wstring> > &result) const {
    // get all sentence features, filtering by current lexicon
    vector<set<int> > dummy;
    get_features(sent,result,dummy,ENCODE_NAME);
  }

  ////////////////////////////////////////////////////////////////
  /// Extract features (by name) for given sentence, filtering
  /// by current lexicon.  Return vector (useful for Java/perl APIs)
  ////////////////////////////////////////////////////////////////

  vector<set<wstring> > fex::encode_name(sentence &sent) const {
    // get all sentence features, filtering by current lexicon
    vector<set<wstring> > result;
    vector<set<int> > dummy;
    get_features(sent,result,dummy,ENCODE_NAME);
    return result;
  }


  ////////////////////////////////////////////////////////////////
  /// Extract features (by code) for given sentence, filtering
  /// by current lexicon.
  ////////////////////////////////////////////////////////////////

  void fex::encode_int(sentence &sent, vector<set<int> > &result) const {
    // get all sentence features, filtering by current lexicon
    vector<set<wstring> > dummy;
    get_features(sent,dummy,result,ENCODE_INT);
  }

  ////////////////////////////////////////////////////////////////
  /// Extract features (by code) for given sentence, filtering
  /// by current lexicon. Return vector (useful for Java/perl APIs)
  ////////////////////////////////////////////////////////////////

  vector<set<int> > fex::encode_int(sentence &sent) const {
    // get all sentence features, filtering by current lexicon
    vector<set<int> > result;
    vector<set<wstring> > dummy;
    get_features(sent,dummy,result,ENCODE_INT);
    return result;
  }

  ////////////////////////////////////////////////////////////////
  /// Extract features (by name and code) for given sentence, filtering
  /// by current lexicon.
  ////////////////////////////////////////////////////////////////

  void fex::encode_all(sentence &sent, vector<set<wstring> > &resN, vector<set<int> > &resI) const {
    // get all sentence features, filtering by current lexicon
    get_features(sent,resN,resI,ENCODE_INT|ENCODE_NAME);
  }

  ////////////////////////////////////////////////////////////////
  /// Clear current lexicon. Useful for training.
  ////////////////////////////////////////////////////////////////

  void fex::clear_lexicon() { 
    lex.clear_lexicon();
  }

  ////////////////////////////////////////////////////////////////
  /// Extract features (by name) for given sentence, add them
  /// to current lexicon. Useful for training.
  ////////////////////////////////////////////////////////////////

  void fex::encode_to_lexicon(sentence &sent) { 

    // get all features, adding them to lexicon.
    vector<set<wstring> > resN;
    encode_name(sent,resN);

    // Add obtained features to lexicon
    vector<set<wstring> >::const_iterator w;
    for (w=resN.begin(); w!=resN.end(); w++)
      for (set<wstring>::const_iterator f=w->begin(); f!=w->end(); f++)
        lex.add_occurrence(*f);
  }

  ////////////////////////////////////////////////////////////////
  /// save lexicon to a file, filtering features with low occurrence rate
  ////////////////////////////////////////////////////////////////

  void fex::save_lexicon(const wstring &file, double thres) const  {
    lex.save_lexicon(file,thres);
  }


  ////////////////////////////////////////////////////////////////
  /// read a condition from RGF file
  ////////////////////////////////////////////////////////////////

  void fex::read_condition(wistringstream &sin, const wstring &rid, const wstring &path,list<fex_condition> &conds, int &op) {

    wstring oper=L"";
    wstring foc,fun,parm;

    // condition identifiers
    int ncond=0;

    // get first condition focus
    sin>>foc;

    conds.clear();
    if (foc==L"ALL") {   // trivial condition, just build it.
      fun=L""; parm=L"";
      ncond++;
      wstring cid = rid+L":"+util::int2wstring(ncond);
      conds.push_back(fex_condition(cid,foc,fun,parm,set_files)); 
    }
    else {
      // get rest of the condition
      sin>>fun>>parm;
      if (fun==L"in_set") fun=L"any_in_set"; // default=any
      if (fun.find(L"in_set")!=wstring::npos) // any_in_set, all_in_set, some_in_set require a file
        parm=util::absolute(parm,path);

      // store first condition
      ncond++;  wstring cid = rid+L":"+util::int2wstring(ncond);
      conds.push_back(fex_condition(cid,foc,fun,parm,set_files));
    
      // while other conditions are coming, read them
      wstring op1;
      while (sin>>op1) {
        if (not oper.empty() and op1!=oper) 
          WARNING(L"AND/OR alternation not allowed in fex rule condition"); 
        sin>>foc>>fun>>parm;
        if (fun==L"in_set") fun=L"any_in_set";  // default=any
        if (fun.find(L"in_set")!=wstring::npos) // any_in_set, all_in_set, some_in_set require a file
          parm=util::absolute(parm,path);

        ncond++;  wstring cid = rid+L":"+util::int2wstring(ncond);
        conds.push_back(fex_condition(cid,foc,fun,parm,set_files));
        oper=op1;
      }     
    }
    op = (oper==L"AND"? OP_AND : (oper==L"OR"? OP_OR : OP_NONE));
  }


  ////////////////////////////////////////////////////////////////
  /// Apply all rules in the pack once to each word in the sentence
  ////////////////////////////////////////////////////////////////

  void fex::precompute_once(const fex_rulepack &pack, sentence &sent) const {

    for (int nw=0; nw<(int)sent.size(); nw++) {
      //    TRACE(3,L"  Precomputing rules for word "+sent[nw].get_form());
      TRACE(3,L"  Precomputing rules for word ");
      TRACE(3,L"      "+sent[nw].get_form());
      for (list<fex_rule>::const_iterator r=pack.rules.begin(); r!=pack.rules.end(); r++) {
        TRACE(4,L"    Checking rule. True condition -> precompute once ");
        /// if all words match the condition, apply rule only once per word.
        /// the rule will store precomputed features for each word position nw
        r->precompute(sent,nw,*Tags);
      }
    }
  }


  ////////////////////////////////////////////////////////////////
  /// Apply all rules in the pack only to proper range around
  /// words matching pack condition
  ////////////////////////////////////////////////////////////////

  void fex::precompute_range(const fex_rulepack &pack, sentence &sent) const {

    fex_status * st = (fex_status *)sent.get_processing_status();

    for (int nw=0; nw<(int)sent.size(); nw++) {
      TRACE(3,L"  Precomputing rules for word: "+sent[nw].get_form());
      if (fex_rule::check_conds(pack.conds, pack.operation, sent[nw], *Tags, st)) {
        for (list<fex_rule>::const_iterator r=pack.rules.begin(); r!=pack.rules.end(); r++) {
          TRACE(4,L"    Checking rule. Condition matches -> precompute range ");
          /// not all words match, compute for all words in range.
          /// the rule will store precomputed features for each word position nw.
          /// the rule will avoid computing twice for the same word.
          int first = max((int)nw+r->get_left(), (int)0);
          int last= min((int)nw+r->get_right(), (int)sent.size()-1);
          for (int nw1=first; nw1<=last; nw1++)
            r->precompute(sent,nw1,*Tags);          
        }
      }
    }      
  }
} // namespace
