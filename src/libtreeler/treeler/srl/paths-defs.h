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
 * \file   paths_defs.h
 * \brief  Has the paths defs
 */

#ifndef TREELER_PATHS_DEFS_H
#define TREELER_PATHS_DEFS_H

#include <list>
#include<string>

namespace treeler {
  namespace srl {
      typedef std::list<int> NodePath;
      typedef std::list<std::string> SynLabelPath;
      typedef std::list<int> SynIntLabelPath;
      typedef std::list<bool> UpDownPath;
  } //end of namespace srl
} // end of treeler namespace

#endif /* TREELER_SRL_PATHS_H */
