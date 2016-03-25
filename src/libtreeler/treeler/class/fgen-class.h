//////////////////////////////////////////////////////////////////
//
//    Treeler - Open-source Structured Prediction for NLP
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//                          02111-1307 USA
//
//    contact: Xavier Carreras (carreras@lsi.upc.es)
//             TALP Research Center
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////
///  \author Xavier Carreras 
///

#ifndef TREELER_FGENCLASS_H
#define TREELER_FGENCLASS_H

#include "treeler/class/class-basic.h"
#include "treeler/util/options.h"

#include <string>

namespace treeler {

  class FGenClass  {
  public:
    FGenClass() 
      {
	_udim = -1;
        _dim = -1;
      }
    ~FGenClass() 
      {}

    void usage(const char* const mesg = "");
    void process_options(Options& options);

    /* part-wise fgen interface */
    void phi_start_pattern(const ClassPattern& x) {};
    void phi_end_pattern(const ClassPattern& x) {};
    const FeatureVector<>* phi(const ClassPattern& x, const PartClass& r);
    void discard(const FeatureVector<>* const F, const ClassPattern& x, const PartClass& r);


    /* batch fgen interface */
    const FeatureVector<>* phi(const ClassPattern& x);
    void discard(const FeatureVector<>* const F, const ClassPattern& x);

  private:    
    int _udim, _L, _dim, _spaces;
    std::string _name;
  };
}

#endif /* TREELER_FGENCLASS_H */
