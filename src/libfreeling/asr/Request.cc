// Request.cc

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

#include "freeling/morfo/asr/Request.h"
#include "freeling/morfo/traces.h"

namespace freeling {
  
  #define MOD_TRACECODE ASR_TRACE
  #define MOD_TRACENAME L"ASR"

  ////////////////////////////////////////////////////////////////
  /// Constructor, initialize audio properties
  ////////////////////////////////////////////////////////////////

  Request::Request(std::istream& stream, int decode_channel, int nbest, float seconds_to_decode) {
    // Process wav file contained in the stream into kaldi's SubVector of BaseFloats
    wh = new kaldi::WaveHolder();
    if (!wh->Read(stream)) {
      throw std::runtime_error("There was a problem reading the wav file");
    }

    // Store the variables of the request
    frequency_ = (int) wh->Value().SampFreq();
    channels_ = wh->Value().Data().NumRows();
    channel_index_ = decode_channel;
    nbest_ = nbest;
    seconds_to_decode_ = seconds_to_decode;
    current_index = 0;

    // Initialize chunk pointer to 
    chunk = NULL;

    // Check if number of channels and selected decode channels are valid. If not, decode channel 0 by default
    if (channel_index_ < 0) {
      WARNING(L"The decoded channel parameter must be either 0 or a positive integer. Decoding channel 0 by default.");
      channel_index_ = 0;
    }
    if (channels_ <= channel_index_) {
      WARNING(L"The provided decoded channel doesn't exist in the audio. Decoding channel 0 by default.");
      channel_index_ = 0;
    }

  }

  ////////////////////////////////////////////////////////////////
  /// Destructor, free space
  ////////////////////////////////////////////////////////////////

  Request::~Request() {
    delete chunk;
    delete wh;
  }

  ////////////////////////////////////////////////////////////////
  /// Get number of samples per second of audio data
  ////////////////////////////////////////////////////////////////

  kaldi::int32 Request::Frequency() const {
    return frequency_;
  }

  ////////////////////////////////////////////////////////////////
  /// Get max number of expected result variants
  ////////////////////////////////////////////////////////////////

  int Request::BestCount() const {
    return nbest_;
  }

  ////////////////////////////////////////////////////////////////
  /// Returns the seconds to decode in each part of the decoding
  ////////////////////////////////////////////////////////////////

  kaldi::BaseFloat Request::SecondsToDecode() const {
    return seconds_to_decode_;
  }

  ////////////////////////////////////////////////////////////////
  /// Get the next chunk of audio data samples
  ////////////////////////////////////////////////////////////////

  kaldi::SubVector<kaldi::BaseFloat> *Request::NextChunk(kaldi::int32 samples_count) {
    int audioLength = (wh->Value().Data().Row(channel_index_)).Dim();
    // If index exceeds length of the audio, we have finished. Return null
    if (current_index >= audioLength) return NULL;

    int end = current_index + samples_count - 1;
    // If end index of the call exceeds audio length, return all remaining audio samples
    if (end >= audioLength) samples_count = audioLength - current_index;
    
    // Get the desired audio chunk 
    chunk = new kaldi::SubVector<kaldi::BaseFloat>((wh->Value().Data().Row(channel_index_)).Range(current_index,samples_count));

    // Increment current index to current position
    current_index += samples_count;

    return chunk;
  }

  ////////////////////////////////////////////////////////////////
  /// Get the complete chunk of audio data samples, at once
  ////////////////////////////////////////////////////////////////

  kaldi::SubVector<kaldi::BaseFloat> *Request::GetAudioChunk() {
    return new kaldi::SubVector<kaldi::BaseFloat>(wh->Value().Data().Row(channel_index_));
  }

} // namespace