#include <iostream>
#include <string>

#include "freeling/morfo/util.h"
#include "asr/asr.h"
#include "feat/wave-reader.h"
#include "base/kaldi-types.h"
#include "matrix/kaldi-vector.h"

using namespace std;

/////////////////////////////////////////
///  Main: dispatch a unique petition
/////////////////////////////////////////

int main(int argc, char *argv[]) {

  freeling::util::init_locale(L"default");

  if (argc != 5) {
    cerr << "Usage:  " << argv[0] << " configfile nbest decode_channel input-audio.wav" << endl;
    exit(1);
  }
  wstring configFile = freeling::util::string2wstring(argv[1]);
  int nbest = freeling::util::wstring2int(freeling::util::string2wstring(argv[2]));
  int decode_channel = freeling::util::wstring2int(freeling::util::string2wstring(argv[3]));
  string audio_file = string(argv[4]);

  // load ASR module
  wcout << L"Loading ASR model..." << endl;
  freeling::asr *decoder = new freeling::asr(configFile);

  // open input .wav file
  ifstream audio;
  audio.open(audio_file, ios::binary);

  // decode input file
  wcout << "Decoding input..." << endl;
  vector<pair<wstring, float> > results = decoder->decode(audio, nbest, decode_channel, 0.0);

  // print results
  wcout << "Result:" << endl;
  wcout << "<asr_results>" << endl;
  for (int i = 0; i < results.size(); i++) {
    wcout << " <text confidence=\"" << results[i].second << "\">" << results[i].first << "</text>" << endl;
  }
  wcout << "</asr_results>" << endl;
}
