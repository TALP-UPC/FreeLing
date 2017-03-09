
#ifndef _ASR_DECODER
#define _ASR_DECODER

#include "asr/Request.h"
#include "asr/Response.h"
#include "asr/Nnet3LatgenFasterDecoder.h"

#include "util/simple-options.h"
#include "base/kaldi-types.h"
#include "matrix/kaldi-vector.h"
#include <sstream>
#include <utility>

namespace freeling {

/////////////////////////////////////////////////////////
///
///  asr class receives an audio and returns
///  its transcription as an string
///
/////////////////////////////////////////////////////////


// DEFINICIO DE OPTIONS CLASS


class asr {
  public:

  // constructor, passar fitxers de configuració
  asr(const std::wstring &configFile);
  // destructor
  ~asr();
  
  /// Decode function with in the wavfile in the form of a string
  std::vector< std::pair<std::wstring, float> > decode(const std::string &wav_file, int nbest, int decode_channel, float seconds_to_decode);
  /// Decode function with a istream as the input. The istream must be opened in binary mode
  std::vector< std::pair<std::wstring, float> > decode(std::istream &in_stream, int nbest, int decode_channel, float seconds_to_decode);
  
  // Returns the frequency of the decoder contained in the class 
  // NECESSARY?
  int getFrequency() const;
  
 private:
  std::string usage_;
  
  void RegisterOptions(kaldi::OptionsItf &so);
  
  Decoder *decoder;
  
  Request* request;
  Response* response;
  /// Frequency of the decoder contained in the class
  int frequency_;
  
};
 
} // namespace

#endif
