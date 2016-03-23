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

#ifndef _ACCENTS_MOD
#define _ACCENTS_MOD

#include <string>
#include <set>
#include <map>

#include "freeling/morfo/sufrule.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///
  ///  The abstract class accents_module generalizes accentuation
  /// rules for different languages.
  ///
  ////////////////////////////////////////////////////////////////

  class accents_module {

  public:
    /// Constructor
    accents_module();
    /// Specific accentuation patterns
    virtual void fix_accentuation(std::set<std::wstring> &, const sufrule &) const =0;
    /// Destructor
    virtual ~accents_module() {};

  };


  ////////////////////////////////////////////////////////////////
  ///
  ///  Derived accents_module for null accentuation (eg english).
  ///
  ////////////////////////////////////////////////////////////////

  class accents_default: public accents_module {

  public:
    /// Constructor
    accents_default();

    /// default accentuation patterns
    void fix_accentuation(std::set<std::wstring> &, const sufrule &) const;
  };


  ////////////////////////////////////////////////////////////////
  ///
  ///  Derived accents_module for Spanish accentuation.
  ///
  ////////////////////////////////////////////////////////////////

  class accents_es: public accents_module {

  private:
    /// regexps to check for useful spanish accentuation and syllabic patterns
    static const regexp llana_acc;
    static const regexp aguda_mal;
    static const regexp monosil;
    static const regexp last_vowel_put_acc;
    static const regexp last_vowel_not_acc;
    static const regexp any_vowel_acc;
    static const regexp diacritic;
    /// maps to store equivalences between accentued and not accentued chars.
    static const std::map<std::wstring,std::wstring> with_acc;
    static const std::map<std::wstring,std::wstring> without_acc;
    /// remove spanish accents from given word
    static std::wstring remove_accent_esp(const std::wstring &);
    /// set spanish accents on last syllabe of given word
    static std::wstring put_accent_esp(const std::wstring &);

  public:
    /// Constructor
    accents_es();

    /// Specific accentuation patterns for Spanish
    void fix_accentuation(std::set<std::wstring> &, const sufrule &) const;
  };

  ////////////////////////////////////////////////////////////////
  ///
  ///  Derived accents_module for Galician accentuation.
  ///
  ////////////////////////////////////////////////////////////////

  class accents_gl: public accents_module {

  private:
    /// regexps to check for useful spanish accentuation and syllabic patterns
    static const regexp last_vowel_put_acc;
    static const regexp last_vowel_not_acc;
    static const regexp any_vowel_acc;
    static const regexp any_closed_vowel_acc;
    static const regexp oxytone_without_acc;
    static const regexp oxytone_with_acc;
    static const regexp diacritic_acc;
    static const regexp past_subj_with_acc;
    static const regexp infinitive_r_accusative_allomorph;
    static const regexp present_s_accusative_allomorph;
   
    /// maps to store equivalences between accentued and not accentued chars.
    static const std::map<std::wstring,std::wstring> with_acc;
    static const std::map<std::wstring,std::wstring> without_acc;
    /// remove spanish accents from given word
    static std::wstring remove_accent_gl(const std::wstring &);
    /// set galician accents on last syllabe of given word
    static std::wstring put_accent_gl(const std::wstring &);
    /// fix for Galician accent pattern
    static void fix_accusative_allomorph_gl(std::set<std::wstring> &);

  public:
    /// Constructor
    accents_gl();

    /// Specific accentuation patterns for Galician
    void fix_accentuation(std::set<std::wstring> &, const sufrule &) const;
  };

} // namespace

#endif
