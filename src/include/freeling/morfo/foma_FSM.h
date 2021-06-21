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

#ifndef _FOMA_FSM_H
#define _FOMA_FSM_H

#include <sstream>
#include <string>

#ifndef _Bool
typedef bool _Bool;
#endif


#include "foma/fomalib.h"


namespace freeling {

  ////////////////////////////////////////////////////////
  ///  Class foma_FSM is a wrapper for the FOMA library, 
  ///  for the specific use of getting entries from a 
  ///  dictionary with minimum edit distance to given key
  ////////////////////////////////////////////////////////

  class WINDLL foma_FSM {
  private:
    /// foma automaton
    struct fsm *fsa;
    /// Handle for foma minimum edit distance automaton
    struct apply_med_handle *h_fsa;
   
    /// Auxiliary for constructors:  create a FSM loading a file
    struct fsm* load_dictionary_file(const std::wstring &fname) const;
    /// Auxiliary for constructors:  create a FSM loading a text buffer
    struct fsm* load_dictionary_buffer(std::wistream &buff) const;
    /// Auxiliary for constructors: Load cost matrix
    void load_cost_matrix(const std::wstring &mcost);
    /// Auxiliary for constructors:  create a compound-detector FSM    
    void create_compound_FSA(const std::list<std::wstring> &joins);

    // Auxiliary for constructors: initialize FSM for minimum edit distance searches
    void init_MED();
    /// Auxiliary for constructor:  complete FSM alphabet with missing symbols from cost matrix
    void complete_alphabet(const std::wstring &);
    // Auxiliary for constructor: Update FSM alphabet by composing it with Sigma* 
    void update_FSM_alphabet(const std::set<std::wstring> &);

  public:
    /// build regular automaton from text file, optional cost matrix, join chars if it is a compound FSA
    foma_FSM(const std::wstring &, const std::wstring &mcost=L"", 
             const std::list<std::wstring> &joins=std::list<std::wstring>());
    /// build regular automaton from text buffer, optional cost matrix, join chars if it is a compound FSA
    foma_FSM(std::wistream &, const std::wstring &mcost=L"", 
             const std::list<std::wstring> &joins=std::list<std::wstring>());
    /// clear 
    ~foma_FSM();

    /// Use automata to obtain closest matches to given form, and add them to given list.
    void get_similar_words(const std::wstring &, std::list<freeling::alternative> &) const;    
    /// set maximum edit distance of desired results
    void set_cutoff_threshold(int);
    /// set maximum number of desired results
    void set_num_matches(int);
    /// Set cost for basic SED operations (insert, delete, substitute)
    void set_basic_operation_cost(int);
    /// Set cost for a particular SED operation (replace "in" with "out")
    void set_operation_cost(const std::wstring &, const std::wstring &, int);
    /// get FSM alphabet
    std::set<std::wstring> get_alphabet();
  };

}

#endif
