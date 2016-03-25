/*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2014   TALP Research Center
 *                       Universitat Politecnica de Catalunya
 *
 *  This file is part of Treeler.
 *
 *  Treeler is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Treeler is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with Treeler.  If not, see <http: *www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   fidx.h
 * \brief  Declares several types of feature indices
 * \author Andreu Mayo, Xavier Carreras
 */

#ifndef BASE_FIDX_H
#define BASE_FIDX_H

#include <string>
#include <sstream>
#include "treeler/base/feature-idx-v0.h"
#include <iostream>

using namespace std;

namespace treeler {

  typedef FeatureIdx FIdxV0;

  class FIdxBits {
    public:
    uint64_t idx;
    
    typedef short Code;
    typedef bool Dir;
    typedef short Tag;
    typedef int Word;
    typedef long BigWord; 

    FIdxBits() {
      idx = 0;
    }
    
    FIdxBits(const uint64_t new_idx) {
      idx = new_idx;
    }

    void clear() {
      idx = 0;
    }
    
    template<typename T>
    static inline Code code(const int a, const T& b) { return 0x00ff & a; }
    static inline Dir  dir(const bool b) { return (b ? 1:0); }
    static inline Tag  tag(const int a) { return 0x00ff & a; }
    static inline Word word(const int a) { return a; }
    static inline Tag  rootTag() { return 0x0ff; }
    static inline Word rootWord() { return 0x0ffff; }
    static inline BigWord big_word(const long a) { return a ;} 

    
    inline FIdxBits& operator<<(const short& t) {
      this->idx <<= 8;
      this->idx |= t;
      return *this;
    }
    
    inline FIdxBits& operator<<(const bool& t) {
      this->idx <<= 1;
      this->idx |= t;
      return *this;
    }
    
    inline FIdxBits& operator<<(const int& t) {
      this->idx <<= 18;
      this->idx |= t;
      return *this;
    }

    inline FIdxBits& operator<<(const uint64_t& t) {
      this->idx <<= 34; //limit to 34 bits
      this->idx |= t;
      return *this;
    }

    inline FIdxBits& operator<<(const long& t) {
      assert(t>=0);
      this->idx <<= 54; //limit to 54 bits
      this->idx |= t;
      return *this;
    }

    /*
    inline void write(ostream& os) const {
      char tmpidx[17];
      tmpidx[16] = '\0';
      feature_idx_sprint(tmpidx,this->idx);
      os << tmpidx;
    }
    */
    inline const uint64_t& operator()() const {
      return this->idx;
    }
 
  };
  
  ostream& operator<<(ostream& os, const FIdxBits& f); 
  
  class FIdxChars {
    public:
    string idx;
    
    typedef string Code;
    typedef char   Dir;
    typedef string Tag;
    typedef string Word;
    typedef string BigWord; 
    
    FIdxChars() {
      idx = "";
    }
    
    FIdxChars(const string& new_idx) {
      idx = new_idx;
    }

    FIdxChars(const FIdxChars& c) {
      //      cerr << "(copy constructor for " << c.idx << ")" << flush;
      idx = c.idx;
    }

    void clear() {
      idx = ""; 
    }

    template<typename T>
    static inline Code code(const T& a, const string& b) { return b; }
    static inline Dir  dir(const bool a) { return (a ? 'r':'l'); }
    static inline Tag  tag(const int a) { stringstream ss; ss<<a; return ss.str(); }
    static inline Tag  tag(const string& a) { return a; }
    static inline Word word(const int a) { stringstream ss; ss<<a; return ss.str(); }
    static inline Word word(const string& a) { return a; }
    static inline Tag  rootTag() { return "*R*"; }
    static inline Word rootWord() { return "*R*"; }
    static inline BigWord big_word(const string& bw){ return bw; }
    
    template<typename T>
    inline FIdxChars& operator<<(const T& t) {
      ostringstream oss; 
      if (this->idx != "") oss << t << ':' << this->idx;
      else oss << t;
      this->idx = oss.str();
      return *this;
    }
    /*
    inline void write(ostringstream& oss) const {    
      oss << this->idx; 
    }
    */
    inline const string& operator()() const {
      return this->idx;
    }
    
  };
  
  ostream& operator<<(ostream& os, const FIdxChars& f); 
  
  class FIdxPair {
    public:
    pair<FIdxBits, FIdxChars> idx;
    
    typedef pair<FIdxBits::Code, FIdxChars::Code> Code;
    typedef pair<FIdxBits::Dir, FIdxChars::Dir> Dir;
    typedef pair<FIdxBits::Tag, FIdxChars::Tag> Tag;
    typedef pair<FIdxBits::Word, FIdxChars::Word> Word;
    typedef pair<FIdxBits::BigWord, FIdxChars::BigWord> BigWord;

    
    FIdxPair() {
      idx.first = FIdxBits();
      idx.second = FIdxChars();
    }
    
    FIdxPair(const pair<FIdxBits, FIdxChars>& new_idx) {
      idx.first = FIdxBits(new_idx.first);
      idx.second = FIdxChars(new_idx.second);
    }
    
    void clear() {
      idx.first.clear(); 
      idx.second.clear(); 
    }

    static inline Code code(const int a, const string& b) { return Code(FIdxBits::code(a, b), FIdxChars::code(a, b)); }
    static inline Dir dir(const bool a) { return Dir(FIdxBits::dir(a), FIdxChars::dir(a)); }
    static inline Tag tag(const int a) { return Tag(FIdxBits::tag(a), FIdxChars::tag(a)); }
    static inline Tag tag(const Tag& a) { return a; }
    static inline Word word(const int a) { return Word(FIdxBits::word(a), FIdxChars::word(a)); }
    static inline Word word(const Word& a) { return a; }
    static inline Tag rootTag() { return Tag(FIdxBits::rootTag(), FIdxChars::rootTag()); }
    static inline Word rootWord() { return Word(FIdxBits::rootWord(), FIdxChars::rootWord()); }
    static inline BigWord big_word(const FIdxBits::BigWord &a, const FIdxChars::BigWord &b){
      return BigWord(a, b);
    }
    
    template<typename T>
    inline FIdxPair& operator<<(const T& t) {
      this->idx.first << t.first;
      this->idx.second << t.second;
      return *this;
    }
    /*
    inline void write(ostringstream& oss) const {
      oss << "<";
      this->idx.first.write(oss);
      oss << "=";
      this->idx.second.write(oss);
      oss << ">";
    }
    */
    inline const pair<FIdxBits, FIdxChars>& operator()() const {
      return this->idx;
    }

  };
  
  ostream& operator<<(ostream& os, const FIdxPair& f); 

  template<typename FA, typename FIdx>
  struct SwitchFIdx {
    typedef FA F;
  };
  
}

#endif
