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

#ifndef _SPLITTER
#define _SPLITTER

#include <map>
#include <set>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class splitter implements a sentence splitter, 
  ///  which accumulates lists of words until a sentence 
  ///  is completed, and then returns a list of 
  ///  sentence objects.
  ////////////////////////////////////////////////////////////////

  class WINDLL splitter {


  private:
    /// configuration options
    bool SPLIT_AllowBetweenMarkers;
    int SPLIT_MaxWords;
    /// Sentence delimiters
    std::set<std::wstring> starters;
    std::map<std::wstring,bool> enders;
    /// Open-close marker pairs (parenthesis, etc)
    std::map<std::wstring,int> markers;

    /// check for sentence markers
    bool end_of_sentence(std::list<word>::const_iterator, const std::list<word> &) const;

    // current state: Remember half-recognized sentences and open markers.
    class splitter_status {
      public:
        bool betweenMrk;
        int no_split_count;
        std::list<int> mark_type;
        std::list<std::wstring> mark_form;
        /// words of current sentence accumulated so far.
        sentence buffer; 
        /// count sentences to assign them id numbers
        size_t nsentence;
    };

  public:
    /// Constructor, given option file
    splitter(const std::wstring &splitfile);
    /// Destructor
    ~splitter();

    typedef splitter_status* session_id;
    /// open a splitting session, get session id
    session_id open_session() const;
    /// close splitting session
    void close_session(session_id ses) const;

    /// Add given list of words to the buffer, and put complete sentences 
    /// that can be build into ls.
    /// The boolean states if a buffer flush has to be forced (true) or
    /// some words may remain in the buffer (false) if the splitter 
    /// needs to wait to see what is coming next.
    /// Each thread using the same splitter needs to open a new session.
    void split(session_id ses, const std::list<word> &lw, bool flush, std::list<sentence> &ls) const;

    /// same than previous method, but result sentences are returned.
    std::list<sentence> split(session_id ses, const std::list<word> &ls, bool flush) const;

    // sessionless splitting.  
    // Equivalent to opening a session, split with flush=true, and close the session
    void split(const std::list<word> &lw, std::list<sentence> &ls) const;
    std::list<sentence> split(const std::list<word> &ls) const;
  };

} // namespace

#endif

