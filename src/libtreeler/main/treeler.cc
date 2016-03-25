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
 * \file   treeler.cc
 * \brief  Main program for using treeler via treeler scripts
 * \author Xavier Carreras
 */

// std
#include <iostream>
#include <vector>
#include <string>
#include <new>
#include <exception>
#include <cstdlib>
#include <cassert>
#include <ctime>

// utility classes
#include "scripts/script.h"
#include "scripts/registry.h"

// treeler
#include "treeler/util/options.h"
#include "treeler/base/exception.h"



using namespace std;
using namespace treeler;
time_t start,end;
double dif;

static void usage(Options& options, const string message = "", string script = "") {
  cerr << "treeler " << VERSION << " --- TALP research center, UPC 2013" << endl;
  cerr << "Usage: treeler " << (script=="" ? "<scriptname>" : script) << " [options]" << endl;

  if (script == "") {
    cerr << endl;
    cerr << "Use 'treeler <scriptname> --help' for help on a script." << endl;
    cerr 
      << "Use 'treeler <scriptname> --mod=<modelname> --help' for help using" << endl
      << "a specific model with a script." << endl;
    //  cerr << " --quieter[=<int>]    : produce less output" << endl;
    cerr << endl;
    cerr << endl;
    /* print the list of available scripts */
    cerr << "Scripts available : ";
    vector<string> names;
    treeler::Registry::getnames("script", names);
    for(int i = 0; i < (int)names.size(); ++i) { 
      if (i>0) cerr << "                    ";
      cerr << names[i] << endl; 
    }
    names.clear();
    cerr << endl;
  }
  else {
    Script* const S= (Script*) treeler::Registry::construct("script", script.c_str());
    S->usage(options);
    delete S;
  }
  cerr << endl;

  if (message != "") {
    cerr << message << endl << endl;   
  }
}




// declaration of subroutines
string parse_argv(Options& options, int argc, char** argv);
void print_argv(std::ostream& out, int argc, char** argv, std::string name, const char* const version);

// exception handler for object allocation
void treeler_new_handler() {
  cerr << endl;
  cerr << "treeler: out of memory!" << endl;
  exit(1);
}

int main(int argc, char ** argv)
{
  time(&start);

  Options options;

  /* set a handler object that will be called when memory runs out */
  set_new_handler(treeler_new_handler);
  
  if(argc == 1) { 
    usage(options); 
    exit(1); 
  }
  
  string scriptname = parse_argv(options, argc, argv);
  if(scriptname=="") {
    usage(options, "Please name a script."); exit(1);
  }
  
  int arg_int;
  string arg_str;
  if(options.get("help", arg_int) || options.get("h", arg_int)) {
    usage(options, "", scriptname);
    exit(1);
  }
  
  // create script and run it
  Script* const S = (Script*) treeler::Registry::construct("script", scriptname.c_str());
  if (S==NULL) {
    usage(options, "Script \"" + scriptname + "\" not available."); 
    exit(1);
  }
  try {
    S->run(options);
  }
  catch (TreelerException& e) {
    cerr << endl; 
    cerr << "*** treeler exception ***" << endl;
    cerr << e.what() << endl;
    cerr << "*** aborting ***" << endl;
  }
  catch (std::exception& e) {
    cerr << endl; 
    cerr << "*** standard exception ***" << endl;
    cerr << e.what() << endl;
    cerr << "*** aborting ***" << endl;
  }
  delete S;
  time_t end;
  time (&end);
  dif = difftime (end,start);
  cerr << endl;
  cerr << endl;
  cerr << "Total time taken " << dif <<" seconds.";
  
  cerr<<endl;
  cerr<<endl;
} // main program


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// low level functions
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/**
 *  Reads arguments from the command line. Arguments starting with '-'
 *  are options. The first non-option argument is assumed to be a
 *  command, and is returned. Any further non-options are assumed to
 *  be configuration files and are read.
 */
string parse_argv(Options& options, int argc, char** argv) {
  string cmd = "";
  for(int i = 1; i < argc; ++i) {
    const char* const arg = argv[i];
    if(arg[0] != '-') {
      if (cmd == "") {
	cmd = string(arg);
	// cerr << "treeler : command is " << cmd << "(" << arg << ")" << endl;
      }
      else {
	//cerr << "treeler : reading options from file " << arg << endl;
	bool b = options.read(string(arg));
	if (!b) {
	  cerr << "treeler : failed to read options from file \"" << arg << "\"." << endl;
	  exit(1);
	}
      }
    }
  }
  /* now parse the command-line arguments (potentially overwriting
     anything set in a config file) */
  options.read(argc, argv);
  return cmd;
}

void print_argv(std::ostream& out, int argc, char** argv, std::string name, const char* const version) {
  out << name << " v" << version << " called as:" << endl;
  for(int i = 0; i < argc; ++i) {
    out << (i == 0 ? "" : "    ");
    if(argv[i][0] == '-') {
      const string arg(argv[i]);
      const string::size_type idx = arg.find_first_of("=");
      if(idx == string::npos) { /* not a --name=value option */
	out << arg;
      } else { /* quote the value in the --name=value pair */
	out << arg.substr(0, idx) << "='" << arg.substr(idx + 1) << "'";
      }
    } else {
      out << argv[i];
      }
    out << (i == argc - 1 ? "" : " \\")
	<< endl;
  }
}
