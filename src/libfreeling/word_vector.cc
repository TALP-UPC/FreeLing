#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <stdio.h>
#include <wchar.h>

#include "freeling/morfo/configfile.h"
#include "freeling/morfo/word_vector.h"

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"WORD_VECTOR"
#define MOD_TRACECODE WORD_VECTOR_TRACE

  ///////////////////////////////////////////////////////////////
  /// Constructor
  ///////////////////////////////////////////////////////////////
  word_vector::word_vector(const std::wstring &modelPath, std::wstring language) {
    // set language
    lang = language;
    
    // read model
    read_model(modelPath);
    
    TRACE(2, L"word_vector module succesfully created");
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Destructor
  /////////////////////////////////////////////////////////////////////////////
  word_vector::~word_vector() {
    if (model_memory != NULL)
      std::free(model_memory);
  }
  
    
  /////////////////////////////////////////////////////////////////////////////
  /// Model Properties
  /////////////////////////////////////////////////////////////////////////////
  
  /// Get vocabulary size
  unsigned int word_vector::get_vocab_size() {return vocabSize;}
  
  /// Get vocabulary as a list
  std::list<std::wstring> word_vector::get_vocab() {
    std::list<std::wstring> vocab_list;
    for (auto const& element : wordVec) {
      vocab_list.push_back(element.first);
    }
    
    return vocab_list;
  }
  
  /// Get vectors dimensionality
  unsigned int word_vector::get_dimensionality() {return dimensionality;}
  
  /// Get model language
  std::wstring word_vector::get_model_language() {return lang;}
  
  
  /////////////////////////////////////////////////////////////////////////////
  /// Word functions
  /////////////////////////////////////////////////////////////////////////////
  
  /// Find if word is in model
  bool word_vector::word_in_model(std::wstring word) {
    return (wordVec.count(word) == 1);
  }
  
  /// Get the vector representing a word. If word is not in the model, NULL is returned
  float* word_vector::get_vector(std::wstring word) {
    if (word_in_model(word)) {
      return wordVec[word];
    } else {
      return NULL;
    }
  }
  
  /// Cos similarity between two words, returns -1 if words not found in model
  float word_vector::cos_similarity(std::wstring word1, std::wstring word2) {
    // check if words exist
    if (word_in_model(word1) && word_in_model(word2)) {
      return cos_similarity(wordVec[word1], wordVec[word2]);
    } else {
      return -1.0;
    }
  }
    
  /// Cos similarity between word vectors
  float word_vector::cos_similarity(float* vec1, float* vec2) {
    // accumulator variables
    float mod_vec1, mod_vec2, sum;
    mod_vec1 = mod_vec2 = sum = 0.0;
    
    // cos similarity = (A·B) / (||A||·||B||) = sum(A_i·B_i) / sqrt(sum(A_i^2))·sqrt(sum(B_i^2))
    for (unsigned int ind = 0; ind < dimensionality; ind++) {
      sum      += vec1[ind]*vec2[ind];
      mod_vec1 += vec1[ind]*vec1[ind];
      mod_vec2 += vec2[ind]*vec2[ind];
    }
    
    // apply sqrt
    mod_vec1 = std::sqrt(mod_vec1);
    mod_vec2 = std::sqrt(mod_vec2);
    
    return sum/(mod_vec1*mod_vec2);
  }
  
  /// Gets the N closest words to the given word (this operation is O(N) on the size of the vocabulary)
  std::list<std::wstring> word_vector::similar_words(std::wstring word, unsigned int num_words) {
    if (!word_in_model(word)) return std::list<std::wstring>();
    std::list<std::wstring> result_list = similar_words(wordVec[word], num_words + 1);
    result_list.pop_front(); // remove "word" from list
    return result_list;
  }
  
  /// Gets the N closest words to the given vector (this operation is O(N) on the size of the vocabulary)
  std::list<std::wstring> word_vector::similar_words(float* vector, unsigned int num_words) {
    // extreme cases
    if (num_words == 0) {
      return std::list<std::wstring>();
    } else if (num_words >= vocabSize) {
      num_words = vocabSize - 1;
    }
    
    // init aux arrays
    std::vector<std::wstring> similar_words_vec(num_words);
    float word_similarities[num_words];
    std::fill_n(word_similarities, num_words, -1);
    
    // search against all words
    for (auto it : wordVec) {
      // add the new word to similar_words array if similarity is high enough
      float sim = cos_similarity(vector, it.second);
      if (sim > word_similarities[num_words - 1]) {
        similar_words_vec[num_words - 1] = it.first;
        word_similarities[num_words - 1] = sim;
        
        // update similar words positions
        unsigned int i = 1;
        while (i < num_words && sim > word_similarities[num_words - 1 - i]) {
          std::wstring aux_word = similar_words_vec[num_words - 1 - i];
          float aux_val = word_similarities[num_words - 1 - i];

          // swap values
          word_similarities[num_words - 1 - i] = word_similarities[num_words - i];
          similar_words_vec[num_words - 1 - i] = similar_words_vec[num_words - i];
          word_similarities[num_words - i] = aux_val;
          similar_words_vec[num_words - i] = aux_word;
          ++i;
        }
      }
    }
    
    return std::list<std::wstring>(similar_words_vec.begin(), similar_words_vec.end());
  }
  
  /// Returns the closest word in the model to the given vector (uses similar_words function)
  std::wstring word_vector::closest_word(float* vector) {
    return similar_words(vector, 1).front();
  }
  
  /// Returns Y in the analogy "A is to B what X is to Y". Words must be in the model
  std::wstring word_vector::analogy(std::wstring A, std::wstring B, std::wstring X) {
    float* vec = analogy_vec(A, B, X);
    std::wstring closest = closest_word(vec);
    delete[] vec;
    return closest;
  }
  
  /// Returns the Y vector representation in the analogy "A is to B what X is to Y". Words must be in the model
  float* word_vector::analogy_vec(std::wstring A, std::wstring B, std::wstring X) {
    float* vec = sub_vectors(wordVec[B], wordVec[A]);
    add_vector(vec, wordVec[X]);
    return vec;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Vector operations
  /////////////////////////////////////////////////////////////////////////////  
    
  /// Add vectors
  float* word_vector::add_vectors(float* vec1, float* vec2) {
    float* result_vec = new float[dimensionality];
    for (unsigned int i = 0; i < dimensionality; i++) {
      result_vec[i] = vec1[i] + vec2[i];
    }
    return result_vec;
  }
  
  /// Subtract vectors
  float* word_vector::sub_vectors(float* vec1, float* vec2) {
    float* result_vec = new float[dimensionality];
    for (unsigned int i = 0; i < dimensionality; i++) {
      result_vec[i] = vec1[i] - vec2[i];
    }
    return result_vec;
  }
  
  /// Add vector2 to vector1
  void word_vector::add_vector(float* vec1, float* vec2) {
    for (unsigned int i = 0; i < dimensionality; i++)
      vec1[i] = vec1[i] + vec2[i];
  }
  
  /// Subtract vector 2 to vector 1
  void word_vector::sub_vector(float* vec1, float* vec2) {
    for (unsigned int i = 0; i < dimensionality; i++)
      vec1[i] = vec1[i] - vec2[i];
  }
  
  /// Divide vector by factor
  void word_vector::div_vector(float* vec, float factor) {
    for (unsigned int i = 0; i < dimensionality; i++)
      vec[i] = vec[i]/factor;
  }

  /// Multipy vector by factor
  void word_vector::mult_vector(float* vec, float factor) { 
    for (unsigned int i = 0; i < dimensionality; i++)
      vec[i] = vec[i]*factor;
  }
  
  /// Normalize vector
  void word_vector::normalize(float* vec) {
    // calculate norm
    float norm = 0;
    for (unsigned int i = 0; i < dimensionality; i++) {
      norm += vec[i]*vec[i];
    }
    norm = std::sqrt(norm);
    
    // normalize diving the vector by its norm
    div_vector(vec, norm);
  }
  
  
  /////////////////////////////////////////////////////////////////////////////
  /// Read/Write model
  /////////////////////////////////////////////////////////////////////////////  
  
  /// Read the word embeddings model
  void word_vector::read_model(const std::wstring &path) {
    
    // check if model file is binary or plain text
    if (wstr_ends_with(path, L".bin")) { //READ AS BINARY FILE  
      FILE *modelFile;
      modelFile = fopen(freeling::util::wstring2string(path).c_str(), "rb");
      
      // check file can be opened
      if (!modelFile)
        ERROR_CRASH(L"Can't open word embeddings model file " + path);
      
      // read vocabSize and dimensionality
      if (!fscanf(modelFile, "%llu", &vocabSize))
        ERROR_CRASH(L"While reading word_vector model: couldn't read vocabulary size");
      if (!fscanf(modelFile, "%llu", &dimensionality))
        ERROR_CRASH(L"While reading word_vector model: couldn't read vectors dimensionality");
      
      // allocate dynamic memory
      model_memory = (float*) std::malloc(vocabSize*sizeof(float)*dimensionality);
      if (!model_memory)
        ERROR_CRASH(L"Memory allocation (" + freeling::util::int2wstring(vocabSize*sizeof(float)*dimensionality) + L" bytes) didn't succeed");
      
      // read model file into wordVec
      for (unsigned long long int word_ind = 0; word_ind < vocabSize; word_ind++) {
        // init next entry members
        float *vector = &model_memory[word_ind*dimensionality];
        
        // read key
        char buffer[MAX_STR_SIZE] = "";
        unsigned int buffer_index = 0;
        
        // get first character, skip blank characters
        buffer[buffer_index] = fgetc(modelFile);
        while(!feof(modelFile) && (buffer[buffer_index] == ' ' || buffer[buffer_index] == '\n'))
          buffer[buffer_index] = fgetc(modelFile);
        
        // read word until blank is found
        ++buffer_index;
        while(!feof(modelFile) && buffer_index < MAX_STR_SIZE) {
          char aux_char;
          aux_char = fgetc(modelFile);
          if (aux_char == ' ') break;
          
          buffer[buffer_index] = aux_char;
          buffer_index++;
        }
        
        // check buffer index and end of file to show warning message
        if (buffer_index >= MAX_STR_SIZE) {
          WARNING(L"While reading word_vector model: word in vocab may be longer than MAX_STR_SIZE");
        } else if (feof(modelFile)) {
          WARNING(L"While reading word_vector model: end of file reached before it was expected");
        }
        
        // read vector values
        if (fread(vector, sizeof(float), dimensionality, modelFile) != dimensionality)
          WARNING(L"While reading word_vector model, fread couldn't read the complete vector");
        
        // store values into the model
        wordVec[freeling::util::string2wstring(std::string(buffer))] = vector;
      }
      fclose(modelFile);
    } else { //READ AS TEXT FILE
      std::wifstream modelFile(freeling::util::wstring2string(path));
      
      // check file can be opened
      if (!modelFile)
        ERROR_CRASH(L"Can't open word embeddings model file " + path);
      
      // read vocabSize and dimensionality
      modelFile >> vocabSize >> dimensionality;
      
      // allocate dynamic memory
      model_memory = (float*) std::malloc(vocabSize*sizeof(float)*dimensionality);
      if (!model_memory)
        ERROR_CRASH(L"Memory allocation (" + freeling::util::int2wstring(vocabSize*sizeof(float)*dimensionality) + L" bytes) didn't succeed");
      
      // read model file into wordVec
      for (unsigned int word_ind = 0; word_ind < vocabSize; word_ind++) {
        // init next entry members
        float *vector = &model_memory[word_ind*dimensionality];
        std::wstring key;
        
        // read key and vector values
        modelFile >> key;
        for (unsigned int dim_ind = 0; dim_ind < dimensionality; dim_ind++) {
          modelFile >> vector[dim_ind];
        }
        
        // store values into the model
        wordVec[key] = vector;
      }
    }
    
    TRACE(3, L"word embeddings model read succesfully");
  }
  
  /// Dumps the model in the specified file. If the file name ends with ".bin", the dump will be in binary format
  void word_vector::dump_model(const std::wstring &path) {
      
    // open file
    FILE *modelFile;
    modelFile = fopen(freeling::util::wstring2string(path).c_str(), "wb");
    if (!modelFile)
      ERROR_CRASH(L"Can't open file " + path + L" (for model dump)");
    
    // write file
    fprintf(modelFile, "%llu %llu\n", vocabSize, dimensionality);
    if (wstr_ends_with(path, L".bin")) { // WRITE AS BINARY FILE  
      for (auto const& element : wordVec) {
        std::string str = freeling::util::wstring2string(element.first);
        fwrite(&str[0], sizeof(char), str.length(), modelFile);
        fwrite(" ", sizeof(char), 1, modelFile);
        fwrite(element.second, sizeof(float), dimensionality, modelFile);
        fwrite("\n", sizeof(char), 1, modelFile);
      }
    } else {                             // WRITE AS PLAIN TEXT FILE
      for (auto const& element : wordVec) {
        std::string str = freeling::util::wstring2string(element.first);
        fwrite(&str[0], sizeof(char), str.length(), modelFile);
        fwrite(" ", sizeof(char), 1, modelFile);
        for (unsigned int i = 0; i < dimensionality - 1; i++)
          fprintf(modelFile, "%f ", element.second[i]);
        fprintf(modelFile, "%f\n", element.second[dimensionality - 1]);
      }
    }
    
    // close file
    fclose(modelFile);
  }
  
  
  /////////////////////////////////////////////////////////////////////////////
  /// Other Functions
  /////////////////////////////////////////////////////////////////////////////  
  
  bool word_vector::wstr_ends_with(std::wstring wstr, std::wstring end) {
    for (unsigned int i = 0; i < end.size(); i++) {
      if (end[end.size() - 1 - i] != wstr[wstr.size() - 1 - i])
        return false;
    }
    return true;
  }
  
} // namespace