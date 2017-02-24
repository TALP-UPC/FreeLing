

///////////////////////////////////////////
//  This program can be used to test a word embeddings model.
//   
//   Usage:
//     ./word_vec model-file
//     
//   Where "model_file" is a word embedding dictionary.
//
//   Dictionaries may be obtained:
//      1.- building them with word2vec or other software.
//      2.- installing freeling with "./configure --enable-download-embeddings"
//      3.- Dowloading them from 
//             nlp.cs.upc.edu/freeling/extradata/freeling_embeddings.tar.bz2
//
///////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <algorithm>

#include "freeling.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/word_vector.h"

using namespace std;

int main(int argc, char* argv[]){

  // set locale to an UTF8 compatible locale
  freeling::util::init_locale(L"default");

  // print usage if config-file missing
  if (argc != 2) {
    wcerr<<L"Usage:  word_vec model-file" << endl; 
    exit(1);
  }
  
  // create a word_vector module with the given model
  wcout << L"Initializing word_vector module..." << flush;
  wstring model_file = freeling::util::string2wstring(argv[1]);
  freeling::word_vector wordVec(model_file);
  wcout << L" DONE" << endl;
  
  // warn the user if he's not using a binary model
  if (model_file.substr(model_file.length() - 4) != L".bin") {
    wcout << L"It seems you are not using a binary model file (.bin), do you want to dump a copy of the model in binary format? (y/n) " << flush;
    wstring user_input;
    wcin >> user_input;
    transform(user_input.begin(), user_input.end(), user_input.begin(), ::tolower);
    if (user_input == L"y" || user_input == L"yes") {
      wcout << L"Writing model file as binary file..." << flush;
      string::size_type found = model_file.find_last_of(L".");
      wordVec.dump_model(model_file.substr(0, found) + L".bin");
      wcout << L" DONE" << endl;
    }
  }
  
  // process input
  wstring line;
  wcout << endl << L"How To Use:" << endl;
  wcout << L"  Write one word to get similar words" << endl;
  wcout << L"  Write two words to get their cos similarity" << endl;
  wcout << L"  Write word_A : word_B :: word_C : ? to find the analogy" << endl << endl;
  while (getline(wcin, line)) {
    // read two words from line
    wstring wordA, wordB, wordC, wordD, wordE;
    
    wistringstream wiss(line);
    wiss >> wordA >> wordB >> wordC >> wordD >> wordE;
    
    if (wordA == L"\n" || wordA == L"") continue;
    if (wordB == L"") {
      unsigned int num_words = 10;
      list<wstring> similar_words = wordVec.similar_words(wordA, num_words); //num_words can be omitted, default value is 10
      wcout << L"#--------------------------------" << endl;
      wcout << L"# Similar words (similarity) " << endl;
      wcout << L"#--------------------------------" << endl;
      if (similar_words.size() == 0)
        wcout << L"#   WORD NOT IN MODEL" << endl;
      else {
        for (auto word : similar_words) {
          wcout << L"#   " << word << L" (" << wordVec.cos_similarity(wordA, word) << L")" << endl;
        }
      }
      wcout << L"#--------------------------------" << endl << endl;
    } else if (wordB == L":" && wordD == L"::") {
      wcout << L"#--------------------------------" << endl;
      if (!wordVec.word_in_model(wordA) || !wordVec.word_in_model(wordC) || !wordVec.word_in_model(wordE)) {
        wcout << L"# WORD(S) NOT IN MODEL" << endl;
      } else {
        float* vec_answer = wordVec.analogy_vec(wordA, wordC, wordE);
        list<wstring> similar_words = wordVec.similar_words(vec_answer, 5);
        wstring answer = similar_words.front();
        wcout << L"# Analogy: \"" << wordA << L"\" is to \"" << wordC << L"\" what \"" << wordE << L"\" is to \"" << answer << L"\"" << endl;
        wcout << L"# Closest words for the analogy:" << endl;
        for (auto word : similar_words) {
          wcout << L"#   " << word << L" (" << wordVec.cos_similarity(vec_answer, wordVec.get_vector(word)) << L")" << endl;
        }
        delete[] vec_answer;
      }
      wcout << L"#--------------------------------" << endl << endl;
    } else {
      // calculate cos similarity
      float sim = wordVec.cos_similarity(wordA, wordB);
      
      // print cos similarity
      wcout << L"#--------------------------------" << endl;
      if (sim == -1) {
        wcout << L"# WORD(S) NOT IN MODEL" << endl;
      } else {
        wcout << L"# Cosine Similarity: " << sim << endl;
      }
      wcout << L"#--------------------------------" << endl << endl;
    }
  }
}
