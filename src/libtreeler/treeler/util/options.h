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
 * \file options.h
 * \brief Declares the class Options
 * \author Mihai Surdeanu, Xavier Carreras
 * \note This file was ported from egstra and deeper.
 */

#ifndef TREELER_OPTIONS_H
#define TREELER_OPTIONS_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "treeler/base/windll.h"

namespace treeler {
  
  /** 
   * \brief Implements management of command line options
   * \author Mihai Surdeanu, Xavier Carreras
   * 
   * Options implements management of command line options in the form
   * of "--name=value" or just "--name", the latter assuming a value
   * of 1. The class provides methods for reading options from
   * a file, and override them with command-line options.
   *
   * \note This class was ported from egstra and deeper.
   *
   * \todo Add support for checking whether an option is valid 
   * 
   * \todo Add support for nested options
   * 
   */
  class WINDLL Options {
    
  public:


    Options() 
      : _load_command("load") 
    {}
      

    
    /** 
     * \brief Reads options from a file
     * 
     * Returns a false value if the action fails. 
     *
     */
    bool read(const std::string & file, bool overwrite = false);

    /** 
     * Writes options to a file 
     */
    void write(const std::string & file);

    /** 
     * Reads options from the command line, in the form "--name=value", or
     * in the form "--name", which assumes a default value of 1
     * @return A positive integer representing the first free position in the
     *         argument array. If some error occured, a Boolean exception 
     *         is thrown out of this method
     */
    int read(int argc, char **argv);

    void display(std::ostream & os, const std::string off="");

    void set(const std::string & name,
	     const std::string & value,
	     const bool clobber = false);

    void set(const std::string & name,
	     const int & value,
	     const bool clobber = false);    
    /**
     * Fetches the value of a option
     * If the option is not found in the hash, the environment is inspected
     */
    bool get(const std::string & name,
	     std::string & value,
	     bool useEnvironment = true);

    /** Fetches a double option */
    bool get(const std::string & name,
	     double & value,
	     bool useEnvironment = true);

    /** Fetches a int option */
    bool get(const std::string & name,
	     int & value,
	     bool useEnvironment = true);
    
    /** Fetches a bool option */
    bool get(const std::string & name,
	     bool & value,
	     bool useEnvironment = true);

    bool contains(const std::string & name);

    const std::vector<std::string>& arguments() const { return _arguments; }

  private:

    bool readNameValue(const std::string & arg,
		       std::string & name,
		       std::string & value,
		       bool defaultValue = false);
    
    bool substitute(const std::string & raw,
		    std::string & value);

    /** finds first quote not preceded by backslash */
    static int findQuote(const std::string & input,
			 int offset);

    std::vector<std::string> _arguments; 
    std::unordered_map<std::string,std::string> _options;
    std::string _calledas;
    std::string _load_command;
  };

}

#endif /* TREELER_OPTIONS_H */
