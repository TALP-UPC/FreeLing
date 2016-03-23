
#ifndef TAG_TUPLESEQ
#define TAG_TUPLESEQ

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>

#include "treeler/base/label.h"
#include "treeler/tag/part-tag.h"
#include "treeler/tag/tag-seq.h"
#include "treeler/base/dictionary.h"
#include "treeler/util/options.h"

using namespace std;

namespace treeler {

  class TupleSeq {
    friend class IOTTag;
    public:
    typedef vector<int> Tuple;

  public:
      static vector<Dictionary> symbolsF;
      static Dictionary symbolsY;

  private:
      int    _id; 
      size_t _dim; 
      vector<string> _fields;
      vector<Tuple>  _tuples;
      TagSeq _y;

    public:
      TupleSeq() {};

      void clear() {
	_fields.clear();
	_tuples.clear();
	_y.clear();
      }
      
      static void load_dictionaries(int D, const string& dir); 
      static int dim() { return (int) symbolsF.size(); }

      int id() const { return _id; }
      void set_id(int i) { _id = i;  }

      int size() const { return _y.size(); }
      const string& field(int i, int d) const { return _fields[i*_dim + d]; };
      const Tuple& tuple(int i) const { return _tuples[i]; };

      const int tag(int i) const { return _y[i]; };
      const TagSeq& y() const { return _y; };
  };
}


#endif 
