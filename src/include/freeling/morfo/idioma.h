//////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public License
//    (GNU AGPL) as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License 
//    along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Muntsa Padro (mpadro@lsi.upc.edu)
//             TALP Research Center
//             despatx Omega.S107 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

///////////////////////////////////
//
//  idioma.h
//
//  Class that implements a Visible Markov Model 
//  to compute the probability that a text is 
//  written in a certain language.
// 
///////////////////////////////////

#ifndef _IDIOMA_H
#define _IDIOMA_H

#include <map>
#include <list>
#include <vector>
#include <string>

#include "freeling/morfo/smoothingLD.h"

namespace freeling {

  ///////////////////////////////////
  /// Class "idioma" implements a visible Markov's model that calculates
  /// the probability that a text is in a certain language.
  ///////////////////////////////////

  class idioma {  

  private:
    // language code
    std::wstring LangCode;

    /// auxiliary for training 
    std::map<std::wstring,double> count;

    /// char to use to create initial state n-gram
    wchar_t phantom;
    /// order of ngram model
    int order;
    /// maximum perplexity to accept a sequence
    double threshold;

    // smoother for ngram probabilities
    smoothingLD<std::wstring,wchar_t> *smooth;

    /// convert a char to a writable represntation for the model file
    static std::wstring to_writable(wchar_t c);
    /// convert a ngram to a writable represntation for the model file
    static std::wstring to_writable(const std::wstring &);

    /// Initial ngram: n-1 phantom characters plus the first actual letter.
    static void initial_ngram(std::wistream &f, std::wstring &ngram, wchar_t &z, int ord, wchar_t ph);
    /// slide ngram window one position to the left
    static void next_ngram(std::wistream &f, std::wstring &ngram, wchar_t &z);
 
  public:
    /// Constructor, given the model file to load
    idioma(const std::wstring &);
    ~idioma();

    /// Calculates the probability that the text is in the instance language.
    double sequence_probability(std::wistream &, size_t &) const;
    /// Compute normalized language probability for given string
    double compute_perplexity(const std::wstring &) const; 

    /// Use given text file to count ngrams and create a model file.
    static void create_model(const std::wstring &modelFile,
                             std::wistream &f, 
                             const std::wstring &code, 
                             int order, 
                             wchar_t phantom);
      

    /// get iso code for current language
    std::wstring get_language_code() const;
    /// get maximum allowed perplexity
    double get_threshold() const;
  };

} // namespace

#endif
