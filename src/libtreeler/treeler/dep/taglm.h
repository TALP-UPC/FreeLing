//////////////////////////////////////////////////////////////////
//
//    Treeler - Open-source Structured Prediction for NLP
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//                          02111-1307 USA
//
//    contact: Xavier Carreras (carreras@lsi.upc.es)
//             TALP Research Center
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////
/* TAGLM
   Adapted from mttag::tag_lm

   Author : Xavier Carreras  */

#ifndef TREELER_DEP_TAGLM_H
#define TREELER_DEP_TAGLM_H

#include <ext/hash_map>
#include <iostream>
#include "treeler/dep/ssentence.h"

using namespace std;
using namespace __gnu_cxx;

namespace __gnu_cxx
{
  template<> struct hash< std::string >
  {
    size_t operator()( const std::string& x ) const
    {
      return hash< const char* >()( x.c_str() );
    }
  };
}

namespace treeler {

  class TAGLM  {
  private:

    static string SEP;
    static string GIVEN;
    static string UNKNOWN;
    static string NULLTOK;

    struct eqstr
    {
      bool operator()(const string& s1, const string& s2) const {
	return s1==s2;
      }
    };


    int _TOTAL_MOD; // total number of modifiers seen in the events
    hash_map<string,int> _countsA, _countsB, _countsC;

    double _bonus; 
    double _weight;

    bool _use_siblings;

/*     bool _cache_active;  */
/*     int _cache_last;  */
/*     struct cache_hash_fun { */
/*       size_t operator()(const mttag_part p) const { */
/* 	return p.hash_code(); */
/*       } */
/*     }; */
    // typedef hash_map<mttag_part, double, cache_hash_fun> cache_hash_t;
    // typedef hash_map<string, double> cache_hash_t;    
    // cache_hash_t _cache_hash;

    struct token {
      int head;
      string word, tag, spine, att_type, pos, state;
    };
    
    bool read_sentence(istream& in, vector<token>& sentence);

    inline double back_off_estimate(int l, int* den, int* num, const int b) const;


    void print_counts(ostream& out, int l, int* den, int* num, const string type) const;
    void print_size() const;

  public:

    TAGLM()
      : _TOTAL_MOD(0), _bonus(0.0), _weight(1.0), _use_siblings(true) //, _cache_active(false), _cache_last(-1)
      {}
    
    ~TAGLM()
      {}

    double score(const SSentence& x, const string& part, bool verbose);
    double score_dep(const SSentence& x, 
		     const int& hidx, const int& midx, const int& cidx, int pos, bool first, bool verbose); 

    double score_stop(const SSentence& x, 
		      bool goes_right, bool first, const int& pos, const int& hidx, const int& cidx, bool verbose);

    void extract_events(string file);
    void dump_events(ostream& out);
    void read_events(istream& in, bool dumped);
    void read_dumped_events(istream& in);
    
    void use_cache(bool b) {
      assert(0);
      bool _cache_active = b; 
      if (_cache_active) {
	cerr << "tag_lm :: using cache" << endl; 
      }
      else {
	cerr << "tag_lm :: no cache" << endl; 
      }
    }

    void set_bonus(double b) { 
      _bonus = b;
      cerr << "tag_lm :: setting word bonus to " << _bonus << endl;
    }
    void set_weight(double w) { 
      _weight = w;
      cerr << "tag_lm :: setting language model weight to " << _weight << endl;
    }

    void use_siblings(bool b) { _use_siblings = b;}

  };
}

#endif
