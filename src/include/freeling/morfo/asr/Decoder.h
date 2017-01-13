// Decoder.h

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.
//
// This file was modified from the originial project from Api.Ai hosted at 
// "https://github.com/api-ai/asr-server" for the ASR system for 
// Freeling, hosted at "https://github.com/TALP-UPC/FreeLing"


#ifndef _DECODER
#define _DECODER

#include "freeling/morfo/language.h"
#include "Request.h"
#include "Response.h"
#include "util/simple-options.h"

namespace freeling {

/////////////////////////////////////////////////////////
///
///  General interface for a kaldi decoder adapted 
///  to the structure of the project
///    
/////////////////////////////////////////////////////////

class Decoder {
public:
  
  /// Destructor
  virtual ~Decoder() {};

  /// Returns a clone of the decoder
  virtual Decoder *Clone() const = 0;

  /// Register options of the specific decoder 
  virtual void RegisterOptions(kaldi::OptionsItf &so) = 0;

  /// Initialize decoder
  virtual bool Initialize(kaldi::OptionsItf &so) = 0;

  /// Perform decoding routine
  virtual void Decode(Request &request, Response &response) = 0;

};

} // namespace

#endif