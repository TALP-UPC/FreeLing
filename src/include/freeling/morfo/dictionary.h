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

#ifndef _DICTIONARY
#define _DICTIONARY

#include <map>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/processor.h"
#include "freeling/morfo/database.h"
#include "freeling/morfo/suffixes.h"
#ifndef NO_LIBFOMA
  #include "freeling/morfo/compounds.h"
#endif

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///
  ///  The class dictionary implements dictionary search and suffix
  ///  analysis for word forms. 
  ///
  ////////////////////////////////////////////////////////////////

  class WINDLL dictionary : public processor {

  private:
    /// configuration options
    bool InverseDict;

    /// Analysis settings
    bool RetokenizeContractions;
    bool AffixAnalysis;
    bool CompoundAnalysis;

    /// suffix analyzer
    affixes *suf;

    #ifndef NO_LIBFOMA
      /// compounds analyzer
      compounds *comp;
    #endif

    /// key-value file or hash
    database *morfodb;
    database *inverdb;

    /// check whether the word is a contraction, and if so, fill the
    /// list with the contracted words
    bool check_contracted(const std::wstring &, std::wstring, 
                          std::wstring, std::list<word> &) const;

    /// Generate valid tag combinations for an ambiguous contraction
    std::list<std::wstring> tag_combinations(std::list<std::wstring>::const_iterator, std::list<std::wstring>::const_iterator)
      const;
    /// parse data string into a map lemma->list of tags
    bool parse_dict_entry(const std::wstring &, std::list<std::pair<std::wstring,std::list<std::wstring> > >&) const;
    /// compact data in format lema1 pos1a|pos1b|pos1c lema2 pos2a|posb to save memory
    std::wstring compact_data(const std::list<std::pair<std::wstring,std::list<std::wstring> > > &) const;

  public:
    /// Constructor
    dictionary(const std::wstring &Lang, const std::wstring &dicFile, 
               const std::wstring &sufFile, const std::wstring &compFile,
               bool invDic=false, bool retok=true);
    /// Destructor
    ~dictionary();

    /// add analysis to dictionary entry (create entry if not there)
    void add_analysis(const std::wstring &, const analysis &);
    /// remove entry from dictionary
    void remove_entry(const std::wstring &);

    /// customize behaviour of dictionary for further analysis
    void set_retokenize_contractions(bool); 
    void set_affix_analysis(bool);
    void set_compound_analysis(bool);

    /// find out whether the dictionary has loaded an affix module
    bool has_affixes() const;
    /// find out whether the dictionary has loaded a compounds module
    bool has_compounds() const;
    
    /// Get dictionary entry for a given form, add to given list.
    void search_form(const std::wstring &, std::list<analysis> &) const;
    /// Fills the analysis list of a word, checking for suffixes and contractions.
    /// Returns true iff the form is a contraction, returns contraction components
    /// in given list
    bool annotate_word(word &, std::list<word> &, bool override=false) const;
    /// Fills the analysis list of a word, checking for suffixes and contractions.
    /// Never retokenizing contractions, nor returning component list.
    /// It is just a convenience equivalent to "annotate_word(w,dummy,true)"
    void annotate_word(word &) const;
    /// Get possible forms for a lemma+pos
    std::list<std::wstring> get_forms(const std::wstring &, const std::wstring &) const;

    /// dump dictionary to a buffer. Either full entries or keys only
    void dump_dictionary(std::wostream &, bool keysonly=false) const;

    /// analyze given sentence
    void analyze(sentence &) const;

    /// inherit other methods
    using processor::analyze;
  };

} // namespace

#endif
