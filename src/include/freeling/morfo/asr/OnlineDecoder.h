
#ifndef ONLINE_DECODER
#define ONLINE_DECODER

#include <iconv.h>
#include <string.h>
#include <list>

#include "freeling/morfo/util.h"
#include "freeling/morfo/asr/Decoder.h"

#include "online2/online-feature-pipeline.h"
#include "online2/onlinebin-util.h"
#include "online2/online-timing.h"
#include "online2/online-endpoint.h"
#include "fstext/fstext-lib.h"
#include "lat/lattice-functions.h"

namespace freeling {

/**
 * Basic implementation of common code for all Kaldi online decoders
 */
class OnlineDecoder : public Decoder {
public:
  OnlineDecoder();
  virtual ~OnlineDecoder();

  virtual void RegisterOptions(kaldi::OptionsItf &so);
  virtual bool Initialize(kaldi::OptionsItf &so);
  virtual void Decode(Request &request, Response &response);

protected:
  struct DecodedData;

  /**
   * Process next data chunk
   */
  virtual bool AcceptWaveform(kaldi::BaseFloat sampling_rate,
                    const kaldi::VectorBase<kaldi::BaseFloat> &waveform,
          const bool do_endpointing) = 0;

  /**
   * Preparare to decoding
   */
  virtual void InputStarted() = 0;
  /**
   * Decoding finished, gets ready to get results
   */
  virtual void InputFinished() = 0;
  /**
   * Put result lattice
   */
  virtual void GetLattice(kaldi::CompactLattice *clat, bool end_of_utterance) = 0;
  /**
   * Clean all data
   */
  virtual void CleanUp() = 0;

  std::string word_syms_rxfilename_;
  kaldi::BaseFloat chunk_length_secs_;
  kaldi::BaseFloat acoustic_scale_;
  kaldi::BaseFloat lm_scale_;
  kaldi::BaseFloat post_decode_acwt_;


  /**
   * Max length of record in seconds to be recognised.
   * All records longer than given value will be truncated. Note: Non-positive value to deactivate.
   */
  kaldi::BaseFloat max_record_size_seconds_;
  /**
   * Max interval length in seconds of lattice recognised unchanged. Non-positive value to deactivate
   */
  kaldi::BaseFloat max_lattice_unchanged_interval_seconds_;

  /** Decoding process timeout given in seconds.
   * Timeout disabled if value is non-positive
   */
  kaldi::BaseFloat decoding_timeout_seconds_;

  bool do_endpointing_;

  std::string fst_rxfilename_;
  int decoder_frequency_;
private:
  fst::SymbolTable *word_syms_;

  kaldi::int32 GetDecodingResults(int bestCount, std::vector<DecodedData> *result);

  void GetRecognitionResult(DecodedData &input, RecognitionResult *output);
  void GetRecognitionResult(vector<DecodedData> &input, vector<RecognitionResult> *output);
};

}

#endif 
