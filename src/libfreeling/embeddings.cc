
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <algorithm>

#include "freeling/morfo/embeddings.h"

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"EMBEDDINGS"
#define MOD_TRACECODE EMBEDDINGS_TRACE

  /// ---------------------------------------------------- 
  ///  Class norm_vector stores a vector and its module
  /// ---------------------------------------------------- 


  ///////////////////////////////////////////////////////////////
  /// create empty vector
  ///////////////////////////////////////////////////////////////

  norm_vector::norm_vector() : vector<float>() { mod=0.0; }

  ///////////////////////////////////////////////////////////////
  /// create vector of given size, initialized to zero
  ///////////////////////////////////////////////////////////////

  norm_vector::norm_vector(unsigned int sz) : vector<float>(sz, 0.0) {
    mod = 0.0;  
  }

  ///////////////////////////////////////////////////////////////
  /// create norm vector from given data 
  ///////////////////////////////////////////////////////////////

  norm_vector::norm_vector(const vector<float> &v) : vector<float>(v.size()) {
    copy(v.begin(), v.end(), this->begin());
    compute_norm();
  }

  ///////////////////////////////////////////////////////////////
  /// destructor
  ///////////////////////////////////////////////////////////////

  norm_vector::~norm_vector() {};

  ///////////////////////////////////////////////////////////////
  // return vector norm
  ///////////////////////////////////////////////////////////////

  float norm_vector::get_norm() const { return mod; }

  ///////////////////////////////////////////////////////////////
  // Normalize vector
  ///////////////////////////////////////////////////////////////

  void norm_vector::normalize() {
    for (unsigned int i=0; i<this->size(); ++i) 
      (*this)[i] /= mod;
    mod = 1.0;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Cos similarity between vectors
  /////////////////////////////////////////////////////////////////////////////

  float norm_vector::cos_similarity(const norm_vector &v1, const norm_vector &v2) {

    if (v1.empty() or v2.empty()) return -1.0;
    // cos similarity = (A·B) / (|A|·|B|) 
    //                = sum(A_i·B_i) / sqrt(sum(A_i^2))·sqrt(sum(B_i^2))
    float sum = 0.0;
    for (unsigned int i= 0; i<v1.size(); ++i) 
      sum += v1[i]*v2[i];
        
    return sum/(v1.mod*v2.mod);
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Vector add operation
  /////////////////////////////////////////////////////////////////////////////

  norm_vector norm_vector::operator+(const norm_vector &v2) const {
    norm_vector vr(this->size());
    for (unsigned int i = 0; i < this->size(); ++i) 
      vr[i] = (*this)[i] + v2[i];
    // compute and store module of new vector
    vr.compute_norm();
    return vr;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Vector substraction operator
  /////////////////////////////////////////////////////////////////////////////

  norm_vector norm_vector::operator-(const norm_vector &v2) const {
    norm_vector vr(this->size());
    for (unsigned int i = 0; i < this->size(); ++i) 
      vr[i] = (*this)[i] - v2[i];
    // compute and store module of new vector
    vr.compute_norm();
    return vr;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Vector multiplication by a float
  /////////////////////////////////////////////////////////////////////////////

  norm_vector norm_vector::operator*(float f) const {
    norm_vector vr(this->size());
    for (unsigned int i = 0; i < this->size(); ++i) 
      vr[i] = (*this)[i] * f;
    // compute and store module of new vector
    vr.compute_norm();
    return vr;
  }

  ///////////////////////////////////////////////////////////////
  // (private) update vector module with current values
  ///////////////////////////////////////////////////////////////

  void norm_vector::compute_norm() {
    mod = 0.0;
    for (unsigned int i=0; i<this->size(); ++i) 
      mod += (*this)[i] * (*this)[i];
    mod = sqrt(mod);
  }

  /// ---------------------------------------------------- 
  ///  Class embeddigns stores a word embeddings model
  /// ---------------------------------------------------- 

  ///////////////////////////////////////////////////////////////
  /// Constructor
  ///////////////////////////////////////////////////////////////

  embeddings::embeddings(const wstring &modelPath) {

    // read binary or text model, depending on file extension
    if (modelPath.substr(modelPath.length()-4) == L".bin" ) 
      load_binary_model(modelPath);
    else 
      load_text_model(modelPath);

    TRACE(2, L"embeddings module succesfully created");
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Destructor
  /////////////////////////////////////////////////////////////////////////////

  embeddings::~embeddings() {}


  /////////////////////////////////////////////////////////////////////////////
  /// auxiliary to load_binary_model.
  /// Read a string from a binary ifstream until a whitespace is found
  /////////////////////////////////////////////////////////////////////////////

  string embeddings::read_string(ifstream &fmod) const {
    string s = "";
    char c = fmod.get();
    while (c!=' ' and c!='\n') {
      s.push_back(c);
      c = fmod.get();
    }
    return s;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Load model in binary format from given file
  /////////////////////////////////////////////////////////////////////////////

  void embeddings::load_binary_model(const wstring &path) {

    // open file
    string fname=util::wstring2string(path);
    ifstream fmod;

    fmod.open(fname.c_str(), ios_base::binary|ios_base::in);
    if (fmod.fail()) {
      ERROR_CRASH(L"Error opening binary model file " + path + L" for reading");
    }

    // read file header with vocabulary and vector sizes
    vocabSize = util::wstring_to<unsigned int>(util::string2wstring(read_string(fmod)));
    dimensionality = util::wstring_to<unsigned int>(util::string2wstring(read_string(fmod)));

    // read each word and associated vector
    for (unsigned int i=0; i<vocabSize; ++i) {
      // read word
      string word = read_string(fmod);
      // read vector
      vector<float> v(dimensionality);
      fmod.read((char*)v.data(), dimensionality*sizeof(float));
      fmod.get(); // consume newline

      // store pair (word,vector) in map
      wordVec.insert(make_pair(util::string2wstring(word), norm_vector(v)));
    }
    // close file
    fmod.close();
  }
    

  /////////////////////////////////////////////////////////////////////////////
  /// Load model in text format from given file
  /////////////////////////////////////////////////////////////////////////////

  void embeddings::load_text_model(const wstring &fname) {

    // open file
    wifstream fmod;
    util::open_utf8_file(fmod, fname);
    if (fmod.fail()) {
      ERROR_CRASH(L"Error opening text model file " + fname + L" for reading");
    }
    // read file header with vocabulary and vector sizes
    fmod >> vocabSize >> dimensionality;

    // read each word and associated vector
    for (unsigned int i=0; i<vocabSize; ++i) {
      // read word
      wstring w;
      fmod >> w;
      // read vector
      vector<float> v(dimensionality);
      for (unsigned int j=0; i<dimensionality; ++j) {
        fmod >> v[j];
      }
      // store pair (word,vector) in map
      wordVec.insert(make_pair(w, norm_vector(v)));
    }

    // close file
    fmod.close();
  }

  
  /////////////////////////////////////////////////////////////////////////////
  /// Get vectors dimensionality
  /////////////////////////////////////////////////////////////////////////////

  unsigned int embeddings::get_dimensionality() const {return dimensionality;}
  
    
  /////////////////////////////////////////////////////////////////////////////
  /// Get vocabulary size
  /////////////////////////////////////////////////////////////////////////////

  unsigned int embeddings::get_vocab_size() const {return vocabSize;}

  
  /////////////////////////////////////////////////////////////////////////////
  /// Get vocabulary as a list
  /////////////////////////////////////////////////////////////////////////////

  list<wstring> embeddings::get_vocab() const {
    list<wstring> vocab_list;
    for (auto const &element : wordVec)
      vocab_list.push_back(element.first);
    return vocab_list;
  }
    
  /////////////////////////////////////////////////////////////////////////////
  /// Find if word is in model
  /////////////////////////////////////////////////////////////////////////////

  bool embeddings::word_in_model(const wstring &word) const {
    return wordVec.find(word) != wordVec.end();
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Get the vector representing a word. 
  /// If word is not in the model an empty vector is returned
  /////////////////////////////////////////////////////////////////////////////

  const norm_vector& embeddings::get_vector(const wstring &word) const {
    unordered_map<wstring,norm_vector>::const_iterator p = wordVec.find(word);
    if (p != wordVec.end()) return p->second;
    else return empty_vector;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Cos similarity between two words, returns -1 if words not found in model
  /////////////////////////////////////////////////////////////////////////////

  float embeddings::cos_similarity(const wstring &word1, const wstring &word2) const {
    // check if words exist
    norm_vector v1 = get_vector(word1);
    norm_vector v2 = get_vector(word2);
    if (v1.empty() or v2.empty()) return -1.0;

    return norm_vector::cos_similarity(v1,v2);
  }
      
  /////////////////////////////////////////////////////////////////////////////
  /// Gets the N closest words to the given word (this operation is O(N) on the size of the vocabulary)
  /////////////////////////////////////////////////////////////////////////////

  list<pair<wstring,float> > embeddings::similar_words(const wstring &word, unsigned int num_words) const {
    list<pair<wstring,float> > result_list;
    norm_vector v1 = get_vector(word);
    if (not v1.empty()) {
      result_list = similar_words(v1, num_words+1);
      result_list.pop_front(); // remove "word" from list
    }
    return result_list;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Gets the N closest words to the given vector (this operation is O(N) on the size of the vocabulary)
  /////////////////////////////////////////////////////////////////////////////

  list<pair<wstring,float> > embeddings::similar_words(const norm_vector &vector, unsigned int num_words) const {
    
    // extreme cases
    if (num_words==0) return list<pair<wstring,float> >();
    else if (num_words>=vocabSize) num_words = vocabSize-1;
    
    // init aux array
    std::vector<pair<wstring,float> > similars(num_words, make_pair(L"-",-1));
    
    // search against all words
    for (auto it : wordVec) {
      // add the new word to similar_words array if similarity is high enough
      float sim = norm_vector::cos_similarity(vector, it.second);
      if (sim > similars[num_words-1].second) {
        similars[num_words-1].first = it.first;
        similars[num_words-1].second = sim;
        
        // keep list of similar words sorted
        unsigned int i = num_words-1;
        while (i>0 and sim>similars[i-1].second) {
          std::swap(similars[i-1],similars[i]);
          --i;
        }
      }
    }
    
    list<pair<wstring,float> > res(similars.begin(), similars.end());
    return res;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Returns the closest word in the model to the given vector (uses similar_words function)
  /////////////////////////////////////////////////////////////////////////////

  wstring embeddings::closest_word(const norm_vector &vector) const {
    return similar_words(vector, 1).front().first;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Returns Y in the analogy "A is to B what X is to Y". Words must be in the model
  /////////////////////////////////////////////////////////////////////////////

  wstring embeddings::analogy(const wstring &A, const wstring &B, const wstring &X) const {
    return closest_word(analogy_vec(A,B,X));
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Returns the Y vector representation in the analogy "A is to B what X is to Y". 
  /// Words must be in the model
  /////////////////////////////////////////////////////////////////////////////

  norm_vector embeddings::analogy_vec(const wstring &A, const wstring &B, const wstring &X) const {
    norm_vector vA = get_vector(A);
    norm_vector vB = get_vector(B);
    norm_vector vX = get_vector(X);
    return vB - vA + vX;
  }
  
  
  /////////////////////////////////////////////////////////////////////////////
  /// Dumps the model in binary format to the specified file. 
  /////////////////////////////////////////////////////////////////////////////

  void embeddings::dump_binary_model(const std::wstring &path) const {
      
    // open file
    string fname=util::wstring2string(path);
    ofstream fmod;
    fmod.open(fname.c_str(), ios_base::binary|ios_base::out);
    if (fmod.fail()) {
      ERROR_CRASH(L"Error opening model file " + path + L" for binary model dump")
    }

    // print file header with vocabulary and vector sizes
    string s = util::wstring2string(util::int2wstring(vocabSize) + L" " + util::int2wstring(dimensionality) + L"\n");

    fmod.write(s.c_str(), s.length());

    // print each word and associated vector
    for (auto const& element : wordVec) {
      s = freeling::util::wstring2string(element.first) + " ";
      fmod.write(s.c_str(), s.length());
      fmod.write((char *)element.second.data(), dimensionality*sizeof(float));
      fmod.write("\n", sizeof(char));
    }
    
    // close file
    fmod.close();
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Dumps the model in text format to the specified file. 
  /////////////////////////////////////////////////////////////////////////////

  void embeddings::dump_text_model(const std::wstring &path) const {
    // open file
    wofstream fmod;
    util::open_utf8_file(fmod, path);
    if (fmod.fail()) {
      ERROR_CRASH(L"Error opening model file " + path + L" for text model dump");
    }
    // print file header with vocabulary and vector sizes
    fmod << vocabSize << L" " << dimensionality << endl;
    // print each word and associated vector
    for (auto const& element : wordVec) {
      fmod << element.first;
      for (unsigned int i=0; i<dimensionality; ++i) 
        fmod << L" " << element.second[i];
      fmod << endl;
    }
    // close file
    fmod.close();
  }
    
} // namespace
