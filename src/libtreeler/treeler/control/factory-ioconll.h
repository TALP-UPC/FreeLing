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
 * \file   factory-ioconll.h
 * \brief  Static methods for creating IOCoNLL components
 * \author Xavier Carreras
 */

#ifndef TREELER_FACTORY_IOCONLL_H
#define TREELER_FACTORY_IOCONLL_H

#include <string>
#include <list>
#include <iostream>
#include <typeinfo>

#include "treeler/control/factory-dep.h"

using namespace std;

namespace treeler {

  namespace control {

    
    /** 
     * \brief A factory for creating IOCoNLL objects
     * 
     * 
     */
    template<typename Symbols, typename Format, typename CharType>
    class Factory<IOCoNLL<Symbols,Format,CharType>> {
    public:
      static string name() {
	return "IOCoNLL"; 
      }

      static void usage(std::ostream& o) {
	o << "IOCoNLL options" << endl;
	dump_options(o, " "); 
	o << endl;
      }

      static void dump_options(std::ostream& o, const string& prf="") {
	o << prf << "--padding=0|1         : add padding to output" << endl;
	o << prf << "--conllx              : set CoNLL-X format (dependency parsing)" << endl;
	o << prf << "--conll00             : set CoNLL-2000 format (chunking)" << endl;
	o << prf << "--conll08             : set CoNLL-2008 format (srl) using predicted columns" << endl;
	o << prf << "--conll08gold         : set CoNLL-2008 format (srl) using gold columns" << endl;
	o << prf << "--conll08gsyn         : set CoNLL-2008 format (srl) using gold columns for syntax " << endl;
	o << prf << "                         (and predicted columns for sentence features)" << endl;
	o << prf << "Fine-grained control (experimental):" << endl; 
	o << prf << "--ids=0|1              : column of token ids at the beginning" << endl;
	o << prf << "--offset=<int>         : offset for token ids (defaults to 1)" << endl;
	o << prf << "--word-column=<int>    : column for words" << endl;
	o << prf << "--lemma-column=<int>   : column for lemmas" << endl;
	o << prf << "--cpos-column=<int>    : column for coarse pos tags" << endl;
	o << prf << "--fpos-column=<int>    : column for coarse pos tags" << endl;
	o << prf << "--morpho-column=<int>  : column for morphologic tags" << endl;
	o << prf << "--sentence-end=<int>   : end of sentence block" << endl;
	o << prf << "--head-column=<int>    : column of syntactic heads" << endl;
	o << prf << "--dep-column=<int>     : column of syntactic label" << endl;
	o << prf << "--dependency-end=<int> : end of dependency block" << endl;
	o << prf << "--pred-ind=<bool>      : column of predicate indicators (Y/_)" << endl;
	o << prf << "--io-debug=0|1         : print debug information" << endl;
	//	o << prf << "--mapi=0|1     : map input values" << endl;
	//	o << prf << "--mapo=0|1     : map output values" << endl;

      }

      static void configure(IOCoNLL<Symbols,Format,CharType>& io, Options& options, int verbose=0, std::ostream& o = cerr) {
	if (verbose) o << "Factory " << name() << " : configuring IOCoNLL" << endl;
	options.get("padding", io.config.add_padding);
	io.config.map_input = true; 
	io.config.map_output = true; 
	options.get("io-debug", io.config.debug);
	options.get("mapi", io.config.map_input);
	options.get("mapo", io.config.map_output);
		
	Format& format = io.config.format; 
	int tmp=0;
	options.get("conllx", tmp); 
	if (tmp) {
	  io.config.ids = true;
	  io.config.offset = 1;
	  format.word = 0;  
	  format.lemma = 1;  
	  format.coarse_pos = 2; 
	  format.fine_pos = 3; 
	  format.morpho_tags = 4; 
	  format.head = 0;
	  format.syntactic_label = 1;
	}
	tmp = 0;
	options.get("conll00", tmp); 
	if (tmp) {
	  io.config.ids = false;
	  format.word = 0;  
	  format.lemma = -1;  
	  format.coarse_pos = -1; 
	  format.fine_pos = 1; 
	  format.morpho_tags = -1; 
	  format.head = -1;
	  format.syntactic_label = -1;
	}
	tmp = 0; 
	options.get("conll08", tmp); 
	if (tmp) {
	  io.config.ids = true;
	  io.config.offset = 1;
	  format.word = 0;  
	  format.lemma = 2;  
	  format.coarse_pos = 4; 
	  format.fine_pos = 4; 
	  format.morpho_tags = 6;
	  format.sentence_end = 6;
	  format.head = 1;
	  format.syntactic_label = 3;
	  format.dependency_end = 3;
	  format.predicate_indicator = true;
	}
	tmp = 0; 
	options.get("conll08gold", tmp); 
	if (tmp) {
	  io.config.ids = true;
	  io.config.offset = 1; 
	  format.word = 0;  
	  format.lemma = 1;  
	  format.coarse_pos = 3; 
	  format.fine_pos = 3; 
	  format.morpho_tags = 5;
	  format.sentence_end = 6;
	  format.head = 0;
	  format.syntactic_label = 2;
	  format.dependency_end = 3;
	  format.predicate_indicator = true;
	}
	tmp = 0; 
	options.get("conll08gsyn", tmp); 
	if (tmp) {
	  io.config.ids = true;
	  io.config.offset = 1; 
	  format.word = 0;  
	  format.lemma = 2;  
	  format.coarse_pos = 4; 
	  format.fine_pos = 4; 
	  format.morpho_tags = 6;
	  format.sentence_end = 6;
	  format.head = 0;
	  format.syntactic_label = 2;
	  format.dependency_end = 3;
	  format.predicate_indicator = true;
	}
	 
	options.get("ids", io.config.ids); 
	options.get("offset", io.config.offset); 
	options.get("word-column", format.word); 
	options.get("lemma-column", format.lemma); 
	options.get("fpos-column", format.fine_pos); 
	options.get("cpos-column", format.coarse_pos); 
	options.get("morpho-column", format.morpho_tags); 
	options.get("sentence-end", format.sentence_end); 
	options.get("head-column", format.head); 
	options.get("dep-column", format.syntactic_label); 
	options.get("dependency-end", format.dependency_end); 
	options.get("pred-ind", format.predicate_indicator); 
      }

    };





    
  } // namespace control
} // namespace treeler


#endif /* TREELER_FACTORY_IOCONLL_H */
