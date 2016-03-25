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
 *  along with Treeler.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \author Xavier Carreras
 */
#ifndef TREELER_DEPSYMBOLS_H
#define TREELER_DEPSYMBOLS_H

#include <string>
using namespace std; 

#include "treeler/base/dictionary.h"
#include "treeler/base/basic-sentence.h"
#include "treeler/dep/dep-tree.h"
#include "treeler/dep/pos-symbols.h"


namespace treeler {

  class DepSymbols;

  namespace _dep_symbols_internal {
    
    template <typename T1, typename T2>
    inline T1 map_value(const Dictionary& d, const T2& t)  { return d.map(t); } 
    
    template <>
    inline int map_value(const Dictionary& d, const int& t) { return t; } 

    template <>
    inline short int map_value(const Dictionary& d, const int& t) { return (short) t; } 

    template <>
    inline string map_value(const Dictionary& d, const string& t) { return t; } 

    // template <> 
    // inline string map_value<string>(const Dictionary& d, const int& t) { return d.map(t); }

    // template <> 
    //   inline int map_value<int>(const Dictionary& d, const string& t) { return d.map(t); }


    template <> 
    inline std::pair<int,string> map_value<std::pair<int,string>>(const Dictionary& d, const string& t) { 
      return make_pair(d.map(t),t); 
    }

    template <> 
    inline std::pair<short int,string> map_value<std::pair<short int,string>>(const Dictionary& d, const string& t) { 
      return make_pair(d.map(t),t); 
    }

    template <> 
    inline std::pair<int,string> map_value<std::pair<int,string>>(const Dictionary& d, const int& t) { 
      return make_pair(t,d.map(t)); 
    }

    template <> 
    inline std::pair<short int,string> map_value<std::pair<short int,string>>(const Dictionary& d, const int& t) { 
      return make_pair(t,d.map(t)); 
    }


    template <int FIELD> 
    struct FieldMap {};

    template <> 
    struct FieldMap<WORD> {
      template <typename SYM, typename T1, typename T2>
      static inline T1 map_field(const SYM& Sym, const T2& t) { 
	return _dep_symbols_internal::map_value<T1,T2>(Sym.d_words, t); 
      }
    };   

    template <> 
    struct FieldMap<LEMMA> {
      template <typename SYM, typename T1, typename T2>
      static inline T1 map_field(const SYM& Sym, const T2& t) { 
	return _dep_symbols_internal::map_value<T1,T2>(Sym.d_lemmas, t); 
      }
    };   

    template <> 
    struct FieldMap<COARSE_POS> {
      template <typename SYM, typename T1, typename T2>
      static inline T1 map_field(const SYM& Sym, const T2& t) { 
	return _dep_symbols_internal::map_value<T1,T2>(Sym.d_coarse_pos, t); 
      }
    };   

    template <> 
    struct FieldMap<FINE_POS> {
      template <typename SYM, typename T1, typename T2>
      static inline T1 map_field(const SYM& Sym, const T2& t) { 
	return _dep_symbols_internal::map_value<T1,T2>(Sym.d_fine_pos, t); 
      }
    };   

    template <> 
    struct FieldMap<MORPHO_TAG> {
      template <typename SYM, typename T1, typename T2>
      static inline T1 map_field(const SYM& Sym, const T2& t) { 
	return _dep_symbols_internal::map_value<T1,T2>(Sym.d_morpho_tags, t); 
      }
    };   

    template <> 
    struct FieldMap<SYNTACTIC_LABEL> {
      template <typename SYM, typename T1, typename T2>
      static inline T1 map_field(const SYM& Sym, const T2& t) { 
	return _dep_symbols_internal::map_value<T1,T2>(Sym.d_syntactic_labels, t); 
      }
    };   

  }


  /** 
   *  \brief A variety of symbol mappings useful for dependency parsing
   *
   */
  class DepSymbols {
  private:
    PoSSymbols _pos_symbols; 


  public:

    Dictionary d_words, d_lemmas, d_coarse_pos, d_fine_pos, d_morpho_tags, d_syntactic_labels;


    template <int FIELD, typename T1, typename T2> 
    inline T1 map_field(const T2& t) const {
      typedef _dep_symbols_internal::FieldMap<FIELD> FieldMap; 
      return FieldMap::template map_field<DepSymbols,T1,T2>(*this, t); 
    }

    void print_stats(ostream& o) const {
      o << "Dep-symbols: " 
	<< "(words: " << d_words.size() << ") " 
	<< "(lemmas: " << d_lemmas.size() << ") " 
	<< "(cpos: " << d_coarse_pos.size() << ") " 
	<< "(fpos: " << d_fine_pos.size() << ") " 
	<< "(morphos: " << d_morpho_tags.size() << ") " 
	<< "(synl: " << d_syntactic_labels.size() << ") " 
	<< endl;
    }

    template <typename LexT1, typename TagT1, typename LexT2, typename TagT2>
    void map(const BasicSentence<LexT1,TagT1>& source, BasicSentence<LexT2,TagT2>& target) const {
      target.resize(source.size()); 
      for (size_t i=0; i< (size_t) source.size(); ++i) {
	auto& st = source[i]; 
	auto& tt = target[i]; 
	tt.word() = map_field<WORD,LexT2>(st.word()); 
	tt.lemma() = map_field<LEMMA,LexT2>(st.lemma()); 
	tt.coarse_pos() = map_field<COARSE_POS,LexT2>(st.coarse_pos()); 
	tt.fine_pos() = map_field<FINE_POS,TagT2>(st.fine_pos());
	tt.morpho_clear(); 	
	for ( auto it = st.morpho_tags().begin(); it != st.morpho_tags().end(); ++it  ) {
	  tt.morpho_push(map_field<MORPHO_TAG,TagT2>(*it) );
	}
      }
    }

    
    template <typename T1, typename T2>
    void map(const DepVector<T1>& source, DepVector<T2>& target) const {
      target.resize(source.size()); 
      for (size_t i=0; i< (size_t) source.size(); ++i) {
	auto& s = source[i]; 
	auto& t = target[i]; 
	t.l = map_field<SYNTACTIC_LABEL,T2>(s.l); 
      }
    }
    
    void load_tag_map(const string& path) {
      _pos_symbols.load_tag_map(path); 
    }
    
    bool is_verb(int t) const { return _pos_symbols.is_verb(t); }
    bool is_verb(string t) const { return _pos_symbols.is_verb( map_field<FINE_POS,int>(t) ); }

    bool is_punc(int t) const { return _pos_symbols.is_punc(t); }
    bool is_punc(string t) const { return _pos_symbols.is_punc( map_field<FINE_POS,int>(t) ); }
    
    bool is_coord(int t) const { return _pos_symbols.is_coord(t); }
    bool is_coord(string t) const { return _pos_symbols.is_coord( map_field<FINE_POS,int>(t) ); }
    
    bool is_past_participle(string t) const { 
      return _pos_symbols.is_past_participle( map_field<FINE_POS,int>(t) ); 
    }
    bool is_preposition(string t) const { 
      return _pos_symbols.is_prep( map_field<FINE_POS,int>(t) ); 
    }
    bool is_noun(string t) const { 
      return _pos_symbols.is_noun( map_field<FINE_POS,int>(t) ); 
    }
    bool is_rb(string t) const { 
      return _pos_symbols.is_rb( map_field<FINE_POS,int>(t) ); 
    }
    bool is_modal(string t) const { 
      return _pos_symbols.is_modal( map_field<FINE_POS,int>(t) ); 
    }
    bool is_to(string t) const { 
      return _pos_symbols.is_to( map_field<FINE_POS,int>(t) ); 
    }

    bool is_lemma_to_be(string t) const { 
      //TODO
      assert(false);
      //return _pos_symbols.is_lemma_to_be( map_field<FINE_POS,int>(t) ); 
      return false;
    }
    
  };  



}

#endif

