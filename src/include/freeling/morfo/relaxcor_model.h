//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2004   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    General Public License for more details.
//
//    You should have received a copy of the GNU General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
//    Author: Jordi Turmo
//    e-mail: turmo@lsi.upc.edu
//
//////////////////////////////////////////////////////////////////

#ifndef RELAXCOR_MODEL_H
#define RELAXCOR_MODEL_H

#include <map>
#include "freeling/windll.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  The basic class relaxcor_model implements the mention-pair model
  ////////////////////////////////////////////////////////////////

  class relaxcor_model {

  public:

    // type representing the feature names with their IDs
    typedef std::map<std::wstring, unsigned int> TfeaturesNames;
    typedef std::map<unsigned int, std::wstring> TfeaturesIDs;
    // type representing the sparse feature vector for any pairwise model
    typedef std::map<unsigned int,bool> Tfeatures;

  protected:

    /// variable for the feature names with their ids 
    TfeaturesNames _Feature_names;
    TfeaturesIDs _Feature_ids;

    /// print all the names and values of the model features
    void print_feature_names() const;

  public:

    relaxcor_model(const std::wstring&);
    virtual ~relaxcor_model() {};
    
    /// checks for existing feature name and returns id if it exists
    bool feature_name_defid(const std::wstring &name, unsigned int &id) const;
    bool is_feature_name(const std::wstring &name) const;
    unsigned int feature_name_id(const std::wstring &name) const;
    /// checks for existing feature id and return name if it exists
    bool feature_id_defname(unsigned int id, std::wstring &name) const;
    std::wstring feature_id_name(unsigned int id) const;

    /// iterators of feature names
    TfeaturesNames::const_iterator begin_features() const;
    TfeaturesNames::const_iterator end_features() const;

    /// returns the weight measuring the coreference degree of a mention-pair
    /// (positive or negative weight) 
    /// Each mention-pair is given as the set of its features
    
    virtual double weight(const Tfeatures&) const = 0;
    virtual void dump() const = 0;

    // print a feature vector (or constraint condition vector)
    // prints only requested features (active or inactive).
    std::wstring print(const Tfeatures &f, bool active) const;
  };

} //namespace

#endif
