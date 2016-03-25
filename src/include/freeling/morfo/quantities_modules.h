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

#ifndef _QUANTITIES_MOD
#define _QUANTITIES_MOD

#include <map>

#include "freeling/morfo/language.h"
#include "freeling/morfo/automat.h"
#include "freeling/morfo/locutions.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Class to store status information
  ////////////////////////////////////////////////////////////////

  class quantities_status : public automat_status {
  public:
    /// values for ratios
    std::wstring value1, value2;
    std::wstring unitType;
    /// auxiliary for storing standarized unit description
    std::wstring unitCode;
  };

  ////////////////////////////////////////////////////////////////
  ///  The abstract class quantities_module generalizes a
  /// percentage, ratios, and currency expression recognizer 
  /// for different languages.
  ////////////////////////////////////////////////////////////////

  class quantities_module: public automat<quantities_status> {

  private:
    virtual void ResetActions(quantities_status *) const ;
    virtual void SetMultiwordAnalysis(sentence::iterator, int, const quantities_status *) const;

  protected:
    virtual void readConfig(const std::wstring &);
    /// translate particular strings to token codes
    std::map<std::wstring,int> tok;
    /// translate fraction strings to their nummerical values
    std::map<std::wstring,long double> fract;
    /// list of measure units and their lexical realizations
    std::map<std::wstring,std::wstring> units;
    /// sub module of locutions class to recognize complex measure units (oz_per_square_inch)
    locutions measures;
    /// special "magnitude" type for currencies.
    std::wstring currency_key;

  public:
    /// Constructor
    quantities_module();
  };


  ////////////////////////////////////////////////////////////////
  ///   The derived class quantities_default implements a default
  ///   quantities recognizer (only percentages are recognized).
  ////////////////////////////////////////////////////////////////

  class quantities_default: public quantities_module {

  private:
    int ComputeToken(int,sentence::iterator&,sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, quantities_status *) const;

  public:
    /// Constructor
    quantities_default(const std::wstring &);
  };


  ////////////////////////////////////////////////////////////////
  ///   The derived class quantities_es implements a Spanish
  ///  quantities recognizer.
  ////////////////////////////////////////////////////////////////

  class quantities_es: public quantities_module {

  private:
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, quantities_status *) const;

  public:
    /// Constructor
    quantities_es(const std::wstring &); 
  };

  ////////////////////////////////////////////////////////////////
  ///   The derived class quantities_ca implements a Catalan
  ///  quantities recognizer.
  ////////////////////////////////////////////////////////////////

  class quantities_ca: public quantities_module {

  private:
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, quantities_status *) const;

  public:
    /// Constructor
    quantities_ca(const std::wstring &); 
  };


  ////////////////////////////////////////////////////////////////
  ///   The derived class quantities_gl implements a Galician
  ///  quantities recognizer.
  ////////////////////////////////////////////////////////////////

  class quantities_gl: public quantities_module {

  private:
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, quantities_status *) const;

  public:
    /// Constructor
    quantities_gl(const std::wstring &); 
  };

  ////////////////////////////////////////////////////////////////
  ///   The derived class quantities_pt implements a Portuguese
  ///  quantities recognizer.
  ////////////////////////////////////////////////////////////////

  class quantities_pt: public quantities_module {

  private:
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, quantities_status *) const;

  public:
    /// Constructor
    quantities_pt(const std::wstring &); 
  };

  ////////////////////////////////////////////////////////////////
  ///   The derived class quantities_en implements an English
  ///  quantities recognizer.
  ////////////////////////////////////////////////////////////////

  class quantities_en: public quantities_module {

  private:
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, quantities_status *) const;

  public:
    /// Constructor
    quantities_en(const std::wstring &); 
  };

  ////////////////////////////////////////////////////////////////
  ///   The derived class quantities_ru implements a Russian
  ///  quantities recognizer.
  ////////////////////////////////////////////////////////////////

  class quantities_ru: public quantities_module {

  private:
    int ComputeToken(int,sentence::iterator&, sentence &) const;
    void StateActions(int, int, int, sentence::const_iterator, quantities_status *) const;

  public:
    /// Constructor
    quantities_ru(const std::wstring &); 
  };

} // namespace

#endif

