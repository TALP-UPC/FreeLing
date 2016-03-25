

#include "treeler/tag/io-tag.h"

namespace treeler {

  CoNLLStream& operator<<(CoNLLStream& strm, const TagSeq& y) {
    const vector<int>& t = y; 
    return strm << t;
  }

  CoNLLStream& operator<<(CoNLLStream& strm, const Label<PartTag>& y) {
    int n = y.size(); 
    vector<int> t(n, -1); 
    for (auto r = y.begin(); r!=y.end(); ++r) {
      if (r->i>=0 and r->i<n) {
	t[r->i] = r->b; 
      }
    }    
    return strm << t;
  }


  CoNLLStream& operator>>(CoNLLStream& strm, TagSeq& y) {
    vector<int>& t = y; 
    return strm >> t;
  }

}


