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
 * \file   factory-tag.h
 * \brief  Static methods for creating tagging components
 * \author Xavier Carreras
 */

#ifndef TREELER_FACTORY_TAG_H
#define TREELER_FACTORY_TAG_H

#include <string>
#include <list>
#include <iostream>
#include <typeinfo>

#include "treeler/tag/tag.h"
#include "treeler/control/factory-base.h"

using namespace std;

namespace treeler {

  namespace control {    

    template <>
    class Factory<TagSymbols> {
    public:
      typedef Factory<DepSymbols> BaseFactory;
      
      static string name() {
	return "TagSymbols";
      }

      static void usage(std::ostream& o) {
	o << "TagSymbols options:" << endl;
	dump_options(o, " ");
	o << endl;
      }

      static void dump_options(std::ostream& o, const string& prf="") {
	BaseFactory::dump_options(o, prf);
	o << prf << "--dict-tags        : path to dictionary file for tagset" << endl;
      }

      static void configure(TagSymbols& symbols, Options& options, bool verbose=true, std::ostream& o = cerr) {
	BaseFactory::configure(symbols, options, verbose, o);
	string path;
	BaseFactory::load_dictionary<TagSymbols>(options, verbose, o, symbols.d_tags, "dict-tags", "dict", "tags.map");
      }
    };


    template <>
    class Factory<TagSeq> {
    public:
      static string name() {
	return "TagSequence";
      }
    };



    /** 
     * \brief A factory of \c PartTag objects
     * \ingroup control 
     */    
    template <>
    class Factory<PartTag> {
    public:
      static string name() {
	return "PartTag";
      }

      static void usage(std::ostream& o) {
	o << "PartTag options:" << endl; 
	dump_options(o, " ");
	o << endl;
      }
            
      static void dump_options(std::ostream& o, const string& prefix="") {
	o << prefix << "--L : number of labels" << endl;
      }

      static void configure(PartTag::Configuration& c, Options& options, int verbose=0, std::ostream& o = cerr) {
	options.get("L", c.L); 
      }
    };
    

    /**** Viterbi parser ****/
    /**
     *  \brief Factory to create \c Viterbi parsers
     *  \ingroup control 
     */    
    template <>
    class Factory<Viterbi> {
    public:
      static string name() {
	return "Viterbi";
      }

      static void usage(std::ostream& o) {
	o << "Viterbi options:" << endl; 
	dump_options(o, " ");
	o << endl;
      }
      
      static void dump_options(std::ostream& o, const string& prefix="") {
	o << prefix << "--L=<int>    : number of tags" << endl;
	o << prefix << "--type=<str> : type of inference" << endl;
	o << prefix << "               (viterbi, mbr)" << endl;
      }

      static void configure(Viterbi& parser, Options& options, int verbose=0, std::ostream& o = cerr) {
	configure(parser.config(), options, verbose, o); 
      }

      static void configure(Viterbi::Configuration& c, Options& options, int verbose=0, std::ostream& o = cerr) {
	c.L = -1;
	if(!options.get("L", c.L)) {
	  o << "Viterbi : please provide the number of tags" << endl; 
	  exit(1);
	}
	if (c.L<=0) {
	  o << "Viterbi : number tags must be higher than 0" << endl; 
	  exit(1); 
	}
	c.inf = "viterbi";
	options.get("inf", c.inf);
	if (c.inf!="viterbi" && c.inf!="mbr" && c.inf!="mbr-count") {
	  o << "Viterbi : wrong type of inference \"" << c.inf << "\"" << endl; 
	  exit(1);
	}
	c.gamma = 1.0;
	options.get("gamma", c.gamma);
	c.verbose = 0; 
	
	options.get("v", c.verbose);
      }
    };
   

    
    /**
     *  \brief Factory to create feature generators of type \c FGenTag
     *  \ingroup control 
     */    
    template <typename X, typename R, typename FIdx>
    class Factory<FGenTag<X,R,FIdx>> {
    public:
      static string name() {
	return "FGenTag<" + Factory<R>::name() + ">";
      }

      static void usage(std::ostream& o) {
	o << "FGenTag options: (no options)" << endl;
	o << endl;
      };

      static void configure(FGenTag<X,R,FIdx>& f, Options& options, bool verbose = true, std::ostream& o = cerr) {
	typename FGenTag<X,R,FIdx>::Configuration& config = f.config(); 
	options.get("backoff", config.backoff); 
      }
      
    };
    

    /**
     *  \brief Factory to create feature generators of type \c FGenTTag
     *  \ingroup control 
     */    
    template <typename X, typename R>
    class Factory<FGenTTag<X,R>> {
    public:
      static string name() {
	return "FGenTTag<" + Factory<R>::name() + ">";
      }

      static void usage(std::ostream& o) {
	o << "FGenTTag options: (no options)" << endl;
	o << endl;
      };

      static void configure(FGenTTag<X,R>& f, Options& options, bool verbose = true, std::ostream& o = cerr) {

      }
      
    };
          
    
  } // namespace control
} // namespace treeler


#endif /* TREELER_FACTORY_TAG_H */
