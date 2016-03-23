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

///////////////////////////////////////////////
//
//   Author: Stanilovsky Evgeny 
//
///////////////////////////////////////////////

#ifndef _PREF_TREE_H_
#define _PREF_TREE_H_

#include <string>
#include <vector>

namespace freeling {

  ///////////////////////////////////////////////
  // Class List is auxiliary to PrefTree
  ///////////////////////////////////////////////

  class List {
  public:
    struct ListRecBase  {
    ListRecBase(): next(NULL), nextList(NULL) {}
    ListRecBase(wchar_t c): symb(c), next(NULL), nextList(NULL) {}
      virtual ~ListRecBase() {}
      wchar_t symb;
      ListRecBase *next;
      List *nextList;
    };
  
    struct ListRec: public ListRecBase {
    ListRec(): ListRecBase() {}
    ListRec(wchar_t c): ListRecBase(c) {}
      virtual ~ListRec() {};
    };
    
    struct ListRecEnd: public ListRecBase {
    ListRecEnd(): ListRecBase() {}
    ListRecEnd(wchar_t c): ListRecBase(c) {}
    };
    
    struct ListRecData: public ListRecBase {
    ListRecData(): ListRecBase(), value(NULL) {}
    ListRecData(wchar_t c): ListRecBase(c), value(NULL) {}
      ~ListRecData() {delete[] value;}
      const wchar_t *value;
      void setValue(const wchar_t *data);
      void delValue() {delete[] value; value = NULL;};      
      const wchar_t *getValue()
      {
        return value;
      }
    };
    
  List(): begin(0), end(0) {}
    inline ListRecBase* find(const wchar_t c);
    void* push(const wchar_t c, const bool wordEnd = false);
    virtual ~List();

  private:
    ListRecBase *begin, *end;
  };

  ///////////////////////////////////////////////
  // Class PrefTree implements a prefix Tree index
  ///////////////////////////////////////////////

  class PrefTree {
  private:
    List *root;
    size_t DELIM_LEN;

  public:
    PrefTree() { root = new List(); DELIM_LEN = wcslen(L" ");}
    ~PrefTree() { delete root; }
    void addWord(const wchar_t *word, List::ListRecBase *lr, const wchar_t *data);
    List::ListRecBase *findWord(const wchar_t *) const;
    List::ListRecData *find(const wchar_t *word) const;
    void deleteWord(const wchar_t *);
  };

} // namespace

#endif
