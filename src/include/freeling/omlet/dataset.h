//////////////////////////////////////////////////////////////////
//
//    Omlet - Open Machine Learning Enhanced Toolkit
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This file is part of the Omlet library
//
//    The Omlet library is free software; you can redistribute it 
//    and/or modify it under the terms of the GNU Affero General Public
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
//    Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110-1301 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx Omega.S112 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

//
// Author: Xavier Carreras
//

#ifndef _DATASET
#define _DATASET

#include <list>
#include <vector>
#include "freeling/omlet/example.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class dataset stores a set of examples uset to train 
  /// adaboost models.
  ////////////////////////////////////////////////////////////////

  class dataset : public std::list<std::list<example>::iterator>
    {
    private:
      /// static list to store all examples from all datasets.
      /// thread unsafe, but it is only used in training.
      static std::list<example> all_examples;

      std::vector<int> size_pos;
      std::vector<int> size_neg;
      int dimension;
      int nlabels;
      void add_member(std::list<example>::iterator);

    public:
      dataset(int nlabels);

      /// input
      void add_example(const example &); 

      /// consultors
      //  int get_size() const;
      int get_negative_size(int l) const;
      int get_positive_size(int l) const;
      int get_nlabels() const;
      int get_dimension() const;

      void split(int feature, dataset &ds0, dataset &ds1) const;

      class iterator : public std::list<std::list<example>::iterator>::iterator {
      public:
        iterator () {};
        iterator (std::list<std::list<example>::iterator>::iterator x) : std::list<std::list<example>::iterator>::iterator(x) {}
        example& operator*() {
          return std::list<std::list<example>::iterator>::iterator::operator*().operator*();
        };
        example* operator->() {
          return std::list<std::list<example>::iterator>::iterator::operator*().operator->();
        };
      };
  
      class const_iterator : public std::list<std::list<example>::iterator>::const_iterator {
      public:
        const_iterator () {};
        const_iterator (std::list<std::list<example>::iterator>::const_iterator x) : std::list<std::list<example>::iterator>::const_iterator(x) {}
        example& operator*() {
          return std::list<std::list<example>::iterator>::const_iterator::operator*().operator*();
        };
        example* operator->() {
          return std::list<std::list<example>::iterator>::const_iterator::operator*().operator->();
        };
      };
  
      iterator begin() { return std::list<std::list<example>::iterator>::begin(); };
      iterator end() { return std::list<std::list<example>::iterator>::end(); };
      const_iterator begin() const { return std::list<std::list<example>::iterator>::begin(); };
      const_iterator end() const { return std::list<std::list<example>::iterator>::end(); };

    };

} // namespace

#endif



















