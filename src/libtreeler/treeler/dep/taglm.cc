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
#include "treeler/dep/taglm.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <cmath>

#define hash_access(H,key,count) \
  {                                                        \
    hash_map<string,int>::const_iterator i = H.find(key);  \
    count = (i==H.end()) ? 0 : i->second;                  \
  }                                                        \

#define cache_access(H,key,b,w) \
  {                                                        \
    cache_hash_t::const_iterator i = H.find(key);          \
    b = i!=H.end();                                        \
    w = (i==H.end()) ? 0 : i->second;                      \
  }                                                        \

using namespace std;
using namespace __gnu_cxx;

namespace treeler {

  string TAGLM::SEP     = "__";
  string TAGLM::GIVEN   = "_|_";
  string TAGLM::UNKNOWN = "?????";
  string TAGLM::NULLTOK = "NULL";
  

  void TAGLM::extract_events(string file) {
    assert(0);
  }

  inline double TAGLM::back_off_estimate(int l, int* den, int* num, const int b) const {
    double score = (double) num[0] / (double) den[0];
    for(int i=1; i<l and den[i]>0; ++i) {       
      double lambda = (double) den[i] / (double)(b+den[i]);
      score = lambda * (double) num[i]/(double) den[i] + (1.0 - lambda) * score;
    }
    return score;
  }
  
  void TAGLM::print_counts(ostream& out, int l, int* den, int* num, const string type) const {
    for(int i=0; i<l; ++i) {       
      if (i>0) out << " "; 
      out << type << i << " " << den[i] << " " << num[i];
    }
  }
  
  double TAGLM::score(const SSentence& x, const string& part, bool verbose) {
    assert(0);
    
//     //    string cache_key;
//     if (_cache_active and (p.type()==mttag_part::STOP or p.type()==mttag_part::DEP)) {
//       if (x->id() != _cache_last) {
// 	_cache_hash.clear();
// 	_cache_last = x->id();
//       }
// //       ostringstream oss; 
// //       oss << p; 
// //       cache_key = oss.str();
//       bool b = false;
//       double score;
//       //      cache_access(_cache_hash, cache_key, b, score);
//       cache_access(_cache_hash, p, b, score); 
//       if (b) {
// 	if (verbose) {cerr << "[cached]" << flush;}
// 	return score;
//       }
//     }

//     if (p.type() == mttag_part::STOP) {
//       return score_stop(x, p, verbose);

//     }
//     else if (p.type()==mttag_part::DEP) {
//       return score_dep(x, p, verbose); 

//     }
//     ///////////////////////////////////////////////////////////////////////
//     ////  UNIGRAM PROBABILITIES : P(word,spine)  //////////////////////////
//     ///////////////////////////////////////////////////////////////////////
//     else if (p.type() == mttag_part::UNI) {
// //       int hn_idx = p.head_node();
// //       int ht_idx = p.head_token();
// //       const mttag_treelet& htree = x->get_token(hn_idx);
// //       const mttag_token& htok = htree[ht_idx];

//       // ROOT 
//       if (p.head_node()==0) {
// 	return 0;
//       }
      
//       assert(_TOTAL_MOD>0);

//       const mttag_token& htok = (x->get_token(p.head_node()))[p.head_token()];

//       // relative freq of the the word and spine
//       int n;
//       hash_access(_countsC, htok.word() + SEP + htok.spine(), n);

//       // we don't want heuristics going to infinity
//       if (n==0) {
// 	n=1;
//       }
//       double p = ((double) n / (double) _TOTAL_MOD);

//       if (verbose) {
// 	cerr << "[U0 " << _TOTAL_MOD << " " << n << " : " << p << "]" << flush;
//       }

//       return _weight*log(p) + _bonus;
//     }
    return 0;
  }





  double TAGLM::score_dep(const SSentence& x, 
			  const int& hidx, const int& midx, const int& cidx, int pos, bool first, bool verbose) {

    const SSentence::Token& htok = x.get_token(hidx);
    const SSentence::Token& mtok = x.get_token(midx);
      
    //      string dir = (hn_idx<mn_idx or (hn_idx==mn_idx and ht_idx<mt_idx)) ? "R" : "L";
    const string dir = (hidx>midx) ? "R" : "L";
    const string state = (first) ? "F+" : "F-";
    const int att_pos = pos;
    const string att_type = "S";
    const string pnode = htok.spine_node(att_pos); 
    const string dsp = dir + SEP + state + SEP + pnode;
      
    // DISTRIBUTION A, generates the top of the spine and pos tag of the dependant node
    // counts for distribution A
    int Aden[5], Anum[5];
      
    // first backoff level for A      
    hash_access(_countsA, dsp, Aden[0]);
    if (Aden[0] == 0) {
      if (verbose) { cerr << "[A0 0]" << " (" << dsp << ") " << flush; }
      //if (_cache_active) _cache_hash[cache_key] = -HUGE_VAL;
      //if (_cache_active) _cache_hash[p] = -HUGE_VAL;
      return -HUGE_VAL;
    }
      
    string mnode_mtag = mtok.spine_top() + SEP + mtok.pos_tag();
    hash_access(_countsA, mnode_mtag + SEP + dsp, Anum[0]);
    if (Anum[0] == 0) {
      if (verbose) { cerr << "[A0 " << Aden[0] << " 0] ( " << mnode_mtag << " " << dsp << " ) " << flush; }
      //	if (_cache_active) _cache_hash[cache_key] = -HUGE_VAL;
      // if (_cache_active) _cache_hash[p] = -HUGE_VAL;
      return -HUGE_VAL;
    }

    // second backoff level for A 
    const string& hnode = att_pos>0 ? htok.spine_node(att_pos-1) : htok.pos_tag();
    string cond = dsp + SEP + hnode;
    hash_access(_countsA, cond, Aden[1]);
    hash_access(_countsA, mnode_mtag + SEP + cond, Anum[1]);
      
    // third backoff level for A 
    cond += SEP + htok.word();
    hash_access(_countsA, cond, Aden[2]);
    hash_access(_countsA, mnode_mtag + SEP + cond, Anum[2]);

      
    bool sib = (_use_siblings);
    int levels_A = 3;
    string snode = NULLTOK, sword; 
    if (sib) {
      if (cidx!=-1) {    
	const SSentence::Token& stok = x.get_token(cidx);
	snode = stok.spine_top();
	sword = stok.word();
      }
	
      levels_A++;	
      cond += SEP + snode;
      hash_access(_countsA, cond, Aden[3]);
      hash_access(_countsA, mnode_mtag + SEP + cond, Anum[3]);	 
	
      if (snode != NULLTOK) {
	levels_A++;
	cond += SEP + sword;
	hash_access(_countsA, cond, Aden[4]);
	hash_access(_countsA, mnode_mtag + SEP + cond, Anum[4]);
      }	
    }
    double pA = back_off_estimate(levels_A, Aden, Anum, 50);

    if (verbose) { 
      cerr << "["; print_counts(cerr, levels_A, Aden, Anum, "A"); cerr << " " << flush; 
    }
      
    // DISTRIBUTION B, generates the spine and type of attachment 
    int Bden[6], Bnum[6];
    // first backoff level for B
    cond = mnode_mtag;
    string r = mtok.spine() + SEP + (att_type);
    hash_access(_countsB, cond, Bden[0]);
    hash_access(_countsB, r + SEP + cond, Bnum[0]);

    // second backoff level
    cond += SEP + dsp;
    hash_access(_countsB, cond, Bden[1]);
    hash_access(_countsB, r + SEP + cond, Bnum[1]);

    // third backoff level
    cond += SEP + hnode;
    hash_access(_countsB, cond, Bden[2]);
    hash_access(_countsB, r + SEP + cond, Bnum[2]);

    // fourth backoff level
    cond += SEP + htok.word();
    hash_access(_countsB, cond, Bden[3]);
    hash_access(_countsB, r + SEP + cond, Bnum[3]);

    int levels_B = 4;
    if (sib) {
      levels_B++;	
      cond += SEP + snode;
      hash_access(_countsB, cond, Bden[4]);
      hash_access(_countsB, r + SEP + cond, Bnum[4]);
	
      if (snode != NULLTOK) {
	levels_B++;
	cond += SEP + sword;
	hash_access(_countsB, cond, Bden[5]);
	hash_access(_countsB, r + SEP + cond, Bnum[5]);
      }	
    }
    double pB = back_off_estimate(levels_B, Bden, Bnum, 50);

    if (verbose) {
      print_counts(cerr, levels_B, Bden, Bnum, "B"); cerr << " " << flush;
    }

    // DISTRIBUTION C, generates the word of the dependent node
    int Cden[3], Cnum[3];
    int levels_C = -1;
    double pC = -1;
    // if the word is unknown, the probability is 1
    if (mtok.unknown_word()) {
      pC = 1;
      levels_C = 1;   // these are set for verbose output
      Cden[0] = 1; Cnum[0] = 1;
    }
    else {
      r = mtok.word();
      levels_C = 2;
      // first backoff level for C
      cond = mtok.spine();
      hash_access(_countsC, cond, Cden[0]);
      hash_access(_countsC, r + SEP + cond, Cnum[0]);
      // additional smoothing for p(w | spine), otherwise it fails when pos tag for word is wrong 
      Cden[0]++;
      Cnum[0]++;

      // second backoff level
      cond += SEP + dir + SEP + htok.spine_nopos() + SEP + htok.word();
      hash_access(_countsC, cond, Cden[1]);
      hash_access(_countsC, r + SEP + cond, Cnum[1]);
      
      if (sib) {
	levels_C++;	
	if (snode == NULLTOK) {
	  cond += SEP + snode;
	}
	else {
	  cond += SEP + sword;
	}
	hash_access(_countsC, cond, Cden[2]);
	hash_access(_countsC, r + SEP + cond, Cnum[2]);       
      }
      pC = back_off_estimate(levels_C, Cden, Cnum, 50);
    }
    if (verbose) { 
      //cerr << "["; print_counts(cerr, levels_A, Aden, Anum, "A"); cerr << " "; 
      //      print_counts(cerr, levels_B, Bden, Bnum, "B"); cerr << " "; 
      print_counts(cerr, levels_C, Cden, Cnum, "C"); 
      cerr << " (" << r << " | " << cond << ") : "; 
      cerr <<  pA*pB*pC << "] " << flush;
    }

    double sco = _weight*log(pA*pB*pC) + _bonus;
    //      if (_cache_active) _cache_hash[cache_key] = _weight*log(pA*pB*pC) + _bonus;
    // if (_cache_active) _cache_hash[p] = sco;
    return sco;
  }


  double TAGLM::score_stop(const SSentence& x, 
			   bool pgoes_right, bool pfirst, const int& pheadpos, const int& hidx, const int& cidx, bool verbose) {
    string dir = (pgoes_right) ? "R" : "L";
    string state = (pfirst) ? "F+" : "F-";
    int att_pos = pheadpos;
    
    const SSentence::Token& htok = x.get_token(hidx);
    
    const string& pnode = htok.spine_node(pheadpos);
    string dsp = dir + SEP + state + SEP + pnode;
    
    // counts for distribution A
    int Aden[5], Anum[5];
    
    // first backoff level for A      
    hash_access(_countsA, dsp, Aden[0]);
    if (Aden[0] == 0) {
      if (verbose) { cerr << "[A0 0] " << flush; }
      //	if (_cache_active) _cache_hash[cache_key] = -HUGE_VAL;
      // if (_cache_active) _cache_hash[p] = -HUGE_VAL;
      return -HUGE_VAL;
    }
    hash_access(_countsA, "STOP" + SEP + dsp, Anum[0]);
    if (Anum[0] == 0) {
      if (verbose) { cerr << "[A0 " << Aden[0] << " 0] " << flush; }
      //	if (_cache_active) _cache_hash[cache_key] = -HUGE_VAL;
      // if (_cache_active) _cache_hash[p] = -HUGE_VAL;
      return -HUGE_VAL;
    }
    
    // second backoff level for A 
    const string& hnode = att_pos>0 ? htok.spine_node(att_pos-1) : htok.pos_tag();
    string cond = dsp + SEP + hnode;
    hash_access(_countsA, cond, Aden[1]);
    hash_access(_countsA, "STOP" + SEP + cond, Anum[1]);
    
    // third backoff level for A 
    cond += SEP + htok.word();
    hash_access(_countsA, cond, Aden[2]);
    hash_access(_countsA, "STOP" + SEP + cond, Anum[2]);
    
    int levels = 0;
    if (!_use_siblings) {
      levels = 3;
    }
    else {
      if (cidx == -1) {
	// third backoff level for A 
	levels = 4;
	cond += SEP + NULLTOK;
	hash_access(_countsA, cond, Aden[3]);
	hash_access(_countsA, "STOP" + SEP + cond, Anum[3]);	 
      }
      else {
	levels = 5;
	const SSentence::Token& stok = x.get_token(cidx);
	string snode = stok.spine_top(); 
	
	cond += SEP + snode;
	hash_access(_countsA, cond, Aden[3]);
	hash_access(_countsA, "STOP" + SEP + cond, Anum[3]);	 
	
	cond += SEP + stok.word();
	hash_access(_countsA, cond, Aden[4]);
	hash_access(_countsA, "STOP" + SEP + cond, Anum[4]);
      }
    }
    
    double pA = back_off_estimate(levels, Aden, Anum, 50);	
    if (verbose) { cerr << "["; print_counts(cerr, levels, Aden, Anum, "A"); cerr << " : " << pA << "] " << flush; }
    double sco = _weight*log(pA);
      //      if (_cache_active) _cache_hash[cache_key] = _weight*log(pA);
    // if (_cache_active) _cache_hash[p] = sco;
    return sco;
  }




  void TAGLM::read_events(istream& in, bool dumped) {   
    /////////
    if (dumped) return read_dumped_events(in); 
    /////////
    cerr << "TAGLM :: reading events " << flush;
    
    string event_type, dir, state, att_type, hword, htag, hspine, mword, mtag, mspine;
    string sword, stag, sspine;
    int position;

    string line;
    int i = 0;
    while (getline(in, line)) {
      //      cerr << line << endl;
      if (++i%100000==0) {
	cerr << "." << flush;
	if (i%1000000==0) 
	  cerr << "(" << (int) i/1000000 << "M)" << flush;
      }

      istringstream sline(line);
      sline >> event_type; 
      if (event_type == "D") {	
	/* 
	   Type Dir Pos State Type HWord HTag HSpine MWord MTag MSpine
	   0    1   2   3     4    5     6    7      8     9    10
	   D R 0 F+ S *R* *R* *R* Resumption NN NN+NPB+NP	
	*/
	sline >> dir;
	sline >> position;
	sline >> state;
	sline >> att_type;
	sline >> hword >> htag >> hspine;
	sline >> mword >> mtag >> mspine;
	if (_use_siblings) {
	  sline >> sword >> stag >> sspine;
	}
      }
      else if (event_type == "STOP") {
	/*
	STOP L 0 F+ Resumption NN NN+NPB+NP
	0    1 2 3  4          5  6  
	*/
	sline >> dir;
	sline >> position; 
	sline >> state;
	sline >> hword >> htag >> hspine;
	if (_use_siblings) {
	  sline >> sword >> stag >> sspine;
	}
      }
      else {

      }

      if (event_type == "D") {
	
	++_TOTAL_MOD;

	string hspine_nopos;
	vector<string> hvspine, mvspine; 
	SSentence::Token::split_spine(hspine, hspine_nopos, hvspine);
	SSentence::Token::split_spine(mspine, mvspine);

	string htag = hvspine[0];
	// shift one position to account for the postag
	string pnode = hvspine[position+1];
	string hnode = hvspine[position];

	string mtag = mvspine[0];
	string mnode = mvspine[mvspine.size()-1];

	string dir_state_pnode = dir + SEP + state + SEP + pnode;
	
	string snode; 
	if (_use_siblings) {
	  if (sword == NULLTOK) {
	    snode = NULLTOK;
	  }
	  vector<string> vsspine; 
	  SSentence::Token::split_spine(sspine, vsspine);
	  snode = vsspine[vsspine.size()-1];
	}

	// HASH A
	string r = mnode + SEP + mtag;
	string cond = dir_state_pnode;
	_countsA[cond]++;
	_countsA[r + SEP + cond]++;

	cond += SEP + hnode;
	_countsA[cond]++;
	_countsA[r + SEP + cond]++;

	if (hword != UNKNOWN) {
	  cond += SEP + hword;
	  _countsA[cond]++;
	  _countsA[r + SEP + cond]++;
	  
	  if (_use_siblings) {
	    cond += SEP + snode;
	    _countsA[cond]++;
	    _countsA[r + SEP + cond]++;
	    
	    if (snode != NULLTOK) {
	      cond += SEP + sword;
	      _countsA[cond]++;
	      _countsA[r + SEP + cond]++;
	    }
	  }
	}

	// HASH B
	cond = r; // i.e. mnode + mtag
	r = mspine + SEP + att_type;
	_countsB[cond]++; 
	_countsB[r + SEP + cond]++; 

	cond += SEP + dir_state_pnode;
	_countsB[cond]++; 
	_countsB[r + SEP + cond]++; 

	cond += SEP + hnode;
	_countsB[cond]++; 
	_countsB[r + SEP + cond]++; 

	if (hword != UNKNOWN) {
	  cond += SEP + hword;
	  _countsB[cond]++; 
	  _countsB[r + SEP + cond]++; 
	  
	  if (_use_siblings) {
	    cond += SEP + snode;
	    _countsB[cond]++;
	    _countsB[r + SEP + cond]++;
	    
	    if (snode!=NULLTOK) {
	      cond += SEP + sword;
	      _countsB[cond]++;
	      _countsB[r + SEP + cond]++;
	    }
	  }
	}

	//	if (mword != UNKNOWN) {

	bool mod_unknown = (mword==UNKNOWN);
	// HASH C
	r = mword;
	cond = mspine; 
	_countsC[cond]++;
	if (!mod_unknown) _countsC[r + SEP + cond]++;
	
	if (hword != UNKNOWN) {
	  cond += SEP + dir + SEP + hspine_nopos + SEP + hword;
	  _countsC[cond]++;
	  if (!mod_unknown) _countsC[r + SEP + cond]++;
	  
	  if (_use_siblings) {
	    if (snode == NULLTOK) {
	      cond += SEP + snode;
	    }
	    else {
	      cond += SEP + sword;
	    }
	    _countsC[cond]++;
	    if (!mod_unknown) _countsC[r + SEP + cond]++;
	  }
	}
      }
      else if (event_type == "STOP") {
	vector<string> hvspine; 
	SSentence::Token::split_spine(hspine, hvspine);

	string htag = hvspine[0];
	// shift one position to account for the postag
	string pnode = hvspine[position+1];
	string hnode = hvspine[position];

	string snode; 
	if (_use_siblings) {
	  if (sword == NULLTOK) {
	    snode = NULLTOK;
	  }
	  vector<string> vsspine; 
	  SSentence::Token::split_spine(sspine, vsspine);
	  snode = vsspine[vsspine.size()-1];
	}

	string cond = dir + SEP + state + SEP + pnode;
	_countsA[cond]++;
	_countsA["STOP" + SEP + cond]++;

	cond += SEP + hnode;
	_countsA[cond]++;
	_countsA["STOP" + SEP + cond]++;

	if (hword != UNKNOWN) {
	  cond += SEP + hword;
	  _countsA[cond]++;
	  _countsA["STOP" + SEP + cond]++;	
	  
	  if (_use_siblings) {
	    cond += SEP + snode;
	    _countsA[cond]++;
	    _countsA["STOP" + SEP + cond]++;
	    
	    if (snode != NULLTOK) {
	      cond += SEP + sword;
	      _countsA[cond]++;
	      _countsA["STOP" + SEP + cond]++;
	    }
	  }	
	}
      }
    }
    cerr << ", OK" << endl;
    print_size();    
  }

  void TAGLM::read_dumped_events(istream& in) {
    string aux, key;
    int count, n;
    cerr << "TAGLM :: reading dumped events ... " << flush;
    while (in >> aux) {
      if (aux == "#####") {
	// skip two fields
	in >> aux;
	in >> aux;
	cerr << " " << aux << flush;
      }
      else if (aux == "TOTAL_MOD") {
	in >> n;
	_TOTAL_MOD = n;
      }
      else if (aux == "BUCKETS") {
	in >> aux;
	in >> n;
        if (aux=="A") {
	  _countsA.resize(n);
	}
	else if (aux=="B") {
	  _countsB.resize(n);
	}
	else if (aux=="C") {
	  _countsC.resize(n);
	}
	else {
	  assert(0);
	}
      }
      else {
	in >> key;
	in >> count;
        if (aux=="A") {
	  _countsA[key] = count;
	}
	else if (aux=="B") {
	  _countsB[key] = count;
	}
	else if (aux=="C") {
	  _countsC[key] = count;
	}
	else {
	  assert(0);
	}
      }
    }
    cerr << " OK" << endl;
    print_size();
  }

  void TAGLM::print_size() const {
    cerr << "TAGLM :: hash sizes ::"
	 <<  " A " << _countsA.size() << " / " << _countsA.bucket_count() 
	 << "; B " << _countsB.size() << " / " << _countsB.bucket_count()
	 << "; C " << _countsC.size() << " / " << _countsC.bucket_count() 
      	 << "; TOTAL_MOD " << _TOTAL_MOD  << ";" << endl;
  }

  void TAGLM::dump_events(ostream& out) {
    out << "TOTAL_MOD " <<  _TOTAL_MOD << endl;
    out << "BUCKETS A " <<  _countsA.bucket_count() << endl;
    out << "BUCKETS B " <<  _countsB.bucket_count() << endl;
    out << "BUCKETS C " <<  _countsC.bucket_count() << endl;
    hash_map<string,int>::const_iterator it, it_end;        
    out << "##### COUNTS A" << endl;
    it = _countsA.begin();
    it_end = _countsA.end();    
    for (; it!=it_end; ++it) {
      out << "A " << it->first << " " << it->second << endl;
    }    
    out << "##### COUNTS B" << endl;
    it = _countsB.begin();
    it_end = _countsB.end();    
    for (; it!=it_end; ++it) {
      out << "B " << it->first << " " << it->second << endl;
    }
    out << "##### COUNTS C" << endl;
    it = _countsC.begin();
    it_end = _countsC.end();    
    for (; it!=it_end; ++it) {
      out << "C " << it->first << " " << it->second << endl;
    }
  }
      
}
