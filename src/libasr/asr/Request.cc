
#include "asr/Request.h"
#include "freeling/morfo/traces.h"

namespace freeling {
  
  //#define MOD_TRACECODE ASR_TRACE
  #define MOD_TRACENAME L"ASR"


  Request::Request(std::istream& stream, int decode_channel, int nbest, float seconds_to_decode) {
    // Process wav file contained in the stream into kaldi's SubVector of BaseFloats
    wh = new kaldi::WaveHolder();
    if (!wh->Read(stream)) {
      throw std::runtime_error("There was a problem reading the wav file");
    }

    frequency_ = (int) wh->Value().SampFreq();
    channels_ = wh->Value().Data().NumRows();
    channel_index_ = decode_channel;
    nbest_ = nbest;
    seconds_to_decode_ = seconds_to_decode;

    // Check if number of channels and selected decode channels are valid. If not, decode channel 0 by default
    if (channel_index_ < 0) {
      WARNING(L"The decoded channel parameter must be either 0 or a positive integer. Decoding channel 0 by default.");
      channel_index_ = 0;
    }
    if (channels_ <= channel_index_) {
      WARNING(L"The provided decoded channel doesn't exist in the audio. Decoding channel 0 by default.");
      channel_index_ = 0;
    }

    current_index = 0;

    AudioChunk = NULL;
    chunk = NULL;
  }

  Request::~Request() {
    delete AudioChunk;
    delete chunk;
    delete wh;
  }

  kaldi::int32 Request::Frequency() const {
    return frequency_;
  }

  int Request::BestCount() const {
    return nbest_;
  }

  kaldi::BaseFloat Request::SecondsToDecode() const {
    return seconds_to_decode_;
  }

  kaldi::SubVector<kaldi::BaseFloat> *Request::NextChunk(kaldi::int32 samples_count) {
    
    int end = current_index + samples_count - 1;
    if (end >= (wh->Value().Data().Row(channel_index_)).Dim()) return NULL;

    chunk = new kaldi::SubVector<kaldi::BaseFloat>((wh->Value().Data().Row(channel_index_)).Range(current_index,samples_count));

    current_index += samples_count;

    return chunk;
  }

  kaldi::SubVector<kaldi::BaseFloat> *Request::GetAudioChunk() {
    return new kaldi::SubVector<kaldi::BaseFloat>(wh->Value().Data().Row(channel_index_));
  }

  void Request::SetAudioChunk(kaldi::SubVector<kaldi::BaseFloat>* audioChunk) {
    AudioChunk = audioChunk;
  }

}
