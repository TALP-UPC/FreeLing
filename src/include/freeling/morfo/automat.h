////////////////////////////////////////////////////////////////
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

#ifndef _AUTOMAT
#define _AUTOMAT

#include <set>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/processor.h"
#include "freeling/morfo/traces.h"

namespace freeling {

#define MAX_STATES 100
#define MAX_TOKENS 50

#define MOD_TRACENAME L"AUTOMAT"
#define MOD_TRACECODE AUTOMAT_TRACE

  ////////////////////////////////////////////////////////////////
  /// Class to store status information
  ////////////////////////////////////////////////////////////////

  class automat_status : public processor_status {
  public:
    // shift beggining of multiword by N words (in_1982 -> 1982)
    int shiftbegin; 
  };

  ////////////////////////////////////////////////////////////////
  ///
  ///   Abstract class to implement a 
  ///   Finite-State Automaton which is used by modules 
  ///   recognizing multiwords (dates, numbers, quantities, ...).
  ///
  /// Details:
  ///
  ///   - Child classes must implement a DFA, by means
  ///   of a transition table. 
  ///   - IMPORTANT: Since many matches may be found in a
  ///   sentence, when arriving to a error or final state, 
  ///   the transition table must provide a default transition
  ///   to "stopState" where the automata will stop, and 
  ///   check for longest match found.
  ///   - Specific actions may be associated to each state 
  ///   using the "StateActions" virtual function. 
  ///   - Token codes for a word must be provided by the
  ///   "ComputeToken" virtual function.
  ///
  ///   Child classes must provide a constructor that:
  ///     - fills the "final" set
  ///     - fills the "trans" table
  ///     - sets initialState and stopState
  ///     - initializes any other information required by the child
  ///
  ///   Child classes must provide the virtual functions:
  ///
  ///      - virtual int ComputeToken(int,sentence::iterator, sentence &);
  ///       (computes the token code, given the state and word. 
  ///        Sentence reference is provided just in case extra info 
  ///        is necessary)
  ///
  ///      - virtual void ResetActions();
  ///      (reset any variables required by state actions)
  ///
  ///      -  virtual void StateActions(int, int, int, sentence::iterator);
  ///      (perform specific state actions)
  ///
  ///      - virtual void SetMultiwordAnalysis(sentence::iterator);
  ///      (once a mw has been detected and build, set its lemma&tag)
  ///
  ///   Child classes must declare and manage any private attribute or function
  ///   they may need to perform the expected computations
  ///
  //////////////////////////////////////////////////////////////////

  template <class T>
    class WINDLL automat : public processor {
  private:
    /// pure virtual function to be provided by the child class.
    /// Computes token code for current word in current state.
    virtual int ComputeToken(int, sentence::iterator &, sentence &) const =0;
    /// pure virtual function to be provided by the child class .
    /// Resets automaton internal variables when a new search is started.
    virtual void ResetActions(T *) const =0;
    /// pure virtual function to be provided by the child class.
    /// Performs appropriate internal actions, given origin and destinanation
    /// states, token code and word.
    virtual void StateActions(int, int, int, sentence::const_iterator, T *) const =0;
    /// pure virtual function to be provided by the child class.
    /// Sets analysis for pattern identified as a multiword.
    virtual void SetMultiwordAnalysis(sentence::iterator, int, const T *) const =0;
    /// virtual function (true by default). Allows the child class to perform a
    /// last-minute check before effectively building the multiword.
    virtual bool ValidMultiWord(const word &w, T *st) const { return(true); }

    /// Private function to re-arrange sentence when match found
    virtual sentence::iterator BuildMultiword(sentence &se, sentence::iterator start, sentence::iterator end, int fs, bool &built, T *st) const {
      sentence::iterator i;
      std::list<word> mw;
      std::wstring form;
    
      TRACE(3,L"Building multiword");
        
      // ignore initial tokens, if needed (e.g. in_1982 -> 1982)
      for (int i=0; i<((automat_status*)st)->shiftbegin && start!=end; i++) start++;
    
      for (i=start; i!=end; i++){
        mw.push_back(*i);           
        form += i->get_form()+L"_";
        TRACE(3,L"added next ["+form+L"]");
      } 
      // don't forget last word
      mw.push_back(*i);           
      form += i->get_form();
      TRACE(3,L"added last ["+form+L"]");
    
      // build new word with the mw list, and check whether it is acceptable
      word w(form,mw);
    
      if (ValidMultiWord(w,st)) {  
        TRACE(3,L"Valid Multiword. Modifying the sentence");
      
        // erasing from the sentence the words that composed the multiword
        end++;
        i=se.erase(start, end);
        // insert new multiword it into the sentence
        i=se.insert(i,w); 
      
        TRACE(3,L"New word inserted");
        // Set morphological info for new MW
        SetMultiwordAnalysis(i,fs,st);
        built=true;
      }
      else {
        TRACE(3,L"Multiword found, but rejected. Sentence untouched");
        ResetActions(st);
        i=start;
        built=false;
      }
    
      return(i);
    }
  
  
  protected:
    /// state code of initial state
    int initialState;
    /// state code for stop State
    int stopState;
    /// Transition tables
    int trans[MAX_STATES][MAX_TOKENS];
    /// set of final states
    std::set<int> Final;

  public:
    /// Constructor
    automat<T>() {};
    /// Destructor
    virtual ~automat<T>() {};

    /// Detect patterns starting at a specific word
    bool matching(sentence &se, sentence::iterator &i) const {
      sentence::iterator j,sMatch,eMatch; 
      int newstate, state, token, fstate;
      bool found=false;

      TRACE(3,L"Checking for mw starting at word '"+i->get_form()+L"'");

      T *pst = new T();
      se.set_processing_status((processor_status *)pst);  
    
      // reset automaton
      state=initialState;
      fstate=0;
      ResetActions(pst);
      ((automat_status *)pst)->shiftbegin=0;
    
      sMatch=i; eMatch=se.end();
      for (j=i;state != stopState && j!=se.end(); j++) {
        // request the child class to compute the token
        // code for current word in current state
        token = ComputeToken(state,j,se);
        // do the transition to new state
        newstate = trans[state][token];
        // let the child class perform any actions 
        // for the new state (e.g. computing date value...)
        StateActions(state, newstate, token, j, pst);
        // change state
        state = newstate;
        // if the state codes a valid match, remember it
        //  as the longest match found so long.
        if (Final.find(state)!=Final.end()) {
          eMatch=j;
          fstate=state;
          TRACE(3,L"New candidate found");
        }
      }
    
      TRACE(3,L"STOP state reached. Check longest match");
      // stop state reached. find longest match (if any) and build a multiword
      if (eMatch!=se.end()) {
        TRACE(3,L"Match found");
        i = BuildMultiword(se, sMatch, eMatch, fstate, found, pst);
        TRACE_SENTENCE(3,se);
      }
    
      se.clear_processing_status();
      return(found);
    }
  

    /// Detect patterns in sentence
    void analyze(sentence &se) const {
      sentence::iterator i;
      bool found=false;

      // check whether there is a match starting at each position i
      for (i=se.begin(); i!=se.end(); i++) {
        if (not i->is_locked_multiwords()) {
          if (matching(se, i)) found=true;
        }
        else {
          TRACE(3,L"Word '"+i->get_form()+L"' is locked. Skipped.");
        }
      }
    
      if (found) se.rebuild_word_index();
    
      // Printing module results
      TRACE_SENTENCE(1,se);
    }

    /// inherit other methods
    using processor::analyze;
  };

#undef MOD_TRACENAME
#undef MOD_TRACECODE

} // namespace

#endif

