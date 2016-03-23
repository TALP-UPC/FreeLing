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
 * @file registry.h
 * @brief Declares the class Registry
 * @author Terry Koo
 */

#ifndef TREELER_UTIL_REGISTRY_H
#define TREELER_UTIL_REGISTRY_H

#include <map>
#include <string>
#include <vector>

namespace treeler {

  /**
   *  \brief A class that provides facilities for dynamically allocating modules
   *  \author Terry Koo
   * 
   *  Registry provides facilities for dynamically allocating modules
   *  like optimizers (Trainer), feature generators (FGen), and
   *  structured models (StructuredModel).  This is based on the
   *  notion of auto-registration of objects.  
   *  
   * \see Register
   * 
   * 
   * \note This class was ported from egstra to treeler
   * 
   */
  class Registry {
  private:
    /* pointer to a function that creates an instance of a class */
    typedef void* (*constructor)();
    /* map from class-name to constructor */
    typedef std::map<std::string,constructor> consmap;
    /* map from type-name to consmap */
    typedef std::map<std::string,consmap> reg;
    
    /* global mapping from class-type to mapping from class-names to
       constructors.  this must be a pointer so that it can be
       properly initialized after the program starts. */
    static reg* _registry;
    
  public:
    /* instantiating a Registry object registers a constructor.  this
       is convenient for the purposes of auto-registering classes. */
    Registry(const char* type, const char* name, constructor f);
    ~Registry();
    
    /* register a constructor for the given type and name.  it is an
       error to register something for a type/name combination that is
       already registered */
    static void add(const char* type, const char* name, constructor f);
    
    /* create an object of the given type and name and return it */
    static void* construct(const char* type, const char* name);

    /* get a list of registered class-names of the given type */
    static void getnames(const char* type, std::vector<std::string>& names);
  };
}

#endif /* TREELER_UTIL_REGISTRY_H */
