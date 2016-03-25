//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#ifndef _CLASSIFIER
#define _CLASSIFIER

#include <string>
#include <vector>

#include "freeling/omlet/example.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// The class classifier is an abstract class that implements 
  ///  a general ML classifier
  ////////////////////////////////////////////////////////////////

  class classifier {

  private:
    // label names, provided in the constructor
    std::vector<std::wstring> labels;
    std::wstring label_others;
  
  public:
    // constructor
    classifier(const std::wstring &);
    //destructor
    virtual ~classifier() {};
    // return number of classes of the model 
    virtual int get_nlabels() const;
    // Return the code for class with given name
    virtual int get_index(const std::wstring &) const;
    // return class name for class with given code
    virtual std::wstring get_label(int) const;
    // return default class name (or empty string if no default class used)
    virtual std::wstring default_class() const;
    /// Classify given example, returning predictions
    virtual void classify(const example &, double[]) const =0;
  };

} // namespace

#endif

