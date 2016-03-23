 /*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2014   TALP Research Center
 *                       Universitat Politecnica de Catalunya
 *
 *  This file is part of Treeler.
 *
 *  Treeler is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Treeler is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with Treeler.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   factory-dep.h
 * \brief  Static methods for creating dependency parsing components
 * \author Xavier Carreras
 */

#ifndef TREELER_FACTORY_DEP_H
#define TREELER_FACTORY_DEP_H

#include <string>
#include <list>
#include <iostream>
#include <typeinfo>

#include "treeler/dep/dep.h"
#include "treeler/dep/fgen-dep-v1.h"

using namespace std;

namespace treeler {

  namespace control {

    /** 
     * \brief A factory of \c DepSymbols objects
     * \ingroup control 
     */    
    template <>
    class Factory<DepSymbols> {
    public:
      static string name() {
	return "DepSymbols";
      }

      static void usage(std::ostream& o) {
	o << "DepSymbols options:" << endl; 
	dump_options(o, " ");
	o << endl;
      }
            
      static void dump_options(std::ostream& o, const string& prf="") {
	o << prf << "--dict              : path to directory of dictionaries" << endl;
	o << prf << "--dict-words        : path to dictionary file for words" << endl;
	o << prf << "--dict-lemmas       : path to dictionary file for lemmas" << endl;
	o << prf << "--dict-cpos         : path to dictionary file for coarse pos tags" << endl;
	o << prf << "--dict-fpos         : path to dictionary file for fine pos tags" << endl;
	o << prf << "--dict-morphos      : path to dictionary file for morpho tags" << endl;
	o << prf << "--dict-dependencies : path to dictionary file for dependency labels" << endl;
	o << prf << "--unlabeled         : will create a dictionary that ignores dependency labels" << endl;
      }

      template <typename SYM>
      static bool load_dictionary(Options& options,
				  bool verbose, std::ostream& o,
				  Dictionary& dict, 
				  const string& dict_option_name, 
				  const string& dir_option_name, 
				  const string& default_dict_name) {
	string path; 
	if (options.get(dict_option_name, path)) {
	  // do nothing
	}
	else if (options.get(dir_option_name, path)) {
	  path += "/" + default_dict_name; 
	}
	else { 
	  return false;
	}	
	if (verbose) {
	  o << "Factory " << Factory<SYM>::name() << " : loading \"" << path << "\" ... " << flush;
	}	
	dict.load(path); 
	if (verbose) {
	  o << dict.size() << " entries" << endl;
	}	  
	return true;		
      }

      template <typename SYM>
      static void configure(SYM& symbols, Options& options, bool verbose=true, std::ostream& o = cerr) {
	string path; 

	//call a factory for PosSymbols or load the tag map
	string dictdir = ".";
	string posfile = "";
	if(!options.get("fpos", posfile)) {
	  if (options.get("fdict", dictdir)) {
	    posfile = dictdir + "/fpos.map"; 
	  }
	  else if (options.get("dict", dictdir)) {
	    posfile = dictdir + "/fpos.map"; 
	  }
	  else {
	    o << "Factory " << Factory<SYM>::name() << " : please provide a dictionary file for pos tags" << endl; 
	    exit(1);
	  }      
	}
	symbols.load_tag_map(posfile);  

	// words
	if (not load_dictionary<SYM>(options, verbose, o, symbols.d_words, "dict-words", "dict", "words.map")) {
	  symbols.d_words.set_str_unknowns("_");
	}

	// lemmas
	if (not load_dictionary<SYM>(options, verbose, o, symbols.d_lemmas, "dict-lemmas", "dict", "lemmas.map")) {
	  symbols.d_lemmas.set_str_unknowns("_");
	}

	// coarse pos
	load_dictionary<SYM>(options, verbose, o, symbols.d_coarse_pos, "dict-cpos", "dict", "cpos.map");
	// fine pos
	load_dictionary<SYM>(options, verbose, o, symbols.d_fine_pos, "dict-fpos", "dict", "fpos.map");
	// morphos
	load_dictionary<SYM>(options, verbose, o, symbols.d_morpho_tags, "dict-morphos", "dict", "morphos.map");
	
	// deps
	int tmp=0; 
	if (options.get("unlabeled", tmp) and tmp==1) {
	  symbols.d_syntactic_labels.universal("_"); 
	}	
	else {
	  load_dictionary<SYM>(options, verbose, o, symbols.d_syntactic_labels, "dict-dependencies", "dict", "dependencies.map");
	}
      }     
    };


    template <typename T>
    class Factory<DepVector<T>> {
    public:
      static string name() {
	return "DepVector";
      }
    };

    
    /** 
     * \brief A factory of \c PartDep1 objects
     * \ingroup control 
     */    
    template <>
    class Factory<PartDep1> {
    public:
      static string name() {
	return "PartDep1";
      }

      static void usage(std::ostream& o) {
	o << "PartDep1 options:" << endl; 
	dump_options(o, " ");
	o << endl;
      }
            
      static void dump_options(std::ostream& o, const string& prefix="") {
	o << prefix << "--L : number of labels" << endl;
      }

      static void configure(PartDep1::Configuration& c, Options& options, std::ostream& o = cerr) {
	options.get("L", c.L); 
      }
    };


    /** 
     * \brief A factory of \c PartDep2 objects
     * \ingroup control 
     */    
    template <>
    class Factory<PartDep2> {
    public:
      static string name() {
	return "PartDep2";
      }

      static void usage(std::ostream& o) {
	o << "PartDep2 options:" << endl; 
	dump_options(o, " ");
	o << endl;
      }
            
      static void dump_options(std::ostream& o, const string& prefix="") {
	o << prefix << "--L : number of labels" << endl;
      }

      static void configure(PartDep2::Configuration& c, Options& options, std::ostream& o = cerr) {
	options.get("L", c.L); 
      }
    };


    
    /**** ProjDep1 parser ****/

    /** 
     * \brief A factory of \c ProjDep1 objects
     * \ingroup control 
     */    
    template <>
    class Factory<ProjDep1> {
    public:

      static string name() {
	return "ProjDep1"; 
      }

      static void usage(std::ostream& o) {
	o << "ProjDep1 options:" << endl; 
	dump_options(o, " ");
	o << endl;
      }
      
      static void dump_options(std::ostream& o, const string& prefix="") {
	o << prefix << "--L=<int>   : number of dependency labels (default 1)" << endl;
	o << prefix << "--multiroot : allow multiple roots in parsing" << endl;
      }

      static void configure(ProjDep1& p, Options& options, int verbose=0, std::ostream& o = cerr) {
	int tmp;
	ProjDep1::Configuration& c = p.configuration();
	c.multiroot = false;
	if (options.get("multiroot", tmp)) {
	  c.multiroot = tmp!=0;
	}
      }

      static void configure(ProjDep1::Configuration& c, Options& options, std::ostream& o = cerr) {
	int tmp;
	c.multiroot = false;
	if (options.get("multiroot", tmp)) {
	  c.multiroot = tmp!=0;
	}
	if (options.get("L", tmp)) {
	  if (tmp<=0) {
	    o << "Factory<ProjDep1> : --L must be a positive value (" << tmp << " was provided)" << endl; 
	    exit(1);
	  }
	  c.L = tmp; 
	}
      }
    };

    
    /**** ProjDep2 parser ****/

    /** 
     * \brief A factory of \c ProjDep2 objects
     * \ingroup control 
     */    
    template <>
    class Factory<ProjDep2> {
    public:
      static string name() {
	return "ProjDep1"; 
      }

      static void usage(std::ostream& o) {
	o << "ProjDep2 options:" << endl; 
	dump_options(o, " ");
	o << endl;
      }
      
      static void dump_options(std::ostream& o, const string& prefix="") {
	o << prefix << "--multiroot : allow multiple roots in parsing" << endl;
      }

      static void configure(ProjDep2& p, Options& options, int verbose=0, std::ostream& o = cerr) {
	if (verbose)
	  o << "Factory<ProjDep2> : configuring parser " << endl; 
	ProjDep2::Configuration& c = p.configuration(); 
	configure(c, options, verbose, o);
	if (verbose)
	  o << "Factory<ProjDep2> : L = " << c.L << " " << " multiroot " << c.multiroot << endl; 
      }

      static void configure(ProjDep2::Configuration& c, Options& options, int verbosei=0, std::ostream& o = cerr) {
	int tmp;
	c.multiroot = false;
	if (options.get("multiroot", tmp)) {
	  c.multiroot = tmp!=0;
	}
	if (options.get("L", tmp)) {
	  if (tmp<=0) {
	    o << "Factory<ProjDep2> : --L must be a positive value " << endl; 
	    exit(1);
	  }
	  c.L = tmp; 
	}
      }
    };

    
    namespace _internal {
      /** 
       * \brief Traits defining the default feature configuration for \c FGenDep according to the type of part
       */
      template <typename T>
	class FGenDepDefaults {
	static string fconf() { return ""; }
      };
      /** 
       * \brief Specialization of \c FGenDepDefaults for \c PartDep1
       */
      template <>
	class FGenDepDefaults<PartDep1> {
      public:
	static string fconf() { return "t|tc|d|dc|b|dist|+w|+p"; }
      };
      /** 
       * \brief Specialization of \c FGenDepDefaults for \c PartDep2
       */
      template <>
	class FGenDepDefaults<PartDep2> {
      public:
	static string fconf() { return "t|tc|d|dc|b|dist|+w|+p|o2"; }
      };
    }


    /**
     *  \brief Factory to create feature generators of type \c FGenDepV1
     *  \ingroup control 
     */
    template <typename X, typename R, typename FIdx>
    class Factory<FGenDepV1<X,R,FIdx>> {
    public:
      
      typedef FGenDepV1<X,R,FIdx> FGen;

      static string name() {
	return "FGenDepV1"; 
      }
      
      static void usage(std::ostream& o) {
	o << "FGenDepV1 options:" << endl;
	dump_options(o, " ");
	o << endl;
      };

      static void dump_options(std::ostream& o, const string& prefix) {
	o << prefix << "--fdict=<path> : pathname to feature-dictionary directory" << endl;
	o << prefix << "--fpos=<path>  : pathname to pos tag dictionary" << endl;
	o << prefix << "--fconf=\"<flag>|...\" : feature-configuration flags" << endl;
	o << prefix << "    t    : token features" << endl;
	o << prefix << "    tc   : token context features" << endl;
	o << prefix << "    d    : dependency features" << endl;
	o << prefix << "    dc   : dependency context features" << endl;
	o << prefix << "    b    : depenedency between features" << endl;
	o << prefix << "    dist : distance features" << endl;
	o << prefix << "    +w   : use all word information" << endl;
	o << prefix << "    w<N> : use only the top-N most frequent words" << endl;
	o << prefix << "    +p   : use POS information" << endl;
	o << prefix << "    ~p   : don't use POS information" << endl;
	o << prefix << "    o2   : second order features" << endl;
	o << prefix << "  (default \"" << _internal::FGenDepDefaults<R>::fconf() << "\")" << endl;	
      }

      static void configure(FGenDepV1<X,R,FIdx>& f, Options& options, bool verbose=true, ostream& o = std::cerr) {
	const string name = "Factory<FGenDepV1<X,R,FIdx>>"; 
	typename FGen::Configuration& config = f.config;

	config.L = -1;
	if(options.get("L", config.L)) {
	  if (config.L==0) {
	    o << name << " : --L=<int> must be positive" << endl; 
	    exit(-1);
	  }
	}
	else {
	  config.L = f.symbols.d_syntactic_labels.size(); 
	  if (config.L==0) config.L = 1; // unlabeled parsing
	}
	o << name << " : using L=" << config.L << " dependency labels" << endl; 

		
	config.dictdir = ".";
	config.posfile = "";
	if(!options.get("fpos", config.posfile)) {
	  if (options.get("fdict", config.dictdir)) {
	    config.posfile = config.dictdir + "/fpos.map"; 
	  }
	  else if (options.get("dict", config.dictdir)) {
	    config.posfile = config.dictdir + "/fpos.map"; 
	  }
	  else {
	    o << name << " : please provide a dictionary file for pos tags" << endl; 
	    exit(1);
	  }      
	}

	//if (verbose) o << name << " : loading tag lexicon from " << config.posfile << " ..." << flush; 
	//f.pos_symbols.load_tag_map(config.posfile);  
	//if (verbose) o << " ok " << endl;
	
	string fconf;
	if(!options.get("fconf", fconf)) {
	  if (verbose) 
	    o << name << " : using default feature configuration \""
	      << _internal::FGenDepDefaults<R>::fconf() << "\"" << endl;
	  fconf = _internal::FGenDepDefaults<R>::fconf();
	  options.set("fconf", fconf);
	}	
	if (verbose) 
	  cerr << name << " : feature conf \"" << fconf << "\"" << endl;	
	configure(config, options, verbose, o);
      }

      static void configure(typename FGen::Configuration& c, Options& options, bool verbose=true, ostream& o = std::cerr) {
	const string name = "Factory<FGenDep<X,Y>::Dep1>"; 
	int tmp;
	c.cutoff = 1;
	if(options.get("fcut", tmp)) {
	  c.cutoff = tmp;
	}	
	string conf;
	if(!options.get("fconf", conf)) {
	  o << name << " : please specify feature configuration" << endl;
	  exit(1);
	}	
	c.use_token = false;
	c.use_token_context = false;
	c.use_dependency = false;
	c.use_dependency_context = false;
	c.use_dependency_between = false;
	c.use_dependency_distance = false;
	
	c.word_limit = -1;
	c.use_pos = false;

	vector<string> flags;
	simpleTokenize(conf, flags, "|");
	for (vector<string>::const_iterator i = flags.begin();
	     i != flags.end();
	     ++i) {
	  if (*i == "t") {
	    if (verbose) o << name << " : activating token features" << endl;
	    c.use_token = true;
	  } else if (*i == "tc") {
	    if (verbose) o << name << " : activating token context features" << endl;
	    c.use_token_context = true;
	  } else if (*i == "d") {
	    if (verbose) o << name << " : activating dependency features" << endl;
	    c.use_dependency = true;
	  } else if (*i == "dc") {
	    if (verbose) o << name << " : activating dependency context features" << endl;
	    c.use_dependency_context = true;
	  } else if (*i == "dist") {
	    if (verbose) o << name << " : activating distance features" << endl;
	    c.use_dependency_distance = true;
	  } else if (*i == "b") {
	    if (verbose) o << name << " : activating between features" << endl;
	    c.use_dependency_between = true;
	  } else if (*i == "+w") {
	    if (verbose) o << name << " : using word information" << endl;
	    c.word_limit = -1;
	  } else if (i->c_str()[0] == 'w') {
	    c.word_limit = atoi(i->c_str() + 1);
	    if (verbose) o << name << " : using only top-" << c.word_limit << " words" << endl;
	  } else if (*i == "+p") {
	    if (verbose) o << name << " : using POS information" << endl;
	    c.use_pos = true;
	  } else if (*i == "~p") {
	    if (verbose) o << name << " : ignoring POS information" << endl;
	    c.use_pos = false;
	  }
	}
	if(c.word_limit == -1) { c.word_limit = INT_MAX; }
      }


    };


    
    /**
     *  \brief Factory to create feature generators of type \c FGenDep 
     *  \ingroup control 
     */
    template <typename X, typename R>
    class Factory<FGenDepV0<X,R>> {
    public:

      static string name() {
	return "FGenDepV0<" + Factory<R>::name() + ">"; 
      }
      
      static void usage(std::ostream& o) {
	o << name() << " options:" << endl;
	dump_options(o, " ");
	o << endl;
      };

      static void dump_options(std::ostream& o, const string& prefix) {
	o << prefix << "--L=<int>      : number of dependency labels" << endl;
	o << prefix << "--fdict=<path> : pathname to feature-dictionary directory" << endl;
	o << prefix << "--fpos=<path>  : pathname to pos tag dictionary" << endl;
	o << prefix << "--fconf=\"<flag>|...\" : feature-configuration flags" << endl;
	o << prefix << "    t    : token features" << endl;
	o << prefix << "    tc   : token context features" << endl;
	o << prefix << "    d    : dependency features" << endl;
	o << prefix << "    dc   : dependency context features" << endl;
	o << prefix << "    b    : depenedency between features" << endl;
	o << prefix << "    dist : distance features" << endl;
	o << prefix << "    +w   : use all word information" << endl;
	o << prefix << "    w<N> : use only the top-N most frequent words" << endl;
	o << prefix << "    +p   : use POS information" << endl;
	o << prefix << "    ~p   : don't use POS information" << endl;
	o << prefix << "    o2   : second order features" << endl;
	o << prefix << "  (default \"" << _internal::FGenDepDefaults<R>::fconf() << "\")" << endl;	
      }

      static void configure(FGenDepV0<X,R>& f, Options& options, bool verbose=true, ostream& o = std::cerr) {
	const string fname = "Factory<" + name() + ">"; 
	typename FGenDepV0<X,R>::Configuration& config = f.config;
	
	config.L = -1;
	if(options.get("L", config.L)) {
	  if (config.L==0) {
	    o << fname << " : --L=<int> must be positive" << endl;
	    exit(-1);
	  }
	}
	else {
	  config.L = f.symbols.d_syntactic_labels.size(); 
	  if (config.L==0) config.L = 1; // unlabeled parsing
	}
	if (verbose) o << fname << " : using L=" << config.L << " dependency labels" << endl; 

	{  // pos symbols info (not available in DepSymbols by default)
	  config.dictdir = ".";
	  config.posfile = "";	
	  if(!options.get("fpos", config.posfile)) {
	    if (options.get("fdict", config.dictdir)) {
	      config.posfile = config.dictdir + "/fpos.map"; 
	    }
	    else if (options.get("dict", config.dictdir)) {
	      config.posfile = config.dictdir + "/fpos.map"; 
	    }
	    else {
	      o << fname << " : please provide a dictionary file for pos tags" << endl; 
	      exit(1);
	    }      
	  }
	  
	  if (verbose) o << fname << " : loading tag lexicon from " << config.posfile << " ..." << flush; 
	  f.symbols.load_tag_map(config.posfile);  
	  if (verbose) o << " ok (" << f.symbols.d_fine_pos.size() << " entries)" << endl;
	  if (verbose) o << fname << " : using " << f.symbols.d_fine_pos.size() << " pos tags from symbols tables" << endl;
	}
	
	string fconf;
	if(!options.get("fconf", fconf)) {
	  if (verbose) 
	    o << fname << " : using default feature configuration \""
	      << _internal::FGenDepDefaults<R>::fconf() << "\"" << endl;
	  fconf = _internal::FGenDepDefaults<R>::fconf();
	  options.set("fconf", fconf);
	}	
	if (verbose) 
	  cerr << fname << " : feature conf \"" << fconf << "\"" << endl;	
	configure(config.config_dep1, options, verbose, o);
	configure(config.config_dep2, options, verbose, o);
      }

      static void configure(FGenBasicDep1::Configuration& c, Options& options, bool verbose=true, ostream& o = std::cerr) {
	const string fname = "Factory<" + name() + ">"; 
	int tmp;
	c.cutoff = 1;
	if(options.get("fcut", tmp)) {
	  c.cutoff = tmp;
	}	
	string conf;
	if(!options.get("fconf", conf)) {
	  o << fname << " : please specify feature configuration" << endl;
	  exit(1);
	}	
	c.use_token = false;
	c.use_token_context = false;
	c.use_dependency = false;
	c.use_dependency_context = false;
	c.use_dependency_between = false;
	c.use_dependency_distance = false;
	
	c.word_limit = -1;
	c.use_pos = false;
	c.use_o2 = false;

	vector<string> flags;
	simpleTokenize(conf, flags, "|");
	for (vector<string>::const_iterator i = flags.begin();
	     i != flags.end();
	     ++i) {
	  if (*i == "t") {
	    if (verbose) o << fname << " : activating token features" << endl;
	    c.use_token = true;
	  } else if (*i == "tc") {
	    if (verbose) o << fname << " : activating token context features" << endl;
	    c.use_token_context = true;
	  } else if (*i == "d") {
	    if (verbose) o << fname << " : activating dependency features" << endl;
	    c.use_dependency = true;
	  } else if (*i == "dc") {
	    if (verbose) o << fname << " : activating dependency context features" << endl;
	    c.use_dependency_context = true;
	  } else if (*i == "dist") {
	    if (verbose) o << fname << " : activating distance features" << endl;
	    c.use_dependency_distance = true;
	  } else if (*i == "b") {
	    if (verbose) o << fname << " : activating between features" << endl;
	    c.use_dependency_between = true;
	  } else if (*i == "+w") {
	    if (verbose) o << fname << " : using word information" << endl;
	    c.word_limit = -1;
	  } else if (i->c_str()[0] == 'w') {
	    c.word_limit = atoi(i->c_str() + 1);
	    if (verbose) o << fname << " : using only top-" << c.word_limit << " words" << endl;
	  } else if (*i == "+p") {
	    if (verbose) o << fname << " : using POS information" << endl;
	    c.use_pos = true;
	  } else if (*i == "~p") {
	    if (verbose) o << fname << " : ignoring POS information" << endl;
	    c.use_pos = false;
	  }
	}
	if(c.word_limit == -1) { c.word_limit = INT_MAX; }
      }

      static void configure(FGenBasicDep2::Configuration& c, Options& options, bool verbose=true, ostream& o = std::cerr) {
	const string fname = "Factory<FGenDep<X,Y>::Dep2>"; 
	string conf;
	if(!options.get("fconf", conf)) {
	  o << fname << " : please specify feature configuration" << endl;
	  exit(1);
	}
	c.use_o2 = false;
	c.use_trigrams = false;
	c.word_limit = -1;
	c.use_pos = false;
	c.on_demand = true;
	
	vector<string> flags;
	simpleTokenize(conf, flags, "|");
	for (vector<string>::const_iterator i = flags.begin();
	     i != flags.end();
	     ++i) {
	  if (*i == "o2") {
	    if (verbose) o << fname << " : activating second-order features" << endl;
	    c.use_o2 = true;
	  } else if (*i == "tri") {
	    if (verbose) o << fname << " : activating trigram features" << endl;
	    c.use_trigrams = true;
	  } else if (*i == "+w") {
	    if (verbose) o << fname << " : using word information" << endl;
	    c.word_limit = -1;
	  } else if (i->c_str()[0] == 'w') {
	    c.word_limit = atoi(i->c_str() + 1);
	    if (verbose) o << fname << " : using only top-" << c.word_limit << " words" << endl;
	  } else if (*i == "+p") {
	    if (verbose) o << fname << " : using POS information" << endl;
	    c.use_pos = true;
	  } else if (*i == "~p") {
	    if (verbose) o << fname << " : ignoring POS information" << endl;
	    c.use_pos = false;
	  }
	}
	if(c.word_limit == -1) { c.word_limit = INT_MAX; }           
      }

    };
    
  } // namespace control
} // namespace treeler


#endif /* TREELER_FACTORY_DEP_H */
