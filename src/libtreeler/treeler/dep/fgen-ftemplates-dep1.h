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
 * \file   fgen-ftemplates-dep1.h
 * \brief  Declaration of FGenFTemplatesDep1
 * \author Xavier Carreras
 */

#ifndef DEP_FGENFTEMPLATESDEP1_H
#define DEP_FGENFTEMPLATESDEP1_H

#include <string>
#include <list>

#include "treeler/base/feature-idx-v0.h"
#include "treeler/dep/dep-symbols.h"


/* Threshold for the token distance within a deependency */
#define TREELER_FTEMPLATES_TOKEN_DISTANCE_THRESHOLD 41

/* max #verbs intervening between a dependency */
#define TREELER_FTEMPLATES_DEP1_VCOUNT_MAX 4
/* max #punc intervening between a dependency */
#define TREELER_FTEMPLATES_DEP1_PCOUNT_MAX 4
/* max #conj intervening between a dependency */
#define TREELER_FTEMPLATES_DEP1_CCOUNT_MAX 4


namespace treeler {

  /** 
      @brief Basic static feature templates for tokens and first-order dependencies

      @author Xavier Carreras, Terry Koo
  */
  class FGenFTemplatesDep1 {
  public:    
    //! Token features
    template <typename Sentence>
    static void phi_token(const Sentence& x, 
			  const int word_limit, const bool use_pos,  
			  std::list<FeatureIdx> F[]);

    //! Features for the imaginary root token 
    template <typename Sentence>
    static void phi_root_token(const Sentence& x, 
			       const int word_limit, const bool use_pos,  
			       std::list<FeatureIdx>& F);

    //! Features for a token context
    //! This function generates features for each token in
    //! a sentence.  The input/output set F is assumed to contain a set of atomic
    //! token features.  These features are replicated to yield a
    //! contextual representation of each token.  In addition, n-grams of
    //! parts of speech are added. 
    //! Currently n has to be <=2.
    template <typename Sentence>
    static void phi_token_context(const Sentence& x, const int n, const int word_limit, const bool use_pos, 
				  std::list<FeatureIdx> F[]);
    
    //! Dependency features
    template <typename Sentence>
    static void phi_dependency(const Sentence& x,
			       const int word_limit, const bool use_pos, 
			       std::list<FeatureIdx> F[]);    


    //! Dependency context features
    template <typename Sentence>
    static void phi_dependency_context(const Sentence& x, const int word_limit, const bool use_pos, 
				       std::list<FeatureIdx> F[]);

    template <typename Sentence>
    static void phi_dependency_distance(const Sentence& x, const int word_limit, const bool use_pos,
					std::list<FeatureIdx> F[]);

    template <typename Sentence>
      static void phi_dependency_between(const DepSymbols& symbols, 
					 const Sentence& x, const int word_limit, const bool use_pos,
					 std::list<FeatureIdx> F[]);
  

  };
}

#include "treeler/dep/fgen-ftemplates-dep1.tcc"

#endif /* DEP_FGENDEPBASIC_H */
