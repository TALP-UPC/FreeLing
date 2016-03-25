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

#include "treeler/dep/pos-symbols.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

using namespace std;

namespace treeler {

  void PoSSymbols::load_tag_map(const string& path) {

    if(_tag_is_verb != NULL) {
      delete [] _tag_is_verb;
      delete [] _tag_is_punc;
      delete [] _tag_is_coord;
      delete [] _tag_is_past_participle;
      delete [] _tag_is_noun;
      delete [] _tag_is_prep;
      delete [] _tag_is_rb;
      delete [] _tag_is_modal;
      delete [] _tag_is_to;
    }

    vector<bool> isverb;
    vector<bool> ispunc;
    vector<bool> iscoord;
    vector<bool> ispastpart;
    vector<bool> isnoun;
    vector<bool> isprep;
    vector<bool> isrb;
    vector<bool> ismodal;
    vector<bool> isto;

    std::ifstream is(path.c_str());
    if (!is) {
      cerr << "PoSSymbols::load_tag_map : can not open file " << path << endl;
      exit(1);
    }
    string line;
    while(getline(is,line)){
      vector<string> tokens;
      istringstream iss(line);
      int idx; 
      string tagstr;
      iss >> idx >> tagstr;      
      const char* const tag = tagstr.c_str();
      if(idx >= (int)isverb.size())  { isverb.resize(idx + 1, false); }
      if(idx >= (int)ispunc.size())  { ispunc.resize(idx + 1, false); }
      if(idx >= (int)iscoord.size()) { iscoord.resize(idx + 1, false); }
      if(idx >= (int)ispastpart.size()) { ispastpart.resize(idx + 1, false); }
      if(idx >= (int)isnoun.size()) { isnoun.resize(idx + 1, false); }
      if(idx >= (int)isprep.size()) { isprep.resize(idx + 1, false); }
      if(idx >= (int)isrb.size()) { isrb.resize(idx + 1, false); }
      if(idx >= (int)ismodal.size()) { ismodal.resize(idx + 1, false); }
      if(idx >= (int)isto.size()) { isto.resize(idx + 1, false); }
      /* check for membership */
      if(tag[0] == 'v' || tag[0] == 'V') {
        isverb[idx] = true;
      } else if(strcmp(tag, "Punc") == 0 ||
          strcmp(tag, "$,") == 0 ||
          strcmp(tag, "$.") == 0 ||
          strcmp(tag, "PUNC") == 0 ||
          strcmp(tag, "punc") == 0 ||
          strcmp(tag, "F") == 0 ||
          strcmp(tag, "IK") == 0 ||
          strcmp(tag, "XP") == 0 ||
          strcmp(tag, ",") == 0 ||
          strcmp(tag, ";") == 0 ||
          tag[0]=='F' || tag[0]=='f') {
        ispunc[idx] = true;
      } else if(strcmp(tag, "Conj") == 0 ||
          strcmp(tag, "KON") == 0 ||
          strcmp(tag, "conj") == 0 ||
          strcmp(tag, "Conjunction") == 0 ||
          strcmp(tag, "CC") == 0 ||
          strcmp(tag, "cc") == 0) {
        iscoord[idx] = true;
      } else if (strcmp(tag, "VBN") == 0){
        ispastpart[idx] = true;
      } else if (tag[0] == 'N'){
        isnoun[idx] = true;
      } else if (strcmp(tag, "IN") == 0){
        isprep[idx] = true;
      } else if (strcmp(tag, "MD") == 0){
        ismodal[idx] = true;
      } else if (tag[0] == 'R' and tag[1] == 'B'){
        isrb[idx] = true;
      } else if (strcmp(tag, "TO") == 0){
        isto[idx] = true;
      }
    }

    is.close();

    /* make room for a final tag -- used for unknown tags */
    {
      isverb.resize(isverb.size()+1, false); 
      ispunc.resize(ispunc.size()+1, false); 
      iscoord.resize(iscoord.size()+1, false); 
      ispastpart.resize(ispastpart.size()+1, false); 
      isnoun.resize(isnoun.size()+1, false); 
      isprep.resize(isprep.size()+1, false); 
      isrb.resize(isrb.size()+1, false); 
      ismodal.resize(ismodal.size()+1, false); 
      isto.resize(isto.size()+1, false); 
    }

    /* copy the vectors into the arrays */
    assert(ispunc.size() == isverb.size());
    assert(iscoord.size() == isverb.size());
    assert(ispastpart.size() == isverb.size());
    assert(isnoun.size() == isverb.size());
    assert(isprep.size() == isverb.size());
    assert(isrb.size() == isverb.size());
    assert(ismodal.size() == isverb.size());
    assert(isto.size() == isverb.size());

    _ntags = ispunc.size();
    _tag_is_verb = new bool[_ntags];
    _tag_is_punc = new bool[_ntags];
    _tag_is_coord = new bool[_ntags];
    _tag_is_past_participle = new bool[_ntags];
    _tag_is_noun = new bool[_ntags];
    _tag_is_prep = new bool[_ntags];
    _tag_is_rb = new bool[_ntags];
    _tag_is_modal = new bool[_ntags];
    _tag_is_to = new bool[_ntags];
    for(int i = 0; i < _ntags; ++i) {
      _tag_is_verb[i]  = isverb[i];
      _tag_is_punc[i]  = ispunc[i];
      _tag_is_coord[i] = iscoord[i];
      _tag_is_past_participle[i] = ispastpart[i];
      _tag_is_noun[i] = isnoun[i];
      _tag_is_prep[i] = isprep[i];
      _tag_is_rb[i] = isrb[i];
      _tag_is_modal[i] = ismodal[i];
      _tag_is_to[i] = isto[i];
    }
  }

}
