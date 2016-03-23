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

#include <fstream>  

#include "freeling/output/output.h"
#include "freeling/output/output_freeling.h"
#include "freeling/output/output_conll.h"
#include "freeling/output/output_json.h"
#include "freeling/output/output_xml.h"
#include "freeling/output/output_naf.h"
#include "freeling/output/output_train.h"

#include "freeling/morfo/configfile.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

using namespace std;
using namespace freeling::io;

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"OUTPUT"
#define MOD_TRACECODE OUTPUT_TRACE

  ///////////////////////////////////////////////////////////////
  ///  Create the appropriate output handler (according to received options)
  //////////////////////////////////////////////////////////////

  output::output(const std::wstring &cfgFile) {

    enum sections {OUTPUT_TYPE};
    freeling::config_file cfg(true);  
    cfg.add_section(L"Type",OUTPUT_TYPE);

    if (not cfg.open(cfgFile))
      ERROR_CRASH(L"Error opening file "+cfgFile);

    wstring output_type= L"";

    // read config file to find out type of output handler
    wstring line; 
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) {
      case OUTPUT_TYPE: {
        output_type = freeling::util::lowercase(line);
        break;
      }
      default: break;
      }
    }
    cfg.close(); 

    // create apropriate kind of output module
    if (output_type==L"conll") 
      who = new output_conll(cfgFile);
    else if (output_type==L"freeling")  
      who = new output_freeling(cfgFile);
    else if (output_type==L"xml")  
      who = new output_xml(cfgFile);
    else if (output_type==L"naf")  
      who = new output_naf(cfgFile);
    else if (output_type==L"json")  
      who = new output_json(cfgFile);
    else if (output_type==L"train")  
      who = new output_train();
    else {
      ERROR_CRASH (L"Unknown or missing output handler type '"+output_type+L"' in file "+cfgFile);
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  Destructor. 
  ///////////////////////////////////////////////////////////////

  output::~output() { delete who; }

  ///////////////////////////////////////////////////////////////
  /// load tagset rules for PoS shortening and MSD descriptions
  ///////////////////////////////////////////////////////////////

  void output::load_tagset(const std::wstring &ftag) { who->load_tagset(ftag); }


  ///////////////////////////////////////////////////////////////
  /// set language 
  ///////////////////////////////////////////////////////////////

  void output::set_language(const std::wstring &lg) { who->set_language(lg); }


  ///////////////////////////////////////////////////////////////
  /// Print appropriate header for the ourput format (e.g. XML header or tag opening)
  ///////////////////////////////////////////////////////////////

  void output::PrintHeader(std::wostream &sout) const { who->PrintHeader(sout); }

  ///////////////////////////////////////////////////////////////
  /// print appropriate footer (e.g. close XML tags)
  ///////////////////////////////////////////////////////////////

  void output::PrintFooter(std::wostream &sout) const { who->PrintFooter(sout); }

  ///////////////////////////////////////////////////////////////
  /// print given sentences to sout in appropriate format (no headers)
  ///////////////////////////////////////////////////////////////

  void output::PrintResults (std::wostream &sout, const std::list<freeling::sentence> &ls) const { 
    who->PrintResults(sout,ls); 
  }
  std::wstring output::PrintResults (const std::list<freeling::sentence> &ls) const {
    return who->PrintResults(ls); 
  }

  ///////////////////////////////////////////////////////////////  
  /// print given document to sout in appropriate format, including headers.
  ///////////////////////////////////////////////////////////////

  void output::PrintResults (std::wostream &sout, const freeling::document &doc) {
    who->PrintResults(sout,doc); 
  }
  std::wstring output::PrintResults (const freeling::document &doc) const {
    return who->PrintResults(doc); 
  }

