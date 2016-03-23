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

#ifndef _MACO_OPTIONS
#define _MACO_OPTIONS

#include <string>
#include "freeling/windll.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class maco_options implements a set of specific options
  /// of the morphological analyzer. Other modules do not have
  /// such a class because they deal with a reduced number of
  /// options
  ////////////////////////////////////////////////////////////////

  class WINDLL maco_options {

  public:
    // Language analyzed
    std::wstring Lang;

    /// Morphological analyzer modules configuration/data files.
    std::wstring LocutionsFile, QuantitiesFile, AffixFile, 
      CompoundFile, DictionaryFile, ProbabilityFile, NPdataFile, 
      PunctuationFile, UserMapFile;

    /// module-specific parameters for number recognition
    std::wstring Decimal, Thousand;
    /// module-specific parameters for probabilities
    double ProbabilityThreshold;
    /// module-specific parameters for dictionary
    bool InverseDict,RetokContractions;

    /// constructor
    maco_options(const std::wstring &); 

    /// Option setting methods provided to ease perl interface generation. 
    /// Since option data members are public and can be accessed directly
    /// from C++, the following methods are not necessary, but may become
    /// convenient sometimes.
    void set_data_files(const std::wstring &usr,
                        const std::wstring &pun, const std::wstring &dic,
                        const std::wstring &aff, const std::wstring &comp,
                        const std::wstring &loc, const std::wstring &nps,
                        const std::wstring &qty, const std::wstring &prb);

    void set_nummerical_points(const std::wstring &dec,const std::wstring &tho);
    void set_threshold(double);
    void set_inverse_dict(bool);
    void set_retok_contractions(bool);
  };

} // namespace

#endif

