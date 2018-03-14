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


#ifndef SAFEMAP_H
#define SAFEMAP_H

#define BOOST_SYSTEM_NO_DEPRECATED
#include <boost/thread/mutex.hpp>
#include <iostream>

////////////////////////////////////////////////////////////////
/// This class provides a map with thread_safe writing acces.
////////////////////////////////////////////////////////////////

template <class T1, class T2> 
  class safe_map : public std::map<T1,T2> {

 private:
    boost::mutex sem;

 public:
    // check if key is in cache, if found, return true  
    // and set value in second parameter
    bool find_safe(const T1 &key, T2 &val) {
      bool b=false;
      sem.lock();
      typename std::map<T1,T2>::const_iterator p=this->find(key);
      if (p!=this->end()) {
        b = true;
        val = p->second;
      }
      sem.unlock();
      return b;
    }

    // insert new pair in cache, with mutex.
    void insert_safe(const T1 &key, const T2 &val) {
      sem.lock();
      this->insert(make_pair(key,val));
      sem.unlock();
    }

    // remove pair from cache, with mutex.
    void erase_safe(const T1 &key) {
      sem.lock();
      this->erase(key);
      sem.unlock();
    }
};


#endif
