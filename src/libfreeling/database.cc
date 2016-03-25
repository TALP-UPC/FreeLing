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

#include <sstream>
#include <fstream>

#include "freeling/morfo/database.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"DATABASE"
#define MOD_TRACECODE DATABASE_TRACE

  ///////////////////////////////////////////////////////////////
  ///  Create an empty database of given type
  ///////////////////////////////////////////////////////////////

  database::database(int type) {
    DBtype=type;
    if (DBtype == DB_PREFTREE)
      dbptree = new PrefTree();
  }

  database::~database() {
    if (DBtype == DB_PREFTREE) delete dbptree;
  }



  ///////////////////////////////////////////////////////////////
  ///  Create a database accessing module, loading a key-value
  /// plain file into a map
  ///////////////////////////////////////////////////////////////

  database::database(const wstring &dbFile) {

    // Load plain dictionary into RAM. 
    if (not dbFile.empty()) {
      wifstream fdic;
      util::open_utf8_file(fdic, dbFile);
      if (fdic.fail()) ERROR_CRASH(L"Error opening file "+dbFile);

      // get first line, check for DB format if any
      wstring line; 
      getline(fdic,line);

      DBtype=DB_MAP; // default
      if (line==L"DB_PREFTREE") 
        DBtype=DB_PREFTREE;

      // skip DB_type line, get first word.
      if (line==L"DB_PREFTREE" or line==L"DB_MAP")
        getline(fdic,line);

      do {
        // split line in key+data
        wstring::size_type pos = line.find(L" ");
        wstring key=line.substr(0,pos);
        wstring data=line.substr(pos+1);      
        add_database(key,data);
      } while (getline(fdic,line));

      fdic.close();
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  add a new pair (key, data) to the database, or if 'key'
  ///  already exists, concatenate 'data' to the existing entry
  ///////////////////////////////////////////////////////////////

  void database::add_database(const wstring &key,const wstring &data) {    
    switch (DBtype) {
    case DB_MAP: {
      map<wstring,wstring>::iterator p = dbmap.find(key);
      if (p!=dbmap.end()) p->second = p->second + L" "+data;
      else dbmap.insert(make_pair(key,data));
      break;
    }
    case DB_PREFTREE: {
      List::ListRecData *res = dbptree->find(key.c_str());
      dbptree->addWord(key.c_str(), res, data.c_str()); 
      break;
    }
      //case DB_HASHMAP: dbhmap.insert(make_pair(key,data));
      //                 break;

    default: break;
    }
  }


  ///////////////////////////////////////////////////////////////
  ///  remove database entry for given key
  ///////////////////////////////////////////////////////////////

  void database::remove_database(const wstring &key) {    

    switch (DBtype) {
    case DB_MAP: 
      dbmap.erase(key);
      break;

    case DB_PREFTREE: 
      dbptree->deleteWord(key.c_str());
      break;

      //case DB_HASHMAP: dbhmap.erase(key);
      //                 break;

    default: break;
    }
  }


  ///////////////////////////////////////////////////////////////
  /// Replace with 'data' information associated to entry 'key'
  /// If the entry is not there, insert it.
  ///////////////////////////////////////////////////////////////

  void database::replace_database(const std::wstring &key, const std::wstring &data) {

    // search key in appropriate index 
    switch (DBtype) {
    case DB_MAP: { 
      map<wstring,wstring>::iterator p=dbmap.find(key);
      if (p!=dbmap.end())
        p->second = data;  // replace data for given entry
      else 
        dbmap.insert(make_pair(key,data));  // new key, add pair. 
      break;
    }
    case DB_PREFTREE: {
      List::ListRecData *res = dbptree->find(key.c_str());
      if (res) 
        res->setValue(data.c_str());  // existing entry, replace data
      else 
        dbptree->addWord(key.c_str(), res, data.c_str());  // new entry, create it
      break;
    }
      //    case DB_HASHMAP: {
      //      __gnu_cxx::hash_map<wstring,wstring>::const_iterator p=dbhmap.find(key);
      //      if (p!=dbhmap.end()) data=p->second;
      //      break;
      //    }
    default: break;
    }
  }


  ///////////////////////////////////////////////////////////////
  ///  search for a string key in the DB, return associated string data.
  ///////////////////////////////////////////////////////////////

  wstring database::access_database(const wstring &key) const {
    // search key in appropriate index 
    switch (DBtype) {
    case DB_MAP: { 
      map<wstring,wstring>::const_iterator p=dbmap.find(key);
      if (p!=dbmap.end()) 
        return p->second;
      break;
    }
    case DB_PREFTREE: {
      List::ListRecData *res = dbptree->find(key.c_str());
      if (res)
        {
          const wchar_t *r = res->getValue();
          return r!=NULL? r: L"";
        }
      break;
    }
      //    case DB_HASHMAP: {
      //      __gnu_cxx::hash_map<wstring,wstring>::const_iterator p=dbhmap.find(key);
      //      if (p!=dbhmap.end()) data=p->second;
      //      break;
      //    }
    default: break;
    }

    return L"";
  }


  ///////////////////////////////////////////////////////////////
  /// dump listing of database to given stream
  ///////////////////////////////////////////////////////////////

  void database::dump_database(wostream &os, bool keysonly) const {
    switch (DBtype) {
    case DB_MAP:
      for (map<wstring,wstring>::const_iterator e=dbmap.begin(); e!=dbmap.end(); e++) {
        os<<e->first;
        if (not keysonly) os<<L" "<<e->second;
        os<<endl;
      }
      break;
  
    case DB_PREFTREE: 
      // to be done
      break;

    default : break;
    }
  }


} // namespace
