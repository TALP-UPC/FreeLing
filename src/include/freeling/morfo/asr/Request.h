// Request.h

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

#ifndef _REQUEST_H
#define _REQUEST_H

#include <istream>

#include "base/kaldi-types.h"
#include "matrix/kaldi-vector.h"
#include "feat/wave-reader.h"

namespace freeling {

/////////////////////////////////////////////////////////
///
///  Receives the audio file in wave file form,
///  transforms it into useful audio data and holds that data 
///    
/////////////////////////////////////////////////////////

class Request {
  public:

    /// Constructor, initialize audio properties
    Request(std::istream& stream, int decode_channel, int nbest, float seconds_to_decode);

    /// Destructor, free space
    ~Request();

    /// Get number of samples per second of audio data 
    kaldi::int32 Frequency() const;

    /// Get max number of expected result variants 
    int BestCount() const; 

    /// Returns the seconds to decode in each part of the decoding. If it's 0.0, 
    /// get all the audio chunk at one time 
    kaldi::BaseFloat SecondsToDecode() const;

    /// Get the next chunk of audio data samples. When the audio processing is finished, will return NULL
    kaldi::SubVector<kaldi::BaseFloat> *NextChunk(kaldi::int32 samples_count);
   
    /// Get the complete chunk of audio data samples, at once
    kaldi::SubVector<kaldi::BaseFloat> *GetAudioChunk();

  private:
    
    /// Frequency of the request audio
    kaldi::int32 frequency_;

    /// Number of channels the request audio has
    kaldi::int32 channels_;

    /// Index of the channel to decode
    kaldi::int32 channel_index_;
    
    /// Length of each part to decode at one time
    kaldi::BaseFloat seconds_to_decode_;

    /// Number of decoding alternatives the user requested
    int nbest_;

    /// Kaldi's wave holder class. This instance holds the audio data
    kaldi::WaveHolder *wh;

    /// Holds the sample index of the audio data vector for the NextChunk 
    /// function, otherwise we wouldn't know which chunk of audio to return next
    int current_index;

    /// Auxiliar variable, needed for the return of the NextChunk function
    kaldi::SubVector<kaldi::BaseFloat> *chunk; 
};

} // namespace

#endif 
