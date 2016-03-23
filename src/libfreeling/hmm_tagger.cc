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
#include <math.h>

#include "freeling/morfo/configfile.h"
#include "freeling/morfo/hmm_tagger.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"HMM_TAGGER"
#define MOD_TRACECODE TAGGER_TRACE

  //---------- Trellis Class  ----------------------------------

  ////////////////////////////////////////////////////////////////
  /// Constructor: Create dynammic storage for delta and phi 
  /// tables in the trellis
  ////////////////////////////////////////////////////////////////

  trellis::trellis(int T, unsigned int kb) {  
    kbest = kb;
    trl.reserve(T);
    trl = vector< map<bigram, path_elements> >(T, map <bigram, path_elements >());
  }

  ////////////////////////////////////////////////////////////////
  /// Destructor: Free dynammic storage for delta and phi tables 
  /// stored in the trellis.
  ////////////////////////////////////////////////////////////////

  trellis::~trellis() {}

  ////////////////////////////////////////////////////////////////
  /// insert a new arc in the trellis.
  /// Time t, state s, prev state sa, prob p
  ////////////////////////////////////////////////////////////////

  void trellis::insert(int t, const bigram &s, const bigram &sa, int kb, double p) {
    map<bigram, path_elements >::iterator i;

    i = trl[t].find(s);
    if (i==trl[t].end()) {
      TRACE(4,L"        Inserting. Is a new element, init list.");
      // new element, just add it.
      path_elements m;
      m.insert(element(sa,kb,p));
      trl[t].insert(make_pair(s,m));
    }
    else { // existing element
      TRACE(4,L"        Inserting. Not a new element, add. List size="+util::int2wstring(i->second.size())+L"/"+util::int2wstring(kbest));

      // access last element in list (worse probability stored)
      path_elements::iterator j=i->second.end(); j--; 
      // if set is full and we don't improve the worse, don't bother trying.
      if (i->second.size()==kbest and p<j->prob)  {
        TRACE(4,L"        Not worth inserting");
        return; 
      }

      // set not full, or element worth storing: extend ordered set appropiately
      i->second.insert(element(sa,kb,p));
      // if kbest was full, erase worse element
      if (i->second.size()>kbest) {
        i->second.erase(--(i->second.end()));
        TRACE(4,L"        list too long. Last erased.");
      }
    }
  }

  ////////////////////////////////////////////////////////////////
  /// Get delta component for kth best path (count from 0)
  ////////////////////////////////////////////////////////////////

  double trellis::delta(int t, const bigram &s, unsigned int k) const {

    if (k>kbest-1) {
      ERROR_CRASH(L"Requested k-best path index is larger than number of stored paths.");
    }

    map<bigram, path_elements >::const_iterator i;
    double res;
    i=trl[t].find(s);
    if (i==trl[t].end()) 
      // not there, return zero prob
      res= ZERO_logprob;
    else {
      // return kth element in the set
      path_elements::const_iterator j;
      unsigned int n=0;
      j=i->second.begin();
      while (n<k) { j++; n++; }
      res = j->prob;
    }
    return res;
  }


  ////////////////////////////////////////////////////////////////
  /// Get phi component for kth best path (count from 0)
  ////////////////////////////////////////////////////////////////

  pair<bigram, int> trellis::phi(int t, const bigram &s, unsigned int k) const {

    if (k>kbest-1) {
      ERROR_CRASH(L"Requested k-best path index is larger than number of stored paths.");
    }

    // return kth element in the set associated to state s at time t
    path_elements::const_iterator j;
    unsigned int n=0;
    j = trl[t].find(s)->second.begin();
    while (n<k) { 
      j++; n++; 
    }

    return make_pair(j->state,j->kbest);
  }

  ////////////////////////////////////////////////////////////////
  /// Get number of elements in kbest set for time t, state s
  ////////////////////////////////////////////////////////////////

  int trellis::nbest(int t, const bigram &s) const {
    return trl[t].find(s)->second.size();
  }

  //////////////////  static components /////////////////

  const float trellis::ZERO_logprob = -numeric_limits<float>::infinity();
  const bigram trellis::initState = bigram(L"0",L"0");
  const bigram trellis::endState = bigram(L"ENDSTATE",L"ENDSTATE");

  /// Comparisons to sort kbest paths by probability
  bool trellis::element::operator<(const element &e) const {return(this->prob<e.prob);}
  bool trellis::element::operator>(const element &e) const {return(this->prob>e.prob);}
  /// Comparison (to please MSVC)
  bool trellis::element::operator==(const element &e) const {return(this->prob==e.prob);}

  /// trellis element constructor
  trellis::element::element(const bigram &s, int k, double p): state(s), kbest(k), prob(p) {}
  /// trellis element destructor
  trellis::element::~element() {}


  //---------- HMMTagger Class  ----------------------------------

  const std::wstring UNOBS_INITIAL_STATE = L"0.x";
  const std::wstring UNOBS_WORD = L"<UNOBSERVED_WORD>";

  ///////////////////////////////////////////////////////////////
  ///  Constructor: Build a HMM tagger, loading probability tables.
  ///////////////////////////////////////////////////////////////

  hmm_tagger::hmm_tagger(const std::wstring &hmmFile, bool rtk, unsigned int force, unsigned int kb) : POS_tagger(rtk,force), probInitial(-1.0), probUnobserved(-1.0) {

    double prob, coef;
    wstring nom1,aux,ftags;

    pA_cache = new safe_map<wstring,double>();
    // pB_cache = new prob_cache(); // see comments below in ProbB_log

    wstring path=hmmFile.substr(0,hmmFile.find_last_of(L"/\\")+1);
    Tags = NULL;
    kbest = kb;

    enum sections {UNIGRAM,BIGRAM,TRIGRAM,INITIAL,WORD,SMOOTHING,FORBIDDEN,TAGSET};

    config_file cfg;  
    cfg.add_section(L"Tag",UNIGRAM); 
    cfg.add_section(L"Bigram",BIGRAM);
    cfg.add_section(L"Trigram",TRIGRAM);
    cfg.add_section(L"Initial",INITIAL);
    cfg.add_section(L"Word",WORD); 
    cfg.add_section(L"Smoothing",SMOOTHING);
    cfg.add_section(L"Forbidden",FORBIDDEN);
    cfg.add_section(L"TagsetFile",TAGSET);

    if (not cfg.open(hmmFile)) {
      ERROR_CRASH(L"Error opening file "+hmmFile);
    }

    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;
      sin.str(line);
    
      // process each line according to the section where it is located
      switch (cfg.get_section()) {
      case UNIGRAM: {
        // Reading tag probabilities
        sin>>nom1>>prob;
        PTag.insert(make_pair(nom1 ,prob));
        break;
      }

      case BIGRAM: {
        // Reading bigram probabilities
        sin>>nom1>>prob;
        vector<wstring> bg = util::wstring2vector(nom1,L".");
        PBg.insert(make_pair(bigram(bg[0],bg[1]), prob));
        break;
      }

      case TRIGRAM: {
        // Reading trigram probabilities
        sin>>nom1>>prob;
        PTrg.insert(make_pair(nom1,prob));
        break;
      }

      case INITIAL: {
        // Reading initial probabilities
        sin>>nom1>>prob;
        if (nom1 == UNOBS_INITIAL_STATE) probInitial = prob;
        else {
          vector<wstring> bg = util::wstring2vector(nom1,L".");
          PInitial.insert(make_pair(bigram(bg[0],bg[1]), prob));
        }
        break;
      }

      case WORD: {
        // Reading word probabilities
        sin>>nom1>>prob;
        if (nom1 == UNOBS_WORD) probUnobserved = prob;
        else PWord.insert(make_pair(nom1,prob));
        break;
      }

      case SMOOTHING: {
        // Reading the coeficients
        sin>>nom1>>coef;
        if (nom1==L"c1") c[0]=coef;
        else if (nom1==L"c2") c[1]=coef;
        else if (nom1==L"c3") c[2]=coef;
        break;
      }

      case FORBIDDEN: {
        if (Tags==NULL) {
          ERROR_CRASH(L"<TagsetFile> section should appear before <Forbidden> in file '"+hmmFile+L"'");
        }

        // Reading forbidden transitions
        sin>>aux;
        bool err=false;
        vector<wstring> l(3,L"");
        
        TRACE(4,L" reading forbidden ("+aux+L")");
        vector<wstring> ltg=util::wstring2vector(aux,L".");
        for (int i=0; i<3; i++) {
          TRACE(4,L"    ... processing  ("+ltg[i]+L")");
          size_t p=ltg[i].find(L"<"); 
          if (p!=wstring::npos) {
            l[i]=ltg[i].substr(p);
            ltg[i]=ltg[i].substr(0,p);
          }
          // it is illegal to use a lema with * or 0, and
          // to use * or 0 out of first position
          err=((not l[i].empty() or i!=0) and (ltg[i]==L"*" or ltg[i]==L"0"));
        }
        
        if (err) {
          WARNING(L"Wrong format for forbidden trigram '"+aux+L"'. Ignored.");
        }
        else {
          
          vector<wstring> stg;
          for (int i=0; i<3; i++) {     
            if (ltg[i]==L"*" or ltg[i]==L"0") {
              stg.push_back(ltg[i]);
              ltg[i]= L"";
            }
            else {
              stg.push_back(Tags->get_short_tag(ltg[i]));
              if (stg[i]==ltg[i]) ltg[i]=L"";
            }
          }
          
          wstring tr=util::vector2wstring(stg,L".");
          wstring lt=util::vector2wstring(ltg,L".");
          wstring lm=util::vector2wstring(l,L".");
          
          wstring s=L""; 
          if (lt!=L".." or lm!=L"..") s=lm+L"#"+lt;
          
          TRACE(4,L"Inserting forbidden ("+tr+L","+s+L")");
          Forbidden.insert(make_pair(tr,s));
        }
        break;
      }

      case TAGSET: {
        // reading Tagset description file name
        sin>>ftags;
        // load tagset description
        TRACE(3,L"Loading tagset file "+ util::absolute(ftags,path));      
        Tags = new tagset(util::absolute(ftags,path));
        break;
      }

      default: break;
      }
    }
    cfg.close();

    if (probInitial == -1.0 or probUnobserved == -1.0) {
      ERROR_CRASH(L"HMM model missing '"+UNOBS_INITIAL_STATE+L"' and/or '"+UNOBS_WORD+L"' entries");
    }

    TRACE(3,L"analyzer succesfully created");
  }


  ////////////////////////////////////////////////
  /// Destructor
  ////////////////////////////////////////////////

  hmm_tagger::~hmm_tagger() {
    delete Tags;
    delete pA_cache;
    //delete pB_cache;
  }

  ////////////////////////////////////////////////
  /// check if a trigram is in the forbidden list.
  ////////////////////////////////////////////////

  bool hmm_tagger::is_forbidden(const wstring &trig, sentence::const_iterator w) const {

    if (Forbidden.empty()) return false;

    bool fbd=false;

    pair<multimap<wstring,wstring>::const_iterator,multimap<wstring,wstring>::const_iterator> range;
    range = Forbidden.equal_range(trig);

    // check all possible rules for that trigram, if any.
    for (multimap<wstring,wstring>::const_iterator t=range.first;
         t!=range.second and not fbd;
         t++) { 

      TRACE(4,L"       Checking rule for forbidden trigram "+t->first+L":"+t->second);
      if (t->second.empty())  // no lemmas, the trigram is always forbidden
        fbd=true;

      else  { 
        // if we get here, the trigram is forbidden only 
        // for some specific lemmas. Check if it's the case.

        vector<wstring> stags=util::wstring2vector(t->first,L".");
        vector<wstring> vaux=util::wstring2vector(t->second,L"#");
        vector<wstring> lems=util::wstring2vector(vaux[0],L".");
        vector<wstring> ltags=util::wstring2vector(vaux[1],L".");

        TRACE(4,L"       check rule: stags=["+util::vector2wstring(stags,L",")+L"] lems=["+util::vector2wstring(lems,L",")+L"] ltags=["+util::vector2wstring(ltags,L",")+L"]");

        sentence::const_iterator wd = w;
        fbd=true; int i=3;
        while (i>0 and fbd) {
          i--;

          // if more detail than short tags is required, look for 
          // the pair tag-lemma in the corresponding word.
          if (not (lems[i].empty() and ltags[i].empty())) {

            fbd=false;
            for (word::const_iterator an=wd->begin(); an!=wd->end() and not fbd; an++) {
              if (not ltags[i].empty() and not lems[i].empty()) {
                // we have both lemma and long tag
                fbd = (ltags[i]==an->get_tag()) and (lems[i]==L"<"+an->get_lemma()+L">");
                TRACE(4,L"       ... checking "+ltags[i]+L","+lems[i]+L": "+(fbd?L"forbidden":L"allowed"));
              }
              else if (ltags[i].empty()) {  // we have lemma, but not long tag.
                fbd = (stags[i]==Tags->get_short_tag(an->get_tag())) and 
                  (lems[i]==L"<"+an->get_lemma()+L">");
                TRACE(4,L"       ... checking "+lems[i]+L" (no ltag): "+(fbd?L"forbidden":L"allowed"));
              }
              else { // we have long tag, but not lemma
                fbd = (ltags[i]==an->get_tag());
                TRACE(4,L"       ... checking "+ltags[i]+L" (no lemm): "+(fbd?L"forbidden":L"allowed"));
              }
            }      
          }
        
          // check position 0 only if there is a lemma to check
          // (i.e. prevent checking for wildcards at sentence beggining)
          if (i>1 or not lems[0].empty()) wd--;
        }
      }
    }

    TRACE(5,L"       ...trigram "+trig+L" is "+(fbd?L"forbidden.":L"not forbidden."));
    return fbd;
  }


  ////////////////////////////////////////////////
  ///  Compute transition log_probability from state_i to state_j,
  ///  returning appropriate smoothed values if no evidence is available.
  ///  If the trigram is in the "forbidden" list, result is probability zero.
  ////////////////////////////////////////////////

  double hmm_tagger::ProbA_log(const bigram &state_i, const bigram &state_j, sentence::const_iterator w) const {
    map <wstring, double>::const_iterator k;
    map <bigram, double>::const_iterator b;
    wstring t3, t2t3, t1t2t3; 

  
    // state_i=t1.t2 --  state_j=t2.t3  
    t3=state_j.second; 
    t2t3 = state_j.first+L"."+state_j.second;
    t1t2t3 = state_i.first+L"." + state_i.second +L"." + t3;

    // if it's a forbidden transition, return zero probability for the transition
    if (is_forbidden(L"*."+t2t3, w) or is_forbidden(t1t2t3,w)) 
      return log(0.0);
    
    // check cache for already computed probs.
    double d=0;
    if (pA_cache->find_safe(t1t2t3,d)) {
      TRACE(5, L"      cached pa("+t1t2t3+L")= "+util::double2wstring(d));      
      return d;
    }
    
    // compute probability using linear interpolation
    double prob=0;

    k=PTag.find(t3);
    if (k!=PTag.end())    // Tag found in map
      prob+=c[0]*(k->second);
    else {     // unobserved tag look for a generic one
      k=PTag.find(L"x");
      prob+=c[0]*(k->second);  
    }
    
    b=PBg.find(state_j);
    if (b!=PBg.end())      // Bigram found in map
      prob+=c[1]*(b->second);
    
    k=PTrg.find(t1t2t3);
    if (k!=PTrg.end())    // Trigram found in map
      prob+=c[2]*(k->second);  
 
    prob=log(prob);
    // cache already computed P(t3|t1t2) to speed further process.  
    pA_cache->insert_safe(t1t2t3,prob);
    
    return prob;
  }


  ///////////////////////////////////////////////////////////
  /// Compute emission log_probability for observation obs
  /// from state_i.
  ///   Pb=P(word|state)=P(state|word)*P(word)/P(state)
  ///   Since states are bigrams: s=t1.t2
  ///     - we approximate P(s)~=P(t2)
  ///     - we approximate P(s|w)~=P(t2|w) 
  ///   Thus: Pb ~= P(t2|w)*P(w)/P(t2)
  ///////////////////////////////////////////////////////////

  double hmm_tagger::ProbB_log(const bigram &state_i, const word & obs) const {
    double pb_log ,plog_word_tag, plog_word, plog_st; 
    wstring tag2;
    word::const_iterator a;
    map <wstring, double>::const_iterator k;

    // second tag in state_i (states are bigrams t1.t2)
    tag2=state_i.second;

    // check cache for already computed P(w|t2)

    /* The use of a cache must be reevaluated, since it introduces unwanted biasses 
       when a word appears e.g. as NP early in the text (only one tag), then the tag NP
       is artificially favoured when the same word appears later with more than one tag

       wstring key=obs.get_lc_form()+L"|"+tag2;
       map<wstring,double>::const_iterator p=pB_cache.find(key);
       if (p!=pB_cache.end()) {
       TRACE(5, L"      cached pb("+key+L")= "+util::double2wstring(p->second));
       return p->second;
       }    
    */
    // get observed word probability
    k=PWord.find(obs.get_lc_form());
    // unobserved word, backoff probability
    if(k==PWord.end())
      plog_word = probUnobserved;
    else
      plog_word=k->second;

    // get tag t2 probability
    k=PTag.find(tag2);
    // unobserved tag
    if (k==PTag.end()) k=PTag.find(L"x");
    plog_st=log(k->second);  // P(s) ~= P(t2)

    // We need P(t2|w). Add prob for any matching tag
    double pa=0;
    for(a=obs.begin(); a!=obs.end(); a++) {
      if (Tags->get_short_tag(a->get_tag())==tag2) 
        pa+=a->get_prob();
    }
    plog_word_tag=log(pa);

    // Compute pb  Pb ~= P(t2|w)*P(w)/P(t2)
    pb_log = plog_word_tag + plog_word - plog_st;

    TRACE(5, L"      plog_word_tag="+util::double2wstring(plog_word_tag));
    TRACE(5, L"      plog_word= "+util::double2wstring(plog_word));
    TRACE(5, L"      plog_st= "+util::double2wstring(plog_st));
    TRACE(5, L"      pb= "+util::double2wstring(pb_log));

    // cache already computed P(w|t2) to speed further process  
    /*  pB_cache.insert(make_pair(key,pb_log)); */

    return pb_log;
  }

  
  ///////////////////////////////////////////////////////////
  /// Compute initial log_probability for  state_i.
  ///////////////////////////////////////////////////////////

  double hmm_tagger::ProbPi_log(const bigram &state_i) const {
    double ppi_log;
    map <bigram, double>::const_iterator k;

    // Initial state probability
    k=PInitial.find(state_i);
    if (k!=PInitial.end())
      ppi_log=(*k).second;
    else if (state_i.first == L"0") { // Unobserved (but possible) initial state    
      ppi_log = probInitial;
    }
    else { // non-initial state, zero probability, but log(0) = -inf,     
      ppi_log = trellis::ZERO_logprob;
    }

    return ppi_log;
  }


  ///////////////////////////////////////////////////////////
  /// Given an *annotated* sentence, compute sequence 
  /// (log) probability according to HMM parameters.
  ///////////////////////////////////////////////////////////

  double hmm_tagger::SequenceProb_log(const sentence &se, int k) const {
    double p=0;
    wstring tag, nexttag;
    bigram st, nextst;

    // iterate through sentence words
    sentence::const_iterator w=se.begin();

    // initial state probability
    tag = Tags->get_short_tag(w->get_tag(k));
    st = bigram(L"0", tag);
    p = ProbPi_log(st);
    // emmission of first word
    p += ProbB_log(st, *w);

    // second word
    w++;
    while (w!=se.end()) {
      // next tag, next state
      nexttag = Tags->get_short_tag(w->get_tag(k));
      nextst = bigram(tag,nexttag);
      // transition
      p += ProbA_log(st, nextst, w);
      // emission
      p += ProbB_log(nextst, *w);
      // move to next word/state
      tag = nexttag;
      st = nextst; 
      w++; 
    }

    return p;  
  }

  ///////////////////////////////////////////////////////////////  
  ///  Disambiguate given sentences with provided options  
  ///////////////////////////////////////////////////////////////  

  void hmm_tagger::annotate(sentence &se) const {
    list<emission_states> lemm;
    list<emission_states>::iterator emms,emmsant;
    emission_states::iterator k,kant;
    sentence::iterator w;
    word::iterator ka;
    double max=0, aux=0, emm=0, pi=0;  
    map<wstring,double>::iterator kd;
    map<wstring,wstring>::iterator ks;
    wstring tag;
    int t;
  
    TRACE(3,L"Analyze one sentence using Viterbi algorithm");
  
    // create tables to disambiguate current sentece
    trellis tr(se.size()+1,kbest);
  
    // Compute possible emission states for each word
    lemm=FindStates(se);
  
    // initialitation (first observation in sequence)
    w=se.begin();
    TRACE(3,L"probability for initial word "+w->get_form());
    emms=lemm.begin();
    for (k=emms->begin(); k!=emms->end(); k++) {
      pi=ProbPi_log(*k); 
      emm=ProbB_log(*k,*w);
      aux = pi+emm;
      TRACE(3,L"    Pi prob for <"+k->first+L", "+k->second+L">="+util::double2wstring(pi)+L";  emm prob="+util::double2wstring(emm)+L";  total="+util::double2wstring(aux));
      tr.insert(0,*k,tr.initState,0,aux);
    }
  
    // compute best path
    TRACE(3,L"Computing best path");
    t=1;
    emmsant=lemm.begin();  emms=++lemm.begin(); 
    for (w=++se.begin(); w!=se.end(); w++) {
    
      TRACE(3,L" Examining word "+w->get_form());
      for(k=emms->begin(); k!=emms->end(); k++) {

        // emmission prob for current word w from state being checked (k).
        emm = ProbB_log(*k,*w);
      
        // Check all possible transitions, remember best path.
        TRACE(3,L"  -- Checking transition to state <"+k->first+L", "+k->second+L">.  Emmission P("+w->get_form()+L"|"+k->first+L","+k->second+L")="+util::double2wstring(emm));
        for (kant=emmsant->begin(); kant!=emmsant->end(); kant++) {
          // Ignore nonsense transitions (i.e. check transition A.B->B.C but not A.B->C.D)
          if (kant->second == k->first) {

            for (int kb=0; kb<tr.nbest(t-1,*kant); kb++) {
              double pant = tr.delta(t-1,*kant,kb);     // best of previous state
              double ptrans = ProbA_log(*kant,*k,w); // transition
              aux = pant + ptrans + emm;             // add best_prev + transition + emmission
            
              TRACE(3,L"       Possible path from "+kant->first+L","+kant->second+L". Best.ant="+util::double2wstring(pant)+L", Ptrans="+util::double2wstring(ptrans)+L", Total path prob="+util::double2wstring(aux)); 
            
              // store path *iff* it is among the k-best
              tr.insert(t,*k,*kant,kb,aux);
            }
          }
        }
      }
    
      t++;      
      emmsant=emms;
      emms++;
    }
  
    // Termination state, last word.
    max=trellis::ZERO_logprob;
    w=--se.end();
    emms=--lemm.end();
    for(k=emms->begin(); k!=emms->end(); k++) {    
      for (int kb=0; kb<tr.nbest(se.size()-1,*k); kb++) {
        aux=tr.delta(se.size()-1,*k,kb);
        TRACE(4, L"       Final delta for "+k->first+L","+k->second+L" is "+util::double2wstring(aux));
        tr.insert(se.size(), tr.endState,*k,kb,aux);
      }
    }

    // erase any previous selection 
    for (w=se.begin(); w!=se.end(); w++) 
      w->unselect_all_analysis();
    
    for (int bp=0; bp<tr.nbest(se.size(), tr.endState); bp++) {

      pair<bigram, int> back=tr.phi(se.size(), tr.endState, bp);
      bigram st=back.first;
      int kb=back.second;

      TRACE(3, L"Recovering best path "+util::int2wstring(bp)+
            L", last state="+st.first+L","+st.second+L":"+util::int2wstring(kb));

      tag=st.second; //last tag of the st
      //(most likely tag for the last word)
      w=--se.end();
      for (t=se.size()-1; t>=0; t--) {  
      
        // get the tags with highest prob among those possible
        list<word::iterator> bestk;
        max=0.0;
        //TRACE(3, L"Word: "+w->get_form()+L" with tag "+tag+L"  state="+st);
        for (ka=w->begin();  ka!=w->end();  ka++) {
          TRACE(3, L"   Checking analysis: "+ka->get_lemma()+L" "+ka->get_tag());
          if (Tags->get_short_tag(ka->get_tag())==tag) {
            // if there are more than one matching tag, pick only 
            // those with highest lexical probability.
            if (ka->get_prob()>max) {
              TRACE(3, L"        ** selected! (new max)");
              max=ka->get_prob();
              bestk.clear();
              bestk.push_back(ka);
            }
            else if (ka->get_prob()==max) {
              TRACE(3, L"        ** selected (added)");
              bestk.push_back(ka);
            }
          }
        }
    
        // mark them as selected analysis for that word
        for (list<word::iterator>::iterator k=bestk.begin(); k!=bestk.end(); k++) 
          w->select_analysis(*k,bp);
      
        // backtrack one more step if possible
        if (t>0) {
          back=tr.phi(t,st,kb);
          st=back.first;
          kb=back.second;

          tag = st.second;
          w--;
        }
      }
    }
  
    TRACE(3,L"sentence analyzed"); 
  }


  ///////////////////////////////////////////////////////////////  
  ///  Obtain a list with the states that *may* have emmited 
  ///  current observation (a sentence).
  ///////////////////////////////////////////////////////////////  

  list<emission_states> hmm_tagger::FindStates(const sentence & sent) const {

    emission_states st; //list with the states that may have emmited two consecutive words
    list<emission_states> ls;
    sentence::const_iterator w1,w2;
    word::const_iterator a1,a2;

    // note that we only consider *selected* analysis for each word, which
    // may be all if the previous step was a morpho analyzer, or just a few 
    // if some kind of predesambiguation has been performed.

    // deal with first word
    w2=sent.begin();
    TRACE(3,L"obtaining the states that may have emmited the initial word: "+w2->get_form());
    for (a2=w2->selected_begin(); a2!=w2->selected_end(); a2++) 
      st.insert(bigram(L"0", Tags->get_short_tag(a2->get_tag())));
    ls.push_back(st);

    // step to second word
    w1=w2; w2++;

    // deal with each word in sentence (from 2nd to last)
    for (; w2!=sent.end();  w1=w2, w2++) {
      // compute list of possible trigrams according to two previous words.
      TRACE(3,L"obtaining the states that may have emmited the word: "+w2->get_form());
      st.clear();

      for (a1=w1->selected_begin(); a1!=w1->selected_end(); a1++)
        for (a2=w2->selected_begin(); a2!=w2->selected_end(); a2++)
          st.insert(bigram(Tags->get_short_tag(a1->get_tag()), Tags->get_short_tag(a2->get_tag())));

      // add list for current word to global list of lists.
      ls.push_back(st);
    }

    return ls;
  }


} // namespace
