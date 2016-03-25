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

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/semdb.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"SEMDB"
#define MOD_TRACECODE SENSES_TRACE


  ///////////////////////////////////////////////////////////////
  ///  Constructor of the auxiliary class "sense_info"
  ///////////////////////////////////////////////////////////////

  sense_info::sense_info(const wstring &syn, const wstring &data) {

    // store sense identifier
    sense=syn; 

    if (not data.empty()) {
      // split data string into fields
      list<wstring> fields=util::wstring2list(data,L" ");
    
      // iterate over field list and store each appropriately
      list<wstring>::iterator f=fields.begin();
    
      //first field: list of hypernyms
      if ((*f)!=L"-") parents=util::wstring2list(*f,L":");   
      // second field: WN semantic file
      f++; semfile=(*f);   
      // third filed: List of EWN top-ontology labels  
      f++; if ((*f)!=L"-") tonto=util::wstring2list(*f,L":");
      // fourth field: SUMO concept
      f++; if ((*f)!=L"-") sumo=*f;
      // fifth field: OpenCYC concept
      f++; if ((*f)!=L"-") cyc=*f;
    }
  }


  ///////////////////////////////////////////////////////////////
  ///  Obtain parent list as string (useful for Java API
  ///////////////////////////////////////////////////////////////

  wstring sense_info::get_parents_string() const {
    return util::list2wstring(parents,L":");
  }

  ///////////////////////////////////////////////////////////////
  ///  Create the sense annotator
  ///////////////////////////////////////////////////////////////

  semanticDB::semanticDB(const std::wstring &wsdFile) {

    wstring formFile,dictFile,wnFile;
    wstring path=wsdFile.substr(0,wsdFile.find_last_of(L"/\\")+1);

    // store PoS appearing in mapping to select relevant dictionary entries later.
    set<wstring> posset;

    enum sections {WN_POS_MAP, DATA_FILES};
    config_file cfg(true);  
    cfg.add_section(L"WNposMap",WN_POS_MAP);
    cfg.add_section(L"DataFiles",DATA_FILES);

    if (not cfg.open(wsdFile))
      ERROR_CRASH(L"Error opening configuration file "+wsdFile);

    // read disambiguator configuration file 
    wstring line; 
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) {
      
      case WN_POS_MAP: {   // reading map freeling PoS -> WNPoS lemma
        wistringstream sin;
        sin.str(line);
        posmaprule r;   // read and store a posmap rule, eg: "VMP v (VMP00SM)"  or  "N n L"
        sin>>r.pos>>r.wnpos>>r.lemma;
        posmap.push_back(r); 
        if (r.lemma!=L"L" and r.lemma!=L"F") 
          posset.insert(r.lemma);
        break;
      }
 
      case DATA_FILES: {  // reading names for data files
        wistringstream sin;
        sin.str(line);
        wstring key; wstring fname;
        sin>>key>>fname;
        if (key==L"formDictFile") formFile=util::absolute(fname,path); 
        else if (key==L"senseDictFile") dictFile=util::absolute(fname,path); 
        else if (key==L"wnFile") wnFile=util::absolute(fname,path); 
        break;
      }

      default: break;
      }
    }
    cfg.close();

    // load the necessary entries from the form dictionary,
    // store them reversed for access (lema,PoS)->form
    if (formFile.empty() or posset.size()==0) 
      form_dict=NULL;
    else {
      wifstream fform;
      util::open_utf8_file(fform, formFile);
      if (fform.fail()) ERROR_CRASH(L"Error opening formDict file "+formFile); 
    
      form_dict = new database(DB_MAP);
      while (getline(fform,line)) {
        wistringstream sin; sin.str(line);
        // get form, 1st field
        wstring form;
        sin>>form;
        // get pairs lemma-pos and select those relevant
        wstring lema,tag; 
        while (sin>>lema>>tag) {
          if (posset.find(tag)!=posset.end())
            form_dict->add_database(lema+L" "+tag,form);
        }
      }
      fform.close();
    }

    // load semanticDB if needed.
    if (dictFile.empty()) 
      sensesdb=NULL;
    else {
      wifstream fsens;
      util::open_utf8_file(fsens, dictFile);
      if (fsens.fail()) ERROR_CRASH(L"Error opening senseDict file "+dictFile); 
    
      sensesdb = new database(DB_MAP);
      while (getline(fsens,line)) {
        wistringstream sin; sin.str(line);
        // get sense, 1st field
        wstring sens;
        sin>>sens;
        wstring tag= sens.substr(sens.find(L"-")+1);
        // get words for current sense. Store both sense->words and word->sense records
        wstring wd; 
        while (sin>>wd) {
          sensesdb->add_database(L"S:"+sens,wd);
          sensesdb->add_database(L"W:"+wd+L":"+tag, sens);
        }
      }
      fsens.close();
    }

    // load WN structure if needed
    if (wnFile.empty()) 
      wndb=NULL;    
    else
      wndb =  new database(wnFile) ;

  }

  ////////////////////////////////////////////////
  /// Destroy sense annotator module, close database.
  ////////////////////////////////////////////////

  semanticDB::~semanticDB() {
    delete sensesdb;
    delete wndb;
    delete form_dict;
  }


  ///////////////////////////////////////////////////////////////
  ///  Get senses for a lemma+pos
  ///////////////////////////////////////////////////////////////  

  list<wstring> semanticDB::get_word_senses(const wstring &form, const wstring &lemma, const wstring &pos) const {

    // get pairs (lemma,pos) to search in WN for this analysis
    list<pair<wstring,wstring> > searchlist;
    get_WN_keys(form, lemma, pos, searchlist);
  
    // search senses for each lemma/tag in the list
    list<wstring> lsen;
    list<pair<wstring,wstring> >::iterator p;
    for (p=searchlist.begin(); p!=searchlist.end(); p++) {
      TRACE(4,L" .. searching "+p->first+L" "+p->second);
      list<wstring> s = util::wstring2list(sensesdb->access_database(L"W:"+p->first+L":"+p->second), L" ");
      lsen.insert(lsen.end(),s.begin(),s.end());
    }
    if (not lsen.empty()) {
      TRACE(4,L" .. senses found: "+util::list2wstring(lsen,L"/"));
    }

    return lsen;
  }


  ///////////////////////////////////////////////////////////////
  ///  Get synonyms for a sense+pos
  ///////////////////////////////////////////////////////////////  

  list<wstring> semanticDB::get_sense_words(const wstring &sens) const{
    return util::wstring2list(sensesdb->access_database(L"S:"+sens), L" ");
  }


  ///////////////////////////////////////////////////////////////
  ///  Get info for a sense+pos
  ///////////////////////////////////////////////////////////////  

  sense_info semanticDB::get_sense_info(const wstring &syn) const{
    /// access DB and split obtained data_string into fields
    /// and store appropriately in a sense_info object
    sense_info sinf(syn,wndb->access_database(syn));
    /// add also list of synset words
    sinf.words=get_sense_words(syn);
    return sinf;
  }


  //////////////////////////////////////////////
  /// Compute list of lemma-pos to search in WN for given 
  /// word and analysis, according to mapping rules.
  //////////////////////////////////////////////

  void semanticDB::get_WN_keys(const wstring &form, const wstring &lemma, const wstring &tag, list<pair<wstring,wstring> > &searchlist) const {

    searchlist.clear();
    // check all possible mappings to WN-PoS
    list<posmaprule>::const_iterator p;
    for (p=posmap.begin(); p!=posmap.end(); p++) {
    
      TRACE(4,L"Check tag "+tag+L" with posmap: "+p->pos+L" "+p->wnpos+L" "+p->lemma);
      if (tag.find(p->pos)==0) {
        TRACE(4,L"     matched");
        // rule matches word PoS. Add pair lemma/pos to the list
        wstring lm;
        if (p->lemma==L"L") lm=lemma;
        else if (p->lemma==L"F") lm=form;
        else { // interpret it as a PoS tag
          TRACE(3,L"Found word matching special map: "+lemma+L" "+p->lemma);
          lm = form_dict->access_database(lemma+L" "+p->lemma);
        }
      
        // split list in case there are more than one form
        list<wstring> fms=util::wstring2list(lm,L" ");
        for (list<wstring>::iterator ifm=fms.begin(); ifm!=fms.end(); ifm++) {
          TRACE(3,L"Adding word '"+form+L"' to be searched with pos="+p->wnpos+L" and lemma="+(*ifm));
          searchlist.push_back(make_pair(*ifm,p->wnpos));
        }
      }
    }
  }  

} // namespace
