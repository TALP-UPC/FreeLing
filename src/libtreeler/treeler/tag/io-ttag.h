
#ifndef TAG_IOTTAG
#define TAG_IOTTAG

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>


#include "treeler/base/label.h"
#include "treeler/base/dataset.h"
#include "treeler/base/dictionary.h"

#include "treeler/tag/tuple-seq.h"
#include "treeler/tag/part-tag.h"
#include "treeler/tag/tag-seq.h"

#include "treeler/util/options.h"
#include "treeler/io/io-conll.h"


using namespace std;

namespace treeler {

  CoNLLStream& operator<<(CoNLLStream& strm, const Label<PartTag>& y); 
  CoNLLStream& operator<<(CoNLLStream& strm, const TupleSeq& x); 


  /**
   * \brief Input-output routines for tagging data in CoNLL format
   * \ingroup tag
   *
   */
  class IOTTag {
  public:
    struct Configuration {
      int num_special_symbols; 

    public:
      Configuration() : num_special_symbols(0) {}
    };

  private:
    static Configuration _config;

  public:
    static void usage() {
      cerr << "IOTTag::usage() : done" << endl; 
    };
    
    static void process_options(Options& options);


    static bool read(istream& iss, TupleSeq& s);

    static bool read(istream& iss, TupleSeq*& x, Label<treeler::PartTag>*& y) {
      TupleSeq* tmp = new TupleSeq; 
      if (read(iss, *tmp)) {
	x = tmp;
	y = new Label<PartTag>();
	PartTag::extract_parts(x->y(), *y); 
	return true;
      }
      else {
	delete tmp;
	return false;
      } 
    }

    template <typename X, typename R>
    static void read_dataset(const std::string& file, DataSet<X,R>& data);

    template <typename X, typename R>
    static void read_dataset(istream& in, DataSet<X,R>& data, bool quiet);

    //    static void write(IOCoNLL0::block<string>& b, const TupleSeq& x); 
    //    static void write(IOCoNLL0::block<string>& b, const Label<PartTag>& y); 

    static void write(ostream& o, const TupleSeq& x) {      
      CoNLLStream strm; 
      strm.prefix = "X"; 
      strm.add_padding = 1; 
      strm << x; 
      o << strm; 
    }

    static void write(ostream& o, const Label<PartTag>& y) {
      CoNLLStream strm; 
      strm.prefix = "Y"; 
      strm << y; 
      o << strm; 
    }

    static void write(ostream& o, const TupleSeq& x, const Label<PartTag>& y) {
      //      IOCoNLL0::block<string> b; 
      //      write(b,x); 
      //      write(b, y); 
      //      IOCoNLL0::add_padding(b); 
      //      IOCoNLL0::write_block(o, b); 
      CoNLLStream strm; 
      strm.prefix = ""; 
      strm.add_padding = 1; 
      strm << x << y; 
      o << strm; 
    }

    static void write(ostream& o, const TupleSeq& x, const Label<PartTag>& y1, const Label<PartTag>& y2) {
      CoNLLStream strm; 
      strm.prefix = ""; 
      strm.add_padding = 1; 
      strm << x << y1 << y2; 
      o << strm; 
      // IOCoNLL0::block<string> b; 
      /* write(b,x);  */
      /* write(b, y1);  */
      /* write(b, y2);  */
      /* IOCoNLL0::add_padding(b);  */
      /* IOCoNLL0::write_block(o, b);  */
    }

    static void write(ostream& o, const TupleSeq& x, const Label<PartTag>& y1, const Label<PartTag>& y2, const Label<PartTag>& y3) {
      CoNLLStream strm; 
      strm.prefix = ""; 
      strm.add_padding = 1; 
      strm << x << y1 << y2 << y3; 
      o << strm; 
      /* IOCoNLL0::block<string> b;  */
      /* write(b,x);  */
      /* write(b, y1);  */
      /* write(b, y2);  */
      /* write(b, y3);  */
      /* IOCoNLL0::add_padding(b);  */
      /* IOCoNLL0::write_block(o, b);  */
    }

      
  };

  template <typename X, typename R>
  void IOTTag::read_dataset(const std::string& file, DataSet<X,R>& data) {
    assert(data.size() == 0);
    cerr << "IOTTag : reading "  << (file=="-" ? "STDIN" : file) << " " << flush;
    istream* in = &std::cin; 
    ifstream fin; 
    if (file != "-") {
      fin.open(file.c_str(), ifstream::in);
      if (!fin.good()) {
	cerr << endl << "IOTTag : error opening file " << file << endl; 
	exit(1);
      }
      in = &fin;
    }     
    bool quiet = false;
    read_dataset(*in, data, quiet);
    if (file!="-") {
      fin.close();
    }
    cerr << " " << data.size() << " examples" << endl;
  }
  
  template <typename X, typename R>
  void IOTTag::read_dataset(istream& in, DataSet<X,R>& data, bool quiet) {
    int id = 0;
    assert(data.size() == 0);    
    X* x = NULL;
    Label<R>* y = NULL;
    while(read(in, x, y)) {
      x->set_id(id++); // incrementing id
      data.push_back(new Example<X,R>(x,y));
      x = NULL;
      y = NULL;
      if(!quiet and (id & 0x7ff) == 0) {
	cerr << "(" << (double)id/1000.0 << "k)" << flush;
      } else if((id & 0xff) == 0) {
	cerr << "." << flush;
      }
    }
  }


};
#endif
