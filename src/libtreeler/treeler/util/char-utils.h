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
 * \file char-utils.h
 * \brief Declares static utility functions for string manipulation
 * \author Mihai Surdeanu
 * \note This file was ported from egstra and deeper
 * \todo Adapt code to Treeler style guides for c++
 */


#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <typeinfo>

#ifndef TREELER_CHAR_UTILS_H
#define TREELER_CHAR_UTILS_H

#undef small

namespace treeler {
  
  template <typename T>
  void simpleTokenize(const std::string & input,
		      std::vector<T> & output,
		      const std::string & separators);


  bool emptyLine(const std::string & input);

  int toInteger(const std::string & s);

  void toString(const int i, std::string& s);

  std::string toUpper(const std::string & s);

  std::string toLower(const std::string & s);

  bool startsWith(const std::string & big,
		  const std::string & small);

  bool endsWith(const std::string & big,
		const std::string & small);

  std::string stripString(const std::string & s,
			  int left,
			  int right);


  // TEMPLATE IMPLEMENTATIONS

  template <typename T>
  void simpleTokenize(const std::string & input,
		      std::vector<T> & output,
		      const std::string & separators);

}

#endif /* TREELER_CHAR_UTILS_H */
