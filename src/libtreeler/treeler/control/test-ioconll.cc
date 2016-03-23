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
 *  along with Treeler.  If not, see <http: *www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   test-ioconll.cc
 * \brief  A test script for IOCoNLL
 * \sa IOCoNLL
 * \author Xavier Carreras
 */

#include "treeler/util/register.h"
#include "treeler/control/test-ioconll.h"
REGISTER_SCRIPT(ioconll, ScriptTestIOCoNLL);


#include <stdio.h>
#include <vector>
#include <algorithm>
#include <typeinfo>

#include "treeler/control/factory-base.h"
#include "treeler/control/factory-scores.h"
#include "treeler/control/factory-tag.h"
#include "treeler/control/factory-dep.h"
#include "treeler/control/factory-ioconll.h"

#include "treeler/util/options.h"
#include "treeler/base/basic-sentence.h"

using namespace std;


namespace treeler {

  //! 
  //
  void ScriptTestIOCoNLL::usage(const string& msg) {
    if (msg!="") {
      cerr << name() << " : " << msg << endl;
      return;
    }
    cerr << name() << " : a tester for IOCoNLL" << endl;
    cerr << endl;
    cerr << name() << " options:" << endl;
    cerr << " --cmd=<command> : command to execute " << endl;
    cerr << "    io : read data from cin and write it to cout " << endl;
    cerr << "    test1 : ..." << endl;
    cerr << "    test2 : CoNLLStream tests" << endl;
    cerr << " --mod=<name>    : model name" << endl;
    cerr << endl;
    
    string model_name;
    if (!Options::get("mod", model_name)) {
      GenericModel::help(cerr); 
    }
    else {
      GenericModel::run(true); 
    }
    cerr << endl;
  }
     
    
  // this is the main non-templated script, instantiates the templated script via a generic model selector
  void ScriptTestIOCoNLL::run(const string& dir, const string& data_file) {
    string cmd = "io"; 
    Options::get("cmd", cmd); 
    if (cmd=="io") {
      GenericModel::run(false);
    }
    else if (cmd=="test1") {
      test1(); 
    }
    else if (cmd=="test2") {
      test2(); 
    }
    else if (cmd=="conllx") {
      test_conllx(); 
    }
    else if (cmd=="tmp") {
      
      typedef Sentence X; 
      typedef PartDep1 R; 
      typedef WFScores<X,R,NullFeatures<X,R,FIdxBits>> WFS; 
      WFS::Scorer wfs; 
      Factory<WFS>::configure(wfs); 


    }
    else {
      cerr << "unknown command \"" << cmd << "\"" << endl;
      exit(1);
    }
  }
    

  void ScriptTestIOCoNLL::testSent() {
    BasicSentence<string,string> ss_sent; 
    ss_sent.add_token(BasicToken<string,string>("The","the","det","determiner"));
    ss_sent.add_token(BasicToken<string,string>("boy","the","det","determiner"));
    ss_sent.add_token(BasicToken<string,string>("eats","the","det","determiner"));
    ss_sent.back().morpho_push("a"); 
    ss_sent.back().morpho_push("b"); 
    ss_sent.back().morpho_push("c"); 

    BasicSentence<int,string> is_sent; 
    is_sent.add_token(BasicToken<int,string>(1,11,"det","determiner"));
    is_sent.add_token(BasicToken<int,string>(2,12,"det","determiner"));
    is_sent.add_token(BasicToken<int,string>(3,13,"det","determiner"));

    CoNLLStream c; 
    c.add_padding = true;
    c << ss_sent << "XXX" << is_sent;
    cout << c;

  }

  void ScriptTestIOCoNLL::test_conllx() {
    istream& input = std::cin;
    ostream& output = std::cout; 
    treeler::CoNLLStream istrm;
    treeler::CoNLLStream ostrm;
    istrm.prefix = "INPUT-->";
    istrm.add_padding = true;
    ostrm.add_padding = true;

    while (input >> istrm) {
      output << istrm;

      vector<string> ids;
      BasicSentence<int,int> sentII; 
      BasicSentence<string,string> sentSS; 
      DepVector<string> depv; 

      istrm >> ids >> sentSS >> depv;
      ostrm << ids << sentSS << depv;
      istrm.rewind();
      istrm >> ids >> sentII;       
      ostrm << "|||" << ids << sentII;
      istrm.rewind();
      CoNLLFormattedSentence<BasicSentence<string,string>> fsentSS(sentSS, 3, 0, -1, -1, -1);
      istrm >> ids >> fsentSS; 
      ostrm << "|||" << ids << sentSS;  
	
      output << ostrm;
      ostrm.clear();
    }
  }


  void ScriptTestIOCoNLL::test1() {
    istream& input = std::cin;
    ostream& output = std::cout; 
    treeler::CoNLLStream strm;
    while (input >> strm) {
      vector<string> v; 
      int n = 0;
      while (strm >> v) {
	output << "(" << ++n << ")";
	for (auto it = v.begin(); it!=v.end(); ++it) {
	  output << " " << *it;
	}
	output << endl;
      }
      output << endl;
    }
  }

  void ScriptTestIOCoNLL::test2() {
    istream& input = std::cin;
    ostream& output = std::cout; 

    DepSymbols symbols; 
    Factory<DepSymbols>::configure(symbols); 
      
    treeler::CoNLLStreamX<DepSymbols> columns(symbols); 
    columns.add_padding = true; 
    while (input >> columns) {
      columns.prefix="INPUT-->";
      output << columns; 
      columns.prefix="OUTPUT->";
      
      if (columns.num_columns()<6) {
	columns.resize(6);
      }
      if (columns.num_columns()<7) {
	// empty HEAD column
	columns << "*";
      }
      if (columns.num_columns()<8) {
	// empty LABEL column
	columns << "*";
      }
      treeler::CoNLLColumn& deph = columns[6];
      treeler::CoNLLColumn& depl = columns[7];
      for (size_t i=0; i<columns.num_rows(); ++i) {
	stringstream ss; 
	ss << i; 
	ss >> deph[i]; 
	depl[i] = "mock";
      }

      output << columns;
    }
  }

  

  // this is the main templated script with all types resolved
  template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
  void ScriptTestIOCoNLL::run(const string& model_name, bool help) {
    cerr << name() << " : running " << model_name << " with ..." << endl;
    cerr << " X=" << typeid(X).name() << endl
	 << " Y=" << typeid(Y).name() << endl
	 << endl;
    
    if (help) {
      Factory<IO>::usage(cerr);
      exit(0);
    }
    
    Configuration config; 
    int tmp = 0; 
    Options::get("x", tmp); 
    config.display_x = tmp!=0; 
    Options::get("y", tmp); 
    config.display_y = tmp!=0;
    tmp = 1; 
    Options::get("xy", tmp); 
    config.display_xy = tmp!=0; 
    bool display = config.display_x or config.display_y or config.display_xy; 
  
      
    typename R::Configuration Rconfig; 
    Factory<R>::configure(Rconfig);
    
    // input output options
    IO io;
    Factory<IO>::configure(io);
          
        
    DataStream<X,Y,IO> ds(io, std::cin);
    config.ifile = ""; 
    Options::get("data", config.ifile); 
    config.from_cin = config.ifile.empty() or config.ifile=="-";   
    if (!config.from_cin) {
      ds.set_file(config.ifile);
    }
    
      
    /* PROCESS EACH EXAMPLE AND DO THE DUMPING */
    typename DataStream<X,Y,IO>::const_iterator it = ds.begin(), it_end = ds.end();
    for (; it!=it_end; ++it) {
      const X& x  = (*it).x();
      const Y& y  = (*it).y();
      if (display) cout << "+----------------------" << endl;
      if (display) cout << "EXAMPLE " << x.id() << endl;
      if (config.display_x) { cout << "X " << x.id() << " "; io.write(cout, x); cout << endl; }
      if (config.display_y) { cout << "Y " << x.id() << " "; io.write(cout, y); cout << endl; }
      if (config.display_xy) { io.write(cout, x, y); }
      
      
    } // for each example    
  }
  
}
