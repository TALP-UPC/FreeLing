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
 * \file   factory-base.h
 * \brief  Static methods for creating basic components
 * \author Xavier Carreras
 */

#ifndef TREELER_FACTORY_BASE_H
#define TREELER_FACTORY_BASE_H

#include <string>
#include <sstream>
#include <list>
#include <iostream>
#include <typeinfo>

using namespace std;

#include "treeler/base/basic-sentence.h"
#include "treeler/base/sentence.h"


namespace treeler {

  /** 
   * \brief Components for controlling the library
   * \ingroup control
   */
  namespace control {

    // command line option
    struct CLOption {
      string name; 
      stringstream help; 

      CLOption()
	: name(""), help()
      {}

      CLOption(const string& n0)
	: name(n0), help()
      {}

      CLOption(const string& n0, const string h0)
	: name(n0), help(h0)
      {}

    };
    
    void display_options(ostream & out, const list<CLOption>& options, const string& prefix = "--");
    

    /** 
     * \brief A generic factory 
     * \ingroup control 
     */    
    template <typename T>
    class Factory {
    public:

    };


    /** 
     */    
    template <typename T1, typename T2>
    class Factory<BasicSentence<T1,T2>> {
    public:
      static string name() {
	return "BasicSentence"; 
      }
    };

    /** 
     */    
    template <>
    class Factory<Sentence> {
    public:
      static string name() {
	return "SentenceV0"; 
      }
    };



        
  } // namespace control
} // namespace treeler


#endif /* TREELER_FACTORY_BASE_H */
