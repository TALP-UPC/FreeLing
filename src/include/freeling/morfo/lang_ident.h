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

// /////////////////////////////////
//
//  lang_ident.h
//
//  Class that implements a language identifier
//
///////////////////////////////////

#ifndef _LANGIDENT_H
#define _LANGIDENT_H

#include <map>
#include <set>
#include <vector>
#include <string>

#include "freeling/windll.h"
#include "freeling/morfo/idioma.h"

namespace freeling {

  //////////////////////////////////////////////////////////////
  /// Class "lang_ident" checks a text against all known languages
  /// and sorts the results by probability.
  /// It creates an instance of "idioma" for each known language, and
  /// checks input text against all existing instances.
  //////////////////////////////////////////////////////////////

  class WINDLL lang_ident {
  private:
    /// List of known languages and language models
    std::map<std::wstring,idioma*> idiomes;
    std::set<std::wstring> all_known_languages;

    /// fill a vector with unsorted perplexities for each language in given set
    void language_perplexities (std::vector<std::pair<double,std::wstring> > &, 
                                const std::wstring &, 
                                const std::set<std::wstring>&) const;

  public:
    /// Build an empty language identifier.
    lang_ident();
    /// Build a language identifier, read options from given file.
    lang_ident(const std::wstring &cfgfile);
    /// destructor
    ~lang_ident();
    /// load given language from given model file, add to existing languages.
    void add_language(const std::wstring &modelfile);
    /// train a model for a language, store in modelFile, and add 
    /// it to the known languages list.
    void train_language(const std::wstring &textfile, const std::wstring &modelfile, 
                        const std::wstring &code, size_t order);
    /// Classify the input text and return the code of the best language among
    /// those in given set. If set is empty all known languages are considered.
    /// If no language reaches the threshold,  "none" is returned
    std::wstring identify_language (const std::wstring &text, 
                                    const std::set<std::wstring> &ls=std::set<std::wstring>()) const; 
    /// Classify the input text and return the code and perplexity for each language
    /// in given set. If set is empty, all known languages are considered.
    void rank_languages (std::vector<std::pair<double,std::wstring> > &result, 
                         const std::wstring &text,
                         const std::set<std::wstring> &ls=std::set<std::wstring>()) const;
    /// for Python API
    std::vector<std::pair<double,std::wstring> > rank_languages (const std::wstring &text,
                                                                 const std::set<std::wstring> &ls=std::set<std::wstring>()) const;
  };

} // namespace

#endif
