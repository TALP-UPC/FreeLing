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

#ifndef _NUMBERS_MOD
#define _NUMBERS_MOD

#include <map>

#include "freeling/regexp.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/automat.h"

namespace freeling {

#define RE_NUM L"^(\\d{1,3}(\\"+MACO_Thousand+L"\\d{3})*|\\d+)(\\"+MACO_Decimal+L"\\d+)?$"
#define RE_NUM_NEG L"^([\\-]?)(\\d{1,3}(\\"+MACO_Thousand+L"\\d{3})*|\\d+)(\\"+MACO_Decimal+L"\\d+)?$"
#define RE_CODE L"^.*[0-9].*$"

  // Auxiliary, kind of code (normal CODE, e.g. "X-23-12A";  ORDinal number, e.g. "4th")
#define CODE 1
#define ORD 2


  ////////////////////////////////////////////////////////////////
  /// Class to store status information
  ////////////////////////////////////////////////////////////////

  class numbers_status : public automat_status {
  public:
    /// partial value of partially build number expression
    long double bilion,milion,units;
    int block;
    int iscode;

    // These are used only in numbers_it. !! unify process with other languages !! 
    long double hundreds;   // this is additional.
    long double thousands;  // this is additional.
    long double floatUnits; // "e tre quarto". Count of how many "halfs", "quartrs" we have
  };


  ////////////////////////////////////////////////////////////////
  ///  The abstract class numbers_module generalizes nummeric
  /// expression recognizer for different languages.
  ////////////////////////////////////////////////////////////////

  class numbers_module : public automat<numbers_status> {

  protected:
    // configuration options
    std::wstring MACO_Decimal, MACO_Thousand;

    /// to map words into numerical values
    std::map<std::wstring,float> value;
    /// to map words into token codes 
    std::map<std::wstring,int> tok;
    /// to map value of power words (billion, million)
    std::map<int,long double> power;

    freeling::regexp RE_code;
    freeling::regexp RE_number;
    freeling::regexp RE_number_neg;

    // reset accumulators
    virtual void ResetActions(numbers_status *) const;
 
  public:
    /// Constructor
    numbers_module(const std::wstring &, const std::wstring &);
  };

  ////////////////////////////////////////////////////////////////
  ///   The derived class numbers_default implements a default 
  ///   number recognizer (only numbers in digits are recognized).
  ////////////////////////////////////////////////////////////////

  class numbers_default : public numbers_module {

  private: 
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, numbers_status *) const;
    void SetMultiwordAnalysis(sentence::iterator, int, const numbers_status *) const;

  public:
    /// Constructor
    numbers_default(const std::wstring &, const std::wstring &);
  };


  ////////////////////////////////////////////////////////////////
  ///   The derived class numbers_es implements a Spanish
  ///   number recognizer.
  ////////////////////////////////////////////////////////////////

  class numbers_es : public numbers_module {

  private:
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, numbers_status *) const;
    void SetMultiwordAnalysis(sentence::iterator, int, const numbers_status *) const;

  public:
    /// Constructor
    numbers_es(const std::wstring &, const std::wstring &);
  };


  ////////////////////////////////////////////////////////////////
  ///   The derived class numbers_ca implements a Catalan
  ///   number recognizer.
  ////////////////////////////////////////////////////////////////

  class numbers_ca : public numbers_module {

  private:
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, numbers_status *) const;
    void SetMultiwordAnalysis(sentence::iterator, int, const numbers_status *) const;

  public:
    /// Constructor
    numbers_ca(const std::wstring &, const std::wstring &);
  };

  ////////////////////////////////////////////////////////////////
  ///   The derived class numbers_gl implements a Galician
  ///   number recognizer.
  ////////////////////////////////////////////////////////////////

  class numbers_gl : public numbers_module {

  private:
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, numbers_status *) const;
    void SetMultiwordAnalysis(sentence::iterator, int, const numbers_status *) const;

  public:
    /// Constructor
    numbers_gl(const std::wstring &, const std::wstring &);
  };

  ////////////////////////////////////////////////////////////////
  ///   The derived class numbers_pt implements a Portuguese
  ///   number recognizer.
  ////////////////////////////////////////////////////////////////

  class numbers_pt : public numbers_module {

  private:
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, numbers_status *) const;
    void SetMultiwordAnalysis(sentence::iterator, int, const numbers_status *) const;

  public:
    /// Constructor
    numbers_pt(const std::wstring &, const std::wstring &);
  };


  ////////////////////////////////////////////////////////////////
  ///   The derived class numbers_it implements a Italian
  ///   number recognizer.
  ////////////////////////////////////////////////////////////////

  class numbers_it : public numbers_module {

  private:
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void ResetActions(numbers_status *) const;
    void StateActions(int, int, int, sentence::const_iterator, numbers_status *) const;
    void SetMultiwordAnalysis(sentence::iterator, int, const numbers_status *) const;

  public:
    /// Constructor
    numbers_it(const std::wstring &, const std::wstring &);
  };


  ////////////////////////////////////////////////////////////////
  ///   The derived class numbers_en implements an English
  ///   number recognizer.
  ////////////////////////////////////////////////////////////////

  class numbers_en : public numbers_module {

  private: 
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, numbers_status *) const;
    void SetMultiwordAnalysis(sentence::iterator, int, const numbers_status *) const;

  public:
    /// Constructor
    numbers_en(const std::wstring &, const std::wstring &);
  };

  ////////////////////////////////////////////////////////////////
  ///   The derived class numbers_ru implements an Russian
  ///   number recognizer.
  ////////////////////////////////////////////////////////////////

  class numbers_ru : public numbers_module 
  {
  private: 
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, numbers_status *) const;
    void SetMultiwordAnalysis(sentence::iterator, int, const numbers_status *) const;

  public:
    numbers_ru(const std::wstring &, const std::wstring &);
  };

  ////////////////////////////////////////////////////////////////
  ///   The derived class numbers_cs implements a Czeck
  ///   number recognizer.
  ////////////////////////////////////////////////////////////////

  class numbers_cs : public numbers_module 
  {
  private: 
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, numbers_status *) const;
    void SetMultiwordAnalysis(sentence::iterator, int, const numbers_status *) const;

  public:
    numbers_cs(const std::wstring &, const std::wstring &);
  };

} // namespace

#endif

