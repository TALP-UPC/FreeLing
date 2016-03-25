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
 * @file char-utils.cc
 * @brief Implements static utility functions for string manipulation
 * @author Mihai Surdeanu
 */

#include "treeler/util/char-utils.h"

#include <list>
#include <stdlib.h>
#include <ctype.h>

using namespace std;

namespace treeler {
  bool startsWith(const std::string & big,
		  const std::string & small)
  {
    if(small.size() > big.size()) return false;

    for(size_t i = 0; i < small.size(); i ++){
      if(small[i] != big[i]){
	return false;
      }
    }

    return true;
  }

  bool endsWith(const std::string & big,
		const std::string & small)
  {
    size_t bigSize = big.size();
    size_t smallSize = small.size();

    if(smallSize > bigSize) return false;

    for(size_t i = 0; i < smallSize; i ++){
      if(small[i] != big[bigSize - smallSize + i]){
	return false;
      }
    }

    return true;
  }

  int toInteger(const std::string & s)
  {
    return strtol(s.c_str(), NULL, 10);
  }

  void toString(const int i, std::string& s) {
    std::ostringstream os;
    os << i;
    s = os.str();
  }


  template <>
  void simpleTokenize(const std::string & input,
		      std::vector<std::string> & output,
		      const std::string & separators) {
    for(int start = input.find_first_not_of(separators);
	start < (int) input.size() && start >= 0;
	start = input.find_first_not_of(separators, start)){
      int end = input.find_first_of(separators, start);
      if(end < 0) end = input.size();
      output.push_back(input.substr(start, end - start));
      start = end;
    }
  }

  template <typename T>
  void simpleTokenize(const std::string & input,
		      std::vector<T> & output,
		      const std::string & separators) {
    for(int start = input.find_first_not_of(separators);
	start < (int) input.size() && start >= 0;
	start = input.find_first_not_of(separators, start)){
      int end = input.find_first_of(separators, start);
      if(end < 0) end = input.size();
      T value; 
      if (start==(end-1) and (input[start]=='_' or input[start]=='-')) {
	value = T(0);
      }
      else {
	std::istringstream iss(input.substr(start, end - start));	
	if (!(iss >> value)) {
	  std::cerr << "simple tokenize : error reading \"" << input.substr(start, end - start) << "\" as a \"" << typeid(T).name() << "\" value" << std::endl;
	  exit(0);
	}
      }
      output.push_back(value);
      start = end;
    }
  }


  std::string toUpper(const std::string & s)
  {
    std::string o = s;

    for(int i = 0; i < (int) o.size(); i ++){
      o[i] = toupper(o[i]);
    }

    return o;
  }

  std::string toLower(const std::string & s)
  {
    std::string o = s;

    for(int i = 0; i < (int) o.size(); i ++){
      o[i] = tolower(o[i]);
    }

    return o;
  }

  std::string stripString(const std::string & s,
			  int left,
			  int right)
  {
    return s.substr(left, s.size() - left - right);
  }

  bool emptyLine(const std::string & input)
  {
    for(size_t i = 0; i < input.size(); i ++)
      if(! isblank(input[i])) return false;
    return true;
  }


  /*
    int main(int argc, char ** argv)
    {
    String p = argv[1];
    vector<String> exp;
    expandNumber(p, exp);
    for(int i = 0; (int) i < exp.size(); i ++){
    cout << "\"" << exp[i] << "\"" << " ";
    }
    cout << endl;
    }
  */

}
