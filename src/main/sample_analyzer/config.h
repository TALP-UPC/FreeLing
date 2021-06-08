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
#include "freeling/morfo/analyzer_options.h"
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

    Port=0;
    
    po::options_description cl_opts = analyzer_options::command_line_opts();
    cl_opts.add_options()
      ("help,h", "Help about command-line options.")
      ("help-cf", "Help about configuration file options.")
#ifndef WIN32
      ("version,v", "Print installed FreeLing version.")
#endif
      ("fcfg,f", po::wvalue<std::wstring>()->default_value(L"",""), "Configuration file to use")
      ("locale",po::wvalue<std::wstring>(),"locale encoding of input text (\"default\"=en_US.UTF-8, \"system\"=current system locale, [other]=any valid locale string installed in the system (e.g. ca_ES.UTF-8,it_IT.UTF-8,...)")
      ("port,p",po::wvalue<int>(),"Port where server is to be started")
      ("workers,w",po::wvalue<int>()->default_value(DEFAULT_MAX_WORKERS),"Maximum number of workers to fork in server mode")
      ("queue,q",po::wvalue<int>()->default_value(DEFAULT_QUEUE_SIZE),"Maximum number of waiting clients.")
      ("server","Activate server mode (default: off)")
      ("ident","Produce language identification as output")
      ("flush","Consider each newline as a sentence end")
      ("noflush","Do not consider each newline as a sentence end")
      ("mode",po::wvalue<InputModes>(),"Input mode (doc,corpus)")
      ("input",po::wvalue<InputFormats>(),"Input format (text,freeling,conll)")
      ("output",po::wvalue<OutputFormats>(),"Output format (freeling,conll,train,xml,json,naf)")
      ("iconll",po::wvalue<std::wstring>(),"CoNLL input definition file")
      ("oconll",po::wvalue<std::wstring>(),"CoNLL output definition file")
      ("fidn,I",po::wvalue<std::wstring>(),"Language identifier file")
      ("ftags",po::wvalue<std::wstring>(),"Tagset description file")
      ("tlevel,l",po::wvalue<int>(),"Debug traces verbosity")
      ("tmod,m",po::wvalue<std::wstring>(),"Mask indicating which modules to trace")
      ;
 
    po::options_description cf_opts = analyzer_options::config_file_opts();
    cf_opts.add_options()
      ("Locale",po::wvalue<std::wstring>()->default_value(L"default","default"),"locale encoding of input text (\"default\"=en_US.UTF-8, \"system\"=current system locale, [other]=any valid locale string installed in the system (e.g. ca_ES.UTF-8,it_IT.UTF-8,...)")
      ("ServerMode",po::wvalue<bool>()->default_value(false),"Activate server mode (default: off)")
      ("ServerPort",po::wvalue<int>(),"Port where server is to be started")
      ("ServerMaxWorkers",po::wvalue<int>()->default_value(DEFAULT_MAX_WORKERS),"Maximum number of workers to fork in server mode")
      ("ServerQueueSize",po::wvalue<int>()->default_value(DEFAULT_QUEUE_SIZE),"Maximum number of waiting requests in server mode")
      ("LangIdent",po::wvalue<bool>()->default_value(false),"Produce language identification as output")
      ("AlwaysFlush",po::wvalue<bool>()->default_value(false),"Consider each newline as a sentence end")
      ("InputMode",po::wvalue<InputModes>()->default_value(MODE_CORPUS),"Input mode (corpus,doc)")
      ("OutputFormat",po::wvalue<OutputFormats>()->default_value(OUT_FREELING),"Output format (freeling,conll,train,xml,json,naf)")
      ("InputFormat",po::wvalue<InputFormats>()->default_value(INP_TEXT),"Input format (text,freeling,conll)")
      ("InputConllConfig",po::wvalue<std::wstring>(),"CoNLL input definition file")
      ("OutputConllConfig",po::wvalue<std::wstring>(),"CoNLL output definition file")
      ("LangIdentFile",po::wvalue<std::wstring>(),"Language identifier file")
      ("TagsetFile",po::wvalue<std::wstring>(),"Tagset description file")
      ("TraceLevel",po::wvalue<int>()->default_value(0),"Debug traces verbosity")
      ("TraceModule",po::wvalue<std::wstring>()->default_value(L"0x0","0x0"),"Mask indicating which modules to trace")
      ;

    // parse options in command line
    po::variables_map vm = analyzer_options::parse_options(ac, av, cl_opts);

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
      std::cerr<<cl_opts<<std::endl;
      exit(0); // return to system
    }

    // Help screen required
    if (vm.count("help-cf")) {
      std::cerr<<cf_opts<<std::endl;
      exit(0); // return to system
    }


    // Unless lang ident, load config file.
    LangIdent = (vm.count("ident") != 0);
    if (not LangIdent) {

      auto f = vm.find("fcfg");
      if (f == vm.end()) {
        std::cerr<<"Configuration file not specified. Please use option -f to provide a configuration file." << std::endl;
        exit(1);
      }
      else
        ConfigFile = f->second.as<std::wstring>();

      // parse ConfigFile for more options
      analyzer_options::parse_options(ConfigFile, cf_opts, vm);      
    }

    extract_options(vm);   
  }



  void extract_options(const po::variables_map & vm) {
    // extract values for options in FreeLing parent class
    analyzer_config::extract_options(vm);

    /// auxiliary to read hex number
    std::wstring tracemod;
  
    // check config file options
    for (auto v : vm) {
      std::string name = v.first;

      if (name == "Locale") Locale = v.second.as<std::wstring>();
      else if (name == "ServerMode") Server = v.second.as<bool>();
      else if (name == "ServerPort") Port = v.second.as<int>();
      else if (name == "ServerMaxWorkers") MaxWorkers = v.second.as<int>();
      else if (name == "ServerQueueSize") QueueSize = v.second.as<int>();
      else if (name == "LangIdent") LangIdent = v.second.as<bool>();
      else if (name == "AlwaysFlush") AlwaysFlush = v.second.as<bool>();
      else if (name == "InputMode") InputMode = v.second.as<InputModes>();
      else if (name == "OutputFormat") OutputFormat = v.second.as<OutputFormats>();
      else if (name == "InputFormat") InputFormat = v.second.as<InputFormats>();
      else if (name == "InputConllConfig") InputConllFile = v.second.as<std::wstring>();
      else if (name == "OutputConllConfig") InputConllFile = v.second.as<std::wstring>();
      else if (name == "LangIdentFile") IDENT_identFile = v.second.as<std::wstring>();
      else if (name == "TagsetFile") TAGSET_TagsetFile = v.second.as<std::wstring>();
      else if (name == "TraceLevel") traces::TraceLevel = v.second.as<int>();
      else if (name == "TraceModule") tracemod = v.second.as<std::wstring>();
    }      
    
    // check command line options, overwritting previous values if needed
    for (auto v : vm) {
      std::string name = v.first;

      if (name == "locale") Locale = v.second.as<std::wstring>();
      else if (name == "port") Port = v.second.as<int>();
      else if (name == "workers") MaxWorkers = v.second.as<int>();
      else if (name == "queue") QueueSize = v.second.as<int>();
      else if (name == "mode") InputMode = v.second.as<InputModes>();
      else if (name == "output") OutputFormat = v.second.as<OutputFormats>();
      else if (name == "input") InputFormat = v.second.as<InputFormats>();
      else if (name == "iconll") InputConllFile = v.second.as<std::wstring>();
      else if (name == "oconll") InputConllFile = v.second.as<std::wstring>();
      else if (name == "fidn") IDENT_identFile = v.second.as<std::wstring>();
      else if (name == "ftags") TAGSET_TagsetFile = v.second.as<std::wstring>();
      else if (name == "tlevel") traces::TraceLevel = v.second.as<int>();
      else if (name == "tmod") tracemod = v.second.as<std::wstring>();
    }      

    // Handle boolean options expressed with --myopt or --nomyopt in command line
    analyzer_config::SetBooleanOptionCL(vm.count("server"),!vm.count("server"),Server,"server");
    analyzer_config::SetBooleanOptionCL(vm.count("ident"),!vm.count("ident"),LangIdent,"ident");
    analyzer_config::SetBooleanOptionCL(vm.count("flush"),vm.count("noflush"),AlwaysFlush,"flush");       
    /// convert hex string to actual value
    std::wstringstream s;
    s << std::hex << tracemod;
    s >> traces::TraceModule;

    // check options involving Filenames for environment vars expansion.
    TAGSET_TagsetFile = util::expand_filename(TAGSET_TagsetFile);

    IDENT_identFile = util::expand_filename(IDENT_identFile);
    if (IDENT_identFile!=L"" and not LangIdent)
      WARNING(L"Language identifier configuration file ignored, since language identification output was not requested.");
    
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

