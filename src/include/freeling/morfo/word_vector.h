#ifndef _WORD_VECTOR
#define _WORD_VECTOR

#include "freeling/morfo/language.h"
#include <unordered_map>

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class word_vector implements an interface for word 
  ///  embeddings based on the original word2Vec implementation.
  ////////////////////////////////////////////////////////////////

  class word_vector {
  private:
    // constants
    static const unsigned int MAX_STR_SIZE = 2000;
    
    // language of the model
    std::wstring lang;
    
    // size of the model vocabulary
    unsigned long long int vocabSize;
    
    // dimensionality of the word embeddings vectors
    unsigned long long int dimensionality;
    
    // word embeddings vectors
    std::unordered_map<std::wstring, float*> wordVec;
    float* model_memory;
    
    //-------------------------------
    // Aux functions
    //-------------------------------
    
    bool wstr_ends_with(std::wstring wstr, std::wstring end);
    
  public:
    /// Constructor
    word_vector(const std::wstring &modelPath, std::wstring language = L"??");
    /// Destructor
    ~word_vector();
    
    //-------------------------------
    // Read/Write model
    //-------------------------------
    
    /// Read the word embeddings model
    void read_model(const std::wstring &path);
    
    /// Dumps the model in the specified file. If the file name ends with ".bin", the dump will be in binary format
    void dump_model(const std::wstring &path);
    
    //-------------------------------
    // Model properties
    //-------------------------------
    
    /// Get vocabulary size
    unsigned int get_vocab_size();
    
    /// Get vocabulary as a list
    std::list<std::wstring> get_vocab();
    
    /// Get vectors dimensionality
    unsigned int get_dimensionality();
    
    /// Get model language
    std::wstring get_model_language();
    
    //-------------------------------
    // Word functions
    //-------------------------------
    
    /// Find if word is in model
    bool word_in_model(std::wstring word);
    
    /// Get the vector representing a word. If word is not in the model, NULL is returned
    float* get_vector(std::wstring word);
    
    /// Cos similarity between two words, returns -1 if words not found in model
    float cos_similarity(std::wstring word1, std::wstring word2);
    
    /// Cos similarity between word vectors
    float cos_similarity(float* vec1, float* vec2);
    
    /// Gets the N closest words to the given word (this operation is O(N) on the size of the vocabulary)
    std::list<std::wstring> similar_words(std::wstring word, unsigned int num_words = 10);
    
    /// Gets the N closest words to the given vector (this operation is O(N) on the size of the vocabulary)
    std::list<std::wstring> similar_words(float* vector, unsigned int num_words = 10);
    
    /// Returns the closest word in the model to the given vector (uses similar_words function)
    std::wstring closest_word(float* vector);
    
    /// Returns Y in the analogy "A is to B what X is to Y". Words must be in the model
    std::wstring analogy(std::wstring A, std::wstring B, std::wstring X);
    
    /// Returns the Y vector representation in the analogy "A is to B what X is to Y". Words must be in the model
    float* analogy_vec(std::wstring A, std::wstring B, std::wstring X);
    
    //-------------------------------
    // Vector operations
    //-------------------------------
    
    /// Add vectors
    float* add_vectors(float* vec1, float* vec2);
    
    /// Subtract vectors
    float* sub_vectors(float* vec1, float* vec2);
    
    /// Add vector2 to vector1
    void add_vector(float* vec1, float* vec2);
    
    /// Subtract vector 2 to vector 1
    void sub_vector(float* vec1, float* vec2);
    
    /// Divide vector by factor
    void div_vector(float* vec, float factor);
    
    /// Multipy vector by factor
    void mult_vector(float* vec, float factor);
    
    /// Normalize vector
    void normalize(float* vec);
    
  };
} // namespace

#endif
