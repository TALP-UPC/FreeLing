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
 * \file   script-sl.h
 * \brief  A script for semantic role labeling
 * \author Xavier Lluis
 */

#ifndef TREELER_SCRIPT_SRL
#define TREELER_SCRIPT_SRL

#include <string>
#include "scripts/script.h"

#include "treeler/control/models.h"
#include "treeler/srl/srl.h"
#include "treeler/srl/sentence.h"
#include "treeler/srl/part-srl0.h"
#include "treeler/srl/srl-eval.h"
#include "treeler/srl/simple-parser.h"
#include "treeler/srl/io.h"
#include "treeler/srl/fgen-srl-v1.h"
#include "treeler/srl/scorers.h"
#include "treeler/srl/factory-srl.h"


namespace treeler {

  namespace control {

    /**
     *  Extension of Model Types
     */
    enum ModelTypesExt {
      SRL0 = ModelTypes::_EXTENSION,
      SRL1
    };

    /**
     *  \brief A basic model for SRL 
     *  \ingroup srl
     */
    template<>
    class Model<SRL1> {
    public:
      static string name() {  return "srl1"; }
      static string brief() { return "basic semantic role labeling"; }
      
      typedef srl::SRLSymbols Symbols;
      typedef srl::Sentence X;
      typedef srl::PredArgSet Y;
      typedef srl::PartSRL R;
      typedef FIdxBits FIdx;
      typedef srl::FGenSRLV1<FIdx> F;
      typedef WFScorer<Symbols, X, R, F> S;
      typedef IOCoNLL<Symbols> IO;
      typedef srl::SimpleParser I;
      typedef srl::SrlEval Eval;
    };


    /**
     *  \brief A script for semantic role labeling
     *  \ingroup srl
     */
    class ScriptSRL: public Script {
    private:
      //typedef srl::CoNLLStream09SRL TestStream;
      typedef ModelSelector<ScriptSRL, SRL1> SRLModel;
      
    public:
      ScriptSRL() :
        Script("srl") {
      }
      ~ScriptSRL() {
      }

      struct Configuration {
      };
      
      static std::string name() {
        return "srl";
      }
      
      void usage(Options& options);
      
      void run(Options& options);
                  
      template<typename Model>
      static void usage(Options& options, ostream& log, const string& prefix);
      
      template<typename Model>
      static void run(Options& options, ostream& log);
      
      template<typename Model>
      static void parse(Options& options, ostream& log);

      template<typename Model>
      static void paths(Options& options, ostream& log);
            
    };
  }
}

#endif

