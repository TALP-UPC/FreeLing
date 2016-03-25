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

#ifndef _TAGGER
#define _TAGGER

#include <map>
#include <list>
#include <set>

#include "freeling/windll.h"
#include "freeling/safe_map.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/tagger.h"
#include "freeling/morfo/tagset.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///
  ///  The class viterbi stores the two maps for each
  /// observation: The delta map stores the maximum probability
  /// for each state in that observation, the phi map stores
  /// the backpath to maximize the probability.
  ///  An instance of this class is created for each sentence
  /// to be tagged, and destroyed when work is finished.
  ///
  ////////////////////////////////////////////////////////////////

  typedef std::pair<std::wstring, std::wstring> bigram;

  class trellis {     
  private:

    /// Each trellis node contains an ordered set of paths.
    /// This class stores one path element.
    class element {
    public:
      /// origin state
      bigram state;
      /// kbest sequence in origin state
      int kbest;
      /// probability of the whole sequence from this origin
      double prob;

      /// constructor
      element(const bigram &, int, double);
      /// destructor
      ~element();
      /// Comparison to sort paths by probability
      bool operator<(const element &) const;
      bool operator>(const element &) const;
      /// Comparison (to please MSVC)
      bool operator==(const element &) const;
    };

    /// Trellis: array of columns.
    /// Each column associates each state with a set of path elements
    /// sorted by descending probability
    typedef std::multiset<element,std::greater<element> > path_elements;
    std::vector<std::map <bigram, path_elements > > trl;

    /// Number of kbest paths to store
    unsigned int kbest;

  public:
    /// Constructor
    trellis(int, unsigned int kb=1);
    /// Destructor
    ~trellis();

    /// add new element to the trellis
    void insert(int, const bigram &, const bigram &, int kb, double);
    /// retieve delta component for kth best path (count starts at 0) for given T and state. 
    double delta(int, const bigram &, unsigned int k=0) const;
    /// retieve phi component for kth best path (count starts at 0) for given T and state
    std::pair<bigram, int> phi(int, const bigram &, unsigned int k=0) const;
    /// Get number of elements in kbest set for given time&state (at most kbest, maybe less)
    int nbest(int, const bigram &) const;

    /// log prob for zero
    static const float ZERO_logprob;
    static const bigram initState;
    static const bigram endState;
  };


  ////////////////////////////////////////////////////////////////
  ///
  ///  The class emission_states stores the list of 
  /// states in the HMM that *may* be generating a given
  /// word given the two previous words (and their valid tags).
  ///
  ////////////////////////////////////////////////////////////////

  class emission_states: public std::set<bigram > {};

  ////////////////////////////////////////////////////////////////
  ///
  ///  The class hmm_tagger implements the syntactic analyzer
  /// and is the main class, which uses all the others.
  ///
  ////////////////////////////////////////////////////////////////

  class WINDLL hmm_tagger: public POS_tagger {
  private:
    // Tagset description
    const tagset *Tags;

    /// maps to store the probabilities
    std::map <std::wstring, double> PTag;
    std::map <bigram, double> PBg;
    std::map <std::wstring, double> PTrg;
    std::map <bigram, double> PInitial;
    std::map <std::wstring, double> PWord;

    /// set of hand-specified forbidden bigram and trigram transitions
    std::multimap <std::wstring, std::wstring> Forbidden;

    // probability for unobserved sentence initial trigrams
    double probInitial;
    // probability for unobserved words  
    double probUnobserved;

    /// thread-safe probabilitiy cache, to speed up computations
    safe_map<std::wstring,double> *pA_cache;
    //prob_cache *pB_cache;

    /// number of best paths to compute
    unsigned int kbest;

    /// coeficients to compute linear interpolation
    double c[3];

    bool is_forbidden(const std::wstring &, sentence::const_iterator) const;
    double ProbA_log(const bigram &, const bigram &, sentence::const_iterator) const;
    double ProbB_log(const bigram &, const word &) const;
    double ProbPi_log(const bigram &) const;

    /// compute possible emission states for each word in sentence.
    std::list<emission_states> FindStates(const sentence &) const;

  public:
    /// Constructor
    hmm_tagger(const std::wstring &, bool, unsigned int, unsigned int kb=1);
    /// Destructor
    ~hmm_tagger();

    /// analyze given sentence 
    void annotate(sentence &) const;
    /// Given an *annotated* sentence, compute (log) probability of k-th best 
    /// sequence according to HMM parameters.
    double SequenceProb_log(const sentence &, int k=0) const;

  };

} // namespace

#endif


