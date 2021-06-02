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

#ifndef _ANALYZER
#define _ANALYZER

#include <iostream> 
#include <list>

#include "freeling.h"
#include "freeling/morfo/analyzer_config.h"

namespace freeling {


////////////////////////////////////////////////////////////////
/// 
///  Class analyzer is a meta class that just calls all modules in
///  FreeLing in the right order.
///  Its construction options allow to instantiate different kinds of
///  analysis pipelines, and or different languages.
///  Also, invocation options may be altered at each call, 
///  tuning the analysis to each particular sentence or document needs.
///  For a finer control, underlying modules should be called directly.
///
////////////////////////////////////////////////////////////////

class WINDLL analyzer {

 private:

   // we use pointers to the analyzers, so we
   // can create only those strictly necessary.
   tokenizer *tk;
   splitter *sp;
   maco *morfo;
   nec *neclass;
   senses *sens;
   ukb *dsb;
   POS_tagger *hmm;
   POS_tagger *relax;
   phonetics *phon;
   chart_parser *parser;
   dep_txala *deptxala;
   dep_treeler *deptreeler;
   dep_lstm *deplstm;
   srl_treeler *srltreeler;
   relaxcor *corfc;
   semgraph_extract *sge;

   // store configuration options used to create the analyzer
   analyzer_config initial_options;
   // invoke options to be used in subsequent calls (defaults to 
   // initial_options, but can be changed)
   analyzer_config::invoke_options current_invoke_options;

   // remember splitter session
   splitter::session_id sp_id;

   // remember token offsets in plain text input
   unsigned long offs;
   // number of sentences processed (used to generate sentence id's)
   unsigned long nsentence;
   // words pending of being splitted in InputMode==CORPUS
   std::list<word> tokens; 

   /// auxiliary for constructors: load modules defined in initial_options
   void create_analyzers();

   /// analyze further levels on a partially analyzed document
   template<class T> void do_analysis(T &doc, const analyzer_config::invoke_options &ivk) const;

   // tokenize and split text.
   void tokenize_split(const std::wstring &text, 
                       std::list<sentence> &ls, 
                       unsigned long &offs, 
                       std::list<word> &av, 
                       unsigned long &nsent, 
                       bool flush, 
                       splitter::session_id sp_ses,
		       const analyzer_config::invoke_options &ivk) const;

   static std::wistream& safe_getline(std::wistream& is, std::wstring& t);

 public:
   analyzer(const analyzer_config &cfg);
   analyzer(const analyzer_config::config_options &cfg);
   ~analyzer();

   const analyzer_config& get_initial_options() const;
   const analyzer_config::invoke_options& get_current_invoke_options() const;
   void set_current_invoke_options(const analyzer_config::invoke_options &opt);

   /// analyze further levels on a partially analyzed document
   void analyze(document &doc) const;
   void analyze(document &doc, const analyzer_config::invoke_options &ivk) const;
   /// analyze further levels on partially analyzed sentences
   void analyze(std::list<sentence> &ls) const;
   void analyze(std::list<sentence> &ls, const analyzer_config::invoke_options &ivk) const;
   /// analyze text as a whole document
   void analyze(const std::wstring &text, document &doc, bool parag=false) const;
   void analyze(const std::wstring &text, document &doc, const analyzer_config::invoke_options &ivk, bool parag=false) const;
   /// analyze text as a whole document, returning it (useful for python API)
   document analyze_as_document(const std::wstring &text, bool parag=false) const;
   document analyze_as_document(const std::wstring &text, const analyzer_config::invoke_options &ivk, bool parag=false) const;
  /// Analyze text as a partial document. Retain incomplete sentences in buffer   
   /// in case next call completes them (except if flush==true)
   void analyze(const std::wstring &text, std::list<sentence> &ls, bool flush=false);
   void analyze(const std::wstring &text, std::list<sentence> &ls, const analyzer_config::invoke_options &ivk, bool flush=false);
   /// Analyze text as a partial document. Retain incomplete sentences in buffer. Return analysis (useful for python API)
   std::list<sentence> analyze(const std::wstring &text, bool flush=false);
   std::list<sentence> analyze(const std::wstring &text, const analyzer_config::invoke_options &ivk, bool flush=false);

   // flush splitter buffer and analyze any pending text. 
   void flush_buffer(std::list<sentence> &ls);
   void flush_buffer(std::list<sentence> &ls, const analyzer_config::invoke_options &ivk);
   // reset tokenizer offset counter
   void reset_offset();
};



} // namespace

#endif

