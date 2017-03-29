
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

#ifndef _PROBABILITIES
#define _PROBABILITIES

#include <map>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/processor.h"
#include "freeling/morfo/tagset.h"

namespace freeling {

  const std::wstring RE_FZ=L"^[FZ]";

  ////////////////////////////////////////////////////////////////
  /// Class probabilities sets lexical probabilities for
  /// each PoS tag of each word in a sentence.
  ////////////////////////////////////////////////////////////////

  class WINDLL probabilities : public processor {
  private:
    /// Auxiliary regexps
    freeling::regexp RE_PunctNum;

    /// Probability threshold for unknown words tags
    double ProbabilityThreshold;
    /// Tagset description, to compute short versions of tags.
    const tagset *Tags;

    /// Interpolation factor to favor suffix probabilities versus ambiguity-class 
    /// probabilities when smoothing known but unobserved words 
    double BiassSuffixes; 

    /// lambda parameter for smoothing via Lidstone's Law 
    double LidstoneLambdaLexical; 
    double LidstoneLambdaClass; 

    /// whether to use guesser for unknown words.
    bool activate_guesser;

    /// unigram probabilities
    std::map<std::wstring,double> single_tags;
    /// probabilities for usual ambiguity classes
    std::map<std::wstring,std::map<std::wstring,double> > class_tags;
    /// lexical probabilities for frequent words 
    std::map<std::wstring,std::map<std::wstring,double> > lexical_tags;
    /// list of tags and probabilities to assign to unknown words
    std::map<std::wstring,double> unk_tags;
    /// list of tag frequencies for unknown word suffixes
    std::map<std::wstring,std::map<std::wstring,double> > unk_suffs;
    /// unknown words suffix smoothing parameter;
    double theeta;
    /// length of longest suffix
    std::wstring::size_type long_suff;

    // list of default preferences of lemma/pos to sort analysis 
    // in case tagger leaves residual ambiguity
    std::set<std::wstring> lemma_prefs;
    std::set<std::wstring> pos_prefs;

    /// Smooth probabilities for the analysis of given word
    void smoothing(word &) const;
    /// Compute p(tag|suffix) using recursively shorter suffixes.
    double compute_probability(const std::wstring &, double, const std::wstring &) const;
    /// Guess possible tags, keeping some mass for previously assigned tags    
    void guesser(word &, double mass=1.0) const;
    /// compare two analysis to set the right order of preference
    bool less(const analysis &a1, const analysis &a2) const;
    /// sort given analysis list using lemma and pos preferences
    void sort_list(std::list<analysis> &ls) const;

  public:
    /// Constructor
    probabilities(const std::wstring &, double);
    /// Destructor  
    ~probabilities();

    /// Assign probabilities for each analysis of given word
    void annotate_word(word &) const;

    /// Turn guesser on/of
    void set_activate_guesser(bool);

    /// Assign probabilities to tags for each word in sentence
    void analyze(sentence &) const;

    /// inherit other methods
    using processor::analyze;
  };

} // namespace

#endif
