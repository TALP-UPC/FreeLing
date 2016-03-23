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
 * \file   srl-eval.h
 * \brief  An evaluator of SRL
 * \author Xavier Lluis
 * 
 */

#ifndef TREELER_SRLEVAL
#define TREELER_SRLEVAL

#include "treeler/srl/part-srl0.h"
#include "treeler/srl/sentence.h"

namespace treeler {
  
  namespace srl{
    
    struct SrlEval {
    public: 

      //! code of the null class
      int null_class;
      
      //! number of processed sentences
      int num_sentences; 
      
      //! number of processed predicates
      int num_predicates; 
      
      //! number of correct args, predicted args, gold args
      int correct_args, predicted_args, gold_args; 
      
      SrlEval() : null_class(-1),
		  num_sentences(0), num_predicates(0), 
		  correct_args(0), predicted_args(0), gold_args(0) {}

      SrlEval(int nc) : null_class(nc), 
			num_sentences(0), num_predicates(0), 
			correct_args(0), predicted_args(0), gold_args(0) {}


      template <typename X>
      void operator()(const X& x, const PredArgSet& y, const PredArgSet& yhat) {
	++num_sentences;

	auto plist = y.predicate_list(); 
	auto phatlist = yhat.predicate_list(); 
	auto ip = plist.begin();
	auto ipe = plist.end(); 
	auto iph = phatlist.begin();
	auto iphe = phatlist.end(); 

	int n = x.size();

	while (ip!=ipe or iph!=iphe) {
	  if (ip!=ipe and iph!=iphe and *ip==*iph) {
	    ++num_predicates; 

	    auto const & a = y.find(*ip)->second;
	    auto const & ahat = yhat.find(*iph)->second;
	    for (int t=0; t<n; ++t) {
	      string r = a(t);
	      string rhat = ahat(t);
	      if (not r.empty() or not rhat.empty()) {
		if (r.empty()) {
		  ++predicted_args; 
		}
		else if (rhat.empty()) {
		  ++gold_args;
		}
		else {
		  ++gold_args; 
		  ++predicted_args; 
		  if (r == rhat) {
		    ++correct_args; 
		  }
		}
	      }
	    }
	    ++ip;
	    ++iph;
	  }
	  else if (ip!=ipe and (iph==iphe or *ip<*iph)) {
	    ++num_predicates; 
	    gold_args += y.find(*ip)->second.size(); 
	    ++ip; 
	  }
	  else {
	    predicted_args += yhat.find(*iph)->second.size(); 
	    ++iph; 
	  }
	}
      }


      template <typename X, typename R>
      void operator()(const X& x, const Label<R>& y, const Label<R>& yhat) {
	++num_sentences;
	num_predicates += x.get_predicates().size(); 
	
	typename Label<R>::const_iterator it_y = y.begin();
	typename Label<R>::const_iterator it_y_end = y.end();
	typename Label<R>::const_iterator it_yhat = yhat.begin();
	typename Label<R>::const_iterator it_yhat_end = yhat.end();
	while ((it_y!=it_y_end) and (it_yhat!=it_yhat_end)) {
	  if (*it_y == *it_yhat) {
	    if (null_class<0 or it_y->rolelabel()!=null_class) {
	      ++correct_args;
	      ++gold_args;
	      ++predicted_args;
	    }
	    ++it_y;
	    ++it_yhat;
	  }
	  else if (*it_y < *it_yhat) {
	    if (null_class<0 or it_y->rolelabel()!=null_class) {
	      ++gold_args;
	    }
	    ++it_y;
	  }
	  else {
	    if (null_class<0 or it_yhat->rolelabel()!=null_class) {
	      ++predicted_args;
	    }
	    ++it_yhat;
	  }
	}
	while (it_y != it_y_end) {
	  if (null_class<0 or it_y->rolelabel()!=null_class) {
	    ++gold_args;
	  }
	  ++it_y;
	}
	while (it_yhat != it_yhat_end) {
	  if (null_class<0 or it_yhat->rolelabel()!=null_class) {
	    ++predicted_args;
	  }
	  ++it_yhat;
	}
      }

      void operator+=(const SrlEval& o) {
	num_sentences += o.num_sentences; 
	num_predicates += o.num_predicates; 
	gold_args += o.gold_args; 
	correct_args += o.correct_args; 
	predicted_args += o.predicted_args; 
      }
      
      //! labeled precision
      double labeled_precision() const { 
        return 100*(double)correct_args/ (double)predicted_args; 
      }
      
      //! labeled recall
      double labeled_recall() const { 
        return 100*(double)correct_args / (double)gold_args; 
      }
      
      string to_string() const {
        ostringstream out; 
        const string SEP = "  "; 
        out << "sentences " << num_sentences;
        out << SEP << "num_predicates " << num_predicates;
        out << SEP << "g " << gold_args;
        out << SEP << "p " << predicted_args;
        out << SEP << "c " << correct_args;
	double p = labeled_precision();
	double r = labeled_recall();
	out << SEP << "prec " << p;
	out << SEP << "rec " << r;
 	out << SEP << "f1 " << 2*p*r/(p+r);
        return out.str();
      }

    };

  } //end of namespace SRL


  /** Example traits for SRL parts*/
  template<typename X>
  class ExampleTraits<X, Label<srl::PartSRL>> {
  public:
    typedef srl::SrlEval Eval;
  };

  /** Example traits for SRL parts*/
  template<typename X>
  class ExampleTraits<X, srl::PredArgSet> {
  public:
    typedef srl::SrlEval Eval;
  };
  
} //end of namespace treeler

#endif
