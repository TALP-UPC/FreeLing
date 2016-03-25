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
 * \author Xavier Carreras
 */
#ifndef TREELER_EXCEPTION_H
#define TREELER_EXCEPTION_H

#include <exception>

/**
 * \brief A simple exception used by treeler methods
 * \author Xavier Carreras
 */
class TreelerException: public std::exception {
 public:
  TreelerException(const std::string& module, const std::string& message)
    : _what(module + ": " + message) 
  {}

  ~TreelerException() throw() {};

  virtual const char* what() const throw()
  {
    return _what.c_str();
  }

 private:
  std::string _what;   
};


#endif
