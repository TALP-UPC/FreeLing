//////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public License
//    (GNU AGPL) as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License 
//    along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Muntsa Padro (mpadro@lsi.upc.edu)
//             TALP Research Center
//             despatx Omega.S107 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////
 
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

#include "freeling/morfo/configfile.h"
#include "freeling/morfo/idioma.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"IDIOMA"
#define MOD_TRACECODE LANGIDENT_TRACE

  // /////////////////////// Class idioma //////////////////////////
  // 
  //   Class that implements a visible Markov's that calculates 
  //   the probability that a text is in a certain language.
  // 
  // ///////////////////////////////////////////////////////////////


  ///////////////////////////////////////////////////////////
  /// Create and instance of the VMM with the given language n-gram model.
  ////////////////////////////////////////////////////////////

  idioma::idioma(const wstring &modelFile) {

    // default values
    order=0;
    threshold=15;

    TRACE(2,L"Loading ngram model from " << modelFile);
    // load ngram statistics and smoothing parameters
    map<wstring,wchar_t> escapes = { {L"\\n", L'\n'}, {L"\\s", L' '}, {L"\\\\", L'\\'} };
    smooth = new smoothingLD<wstring,wchar_t>(modelFile,escapes);

    // config file
    enum sections {CODE,ORDER,THRESHOLD,PHANTOM};
    config_file cfg(true);  
    cfg.add_section(L"Code",CODE,true);
    cfg.add_section(L"Order",ORDER,true);
    cfg.add_section(L"Phantom",PHANTOM,true);
    cfg.add_section(L"Threshold",THRESHOLD,false); // optional, use default if missing

    if (not cfg.open(modelFile))
      ERROR_CRASH(L"Error opening file "+modelFile);
  
    // load list of languages and their model files
    wstring line; 
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) {

      case CODE: {// read language code
        LangCode=line;
        break;
      }

      case ORDER: {// read order of ngram model
        wistringstream sin;
        sin.str(line);
        sin>>order;
        break;
      }

      case PHANTOM: {// read phantom char for initial state
        wistringstream sin;
        sin.str(line);
        sin>>phantom;
        break;
      }

      case THRESHOLD: {// read maximum perplexity to accept a sequence 
        wistringstream sin;
        sin.str(line);
        sin>>threshold;
        break;
      }

      default: break;
      }
    }
    cfg.close();

    TRACE(2,L"Loaded model for language "+LangCode);
  }

  //////////////////////////////////////////////////////////
  /// destructor
  //////////////////////////////////////////////////////////

  idioma::~idioma() { delete smooth; }


  //////////////////////////////////////////////////////////
  /// return code for current language
  //////////////////////////////////////////////////////////

  wstring idioma::get_language_code() const { return LangCode; }

  //////////////////////////////////////////////////////////
  /// get maximum allowed perplexity
  //////////////////////////////////////////////////////////
  
  double idioma::get_threshold() const { return threshold; }

  //////////////////////////////////////////////////////////
  /// Compute probabiltiy for given sequence according to 
  /// current model. Parameter "len" gets the actual length 
  /// of the sequence used after skipping redundant whitespaces.
  //////////////////////////////////////////////////////////

  double idioma::sequence_probability(wistream &f, size_t &len) const {

    TRACE(3,L"Computing sequence probability.");

    // initialize sequence probability using as initial state
    // the phantom  n-1 gram, plus the first character in the sequence.
    wstring ngram;
    wchar_t z;
    initial_ngram(f,ngram,z,order,phantom);

    double prob=0;
    len=0;
    while (not f.eof()) {

      TRACE(3,L"Current ngram: ("+to_writable(ngram)+L")  next symbol: ("+to_writable(z)+L")");

      /// Update sequence probability
      double pn = smooth->Prob(ngram,z);
      prob += pn;
      TRACE(4,L"   Ngram prob="+util::double2wstring(pn)+L" Accumulated prob="+util::double2wstring(prob));

      if (z==L'\n') 
        // Found end-of-paragraph.  Reset to initial state for next paragraf
        initial_ngram(f,ngram,z,order,phantom);
      
      else 
        // normal next, shift ngram window.
        next_ngram(f,ngram,z);
      
      len++;

    }
    
    return prob;
  }


  //////////////////////////////////////////////////////////
  /// Compute language probability, normalizing sequence
  /// probability by the senquence length.
  /////////////////////////////////////////////////////////

  double idioma::compute_perplexity(const wstring &text) const {

    size_t len;
    wistringstream sin(text);
    double prob = sequence_probability(sin, len);
    TRACE(4,L"   Sequence log probability="+util::double2wstring(prob)+L"  len="+util::double2wstring(len));

    double perp = exp(-prob/len);
    TRACE(4,L"   Perplexity = "+util::double2wstring(perp));
    return perp;
  }

  
  /////////////////////////////////////////////////////////////////////
  /// Initial ngram: n-1 phantom chars plus the first actual letter.
  //////////////////////////////////////////////////////////////////////

  void idioma::initial_ngram(wistream &f, wstring &ngram, wchar_t &z, int ord, wchar_t ph) {
    // Reset to init state for next paragraf
    ngram = wstring(ord-1,ph);
    if (not f.eof()) {
      f >> noskipws >> z;
      while (not f.eof() and (z==L' ' or z==L'\t' or z==L'\n')) 
        f >> noskipws >> z;
      z = towlower(z);
    }
  }

  /////////////////////////////////////////////////////////////////////
  /// slide ngram window one position to the left
  //////////////////////////////////////////////////////////////////////

  void idioma::next_ngram(wistream &f, wstring &ngram, wchar_t &z)  {
    ngram += z;
    ngram.erase(ngram.begin());
    
    // get next char
    f >> noskipws >> z; 
    // skip redundant whitespaces
    if (ngram.back()==L' ' or ngram.back()==L'\t') {
      while (not f.eof() and (z==L' ' or z==L'\t')) f >> noskipws >> z;
    }
    // z is next non whitespace char, ready for next ngram transition
    z = towlower(z);
  }


  ///////////////////////////////////////////////////////////////////////
  /// Convert an ascii string with newlines and whitespaces to 
  /// something easily writable
  ///////////////////////////////////////////////////////////////////////

  wstring idioma::to_writable(wchar_t c) {
    if (c==L'\n') return L"\\n";
    else if (c==L' ') return L"\\s";
    else if (c==L'\\') return L"\\\\";
    else return wstring(1,c);
  } 

  ///////////////////////////////////////////////////////////////////////
  /// Convert an ascii string with newlines and whitespaces to 
  /// something easily writable
  ///////////////////////////////////////////////////////////////////////

  wstring idioma::to_writable(const wstring &s) {
    wstring x;
    for (unsigned int i=0; i<s.size(); i++) 
      x = x + to_writable(s[i]);  
    return x;
  }

  ///////////////////////////////////////////////////////////////////// 
  /// Use given text file to count ngrams and create a model file.

  void idioma::create_model(const wstring &modelFile,
                            wistream &f, 
                            const wstring &code, 
                            int ord, 
                            wchar_t phant) {
    
    TRACE(3,L"Training. Creating model");
    map<wstring,size_t> counts;
    set<wchar_t> types;

    // initial ngram
    wstring ngram;
    wchar_t z;
    initial_ngram(f,ngram,z,ord,phant);
      
    while (not f.eof()) {
      // set of observed different unigrams
      types.insert(z);

      TRACE(3,L"Current trigram: ("+to_writable(ngram)+L")  next: ("+to_writable(z)+L")");
      
      /// Count n-gram occurrence
      counts[ngram+z]++;
      
      if (z==L'\n') 
        // Found end-of-paragraph. Reset to init state for next paragraf
        initial_ngram(f,ngram,z,ord,phant);
      
      else 
        // normal next, shift ngram window.
        next_ngram(f,ngram,z);
    }

    // dump model to output file
    wofstream sout;
    util::open_utf8_file(sout, modelFile);
    if (sout.fail()) ERROR_CRASH(L"Error opening file "+modelFile);

    // Write model parameters
    sout << L"<Code>" << endl << code << endl << L"</Code>" << endl;    
    sout << L"<Order>" << endl << ord << endl << L"</Order>" << endl;    
    sout << L"<Phantom>" << endl << phant << endl << L"</Phantom>" << endl;    
    sout << L"<Smoothing>" << endl;
    sout << L"LinearDiscountAlpha 0.1" << endl;    
    sout << L"VocabularySize " << int(types.size()*1.1) << endl;    
    sout << L"</Smoothing>" << endl;
    sout << L"<NGrams>" << endl;
    for (map<wstring,size_t>::const_iterator x=counts.begin(); x!=counts.end(); x++) {
      sout << x->second;
      for (wstring::const_iterator c=x->first.begin(); c!=x->first.end(); c++) 
        sout << L" " << to_writable(*c);
      sout << endl;
    }
    sout << L"</NGrams>" << endl;
    
    sout.close();
  }


} // namespace
