#include <iostream>
#include <fstream>
#include <algorithm>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/embeddings.h"

using namespace std;

int main(int argc, char* argv[]){

  // set locale to an UTF8 compatible locale
  freeling::util::init_locale(L"default");

  // print usage if config-file missing
  if (argc != 2) {
    wcerr<<L"Usage:  convert_model model-file" << endl; 
    exit(1);
  }
  
  // create a word_vector module with the given model
  wcout << L"Initializing word_vector module..." << flush;
  wstring model_file = freeling::util::string2wstring(argv[1]);
  freeling::embeddings wordVec(model_file);
  wcout << L" DONE" << endl;

  wstring basename = freeling::util::string2wstring(model_file.substr(0, model_file.find_last_of(L".")));
  
  // Text model, convert to binary
  if (model_file.substr(model_file.length()-4) != L".bin") {
    wcout << L"Converting model to binary format..." << flush;
    wordVec.dump_binary_model(basename + L".bin");
    wcout << L" DONE" << endl;
  } 

  // Binary model, convert to text.
  else {
    wcout << L"Converting model to words format..." << flush;
    wordVec.dump_text_model(basename + L".txt");
    wcout << L" DONE" << endl;
  }
}
