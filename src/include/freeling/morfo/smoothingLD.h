////////////////////////////////////////////////////////////////
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

#ifndef _SMOOTHING_LD
#define _SMOOTHING_LD

#include <cmath>
using std::log;

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"


#undef MOD_TRACENAME
#define MOD_TRACENAME L"SMOOTHING"
#undef MOD_TRACEMODULE
#define MOD_TRACEMODULE LANGIDENT_TRACE

namespace freeling {
  
  ///////////////////////////////////////////////////////////////
  /// Class smoothingLD computes linear-discount smoothed conditional 
  /// probabilities P(z|w1...wn) for n-gram transitions.
  /// 
  /// template parameters:
  ///   - E is the type of the ngram elements (e.g. char, int, string, etc.)
  ///   - G is the type containing the ngram (e.g. string, list<int>, vector<string>, etc.)
  ///     Elements of G must be of type E. 
  ///     G must have operations push_back(E), erase(G::iterator), and begin()
  ///////////////////////////////////////////////////////////////

  template <class G,class E> 
  class smoothingLD {
    
  private:
    /// log alpha and 1-alpha parameter for linear discount 
    double alpha; 
    double notalpha;
    /// map to store ngram counts (for any size of ngram)
    std::map<G,double> counts;
    
    // probability of unseen unigrams
    double pUnseen;
    // total number of observations
    double nobs; 

    /// order of the n-gram model. Order=3 => trigram model P(z|xy)
    size_t order;

    /// translation table for escaped symbols in input (e.g. \s, \n, \t...)
    std::map<std::wstring,E> escapes;
    
    //////////////////////////////////////////
    // Get observed counts for given ngram
    
    double count(const G &ngram) const {
      typename std::map<G, double>::const_iterator p = counts.find(ngram);
      if (p!=counts.end()) return p->second;
      else return -1;
    }
    
        
  public:
    
    //////////////////////////////////////////
    /// Constructor, load data from config file
    
    smoothingLD(const std::wstring &cfgFile, 
                const std::map<std::wstring,E> &esc=std::map<std::wstring,E>()) : escapes(esc) { 
        
      double ntypes=0; 
      double vsize=0;
      nobs=0; 
      order=0;
      
      enum sections {ORDER,NGRAMS,SMOOTHING};
      config_file cfg(true);  
      cfg.add_section(L"Order",ORDER,true);
      cfg.add_section(L"NGrams",NGRAMS,true);
      cfg.add_section(L"Smoothing",SMOOTHING,true);
      
      if (not cfg.open(cfgFile))
        ERROR_CRASH(L"Error opening file "+cfgFile);
      
      std::wstring line; 
      while (cfg.get_content_line(line)) {
        
        std::wistringstream sin;
        sin.str(line);
        
        // process each content line according to the section where it is found
        switch (cfg.get_section()) {
          
        case ORDER: {// read order of ngram model
          std::wistringstream sin;
          sin.str(line);
          size_t x; sin>>x;
          if (order!=0 and order!=x)
            ERROR_CRASH(L"ERROR - Specified model order does not match ngram size");
          order = x;
          break;
        }

        case SMOOTHING: { // reading general parameters
          std::wstring name;
          sin>>name;
          if (name==L"LinearDiscountAlpha") { double a; sin>>a; alpha = log(a); notalpha=log(1-a); }
          else if (name==L"VocabularySize") sin>>vsize;
          else ERROR_CRASH(L"Unexpected smoothing option '"+name+L"'");
          break;
        }
          
        case NGRAMS: {  // reading ngram counts
          // read counts for this ngram to table
          double c; sin >> c;
          // read ngram components into a G object.
          G ngram;
          std::wstring w; 
          while (sin >> w) {
            typename std::map<std::wstring,E>::const_iterator p=escapes.find(w);
            if (p!=escapes.end())                
              ngram.push_back(p->second);
            else
              ngram.push_back(util::wstring_to<E>(w));
          }

          if (order==0) order = ngram.size();
          else if (order != ngram.size()) 
            ERROR_CRASH(L"ERROR - Mixed order ngrams in input file, or specified model order does not match ngram size");

          // add ngram (and n-i gram) counts to the model
          while (ngram.size()>1) {
            // insert ngram count, or increase if it already existed
            std::pair<typename std::map<G,double>::iterator,bool> x = counts.insert(make_pair(ngram,c));
            if (not x.second) x.first->second += c;            
            // shorten n gram and loop to insert n-1 gram
            ngram.erase(std::prev(ngram.end()));
          }
          // unigram is left. Add it
          std::pair<typename std::map<G,double>::iterator,bool> x = counts.insert(make_pair(ngram,c));
          if (x.second) ntypes++; // new unigram inserted, increase type count
          else x.first->second += c;  // existing unigram, increase count

          // update total observation counts
          nobs += c;
          break;
        }
          
        default: break;
        }
      }
      cfg.close();       

      // precompute logs needed for logprob
      if (vsize<=ntypes)
        ERROR_CRASH(L"VocabularySize can not be smaller than number of different observed unigrams.");
      pUnseen = -log(vsize-ntypes); // log version of 1/(vsize-ntypes)
      nobs = log(nobs);
      for (typename std::map<G,double>::iterator c=counts.begin(); c!=counts.end(); c++) 
        c->second = log(c->second); 
    }
    
    //////////////////////////////////////////
    /// destructor
    
    ~smoothingLD() {}
         
    //////////////////////////////////////////
    /// Compute smoothed conditional log prob of seeing 
    /// symbol z following given ngram P(z|ngram)
    
    double Prob(const G &ngram, const E &z) const {
      
      // seq =  n+1 gram ( ngram + z )
      G seq = ngram; 
      seq.push_back(z);
      // log count of complete ngram
      double c = count(seq);

      if (ngram.size()==0) {
        // no conditioning history, use unigrams (seq = [z])
        if (c>=0) return notalpha + c - nobs;  // log version of (1-alpha)*count(c)/nobs
        else return alpha + pUnseen;   // log version of alpha * punseen
      }
      
      else {
        // conditioning history, use LD smoothing
        if (c>=0) return notalpha + c - count(ngram); // log version of (1-alpha)*count(c)/count(ngram)
        else {
          // shorten history and recurse
          G sht = ngram; sht.erase(sht.begin());
          return alpha + Prob(sht,z);  // log version of alpha * Prob(sht,z)
        }
      }
    }
  };
  
} // namespace

#undef MOD_TRACENAME
#undef MOD_TRACECODE

#endif

