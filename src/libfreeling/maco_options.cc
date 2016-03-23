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

#include "freeling/morfo/maco_options.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"MACO_OPTIONS"
#define MOD_TRACECODE OPTIONS_TRACE

  ///////////////////////////////////////////////////////////////
  ///  Create an options set for morpho analyzer. 
  ///  Initialize with default values.
  ///////////////////////////////////////////////////////////////

  maco_options::maco_options(const std::wstring &lg) {

    Lang=lg;

    UserMapFile=L""; LocutionsFile=L"";   QuantitiesFile=L"";
    AffixFile=L"";   ProbabilityFile=L""; DictionaryFile=L""; 
    NPdataFile=L"";  PunctuationFile=L""; CompoundFile=L"";
  
    Decimal=L",";  Thousand=L".";
    ProbabilityThreshold=0.001;
    InverseDict=false;
    RetokContractions=true;
  }

  void maco_options::set_data_files(const std::wstring &usr,
                                    const std::wstring &pun, const std::wstring &dic,
                                    const std::wstring &aff, const std::wstring &comp,
                                    const std::wstring &loc, const std::wstring &nps,
                                    const std::wstring &qty, const std::wstring &prb) {
    UserMapFile=usr;
    LocutionsFile=loc;    QuantitiesFile=qty;  AffixFile=aff; 
    ProbabilityFile=prb;  DictionaryFile=dic;  NPdataFile=nps;
    PunctuationFile=pun;  CompoundFile=comp;
  }


  void maco_options::set_nummerical_points(const std::wstring &dec,const std::wstring &tho) {
    Decimal=dec;  
    Thousand=tho;
  }
  void maco_options::set_threshold(double t) {
    ProbabilityThreshold=t;
  }

  void maco_options::set_inverse_dict(bool b) {
    InverseDict=b;
  }

  void maco_options::set_retok_contractions(bool b) {
    RetokContractions=b;
  }

} // namespace
