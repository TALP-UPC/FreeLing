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
 * \file   scorers.h
 * \brief  
 */
#ifndef TREELER_SRL_SCORERS_H
#define TREELER_SRL_SCORERS_H

#include <iostream>
#include <algorithm>
#include <string>

#include "treeler/srl/srl.h"
#include "treeler/srl/fgen-srl-v1.h"

using namespace std;

namespace treeler {

  namespace srl {

    /*class OracleScores; 

    class OracleScorer {
    public:
      typedef OracleScores Scores; 

      OracleScorer(const SRLSymbols& sym) 
        :  _symbols(sym)
      {}

      void scores(const Sentence& x, Scores& s);

    private:
      const SRLSymbols& _symbols;

    };

    class OracleScores {      
    private:
      const Sentence* _x; 
    public: 

      typedef OracleScorer Scorer;

      OracleScores() 
        : _x(NULL)
      {}

      void set_x(const Sentence& x) {
        _x = &x;
      }

      string operator()(int pred, int arg) {
        const string null = ""; 
        auto a = _x->get_gold_pred_args().find(pred); 
        if (a != _x->get_gold_pred_args().end()) {
          const PredArgStructure& pa = a->second;
          if (pa(arg).empty()){
            return "_";
          } else {
            return ""+pa(arg);          
          }
        }
        return null; 
      }        
    };


    void OracleScorer::scores(const Sentence& x, Scores& s) {
      s.set_x(x);
    }*/    

  }  // namespace srl
}

#endif
