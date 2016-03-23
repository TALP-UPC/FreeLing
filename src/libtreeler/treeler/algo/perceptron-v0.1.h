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
 *  along with Treeler.  If not, see <http: *www.gnu.org/licenses/>.
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

#ifndef LEARN_PERCEPTRON01_H
#define LEARN_PERCEPTRON01_H

#include "treeler/base/parameters.h"
#include "treeler/learn/learn-utils.h"

#include <string>

namespace treeler {

  /** 
   *   \brief  A generic Perceptron for structured prediction, templated on patterns X and parts R 
   *   \author Xavier Carreras and Terry Koo
   *   \note   This code originated from a non-generic version of Perceptron in egstra
   * 
   *  A generic Perceptron for structured prediction, templated on patterns X and parts R. 
   *  Properties:
   *  - Supports parameter averaging
   *  - 
   * 
   */
  template <typename X, typename R>
  class Perceptron01 {
  public:
    Perceptron01() : _W(NULL) {}
    ~Perceptron01() {}

    //    /* these can be set only once */
    // void set_model(gstructured_model<Part>* M) {
    //   assert(_M == NULL); assert(M != NULL); _M = M;
    // }
    //    void set_fgen(gfgen<Part>* fg) {
    //   assert(_fg == NULL); assert(fg != NULL); _fg = fg;
    // }
    // void set_dir(const std::string& dir) {
    //   assert(_dir.size() == 0); assert(dir.size() > 0); _dir = dir;
    // }


    void usage(const char* const mesg = "");
    void process_options();

    void train(StructuredModel<X,R>& model, 
	       DataSet<X,R>& trdata, 
	       DataSet<X,R>& valdata, 
	       const int T);
    private:

    /* examine this example */
    void process_example(StructuredModel<X,R>& model, const Example<X,R>& ex);
        
    /**** global algorithm parameters *****/
    /* primal parameters */
    Parameters* _W;

    /***** training invocation variables *****/
    /* style of randomization */
    LearnUtils::randomization_t _rand_type;
    /* seed used for srandom() */
    int _seed;
    /* whether the primal parameters are averaged */
    bool _averaged;
    /* whether the primal parameters are sparse */
    bool _sparse;
    /* data type of the parameters */
    std::string _paramtype;
    /* number of examples processed on the current training run */
    int _num_processed;
    /* number of mistakes made during training */
    int _mistakes;
    /* number of mistakes at part level, i.e. hamming loss, made during training */
    int _hamming_loss;
    /* whether to write parameter vectors to disk */
    bool _writeparams;
    /* whether to write parameter vectors to disk every time this number of examples are processed */
    int _writeparams_every;

    /* whether to resume training, initializing the parameters from some given file */
    bool _resume_training;
    string _resume_initial_parameters;

    //    gstructured_model<Part>* _M;
    // gfgen<Part>* _fg;
    /* model directory, for parameter and configuration files */
    // std::string _dir;

    // verbosity level
    int _verbose;

  };
}

#include "treeler/learn/perceptron-v0.1.tcc"

#endif /* LEARN_PERCEPTRON_H */
