

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
#include "freeling/morfo/embeddings.h"

using namespace std;

int main(int argc, char* argv[]){

  // set locale to an UTF8 compatible locale
  freeling::util::init_locale(L"default");

  // print usage if config-file missing
  if (argc != 2) {
    wcerr << L"Usage:  word_vec model-file" << endl; 
    exit(1);
  }
  
  // create a word_vector module with the given model
  wcout << L"Initializing word_vector module..." << flush;
  wstring model_file = freeling::util::string2wstring(argv[1]);
  freeling::embeddings wordVec(model_file);
  wcout << L" DONE" << endl;
    
  // process input
  wstring line;
  wcout << endl << L"How To Use:" << endl;
  wcout << L"  Write 'DUMPBIN filename' or 'DUMPTXT filename' to dump the model in binary/text format" << endl;
  wcout << L"  Write one word to get similar words" << endl;
  wcout << L"  Write two words to get their cos similarity" << endl;
  wcout << L"  Write word_A : word_B :: word_C : ? to find the analogy" << endl << endl;

  while (getline(wcin, line)) {
    // read two words from line
    wstring wordA, wordB, wordC, wordD, wordE;
    
    wistringstream wiss(line);
    wiss >> wordA >> wordB >> wordC >> wordD >> wordE;

    if (wordA == L"\n" || wordA == L"") continue;

    // dump model ------------------------------
    if (wordA==L"DUMPBIN") {
      wcout << endl << L"Writting binary model to file " << wordB << L"..." << flush;
      wordVec.dump_binary_model(wordB);
      wcout << L" DONE" << endl;
    }
    else if (wordA==L"DUMPTXT") {
      wcout << endl << L"Writting text model to file " << wordB << L"..." << flush;
      wordVec.dump_text_model(wordB);
      wcout << L" DONE" << endl;
    }
    
    // single word, find similar words ------------------------------
    else if (wordB == L"") {

      // get most similar words (number defaults to 10)
      list<pair<wstring,float> > similar_words = wordVec.similar_words(wordA); 

      wcout << L"#--------------------------------" << endl;
      wcout << L"# Similar words (similarity) " << endl;
      wcout << L"#--------------------------------" << endl;
      if (similar_words.size() == 0)
        wcout << L"#   WORD NOT IN MODEL" << endl;
      else {
        for (auto word : similar_words) 
          wcout << L"#   " << word.first << L" (" << word.second << L")" << endl;
      }
      wcout << L"#--------------------------------" << endl << endl;
    } 

    // analogy ------------------------------
    else if (wordB == L":" && wordD == L"::") {

      wcout << L"#--------------------------------" << endl;
      if (not wordVec.word_in_model(wordA) or not wordVec.word_in_model(wordC) or not wordVec.word_in_model(wordE)) 
        wcout << L"# WORD(S) NOT IN MODEL" << endl;

      else {
        freeling::norm_vector vec_answer = wordVec.analogy_vec(wordA, wordC, wordE);
        list<pair<wstring,float> > similar_words = wordVec.similar_words(vec_answer, 5);
        wstring answer = similar_words.front().first;
        wcout << L"# Analogy: \"" << wordA << L"\" is to \"" << wordC 
              << L"\" what \"" << wordE << L"\" is to \"" << answer << L"\"" << endl;
        wcout << L"# Closest words for the analogy:" << endl;
        for (auto word : similar_words) {
          wcout << L"#   " << word.first << L" (" << word.second << L")" << endl;
        }
      }
      wcout << L"#--------------------------------" << endl << endl;
    } 

    // two words, compute similarity ------------------------------
    else {
      // calculate cos similarity
      float sim = wordVec.cos_similarity(wordA, wordB);
      
      // print cos similarity
      wcout << L"#--------------------------------" << endl;
      if (sim == -1) 
        wcout << L"# WORD(S) NOT IN MODEL" << endl;
      else 
        wcout << L"# Cosine Similarity: " << sim << endl;
      wcout << L"#--------------------------------" << endl << endl;
    }
  }
}
