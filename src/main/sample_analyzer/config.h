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

#ifndef _CONFIG
#define _CONFIG

#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/program_options.hpp>

#include "freeling/version.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/analyzer_config.h"

#define MOD_TRACENAME L"CONFIG_OPTIONS"

// Default server parameters
#define DEFAULT_MAX_WORKERS 5   // maximum number of workers simultaneously active.
#define DEFAULT_QUEUE_SIZE 32   // maximum number of waiting clients

// codes for InputMode
typedef enum {MODE_CORPUS,MODE_DOC} InputModes;
// codes for OutputFormat
typedef enum {OUT_FREELING,OUT_TRAIN,OUT_CONLL,OUT_XML,OUT_JSON,OUT_NAF} OutputFormats;
// codes for InputFormat
typedef enum {INP_TEXT, INP_FREELING, INP_CONLL} InputFormats;

std::wistream& operator>>(std::wistream& in, InputModes& val) {
  std::wstring token;
  in >> token;
  if (token==L"corpus") val = MODE_CORPUS;
  else if (token==L"doc") val = MODE_DOC;
  else {
     val = MODE_CORPUS;
     WARNING(L"Unknown or invalid input mode: "<<token<<L". Using default.");
  }
  return in;
}
 
std::wistream& operator>>(std::wistream& in, OutputFormats& val) {
  std::wstring token;
  in >> token;
  if (token==L"freeling") val = OUT_FREELING;
  else if (token==L"conll") val = OUT_CONLL;
  else if (token==L"train") val = OUT_TRAIN;
  else if (token==L"xml") val = OUT_XML;
  else if (token==L"json") val = OUT_JSON ;
  else if (token==L"naf") val = OUT_NAF;
  else {
     val = OUT_FREELING;
     WARNING(L"Unknown or invalid output format: "<<token<<L". Using default.");
  }
  return in;
}
 
std::wistream& operator>>(std::wistream& in, InputFormats& val) {
  std::wstring token;
  in >> token;
  if (token==L"text") val = INP_TEXT;
  else if (token==L"freeling") val = INP_FREELING;
  else if (token==L"conll") val = INP_CONLL;
  else {
    val = INP_FREELING;
    WARNING(L"Unknown or invalid input format: "<<token<<L". Using default.");
  }
  return in;
}
 

namespace po = boost::program_options;
using namespace freeling;


////////////////////////////////////////////////////////////////
///  Class config implements a set of specific options
/// for the NLP analyzer, providing a C++ wrapper to 
/// libcfg+ library.
////////////////////////////////////////////////////////////////

class config : public analyzer_config {

public:
  std::wstring ConfigFile;

  /// Server mode on/off
  bool Server;
  /// port number for server mode  
  int Port; 
  /// Maximum number of workers to fork (i.e. number of simultaneously atended clients)
  int MaxWorkers;
  /// Size of socket queue (number of clients waiting to be atended without being rejected)
  int QueueSize;

  /// Locale of text to process
  std::wstring Locale;

  /// Configuration file for language identifier
  bool LangIdent = false;
  std::wstring IDENT_identFile;

  /// Default mode used to process input: 
  ///   DOC: load a document, then process it. 
  ///   CORPUS: infinite sentence-by-sentnce processing
  InputModes InputMode  = MODE_CORPUS;
  /// Selected default input and output format
  OutputFormats OutputFormat = OUT_FREELING;
  InputFormats InputFormat = INP_TEXT;
  std::wstring InputConllFile;
  std::wstring OutputConllFile;

  /// whether splitter buffer must be flushed at each line
  bool AlwaysFlush = false;

  /// Tagset to use for shortening tags in output
  std::wstring TAGSET_TagsetFile;

  /// constructor
  config(int ac, char **av) {

    /// auxiliary to read hex number
    std::wstring tracemod;  
    Port=0;

    command_line_options().add_options()
      ("help,h", "Help about command-line options.")
      ("help-cf", "Help about configuration file options.")
#ifndef WIN32
      ("version,v", "Print installed FreeLing version.")
#endif
      ("fcfg,f", po::wvalue<std::wstring>(&ConfigFile)->default_value(L"",""), "Configuration file to use")
      ("locale",po::wvalue<std::wstring>(&Locale),"locale encoding of input text (\"default\"=en_US.UTF-8, \"system\"=current system locale, [other]=any valid locale string installed in the system (e.g. ca_ES.UTF-8,it_IT.UTF-8,...)")
      ("port,p",po::wvalue<int>(&Port),"Port where server is to be started")
      ("workers,w",po::wvalue<int>(&MaxWorkers)->default_value(DEFAULT_MAX_WORKERS),"Maximum number of workers to fork in server mode")
      ("queue,q",po::wvalue<int>(&QueueSize)->default_value(DEFAULT_QUEUE_SIZE),"Maximum number of waiting clients.")
      ("server","Activate server mode (default: off)")
      ("ident","Produce language identification as output")
      ("flush","Consider each newline as a sentence end")
      ("noflush","Do not consider each newline as a sentence end")
      ("mode",po::wvalue<InputModes>(&InputMode),"Input mode (doc,corpus)")
      ("input",po::wvalue<InputFormats>(&InputFormat),"Input format (text,freeling,conll)")
      ("output",po::wvalue<OutputFormats>(&OutputFormat),"Output format (freeling,conll,train,xml,json,naf)")
      ("iconll",po::wvalue<std::wstring>(&InputConllFile),"CoNLL input definition file")
      ("oconll",po::wvalue<std::wstring>(&OutputConllFile),"CoNLL output definition file")
      ("fidn,I",po::wvalue<std::wstring>(&IDENT_identFile),"Language identifier file")
      ("ftags",po::wvalue<std::wstring>(&TAGSET_TagsetFile),"Tagset description file")
      ("tlevel,l",po::wvalue<int>(&traces::TraceLevel),"Debug traces verbosity")
      ("tmod,m",po::wvalue<std::wstring>(&tracemod),"Mask indicating which modules to trace")
      ;
 
    config_file_options().add_options()
      ("Locale",po::wvalue<std::wstring>(&Locale)->default_value(L"default","default"),"locale encoding of input text (\"default\"=en_US.UTF-8, \"system\"=current system locale, [other]=any valid locale string installed in the system (e.g. ca_ES.UTF-8,it_IT.UTF-8,...)")
      ("ServerMode",po::wvalue<bool>(&Server)->default_value(false),"Activate server mode (default: off)")
      ("ServerPort",po::wvalue<int>(&Port),"Port where server is to be started")
      ("ServerMaxWorkers",po::wvalue<int>(&MaxWorkers)->default_value(DEFAULT_MAX_WORKERS),"Maximum number of workers to fork in server mode")
      ("ServerQueueSize",po::wvalue<int>(&QueueSize)->default_value(DEFAULT_QUEUE_SIZE),"Maximum number of waiting requests in server mode")
      ("LangIdent",po::wvalue<bool>(&LangIdent)->default_value(false),"Produce language identification as output")
      ("AlwaysFlush",po::wvalue<bool>(&AlwaysFlush)->default_value(false),"Consider each newline as a sentence end")
      ("InputMode",po::wvalue<InputModes>(&InputMode)->default_value(MODE_CORPUS),"Input mode (corpus,doc)")
      ("OutputFormat",po::wvalue<OutputFormats>(&OutputFormat)->default_value(OUT_FREELING),"Output format (freeling,conll,train,xml,json,naf)")
      ("InputFormat",po::wvalue<InputFormats>(&InputFormat)->default_value(INP_TEXT),"Input format (text,freeling,conll)")
      ("InputConllConfig",po::wvalue<std::wstring>(&InputConllFile),"CoNLL input definition file")
      ("OutputConllConfig",po::wvalue<std::wstring>(&OutputConllFile),"CoNLL output definition file")
      ("LangIdentFile",po::wvalue<std::wstring>(&IDENT_identFile),"Language identifier file")
      ("TagsetFile",po::wvalue<std::wstring>(&TAGSET_TagsetFile),"Tagset description file")
      ("TraceLevel",po::wvalue<int>(&traces::TraceLevel)->default_value(0),"Debug traces verbosity")
      ("TraceModule",po::wvalue<std::wstring>(&tracemod)->default_value(L"0x0","0x0"),"Mask indicating which modules to trace")
      ;

    // variable map for option parser
    po::variables_map vm;  
    
    try {
      po::store(po::parse_command_line(ac, av, command_line_options()), vm);
      po::notify(vm);    
    } 
    catch (std::exception &e) {
      std::cerr <<"Error while parsing command line: "<<e.what() << std::endl;
      exit(1);
    }

    // Version required
    if (vm.count("version")) {
      #ifndef FREELING_VERSION
        std::cerr<<"Option '--version' not available in this executable."<<std::endl;
      #else
        std::cerr<<FREELING_VERSION<<std::endl;
      #endif
      exit(0); // return to system
    }

    // Help screen required
    if (vm.count("help")) {
      std::cerr<<command_line_options()<<std::endl;
      exit(0); // return to system
    }

    // Help screen required
    if (vm.count("help-cf")) {
      std::cerr<<config_file_options()<<std::endl;
      exit(0); // return to system
    }

    // Handle boolean options expressed with --myopt or --nomyopt in command line
    analyzer_config::SetBooleanOptionCL(vm.count("server"),!vm.count("server"),Server,"server");
    analyzer_config::SetBooleanOptionCL(vm.count("ident"),!vm.count("ident"),LangIdent,"ident");
    analyzer_config::SetBooleanOptionCL(vm.count("flush"),vm.count("noflush"),AlwaysFlush,"flush");
    
    // Unless lang ident, load config file.
    if (not LangIdent) {
      if (ConfigFile.empty()) {
        std::cerr<<"Configuration file not specified. Please use option -f to provide a configuration file." << std::endl;
        exit(1);
      }

      // parse ConfigFile for more options
      parse_options(ConfigFile, vm);
    }
    
    /// convert hex string to actual value
    std::wstringstream s;
    s << std::hex << tracemod;
    s >> traces::TraceModule;

    // check options involving Filenames for environment vars expansion.
    IDENT_identFile = util::expand_filename(IDENT_identFile);
    TAGSET_TagsetFile = util::expand_filename(TAGSET_TagsetFile);
    
    // conll format configuration files
    OutputConllFile = util::expand_filename(OutputConllFile);
    if (OutputConllFile!=L"" and OutputFormat!=OUT_CONLL)
      WARNING(L"Output CoNLL format configuration ignored, since selected output format is not 'conll'.");

      // conll format configuration files
    InputConllFile = util::expand_filename(InputConllFile);
    if (InputConllFile!=L"" and InputFormat!=INP_CONLL) 
      WARNING(L"Input CoNLL format configuration ignored, since selected input format is not 'conll'.");

  }
  

};


#endif

