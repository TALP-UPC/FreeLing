
#ifndef _EMBEDDINGS
#define _EMBEDDINGS

#include <vector>
#include <list>
#include <unordered_map>

#include "freeling/windll.h"

namespace freeling {


  class norm_vector : public std::vector<float> {
  public:
    /// create empty vector
    norm_vector();
    /// create vector of given size, initialized to zero
    norm_vector(unsigned int sz);
    /// create norm vector from given data 
    norm_vector(const std::vector<float> &v);
    /// destructor
    ~norm_vector();

    /// get vector norm
    float get_norm() const ;
    /// normalize vector
    void normalize();

    /// compute cosine similarity between given vectors
    static float cos_similarity(const norm_vector &v1, const norm_vector &v2); 
    /// vector addition
    norm_vector operator+(const norm_vector &v2) const;
    /// vector substraction
    norm_vector operator-(const norm_vector &v2) const;
    /// vector * float
    norm_vector operator*(float f) const;

  private:
    /// update vector module with current values
    void compute_norm();
    /// vector module, computed once
    float mod;    
  };

  ////////////////////////////////////////////////////////////////
  ///  Class embeddings implements an interface for word 
  ///  embeddings based on the original word2Vec implementation.
  ////////////////////////////////////////////////////////////////

  class WINDLL embeddings {
  private:
    /// size of the model vocabulary
    unsigned int vocabSize;
    /// dimensionality of the word embeddings vectors
    unsigned int dimensionality;
    /// word embeddings vectors
    std::unordered_map<std::wstring, norm_vector> wordVec;
    /// empty vector, to use as wildcard
    norm_vector empty_vector;

    // auxiliary for load_binary_model
    std::string read_string(std::ifstream &fmod) const;
    
  public:
    /// Constructor
    embeddings(const std::wstring &modelPath);
    /// Destructor
    ~embeddings();
    
    //-------------------------------
    // Read/Write model
    //-------------------------------
    
    /// Dumps the model in the specified file. 
    void dump_binary_model(const std::wstring &path) const;
    /// Dumps the model in the specified file. 
    void dump_text_model(const std::wstring &path) const;
    /// Loads model from specified file. 
    void load_binary_model(const std::wstring &fname);
    /// Loads model from specified file. 
    void load_text_model(const std::wstring &fname);
    /// Loads model from specified file. 
    void load_gzip_model(const std::wstring &fname);
    
    //-------------------------------
    // Model properties
    //-------------------------------
    
    /// Get vocabulary size
    unsigned int get_vocab_size() const;
    
    /// Get vocabulary as a list
    std::list<std::wstring> get_vocab() const;
    
    /// Get vectors dimensionality
    unsigned int get_dimensionality() const;
    
    //-------------------------------
    // Word functions
    //-------------------------------
    
    /// Find if word is in model
    bool word_in_model(const std::wstring &word) const;
    
    /// Get the vector representing a word. If word is not in the model, NULL is returned
    const norm_vector& get_vector(const std::wstring &word) const;
    const std::vector<float>& get_base_vector(const std::wstring &word) const;

    /// Cos similarity between two words, returns -1 if words not found in model
    float cos_similarity(const std::wstring &word1, const std::wstring &word2) const;
        
    /// Gets the N closest words to the given word (this operation is O(N) on the size of the vocabulary)
    std::list<std::pair<std::wstring,float> > similar_words(const std::wstring &word, unsigned int num_words = 10) const;
    
    /// Gets the N closest words to the given vector (this operation is O(N) on the size of the vocabulary)
    std::list<std::pair<std::wstring,float> > similar_words(const norm_vector &v, unsigned int num_words = 10) const;
    
    /// Returns the closest word in the model to the given vector (uses similar_words function)
    std::wstring closest_word(const norm_vector &v) const;
    
    /// Returns Y in the analogy "A is to B what X is to Y". Words must be in the model
    std::wstring analogy(const std::wstring &A, const std::wstring &B, const std::wstring &X) const;
    
    /// Returns the Y vector representation in the analogy "A is to B what X is to Y". Words must be in the model
    norm_vector analogy_vec(const std::wstring &A, const std::wstring &B, const std::wstring &X) const;
    
  };
} // namespace

#endif
