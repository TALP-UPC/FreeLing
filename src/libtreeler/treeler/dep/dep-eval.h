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
 * \file   dep-eval.h
 * \brief  An evaluator of dependency trees
 * \author Xavier Carreras
 * 
 */

#ifndef TREELER_DEPEVAL
#define TREELER_DEPEVAL

namespace treeler {

  struct DepEval {
  public: 
    
    //! number of processed sentences
    int s; 
    
    //! number of processed tokens
    int n; 
    
    //! number of correct heads, correct labels, correct heads and labels
    int ch, cl, chl; 
    
    DepEval() : s(0), n(0), ch(0), cl(0), chl(0) {}

    template <typename X, typename R>
    void operator()(const X& x, const Label<R>& y, const Label<R>& yhat) {
      typedef int LabelT;
      DepVector<LabelT> z, zhat; 
      R::compose(x, y, z); 
      R::compose(x, yhat, zhat); 
      (*this)(x, z, zhat);
    }
    
    template <typename X, typename LabelT>
    void operator()(const X& x, const DepVector<LabelT>& y, const DepVector<LabelT>& yhat) {
      ++s;
      n += x.size(); 
      DepVector<LabelT> z, zhat; 
      for (int i=0; i<x.size(); ++i) {
	const HeadLabelPair<LabelT>& hl = y[i];
	const HeadLabelPair<LabelT>& hlhat = yhat[i];
	if (hl.h==hlhat.h) {
	  ++ch; 
	  if (hl.l==hlhat.l) ++chl;
	}
	if (hl.l==hlhat.l) ++cl;
      }
    }


    //! unlabeled attachment accuracy 
    double uas() const { return 100*(double)ch / (double)n; }

    //! labeled attachment accuracy 
    double las() const { return 100*(double)chl / (double)n; }

    //! label accuracy 
    double label_accuracy() const { return 100*(double)cl / (double)n; }

    string to_string() const {
      ostringstream out; 
      const string SEP = "  "; 
      out << "sentences " << s;
      out << SEP << "tokens " << n;
      out << SEP << "uas " << uas(); 
      out << SEP << "las " << las(); 
      out << SEP << "lacc " << label_accuracy(); 
      return out.str();
    }

  };
 
}

#endif
