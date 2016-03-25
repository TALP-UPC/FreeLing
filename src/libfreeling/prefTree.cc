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

#include "freeling/morfo/prefTree.h"
#include "freeling/morfo/traces.h"


namespace freeling {

#define MOD_TRACENAME L"PREF_TREE"
#define MOD_TRACECODE DICT_TRACE
  ///////////////////////////////////////////////
  //   Class List implementation
  ///////////////////////////////////////////////

  List::~List()
  {
    if (!begin)
      return;
    ListRecBase *tmp = begin, *tmp1;
    while (tmp)
      {   
        tmp1 = tmp;
        tmp = tmp->next;
        delete tmp1->nextList;
        delete tmp1;
      }
  }

  void *List::push(const wchar_t c, const bool wordEnd)
  {
    ListRecBase *n = find(c);
    if (!n) 
      {
        if (!wordEnd) 
          n = new ListRecBase(c);
        else 
          n = new ListRecData(c);

        if (end) 
          end->next = n;    
        else 
          begin = n;

        end = n;
      }
    else if (wordEnd) 
      {      
        ListRecBase *tmp = begin, *prev = NULL;
        while (tmp != n)
          {  
            prev = tmp;
            tmp = tmp->next;
          }

        if (n==end) 
          end = NULL;
        
        ListRecBase *ntmp = n->next; // save previous state - next
        List *l = n->nextList;       // save previous state - nextList
        delete n;                    // delete old ListRecBase
        n = new ListRecData(c);      // create new ListRecData
        n->next = ntmp;
        n->nextList = l;
      
        if (prev) prev->next = n;
        else begin = n;
      
        if (!end) end = n;
      }
    
    if (!wordEnd)
      {
        if (!(n->nextList))
          n->nextList = new List();    
        return n->nextList;
      }
    else
      return n;
  }

  List::ListRecBase *List::find(const wchar_t c)
  {
    if (!begin)
      return 0;
    ListRecBase *tmp = begin;
    while (tmp && tmp->symb != c)
      {   
        if (tmp->next)
          {
            if (tmp->next->symb == c)
              return tmp->next;
            tmp = tmp->next->next;
          }
        else
          {
            tmp = NULL;
            break;                  
          }
      }
    return tmp? tmp: NULL;
  }

  void List::ListRecData::setValue(const wchar_t *data) 
  {
    delete[] value;
    value = data;
  }

  ///////////////////////////////////////////////
  //   Class PrefTree implementation
  ///////////////////////////////////////////////

  void PrefTree::addWord(const wchar_t *word, List::ListRecBase *lr, const wchar_t *sIn)
  {
    if (!word)
      return;
    List *l = NULL;
    if (!lr)
      {
        l = root;
        while(*word)
          {
            l = (List *)l->push(*word, !*(word+1));
            ++word;
          }
      }
    else
      l = (List *)lr;
  
    List::ListRecData *ldata = (List::ListRecData *)l;
    const wchar_t *data = ldata->getValue();
    wchar_t *newStr;
    size_t s = 0;
    if (data)
      {
        s = wcslen(data) + DELIM_LEN + wcslen(sIn) + 1;;
        newStr = new wchar_t[s];
#if defined WIN32 || defined WIN64
        wcscpy_s(newStr, s, data);
        wcscat_s(newStr, s, L" ");
#else
        wcscpy(newStr, data);
        wcscat(newStr, L" ");
#endif
      }
    else
      {
        s = wcslen(sIn) + 1;
        newStr = new wchar_t[s];
        *newStr = 0;
      }
#if defined WIN32 || defined WIN64
    wcscat_s(newStr, s, sIn);
#else
    wcscat(newStr, sIn);
#endif
    ldata->setValue(newStr);
  }

  List::ListRecBase *PrefTree::findWord(const wchar_t *word) const
  {
    List *n = root;
    List::ListRecBase *lr = NULL;
    while (*word && n && (lr = n->find(*word)))
      {
        n = lr->nextList;
        ++word;
      }

    if (!lr || *word)
      return NULL;
    else
      return lr;
  }

  List::ListRecData *PrefTree::find(const wchar_t *word) const
  {
    List::ListRecBase *rec = findWord(word);
    if (!rec)
      return NULL;
    List::ListRecData *data = dynamic_cast<List::ListRecData *>(rec);
    //if (!data) for pure key
    //    List::ListRecEnd *end = dynamic_cast<List::ListRecData *>(rec);
    return data? data: NULL;
  }

  void PrefTree::deleteWord(const wchar_t *word)
  {
    List::ListRecData* data = this->find(word);
    if (data && data->getValue())
      data->delValue();
  }

} // namespace
