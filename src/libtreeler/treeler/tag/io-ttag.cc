
#include "treeler/tag/io-ttag.h"

namespace treeler {

  CoNLLStream& operator<<(CoNLLStream& s, const TupleSeq& x) {
    int n = x.size();
    int m = s.num_columns(); 
    if (m==0) {
      //      c.add_ids(n);
      //      m++;
    }
    int ext = 0; 
    // Options::get("io-ext", ext); 
    if (ext) {
      s.A.resize(m+x.dim()*2, CoNLLColumn(n));
      for (int i=0; i<n; ++i) {	  
	for (int d=0; d<x.dim(); ++d) {
	  s.A[m+2*d][i] = x.field(i,d);
	}
	const TupleSeq::Tuple& t = x.tuple(i); 
	for (int d=0; d<x.dim(); ++d) {
	  ostringstream oss; 
	  oss << t[d] << ":" << x.symbolsF[d].map(t[d]); 
	  s.A[m+2*d+1][i] = oss.str();
	}
      }
    }
    else {
      s.A.resize(m+x.dim(), CoNLLColumn(n));
      for (int i=0; i<n; ++i) {
	for (int d=0; d<x.dim(); ++d) {
	  s.A[m+d][i] = x.field(i,d);
	}
      }
    }
    return s; 
  }


  CoNLLStream& operator<<(CoNLLStream& s, const Label<PartTag>& y) {

    size_t n = s.num_rows();
    if (n==0) n=y.size();
    s.A.emplace_back( CoNLLColumn(n) );
    Label<PartTag>::const_iterator it = y.begin();
    const Label<PartTag>::const_iterator it_end = y.end();
    CoNLLColumn::iterator dest = s.A.back().begin();	 
    for (; it != it_end; ++it, ++dest) {
      stringstream ss; 
      ss << TupleSeq::symbolsY.map(it->b); 
      ss >> *dest;
    }
    return s;
  }


  IOTTag::Configuration IOTTag::_config;

  void IOTTag::process_options(Options& options) {
    cerr << "IOTTag::process_options() : " << endl; 
    int D; 
    if (!options.get("D", D)) {
      cerr << "IOTTag : please specify number of input dimensions --D=<int>" << endl; 
      exit(0); 
    }
    string dir; 
    if (!options.get("dir", dir)) {
      cerr << "IOTTag : please specify directory of dictionaries with --dir=<path>" << endl; 
      exit(0); 
    }
    TupleSeq::load_dictionaries(D, dir);  

    _config.num_special_symbols = 2;
    options.get("io-nBE", _config.num_special_symbols);

    cerr << "IOTTag::process_options() : done" << endl; 
  }; 
  

  // void IOTTag::write(IOCoNLL0::block<string>& b, const TupleSeq& x) {
  //   int ext = 0; 
  //   options.get("io-ext", ext); 
  //   if (ext) {
  //     b.resize(x.dim()*2, IOCoNLL0::column<string>(x.size())); 
  //     for (int i=0; i<x.size(); ++i) {	  
  // 	for (int d=0; d<x.dim(); ++d) {
  // 	  b[2*d][i] = x.field(i,d);
  // 	}
  // 	const TupleSeq::Tuple& t = x.tuple(i); 
  // 	for (int d=0; d<x.dim(); ++d) {
  // 	  ostringstream oss; 
  // 	  oss << t[d] << ":" << x.symbolsF[d].map(t[d]); 
  // 	  b[2*d+1][i] = oss.str();
  // 	}
  //     }
  //   }
  //   else {
  //     b.resize(x.dim(), IOCoNLL0::column<string>(x.size())); 
  //     for (int i=0; i<x.size(); ++i) {
  // 	for (int d=0; d<x.dim(); ++d) {
  // 	  b[d][i] = x.field(i,d);
  // 	}
  //     }
  //   }
  // }

  // void IOTTag::write(IOCoNLL0::block<string>& b, const Label<PartTag>& y) {
  //   int ext = 0; 
  //   options.get("io-ext", ext); 

  //   int m = b.size();
  //   int n = (m==0) ? 0 : b[0].size();
  //   b.resize(m+1+ext, IOCoNLL0::column<string>(n,"_")); 
  //   IOCoNLL0::column<string>& tags = b[m];
  //   Label<PartTag>::const_iterator r=y.begin(), rend=y.end(); 
  //   while (r!=rend) {
  //     if (r->i >= n) {
  // 	n = r->i +1; 
  // 	tags.resize(n, "_");
  // 	if (ext) b[m+1].resize(n, "_");
  //     }
  //     tags[r->i] = TupleSeq::symbolsY.map(r->b); 
  //     if (ext) {
  // 	ostringstream oss; 
  // 	oss << r->b; 
  // 	b[m+1][r->i] = oss.str(); 
  //     }
  //     ++r;
  //   }
  // }


  bool IOTTag::read(istream& iss, TupleSeq& s) {
      if (!s._y.empty()) {
	return false; 
      }
      s._dim = s.dim();
      string line; 
      int n = 0;
      while (getline(iss, line)) {
	++n; 
	istringstream ssline(line); 
	vector<string> row; 
	string tmp; 
	while (ssline >> tmp) {
	  row.push_back(tmp); 
	}
	if (row.empty()) break;
	if (row.size()!=(s._dim+1)) {
	  cerr << "scrf/sentence: error in line " << n << ", wrong number of fields (\"" << line << "\")" << endl;
	  exit(0); 
	}
	
	// add special start token(s)
	if (n==1) {
	  for (int k=0; k<_config.num_special_symbols; ++k) {
	    s._tuples.emplace_back(s._dim);	    
	    TupleSeq::Tuple& t = s._tuples.back();
	    for (size_t d=0; d<s._dim; ++d) {
	      s._fields.push_back("__B__");
	      int v = s.symbolsF[d].map("__B__");
	      assert(v!=s.symbolsF[d].id_unknowns());
	      t[d] = v;
	    }
	    int y = s.symbolsY.map("__B__");
	    s._y.push_back(y);
	  }
	}
	
	s._tuples.emplace_back(s._dim);	    
	TupleSeq::Tuple& t = s._tuples.back();
	for (size_t d=0; d<s._dim; ++d) {
	  s._fields.push_back(row[d]);
	  int v = s.symbolsF[d].map(row[d]); 
	  if (v==s.symbolsF[d].id_unknowns()) {
	    t[d] = -1; 
	  }
	  else {
	    t[d] = v;
	  }
	}
	int y = s.symbolsY.map(row[s._dim]);
	s._y.push_back(y);
      }
      
      // add special end token(s)
      {
	for (int k=0; k<_config.num_special_symbols; ++k) {
	  s._tuples.emplace_back(s._dim);	    
	  TupleSeq::Tuple& t = s._tuples.back();
	  for (size_t d=0; d<s._dim; ++d) {
	    s._fields.push_back("__E__");
	    int v = s.symbolsF[d].map("__E__");
	    assert(v!=s.symbolsF[d].id_unknowns());
	    t[d] = v;
	  }
	  int y = s.symbolsY.map("__E__");
	  s._y.push_back(y);
	}
      }
      
      return (n!=0); 
    }


};
