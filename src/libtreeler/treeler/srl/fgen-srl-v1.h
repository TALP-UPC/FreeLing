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
 *  along with Treeler.  If not, see <http:://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   fgen-srl-v1.h
 * \brief  Standard features for SRL
 * \author 
 */

#ifndef SRL_FGENSRL_V1_H
#define SRL_FGENSRL_V1_H

#include <string>
#include "treeler/base/feature-vector.h"
#include "treeler/dep/fgen-dep-v1.h"
#include "treeler/srl/srl.h"

#include "treeler/srl/paths-defs.h"
#include "treeler/srl/part-srl0.h"

#include "treeler/srl/ftemplates-srl.h"

#include <iostream>

namespace treeler {

  namespace srl {
    /** 
	\brief A structure of features for an input pattern 
    */
    template <typename FIdx>
    class FeaturesSRLV1;

    /** 
     *  \brief Feature extractor for SRL
     */
    template <typename FIdx_par = FIdxBits>
    class FGenSRLV1 { 
    public:
      typedef FIdx_par FIdx;
      typedef srl::Sentence X;
      
      // a feature structure for an X instance
      typedef FeaturesSRLV1<FIdx> Features;

      struct Configuration {
	//! Number of SRL labels
	int L;
	int syn_offset; 

	typename Features::FTemplatesDep::Configuration ft_dep_config; 
	typename Features::FTemplatesSRL::Configuration ft_srl_config; 

	Configuration() 
	  : L(1), syn_offset(1)
	{}
      };
      
    private:
      const SRLSymbols& symbols;
      Configuration _config;


    public:

      FGenSRLV1(const SRLSymbols& sym) : 
	symbols(sym)
      {
	_config.L = sym.d_semantic_labels.size() + 1;
	assert(_config.L > 1);
      }
      
      int spaces() const { 
	assert(_config.L > 1);
	//we take 4 spaces
	//3 for the first order dep feats {head tok, mod tok, dep}
	//and the 4th for the srl feats
	//but currently only one space is being used
	return 4*_config.L + 1; 
      };


      // creates features for x in f
      void features(const X& x, Features& f) const; 

      Configuration& config(){
	return _config;
      }

    };


    template <typename FIdx>
    void FGenSRLV1<FIdx>::features(const X& x, Features& f) const {
      assert(f._x==NULL);
      f._x = &x;
      f._symbols = &symbols;
      f._spaces = spaces();
      f._syn_offset = _config.syn_offset;
      f.ft_dep_config = _config.ft_dep_config;
      f.ft_srl_config = _config.ft_srl_config;
    }



    // the features for all parts of a pattern
    template <typename FIdx>
    class FeaturesSRLV1 {
    public:
      friend class FGenSRLV1<FIdx>; 

      typedef srl::Sentence X; 
      typedef FTemplatesDep1V1 FTemplatesDep; 
      typedef srl::FTemplatesSRL FTemplatesSRL; 

    private:
      const SRLSymbols* _symbols;
      const X* _x; // a pointer to the pattern 
      int _spaces;
      int _syn_offset;

      typename FTemplatesDep::Configuration ft_dep_config; 
      typename FTemplatesSRL::Configuration ft_srl_config; 

    public:
      // creation and destruction
      
      FeaturesSRLV1()
	: _symbols(NULL), _x(NULL), _spaces(0), _syn_offset(1)
      {} 
      
      ~FeaturesSRLV1() { clear(); }
      
      // clears the feature structure
      void clear();
      
      //! Returns a feature vector for part <pred,arg,role>
      FeatureVector<FIdx>* phi(const srl::PartSRL& part); 
      
      FeatureVector<FIdx>* operator()(const srl::PartSRL& part) {
	return phi(part);
      }
      
      void label_offset(FeatureVector<FIdx>* f, int role) const;

      void discard(const FeatureVector<FIdx>* f) const; 
    };


    template <typename FIdx>
    void FeaturesSRLV1<FIdx>::clear() {
      _x = NULL;
      _symbols = NULL; 
    }

    template <typename FIdx>
    FeatureVector<FIdx>* FeaturesSRLV1<FIdx>::phi(const srl::PartSRL& part) {
      assert(_x!=NULL); 
      assert(_symbols!=NULL); 
      
      // extract the syn feats
      PartDep1 part_dep(part.pred(), part.arg(), 0);
      FeatureVector<FIdx> *f_syn = new FeatureVector<FIdx>;
      DepFeaturesV1Extractor<X,PartDep1,FIdx>::extract(*_symbols, *_x, part_dep, f_syn);
      f_syn->val = NULL;
      f_syn->next = NULL;
      
      // extract the SRL feats
      FeatureVector<FIdx> *f_sem = new FeatureVector<FIdx>;
      FTemplatesSRL::extract<FIdx>(ft_srl_config, *_symbols, *_x, part.p_, part.a_, f_sem);
      f_sem->val = NULL;
      f_sem->next = NULL;
      
      // set the parameter offsets
      const int rolelabel = part.rolelabel();      
      f_syn->offset = rolelabel * 4 + _syn_offset; 
      f_sem->offset = rolelabel * 4 + 0;           

      //concat feat vectors
      f_syn->next = f_sem;

      return f_syn;
    }


    template <typename FIdx>
    void FeaturesSRLV1<FIdx>::label_offset(FeatureVector<FIdx>* f, int role) const {
      FeatureVector<FIdx>* f_syn = f;
      FeatureVector<FIdx>* f_sem = f_syn->next;
      f_syn->offset = role * 4 + _syn_offset;
      f_sem->offset = role * 4 + 0;
    }
        

    template <typename FIdx>
    void FeaturesSRLV1<FIdx>::discard(const FeatureVector<FIdx>* const f) const {
      // recurse if necessary
      if (f->next!=NULL) {
	discard(f->next);
      }
      delete [] f->idx; 
      delete f;
    }

  }

  template <typename FIdxA, typename FIdxB>
  struct SwitchFIdx<srl::FGenSRLV1<FIdxA>,FIdxB> {
    typedef srl::FGenSRLV1<FIdxB> F;
  };

}


#endif /* SRL_FGEN */
