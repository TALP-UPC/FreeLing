
#ifndef DECODER
#define DECODER

#include "asr/Request.h"
#include "asr/Response.h"

#include "util/parse-options.h"

namespace freeling {

class Decoder {
public:
  virtual ~Decoder() {};

  /// Returns a clone of the decoder
  virtual Decoder *Clone() const = 0;
  
  /** Register options which can be defined via command line arguments */
  virtual void RegisterOptions(kaldi::OptionsItf &so) = 0;
  
  /// Initialize decoder
  virtual bool Initialize(kaldi::OptionsItf &so) = 0;
  
  /// Perform decoding routine
  virtual void Decode(Request &request, Response &response) = 0;
  
};

}

#endif
