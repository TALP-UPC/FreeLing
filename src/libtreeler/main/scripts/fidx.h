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
 * \file   script-fidx.h
 * \brief  A script for developing different feature index types
 * \author Xavier Carreras
 */

#ifndef TREELER_SCRIPT_FIDX
#define TREELER_SCRIPT_FIDX

#include <string>

#include "treeler/control/script.h"
#include "treeler/base/label.h"
#include "treeler/base/parameters.h"
#include "treeler/base/scores.h"
#include "treeler/io/io-conll.h"


// model tag
#include "treeler/tag/tag.h"

// model dep
#include "treeler/dep/dep.h"


namespace treeler {


  class ScriptFIdx: public Script {
  public:
    ScriptFIdx()
      : Script("fidx")
      {} 
    ~ScriptFIdx()
      {} 

    struct Configuration {
      string ifile;
      bool from_cin;
      // whether to enumerate parts, and which selection
      int run_parts_y; 
      int run_parts_all; 
      int run_parts_max;
      int run_parts_cin;
      // when _allparts, step to increment parts
      int parts_step;
      
      // display options, general
      int display_x; 
      int display_y; 
      int display_xy; 
      
      // display options for parts
      int display_fvec; 
      int display_oracle;
      int display_score;
      int display_marginal;
      
      // whether to calculate max solution
      int run_max; 
      int display_max;       
    };

    static string name() { return "fidx";  }

    void usage(const std::string& msg);
    void run(const std::string& dir, const std::string& data_file);

  private:
    
    template <int M>
    void usage(const string& mod_name);

    void run_script(bool help);

    template <int M>
      void run_script(const string& model_name, bool help);
    
    template <int M, typename FIDX>
      void run_script(const string& model_name, bool help);
    
    template <typename X, typename Y, typename R, typename F, typename IO>
      void test_params();
    
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename O, typename IO>
      void dump_fvec(const string& model_name, bool help);

    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename O, typename IO, typename RIT>
      void dump_fvec_body(const Configuration& config, const string& label, 
			   const X& x, typename F::Features* features, 
			   RIT r, RIT r_end);

  };
}


#endif
