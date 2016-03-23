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
 * \file   token.cc
 * \brief  Implementation of methods of the class Token
 * \author Mihai Surdeanu, Xavier Carreras
 * \note   This file was ported from egstra
 */

#include <assert.h>
#include <iostream>

#include "treeler/base/token.h"
#include "treeler/util/char-utils.h"


using namespace std;

namespace treeler {


  Token::Token(int word,int lemma,int coarseTag,int fineTag,int feats)
  {
    _word = word;
    assert(_word>=0);
    _lemma = lemma;
    _coarse_tag = coarseTag;
    _fine_tag = fineTag;
    _feats = feats; 
    //    if(_fine_tag > _maxftag)   { _maxftag = _fine_tag; }
    //    if(_coarse_tag > _maxctag) { _maxctag = _coarse_tag; }
   }

  // int Token::_maxftag = -1;
  // int Token::_maxctag = -1;
  // int Token::_ntags = -1;
  // bool* Token::_tag_is_verb = NULL;
  // bool* Token::_tag_is_punc = NULL;
  // bool* Token::_tag_is_coord = NULL;
  
}

