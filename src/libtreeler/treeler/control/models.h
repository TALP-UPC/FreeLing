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
 * \file   models.h
 * \brief  Various definitions of structured prediction models
 * \author Xavier Carreras
 */

#ifndef TREELER_MODELS
#define TREELER_MODELS

#include <string>
#include <iostream>
using namespace std;


// model class
// #include "treeler/class/class-basic.h"
// #include "treeler/class/fgen-class.h"
// #include "treeler/class/io-class.h"

// model tag
#include "treeler/tag/tag.h"

// model dep
#include "treeler/dep/dep.h"


namespace treeler {

  namespace control {
    
    /** 
     * \brief A list of model types implemented in treeler
     * 
     * 
     */ 
    enum ModelTypes { MC, TAG1, TTAG, DEP1, DEP1F, DEP2, DEP2F, DEP1V0, SHAG1, ISHAG1, _EXTENSION };
    
    template <int MODELTYPE> 
    class Model {
    };
    

    template <>
    class Model<TAG1> {
    public: 
      static string name() { return "tag1"; };
      static string brief() { return "tagging with bigram factorizations"; };

      typedef TagSymbols Symbols;
      typedef BasicSentence<string,string> X;
      typedef TagSeq Y;
      typedef PartTag R; 
      typedef Viterbi I;
      typedef FGenTag<X,R> F; 
      typedef WFScorer<Symbols,X,R,F> S; 
      typedef IOCoNLL<Symbols> IO;    

      typedef TagEval Eval;
    };

    

    struct SentenceStr : public BasicSentence<string,string> {};
    struct FGenDep1V0  : public FGenDepV0<SentenceStr,PartDep1> {};

    template <>
    class Model<DEP1> {
    public:
      static string name() { return "dep1"; };
      static string brief() { return "first-order projective dependency parsing"; };
      typedef DepSymbols Symbols; 
      typedef BasicSentence<string,string> X; 
      //typedef SentenceStr X; 
      typedef DepVector<string>  Y; 
      typedef IOCoNLL<Symbols> IO; 

      typedef ProjDep1 I;
      typedef PartDep1 R; 
      typedef FGenDepV0<X,R> F;
      //typedef FGenDep1V0 F;
      typedef WFScorer<Symbols,X,R,F> S; 

      typedef DepEval Eval;
    };

    template <>
      class Model<DEP1F> {
    public:
      static string name() { return "dep1f"; };
      static string brief() { return "first-order projective dependency parsing (uses new feature tuples)"; };
      typedef DepSymbols Symbols; 
      typedef BasicSentence<string,string> X; 
      typedef DepVector<string>  Y; 
      typedef IOCoNLL<Symbols> IO; 

      typedef ProjDep1 I;
      typedef PartDep1 R; 
      typedef FGenDepV1<X,R> F;
      typedef WFScorer<Symbols,X,R,F> S; 

      typedef DepEval Eval;

    };

    template <>
      class Model<DEP2> {
    public:
      static string name() { return "dep2"; };
      static string brief() { return "second-order projective dependency parsing"; };

      typedef DepSymbols Symbols; 
      typedef BasicSentence<string,string> X; 
      typedef DepVector<string> Y; 
      typedef IOCoNLL<Symbols> IO; 
      typedef ProjDep2 I;
      typedef PartDep2 R; 
      typedef FGenDepV0<X,R> F;
      typedef WFScorer<Symbols,X,R,F> S;
      typedef DepEval Eval;
    };

    template <>
      class Model<DEP2F> {
    public:
      static string name() { return "dep2f"; };
      static string brief() { return "second-order projective dependency parsing, new features"; };

      typedef DepSymbols Symbols; 
      typedef BasicSentence<string,string> X; 
      typedef DepVector<string> Y; 
      typedef IOCoNLL<Symbols> IO; 
      typedef ProjDep2 I;
      typedef PartDep2 R; 
      typedef FGenDepV1<X,R> F;
      typedef WFScorer<Symbols,X,R,F> S;       
      typedef DepEval Eval;
    };

    template <>
      class Model<SHAG1> {
    public:
      static string name() { return "shag1"; };
      static string brief() { return "first-order SHAG"; };
      typedef DepSymbols Symbols; 
      typedef BasicSentence<std::string,std::string> X; 
      typedef DepVector<string> Y; 
      typedef ProjDep1 I;
      typedef I::R R; 
      typedef FGenDepV1<X,R> F; 
      typedef WFScorer<Symbols,X,R,F> S; 
      typedef IOCoNLL<Symbols> IO; 
    };

    template <>
      class Model<ISHAG1> {
    public:
      static string name() { return "ishag1"; };
      static string brief() { return "first-order SHAG"; };
      typedef DepSymbols Symbols; 
      typedef BasicSentence<int,int> X; 
      typedef DepVector<int> Y; 
      typedef ProjDep1 I;
      typedef I::R R; 
      typedef FGenDepV1<X,R> F; 
      typedef WFScorer<Symbols,X,R,F> S; 
      typedef IOCoNLL<Symbols>  IO; 
    };
   

  }    
}

#include "treeler/control/model-selector.h"

#endif
