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
 * \file   dictionary.h
 * \brief  Declaration of class Dictionary
 * \author Xavier Carreras
 */
#ifndef TREELER_DICTIONARY_H
#define TREELER_DICTIONARY_H

#include <vector>
#include <map>
#include <string>
#include <iostream>

#include "treeler/base/exception.h"
#include "treeler/base/windll.h"

namespace treeler {

  /**
   * \brief A dictionary of values, mapping strings to/from integers ids
   * \author Xavier Carreras
   * 
   * Maps values from strings to integer ids and viceversa. The
   * mapping is read from a file, whose format is a number of lines of
   * the form:
   * <id> <string>
   * 
   * Other fields in a line are ignored. The integer ids start at 0
   * and need to be consecutive.
   * 
   * Supports unknown strings. These are mapped to value N, where N is
   * the size of the dictionary.
   *
   * \todo add options to load the dictionary for mapping or unmapping
   * only, in order to save memory 
   * 
   * \todo allow sparse dictionaries, i.e. the ids do not need to be
   * consecutive integers
   *
   * \todo add support for gzipped dictionary files
   *
   */
  class WINDLL Dictionary {
  public:
  Dictionary() 
    : _n(0), _id_unknowns(0), _str_unknowns("__UNK__"), _universal(false)
      {}
    
    //! Returns the number of entries in the dictionary
    int size() const { return _n; }

    //! empty or not
    bool empty() const { return _n==0; }

    //! Loads a dictionary from a file
    void load(const std::string& file);

    //! Sets the dictionary such that any string maps to a unique
    //! universal code with id=0 and lab as its label. The size of
    //! this dictionary will be 1.
    void universal(const std::string& lab) {
      _universal = true;
      _n = 1; 
      _str2int.clear();
      _int2str.clear();
      _id_unknowns = 0; 
      _str_unknowns = lab; 
    } 

    //! Dumps a dictionary to an output stream
    void dump(std::ostream& o) const;    

    //! Maps an integer value to its string value. Returns
    // the str for unknowns if the integer is not in the valid range.
    std::string map(const int& vint) const;

    //! Maps a string value to its integer id
    //! Returns the id for unknowns if not found
    int map(const std::string& vstr) const;      
    
    //! Maps a string value to its integer id
    //! Creates a new entry if necessary
    int map_or_create(const std::string& vstr);

    //! Sets the unknown string for ids not in range
    void set_str_unknowns(const std::string& s) { _str_unknowns = s; }

    //! Returns the unknown string for ids not in range
    std::string str_unknowns(const std::string& s) const { return _str_unknowns; }

    //! Returns the id for unknown strings
    int id_unknowns() const { return _id_unknowns; }

  private:
    typedef std::map<std::string,int> str2int_t; 
    typedef std::vector<std::string>  int2str_t; 
    
    int2str_t _int2str;
    str2int_t _str2int;
    int _n;
    int _id_unknowns; 
    std::string _str_unknowns;    
    bool _universal; 
  };

}

#endif
