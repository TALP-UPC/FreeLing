/*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2011   TALP Research Center
 *                       Universitat Politecnica de Catalunya
 *
 *  This file is part of Treeler.
 *
 *  Treeler is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Treeler is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Treeler.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   perceptron.h
 * \brief  Declaration of a generic Perceptron
 * \author Xavier Carreras, Terry Koo
 */

#ifndef LEARN_PERCEPTRON_H
#define LEARN_PERCEPTRON_H

#include "treeler/base/parameters.h"
#include "treeler/algo/learn-utils.h"

#include <string>

namespace treeler {

  /** 
   * \brief  A generic Perceptron for structured prediction, templated on patterns X and parts R 
   * \author Xavier Carreras and Terry Koo
   * \ingroup base
   * \note   This code originated from a non-generic version of Perceptron in egstra
   * 
   *  A generic Perceptron for structured prediction, with support for parameter averaging. 
   * 
   * \tparam X The type of input patterns
   * \tparam R The type of part factorization
   * \tparam I A class providing an \c argmax routine
   * \tparam S The type of scores of pattern-part pairs
   * \tparam F A feature generator 
   * \tparam IO An Input-Output component that writes the progress of the algorithm
   * 
   */
  template <typename Model>
  class Perceptron {
  public:

    struct Configuration {
      /* max number of training epochs */
      int T;
      /* directory where to save parameter files to */ 
      string wdir; 

      /* style of randomization */
      LearnUtils::randomization_t rand_type;
      /* seed used for srandom() */
      int seed;
      /* whether the primal parameters are averaged */
      bool averaged;
      /* whether the primal parameters are sparse */
      bool sparse;
      /* data type of the parameters */
      std::string paramtype;
      /* whether to write parameter vectors to disk */
      bool writeparams;
      /* whether to write parameter vectors to disk every time this number of examples are processed */
      int writeparams_every;
      
      /* whether to resume training, initializing the parameters from some given file */
      bool resume_training;
      string resume_initial_parameters;
      
      /* verbosity level */
      int verbose;
    };
    

    /* static void usage(const char* const mesg = ""); */
    /* static void process_options(Configuration& config); */
    
    static void train(Configuration& config, 
		      const typename Model::Symbols& symbols, 
		      typename Model::I& parser,
		      typename Model::F& fgen, 
		      typename Model::IO& io,
		      DataSet<typename Model::X, Label<typename Model::R>>& trdata, 
		      DataSet<typename Model::X, typename Model::Y>& valdata);
  private:
    
    struct Status {
      /* number of examples processed on the current training run */
      int num_processed;
      /* number of mistakes made during training */
      int mistakes;
      /* number of mistakes at part level, i.e. hamming loss, made during training */
      int hamming_loss;
      /* */
      double cpu_time;
    };


    /* examine this example */
    static void process_example(const Configuration& config, 
				typename Model::I& parser, 
				typename Model::S& scorer, 
				const Example<typename Model::X, Label<typename Model::R>>& ex,
				Status& status);
        
  };
}

#include "treeler/algo/perceptron.tcc"

#endif /* LEARN_PERCEPTRON_H */
