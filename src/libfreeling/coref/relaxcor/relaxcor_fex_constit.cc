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
//   This is an implementation inspired in the PhD Thesis of Emili Sapena
//
//   Little differences:
//   - in drop_det
//
///////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//    Class for the feature extractor.
//////////////////////////////////////////////////////////////////

#include <string>
#include <stdlib.h> // abs
#include <ctime>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/relaxcor_fex_constit.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"RELAXCOR_FEX"
#define MOD_TRACECODE COREF_TRACE

  /////////////////////////////////////////////
  /// Class feature_cache stores already computed mention features
  /////////////////////////////////////////////

  feature_cache::feature_cache() {};
  feature_cache::~feature_cache() {};

  void feature_cache::set_feature(int id, mentionFeature f, unsigned int v) {
    features[id][f]=v;
  }
  void feature_cache::set_feature(int id, mentionWsFeature f, const vector<wstring>& v) {
    wsfeatures[id][f]=v;
  }
  unsigned int feature_cache::get_feature(int id, mentionFeature f) const {
    return features.find(id)->second.find(f)->second;
  }
  const vector<wstring>& feature_cache::get_feature(int id, mentionWsFeature f) const {
    return wsfeatures.find(id)->second.find(f)->second;
  }
  bool feature_cache::computed_feature(int id, mentionFeature f) const {
    return features.find(id)!=features.end() and features.find(id)->second.find(f)!=features.find(id)->second.end();
  }
  bool feature_cache::computed_feature(int id, mentionWsFeature f) const {
    return wsfeatures.find(id)!=wsfeatures.end() and wsfeatures.find(id)->second.find(f)!=wsfeatures.find(id)->second.end();
  }


  const freeling::regexp relaxcor_fex_constit::acronym_re1(L"^[A-Z]+\\_?\\&?\\_?[A-Z]+$");
  const freeling::regexp relaxcor_fex_constit::acronym_re2(L"^([A-Z]\\.)+\\_?\\&?\\_?([A-Z]\\.)+$");
  const freeling::regexp relaxcor_fex_constit::en_reflexive_re(L"sel(f|ves)$");
  const freeling::regexp relaxcor_fex_constit::en_demostrative_re(L"^that|this|those|these$");
  const freeling::regexp relaxcor_fex_constit::en_indefinite_re(L"^a|an|some|many|several|any|other|others|another|anothers|every|each$");
  const freeling::regexp relaxcor_fex_constit::initial_letter_re1(L"^[A-Z]\\.$");
  const freeling::regexp relaxcor_fex_constit::initial_letter_re2(L"^[A-Z].+");
  const freeling::regexp relaxcor_fex_constit::en_det_singular_re(L"^this|that$");
  const freeling::regexp relaxcor_fex_constit::en_det_plural_re(L"^these|those$");
  const freeling::regexp relaxcor_fex_constit::cat_verb_be_re1(L"^(soc|ets|és|som|sou|son)$");
  const freeling::regexp relaxcor_fex_constit::cat_verb_be_re2(L"^(vaig|vas|va|vam|vau|van)$");
  const freeling::regexp relaxcor_fex_constit::en_verb_be_re(L"^(is|are|was|were|a?m)$"); 
  const freeling::regexp relaxcor_fex_constit::es_verb_be_re(L"^(soy|eres|es|somos|sois|son|fui|fuiste|fue|fuimos|fuisteis|fueron)$"); 
  const freeling::regexp relaxcor_fex_constit::arg_re(L"^(.-)?A[\\w\\d]|arg[\\w\\d]"); 
  const freeling::regexp relaxcor_fex_constit::role_re(L"^A.-|arg.-"); 


  //////////////////////////////////////////////////////////////////
  /// Constructor. Sets defaults
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_constit::relaxcor_fex_constit(const wstring &filename, const relaxcor_model &m) : relaxcor_fex(m) {

    // default set of features
    _Active_features = RCF_SET_ALL;

    // read configuration file and store information       
    enum sections {LANGUAGE, TAGSET, LABELS, SEM_DB, ACTIVE_FEATURES, DET_WORDS, PRON_WORDS, SEM_CLASS, CAPITALS, NATIONALITIES, GPE_REGEXPS, NICKS, FORENAMES, ACRO_TERMS, PERSONS, TITLES, OTHER_SECTION};
    config_file cfg(false,L"%");
    map<unsigned int, wstring> labels_section;

    cfg.add_section(L"Language",LANGUAGE,true);
    cfg.add_section(L"Tagset",TAGSET, true);
    cfg.add_section(L"Labels",LABELS, true);
    cfg.add_section(L"SemDB",SEM_DB, true);
    cfg.add_section(L"ActiveFeatures",ACTIVE_FEATURES);
    cfg.add_section(L"DetWords",DET_WORDS, true);
    cfg.add_section(L"PronWords",PRON_WORDS, true);
    cfg.add_section(L"SemClass",SEM_CLASS, true);
    cfg.add_section(L"Capitals",CAPITALS);
    cfg.add_section(L"Nationalities",NATIONALITIES);
    cfg.add_section(L"GPEregexps",GPE_REGEXPS);
    cfg.add_section(L"NickNames",NICKS);
    cfg.add_section(L"ForenameAlias",FORENAMES);
    cfg.add_section(L"AcroTerms",ACRO_TERMS);
    cfg.add_section(L"PersonNames",PERSONS);
    cfg.add_section(L"Titles",TITLES);

    if (not cfg.open(filename))
      ERROR_CRASH(L"Error opening file "+filename);

    map<unsigned int, bool> exists_section;
    wstring path=filename.substr(0,filename.find_last_of(L"/\\")+1);
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
        sin >> name >> val;
	freeling::regexp re(val);
	_Labels.insert(make_pair(name,re));
        break;
      }
      case TAGSET: {
	// load POS tagset
	wstring fname;
	sin>>fname;
	wstring ftagset = util::absolute(fname,path);
        _POS_tagset= new tagset(ftagset);
	break;
      }
      case SEM_DB: {
        // load SEMDB section           
        wstring fname;
        sin>>fname;
        wstring sdbf= util::absolute(fname,path);
        _Semdb= new semanticDB(sdbf);
        TRACE(6,L"Feature extractor loaded SemDB");	
        break;
      }
      case ACTIVE_FEATURES: {
        // Read the mask for active features
        sin>>std::hex>>_Active_features;
	break;
      }
      case DET_WORDS: {
        // Read those determinants which will be droped out of mentions for computing string_matching 
	wstring val;
	sin>>val;
	_Det_words.insert(val);
	break;
      }
      case PRON_WORDS: {
        // Read those words being pronouns for things
	wstring key,val;
	map<wstring,wstring> feats;
	sin>>key;
	sin>>val; feats.insert(make_pair(L"gen",val));
	sin>>val; feats.insert(make_pair(L"per",val));	
	sin>>val; feats.insert(make_pair(L"num",val));	
	_Prons_feat.insert(make_pair(key, feats));
	break;
      }
      case SEM_CLASS: {
        // Read semantic classes for common nouns
	wstring key,val1,val2;
      	sin >> key >> val1 >> val2;
	freeling::regexp re(val2);	
	_Sem_classes.insert(make_pair(key,make_pair(val1,re)));
	break;
      }
      case CAPITALS: {
        // load gazeetter of capitals and countries   
        wstring fname;
        sin>>fname;
	wstring gaz=util::absolute(fname,path);
        read_countries_capitals(gaz);
        TRACE(6,L"Capitals loaded");
	break;
      }
      case NATIONALITIES: {
        // load gazeetter of nationalities  
        wstring fname;
        sin>>fname;
	wstring gaz=util::absolute(fname,path);
        read_pairs(gaz, _Nationalities);
        TRACE(6,L"Nationalities loaded");
	break;
      }
      case GPE_REGEXPS: {
	// load GPE regular expressions  
	  wstring fname;
        sin>>fname;
	wstring gaz=util::absolute(fname,path);
        read_gpe_regexps(gaz);
        TRACE(6,L"GPE regexps loaded");
        break;	
      }
      case NICKS: {
        // load nick names  
	// NOW THE FILE DO NOT CONTAIN NICK NAMES BUT FORENAMES. IT IS A COPY
        wstring fname;
        sin>>fname;
	wstring gaz=util::absolute(fname,path);
        read_same_names(gaz, _Nicks);
        TRACE(6,L"Nick names loaded");
	break;
      }
      case FORENAMES: {
        // load forename aliases  
        wstring fname;
        sin>>fname;
	wstring gaz=util::absolute(fname,path);
        read_same_names(gaz, _Forenames);
        TRACE(6,L"Forename aliases loaded");
	break;
      }
      case ACRO_TERMS: {
        // load acronym terms (infix and suffix)             
        wstring name, val;
	sin>>name;
        sin>>val;
	freeling::regexp re(val);
        _AcroTerms.insert(make_pair(name,re));
        break;
      }
      case PERSONS: {
        // load person names and genders   
        wstring fname;
        sin>>fname;
	wstring gaz=util::absolute(fname,path);
        read_pairs(gaz, _Person_Names);
        TRACE(6,L"Person names and genders loaded");
	break;
      }
      case TITLES: {
        // load titless and genders   
        wstring fname;
        sin>>fname;
	wstring gaz=util::absolute(fname,path);
        read_pairs(gaz, _Titles);
        TRACE(6,L"Titles and genders loaded");
	break;
      }
      default: break;
      }
    }

    TRACE(2,L"Module successfully loaded");

}

  //////////////////////////////////////////////////////////////////
  /// Destructor
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_constit::~relaxcor_fex_constit() {
    delete _Semdb;
    delete _POS_tagset;
  }


  // ================  Groups of Feature extraction functions ==============

  //////////////////////////////////////////////////////////////////
  ///    Structural features.
  //////////////////////////////////////////////////////////////////

  void relaxcor_fex_constit::get_structural(const mention &m1, const mention &m2, relaxcor_model::Tfeatures &ft, feature_cache &fcache) const {
    TRACE(6,L"get structural features");
    
    // distance in #sentences
    unsigned int dist = abs(m1.get_n_sentence() - m2.get_n_sentence());
    ft[ID(L"RCF_DIST_SEN_0")]   = (dist==0)? true : false;
    ft[ID(L"RCF_DIST_SEN_1")]   = (dist==1)? true : false;
    ft[ID(L"RCF_DIST_SEN_L3")]  = (dist<=3)? true : false;
    ft[ID(L"RCF_DIST_SEN_G3")]  = (dist>3)?  true : false;
    // #mentions between both mentions
    dist = abs(m1.get_id() - m2.get_id()) - 1;
    ft[ID(L"RCF_DIST_MEN_0")]   = (dist==0)?  true : false;
    ft[ID(L"RCF_DIST_MEN_L3")]  = (dist<=3)?  true : false;
    ft[ID(L"RCF_DIST_MEN_L10")] = (dist<=10)? true : false;
    ft[ID(L"RCF_DIST_MEN_G10")] = (dist>10)?  true : false;
    // distance in #phrases
    dist = dist_in_phrases(m1,m2,fcache);
    TRACE(6,L"      dist in phrases "+m1.value()+L":"+m2.value()+L" = "+util::int2wstring(dist));
    ft[ID(L"RCF_DIST_PHR_0")]   = (dist==0)?  true : false;
    ft[ID(L"RCF_DIST_PHR_1")]   = (dist==1)?  true : false;
    ft[ID(L"RCF_DIST_PHR_L3")]  = (dist<=3)?  true : false;
    // in quotes?
    ft[ID(L"RCF_I_IN_QUOTES")] = (in_quotes(m1,fcache) ==1)? true : false;
    ft[ID(L"RCF_J_IN_QUOTES")] = (in_quotes(m2,fcache) ==1)? true : false;
    // first mention in sentence?
    ft[ID(L"RCF_I_FIRST")] = m1.is_initial();
    ft[ID(L"RCF_J_FIRST")] = m2.is_initial();
    // one is appositive of the other
    ft[ID(L"RCF_APPOSITIVE")] = appositive(m1,m2,fcache);
    // one is nested to the other
    ft[ID(L"RCF_NESTED")] = nested(m1,m2);
    // type of mention
    ft[ID(L"RCF_I_TYPE_P")] = m1.is_type(mention::PRONOUN);
    ft[ID(L"RCF_I_TYPE_S")] = m1.is_type(mention::NOUN_PHRASE) or m1.is_type(mention::COMPOSITE) or m1.is_type(mention::VERB_PHRASE);
    ft[ID(L"RCF_I_TYPE_E")] = m1.is_type(mention::PROPER_NOUN);
    ft[ID(L"RCF_J_TYPE_P")] = m2.is_type(mention::PRONOUN);
    ft[ID(L"RCF_J_TYPE_S")] = m2.is_type(mention::NOUN_PHRASE) or m2.is_type(mention::COMPOSITE) or m2.is_type(mention::VERB_PHRASE);
    ft[ID(L"RCF_J_TYPE_E")] = m2.is_type(mention::PROPER_NOUN);
  }

  void relaxcor_fex_constit::get_lexical(const mention &m1, const mention &m2, relaxcor_model::Tfeatures &ft, feature_cache &fcache) const {
    TRACE(6,L"get lexical features");
    
    // string matchings without some first determinants (param DetWords) 
    ft[ID(L"RCF_STR_MATCH")]       = string_match(m1, m2, fcache);
    ft[ID(L"RCF_PRO_STR")]         = pronoun_string_match(m1, m2, ft[ID(L"RCF_STR_MATCH")], fcache);
    ft[ID(L"RCF_PN_STR")]          = proper_noun_string_match(m1, m2, ft[ID(L"RCF_STR_MATCH")], fcache);
    ft[ID(L"RCF_SOON_STR_NONPRO")] = no_pronoun_string_match(m1, m2, ft[ID(L"RCF_STR_MATCH")], fcache);
    // head and term equivalences
    ft[ID(L"RCF_I_HEAD_TERM")] = (head_is_term(m1, fcache) == 1)? true : false;
    ft[ID(L"RCF_J_HEAD_TERM")] = (head_is_term(m2, fcache) == 1)? true : false;
    ft[ID(L"RCF_HEAD_MATCH")]  = util::lowercase(m1.get_head().get_form()) == util::lowercase(m2.get_head().get_form());
    ft[ID(L"RCF_TERM_MATCH")]  = util::lowercase(compute_term(m1)) == util::lowercase(compute_term(m2));

    // one is alias of the other
    unsigned int known = alias(m1, m2, fcache);
    ft[ID(L"RCF_ALIAS_YES")] = (known == 1)? true : false;
    ft[ID(L"RCF_ALIAS_NO")]  = (known == 0)? true : false;
    ft[ID(L"RCF_ALIAS_UN")]  = (known == 2)? true : false;
    TRACE(6, L"   Alias = "+ wstring(known==0 ? L"no" : (known==1? L"yes" : L"unknown") ) );
  }

  void relaxcor_fex_constit::get_morphological(const mention &m1, const mention &m2, relaxcor_model::Tfeatures &ft, vector<mention> &mentions, feature_cache &fcache) const {
    TRACE(6,L"get morphological features");
    
    ft[ID(L"RCF_I_POSSESSIVE")] = (is_possessive(m1, fcache) == 1)? true : false;
    ft[ID(L"RCF_J_POSSESSIVE")] = (is_possessive(m2, fcache) == 1)? true : false;
    // they agree in number
    unsigned int num = same_number(m1,m2, fcache);
    ft[ID(L"RCF_NUMBER_YES")] = (num == 1) ? true : false;
    ft[ID(L"RCF_NUMBER_NO")]  = (num == 0) ? true : false;
    ft[ID(L"RCF_NUMBER_UN")]  = (num == 2) ? true : false;
    // they agree in gender
    unsigned int gen = same_gender(m1,m2, fcache);
    ft[ID(L"RCF_GENDER_YES")] = (gen == 1) ? true : false;
    ft[ID(L"RCF_GENDER_NO")]  = (gen == 0) ? true : false;
    ft[ID(L"RCF_GENDER_UN")]  = (gen == 2) ? true : false;
    // they are 3rd person
    ft[ID(L"RCF_I_THIRD_PERSON")] = (is_3rd_person(m1, fcache) == 1) ? true : false;
    ft[ID(L"RCF_J_THIRD_PERSON")]  = (is_3rd_person(m2, fcache) == 1) ? true : false;
    // they are proper nouns
    ft[ID(L"RCF_I_PROPER_NAME")] = m1.is_type(mention::PROPER_NOUN);
    ft[ID(L"RCF_J_PROPER_NAME")] = m2.is_type(mention::PROPER_NOUN);
    // they are nouns
    ft[ID(L"RCF_I_NOUN")] = m1.is_type(mention::NOUN_PHRASE);
    ft[ID(L"RCF_J_NOUN")] = m2.is_type(mention::NOUN_PHRASE);
    // they agree in gender and number
    unsigned int agree=agreement(m1,m2, fcache);
    ft[ID(L"RCF_AGREEMENT_YES")] = (agree == 1)? true : false;
    ft[ID(L"RCF_AGREEMENT_NO")]  = (agree == 0)? true : false;
    ft[ID(L"RCF_AGREEMENT_UN")]  = (agree == 2)? true : false;
    // m1 is the closest referent to m2 
    unsigned int cagree=closest_agreement(m1,m2,mentions, fcache);
    ft[ID(L"RCF_C_AGREEMENT_YES")] = (cagree == 1)? true : false;
    ft[ID(L"RCF_C_AGREEMENT_NO")]  = (cagree == 0)? true : false;
    ft[ID(L"RCF_C_AGREEMENT_UN")]  = (cagree == 2)? true : false;
    // they are reflexive
    ft[ID(L"RCF_I_REFLEXIVE")] = (is_reflexive(m1, fcache) == 1) ? true : false;
    ft[ID(L"RCF_J_REFLEXIVE")] = (is_reflexive(m2, fcache) == 1) ? true : false;
  }

  void relaxcor_fex_constit::get_syntactic(const mention &m1, const mention&m2, relaxcor_model::Tfeatures &ft, vector<mention> &mentions, feature_cache &fcache) const {
    TRACE(6,L"get syntactic features");
  
    // they are definite noun phrases
    ft[ID(L"RCF_I_DEF_NP")] = (is_def_NP(m1, fcache) == 1) ? true : false;
    ft[ID(L"RCF_J_DEF_NP")] = (is_def_NP(m2, fcache) == 1) ? true : false;
    // they are demonstrative noun phrases
    ft[ID(L"RCF_I_DEM_NP")] = (is_dem_NP(m1, fcache) == 1) ? true : false;
    ft[ID(L"RCF_J_DEM_NP")] = (is_dem_NP(m2, fcache) == 1) ? true : false;
    // they share the maximal noun phrase
    ft[ID(L"RCF_MAXIMAL_NP")] = share_maximal_NP(m1, m2, mentions, fcache);
    // they are maximal noun phrases
    ft[ID(L"RCF_I_MAXIMAL_NP")] = (is_maximal_NP(m1, mentions, fcache) == 1) ? true : false;
    ft[ID(L"RCF_J_MAXIMAL_NP")] = (is_maximal_NP(m2, mentions, fcache) == 1) ? true : false;
    // they are indefinite noun phrases
    ft[ID(L"RCF_I_INDEF_NP")] = (is_indef_NP(m1, fcache) == 1) ? true : false;
    ft[ID(L"RCF_J_INDEF_NP")] = (is_indef_NP(m2, fcache) == 1) ? true : false;
    // they are composite
    ft[ID(L"RCF_I_COMPOSITE")] = m1.is_type(mention::COMPOSITE);
    ft[ID(L"RCF_J_COMPOSITE")] = m2.is_type(mention::COMPOSITE);
    // they are embedded
    ft[ID(L"RCF_I_EMBEDDED")] = (is_embedded_noun(m1, mentions, fcache) == 1) ? true : false;
    ft[ID(L"RCF_J_EMBEDDED")] = (is_embedded_noun(m2, mentions, fcache) == 1) ? true : false;
    // they satisfy the positive/negative conditions of Binding Theory
    parse_tree::const_iterator pt1=m1.get_ptree();
    parse_tree::const_iterator pt2=m2.get_ptree();
    bool cc12 = parse_tree::C_commands(pt1,pt2);
    bool cc21 = parse_tree::C_commands(pt2,pt1);
    ft[ID(L"RCF_BINDING_POS")] = binding_pos(m1, m2, cc12, fcache) or binding_pos(m2, m1, cc21, fcache);
    ft[ID(L"RCF_BINDING_NEG")] = binding_neg(m1, m2, cc12, fcache) or binding_neg(m2, m1, cc21, fcache);
    // they satisfy C_command features
    ft[ID(L"RCF_C_COMMANDS_IJ")] = cc12;
    ft[ID(L"RCF_C_COMMANDS_JI")] = cc21;
    // SRL arguments
    // labels Y within RCF_X_SRL_ARG_Y are those defined as Semeval
    // should be changed to those from CoNLL 
    wstring args1=L"", preds1=L"";
    get_arguments(m1, args1, preds1, fcache);
    ft[ID(L"RCF_I_SRL_ARG_0")] = args1.find('0')!=string::npos;
    ft[ID(L"RCF_I_SRL_ARG_1")] = args1.find('1')!=string::npos;
    ft[ID(L"RCF_I_SRL_ARG_2")] = args1.find('2')!=string::npos;
    ft[ID(L"RCF_I_SRL_ARG_X")] = args1.find_first_of(L"345")!=string::npos;
    ft[ID(L"RCF_I_SRL_ARG_NUM")] = ft[ID(L"RCF_I_SRL_ARG_0")] or ft[ID(L"RCF_I_SRL_ARG_1")] or ft[ID(L"RCF_I_SRL_ARG_2")] or ft[ID(L"RCF_I_SRL_ARG_X")];
    ft[ID(L"RCF_I_SRL_ARG_M")] = args1.find('M')!=string::npos;
    ft[ID(L"RCF_I_SRL_ARG_N")] = args1==argument::EMPTY_ROLE;
    ft[ID(L"RCF_I_SRL_ARG_Z")] = !ft[ID(L"RCF_I_SRL_ARG_N")] and !ft[ID(L"RCF_I_SRL_ARG_NUM")] and !ft[ID(L"RCF_I_SRL_ARG_M")];
    wstring args2=L"", preds2=L"";
    get_arguments(m2, args2, preds2, fcache);
    ft[ID(L"RCF_J_SRL_ARG_0")] = args2.find('0')!=string::npos;
    ft[ID(L"RCF_J_SRL_ARG_1")] = args2.find('1')!=string::npos;
    ft[ID(L"RCF_J_SRL_ARG_2")] = args2.find('2')!=string::npos;
    ft[ID(L"RCF_I_SRL_ARG_X")] = args2.find_first_of(L"345")!=string::npos;
    ft[ID(L"RCF_J_SRL_ARG_NUM")] = ft[ID(L"RCF_J_SRL_ARG_0")] or ft[ID(L"RCF_J_SRL_ARG_1")] or ft[ID(L"RCF_J_SRL_ARG_2")] or ft[ID(L"RCF_J_SRL_ARG_X")];
    ft[ID(L"RCF_J_SRL_ARG_M")] = args2.find('M')!=string::npos;
    ft[ID(L"RCF_J_SRL_ARG_N")] = args2==argument::EMPTY_ROLE;
    ft[ID(L"RCF_J_SRL_ARG_Z")] = !ft[ID(L"RCF_J_SRL_ARG_N")] and !ft[ID(L"RCF_J_SRL_ARG_NUM")] and !ft[ID(L"RCF_J_SRL_ARG_M")];
    ft[ID(L"RCF_SRL_SAMEVERB")] = same_preds(ft[ID(L"RCF_DIST_SEN_0")], preds1, preds2, fcache);
    ft[ID(L"RCF_SAME_SRL_ARG")] = same_args(ft[ID(L"RCF_DIST_SEN_0")], args1, args2, ft, fcache);

  }

  void relaxcor_fex_constit::get_semantic(const mention &m1, const mention&m2, relaxcor_model::Tfeatures &ft, vector<mention> &mentions, feature_cache &fcache) const {
    TRACE(6,L"get semantic features");

    /// the are close and separated by the verb "to be"
    ft[ID(L"RCF_VERB_IS")] = separated_by_verb_is(m1,m2,mentions, fcache);
    /// they are of the same semantic class
    unsigned int sem_match = sem_class_match(m1,m2, fcache);
    ft[ID(L"RCF_SEMCLASS_YES")] = (sem_match == 1) ? true : false;
    ft[ID(L"RCF_SEMCLASS_NO")] = (sem_match == 0) ? true : false;
    ft[ID(L"RCF_SEMCLASS_UN")] = (sem_match == 2) ? true : false;
    // they are person
    ft[ID(L"RCF_I_PERSON")] = is_semantic_type(m1, L"person",fcache);
    ft[ID(L"RCF_J_PERSON")] = is_semantic_type(m2, L"person",fcache);
    // they are organizations
    ft[ID(L"RCF_I_ORGANIZATION")] = is_semantic_type(m1, L"organization",fcache);
    ft[ID(L"RCF_J_ORGANIZATION")] = is_semantic_type(m2, L"organization",fcache);
    // they are locations
    ft[ID(L"RCF_I_LOCATION")] = is_semantic_type(m1, L"location",fcache);
    ft[ID(L"RCF_J_LOCATION")] = is_semantic_type(m2, L"location",fcache);
    // they are animacy
    ft[ID(L"RCF_ANIMACY")] = animacy(m1, m2, fcache);
    // they are semantically incompatible
    ft[ID(L"RCF_INCOMPATIBLES")] = incompatible(m1, m2, fcache);
    // SRL features
    // they have the same semantic role (following semeval) for non numerical arguments
    // (LOC,EXT,TMP,DIS,ADV,NEG,MOD,CAU,PNC,MNR,DIR,PRD)

    vector<wstring> roles1, roles2;
    get_roles(m1, roles1, fcache);
    get_roles(m2, roles2, fcache);

    ft[ID(L"RCF_SRL_SAME_ROLE")] = (roles1.size()<roles2.size())? same_roles(roles1,roles2, fcache) : same_roles(roles2,roles1, fcache);
    
  }

  void relaxcor_fex_constit::get_discourse(const mention &m1, const mention&m2, relaxcor_model::Tfeatures &ft, feature_cache &fcache) const {
     TRACE(6,L"get discourse features");

     // include if necessary
 }

  void relaxcor_fex_constit::get_group_features(vector<mention> &mentions, relaxcor_model::Tfeatures &ft, feature_cache &fcache) const {
    TRACE(6,L"get group features");

    // include if necessary
  }

  // =============================================================
  // ================  Feature extraction functions ==============
  // =============================================================

  // =============================================================
  // ================  Structural extraction functions ===========
  // =============================================================

  //////////////////////////////////////////////////////////////////
  ///    Returns the distance in number of phrases between m1 and m2
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::dist_in_phrases(const mention &m1, const mention &m2, feature_cache &fcache) const {

    TRACE(6,L"   dist_in_phrases");

    if (m1.get_n_sentence() != m2.get_n_sentence()) return VERY_BIG;
    if (nested(m1,m2)) return 0;

    wstring args1=L"", preds1=L"";
    get_arguments(m1,args1,preds1, fcache);
    wstring args2=L"", preds2=L"";
    get_arguments(m2,args2,preds2, fcache);

    TRACE(6,L"      preds(m1)=" + preds1 + L" args(m1)=" + args1 + L"; preds(m2)=" + preds2 + L" args(m2)=" + args2);

    if (preds1.length()==0 or preds2.length()==0) return VERY_BIG;

    wstring::const_iterator it1=preds1.begin(), it2=preds2.begin();
    unsigned int dif=VERY_BIG;

    while (it1!=preds1.end() and it2!=preds2.end() and dif>0) {

      TRACE(7,L"      m1 is argument of pred "+util::int2wstring(*it1));
      TRACE(7,L"      m2 is argument of pred "+util::int2wstring(*it2));

      unsigned int dif_after= (*it1>*it2)? (*it1)-(*it2) : (*it2)-(*it1);

      if ((dif > dif_after)) dif=dif_after;

      if (*it1 > *it2) it2++;
      else if (*it1 < *it2) it1++;
      else {it1++; it2++;}
    }

    return dif;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'm' is in quotes
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::in_quotes(const mention &m, feature_cache &fcache) const {

    int id=m.get_id();

    if (not fcache.computed_feature(id, feature_cache::IN_QUOTES)) {
      paragraph::const_iterator s = m.get_sentence();
      unsigned int inside = 0;
      sentence::const_iterator first = m.get_it_begin();
      sentence::const_iterator end = m.get_it_end();
      sentence::const_iterator last = --end;

      if (( first->get_tag() == L"Fe" or first->get_tag() == L"Fra") and (last->get_tag() == L"Fe" or last->get_tag() == L"Frc"))
	fcache.set_feature(id, feature_cache::IN_QUOTES, 1);
      else {
	for (sentence::const_iterator it=s->begin(); it!=s->end() && it!=first; it++) {
	  if      (it->get_lemma() == L"``") inside = 1 ;
	  else if (it->get_lemma() == L"''") inside = 0 ;
	  else if (it->get_tag() == L"Fra")  inside = 1;
	  else if (it->get_tag() == L"Frc")  inside = 0;
	  else if (it->get_tag() == L"Fe")   inside = 1-inside;
	}
	fcache.set_feature(id, feature_cache::IN_QUOTES, inside);
      }
    }

    unsigned int r = fcache.get_feature(id, feature_cache::IN_QUOTES);
    TRACE(6,L"   in_quotes " + util::int2wstring(id) + L" = "+util::int2wstring(r));
    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether one is appositive to the other
  ///    simple : if separated by colon 
  ///    warning: not always true (e.g. Peter, Mary and John were there.)
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_constit::appositive(const mention &m1, const mention &m2, feature_cache &fcache) const {

    sentence::const_iterator m1_end = m1.get_it_end();
    sentence::const_iterator m2_end = m2.get_it_end();
 
    bool r = m1.get_n_sentence() == m2.get_n_sentence() and not nested(m1,m2) and 
      ((m1_end!=m1.get_sentence()->end() and m2.get_it_begin()==++m1_end and m1.get_it_end()->get_tag()==L"Fc") or 
       (m2_end!=m2.get_sentence()->end() and m1.get_it_begin()==++m2_end and m2.get_it_end()->get_tag()==L"Fc"));

    TRACE(6,L"   appositive = "+util::int2wstring(r));
    return r;
  }

  ////////////////////////////////////////////////////
  ///    Returns whether one is nested to the other 
  ////////////////////////////////////////////////////

  bool relaxcor_fex_constit::nested(const mention &m1, const mention &m2) const {

    bool r = m1.get_n_sentence() == m2.get_n_sentence()
           and
           ( (m2.get_pos_begin()<=m1.get_pos_begin() and m2.get_pos_end()>=m1.get_pos_end())
             or
	     (m1.get_pos_begin()<=m2.get_pos_begin() and m1.get_pos_end()>=m2.get_pos_end()));

    TRACE(6,L"   nested = "+util::int2wstring(r));
    return r;
  }
 
  ////////////////////////////////////////////////////
  ///    Returns whether one is intersected to the other 
  ////////////////////////////////////////////////////

  bool relaxcor_fex_constit::intersected(const mention &m1, const mention &m2) const {

    bool r = m1.get_n_sentence() == m2.get_n_sentence()
           and
      ( (m2.get_pos_begin()<m1.get_pos_begin() and m1.get_pos_begin()<=m2.get_pos_end() and m2.get_pos_end()<m1.get_pos_end())
             or
	(m1.get_pos_begin()<m2.get_pos_begin() and m2.get_pos_begin()<=m1.get_pos_end() and m1.get_pos_end()<m2.get_pos_end()));

    TRACE(6,L"   intersected = "+util::int2wstring(r));

    return r;
  }

  // =============================================================
  // ================  Lexical extraction functions ===========
  // =============================================================

  ////////////////////////////////////////////////////////////////////////////
  ///    Returns whether both match without considering DetWords at starting
  ////////////////////////////////////////////////////////////////////////////
  
  bool relaxcor_fex_constit::string_match(const mention &m1, const mention &m2, feature_cache &fcache) const {
    wstring dd1 = drop_det(m1);
    wstring dd2 = drop_det(m2);
    bool r = (dd1==dd2);
    TRACE(6,L"   string match = " + util::int2wstring(r) + L". drop det 1 = " + dd1 + L", drop det 2 = " + dd2);
    return r;
  }

  ////////////////////////////////////////////////////////////////////////////
  ///    Returns whether both are equivalent pronouns 
  ////////////////////////////////////////////////////////////////////////////
  
  bool relaxcor_fex_constit::pronoun_string_match(const mention &m1, const mention &m2, bool match, feature_cache &fcache) const {
    bool r = match and m1.get_type() == mention::PRONOUN and m2.get_type() == mention::PRONOUN;
    TRACE(6,L"   pronoun string match = " + util::int2wstring(r) );
    return r;
  }

  ////////////////////////////////////////////////////////////////////////////
  ///    Returns whether both are equivalent proper nouns 
  ////////////////////////////////////////////////////////////////////////////
  
  bool relaxcor_fex_constit::proper_noun_string_match(const mention &m1, const mention &m2, bool match, feature_cache &fcache) const {
    bool r =  m1.get_type() == mention::PROPER_NOUN and m2.get_type() == mention::PROPER_NOUN and match;
    TRACE(6,L"   proper noun string match = " + util::int2wstring(r) );
    return r;
  }

  ////////////////////////////////////////////////////////////////////////////
  ///    Returns whether both are equivalents but NOT pronouns 
  ////////////////////////////////////////////////////////////////////////////
  
  bool relaxcor_fex_constit::no_pronoun_string_match(const mention &m1, const mention &m2, bool match, feature_cache &fcache) const {
    bool r = match and  m1.get_type() != mention::PRONOUN and m2.get_type() != mention::PRONOUN;
    TRACE(6,L"   non-pronoun string match = " + util::int2wstring(r) );
    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'm' has head equal to term
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::head_is_term(const mention &m, feature_cache &fcache) const {

    int id=m.get_id();

    if (not fcache.computed_feature(id, feature_cache::HEAD_TERM)) {
      (m.get_head().get_form() == compute_term(m)) ? 
	fcache.set_feature(id, feature_cache::HEAD_TERM, 1) : fcache.set_feature(id, feature_cache::HEAD_TERM, 0);
    }

    unsigned int r = fcache.get_feature(id, feature_cache::HEAD_TERM);
    TRACE(6,L"   head is term " + util::int2wstring(id) + L" = "+util::int2wstring(r));
    return r;
  }

  ///////////////////////////////////////////////////////////////////////////////////////////
  ///    Returns whether one mention is alias of the other one, for proper nouns.
  ///    For the rest of mention types, returns unknown
  ///////////////////////////////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::alias(const mention &m1, const mention &m2, feature_cache &fcache) const {
    TRACE(6,L"   Alias: '"+m1.value()+L"' vs '"+m2.value()+L"'");

    if (nested(m1,m2)) return 0;
    if (m1.is_type(mention::PRONOUN) or m2.is_type(mention::PRONOUN)) return 2;
 
    // number are not aliases
    if ((_Labels.find(L"NUM")->second.search(parse_tree::get_head_label(m1.get_ptree())) and 
	 m1.get_pos_begin()==m1.get_pos_end()) or
	(_Labels.find(L"NUM")->second.search(parse_tree::get_head_label(m2.get_ptree())) and
	 m2.get_pos_begin()==m2.get_pos_end()))
      return 0;

    // one is proper name
    if (m1.is_type(mention::PROPER_NOUN) or m2.is_type(mention::PROPER_NOUN)) {
 
      //wstring str1= m1.is_type(mention::PROPER_NOUN) ? m1.get_head().get_form() : string_merge(m1,false);
      //wstring str2= m2.is_type(mention::PROPER_NOUN)? m2.get_head().get_form() : string_merge(m2,false);
      wstring str1= m1.get_pos_begin()==m1.get_pos_end() ? m1.value() : string_merge(m1,false);
      wstring str2= m2.get_pos_begin()==m2.get_pos_end() ? m2.value() : string_merge(m2,false);

      if (util::lowercase(str1) == util::lowercase(str2)) return 1;

      TRACE(6,L"      Alias: geo");

      // Case: GEO/GPE aliases
      unsigned int geo=geo_match(m1, m2);
      if (geo!=2) return geo; 

      TRACE(6,L"      Alias: different gender");
      
      // Case: PERSON aliases
      if (not same_gender(m1, m2, fcache)) return 0;

      wstring strA(str1);
      wstring strB(str2);
      vector<wstring> wordsA = split_words(strA);
      vector<wstring> wordsB = split_words(strB);

      TRACE(6,L"      Alias: initials match ("+strA+L","+strB+L")");

      // // remove titles at the beginning
      if (_Titles.find(wordsA.front())!=_Titles.end()) { 
	for (unsigned int i=1; i<wordsA.size(); i++) wordsA[i-1]=wordsA[i]; 
	wordsA.pop_back();
	strA = util::vector2wstring(wordsA,L"_");
      }
      if (_Titles.find(wordsB.front())!=_Titles.end()) { 
	for (unsigned int i=1; i<wordsB.size(); i++) wordsB[i-1]=wordsB[i]; 
	wordsB.pop_back();
	strB = util::vector2wstring(wordsB,L"_");
      }

      // // the first word of the first mention is initial letter (X.) of the second mention
      if (wordsA.size()>0 and wordsB.size()>0) {
	unsigned int initials = (strA.length() < strB.length()) ? initials_match(wordsA,wordsB) : initials_match(wordsB,wordsA);

	if (initials!=2) return initials;
        
	TRACE(6,L"      Alias: nick name match");

	// // one is nick name of the other
	// without titles
	unsigned int nick = nick_name_match(strA, strB); 

	if (nick!=2) return nick;
 
	TRACE(6,L"      Alias: forename match");

	// // forenames shared 
	// WARNING: forenames which can also be verbs, adj or common nouns should be included in <Names> of np.dat
	unsigned int forenames = forenames_match(wordsA,wordsB);

	if (forenames!=2) return forenames;

	TRACE(6,L"      Alias: first name included");

	// // one seems first name included in the other
	if (m1.is_type(mention::PROPER_NOUN) and m2.is_type(mention::PROPER_NOUN)) {
	  // the next line introduces a constraint too hard
	  // and  is_semantic_type(m1,L"person") and is_semantic_type(m2,L"person")) {
	  unsigned int first_name=2;
	  if (wordsA.size() == 1)  first_name=first_name_match(wordsA.front(),wordsB);
	  else if (wordsB.size() == 1) first_name=first_name_match(wordsB.front(),wordsA);

	  if (first_name!=2) return first_name;
	}

      }
      
      TRACE(6,L"      Alias: acronym");

      wordsA = split_words(str1);
      wordsB = split_words(str2);

      // Case: ORG aliases (acronyms)
      if (is_acronym(str1) or is_acronym(str2)) {
	// one is acronym of the other
	//unsigned int acronym = (is_acronym(str1)) ? acronym_of(wordsA,wordsB) : acronym_of(wordsB,wordsA);
	//if (acronym==1) return 1;
	
	return (is_acronym(str1)) ? acronym_of(wordsA,wordsB) : acronym_of(wordsB,wordsA);
      }
      
      TRACE(6,L"      Alias: similar enough");

      // differs from Sapena's
      double Lev= lex_dist(str1,str2);

      return (Lev > 0.15) ? 0 : 1;
    }

    return 2;
  }


  // =============================================================
  // ================  Morphological extraction functions ========
  // =============================================================

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'm' is possessive pronoun
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::is_possessive(const mention &m, feature_cache &fcache) const {

    int id=m.get_id();
    
    if (_Labels.find(L"POSS")==_Labels.end())
      ERROR_CRASH(L"Error: POSS labels not defined in config file");

    if (not fcache.computed_feature(id, feature_cache::POSSESSIVE))
      _Labels.find(L"POSS")->second.search(m.get_head().get_tag())?
	fcache.set_feature(id, feature_cache::POSSESSIVE, 1) : fcache.set_feature(id, feature_cache::POSSESSIVE, 0);

    unsigned int r = fcache.get_feature(id, feature_cache::POSSESSIVE);
    TRACE(6,L"   is possessive " + util::int2wstring(id) + L" = "+util::int2wstring(r));
    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' and 'j' agree in number
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::same_number(const mention &m1, const mention &m2, feature_cache &fcache) const {

    // values for 'number' (0-unknown, 1-sing, 2-bi, 3-plural 4-invariant)
    // some values are not possible for a given language

    if (not fcache.computed_feature(m1.get_id(), feature_cache::NUMBER))
      fcache.set_feature(m1.get_id(), feature_cache::NUMBER, get_number(m1));
    unsigned int num1 = fcache.get_feature(m1.get_id(), feature_cache::NUMBER);

    if (not fcache.computed_feature(m2.get_id(), feature_cache::NUMBER))
      fcache.set_feature(m2.get_id(), feature_cache::NUMBER, get_number(m2));
    unsigned int num2 = fcache.get_feature(m2.get_id(), feature_cache::NUMBER);
    
    TRACE(7,L"      same number: "+m1.get_head().get_form() +L"=" + util::int2wstring(num1)+L", "
                                  +m2.get_head().get_form()+L"="+util::int2wstring(num2));

    unsigned int r = 0;
    if (num1==0 or num2==0) r = 2;
    else if (num1==num2 or num1==4 or num2==4) r = 1;

    TRACE(6, L"   same number = "+ wstring(r==0 ? L"no" : (r==1? L"yes" : L"unkown") ) );
    return r;
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' and 'j' agree in gender
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::same_gender(const mention &m1, const mention &m2, feature_cache &fcache) const {
    // values for 'gender' (0-unknown, 1-m, 2-f, 3-mf, 4-n)

    if (not fcache.computed_feature(m1.get_id(), feature_cache::GENDER))
      fcache.set_feature(m1.get_id(), feature_cache::GENDER, get_gender(m1,fcache));
    unsigned int gen1 = fcache.get_feature(m1.get_id(), feature_cache::GENDER);

    if (not fcache.computed_feature(m2.get_id(), feature_cache::GENDER))
      fcache.set_feature(m2.get_id(), feature_cache::GENDER, get_gender(m2,fcache));
    unsigned int gen2 = fcache.get_feature(m2.get_id(), feature_cache::GENDER);
    
    TRACE(7,L"      same gender: "+m1.get_head().get_form() +L"=" + util::int2wstring(gen1)+L", "
                                  +m2.get_head().get_form()+L"="+util::int2wstring(gen2));

    unsigned int r = 0;
    if (gen1==0 or gen2==0) r = 2;
    else if (gen1==gen2 or (gen2!=4 and gen1==3) or (gen1!=4 and gen2==3)) r = 1; // differs from Sapena's version

    TRACE(6, L"      same gender = "+ wstring(r==0 ? L"no" : (r==1? L"yes" : L"unkown") ) );
    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'm' is 3rd person
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::is_3rd_person(const mention &m, feature_cache &fcache) const {

    int id=m.get_id();

    if (not fcache.computed_feature(id, feature_cache::THIRD_PERSON)) {
      if (m.is_type(mention::PRONOUN)) {
	if (_Language == L"SPANISH" or _Language == L"CATALAN") {
	  wstring tag=m.get_head().get_tag();
	  // e.g. 'ello' has person=0.
	  if (extract_msd_feature(tag,L"person") == L"3") fcache.set_feature(id, feature_cache::THIRD_PERSON, 1) ;
          else fcache.set_feature(id, feature_cache::THIRD_PERSON, 0);
	}
	if (_Language == L"ENGLISH") {
	  wstring head_word=m.get_head().get_lc_form();

          map<wstring,map<wstring,wstring>>::const_iterator p=_Prons_feat.find(head_word);
	  if (p!=_Prons_feat.end()) {
            map<wstring,wstring>::const_iterator q = p->second.find(L"per");
            if (q != p->second.end() and q->second == L"3")
	      fcache.set_feature(id, feature_cache::THIRD_PERSON, 1);
            else 
              fcache.set_feature(id, feature_cache::THIRD_PERSON, 0);
          }
	  else
	    // by default
	    fcache.set_feature(id, feature_cache::THIRD_PERSON, 1);
	}
	// other languages
      }
      else fcache.set_feature(id, feature_cache::THIRD_PERSON, 1);
    }

    unsigned int r = fcache.get_feature(id, feature_cache::THIRD_PERSON);
    TRACE(6,L"   is 3rd person " + util::int2wstring(id) + L" = " + util::int2wstring(r));
    return r;
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns 1 if they agree in number and gender 
  ///            0 if they do not agree in number and in gender
  ///            2 otherwise
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::agreement(const mention &m1, const mention &m2, feature_cache &fcache) const {

    unsigned int r = 2;
    if (same_gender(m1,m2, fcache)==1 and same_number(m1,m2, fcache)==1) 
      // if one is common and the other is female they do not agree (eg.: "las barras" <-> "ellos")
      r = ((get_gender(m1,fcache)==3 and get_gender(m2,fcache)==2) or (get_gender(m2,fcache)==3 and get_gender(m1,fcache)==2)) ? 0 : 1;

    else if (same_gender(m1,m2, fcache)==0 or same_number(m1,m2, fcache)==0) 
      r = 0;

    TRACE(6,L"   agreement = " + wstring(r==0 ? L"no" : (r==1? L"yes" : L"unkown") ) );   
    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns 1 m1 is the closest referent to m2
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::closest_agreement(const mention &m1, const mention &m2, vector<mention> &mentions, feature_cache &fcache) const {

    unsigned int r = agreement(m1,m2,fcache);

    if (r == 1) {
      bool found=false;
      for (int i=m1.get_id()+1; i<m2.get_id() and not found; i++)
        found = (agreement(mentions[i],m2, fcache)==1);
      if (found) r=0;
    }

    TRACE(6,L"   closest agreement = " + wstring(r==0 ? L"no" : (r==1? L"yes" : L"unkown") ) );   
    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns 1 when it is a reflexive pronoun
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::is_reflexive(const mention &m, feature_cache &fcache) const {
    int id=m.get_id();

    if (not fcache.computed_feature(id, feature_cache::REFLEXIVE)) {
      if (_Language==L"ENGLISH") {
	if (m.is_type(mention::PRONOUN)) {
	  wstring head_word=m.get_head().get_lc_form();
	  en_reflexive_re.search(head_word) ? fcache.set_feature(id, feature_cache::REFLEXIVE, 1) : fcache.set_feature(id, feature_cache::REFLEXIVE, 0);
	}
	else fcache.set_feature(id, feature_cache::REFLEXIVE, 0);
      }
      // other languages
      else fcache.set_feature(id, feature_cache::REFLEXIVE, 0);
    }
  
    unsigned int r=fcache.get_feature(id, feature_cache::REFLEXIVE);
    TRACE(6,L"   is reflexive " + util::int2wstring(id) + L" = " + util::int2wstring(r));
    return r;
  }

  // =============================================================
  // ================  Syntactic extraction functions ========
  // =============================================================

  //////////////////////////////////////////////////////////////////
  ///    Returns 1 when it is a definite noun phrase 
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::is_def_NP(const mention &m, feature_cache &fcache) const {

    int id=m.get_id();

    if (not fcache.computed_feature(id, feature_cache::DEF_NP)) {
      if (m.is_type(mention::NOUN_PHRASE)) {
	if (_Language==L"ENGLISH") {
	  wstring word=util::lowercase(m.get_it_begin()->get_form());
	  (word==L"the") ? fcache.set_feature(id, feature_cache::DEF_NP, 1) : fcache.set_feature(id, feature_cache::DEF_NP, 0);
	}
	else if (_Language==L"SPANISH" or _Language==L"CATALAN") {
          map<wstring,wstring> msd = _POS_tagset->get_msd_features_map(m.get_it_begin()->get_tag());
          if (msd[L"pos"]==L"determiner" and msd[L"type"]==L"article") 
            fcache.set_feature(id, feature_cache::DEF_NP, 1);
          else 
            fcache.set_feature(id, feature_cache::DEF_NP, 0);
        }
	// other languages
	else fcache.set_feature(id, feature_cache::DEF_NP, 0);
      }
      else fcache.set_feature(id, feature_cache::DEF_NP, 0);
    }  

    unsigned int r=fcache.get_feature(id, feature_cache::DEF_NP);
    TRACE(6,L"   is defNP " + util::int2wstring(id) + L" = " + util::int2wstring(r));
    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns 1 when it is a demonstrative noun phrase 
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::is_dem_NP(const mention &m, feature_cache &fcache) const {

    int id=m.get_id();

    if (not fcache.computed_feature(id, feature_cache::DEM_NP)) {
      if (m.is_type(mention::NOUN_PHRASE)) {
	if (_Language==L"ENGLISH") {
	  wstring word=util::lowercase(m.get_it_begin()->get_form());
	  (en_demostrative_re.search(word)) ? fcache.set_feature(id, feature_cache::DEM_NP, 1) : fcache.set_feature(id, feature_cache::DEM_NP, 0);
	}
	else if (_Language==L"SPANISH" or _Language==L"CATALAN") {
          map<wstring,wstring> msd = _POS_tagset->get_msd_features_map(m.get_it_begin()->get_tag());
          if (msd[L"pos"]==L"determiner" and msd[L"type"]==L"demonstrative") 
            fcache.set_feature(id, feature_cache::DEM_NP, 1);
          else 
            fcache.set_feature(id, feature_cache::DEM_NP, 0);
	}
	// other languages
	else fcache.set_feature(id, feature_cache::DEM_NP, 0);
      }
      else fcache.set_feature(id, feature_cache::DEM_NP, 0);
    }  

    unsigned int r=fcache.get_feature(id, feature_cache::DEM_NP);
    TRACE(6,L"   is demNP " + util::int2wstring(id) + L" = " + util::int2wstring(r));
    return r;
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns true when they share the maximal noun phrase 
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_constit::share_maximal_NP(const mention &m1,  const mention &m2, std::vector<mention> &mentions, feature_cache &fcache) const {

    int id1=get_maximal_NP(m1, mentions, fcache);
    int id2=get_maximal_NP(m2, mentions, fcache);

    bool r = (id1 == id2);
    TRACE(6,L"   share maximal NP = "+wstring(r? L"yes" : L"no"));
    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns 1 when it is a maximal noun phrase 
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::is_maximal_NP(const mention &m, std::vector<mention> &mentions, feature_cache &fcache) const {

    int id=m.get_id();
    int id1=get_maximal_NP(m, mentions, fcache);

    bool r = (id == id1);
    TRACE(6,L"   is maximal NP " + util::int2wstring(id) + L" = "+wstring(r? L"yes" : L"no"));
    return r;
  }
  
  //////////////////////////////////////////////////////////////////
  ///    Returns 1 when it is an indefinite noun phrase 
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::is_indef_NP(const mention &m, feature_cache &fcache) const {

    int id=m.get_id();

    if (not fcache.computed_feature(id, feature_cache::INDEF_NP)) {
      if (m.is_type(mention::NOUN_PHRASE)) {
	if (_Language==L"ENGLISH") {
	  wstring word=util::lowercase(m.get_it_begin()->get_form());
	  (en_indefinite_re.search(word)) ? fcache.set_feature(id, feature_cache::INDEF_NP, 1) : fcache.set_feature(id, feature_cache::INDEF_NP, 0);
	}
	else if (_Language==L"SPANISH" or _Language==L"CATALAN") {
          map<wstring,wstring> msd = _POS_tagset->get_msd_features_map(m.get_it_begin()->get_tag());
          if (msd[L"pos"]==L"determiner" and msd[L"type"]==L"indefinite") 
            fcache.set_feature(id, feature_cache::INDEF_NP, 1);
          else 
            fcache.set_feature(id, feature_cache::INDEF_NP, 0);
	}
	// other languages
	else fcache.set_feature(id, feature_cache::INDEF_NP, 0);
      }
      else fcache.set_feature(id, feature_cache::INDEF_NP, 0);
    }  

    unsigned int r=fcache.get_feature(id, feature_cache::INDEF_NP);
    TRACE(6,L"   is indefNP " + util::int2wstring(id) + L" = " + util::int2wstring(r));
    return r;      
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns 1 when it is a noun and is not a maximal noun phrase 
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_constit::is_embedded_noun(const mention &m, std::vector<mention> &mentions, feature_cache &fcache) const {

    int id=m.get_id();

    if (not fcache.computed_feature(id, feature_cache::EMBEDDED_NOUN)) {
      
      ((m.is_type(mention::NOUN_PHRASE) or m.is_type(mention::PROPER_NOUN)) and is_maximal_NP(m, mentions, fcache)==0) ?
	fcache.set_feature(id, feature_cache::EMBEDDED_NOUN, 1) : fcache.set_feature(id, feature_cache::EMBEDDED_NOUN, 0);
    }

    unsigned int r = fcache.get_feature(id, feature_cache::EMBEDDED_NOUN);
    TRACE(6,L"   is embedded noun " + util::int2wstring(id) + L" = " + util::int2wstring(r));
    return r;      
  }

  ///////////////////////////////////////////////////////////////////////////////
  ///    Returns true if they satisfy positive conditions of Binding Theory 
  ///////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_constit::binding_pos(const mention &m1, const mention &m2, bool ccommand, feature_cache &fcache) const {

    bool r;
    if (m1.get_n_sentence()!=m2.get_n_sentence() or nested(m1,m2)) 
      r = false;

    else 
      r = ccommand and is_reflexive(m2, fcache)==1 and agreement(m1,m2, fcache)==1 and animacy(m1,m2, fcache); 

    TRACE(6,L"   binding pos = "+wstring(r? L"yes" : L"no"));
    return r;
  }

  ///////////////////////////////////////////////////////////////////////////////
  ///    Returns true if they satisfy negative conditions of Binding Theory 
  ///////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_constit::binding_neg(const mention &m1, const mention &m2, bool ccommand, feature_cache &fcache) const {

    bool r;
    if (m1.get_n_sentence()!=m2.get_n_sentence() or nested(m1,m2))
       r = false;

    else if (m2.is_type(mention::PRONOUN))
      r = ccommand and is_reflexive(m2, fcache)==0;

    else
      r = ccommand;

    TRACE(6,L"   binding neg = "+wstring(r? L"yes" : L"no"));
    return r;
}
  
  /////////////////////////////////////////////////////////////////////////////
  ///    Returns a wstring with the arguments (concatenation of [0..5] or M) 
  ///    and a wstring with the predicates of the mention 
  /////////////////////////////////////////////////////////////////////////////

  void relaxcor_fex_constit::get_arguments(const mention &m, wstring &args, wstring &preds, feature_cache &fcache) const {
    int id=m.get_id();

    if (not fcache.computed_feature(id, feature_cache::ARGUMENTS)) {
      paragraph::const_iterator s=m.get_sentence();
      const sentence::predicates &vpreds=s->get_predicates();
      const dep_tree &dpt=s->get_dep_tree();
      dep_tree::const_iterator n=dpt.get_node_by_pos(m.get_head().get_position());
      
      for (sentence::predicates::const_iterator it=vpreds.begin(); it!=vpreds.end(); it++) {

	wstring arg =get_argument(it, n, s);
	vector<wstring> matches;

	if (arg_re.search(arg,matches)) {
	  /// [0..5] or M -> size=1
	  preds += util::int2wstring(s->get_predicate_number(it->get_position()));
	  args += matches[0].substr(matches[0].size()-1);
	}
      }

      vector<wstring> argsv;
      argsv.push_back(args);
      argsv.push_back(preds);

      fcache.set_feature(id, feature_cache::ARGUMENTS, argsv);
    }
    else {
      args=fcache.get_feature(id, feature_cache::ARGUMENTS).at(0);
      preds=fcache.get_feature(id, feature_cache::ARGUMENTS).at(1);
    }

    TRACE(7,L"      arguments and predicates: "+m.get_head().get_form()+L" "+util::int2wstring(m.get_pos_begin())+L":"+util::int2wstring(m.get_pos_end())+L"  args=["+args+L"]   preds=["+preds+L"]" );
  }


  ///////////////////////////////////////////////////////////////////////////// 
  ///    Returns true if both mentions have the same argument type 
  ///    (0, 1, 2, M or other)
  /////////////////////////////////////////////////////////////////////////////  

  bool relaxcor_fex_constit::same_args(bool same_sentence, const wstring &a1, const wstring &a2, relaxcor_model::Tfeatures &ft, feature_cache &fcache) const {

    bool r=false;
    if (same_sentence) {
      if ((ft[ID(L"RCF_I_SRL_ARG_0")] and ft[ID(L"RCF_J_SRL_ARG_0")]) 
          or (ft[ID(L"RCF_I_SRL_ARG_1")] and ft[ID(L"RCF_J_SRL_ARG_1")]) 
          or (ft[ID(L"RCF_I_SRL_ARG_2")] and ft[ID(L"RCF_J_SRL_ARG_2")])
          or (ft[ID(L"RCF_I_SRL_ARG_M")] and ft[ID(L"RCF_J_SRL_ARG_M")])) 
	r = true;
      else {
	r = false;
	for (wstring::const_iterator it=a1.begin(); it!=a1.end() and !r; it++)
	  r = a2.find(*it) != string::npos;
      }
    }

    TRACE(6,L"   same args = "+wstring(r? L"yes" : L"no"));
    return r;
  }



  ///////////////////////////////////////////////////////////////////////////// 
  ///    Returns true if both mentions share the same predicate  
  ///    (0, 1, ...)
  /////////////////////////////////////////////////////////////////////////////  

  bool relaxcor_fex_constit::same_preds(bool same_sentence, const wstring &p1, const wstring &p2, feature_cache &fcache) const {

    bool found=false;
    if (same_sentence)
      for (wstring::const_iterator it=p1.begin(); it!=p1.end() and !found; it++)
	found = p2.find(*it) != string::npos;

    TRACE(6,L"   same preds = "+wstring(found? L"yes" : L"no"));
    return found;
  }


  // =============================================================
  // ================  Semantic extraction functions =============
  // =============================================================

  ///////////////////////////////////////////////////////////////////////////////
  ///    Returns 1 when they are close and separated by the verb "to be"
  ///////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_constit::separated_by_verb_is(const mention &m1, const mention &m2, vector<mention> &mencions, feature_cache &fcache) const {
    bool r = false;
    if (m1.get_n_sentence()==m2.get_n_sentence() and not nested(m1,m2) 
        and is_maximal_NP(m1,mencions, fcache) and is_maximal_NP(m2,mencions, fcache)) 
      r = (m2.get_pos_begin()>m1.get_pos_end()) ? verb_is_between(m1,m2) : verb_is_between(m2,m1);

    TRACE(6,L"   separated by verb 'to be' = "+wstring(r? L"yes" : L"no"));
    return r;
  }

  ///////////////////////////////////////////////////////////////////////////////
  ///    Returns 1 when they match the semantic class
  ///////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_constit::sem_class_match(const mention &m1, const mention &m2, feature_cache &fcache) const {
    mention::SEMmentionType sem_class1=extract_semclass(m1, fcache);
    mention::SEMmentionType sem_class2=extract_semclass(m2, fcache);

    bool r = (sem_class1==sem_class2 or 
             (sem_class1==mention::PER and sem_class2==mention::MALE) or
             (sem_class1==mention::PER and sem_class2==mention::FEMALE) or
             (sem_class2==mention::PER and sem_class1==mention::MALE) or
             (sem_class2==mention::PER and sem_class1==mention::FEMALE));
    TRACE(6,L"   semclass match = "+wstring(r? L"yes" : L"no"));
    return r;
  }

  ///////////////////////////////////////////////////////////////////////////////
  ///    Returns true if it has semantic type equal to type
  ///////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_constit::is_semantic_type(const mention &m, const wstring &type, feature_cache & fcache) const {

    mention::SEMmentionType sclass=extract_semclass(m, fcache);

    bool r = false;
    if (type==L"person")
      r = sclass==mention::PER or sclass==mention::MALE or sclass==mention::FEMALE;
    else if (type==L"organization")
      r = sclass==mention::ORG;
    else if (type==L"location")
      r =  sclass==mention::GEO;
    else if (type==L"other")
      r = sclass==mention::OTHER;

    TRACE(7,L"      is semtype "+type+L" = "+wstring(r? L"yes" : L"no"));
    return r;
  }

  ///////////////////////////////////////////////////////////////////////////////
  ///    Returns true if both are person or both are not 
  ///////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_constit::animacy(const mention &m1, const mention &m2, feature_cache &fcache) const {

    //return (is_semantic_type(m1,L"person") and is_semantic_type(m2,L"person")) or 
    //       (!is_semantic_type(m1,L"person") and !is_semantic_type(m2,L"person"));

    // differs from Sapena,12
    // I use a more restricted definition
    bool r = (is_semantic_type(m1,L"person",fcache) and is_semantic_type(m2,L"person",fcache));
    TRACE(6,L"   animacy = " + wstring(r? L"yes" : L"no"));
    return r;
  }

  ///////////////////////////////////////////////////////////////////////////////
  ///    Returns true if they are semantically incompatible
  ///////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_constit::incompatible(const mention &m1, const mention &m2, feature_cache &fcache) const {
    bool r = nested(m1,m2) 
             and (is_possessive(m1, fcache) or is_possessive(m2, fcache)) 
             and (!m1.is_type(mention::PRONOUN) or !m2.is_type(mention::PRONOUN));

    TRACE(6,L"   incompatible = " + wstring(r? L"yes" : L"no"));
    return r;
  }

  
  //////////////////////////////////////////////////////////////////////////////
  ///    Returns a vector of wstrings with the roles of non-numerial arguments 
  ///    of the mention 
  ///    (LOC,EXT,TMP,DIS,ADV,NEG,MOD,CAU,PNC,MNR,DIR,PRD)  
  //////////////////////////////////////////////////////////////////////////////

  void relaxcor_fex_constit::get_roles(const mention &m, vector<wstring>& roles, feature_cache &fcache) const {

    int id=m.get_id();

    if (not fcache.computed_feature(id, feature_cache::ROLES)) {

      paragraph::const_iterator s=m.get_sentence();
      sentence::predicates vpreds=s->get_predicates();
      const dep_tree &dpt=s->get_dep_tree();
      dep_tree::const_iterator n=dpt.get_node_by_pos(m.get_head().get_position());
      
      for (sentence::predicates::const_iterator it=vpreds.begin(); it!=vpreds.end(); it++) {
	wstring arg =get_argument(it, n, s);
	vector<wstring> matches;
	
	if (role_re.search(arg))
	  /// LOC,EXT,TMP,DIS,ADV,NEG,MOD,CAU,PNC,MNR,DIR,PRD -> size=3
	  roles.push_back(arg.substr(arg.size()-3));
      }
      fcache.set_feature(id, feature_cache::ROLES, roles);
    }

    roles = fcache.get_feature(id, feature_cache::ROLES);
    TRACE(6,L"   get roles "+util::int2wstring(id)+L" = ["+util::vector2wstring(roles,L",")+L"]");
  }

  //////////////////////////////////////////////////////////////////////////////
  ///    Returns true if both mentions have the same role type       
  ////////////////////////////////////////////////////////////////////////////// 

  bool relaxcor_fex_constit::same_roles(const vector<wstring> &r1, const vector<wstring> &r2, feature_cache &fcache) const {
    bool found=false;
    
    for (vector<wstring>::const_iterator it1=r1.begin(); it1!=r1.end() and !found; it1++)
      for (vector<wstring>::const_iterator it2=r2.begin(); it2!=r2.end() and !found; it2++)
	found = (*it1).compare(*it2)==0;

    return found;
  }


  void relaxcor_fex_constit::read_countries_capitals(const wstring &fname) {
    wifstream f;
    util::open_utf8_file(f, fname);

    if (f.fail()) ERROR_CRASH(L"Error opening capitals file "+fname);
    wstring line;

    while (getline(f,line)) {
      wistringstream sin; sin.str(line);
      wstring country; sin>>country;
      wstring capital;

      while (sin>>capital) {
        _Capitals[capital]=country;
        _Countries.insert(make_pair(country,capital));
      }
    }

    f.close();
  }

  void relaxcor_fex_constit::read_gpe_regexps(const wstring &fname) {
    wifstream f;
    util::open_utf8_file(f, fname);

    if (f.fail()) ERROR_CRASH(L"Error opening gpe_regexps file "+fname);
    wstring line;
   
    while (getline(f,line))
      _GPE_regexps.push_back(freeling::regexp(line));
    
    f.close();
  }

  void relaxcor_fex_constit::read_pairs(const wstring &fname, map<wstring, wstring> &m) {
    wifstream f;
    util::open_utf8_file(f, fname);
    if (f.fail()) ERROR_CRASH(L"Error opening file "+fname);
    wstring line;

    while (getline(f,line)) {
      wistringstream sin; sin.str(line);
      wstring key; sin>>key;
      wstring val; sin>>val;
      m[key]=val;
    }

    f.close();
  }

  void relaxcor_fex_constit::read_same_names(const wstring &fname, map<wstring, vector<unsigned int> > &m) {
    wifstream f;
    util::open_utf8_file(f, fname);
    if (f.fail()) ERROR_CRASH(L"Error opening file "+fname);
    wstring line;

    // two names are the same entity if they have the same id
    // two names can belong to different entities
    while (getline(f,line)) {
      wistringstream sin; sin.str(line);
      wstring name;
      unsigned int id=0;

      while (sin>>name)
        m[name].push_back(id);

      id++;
    }

    f.close();
  }

  // ===================================================
  // ================  morphological auxiliar functions
  // ===================================================

  wstring relaxcor_fex_constit::drop_det(const mention &m) const {
    sentence::const_iterator it=m.get_it_begin();
    wstring first=util::lowercase(m.get_it_begin()->get_form());

    wstring A = (_Det_words.find(first)!=_Det_words.end())? L"" : m.get_it_begin()->get_form();

    for (it++; it != m.get_it_end(); it++) 
      A = (A==L"")?  A = it->get_form() : A = A + L" " + it->get_form();

    return A;
  }
  
  // ===================================================

  wstring relaxcor_fex_constit::compute_term(const mention &m) const {
    
    if (_Labels.find(L"NC")==_Labels.end() or _Labels.find(L"PN")==_Labels.end() or _Labels.find(L"ADJ")==_Labels.end())
      ERROR_CRASH(L"compute_term. Error: NC, PN or ADJ labels not defined in config file");

    bool term_token=true;
    wstring A;

    if (_Language == L"ENGLISH" or _Language == L"SPANISH") {  

      size_t pfirst = m.get_it_begin()->get_position();
      sentence::const_iterator it=m.get_it_head();
      A = it->get_form();
      while ( it->get_position() > pfirst and term_token) {
        it--;

	if (_Labels.find(L"NC")->second.search(it->get_tag()) or
	    _Labels.find(L"PN")->second.search(it->get_tag()) or
	    _Labels.find(L"ADJ")->second.search(it->get_tag())) 
	  A = it->get_form() + L" " + A;  
	else
	  term_token=false;

      }
    }

    if (_Language == L"SPANISH") {
      // In Spanish, term tokens can also be in the right of the head
      sentence::const_iterator it=m.get_it_head();
      it++;
      term_token = true;
      while (it!=m.get_it_end() and term_token) {
	if (_Labels.find(L"NC")->second.search(it->get_tag()) or
	    _Labels.find(L"PN")->second.search(it->get_tag()) or
	    _Labels.find(L"ADJ")->second.search(it->get_tag())) 
	  A = A + L" " + it->get_form();  
	else
	  term_token=false;

        it++;
      }
    }

    TRACE(7,L"   compute term " + util::int2wstring(m.get_id())+L" = "+A);
    return A;
  }

  // ===================================================

  wstring relaxcor_fex_constit::string_merge(const mention &m, bool clean) const {

    wstring A=L"";

    if (_Labels.find(L"PN") == _Labels.end())
	ERROR_CRASH(L"Error: PN label not defined in config file");

    if (_Labels.find(L"PN")->second.search(m.get_it_begin()->get_tag())){
      for (sentence::const_iterator it=m.get_it_begin(); it!=m.get_it_end(); it++)
	A = (A==L"")? it->get_form() : A+L"_"+it->get_form();
      A = util::lowercase(A);
    }
 
    else if (clean) {
      sentence::const_iterator it=m.get_it_begin();
      for (it++; it!=m.get_it_end(); it++)
	A = (A==L"")? it->get_form() : A+L"_"+it->get_form();
      A = util::lowercase(A);
    }

    else {
      for (sentence::const_iterator it=m.get_it_begin(); it!=m.get_it_end(); it++)
	A = (A==L"")? it->get_form() : A+L"_"+it->get_form();
    }

    TRACE(7,L"      alias: string merge = "+A);
    return A;
  }

  // ===================================================

  unsigned int relaxcor_fex_constit::geo_match(const mention &m1, const mention &m2) const {
    TRACE(7,L"      alias: geo match");

    if (m1.get_type() != mention::PROPER_NOUN and m2.get_type() != mention::PROPER_NOUN)
      return 2;


    // WARNING in CoNLL countries and gentilicies are not coreferent!! 

    /* wstring A=m1.get_head().get_form();
       wstring B=m2.get_head().get_form();    

    // if m1 is location and m2 is not a location (then it can be a nationality)
    // given that NERC is not as accurated as required I comment this condition
    //if (_Labels.find(L"LOC")->second.search(m1.get_it_begin()->get_tag()) and
    //    !_Labels.find(L"LOC")->second.search(m2.get_it_begin()->get_tag())) {
    
      if (_Countries.find(A) != _Countries.end() and
	  _Nationalities.find(util::lowercase(B)) != _Nationalities.end()) 
	return (_Nationalities.find(util::lowercase(B))->second == A) ? 1 : 0;

    // if m2 is location and m1 is not a location (then it can be a nationality)
    //if (_Labels.find(L"LOC")->second.search(m2.get_it_begin()->get_tag()) and
    //	!_Labels.find(L"LOC")->second.search(m1.get_it_begin()->get_tag())) {
 
      if (_Countries.find(B) != _Countries.end() and
	  _Nationalities.find(util::lowercase(A)) != _Nationalities.end())
	return (_Nationalities.find(util::lowercase(A))->second == B) ? 1 : 0;
    */

    // case of coreferent GPEs

    wstring A=m1.value();
    wstring B=m2.value();

    // A is the country
    if (_Countries.find(A) != _Countries.end()) {
      //find a GPE expression matching B as nationality or A as country
      //EX: "Spanish Govenment", "Government of Spain" 
      vector<wstring> match;
      
      for( vector<freeling::regexp>::const_iterator it=_GPE_regexps.begin();
	   it != _GPE_regexps.end(); it++) {
	if (it->search(util::lowercase(B), match)) {
	  if (_Nationalities.find(match[0])->second == A) return 1; 
	  if (match[0]==util::lowercase(A)) return 1;
	  return 0;
	}
      }
    }
    // B is the country
    else if (_Countries.find(B) != _Countries.end()) {
      //find the GPE expression matching A as nationality
      vector<wstring> match;
    
      for( vector<freeling::regexp>::const_iterator it=_GPE_regexps.begin();
	   it != _GPE_regexps.end(); it++) {
	if (it->search(util::lowercase(A), match)) {
	  if (_Nationalities.find(match[0])->second == B) return 1; 
	  if (match[0]==util::lowercase(B)) return 1;
	  return 0;
	}
      }
    }

    if (m1.get_type() != mention::PROPER_NOUN or m2.get_type() != mention::PROPER_NOUN)
      return 2;

    // if m1 and m2 are the same both proper names
    if (A == B) return 1;

    // WARNING in CoNLL countries and capitals do not corefer!!
    /*
    // if one is capital of the other
    if ((_Capitals.find(A)!=_Capitals.end() and _Capitals.find(A)->second==B) or
	(_Capitals.find(B)!=_Capitals.end() and _Capitals.find(B)->second==A))
      return 1;
    // if one is a capital and the other is a country
    if ((_Capitals.find(A)!=_Capitals.end() or _Countries.find(A)!=_Countries.end()) and
	(_Capitals.find(B)!=_Capitals.end() or _Countries.find(B)!=_Countries.end()))
      return 0;
    */
    return 2;
  }

  // ===================================================

  vector<wstring> relaxcor_fex_constit::split_words(const wstring &s) const {
 
    vector<wstring> v;

    if (is_acronym(s)) {
      for (unsigned int i=0; i<s.length(); i++) 
	if (s[i]!= '.' and s[i]!='_' and s[i]!='&') v.push_back(s.substr(i,1));
    }
    else {
      v=util::wstring2vector(s, L"_");
      // Delete INFIX components
      for (vector<wstring>::iterator it=v.begin(); it!=v.end(); it++)
	//if (_AcroTerms.find(L"INFIX")->second.search(*it) v.erate(it);
        if (*it==L"&") v.erase(it);
    }

    TRACE(7,L"      alias: split words("+s+L") = ["+util::vector2wstring(v,L",")+L"]");
    return v;
  }

  // ===================================================

  bool relaxcor_fex_constit::is_acronym(const wstring &s) const {
    bool r = acronym_re1.search(s) or acronym_re2.search(s);
    TRACE(7,L"      alias: is acronym "+s+L" = "+wstring(r?L"yes":L"no"));
    return r;
  }

  // ===================================================

  unsigned int relaxcor_fex_constit::acronym_of(const vector<wstring> &A, const vector<wstring> &B) const {
    TRACE(7,L"      alias: acronym of");

    if(_AcroTerms.find(L"INFIX")==_AcroTerms.end() or
       _AcroTerms.find(L"SUFFIX")==_AcroTerms.end())
      ERROR_CRASH(L"Error: AcroTerms INFIX or SUFFIX not defined in config file. ");

    if (A.size() > B.size()) return 0;

    unsigned int j=0;

    for (unsigned int i=0; i<A.size(); i++) {

      // the ith letter in A match the initial letter of the ith word in B
      if (j<B.size() and A[i]==B[j].substr(0,1))
	j++;
      else 
	return 0;

      unsigned int sfw=0;
      // length of a sequence of functional words must be no longer than 1
      while (j<B.size() and _AcroTerms.find(L"INFIX")->second.search(B[j])) {
	sfw++; j++;
      }
      if (sfw>1) return 0;

    }
    // B can terminate with special words denoting ORGs
    if (j==B.size()-1 and _AcroTerms.find(L"SUFFIX")->second.search(B[j])) 
	return 1;
    
    return j==B.size() ? 1 : 0;

  }

  // ===================================================

  unsigned int relaxcor_fex_constit::initials_match(const vector<wstring> &A, const vector<wstring> &B) const {
    TRACE(7,L"      alias: initials match");

    if (A.size() == 0 or B.size() == 0) return 0;
    
    if (initial_letter_re1.search(A[0]) and initial_letter_re2.search(A[A.size()-1])) {
      
      unsigned int i=0;
      while (i<A.size() and initial_letter_re1.search(A[i]) and A[i].substr(0,1) == B[i].substr(0,1)) 
	i++; 

      if (i<A.size() and initial_letter_re1.search(A[i]) and A[i].substr(0,1) != B[i].substr(0,1)) return 0;

      if (i<A.size() and !initial_letter_re1.search(A[i])) {

	while (i<A.size() and A[i]==B[i]) i++;
	
	return (i==A.size()) ? 1 : 0;
      }

      return 1; // this is the same case as both being the same acronym with dots
    }

    return 2;
  }

  // ===================================================

  double relaxcor_fex_constit::lex_dist(const wstring &a, const wstring &b) const {    

    double ld = 0.0;

    if (a != b) {
      // levenshtein distance
      unsigned int ml = max(a.length(), b.length());
      ld = levenshtein(a,b) / ml;
    }

    TRACE(7,L"      alias: lex dist = "+util::double2wstring(ld));
    return ld;
  }

  // ===================================================

  unsigned int relaxcor_fex_constit::nick_name_match(const wstring &a, const wstring &b) const {
    
    unsigned int state=2;
    if (_Nicks.find(a)!=_Nicks.end() and _Nicks.find(b)!=_Nicks.end())
      
      for (vector<unsigned int>::const_iterator ita = _Nicks.find(a)->second.begin(); ita != _Nicks.find(a)->second.end() and state==2; ita++) {

	for (vector<unsigned int>::const_iterator itb = _Nicks.find(b)->second.begin(); itb != _Nicks.find(b)->second.end() and state==2; itb++)
	  if ( *ita == *itb ) state = 1;

	if ( state != 1 ) state = 0;
      }

    TRACE(7,L"      alias: nick name match = "+util::int2wstring(state));
    return state;
  }

  // ===================================================

  unsigned int relaxcor_fex_constit::forenames_match(const vector<wstring> &A, const vector<wstring> &B) const {

    unsigned int state=2;

    // find the maximum substring which are forenames ie.: Mary_Josephine = MaryJo = Mary = Josephine
    unsigned int n,m;

    for (n=0; n<A.size() and state==2; n++) {
      wstring strA = subvector2wstring(A,0,A.size()-n-1,L"_");
      
      for (m=0; m<B.size() and state==2; m++) {
	wstring strB = subvector2wstring(B,0,B.size()-m-1,L"_");

	if (_Forenames.find(strA)!=_Forenames.end() and _Forenames.find(strB)!=_Forenames.end()) {

	  for (vector<unsigned int>::const_iterator ita = _Forenames.find(strA)->second.begin(); ita != _Forenames.find(strA)->second.end() and state==2; ita++) {

	    for (vector<unsigned int>::const_iterator itb = _Forenames.find(strB)->second.begin(); itb != _Forenames.find(strB)->second.end() and state==2; itb++) {
	      if ( *ita == *itb ) state = 1;
	    
	    }
	    if ( state != 1 ) state = 0;
	  }
	 
	  unsigned int n1=A.size()-n, m1=B.size()-m;
	  while (n1<A.size() and m1<B.size() and state==1) {
	    if (A[n1] != B[m1]) state=0;
	    n1++; m1++;
	  }
	}
      }
    }

    TRACE(7,L"      alias: forename match = "+util::int2wstring(state));
    return state;
  }

  // ===================================================

  wstring relaxcor_fex_constit::subvector2wstring(const vector<wstring>&v, unsigned int i, unsigned int j, const wstring& sep) const {
    wstring res = v[i];
    for (unsigned int k=i+1; k<=j; k++)
      res = res + sep + v[k];
    return res;
  }

  // ===================================================

  unsigned int relaxcor_fex_constit::first_name_match(const wstring &a, const vector<wstring> &B) const {

    bool found = false;
    for (vector<wstring>::const_iterator it = ++B.begin(); it!=B.end() and not found; it++)
      found = (a == (*it));

    unsigned int r = (found ? 1 : 2);
    TRACE(7,L"      alias: first names match = "+util::int2wstring(r));
    return r;
  }

  // ===================================================

  double relaxcor_fex_constit::levenshtein(const wstring &a, const wstring &b) const {

    // if one string is empty then return the length of the other
    if (a.length() == 0) return b.length();
    if (b.length() == 0) return a.length();

    vector< vector<double> > M(a.length()+1, vector<double>(b.length()+1));

    // initialize distance matrix
    // first row = 0 .. length_a
    // first col = 0 .. length_b
    // rest = 0
    for (unsigned int i=0; i<=a.length(); i++) {
      for(unsigned int j=0; j<=b.length(); j++) {
	M[i][j]=0;
	M[0][j]=j;
      }
      M[i][0]=i;
    }
    
    // char-by-char processing
    for (unsigned int i=1; i<=a.length(); i++) 
      for(unsigned int j=1; j<=b.length(); j++) {
	
	// substitution cost
	int cost = (a.substr(i-1,1) == b.substr(j-1,1)) ? 0 : 1;

	M[i][j] = min( M[i-1][j]+1, min( M[i][j-1]+1, M[i-1][j-1]+cost ) );
      }
    
    return M[a.length()][b.length()];
  }

  // ===================================================
  // for Spanish, Catalan, English

  unsigned int relaxcor_fex_constit::get_number(const mention &m) const {
    
    // values for 'number' (0-unknown, 1-sing, 2-bi, 3-plural  4-invariant)

    if (m.is_type(mention::COMPOSITE)) return 3;
    
    wstring tag=m.get_head().get_tag();
    
    // a 1-term mention (Proper Name, Pronoun, Noun)
    sentence::const_iterator it=m.get_it_begin();
    if (++it==m.get_it_end()) {

      if (_Language==L"SPANISH" or _Language==L"CATALAN") {
	// EAGLE Labels
	wstring num = extract_msd_feature(tag,L"num");

	if (num==L"-") return 0;
	if (num==L"s") return 1;
	if (num==L"p") return 3;	
	if (num==L"c") return 4; 
      }
    }
    // a multi-term mention (Noun Phrase)
    else {
      wstring word_head = m.get_head().get_form();
      sentence::const_iterator pos_head = m.get_it_head();

      if (_Language==L"SPANISH" or _Language==L"CATALAN") {
	// EAGLE Labels
	wstring num = L"-";

	// the head is a proper noun
	if (_Labels.find(L"PN")->second.search(tag)) {
	  // if it is the first token I assume that it is singular
	  if (pos_head==m.get_it_begin())
	    num=L"s";
	  else {
	    sentence::const_iterator before_head=--pos_head;
	    num=extract_msd_feature(before_head->get_tag(),L"num");
	  }
     	}
	else 
	  num = extract_msd_feature(tag,L"num");
	  
	if (num==L"-") return 0;
	if (num==L"s") return 1;
	if (num==L"p") return 3;	
	if (num==L"c") return 4;
      }
      else if (_Language==L"ENGLISH") {

	// the head is a proper noun (NNP or NNPS cannot occur in freeling)
	if (_Labels.find(L"PN")->second.search(tag)) {

	  // if it is the first token I assume that it is singular
	  if (pos_head==m.get_it_begin()) return 1;
	  else {
	    sentence::const_iterator before_head=--pos_head;
	    wstring prev_tag=before_head->get_tag();

	    // if previous token is a singular noun or proper noun
	    if (_Labels.find(L"NCS")->second.search(prev_tag) or _Labels.find(L"PN")->second.search(prev_tag)) return 1;
	    // if previous token is a plural noun
	    if (_Labels.find(L"NCP")->second.search(prev_tag)) return 3;

	    wstring prev_word=before_head->get_form();

	    // if previous token is a singular det
	    if (en_det_singular_re.search(prev_word)) return 1;
	    // if previous token is a plural det
	    if (en_det_plural_re.search(prev_word)) return 3;
	    return 0;
	  }
	}
      }    
    }
    
    // for 1-term English mentions
    if (_Language==L"ENGLISH") {
	// common nouns (NNP or NNPS cannot occur in freeling) and proper nouns in the case of 1-term
	if (_Labels.find(L"NCS")->second.search(tag) or _Labels.find(L"PN")->second.search(tag)) return 1;
	if (_Labels.find(L"NCP")->second.search(tag)) return 3;
	// proper nouns
	if (m.is_type(mention::PROPER_NOUN)) { 
	  // EAGLE Labels
	  wstring num = extract_msd_feature(tag,L"num");
	    
	  if (num==L"-") return 0;
	  if (num==L"s") return 1;
	  if (num==L"p") return 3;	
	  if (num==L"n") return 4; 
	}
	// pronouns
	wstring word=m.get_head().get_lc_form();
	if (_Prons_feat.find(word)!=_Prons_feat.end()) {
	  if (_Prons_feat.find(word)->second.find(L"num")->second == L"s") return 1;
	  if (_Prons_feat.find(word)->second.find(L"num")->second == L"p") return 3;
	  if (_Prons_feat.find(word)->second.find(L"num")->second == L"a") return 4; //new with respect to Sapena's version
	}
	// otherwise
	return 0;
    }

    return 0;
  }

  // ===================================================
  ///  Extract number digit from a EAGLES (spanish) PoS tag

  //  wstring relaxcor_fex_constit::extract_number(const wstring &tag) {
  //
  //    map<wstring,wstring> msd = _POS_tagset->get_msd_features_map(tag);
  //    map<wstring,wstring>::const_iterator p=msd.find(L"num");
  //    if (p!=msd.end())
  //      return p->second;
  //    else 
  //      return L"-";
  //
  //    // EAGLE Labels                           
  //    //if (tag[0]==L'A' or tag[0]==L'D' or tag[0]==L'P') return tag[4];
  //    //else if (tag[0]==L'N') return tag[3];
  //    //else return L'-';
  //  }

  // ===================================================
  // for Spanish, Catalan, English

  unsigned int relaxcor_fex_constit::get_gender(const mention &m, feature_cache &fcache) const {
    
    // values for 'gender' (0-unknown, 1-m, 2-f, 3-c 4-n) 

    if (m.is_type(mention::COMPOSITE)) return 3;
    
    // SAME FOR ALL LANGUAGES (?) ...
    
    TRACE(7,L"      get_gender for "+m.get_head().get_lc_form()+L" type="+util::int2wstring(m.get_type()));

    // the head is a pronoun
    if (m.is_type(mention::PRONOUN)) {
      wstring gen=L"-";
      wstring head_word=m.get_head().get_lc_form();

      if (_Prons_feat.find(head_word)!=_Prons_feat.end()) {
	wstring gender =_Prons_feat.find(head_word)->second.find(L"gen")->second;

        TRACE(7,L"      gender for pronoun "+head_word+L"="+gender);

	if (gender == L"-") return 0;
	if (gender == L"m") return 1;
	if (gender == L"f") return 2;
	if (gender == L"a") return 3;
	if (gender == L"n") return 4;
      }
    }

    // the head is a proper noun starting with a title ("Mr.", "Marqués", ...)
    // given the confusion between different NE types, I applied this to any proper noun
    if (m.is_type(mention::PROPER_NOUN)) {
    
      // being a proper name, the mention consists of one token
      // split it into different tokens
      wstring gen=L"-";
      vector<wstring> m_vec=util::wstring2vector(m.get_head().get_form(),L"_");
      
      // find the gender when the first token is a title
      if (_Titles.find(m_vec[0])!=_Titles.end()) {
	gen=_Titles.find(m_vec[0])->second;
      }

      if (gen==L"m") return 1;
      if (gen==L"f") return 2;
      if (gen==L"a") return 3;
      gen=L"-";

      // find the gender when the first token is person name
      if (is_semantic_type(m,L"person",fcache) and (_Person_Names.find(m_vec[0])!=_Person_Names.end())) {
	  gen=_Person_Names.find(m_vec[0])->second;
      }

      if (gen==L"m") return 1;
      if (gen==L"f") return 2;
      if (gen==L"a") return 3;
    }

    // DIFFERENT FOR EACH LANGUAGE

    else if (_Language==L"SPANISH" or _Language==L"CATALAN") {
      if (m.is_type(mention::PROPER_NOUN)) {
	  if (is_semantic_type(m,L"organization",fcache)) return 2;
          else if (is_semantic_type(m,L"location",fcache)) return 3; 
          else if (is_semantic_type(m,L"other",fcache)) return 0;
      }
    }
    else if (_Language==L"ENGLISH")
      if (m.is_type(mention::PROPER_NOUN) and (is_semantic_type(m,L"organization",fcache) or is_semantic_type(m,L"location",fcache) or is_semantic_type(m,L"other",fcache))) return 4;

    // add other outputs for NE types different to PER for other languages

    // the mention is neither a pronoun nor a proper noun
    if (_Language==L"SPANISH" or _Language==L"CATALAN") {
	wstring tag=m.get_head().get_tag();
	wstring gen = extract_msd_feature(tag,L"gen");

	if (gen==L"-") return 0;
	if (gen==L"m") return 1;
	if (gen==L"f") return 2;	
	if (gen==L"c") return 3;
	if (gen==L"n") return 4;
	return 0;
    }
    else if (_Language==L"ENGLISH") {
      mention::SEMmentionType sclass=extract_semclass(m, fcache);

	if (sclass==mention::PER) return 3;
	if (sclass==mention::MALE) return 1;
	if (sclass==mention::FEMALE) return 2;
        return 4;	
    }
    
    return 0;
  }

  // ===================================================

  //  wstring relaxcor_fex_constit::extract_gender(const wstring &tag) {
  //    map<wstring,wstring> msd = _POS_tagset->get_msd_features_map(tag);
  //    map<wstring,wstring>::const_iterator p=msd.find(L"gen");
  //    if (p!=msd.end())
  //      return p->second;
  //    else 
  //      return L"-";
  // 
  //    //if (tag[0]==L'N' and tag[1]==L'C') return tag[2];
  //    //else if (tag[0]==L'A' or tag[0]==L'D' or tag[0]==L'P') return tag[3];
  //    //else return L'-';
  //  }
 
  // ===================================================

  mention::SEMmentionType relaxcor_fex_constit::extract_semclass(const mention &m, feature_cache &fcache) const {

    mention::SEMmentionType sclass= mention::OTHER;  

    if (not fcache.computed_feature(m.get_id(),feature_cache::SEM_CLASS)) {
      const word &w = m.get_head();
      wstring tag = w.get_tag();

      // a proper noun
      if (_Labels.find(L"PN")->second.search(tag)) {
        if (tag.length()>=6) {
          if      (tag.substr(4,2) == L"SP") sclass = mention::PER;
         else if (tag.substr(4,1) == L"O") sclass = mention::ORG;
          else if (tag.substr(4,1) == L"G") sclass = mention::GEO;
        }
      }

      // a pronoun
      else if (_Labels.find(L"PRON")->second.search(tag)) {
	wstring head_word=w.get_lc_form();
	
	if (_Prons_feat.find(head_word)!=_Prons_feat.end()) {
          wstring gender =_Prons_feat.find(head_word)->second.find(L"gen")->second;	    
          if (gender == L"m") sclass = mention::MALE;
          if (gender == L"f") sclass = mention::FEMALE;
          if (gender == L"n") sclass = mention::NOTPER;
          if (gender == L"a") sclass = mention::PER;
	}
      }
      
      // otherwise
      else if (_Labels.find(L"NC")->second.search(tag)) {
	wstring sense=L"";
	const list<pair<wstring,double> > &ls = w.get_senses();
	if (not ls.empty()) {
	  sense = ls.begin()->first;
	  
	  // Finding a semantic class depends on the order of searching. The confusion 
          // is relevant in the case of person, male and female
	  // First I will look for all the classes and later on i will decide the final class
	  vector<bool> is_class(5,false);
	  
	  isa(sense, is_class);

	  if (is_class[mention::MALE]) sclass=mention::MALE;
	  else if (is_class[mention::FEMALE]) sclass=mention::FEMALE;
	  else if (is_class[mention::PER]) sclass=mention::PER;
	  else if (is_class[mention::ORG]) sclass=mention::ORG;
	  else if (is_class[mention::GEO]) sclass=mention::GEO;
	}
      }
      fcache.set_feature(m.get_id(), feature_cache::SEM_CLASS, sclass);
    }

    TRACE(7,L"      extract semclass = "+util::int2wstring(sclass));
    return (mention::SEMmentionType) fcache.get_feature(m.get_id(), feature_cache::SEM_CLASS);
  }

  // ===================================================

  void relaxcor_fex_constit::isa(const wstring& s, vector<bool>& is_class) const {

    // look for the semantic classes of its senses
    list<wstring> senses=_Semdb->get_sense_words(s);

    for (list<wstring>::const_iterator it1=senses.begin(); it1!=senses.end(); it1++) {

      if (_Sem_classes.find(L"male")->second.second.search(*it1)) is_class[mention::MALE]=true;
      if (_Sem_classes.find(L"female")->second.second.search(*it1)) is_class[mention::FEMALE]=true;
      if (_Sem_classes.find(L"person")->second.second.search(*it1)) is_class[mention::PER]=true;
      if (_Sem_classes.find(L"organization")->second.second.search(*it1)) is_class[mention::ORG]=true;
      if (_Sem_classes.find(L"location")->second.second.search(*it1)) is_class[mention::GEO]=true;
    }

    // look for the semantic classes of its parents
    sense_info si = _Semdb->get_sense_info(s);
    wstring parents_str=si.get_parents_string();
    vector<wstring> parents= util::wstring2vector(parents_str,L":");

    for (vector<wstring>::const_iterator it=parents.begin(); it!=parents.end(); it++) {

      if (_Sem_classes.find(L"male")->second.first==(*it)) is_class[mention::MALE]=true;
      if (_Sem_classes.find(L"female")->second.first==(*it)) is_class[mention::FEMALE]=true;
      if (_Sem_classes.find(L"person")->second.first==(*it)) is_class[mention::PER]=true;
      if (_Sem_classes.find(L"organization")->second.first==(*it)) is_class[mention::ORG]=true;
      if (_Sem_classes.find(L"location")->second.first==(*it)) is_class[mention::GEO]=true;
    }

    for (vector<wstring>::const_iterator it=parents.begin(); it!=parents.end(); it++) 
      isa(*it, is_class);
  }


  // ===================================================
  ///  Extract given MSD feature (gen,num,person) from given PoS tag

  wstring relaxcor_fex_constit::extract_msd_feature(const wstring &tag, const wstring &feature) const {
    map<wstring,wstring> msd = _POS_tagset->get_msd_features_map(tag);
    map<wstring,wstring>::const_iterator p=msd.find(feature);
    if (p!=msd.end())
      return p->second;
    else 
      return L"-";
  }


  // ===================================================
  ///  Extract number digit from a EAGLES (spanish) PoS tag

  //  wstring relaxcor_fex_constit::extract_person(const wstring &tag) {
  //
  //    map<wstring,wstring> msd = _POS_tagset->get_msd_features_map(tag);
  //    map<wstring,wstring>::const_iterator p=msd.find(L"person");
  //    if (p!=msd.end())
  //      return p->second;
  //    else 
  //      return L"-";
  //
  //    // EAGLE Labels                           
  //    //if (tag[0]==L'P') return tag[2];
  //    //else return L'-';
  //  }

  // ===================================================
  // ================  syntactic auxiliar functions
  // ===================================================

  int relaxcor_fex_constit::get_maximal_NP(const mention &m,  std::vector<mention> &mentions, feature_cache &fcache) const {

    int id=m.get_id();

    if (not fcache.computed_feature(id, feature_cache::MAXIMAL_NP)) {
      bool stop=false;
      int start=m.get_pos_begin();
      int end=m.get_pos_end();
      int id1=m.get_id();

      for (int i=id-1; i>=0 and !stop; i--) {
	int start_i=mentions[i].get_pos_begin();
	int end_i=mentions[i].get_pos_end();
	if (start_i<=start and end_i>=end) id1=mentions[i].get_id();
	else if (end_i<start) stop=true;
      }

      fcache.set_feature(id, feature_cache::MAXIMAL_NP, id1);
    }

    int r = fcache.get_feature(id, feature_cache::MAXIMAL_NP); 
    TRACE(7,L"      get_maximal_NP " + util::int2wstring(id)+ L" = "+ util::int2wstring(r));
    return r;
  }

  const wstring& relaxcor_fex_constit::get_argument(sentence::predicates::const_iterator it_pred, dep_tree::const_iterator it_n, paragraph::const_iterator s) const {
    TRACE(7,L"      get_argument: "+it_n->get_word().get_form());
 
      size_t p=it_n->get_word().get_position();
      if (it_pred->has_argument(p)) 
	return it_pred->get_argument_by_pos(p).get_role();

      // look at her parent (e.g: a mention included in a modifier PP)
      // single_subsumed_mention: -> "his" is single_subsumed by "his wife" 
      // and it has not the argument of its parent
      if (not it_n.is_root()) {
	it_n=it_n.get_parent();
	p=it_n->get_word().get_position();
	if (not it_n.is_root() and not s->is_predicate(p)) 
	  return get_argument(it_pred, it_n, s);
      }
      
      return argument::EMPTY_ROLE;
  }

  // ===================================================
  // ================  semantic auxiliar functions
  // ===================================================

  bool relaxcor_fex_constit::verb_is_between(const mention &m1, const mention &m2) const {

    if (_Labels.find(L"VERB")==_Labels.end())
      ERROR_CRASH(L"Error: VERB labels not defined in config file");

    // if they are not in the same sentence or they are not close enough
    if ( m1.get_n_sentence()!=m2.get_n_sentence() or
         nested(m1, m2) or intersected(m1, m2) or
	 (m2.get_pos_begin()>m1.get_pos_begin() and m2.get_pos_begin()-m1.get_pos_end()>=5) or
	 (m1.get_pos_begin()>m2.get_pos_begin() and m1.get_pos_begin()-m2.get_pos_end()>=5))
      return false;

    bool found=false;
    wstring expr(L"");

    if ((_Language == L"ENGLISH") or (_Language == L"SPANISH")) {
      const freeling::regexp & re = (_Language == L"ENGLISH")? en_verb_be_re : es_verb_be_re;

      for (sentence::const_iterator it=m1.get_it_end(); it!=m2.get_it_begin() and !found; it++) {
	sentence::const_iterator it1 = it; it1++;

	found = (_Labels.find(L"VERB")->second.search(it->get_tag()) and 
		 re.search(it->get_form()) and
		 not _Labels.find(L"VERB")->second.search(it1->get_tag()));
      }
    }
    else if (_Language == L"CATALAN") {
      for (sentence::const_iterator it=m1.get_it_end(); it!=m2.get_it_begin() and !found; it++) {
	sentence::const_iterator it1 = it; it1++;
	sentence::const_iterator it2 = it1; it2++;

	found = (_Labels.find(L"VERB")->second.search(it->get_tag()) and 
		 ((cat_verb_be_re1.search(it->get_form()) and
		   _Labels.find(L"VERB")->second.search(it1->get_tag())) or
		  (cat_verb_be_re2.search(it->get_form()) and
		   (it1->get_form() == L"ser") and
		   not _Labels.find(L"VERB")->second.search(it2->get_tag()))));
      }
    }

    TRACE(7,L"      verb 'to be'_between "+m1.get_head().get_form()+L" "+util::int2wstring(m1.get_n_sentence())+L":"+util::int2wstring(m1.get_pos_begin())+L":"+util::int2wstring(m1.get_pos_end())+L" "+m2.get_head().get_form()+L" "+util::int2wstring(m2.get_n_sentence())+L":"+util::int2wstring(m2.get_pos_begin())+L":"+util::int2wstring(m2.get_pos_end())+L" = "+wstring(found?L"yes":L"no"));
    return found;
  }
  
  //////////////////////////////////////////////////////////////////
  ///    Extract the configured features for a pair of mentions 
  //////////////////////////////////////////////////////////////////

  void relaxcor_fex_constit::extract_pair(mention &m1, mention &m2, relaxcor_model::Tfeatures &ft,vector<mention> &mentions, feature_cache &fcache) const {

    if (_Active_features & RCF_SET_STRUCTURAL) get_structural(m1, m2, ft, fcache);    
    if (_Active_features & RCF_SET_LEXICAL)    get_lexical(m1, m2, ft, fcache);
    if (_Active_features & RCF_SET_MORPHO)     get_morphological(m1, m2, ft, mentions, fcache);
    if (_Active_features & RCF_SET_SYNTACTIC)    get_syntactic(m1, m2, ft, mentions, fcache);
    if (_Active_features & RCF_SET_SEMANTIC)    get_semantic(m1, m2, ft, mentions, fcache);
   
  }
  

  ////////////////////////////////////////////////////////////////// 
  ///    Extract the configured features for all mentions  
  //////////////////////////////////////////////////////////////////

  relaxcor_fex::Mfeatures relaxcor_fex_constit::extract(vector<mention> &mentions) const {

    relaxcor_fex::Mfeatures M;
    feature_cache fcache;

    for (vector<mention>::iterator m1=mentions.begin()+1; m1!=mentions.end(); ++m1) {
      TRACE(4,L"Extracting all pairs for mention "+util::int2wstring(m1->get_id())+L" ("+m1->value()+L") " + m1->get_head().get_form() + L" " + m1->get_head().get_lemma() + L" " + m1->get_head().get_tag());
      clock_t t0 = clock();  // final time
      
      for (vector<mention>::iterator m2=mentions.begin(); m2!=m1; ++m2) {	
	wstring mention_pair = util::int2wstring(m1->get_id());
	mention_pair += L":";
	mention_pair += util::int2wstring(m2->get_id());

	TRACE(5,L"PAIR: "+mention_pair+L" "+m1->get_head().get_form()+L":"+m2->get_head().get_form());
	
        extract_pair(*m1, *m2, M[mention_pair], mentions, fcache);
      }

      clock_t t1 = clock();  // final time
      TRACE(4,L"Extraction time for mention "+util::int2wstring(m1->get_id())+L" ("+m1->value()+L"): "+util::double2wstring(double(t1-t0)/double(CLOCKS_PER_SEC)));
    }

    return M;
  }

} // namespace
