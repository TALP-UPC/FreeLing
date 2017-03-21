#include <iostream>
#include <string>

#include "freeling/morfo/util.h"
#include "asr/asr.h"
#include "asr/base64.h"
#include "feat/wave-reader.h"
#include "base/kaldi-types.h"
#include "matrix/kaldi-vector.h"

using namespace std;

/////////////////////////////////////////
///  Main: dispatch a unique petition
/////////////////////////////////////////

int main(int argc, char *argv[]) {

  freeling::util::init_locale(L"default");

  if (argc != 4) {
    cerr << "Usage:  " << argv[0] << " configfile nbest decode_channel < input-audio" << endl;
    exit(1);
  }
  wstring configFile = freeling::util::string2wstring(argv[1]);
  int nbest = freeling::util::wstring2int(freeling::util::string2wstring(argv[2]));
  int decode_channel = freeling::util::wstring2int(freeling::util::string2wstring(argv[3]));

  // load ASR module
  wcout << L"Loading ASR model..." << endl;
  freeling::asr *decoder = new freeling::asr(configFile);

  // read stdin (base 64 encoded audio) into a string
  string file = "";
  char s;
  while(cin.get(s)) { if (s!='\n') file += s; }

  // decode base 64 and convert to byte stream
  string transformedFile = base64_decode(file);
  istringstream stream(ios_base::binary);
  stream.str(transformedFile);

  // read .wav header
  kaldi::WaveInfoHolder wh;
  if (not wh.Read(stream)){
    cout << "Error reading the wav file" << endl;
    exit(1);
  }

  if (wh.Value().SampFreq() != decoder->getFrequency()) {
    wcerr << L"Input frequency ("<< wh.Value().SampFreq() <<") does not match model frequency ("<< decoder->getFrequency() <<")." << endl;
    exit(1);
  }

  if (decode_channel >= wh.Value().Data().NumRows() ) {
    wcerr << L"Input file has "<< wh.Value().Data().NumRows() <<" channels. Can not acces requested channel "<< decode_channel <<"." << endl;
    exit(1);
  }

  // decode input file
  wcout << "Decoding input..." << endl;
  vector<pair<wstring, float> > results = decoder->decode(transformedFile, nbest, decode_channel, 0.0);

  // print results
  wcout << "Result:" << endl;
  wcout << "<asr_results>" << endl;
  for (int i = 0; i < results.size(); i++) {
    wcout << " <text confidence=\"" << results[i].second << "\">" << results[i].first << "</text>" << endl;
  }
  wcout << "</asr_results>" << endl;
}
