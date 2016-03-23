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
 * @file options.cc
 * @brief Implements the class Options
 * @author Mihai Surdeanu, Xavier Carreras
 * @note This class was ported from egstra and deeper
 */

#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#if defined WIN32 || defined WIN64
  #include <direct.h>
#else
  #include <unistd.h>
#endif
#include <string.h>
#include <cassert>

#include "treeler/util/options.h"
#include "treeler/base/exception.h"


#define OPTIONS ((StringMap<string>*)_options)

using namespace std;

namespace treeler {

  void Options::set(const string & name,
		    const string & value,
		    bool clobber) {
    auto it = _options.find(name); 
    if (it != _options.end()) {
      if (clobber) {
	it->second = value; 
      }
      return;
    }
    else {
      _options[name] = value;
    }
  }

  void Options::set(const string & name,
                    const int & value,
                    bool clobber) {
    ostringstream oss;
    oss << value;    
    set(name, oss.str(), clobber);
  }

  bool Options::contains(const string & name) {
    auto it = _options.find(name); 
    return it!=_options.end();
  }


#define MAX_PAR_LINE (16 * 1024)

  /** Returns the position of the first non-space character */
  static int skipSpaces(const char * line)
  {
    int i = 0;
    for(; line[i] != '\0'; i ++){
      if(! isspace(line[i])){
	break;
      }
    }
    return i;
  }

  void Options::write(const string& file) {
    FILE* out = fopen(file.c_str(), "w");
    if (out==NULL) {
      cerr << "Options: can not open options file \"" << file << "\"!" << endl; 
      exit(0);
    }
    if(_calledas.size() > 0) {
      fprintf(out, "#!%s\n\n", _calledas.c_str());
    }
    for(auto it = _options.begin();
	it != _options.end(); ++it) {
      const char* name = it->first.c_str();
      const char* value = it->second.c_str();
      fprintf(out, "%s=\"%s\"\n", name, value);
    }
    fclose(out);
  }

  bool Options::read(const string & file, bool overwrite)		     
  {
    ifstream is(file.c_str());

    if(!is){
      return false; 
      cerr << "Failed to open file: " << file << endl;
      throw(false);
    }
    
    char line[MAX_PAR_LINE];
    while(is.getline(line, MAX_PAR_LINE)){
      int start = skipSpaces(line);

      //
      // this is a blank line
      //
      if(line[start] == '\0'){
	continue;
      }

      //
      // this is a comment line
      //
      if(line[start] == '#'){
	continue;
      }

      //
      // read the current option
      //
      string name, value;
      if(readNameValue(line + start, name, value, false) == false){
	ostringstream oss; 
	oss << "failed to parse argument \"" << line + start << "\" in file \"" << file << "\"" << endl;
	TreelerException e("options", oss.str());
	throw(e);
      }

      if (name == _load_command) {
	// value is an options file, read it
	read(value, overwrite);
      }
      else if(overwrite == true || contains(name) == false){
	//
	// add the option to hash
	//	
	set(name, value, overwrite);
      }
    }
    return true;
  }
  
  int Options::read(int argc, 
		    char ** argv)
  {
    const char* exec = argv[0];
    if(exec[0] == '/') { /* absolute path to executable */
      _calledas = exec[0];
    } else {
      int size = 256;
      char* tmp = (char*)malloc(size*sizeof(char));
      while(getcwd(tmp, size) == NULL) {
	size *= 2; tmp = (char*)realloc(tmp, size*sizeof(char));
      }
      string cwd = tmp;
      free(tmp);
      _calledas = cwd + "/" + argv[0];
    }

    int current = 1;
    for(; current < argc; ++current){
      int length = strlen(argv[current]);

      //
      // found something that looks like a option:
      // --name=value
      //
      if(length > 2 && strncmp(argv[current], "--", 2) == 0){
	string arg;
	for(int i = 2; i < length; i ++){
	  arg.append(1, (char) argv[current][i]);
	}

	string name, value;
	if(readNameValue(arg, name, value, true) == false){
	  cerr << "Failed to parse argument \"" << arg << "\"" << endl;
	  throw(false);
	}

	/* clobber anything pre-existing */
	set(name, value, true);

      } else{
	_arguments.push_back(string(argv[current])); 
      }
    }

    return current;
  }

  bool Options::readNameValue(const string & arg, 
			      string & name,
			      string & value,
			      bool defaultValue)
  {
    const string sep = " \t=";

    int nameStart = arg.find_first_not_of(sep, 0);
    if(nameStart < 0) return false;

    int nameEnd = arg.find_first_of(sep, nameStart);
    if(nameEnd < 0) nameEnd = arg.size();

    name = arg.substr(nameStart, nameEnd - nameStart);

    if(nameEnd < (int) arg.size()){

      int valueStart = arg.find_first_not_of(sep, nameEnd);
      if(valueStart < 0) return false;

      string raw;

      if(arg[valueStart] == '\"'){
	int valueEnd = findQuote(arg, valueStart + 1);
	raw = arg.substr(valueStart + 1, valueEnd - valueStart - 1);
      } else{
	int valueEnd = arg.find_first_of(sep, valueStart);
	if(valueEnd < 0) valueEnd = arg.size();
	raw = arg.substr(valueStart, valueEnd - valueStart);
      }

      if(substitute(raw, value) == false){
	return false;
      }
      //value = raw; 
    } else{
      if(defaultValue == true){
	// Assume a default value of "1"
	value = "1";
      } else{
	return false;
      }
    }

    return true;
  }

  int Options::findQuote(const string & input,
			 int offset)
  {
    for(int i = offset; i < (int) input.size(); i ++){
      if(input[i] == '\"' &&
	 (i == 0 || input[i - 1] != '\\')){
	return i;
      }
    }
    return input.size();
  }

  bool Options::substitute(const string & raw,
			   string & value)
  {
    ostringstream out;
    int end = -1;

    for(unsigned int i = 0; i < raw.size(); i ++){
    
      //
      // found a complete variable: ${...}
      //
      if(i < raw.size() - 3 && raw[i] == '$' && raw[i + 1] == '{' &&
	 (end = raw.find_first_of('}', i + 2)) > 0){
      
	string varName = raw.substr(i + 2, end - i - 2);
	string varValue;

	// fetch the variable value
	if(get(varName, varValue, true) == false){
	  cerr << "Undefined option: " << varName << endl;
	  return false;
	}

	// add this value to stream
	out << varValue;

	i = end;
      } 
      // found a special character preceded by backslash
      else if(raw[i] == '\\' && i < raw.size() - 1){
	out << raw[i + 1];
	i++;
      } else{
	out << raw[i];
      }
    }

    value = out.str();
    return true;
  }

  bool Options::get(const string & name,
		    string & value,
		    bool useEnvironment)
  {
    auto it = _options.find(name);
    if(it != _options.end()){
      value = it->second;
      return true;
    }

    if(useEnvironment == false){
      return false;
    }

    // convert the name to regular chars
    string n;
    for(unsigned int i = 0; i < name.size(); i ++){
      n.append(1, (char) name[i]);
    }

    char * env = getenv(n.c_str());

    if(env == NULL){
      return false;
    }

    // convert the value to a string
    value.clear();
    int length = strlen(env);
    for(int i = 0; i < length; i ++){
      value.append(1, (char) env[i]);
    }

    return true;
  }

  bool Options::get(const string & name,
		    double & value,
		    bool useEnvironment)
  {
    string str;
    if(get(name, str, useEnvironment) == false){
      return false;
    }
    value = strtod(str.c_str(), NULL);
    return true;
  }

  bool Options::get(const string & name,
		    int & value,
		    bool useEnvironment)
  {
    string str;
    if(get(name, str, useEnvironment) == false){
      return false;
    }
    value = strtol(str.c_str(), NULL, 10);
    return true;
  }

  bool Options::get(const string & name,
		    bool & value,
		    bool useEnvironment)
  {
    string str;
    if(get(name, str, useEnvironment) == false){
      return false;
    }
    std::istringstream iss(str); 
    return bool(iss >> value);
  }

  void Options::display(ostream & os, const string off)
  {
    for(auto it = _options.begin();
	it != _options.end(); it ++){
      os << off << it->first << " = " 
	 << it->second << endl;
    }
  }

  /*
    int main()
    {
    Options::read("PARAMS");
    Options::display(cerr);
    }
  */
}
