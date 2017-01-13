// OnlineDecoder.h

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

#ifndef ONLINE_DECODER
#define ONLINE_DECODER

#include "freeling/morfo/util.h"

#include "Decoder.h"
#include "online2/online-feature-pipeline.h"
#include "online2/onlinebin-util.h"
#include "online2/online-timing.h"
#include "online2/online-endpoint.h"
#include "fstext/fstext-lib.h"
#include "lat/lattice-functions.h"
#include <iconv.h>
#include <string.h>
#include <list>

namespace freeling {

/////////////////////////////////////////////////////////
///
///  Basic implementation of common code for all Kaldi online decoders
///    
/////////////////////////////////////////////////////////

class OnlineDecoder : public Decoder {
public:

  /// Destructor
  virtual ~OnlineDecoder();

  /// Register custom options of the decoder
  virtual void RegisterOptions(kaldi::OptionsItf &so);

  /// Initialize the decoder: loads all necessary files
  virtual bool Initialize(kaldi::OptionsItf &so);

  /// Decode main routine
  virtual void Decode(Request &request, Response &response);

protected:

  /// Definition of the struct
  struct DecodedData;

  /// Accepts a chunk of audio data
  virtual bool AcceptWaveform(kaldi::BaseFloat sampling_rate,
                    const kaldi::VectorBase<kaldi::BaseFloat> &waveform,
          const bool do_endpointing) = 0;

  /// Preparare to do the decoding process
  virtual void InputStarted() = 0;

  /// Decoding finished, gets ready to get results
  virtual void InputFinished() = 0;
  
  /// Put result in a kaldi lattice
  virtual void GetLattice(kaldi::CompactLattice *clat, bool end_of_utterance) = 0;
  
  /// Clean all data
  virtual void CleanUp() = 0;

  /// Holds the path to the "words.txt" file
  std::string word_syms_rxfilename_;

  /// Holds the path to the "HCLG.fst" file
  std::string fst_rxfilename_;

  /// Properties of the decoder

  /// Frequency of the decoder
  int decoder_frequency_;

  /// Acoustic scale for the decoding process
  kaldi::BaseFloat acoustic_scale_;

  /// Language model scale for the decoding process
  kaldi::BaseFloat lm_scale_;

  /// Post decode acoustic scale for the decoding process
  kaldi::BaseFloat post_decode_acwt_;


  /** Properties of the decoder for the online funcionality
   ** Uncomment if online funcionality is to be implemented

  /// Length of the audio chunk to be decoded 
  kaldi::BaseFloat chunk_length_secs_;

  /// Max length of record in seconds to be recognised.
  /// All records longer than given value will be truncated. Note: Non-positive value to deactivate.
  kaldi::BaseFloat max_record_size_seconds_;

  /// Max interval length in seconds of lattice recognised unchanged. Non-positive value to deactivate
  kaldi::BaseFloat max_lattice_unchanged_interval_seconds_;

  /// Decoding process timeout given in seconds.
  /// Timeout disabled if value is non-positive
  kaldi::BaseFloat decoding_timeout_seconds_;

  /// Whether to check for endpoints in the decoding process
  bool do_endpointing_;
  
  **/

private:

  /// Instance of the symbol table from "words.txt"
  fst::SymbolTable *word_syms_;

  /// Extracts and converts results from kaldi's decoding process
  kaldi::int32 GetDecodingResults(int bestCount, std::vector<DecodedData> *result);

  /// Converts a result from the decoding process into readable text
  void GetRecognitionResult(DecodedData &input, RecognitionResult *output);

  /// Converts a set of results from the decoding process into readable text
  void GetRecognitionResult(vector<DecodedData> &input, vector<RecognitionResult> *output);
};

} // namespace

#endif 
