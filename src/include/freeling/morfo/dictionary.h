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
#include "freeling/morfo/analyzer_config.h"
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
    // store configuration options used to create the module
    analyzer_config initial_options;
    // invoke options to be used in subsequent calls (defaults to 
    // initial_options, but can be changed)
    analyzer_config::invoke_options current_invoke_options;

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
    dictionary(const analyzer_config &opts);
    /// Destructor
    ~dictionary();

    /// convenience:  retrieve options used at creation time (e.g. to reset current config)
    const analyzer_config& get_initial_options() const;
    /// set configuration to be used by default
    void set_current_invoke_options(const analyzer_config::invoke_options &opt);
    /// get configuration being used by default
    const analyzer_config::invoke_options& get_current_invoke_options() const;

    /// add analysis to dictionary entry (create entry if not there)
    void add_analysis(const std::wstring &, const analysis &);
    /// remove entry from dictionary
    void remove_entry(const std::wstring &);
    
    /// Get dictionary entry for a given form, add to given list.
    void search_form(const std::wstring &, std::list<analysis> &) const;
    /// Fills the analysis list of a word, checking for suffixes and contractions.
    /// Returns true iff the form is a contraction, returns contraction components
    /// in given list
    bool annotate_word(word &, std::list<word> &, const analyzer_config::invoke_options &opts) const;
    /// annotate word with default options
    bool annotate_word(word &, std::list<word> &) const;
    /// Fills the analysis list of a word, checking for suffixes and contractions.
    /// Never retokenizing contractions, nor returning component list.
    /// It is just a convenience equivalent to "annotate_word(w,dummy,opts)
    /// with opts.retokenize=opts.compound=false"
    void annotate_word(word &) const;
    /// Get possible forms for a lemma+pos
    std::list<std::wstring> get_forms(const std::wstring &, const std::wstring &) const;

    /// dump dictionary to a buffer. Either full entries or keys only
    void dump_dictionary(std::wostream &, bool keysonly=false) const;

    /// analyze given sentence with given options
    void analyze(sentence &se, const analyzer_config::invoke_options &opts) const;
    /// analyze given sentence with default options
    void analyze(sentence &se) const;

    /// inherit other methods
    using processor::analyze;
  };

} // namespace

#endif
