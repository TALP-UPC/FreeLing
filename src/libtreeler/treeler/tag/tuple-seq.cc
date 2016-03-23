
#include "treeler/tag/tuple-seq.h"

#include <sstream>
#include <iostream>

using namespace std; 


namespace treeler {

  vector<Dictionary> TupleSeq::symbolsF; 
  Dictionary TupleSeq::symbolsY; 

  void TupleSeq::load_dictionaries(int D, const string& dir) {
    symbolsF.resize(D);    
    for (int d=0; d<D; ++d) {
      ostringstream oss; 
      oss << dir << "/fsymbols." << d << ".txt"; 
      symbolsF[d].load(oss.str());
    }    
    string dictyfile = dir + "/osymbols.txt";
    symbolsY.load(dictyfile);        
  }


}
