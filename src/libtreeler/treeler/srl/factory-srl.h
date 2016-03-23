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
 * \file   factory-srl.h
 * \brief  Factories for creating SRL components using command-line options
 * \author Xavier Carreras
 */

#ifndef TREELER_FACTORY_SRL
#define TREELER_FACTORY_SRL

#include "treeler/util/options.h"
#include "treeler/control/factory-base.h"

#include <algorithm>
#include <string>
#include <list>
#include <iostream>
#include <typeinfo>

///#include "treeler/srl/oracle-scores.h"
#include "treeler/control/factory-dep.h"
#include "treeler/srl/simple-parser.h"

namespace treeler {
  namespace control {

    template <>
    class Factory<srl::SRLSymbols> {
    public:
      typedef Factory<DepSymbols> BaseFactory; 

      static string name() {
        return "SRLSymbols";
      }

      static void usage(std::ostream& o) {
        o << "SRL Symbols options:" << endl; 
        dump_options(o, " ");
        o << endl;
      }

      static void dump_options(std::ostream& o, const string& prf="") {
        BaseFactory::dump_options(o, prf); 
        o << prf << "--dict-roles        : path to dictionary file for semantic role labels" << endl;
      }

      static void configure(srl::SRLSymbols& symbols, Options& options, bool verbose=true, std::ostream& o = cerr) {
        BaseFactory::configure(symbols, options, verbose, o); 
        string path; 

        // sem labels
	BaseFactory::load_dictionary<srl::SRLSymbols>(options, verbose, o, symbols.d_semantic_labels, "dict-roles", "dict", "roles.map");       
	string first_label = symbols.d_semantic_labels.map(0); 
	if (first_label != "_") {
	  ostringstream oss; 
	  oss << "in role dictionary, label 0 must be associated with null role \"_\", but it is associated with \"" << first_label << "\"";
	  TreelerException e("Factory<" + name() + ">", oss.str()); 
	  throw(e);
	}

        if (options.get("dict-fpos", path)) {
          symbols.load_tag_map(path);
	}
        else if (options.get("dict", path)) {
          symbols.load_tag_map(path+"/fpos.map");
	}
        //load the POS filter
        bool apply_pos_filter = false;
        options.get("apply-pos-filter", apply_pos_filter);
        if (apply_pos_filter){
          symbols.load_pos_filter(path+"/posfilter.map");
        }
      }           
    };


    template <>
    class Factory<srl::Sentence> {
    public:
      static string name() {
        return "SRLSentence";
      }
    };

    template <>
    class Factory<srl::PredArgSet> {
    public:
      static string name() {
	return "PredArgSet";
      }
    };


    template <>
    class Factory<srl::PartSRL> {
    public:
      static string name() {
        return "PartSRL";
      }
      
      static void usage(std::ostream& o) {
      }

      static void configure(srl::PartSRL::Configuration& t, Options& o, bool verbose = true) { 
      }

    };

    //configure the simple parser
    template <>
    class Factory<srl::SimpleParser> {
    public:
      static string name() {
        return "SimpleParser";
      }

      static void usage(std::ostream& o) {
	o << "SRL Simple Parser options:" << endl; 
	list<CLOption> olist; 
	olist.emplace_back("scope=direct|ancestor|all"); 
	olist.back().help << "syntactic scope of a predicate " << endl 
			  << "to select argument candidates"; 
	olist.emplace_back("apply-pos-filter", "weather to restrict roles with respect to PoS constraints");
	olist.emplace_back("v=<int>", "verbosity level");
	display_options(o, olist, " "); 
	o << endl; 
      }

      static void configure(srl::SimpleParser& t, Options& options, bool verbose = true, ostream& log = cerr) { 
        typedef srl::SimpleParser::Configuration ParserConfig;
        ParserConfig& config = t.config();
        configure(config, options, verbose, log);
      }
      
      static void configure(srl::SimpleParser::Configuration& c, Options& options, bool verbose = true, ostream& log = cerr) { 
        bool apply_pos_filter = false;
        if (options.get("apply-pos-filter", apply_pos_filter)) {
          c.apply_pos_filter = apply_pos_filter;
        }
        string scope;
        if (options.get("scope", scope)) {
          if (scope == "direct") {
	    c.scope = srl::SimpleParser::DIRECT;
	  } else if (scope == "ancestor"){
	    c.scope = srl::SimpleParser::ANCESTOR;
          } else if (scope == "all"){
            c.scope = srl::SimpleParser::ALL;
          } else {
	    if (verbose)
	      log << "Factory " << name() << " : please supply valid option for --scope (direct, ancestor, all)" << endl;
	    exit(0);
	  }
        }
	else {
	  scope = "ancestor (default value)"; 
	  c.scope = srl::SimpleParser::ANCESTOR;
	}
	if (verbose) {	  
	  cerr << "Factory " << name() << " : argument scope is " << scope << endl; 
	}

	options.get("v", c.verbose);
	options.get("b", c.score_blockwise);
	if (verbose and c.score_blockwise) {	  
	  log << "Factory " << name() << ": scoring blockwise" << endl;
	}
      }      
    };


    template <typename FIdx>
    class Factory<srl::FGenSRLV1<FIdx>> {
    public:
      static string name() {
        return "FGenSRLV1";
      }

      typedef typename srl::FGenSRLV1<FIdx>::Configuration FGenConfig;
      static void usage(std::ostream& o) {
	o << "SRL FGen options" << endl; 
	list<CLOption> olist; 
	olist.emplace_back("syn-offset", "parameter offset for syntactic features");
	Factory<typename srl::FGenSRLV1<FIdx>::Features::FTemplatesSRL >::dump_options(olist);	
	display_options(o, olist, " "); 
	o << endl; 
      }
      
      static void configure(srl::FGenSRLV1<FIdx>& t, Options& options, bool verbose = false) { 
	configure(t.config(), options, verbose);
      }
      
      static void configure(typename srl::FGenSRLV1<FIdx>::Configuration& c, Options& options, bool verbose = false) { 
	if (options.get("syn-offset", c.syn_offset) and verbose) {
	  cerr << "Factory " << name() << " : setting syn-offset to " << c.syn_offset << endl;
	}
	Factory<typename srl::FGenSRLV1<FIdx>::Features::FTemplatesSRL >::configure(c.ft_srl_config, options, verbose);
      }      
    };

    
    template <>
    class Factory<srl::FTemplatesSRL> {
    public:
      typedef srl::FTemplatesSRL::Configuration Config;
      static void dump_options(list<CLOption>& olist) {
	olist.emplace_back("fsrl-usyn", "sets all syntactic labels to NO_LABEL");
	olist.emplace_back("fsrl-path", "use or not path features");
      }
      
      static void configure(Config& c, Options& options, bool verbose = false) { 	
	options.get("fsrl-usyn", c.unlabeled_syn); 
	options.get("fsrl-path", c.path);  
      }
      
    };
    
  }

}

#endif
