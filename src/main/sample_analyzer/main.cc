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


//------------------------------------------------------------------//
//
//                    IMPORTANT NOTICE
//
//  This file contains a simple main program to illustrate 
//  usage of FreeLing analyzers library.
//
//  This sample main program may be used straightforwardly as 
//  a basic front-end for the analyzers (e.g. to analyze corpora)
//
//  Neverthless, if you want embed the FreeLing libraries inside
//  a larger application, or you want to deal with other 
//  input/output formats (e.g. XML), the efficient and elegant 
//  way to do so is consider this file as a mere example, and call 
//  the library from your your own main code.
//
//------------------------------------------------------------------//

#include <sstream>
#include <iostream> 

#include <map>
#include <list>

// client/server communication
#include "socket.h"
/// config file/options handler for this particular sample application
#include "config.h"

/// headers to call freeling library
#include "freeling/morfo/analyzer.h"

/// functions to print results depending on configuration options
#include "freeling/output/output_freeling.h"
#include "freeling/output/output_train.h"
#include "freeling/output/output_conll.h"
#include "freeling/output/output_xml.h"
#include "freeling/output/output_json.h"
#include "freeling/output/output_naf.h"
#include "freeling/output/input_conll.h"
#include "freeling/output/input_freeling.h"

// Semaphores and stuff to handle children count in server mode
#ifdef WIN32
  #include <windows.h>
  #define getpid() GetCurrentProcessId()
  #define pid_t DWORD
#else
  #include <sys/wait.h>
  #include <semaphore.h>
  sem_t semaf;
#endif

// server performance statistics
#include "stats.h"

// Client/server socket
socket_CS *sock; 
bool ServerMode; 

using namespace std;
using namespace freeling;

//////// Auxiliary functions for server mode  //////////

#ifndef WIN32
//---- Capture signal informing that a child ended ---
void child_ended(int n) {
  int status;
  wait(&status);  
  sem_post(&semaf);
}

//----  Capture signal to shut server down cleanly
void terminate (int param) {
  wcerr<<L"SERVER.DISPATCHER: Signal received. Stopping"<<endl;
  exit(0);
}
#endif

//----  Initialize server socket, signals, etc.
void InitServer(config *cfg) {

  pid_t myPID=getpid();
  char host[256];
  if (gethostname(host,256)!=0) 
    strcpy(host, "localhost"); 

  wcerr<<endl;
  wcerr<<L"Launched server "<<myPID<<L" at port "<<cfg->Port<<endl;
  wcerr<<endl;
  wcerr<<L"You can now analyze text with the following command:"<<endl;
  wcerr<<L"  - From this computer: "<<endl;;
  wcerr<<L"      analyzer_client "<<cfg->Port<<L" <input.txt >output.txt"<<endl;
  wcerr<<L"      analyzer_client localhost:"<<cfg->Port<<L" <input.txt >output.txt"<<endl;
  wcerr<<L"  - From any other computer: "<<endl;
  wcerr<<L"      analyzer_client "<<util::string2wstring(host)<<L":"<<cfg->Port<<L" <input.txt >output.txt"<<endl;
  wcerr<<endl;
  wcerr<<L"Stop the server with: "<<endl;
  wcerr<<L"      analyze stop "<<myPID<<endl;
  wcerr<<endl;

  // open sockets to listen for clients
  sock = new socket_CS(cfg->Port,cfg->QueueSize);
  #ifndef WIN32
    // Capture terminating signals, to exit cleanly.
    signal(SIGTERM,terminate); 
    signal(SIGQUIT,terminate);   
    // Be signaled when children finish, to keep count of active workers.
    signal(SIGCHLD,child_ended); 
    // Init worker count sempahore
    sem_init(&semaf,0,cfg->MaxWorkers);
  #endif
}

//----  Wait for a client and fork a worker to attend its requests
int WaitClient() {
  int pid=0;
  #ifndef WIN32
    wcerr<<L"SERVER.DISPATCHER: Waiting for a free worker slot"<<endl;
    sem_wait(&semaf);
  #endif
  wcerr<<L"SERVER.DISPATCHER: Waiting connections"<<endl;
  sock->wait_client();
  
  // If we are a Linux server, fork a worker.
  // On windows, only serve one client at a time.
  #ifndef WIN32
    pid = fork();
    if (pid < 0) wcerr<<L"ERROR on fork"<<endl;
    
    if (pid!=0) {
      // we are the parent. Close client socket and wait for next client
      sock->set_parent();
      wcerr<<L"SERVER.DISPATCHER: Connection established. Forked worker "<<pid<<"."<<endl;
    }
    else { 
      // we are the child. Close request socket and prepare to get data from client.
      sock->set_child();
    }
  #endif

  return pid;
}

//----  Send ACK to the client, informing that we expect more 
//----  data to be able to send back an analysis.
void SendACK () {  
  sock->write_message("FL-SERVER-READY");  
}

//---- Read a line from input channel
bool CheckStatsCommands(const wstring &text, ServerStats &stats) {
  bool b=false;
  if (text==L"RESET_STATS") { 
    stats.ResetStats();
    SendACK();
    b=true;
  }
  else if (text==L"PRINT_STATS") {
    sock->write_message(util::wstring2string(stats.GetStats()));
    b=true;
  }
  return b;
}

//---- Clean up and end worker when client finishes.
void CloseWorker(ServerStats *stats) {
  wcerr<<L"SERVER.WORKER: client ended. Closing connection."<<endl;
  delete stats;
  sock->close_connection();
  #ifndef WIN32
    exit(0);
  #endif
}

/////// Functions to wrap I/O mode (server socket vs stdin/stdout) ////////

//---- Read a line from input channel
int ReadLine(wstring &text) {
  int n=0;
  if (ServerMode) {
    string s;
    n = sock->read_message(s);
    text = util::string2wstring(s);
  }
  else 
    if (getline(wcin,text)) n=1;

  return n;
}

//---- Output a string to output channel
void OutputString(const wstring &s) {
  if (ServerMode) 
    sock->write_message(util::wstring2string(s));
  else 
    wcout<<s;
}

//---- Output a list of tokens to output channel
void OutputTokens(const list<word> &av) {
 list<word>::const_iterator w;
 for (w=av.begin(); w!=av.end(); w++) 
   OutputString(w->get_form()+L"\n");
}

//---- Output analysis result to output channel
void OutputSentences(const io::output_handler &out, list<sentence> &ls) {

  if (ServerMode) {
    if (ls.empty()) {
      SendACK();
      return;
    }
    
    wostringstream sout;
    out.PrintResults(sout,ls);
    sock->write_message(util::wstring2string(sout.str()));
  }
  else {
    out.PrintResults(wcout,ls);
  }
}


//---- Output analysis result to output channel
void OutputDocument(const io::output_handler &out, const document &doc) {

  // not in server mode, print results to wcout
  if (not ServerMode) {
    out.PrintResults(wcout,doc);
    return;
  }

  // in server mode but nothing to output, just send ACK to client 
  if (doc.empty() or doc.front().empty()) {
    SendACK();
    return;
  }
    
  // server mode, something to output. Send it to client
  wostringstream sout;
  out.PrintResults(sout,doc);
  sock->write_message(util::wstring2string(sout.str()));  
}


//---- Create output handler for requested format
io::output_handler* create_output_handler(config *cfg) {

  io::output_handler *out;
  if (cfg->OutputFormat==OUT_TRAIN) {
    out = new io::output_train();
  }
  else if (cfg->OutputFormat==OUT_CONLL) {
    if (not cfg->OutputConllFile.empty()) 
      out = new io::output_conll(cfg->OutputConllFile);
    else {
       out = new io::output_conll();
       out->load_tagset(cfg->TAGSET_TagsetFile);
    }
  }
  else if (cfg->OutputFormat==OUT_XML) {
    out = new io::output_xml();
    out->load_tagset(cfg->TAGSET_TagsetFile);
  }
  else if (cfg->OutputFormat==OUT_JSON)  {
    out = new io::output_json();
    out->load_tagset(cfg->TAGSET_TagsetFile);
  }
  else if (cfg->OutputFormat==OUT_NAF)  {
    io::output_naf *onaf = new io::output_naf();
    onaf->load_tagset(cfg->TAGSET_TagsetFile);
    onaf->set_language(cfg->analyzer_config_options.Lang);

    onaf->ActivateLayer(L"text",true);
    onaf->ActivateLayer(L"terms",cfg->analyzer_invoke_options.OutputLevel>=MORFO);
    onaf->ActivateLayer(L"entities",cfg->analyzer_invoke_options.OutputLevel>=TAGGED);
    onaf->ActivateLayer(L"chunks",cfg->analyzer_invoke_options.OutputLevel==SHALLOW);
    onaf->ActivateLayer(L"constituency",cfg->analyzer_invoke_options.OutputLevel>=PARSED
                        and cfg->analyzer_invoke_options.DEP_which==TXALA);
    onaf->ActivateLayer(L"deps",cfg->analyzer_invoke_options.OutputLevel>=DEP);
    onaf->ActivateLayer(L"srl",cfg->analyzer_invoke_options.OutputLevel>=DEP 
                        and cfg->analyzer_invoke_options.DEP_which==TREELER);
    onaf->ActivateLayer(L"coreferences",cfg->analyzer_invoke_options.OutputLevel>=COREF);
    out = onaf;
  }
  else { //  default, cfg->OutputFormat==OUT_FREELING
    io::output_freeling *ofl = new io::output_freeling();
    ofl->output_senses(cfg->analyzer_invoke_options.SENSE_WSD_which!=NO_WSD);
    ofl->output_all_senses(cfg->analyzer_invoke_options.SENSE_WSD_which!=MFS);
    ofl->output_phonetics(cfg->analyzer_invoke_options.PHON_Phonetics);
    ofl->output_dep_tree(cfg->analyzer_invoke_options.OutputLevel>=DEP);
    ofl->output_corefs(cfg->analyzer_invoke_options.OutputLevel>=COREF);
    out = ofl;
  }

  return out;
}


//---- Create input handler for requested format
io::input_handler* create_input_handler(config *cfg) {

  io::input_handler *inp;
  if (cfg->InputFormat==INP_CONLL) {
    if (not cfg->InputConllFile.empty()) inp = new io::input_conll(cfg->InputConllFile);
    else inp = new io::input_conll();
  }
  else if (cfg->InputFormat==INP_FREELING)  inp = new io::input_freeling();
  else inp = NULL;
  
  return inp;
}

//---- Load options from config file and command line
config* load_config(int argc, char *argv[]) {
  
  config* cfg = new config(argc,argv);
  
  ServerMode = cfg->Server;
  
  // If server activated, make sure port was specified, and viceversa.
  if (ServerMode and cfg->Port==0) {
    wcerr <<L"Error - Server mode requires the use of option '--port' to specify a port number."<<endl;
    exit (1);    
  }
  else if (not ServerMode and cfg->Port>0) {
    wcerr <<L"Error - Ignoring unexpected server port number. Use '--server' option to activate server mode."<<endl;
    cfg->Port=0;
  }

  /// Check options are minimally consistent
  
  /// Check that required configuration can satisfy required requests
  /// 'analyzer' constructor will check most of them, so we only need to worry about lang ident
  if (cfg->analyzer_invoke_options.OutputLevel==IDENT and cfg->IDENT_identFile.empty()) {
    wcerr <<L"Error - No configuration file provided for language identifier."<<endl;
    exit (1);        
  }
  
  if (cfg->analyzer_invoke_options.OutputLevel>=COREF and 
      not cfg->analyzer_invoke_options.NEC_NEClassification and 
      not cfg->analyzer_config_options.NEC_NECFile.empty()) {
    cfg->analyzer_invoke_options.NEC_NEClassification = true;
    wcerr << L"NEC activated since coreference or semantic graph was requested."<<endl;
  }
  
  if (cfg->analyzer_invoke_options.OutputLevel>=COREF and 
      cfg->analyzer_invoke_options.SENSE_WSD_which!=UKB and 
      not cfg->analyzer_config_options.SENSE_ConfigFile.empty()) {
    cfg->analyzer_invoke_options.SENSE_WSD_which = UKB;
    wcerr << L"UKB sense disambiguation activated since coreference or semantic graph was requested."<<endl;
  }
  
  // ignore unneeded config files, to prevent analyer from loading modules we know we won't use  
  if (not cfg->analyzer_config_options.TOK_TokenizerFile.empty()
      and (cfg->analyzer_invoke_options.InputLevel>TOKEN 
           or cfg->analyzer_invoke_options.OutputLevel<TOKEN))  cfg->analyzer_config_options.TOK_TokenizerFile = L"";
  if (not cfg->analyzer_config_options.SPLIT_SplitterFile.empty()
      and (cfg->analyzer_invoke_options.InputLevel>SPLITTED
           or cfg->analyzer_invoke_options.OutputLevel<SPLITTED)) cfg->analyzer_config_options.SPLIT_SplitterFile = L"";
  if (not cfg->analyzer_config_options.TAGGER_HMMFile.empty() 
      and (cfg->analyzer_invoke_options.TAGGER_which!=HMM
           or (cfg->analyzer_invoke_options.InputLevel>TAGGED
               or cfg->analyzer_invoke_options.OutputLevel<TAGGED))) cfg->analyzer_config_options.TAGGER_HMMFile = L"";
  if (not cfg->analyzer_config_options.TAGGER_RelaxFile.empty() 
      and (cfg->analyzer_invoke_options.TAGGER_which!=RELAX
           or (cfg->analyzer_invoke_options.InputLevel>TAGGED
               or cfg->analyzer_invoke_options.OutputLevel<TAGGED))) cfg->analyzer_config_options.TAGGER_RelaxFile = L"";
  if (not cfg->analyzer_config_options.PARSER_GrammarFile.empty() 
      and (cfg->analyzer_invoke_options.InputLevel>SHALLOW
           or cfg->analyzer_invoke_options.OutputLevel<SHALLOW)
      and (cfg->analyzer_invoke_options.InputLevel>DEP
           or cfg->analyzer_invoke_options.OutputLevel<DEP)) cfg->analyzer_config_options.PARSER_GrammarFile = L"";
  if (not cfg->analyzer_config_options.DEP_TxalaFile.empty() 
      and ((cfg->analyzer_invoke_options.DEP_which!=TXALA and cfg->analyzer_invoke_options.OutputLevel<COREF)
           or (cfg->analyzer_invoke_options.InputLevel>DEP
               or cfg->analyzer_invoke_options.OutputLevel<PARSED))) cfg->analyzer_config_options.DEP_TxalaFile = L"";
  if (not cfg->analyzer_config_options.DEP_TreelerFile.empty() 
      and ((cfg->analyzer_invoke_options.DEP_which!=TREELER and cfg->analyzer_invoke_options.OutputLevel<COREF)
           or (cfg->analyzer_invoke_options.InputLevel>DEP
               or cfg->analyzer_invoke_options.OutputLevel<DEP))) cfg->analyzer_config_options.DEP_TreelerFile = L"";
  if (not cfg->analyzer_config_options.COREF_CorefFile.empty()
      and (cfg->analyzer_invoke_options.InputLevel>=COREF 
           or cfg->analyzer_invoke_options.OutputLevel<COREF)) cfg->analyzer_config_options.COREF_CorefFile = L"";

  if (not cfg->analyzer_config_options.SEMGRAPH_SemGraphFile.empty()
      and (cfg->analyzer_invoke_options.InputLevel>=SEMGRAPH 
           or cfg->analyzer_invoke_options.OutputLevel<SEMGRAPH)) 
    cfg->analyzer_config_options.SEMGRAPH_SemGraphFile = L"";
  
  if (not cfg->analyzer_config_options.PHON_PhoneticsFile.empty() and not cfg->analyzer_invoke_options.PHON_Phonetics)
    cfg->analyzer_config_options.PHON_PhoneticsFile = L"";
  if (not cfg->analyzer_config_options.NEC_NECFile.empty() and not cfg->analyzer_invoke_options.NEC_NEClassification)
    cfg->analyzer_config_options.NEC_NECFile = L"";
  if (not cfg->analyzer_config_options.SENSE_ConfigFile.empty() and cfg->analyzer_invoke_options.SENSE_WSD_which==NO_WSD)
    cfg->analyzer_config_options.SENSE_ConfigFile = L"";
  if (not cfg->analyzer_config_options.UKB_ConfigFile.empty() and cfg->analyzer_invoke_options.SENSE_WSD_which!=UKB)
    cfg->analyzer_config_options.UKB_ConfigFile = L"";
  
  if (cfg->OutputFormat == OUT_NAF and cfg->InputMode != MODE_DOC) {
    cfg->InputMode = MODE_DOC;
    wcerr << L"Input mode switched to 'doc' since NAF output format was requested." << endl;
  }
  
  if (cfg->analyzer_invoke_options.OutputLevel>=COREF and cfg->InputMode != MODE_DOC) {
    cfg->InputMode = MODE_DOC;
    wcerr << L"Input mode switched to 'doc' since coreference or semantic graph was requested."<<endl;
  }
  
  if (cfg->analyzer_invoke_options.InputLevel > cfg->analyzer_invoke_options.OutputLevel) {
    wcerr<<L"Error - Input analysis level can not be more complex than desired output analysis level."<<endl;
    exit(1);
  }
  if (cfg->analyzer_invoke_options.InputLevel == cfg->analyzer_invoke_options.OutputLevel) 
    wcerr<<L"Warning - Input and output analysis levels are the same."<<endl;

  if (cfg->InputFormat==INP_FREELING and 
      (cfg->analyzer_invoke_options.InputLevel < SPLITTED or
       cfg->analyzer_invoke_options.InputLevel > SENSES)) {
    wcerr<<L"Error - 'freeling' input format only accepts input analysis levels 'splitted', 'morfo', 'tagged', and 'senses'."<<endl;
    exit(1);
  }
  if (cfg->InputFormat==INP_CONLL and cfg->analyzer_invoke_options.InputLevel < TAGGED) {
    wcerr<<L"Error - 'conll' input format only accepts input analysis levels >= tagged."<<endl;
    exit(1);
  }
  if (cfg->InputFormat==INP_TEXT and cfg->analyzer_invoke_options.InputLevel!=TEXT) {
    wcerr<<L"Error - 'text' input format only accepts input analysis level 'text'."<<endl;
    exit(1);
  }
  
  return cfg; 
}

//---------------------------------------------
// Process all input as a single document
//---------------------------------------------

void load_document(wstring &text, ServerStats &stats) {
  
  // read whole document text
  wstring line;
  while (ReadLine(line) and not (ServerMode and line==L"FLUSH_BUFFER")) {
    if (ServerMode) {
      // if it is a stats command from the client, process it and go for next line.
      if (CheckStatsCommands(line,stats)) continue;
      // tell the client to send more text
      SendACK();
    }
    // accumulate the whole document before processing
    text = text + line + L"\n";
  }
}


//---------------------------------------------
// Process all input incrementally, line by line, 
// outputting results as soon as they are available
//---------------------------------------------

void process_text_incremental(analyzer &anlz, ServerStats &stats, const io::output_handler &out, bool flush) {

  // read and analyze text incrementally
  list<sentence> ls;
  wstring line;  
  while (ReadLine(line)) {
    if (ServerMode) {
      // if it is a stats command from the client, process it and go for next line.
      if (CheckStatsCommands(line,stats)) continue;

      // if the client requested a flush, do it and send results.
      if (line==L"FLUSH_BUFFER") {
        anlz.flush_buffer(ls);
        OutputSentences(out,ls);
        anlz.reset_offset();
        continue;
      }
    }
    
    // analyze text and output results
    anlz.analyze(line,ls,flush);

    OutputSentences(out,ls);
  }

  // output pending text, if any.
  anlz.flush_buffer(ls);
  OutputSentences(out,ls);
}

//---------------------------------------------
// Process all input incrementally, line by line, 
// outputting results as soon as they are available
//---------------------------------------------

void process_columns_incremental(const analyzer &anlz, ServerStats &stats, const io::input_handler &inp, const io::output_handler &out) {

  // read and analyze text incrementally. Text is analyzed in some column format
  list<sentence> ls;
  wstring text, line;  
  while (ReadLine(line)) {
    if (ServerMode) {
      // if it is a stats command from the client, process it and go for next line.
      if (CheckStatsCommands(line,stats)) continue;

      // if the client requested a flush request, treat it as a sentence ending.
      if (line==L"FLUSH_BUFFER") line = L"";
    }

    if (not line.empty()) 
      // normal line, add to sentence
      text = text + line + L"\n";
    
    else {
      // end-of-sentence reached, add empty line.
      text = text + L"\n";
 
      // convert columns to freeling sentence
      inp.input_sentences(text,ls);
      // analyze
      anlz.analyze(ls);
      // output results
      OutputSentences(out,ls);

      // clear for next sentence.
      ls.clear();
      text = L"";
    }

  }
}


//---------------------------------------------
// Main program
//---------------------------------------------

int main (int argc, char **argv) {
   
  // read configuration file and command-line options
  config *cfg = load_config(argc,argv);
 
  /// set the locale to UTF to properly handle special characters.
  util::init_locale(cfg->Locale);

  // create input/output handlers appropriate for requested type of input/output.
  io::output_handler *out = create_output_handler(cfg);
  io::input_handler *inp = create_input_handler(cfg);

  // create lang ident or analyzer, depending on requested output
  lang_ident *ident=NULL;
  analyzer *anlz=NULL;
  if (cfg->analyzer_invoke_options.OutputLevel == IDENT) 
    ident = new lang_ident(cfg->IDENT_identFile);
  else {
    anlz = new analyzer(cfg->analyzer_config_options);
    anlz->set_current_invoke_options(cfg->analyzer_invoke_options);
  }

  if (ServerMode) {
    wcerr<<L"SERVER: Analyzers loaded."<<endl;
    InitServer(cfg);
  }
  ServerStats *stats=NULL;

  bool stop=false;    /// The server version will never stop. 
  while (not stop) {  /// The standalone version will stop after one iteration.

    if (ServerMode) {
      int n=WaitClient(); // Wait for a client and fork a worker to attend it.
      if (n!=0) continue; // If we are the dispatcher, go to wait for a new client.
      stats = new ServerStats();  // If we are the worker, get ready.
    }
    
    // ---------------------------------------------------------------
    // if language identification requested, do not enter analysis loop, 
    // just identify language for each line.
    if (cfg->analyzer_invoke_options.OutputLevel == IDENT) {
      wstring text;
      while (ReadLine(text)) {
        // call the analyzer to identify language
        OutputString (ident->identify_language(text)+L"\n");
      }
    }
 
    // ---------------------------------------------------------------
    // Process text documentwise
    else if (cfg->InputMode == MODE_DOC) {
      // load whole document in a string
      wstring text;
      load_document(text, *stats);
      
      document doc; 
      // if input is plain text, analyze directly
      if (cfg->InputFormat == INP_TEXT) 
        anlz->analyze(text,doc,cfg->AlwaysFlush);

      // if input is partially analyzed, load it into a document, and analyze
      else {  
        inp->input_document(text,doc);
        anlz->analyze(doc);
      }
      
      // output results
      OutputDocument(*out,doc);
    }

    // ---------------------------------------------------------------
    // proces text line by line, produce output incrementally
    else if (cfg->InputMode == MODE_CORPUS) {
      if (cfg->InputFormat == INP_TEXT) 
        process_text_incremental(*anlz,*stats,*out,cfg->AlwaysFlush);
      else 
        process_columns_incremental(*anlz,*stats,*inp,*out);    
    }
    

    // if we are a forked server attending a client, and the client is done, we exit.
    if (ServerMode) CloseWorker(stats);
    // if not server version, stop when document is processed
    else stop=true;   
  }
  
  // clean up and exit
  delete cfg;
}

