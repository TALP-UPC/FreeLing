#ifndef _REQUEST_H
#define _REQUEST_H

#include <istream>

#include "base/kaldi-types.h"
#include "matrix/kaldi-vector.h"
#include "feat/wave-reader.h"

namespace freeling {

class Request {
  public:

    /// Constructor, initialize audio properties
    Request(std::istream& stream, int decode_channel, int nbest, float seconds_to_decode);

    /// Destructor, free space
    ~Request();

    /** Get number of samples per second of audio data */
    kaldi::int32 Frequency() const;

    /** Get max number of expected result variants */
    int BestCount() const; 

    /** Returns the seconds to decode in each part of the decoding. If it's 0.0, */
    /** get all the audio chunk at one time */
    kaldi::BaseFloat SecondsToDecode() const;

    /** Get milliseconds interval between intermediate results.
     *  If non-positive given then no intermediate results would be calculated */
    //virtual kaldi::int32 IntermediateIntervalMillisec(void) const = 0;

    /** Get end-of-speech points detection flag. */
    //virtual bool DoEndpointing(void) const = 0;

    /**
     * Get the next chunk of audio data samples.
     */
    kaldi::SubVector<kaldi::BaseFloat> *NextChunk(kaldi::int32 samples_count);
   
    /**
     * Get the chunk of audio data samples.
     */
    kaldi::SubVector<kaldi::BaseFloat> *GetAudioChunk();

    /**
     * Set the chunk of audio data samples.
     */
    void SetAudioChunk(kaldi::SubVector<kaldi::BaseFloat>* audioChunk);


    /**
     * Get next chunk of audio data samples.
     * Max number of samples specified by samples_count value.
     * Read timeout specified by timeout_ms.
     */
    //virtual kaldi::SubVector<kaldi::BaseFloat> *NextChunk(kaldi::int32 samples_count, kaldi::int32 timeout_ms) = 0;

  private:
    kaldi::int32 frequency_;
    kaldi::int32 channels_;
    kaldi::int32 channel_index_;
    kaldi::int32 bytes_per_sample_;
    kaldi::BaseFloat seconds_to_decode_;

    int nbest_;

    std::istream *is;
    kaldi::WaveHolder *wh;
    int current_index;
    std::vector<kaldi::BaseFloat> buffer;

    kaldi::SubVector<kaldi::BaseFloat> *AudioChunk; //new kaldi::SubVector<kaldi::BaseFloat>(buffer.data(), buffer.size());
    kaldi::SubVector<kaldi::BaseFloat> *chunk; //new kaldi::SubVector<kaldi::BaseFloat>(buffer.data(), buffer.size());
};

} 

#endif /* _REQUEST_H */
