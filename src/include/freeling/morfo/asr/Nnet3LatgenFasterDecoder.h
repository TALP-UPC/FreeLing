// Nnet3LatgenFasterDecoder.h

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

#ifndef FASTER_DECODER
#define FASTER_DECODER

#include "OnlineDecoder.h"
#include "online2/online-nnet3-decoding.h"
#include "online2/online-nnet2-feature-pipeline.h"

namespace freeling {

/// DecoderOptions struct holds all the custom options of the decoder
struct DecoderOptions {
    string featuretype;
    int framesubsamplingfactor;
    int maxactive;     
    float beam;
    float latticebeam;
    float acousticscale;
    float lmscale;
    float postdecodeacwt;
    float chunklengthsecs;
    float maxrecordsizesecs;
    float maxlatticeunchangedintervalsecs;
    float decodingtimeoutsecs;
};

/////////////////////////////////////////////////////////
///
///  Holds and calls kaldi's inner decoder,
///  which is responsible for the decoding process
///    
/////////////////////////////////////////////////////////

class Nnet3LatgenFasterDecoder : public OnlineDecoder {
public:

  /// Constructor, configures the decoder
  Nnet3LatgenFasterDecoder(int frequency, const std::string &words_filename, const std::string &fst_filename, const std::string &nnet3_model_filename, const std::string &mfcc_config_filename, const std::string &ivector_config_filename, const DecoderOptions &options);
  
  /// Destructor
  virtual ~Nnet3LatgenFasterDecoder();

  /// Clone decoder
  virtual Nnet3LatgenFasterDecoder *Clone() const;

  /// Register options of the decoder
  virtual void RegisterOptions(kaldi::OptionsItf &so);

  /// Initialize the decoder: reads in all the necessary files
  virtual bool Initialize(kaldi::OptionsItf &so);

protected:

  /// Main audio acceptance function
  /// To be called from parent class
  virtual bool AcceptWaveform(kaldi::BaseFloat sampling_rate,
      const kaldi::VectorBase<kaldi::BaseFloat> &waveform,
      const bool do_endpointing);

  /// Prepare decoder to process input audio
  virtual void InputStarted();

  /// Prepare decoder to extract the results
  virtual void InputFinished();

  /// Extract kaldi's lattice which contains the results
  virtual void GetLattice(kaldi::CompactLattice *clat, bool end_of_utterance);

  /// Clean up after the decoding process
  virtual void CleanUp();

private:

  /// Holds the path for the "final.mdl" file
  std::string nnet3_rxfilename_;

  /// Whether the decoder has online funcionality
  bool online_;

  /// Configuration for the endpoints
  kaldi::OnlineEndpointConfig endpoint_config_;

  /// feature_config includes configuration for the iVector adaptation,
  /// as well as the basic features.
  kaldi::OnlineNnet2FeaturePipelineConfig feature_config_;

  /// Configuration of the neural net
  kaldi::OnlineNnet3DecodingConfig nnet3_decoding_config_;

  /// Contains info about the calculated features from the input pipeline
  kaldi::OnlineNnet2FeaturePipelineInfo *feature_info_;

  /// Instance of the language grammar in fst form
  fst::Fst<fst::StdArc> *decode_fst_;

  /// Transition model obtained from the "final.mdl" file
  kaldi::TransitionModel *trans_model_;

  /// Neural net obtained from the "final.mdl" file
  kaldi::nnet3::AmNnetSimple *nnet_;

  /// Contains information of the speaker to improve decode precision
  kaldi::OnlineIvectorExtractorAdaptationState *adaptation_state_;

  /// Pipeline of the features extracted from the audio
  kaldi::OnlineNnet2FeaturePipeline *feature_pipeline_;
  
  /// Instance of kaldi's decoder
  kaldi::SingleUtteranceNnet3Decoder *decoder_;
};

} // namespace

#endif 
