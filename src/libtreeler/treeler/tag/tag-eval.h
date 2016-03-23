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
 * \file   tag-eval.h
 * \brief  An evaluator of tag sequences
 * \author Xavier Carreras
 * 
 */

#ifndef TREELER_TAGEVAL
#define TREELER_TAGEVAL

namespace treeler {

  struct TagEval {
  public: 
    
    //! number of processed sentences
    int s; 
    
    //! number of processed tokens
    int n; 
    
    //! number of correct tags
    int ctags; 
    
    TagEval() : s(0), n(0), ctags(0) {}

    template <typename X, typename R>
    void operator()(const X& x, const Label<R>& y, const Label<R>& yhat) {
      ++s;
      n += x.size(); 
      
      vector<int> ctag(x.size(), -1); 
      for (auto r=y.begin(); r!=y.end(); ++r) {
	ctag[r->i] = r->b;
      }

      for (auto r=yhat.begin(); r!=yhat.end(); ++r) {
	if (ctag[r->i] == r->b) {
	  ++ctags;
	}
      }
    }

    template <typename X>
    void operator()(const X& x, const TagSeq& y, const TagSeq& yhat) {
      ++s;
      n += x.size(); 
      for (int i=0; i<n; ++i) {
	if (y[i] == yhat[i]) {
	  ++ctags;
	}
      }
    }


    //! tag accuracy 
    double tag_accuracy() const { return 100*(double)ctags / (double)n; }

    string to_string() const {
      ostringstream out; 
      const string SEP = "  "; 
      out << "sentences " << s;
      out << SEP << "tokens " << n;
      out << SEP << "acc " << tag_accuracy(); 
      return out.str();
    }

  };
 
}

#endif
