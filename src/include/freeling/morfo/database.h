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

#ifndef _DATABASE
#define _DATABASE

#include <string>
#include <map>
//#include <hash_map>

#include "freeling/morfo/prefTree.h"
#include "freeling/windll.h"

#define DB_MAP 0
#define DB_PREFTREE 1
//#define DB_HASHMAP 2

namespace freeling {

  ///////////////////////////////////////////////////////////////
  ///  Class to wrap a berkeley DB database and unify access.
  ///  All databases in Freeling use a string key to index string data.
  ///////////////////////////////////////////////////////////////

  class WINDLL database {
 
  private:
    /// DB type (map, hash_map, preftree)
    int DBtype;
    /// Dictionary for map type
    std::map<std::wstring,std::wstring> dbmap;
    /// Dictionary for hash_map type
    //__gnu_cxx::hash_map<std::wstring, std::wstring> dbhmap;
    /// Dictionary for pref tree type
    PrefTree *dbptree;

  public:
    /// constructor
    database(int);
    /// constructor
    database(const std::wstring &);
    /// destructor
    ~database();

    /// add a new pair (key, data) to the database
    void add_database(const std::wstring &, const std::wstring &);
    /// remove an entry from the database
    void remove_database(const std::wstring &);    
    /// replace the data for an entry in the database
    void replace_database(const std::wstring &, const std::wstring &);
    /// search for a string key in the DB, return associated string data.
    std::wstring access_database(const std::wstring &) const;
    /// dump listing of database content to given stream
    void dump_database(std::wostream &, bool keysonly=false) const;
  };

} // namespace

#endif
