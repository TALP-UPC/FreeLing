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
 * \file   script-dump.h
 * \brief  A script for dumping part-factored structures
 * \author Xavier Carreras
 */

#ifndef TREELER_TEST_IOCONLL
#define TREELER_TEST_IOCONLL

#include <string>

#include "treeler/control/script.h"
#include "treeler/control/models.h"
#include "treeler/base/basic-sentence.h"

#include "treeler/io/io-conll-x.h"

namespace treeler {

  using namespace control;

  namespace control {
    
    enum ModelTypesExtended {
      IO0 = control::ModelTypes::_EXTENSION, 
      IOII, 
      IOSS, 
      IOSI,
    };
        
    /**
     */
    template <>
      class Model<IO0> :  public Model<DEP1> {
    public:
      static string name() { return "io0"; }
      static string brief() { return "description for " + name() + " here"; }
      typedef Sentence X; 
      typedef PartDep1 R; 
      typedef Label<R> Y; 
      typedef NullInference<X,Y,R> I; 
      typedef IOCoNLLX IO;
    };

    /**
     */
    template <>
      class Model<IOSS> :  public Model<DEP1> {
    public:
      static string name() { return "ioss"; }
      static string brief() { return "description for " + name() + " here"; }
      typedef BasicSentence<string,string> X; 
      typedef PartDep1 R; 
      typedef Label<R> Y; 
      typedef NullInference<X,Y,R> I; 
      typedef IOCoNLLX IO;
    };

    /**
     */
    template <>
      class Model<IOII> :  public Model<DEP1> {
    public:
      static string name() { return "ioii"; }
      static string brief() { return "description for " + name() + " here"; }
      typedef BasicSentence<int,int> X; 
      typedef PartDep1 R; 
      typedef Label<R> Y; 
      typedef NullInference<X,Y,R> I; 
      typedef IOCoNLLX IO;
    };

    /**
     */
    template <>
      class Model<IOSI> :  public Model<DEP1> {
    public:
      static string name() { return "iosi"; }
      static string brief() { return "description for " + name() + " here"; }
      typedef BasicSentence<string,int> X; 
      typedef PartDep1 R; 
      typedef Label<R> Y; 
      typedef NullInference<X,Y,R> I; 
      typedef IOCoNLLX IO;
    };
    
  }


  /** 
   *  \brief A script for dumping part-factored objects of various kinds
   *  \ingroup control
   */  
  class ScriptTestIOCoNLL: public Script {
    private:
    typedef ModelSelector<ScriptTestIOCoNLL, DEP1, IO0, IOSS, IOII, IOSI> GenericModel;
      
    public:
    ScriptTestIOCoNLL()
      : Script("")
	{} 

    ~ScriptTestIOCoNLL()
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
      
      // whether to calculate max solution
      int run_max; 
      int display_max;       
    };
    
    static std::string name() { return "test-ioconll"; }
    
    void usage(const std::string& msg);
    
    void run(const std::string& dir, const std::string& data_file);

    void test1(); 
    void test2(); 
    void test_conllx(); 
    void test_conll08(); 
    void test_conll03(); 
    
    void testSent(); 

    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    static void run(const string& name, bool help);

      
  };
}


#endif
