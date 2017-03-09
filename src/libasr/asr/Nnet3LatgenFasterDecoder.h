
#ifndef FASTER_DECODER
#define FASTER_DECODER

#include "asr/OnlineDecoder.h"

#include "online2/online-nnet3-decoding.h"
#include "online2/online-nnet2-feature-pipeline.h"

namespace freeling {

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

class Nnet3LatgenFasterDecoder : public OnlineDecoder {
public:
  Nnet3LatgenFasterDecoder(int frequency, const std::string &words_filename, const std::string &fst_filename, const std::string &nnet3_model_filename, const std::string &mfcc_config_filename, const std::string &ivector_config_filename, const DecoderOptions &options);
  virtual ~Nnet3LatgenFasterDecoder();

  virtual Nnet3LatgenFasterDecoder *Clone() const;
  virtual void RegisterOptions(kaldi::OptionsItf &so);
  virtual bool Initialize(kaldi::OptionsItf &so);

protected:
  virtual bool AcceptWaveform(kaldi::BaseFloat sampling_rate,
      const kaldi::VectorBase<kaldi::BaseFloat> &waveform,
      const bool do_endpointing);
  virtual void InputStarted();
  virtual void InputFinished();

  virtual void GetLattice(kaldi::CompactLattice *clat, bool end_of_utterance);
  virtual void CleanUp();
private:
  std::string nnet3_rxfilename_;

    bool online_;
    kaldi::OnlineEndpointConfig endpoint_config_;

    // feature_config includes configuration for the iVector adaptation,
    // as well as the basic features.
    kaldi::OnlineNnet2FeaturePipelineConfig feature_config_;
    kaldi::OnlineNnet3DecodingConfig nnet3_decoding_config_;

    kaldi::OnlineNnet2FeaturePipelineInfo *feature_info_;
    fst::Fst<fst::StdArc> *decode_fst_;
    kaldi::TransitionModel *trans_model_;
    kaldi::nnet3::AmNnetSimple *nnet_;


    kaldi::OnlineIvectorExtractorAdaptationState *adaptation_state_;
    kaldi::OnlineNnet2FeaturePipeline *feature_pipeline_;
    kaldi::SingleUtteranceNnet3Decoder *decoder_;
};

} 

#endif 
