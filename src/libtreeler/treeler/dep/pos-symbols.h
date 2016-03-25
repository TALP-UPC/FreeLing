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
#ifndef TREELER_POSSYMBOLS_H
#define TREELER_POSSYMBOLS_H

#include <string>
#include <cassert>
#include <iostream>

#include "treeler/base/windll.h"

using namespace std; 

namespace treeler {

  /**
   *  Linguistic information about part-of-speech tags
   */
  class WINDLL PoSSymbols {
  private:
    /* mapping from POS tags (coarse and fine) to tag classes */
    int _ntags;
    bool* _tag_is_verb;
    bool* _tag_is_punc;
    bool* _tag_is_coord;
    bool* _tag_is_past_participle;
    bool* _tag_is_noun;
    bool* _tag_is_prep;
    bool* _tag_is_modal;
    bool* _tag_is_rb;
    bool* _tag_is_to;
 
  public:

    PoSSymbols() 
      : _ntags(0), _tag_is_verb(NULL), _tag_is_punc(NULL), _tag_is_coord(NULL),
        _tag_is_past_participle(NULL),
        _tag_is_noun(NULL),
        _tag_is_prep(NULL),
        _tag_is_modal(NULL),
        _tag_is_rb(NULL),
        _tag_is_to(NULL)
      {}

    void load_tag_map(const string& path); 

    //! Indicates whether the token is a verb
    //! \todo This implementation is treebank-dependant, needs a rewrite. 
    bool is_verb(int tag) const {
      assert(tag >= 0);
      assert(tag < _ntags);
      return _tag_is_verb[tag];
    }

    //! Indicates whether the token is a punctuation sign
    //! \todo This implementation is treebank-dependant, needs a rewrite. 
    bool is_punc(int tag) const {
      assert(tag >= 0); assert(tag < _ntags);
      return _tag_is_punc[tag];
    }
    
    //! Indicates whether the token is a coordination
    //! \todo This implementation is treebank-dependant, needs a rewrite. 
    bool is_coord(int tag) const {
      assert(tag >= 0); assert(tag < _ntags);
      return _tag_is_coord[tag];
    }

    //! Indicates whether the token is a past participle
    //! \todo This implementation is treebank-dependant, needs a rewrite. 
    bool is_past_participle(int tag) const {
      assert(tag >= 0); assert(tag < _ntags);
      return _tag_is_past_participle[tag];
    }

    //! Indicates whether the token is a noun
    //! \todo This implementation is treebank-dependant, needs a rewrite. 
    bool is_noun(int tag) const {
      assert(tag >= 0); assert(tag < _ntags);
      return _tag_is_noun[tag];
    }

    //! Indicates whether the token is a preposition 
    //! \todo This implementation is treebank-dependant, needs a rewrite. 
    bool is_prep(int tag) const {
      assert(tag >= 0); assert(tag < _ntags);
      return _tag_is_prep[tag];
    }

    //! Indicates whether the token is a modal
    //! \todo This implementation is treebank-dependant, needs a rewrite. 
    bool is_modal(int tag) const {
      assert(tag >= 0); assert(tag < _ntags);
      return _tag_is_modal[tag];
    }

    //! Indicates whether the token is a modal
    //! \todo This implementation is treebank-dependant, needs a rewrite. 
    bool is_rb(int tag) const {
      assert(tag >= 0); assert(tag < _ntags);
      return _tag_is_rb[tag];
    }

    //! Indicates whether the token is a modal
    //! \todo This implementation is treebank-dependant, needs a rewrite. 
    bool is_to(int tag) const {
      assert(tag >= 0); assert(tag < _ntags);
      return _tag_is_to[tag];
    }

  };

}

#endif

