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

#ifndef _ANALYZER_OPTIONS
#define _ANALYZER_OPTIONS

#include <string> 

#include <boost/program_options.hpp>
namespace po=boost::program_options;

#include "freeling/windll.h"
#include "freeling/morfo/analyzer_config.h"

namespace freeling {


class WINDLL analyzer_options {
                                 
  /// constructor
  analyzer_options();
  /// destructor
  ~analyzer_options();
  
  // get basic analyzer options description
  po::options_description command_line_opts();
  po::options_description config_file_opts();

  /// load provided options from a config file, update variables map  
  void parse_options(const std::wstring &cfgFile,
                     const po::options_description &cf_opts,
                     po::variables_map &vm);
  /// load provided options from a config file, return variables map
  po::variables_map parse_options(const std::wstring &cfgFile,
                                  const po::options_description &cf_opts);
  /// load basic options from a config file, return variables map
  po::variables_map parse_options(const std::wstring &cfgFile);
  /// load provided options from command line, update variables map  
  void parse_options(int ac, char *av[],
                     const po::options_description &cl_opts,
                     po::variables_map &vm);  
  /// load provided options from command line, return variables map  
  po::variables_map parse_options(int ac, char *av[],
                                       const po::options_description &cl_opts);
  /// load default options from command line   
  po::variables_map parse_options(int ac, char *av[]);
  /// load provided options from config file and command line  
  po::variables_map parse_options(const std::wstring &cfgFile, int ac, char *av[],
                                  const po::options_description &cf_opts,
                                  const po::options_description &cl_opts);
  /// load default options from config file and command line  
  po::variables_map parse_options(const std::wstring &cfgFile, int ac, char *av[]);
  
};

} // namespace


#endif
