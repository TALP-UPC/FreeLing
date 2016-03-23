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
 * \file register.h
 * \brief Macros for registering objects
 * \author Terry Koo
 * \note This file was ported from egstra
 * \see Registry
 */

#ifndef TREELER_REGISTER_H
#define TREELER_REGISTER_H

#include "scripts/registry.h"

/* Use these macros to register your objects */
#define REGISTER_SCRIPT(name, object)		\
  REGISTER(script, name, object)
#define REGISTER_TRAINER(name, object)		\
  REGISTER(trainer, name, object)
#define REGISTER_FGEN(name, object)		\
  REGISTER(fgen, name, object)
#define REGISTER_MODEL(name, object)		\
  REGISTER(model, name, object)


/**
 *  Underlying macro that actually registers an object.  There are two
 *  steps to the registration process:
 *
 *  -# Create a wrapper function that constructs the desired object.
 *
 *  -# In order to get the register::add() function to be called,
 *     instantiate a single registry object at global scope.  Thus, at
 *     program initialization, the object and constructor will be
 *     registered. 
 *
 * \see Registry
 */
#define REGISTER(type, name, object)				\
  void*								\
  make_ ## type ## _ ## name() {				\
    return new treeler::object();				\
  }								\
  treeler::Registry						\
  reg_ ## type ## _ ## name(#type,				\
			    #name,				\
			    make_ ## type ## _ ## name)

#endif /* TREELER_REGISTER_H */
