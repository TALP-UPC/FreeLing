/////////////////////////////////////////////////////////////////
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
#include <vector>

#include "freeling/morfo/dictionary.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"DICTIONARY"
#define MOD_TRACECODE DICT_TRACE


  ///////////////////////////////////////////////////////////////
  ///  Create a dictionary module, open database.
  ///////////////////////////////////////////////////////////////

  dictionary::dictionary(const std::wstring &Lang, const std::wstring &dicFile, 
                         const std::wstring &sufFile, const std::wstring &compFile,
                         bool invDic, bool retok) { 
    // remember whether dictionary must offer inverse access (lema#pos -> form)
    InverseDict = invDic;
    // remember whether contraction retokenization is to be performed
    RetokenizeContractions = retok;

    enum sections {INDEX, ENTRIES};
    config_file cfg;
    cfg.add_section(L"IndexType",INDEX);
    cfg.add_section(L"Entries",ENTRIES);

    if (not cfg.open(dicFile))
      ERROR_CRASH(L"Error opening file "+dicFile);
  
    //auxiliary to parse data lines
    list<pair<wstring,list<wstring> > > lems;  
    morfodb=NULL;
    inverdb=NULL;

    wstring line; 
    while (cfg.get_content_line(line)) {

      // process each content line according to the section where it is found
      switch (cfg.get_section()) {

      case INDEX: { // reading index type
        int type=-1;
        if (line==L"DB_PREFTREE") type=DB_PREFTREE;
        else if (line==L"DB_MAP") type=DB_MAP;
        else ERROR_CRASH(L"Invalid IndexType '"+line+L"' specified in dictionary file "+dicFile);
        // create database for dictionary entries
        morfodb = new database(type);
        // create inverse dict if needed
        if (InverseDict) inverdb=new database(DB_MAP);
        break;
      }

      case ENTRIES: { // reading an entry line
        if (morfodb==NULL) ERROR_CRASH(L"No IndexType specified in dictionary file "+dicFile);
        
        // split line in key+data
        wstring::size_type pos = line.find(L" ");
        wstring key=line.substr(0,pos);
        wstring data=line.substr(pos+1);      
        
        if (key.empty())
          ERROR_CRASH(L"Invalid format. Unexpected blank line in "+dicFile);
        
        // convert data string intro map lemma->list-of-tags
        lems.clear();
        if (not parse_dict_entry(data,lems))
          ERROR_CRASH(L"Invalid pair lemma-tag in dictionary line: "+key+L" "+data);
        
        // compact data in format "lema1 pos1a|pos1b|pos1c lema2 pos2a|pos2b" to save memory
        data = compact_data(lems);
        
        // add to database
        morfodb->add_database(key,data);
        
        // add info to inverse dict entry if needed.
        if (InverseDict)
          for (list<pair<wstring,list<wstring> > >::const_iterator p=lems.begin(); p!=lems.end(); p++)
            for (list<wstring>::const_iterator t=p->second.begin(); t!=p->second.end(); t++) 
              inverdb->add_database(p->first+L"#"+*t, key);      

        break;
      }

      default: break;
      }
    }

    cfg.close();

    // create affix analyzer if required
    suf = NULL;
    if (not sufFile.empty()) suf = new affixes(Lang, sufFile, *this);
    AffixAnalysis = (suf != NULL);

    // create compound analyzer if required
    #ifdef NO_LIBFOMA
      CompoundAnalysis = false;
    #else
      comp = NULL;
      if (not compFile.empty()) comp = new compounds(compFile, *this);
      CompoundAnalysis = (comp != NULL);
    #endif
  
    TRACE(3,L"analyzer succesfully created");
  }
  

  ////////////////////////////////////////////////
  /// Destroy dictionary module, close database.
  ////////////////////////////////////////////////

  dictionary::~dictionary(){
    // Close the database
    delete(morfodb);
    delete(inverdb);
    // delete affix analyzer, if any
    delete suf;
    #ifndef NO_LIBFOMA
      // delete compound analyzer, if any
      delete(comp);
    #endif
  }


  ////////////////////////////////////////////////////////////////
  /// parse data string into a map lemma->list of tags
  ////////////////////////////////////////////////////////////////

  bool dictionary::parse_dict_entry(const wstring &data, list<pair<wstring,list<wstring> > > &lems) const {

    list<wstring> lt;  // list of tags for current lemma
    list<wstring> ll;  // list of lemmas seen so far
    map<wstring,list<wstring> > aux; // map lemma->list of tags

    list<wstring> d=util::wstring2list(data,L" ");  // split data

    for (list<wstring>::iterator f=d.begin(); f!=d.end(); f++) {
      // get lemma
      list<wstring>::const_iterator lemma = f++;
      if (f==d.end()) return false; // mismatched pair, return error.
      // get tag
      list<wstring>::const_iterator tag = f;

      // add tag to list of tags for this lemma
      map<wstring,list<wstring> >::iterator p = aux.find(*lemma);
      if (p==aux.end()) {   // new lemma, create it
        lt.clear(); lt.push_back(*tag);
        aux.insert(make_pair(*lemma,lt));
        ll.push_back(*lemma);  // keep lemmas in a list, for later
      }
      else 
        // existing lemma, add tag
        p->second.push_back(*tag);
    }

    lems.clear();
    ll.sort();  // sort lemmas alphabetically

    for (list<wstring>::iterator k=ll.begin(); k!=ll.end(); k++) {
      // efficiently get list of tags for the lemma from map aux
      lems.push_back(make_pair(*k,list<wstring>()));  
      lems.back().second.swap(aux[*k]);
      // sort tags alphabetically
      lems.back().second.sort();
    }

    return true; // parsing worked, return ok.
  }


  ////////////////////////////////////////////////////////////////
  /// compact data in format lema1 pos1a|pos1b|pos1c lema2 pos2a|posb to save memory
  ////////////////////////////////////////////////////////////////

  /// auxiliary to compact_data
  const std::wstring TAG_DIVIDER = L"|";  
  const std::wstring LEMMA_DIVIDER = L" ";

  wstring dictionary::compact_data(const list<pair<wstring,list<wstring> > > &lems) const {  
    wstring cdata;
    for (list<pair<wstring,list<wstring> > >::const_iterator p=lems.begin(); p!=lems.end(); p++) {
      cdata = cdata + (p==lems.begin() ? L"" : LEMMA_DIVIDER)
        + p->first + LEMMA_DIVIDER + util::list2wstring(p->second,TAG_DIVIDER);
    }
    return cdata;
  }


  /////////////////////////////////////////////////////////////////////////////
  /// remove entry from dictionary
  /////////////////////////////////////////////////////////////////////////////

  void dictionary::remove_entry(const std::wstring &form) {

    // look for analysis in inverse dict and remove them.
    if (InverseDict) {    
      list<analysis> la;
      search_form(form,la);
      for (list<analysis>::iterator a=la.begin(); a!=la.end(); a++) {
        // get list of forms for this analysis
        wstring fms = inverdb->access_database(a->get_lemma()+L"#"+a->get_tag());
        list<wstring> lf = util::wstring2list(fms,L" ");
        // remove form from list
        lf.remove(form);
      
        if (lf.empty()) 
          // list is empty, remove entry
          inverdb->remove_database(a->get_lemma()+L"#"+a->get_tag());
        else
          // forms remain, store reduced list.  
          inverdb->replace_database(a->get_lemma()+L"#"+a->get_tag(), util::list2wstring(lf,L" "));
      }
    }

    // remove main entry
    morfodb->remove_database(form);
  }

  /////////////////////////////////////////////////////////////////////////////
  /// add analysis to dictionary entry (create entry if not there)
  /////////////////////////////////////////////////////////////////////////////

  void dictionary::add_analysis(const std::wstring &form, const analysis &newan) {

    // see if the form already exists
    wstring data = morfodb->access_database(form);

    if (data.empty()) { //new form, add it
      morfodb->add_database(form, newan.get_lemma()+L" "+newan.get_tag());
      // add pair to inverse dict if required
      if (InverseDict) 
        inverdb->add_database(newan.get_lemma()+L"#"+newan.get_tag(), form);
    }

    else { // known form. Add analysis to existing list.

      // check if lemma was already there
      list<wstring> lan=util::wstring2list(data,LEMMA_DIVIDER);
      wstring lemma,tags;
      bool l_found=false;
      bool t_found=false;
      for (list<wstring>::iterator lm=lan.begin(); lm!=lan.end() and not l_found; lm++) {
        lemma = (*lm); lm++; 
        tags = (*lm);
        if (lemma == newan.get_lemma()) {
          l_found = true;
          // lemma found, look for analysis.
          list<wstring> lt = util::wstring2list(*lm,TAG_DIVIDER);
          // check if tag was there too
          t_found=false;
          for (list<wstring>::iterator tg=lt.begin(); tg!=lt.end() and not t_found; tg++) 
            t_found = (*tg)==newan.get_tag();

          // lemma was there but tag wasn't: add tag and store new tag list
          if (not t_found) 
            (*lm) = (*lm)+TAG_DIVIDER+newan.get_tag();
        }
      }

      // lemma was not found: add pair (lemma,tag)
      if (not l_found) {
        lan.push_back(newan.get_lemma());
        lan.push_back(newan.get_tag());
      }

      // if we added either a new pair, or a new tag to an existing lemma, update database
      if (not l_found or not t_found) {
        // Store modified list of lemmas and tags
        morfodb->replace_database(form, util::list2wstring(lan,LEMMA_DIVIDER));
        // add pair to inverse dict if required
        if (InverseDict) 
          inverdb->add_database(newan.get_lemma()+L"#"+newan.get_tag(), form);
      }
    }
  }

  /// customize behaviour of dictionary for further analysis
  void dictionary::set_retokenize_contractions(bool rtk) { RetokenizeContractions = rtk; }
  /// customize behaviour of dictionary for further analysis
  void dictionary::set_affix_analysis(bool aff) { AffixAnalysis = aff; }
  /// customize behaviour of dictionary for further analysis
  void dictionary::set_compound_analysis(bool cmp) { CompoundAnalysis = cmp; }
  
  /// find out whether the dictionary has loaded an affix module
  bool dictionary::has_affixes() const { return (suf!=NULL); }

  /// find out whether the dictionary has loaded a compounds module
  bool dictionary::has_compounds() const { 
    #ifdef NO_LIBFOMA
      return false;
    #else
      return (comp!=NULL);
    #endif
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Get possible forms for a lemma+pos
  /////////////////////////////////////////////////////////////////////////////

  list<wstring> dictionary::get_forms(const wstring &lemma, const wstring &tag) const {
    list<wstring> r;

    if (InverseDict) 
      r = util::wstring2list(inverdb->access_database(lemma+L"#"+tag),L" ");
    else
      WARNING(L"get_forms called but dictionary was created with inverseDict=false."); 

    return r;
  }

  ////////////////////////////////////////////////////////////////
  /// Generate valid tag combinations for an ambiguous contraction
  ////////////////////////////////////////////////////////////////

  list<wstring> dictionary::tag_combinations(list<wstring>::const_iterator p, list<wstring>::const_iterator last) const {

    if (p==last) {
      // last element,return possible values
      return util::wstring2list(*p,L"/");
    }
    else {
      // possible tags for current element
      list<wstring> curr = util::wstring2list(*p,L"/");
      // recursive call to get possible tag combinations for following elements
      list<wstring> c = tag_combinations(++p,last);
      // extend obtained combinations with tags for current element
      list<wstring> res;
      for (list<wstring>::iterator i=curr.begin(); i!=curr.end(); i++)
        for (list<wstring>::iterator j=c.begin(); j!=c.end(); j++)
          res.push_back((*i)+L"+"+(*j));

      return res;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  ///  Search form in the dictionary, according to given options,
  ///  *Add* found analysis to the given list
  /////////////////////////////////////////////////////////////////////////////

  void dictionary::search_form(const std::wstring &s, std::list<analysis> &la) const {

    // lowercase the string
    wstring key = util::lowercase(s);

    // search word in the active dictionary  
    wstring data = morfodb->access_database(key);

    if (not data.empty()) {
      // process the data string into analysis list
      wstring::size_type p=0, q=0;
      while (p!=wstring::npos) {
        TRACE(3,L"word '"+s+L"'. remaining data: ["+data.substr(p)+L"]");
        // get lemma
        q=data.find_first_of(LEMMA_DIVIDER,p);
        wstring lem=data.substr(p,q-p);
        TRACE(4,L"   got lemma="+lem+L" p="+util::int2wstring(p)+L" q="+util::int2wstring(q));
        // get tags
        p=q+1;
        q=data.find_first_of(LEMMA_DIVIDER,p);
        list<wstring> tags=util::wstring2list(data.substr(p,q-p),TAG_DIVIDER);
        TRACE(4,L"   got tags="+util::list2wstring(tags,TAG_DIVIDER)+L" p="+util::int2wstring(p)+L" q="+(q==wstring::npos?L"npos":util::int2wstring(q)));
        analysis a;
        for (list<wstring>::iterator t=tags.begin(); t!=tags.end(); t++) {
          // insert analysis
          TRACE(3,L"Adding ("+lem+L","+*t+L") to analysis list");
          a.init(lem,*t);
          la.push_back(a);
        }
        // prepare next
        p= (q==wstring::npos? wstring::npos : q+1);
      }
    }
  }


  ////////////////////////////////////////////////////////////////////////
  /// Check whether the given word is a contraction, if so, obtain 
  /// composing words (and store them into lw).
  ////////////////////////////////////////////////////////////////////////

  bool dictionary::check_contracted(const wstring &form, wstring lem, 
                                    wstring tag, std::list<word> &lw) const {
    wstring cl;
    list<wstring> ct;
    list<analysis> la;
    size_t pl,pt;
    list<analysis>::const_iterator a;
    bool contr;

    TRACE(3,L"check contracted for word "<<form<<L" ("<<lem<<L","<<tag<<L")");

    // if it is a multiword or compound, will conflict with contraction checking: drop it.
    if ( lem.find_first_of(L"_")!=wstring::npos ) return false;

    // remember the capitalization pattern of the word, to keep the format in its parts
    int caps=util::capitalization(form); 
    
    contr=false;
    pl=lem.find_first_of(L"+");   pt=tag.find_first_of(L"+");

    while (pl!=wstring::npos && pt!=wstring::npos) {
      contr=true;    // at least one "+" found. it's a contraction

      // split contracted component out of "lem" and "tag" strings
      cl=lem.substr(0,pl);   ct=util::wstring2list(tag.substr(0,pt),L"/");
      lem=lem.substr(pl+1);  tag=tag.substr(pt+1);
      TRACE(3,L"Searching contraction component... "+cl+L"_"+util::list2wstring(ct,L"/"));

      // obtain analysis for contracted component, and keep analysis matching the given tag/s
      la.clear();
      search_form(cl,la);

      cl=util::capitalize(cl,caps,lw.empty());
      word c(cl);
      // add all matching analysis for the component
      for (a=la.begin(); a!=la.end(); a++) {
        for (list<wstring>::const_iterator t=ct.begin(); t!=ct.end(); t++) {
          if (a->get_tag().find(*t)==0 || (*t)==L"*") {
            c.add_analysis(*a); 
            TRACE(3,L"  Matching analysis: "+a->get_tag());
          }
        }
      }
      lw.push_back(c);

      if (c.get_n_analysis()==0) 
        ERROR_CRASH(L"Tag not found for contraction component. Check dictionary entries for '"+form+L"' and '"+cl+L"'");
  
      // look for next component
      pl=lem.find_first_of(L"+");  pt=tag.find_first_of(L"+");
    }

    // process last component (only if it was a contraction)
    if (contr) {

      cl=lem.substr(0,pl);   ct=util::wstring2list(tag.substr(0,pt),L"/");
      lem=lem.substr(pl+1); tag=tag.substr(pt+1);
      TRACE(3,L"Searching contraction component... "+cl+L"_"+util::list2wstring(ct,L"/"));

      list<analysis> la;
      search_form(cl,la);

      // create new word for the component, in the same format than the original
      if (caps==2)  // uppercase all
        cl=util::uppercase(cl);   
      else if (caps==1 and lw.empty()) // capitalize first letter of first word
        cl[0] = towupper(cl[0]);

      word c(cl);
      for (a=la.begin(); a!=la.end(); a++) {
        for (list<wstring>::const_iterator t=ct.begin(); t!=ct.end(); t++) {
          if (a->get_tag().find(*t)==0 || (*t)==L"*") {
            c.add_analysis(*a); 
            TRACE(3,L"  Matching analysis: "+a->get_tag());
          }
        }
      }
      lw.push_back(c);

      if (c.get_n_analysis()==0) 
        ERROR_CRASH(L"Tag not found for contraction component. Check dictionary entry for "+form);
    }

    return (contr);
  }


  /////////////////////////////////////////////////////////////////////////////
  ///  Search form in the dictionary.
  ///  *Add* found analysis to the given word.
  ///  Return true iff word is a contraction.
  /////////////////////////////////////////////////////////////////////////////

  bool dictionary::annotate_word(word &w, list<word> &lw,
				 dictionary::Option compounds,
				 dictionary::Option retok) const {

    if (retok==DEFAULT) retok = (RetokenizeContractions? ON : OFF);
    if (compounds==DEFAULT) compounds = (CompoundAnalysis? ON : OFF);
    
    ///////////// SEARCH IN DICTIONARY
    TRACE(3,L"Searching in dictionary, word: "+w.get_form());
    list<analysis> la;
    search_form(w.get_form(), la);

    // record this word was found in the dictionary
    if (la.size()>0) w.set_analyzed_by(word::DICTIONARY);
    TRACE(3,L"   Found "+util::int2wstring(la.size())+L" analysis.");

    for (list<analysis>::const_iterator a=la.begin(); a!=la.end(); a++) {
      w.add_analysis(*a);
      TRACE(4,L"   added analysis "+a->get_lemma());
    }

    ///////////// CHECK FOR AFFIXES
    if (AffixAnalysis) {
      // check whether the word is a derived form via suffixation
      TRACE(2,L"Affix analisys active. SEARCHING FOR AFFIX. word n_analysis="+util::int2wstring(w.get_n_analysis()));
      suf->look_for_affixes(w);
    }

    #ifndef NO_LIBFOMA
      ///////////// CHECK FOR COMPOUNDS
      if (compounds) {
        // check whether the word is a compound
        TRACE(2,L"Compound analisys active. SEARCHING FOR COMPOUND. word n_analysis="+util::int2wstring(w.get_n_analysis()));
	// if it is a compound, deactivate retokenization
        if (comp->check_compound(w)) {
	  retok = OFF;
	}
      }
    #endif

    ///////////// HANDLE CONTRACTION RETOKENIZATION, IF ANY
    bool contr=false;
    if (retok==OFF) {
      // RetokenizeContractions is OFF, or overriden for this call.
      // Just add retokenization information to each analysis, in case it is needed later.
      list<analysis> newla;
      analysis na;
      for (word::iterator a=w.begin(); a!=w.end(); a++) {
        // split contractoin tags, if any (e.g. PRP+MD/VB =>  PRP, MD/VB)
        list<wstring> tgs = util::wstring2list(a->get_tag(),L"+");
        // generate tag combinations for contractions (e.g. PRP+MD, PRP+VB).
        // Regular words get just 1 combination consisting of a single tag
        list<wstring> tc = tag_combinations(tgs.begin(),--tgs.end()); 

        if (tc.size()>1) {
          newla.clear();
          // if more than one combination, replace analysis with generated combinations
          for (list<wstring>::const_iterator tag = tc.begin(); tag!=tc.end(); tag++) {
            na.init(a->get_lemma(),*tag);
            newla.push_back(na);
          }
          // replace analysis with expanded combinations
          word::iterator p=w.erase(a);
          w.insert(p,newla.begin(),newla.end());
          // fix iterator and continue
          a=p; a--;
        }
      }

      // add retok information to analysis requiring it
      for (word::iterator a=w.begin(); a!=w.end(); a++) {
        lw.clear();
        if (check_contracted(w.get_form(),a->get_lemma(),a->get_tag(),lw)) {
          // The analysis is retokenizable, lw contains retokenization info.
          a->set_retokenizable(lw);
        }
      }	
    }
    else {
      // RetokenizeContractions is ON. Only first contracted
      // analysis is considered.  Any other is ignored with a warning.
    
      // find first contracted analysis, if any
      word::iterator ca = w.begin();
      while ( ca!=w.end() and (ca->get_lemma().find_first_of(L"+")==wstring::npos or
                               ca->get_tag().find_first_of(L"+")==wstring::npos) ) {
        ca++;
      }
    
      if (ca!=w.end() and w.get_n_analysis()>1) {
        // contraction found. If more analysis, issue a warning.
        WARNING(L"Contraction "<<w.get_form()<<L" has several analysis in dictionary. All ignored except ("
		<<ca->get_lemma()<<L","<<ca->get_tag()<<L"). Set RetokenizeContractions=false to keep all analysis.");
      }
      else 
        // one analysis, or no contractions (or no analysis)
        ca=w.begin();
    
      if (ca!=w.end() and check_contracted(w.get_form(),ca->get_lemma(),ca->get_tag(),lw)) 
        // The analysis is retokenizable, lw contains retokenization info.      
        contr = true;
    }

    return contr;
  }


  /////////////////////////////////////////////////////////////////////////////
  ///  Search form in the dictionary.
  ///  *Add* found analysis to the given word.
  ///  Do not apply compounds analysis,
  ///  do not retokenize contractions, nor return a component list.
  /////////////////////////////////////////////////////////////////////////////

  void dictionary::annotate_word(word &w) const {
    list<word> lw;
    annotate_word(w,lw,OFF,OFF);
  }

  ////////////////////////////////////////////////////////////////////////
  /// dump dictionary to a buffer. Either full entries or keys only
  ////////////////////////////////////////////////////////////////////////

  void dictionary::dump_dictionary(std::wostream &buff, bool keysonly) const {
    morfodb->dump_database(buff,keysonly);
  }


  ////////////////////////////////////////////////////////////////////////
  ///  Dictionary search and affix analysis for all words
  /// in a sentence, using given options.
  ////////////////////////////////////////////////////////////////////////

  void dictionary::analyze(sentence &se) const {
    sentence::iterator pos;

    bool contr=false;
    for (pos=se.begin(); pos!=se.end(); ++pos){
      // Process the word if it hasn't been annotated by previous modules,
      // except if annotated by "numbers" module, ("uno","one" are numbers 
      // but also pronouns, "one must do whatever must be done")
      if (!pos->get_n_analysis() || (pos->get_n_analysis() && pos->get_tag()[0]==L'Z')) {
        TRACE(1,L"Annotating word: "+pos->get_form());

        list<word> lw;
        if (annotate_word(*pos,lw)) { 
          // word is a contraction. Create new tokens, fix sentence.

          TRACE(2,L"Contraction found, replacing... "+pos->get_form()
                +L". span=("+util::int2wstring(pos->get_span_start())
                +L","+util::int2wstring(pos->get_span_finish())+L")");

          wstring worig = pos->get_form();
          int st=pos->get_span_start(); 
          int fin=pos->get_span_finish();

          list<word>::iterator i = lw.begin();
          int n = 0;
          if (worig.find(i->get_form()) == 0) {
            // first word is a prefix of the contraction, get its spans
            i->set_span(st,st + i->get_form().length());
            i->user=pos->user;          
            i->set_analyzed_by(word::DICTIONARY);
            TRACE(2,L"  Inserting "+i->get_form()+L". span=("+util::int2wstring(i->get_span_start())+L","+util::int2wstring(i->get_span_finish())+L")");
            pos = se.insert(pos,*i); 
            pos++;

            // new start for next word
            st = st + i->get_form().length();
            // remove prefix from original word
            worig = worig.substr(i->get_form().length());
            // move to second word
            ++i; ++n;
          }
          
          // distribute length among (remaining) number of words, making sure it is not zero
          int step=(fin-st)/(lw.size()-n); 
          step=max(1,step);
        
          while ( i!=lw.end() ) {
            // span end for curent token. Make sure last token span 
            // matches original token
            int f= (n==lw.size()-1 ? fin : st+step);
            // set span for current token.
            i->set_span(st,f);
            i->user=pos->user;
          
            i->set_analyzed_by(word::DICTIONARY);

            TRACE(2,L"  Inserting "+i->get_form()+L". span=("+util::int2wstring(i->get_span_start())+L","+util::int2wstring(i->get_span_finish())+L")");
            pos=se.insert(pos,*i); 
            pos++;
            st=st+step;
          
            contr=true;
            
            ++n;
            ++i;
          }
        
          TRACE(2,L"  Erasing "+pos->get_form());
          sentence::iterator q=pos; q--;  // save pos of previous word
          se.erase(pos);                  // erase contracted word
          pos=q;                          // fix iteration control
        }
      }
    }

    // if any word was a contraction, rebuild sentence index.
    if (contr) se.rebuild_word_index();
    TRACE_SENTENCE(1,se);
  }

} // namespace
