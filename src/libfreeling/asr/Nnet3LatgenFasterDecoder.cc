// Nnet3LatgenFasterDecoder.cc

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

#include <iostream>
#include "freeling/morfo/asr/Nnet3LatgenFasterDecoder.h"

namespace freeling {

  //#define MOD_TRACECODE ASR_TRACE
  #define MOD_TRACENAME L"ASR"

Nnet3LatgenFasterDecoder::Nnet3LatgenFasterDecoder(int frequency, const std::string &words_filename, const std::string &fst_filename, const std::string &nnet3_model_filename, const std::string &mfcc_config_filename, const std::string &ivector_config_filename, const DecoderOptions &options) {
  online_ = false;
  decode_fst_ = NULL;
  trans_model_ = NULL;
  nnet_ = NULL;
  feature_info_ = NULL;

  decoder_frequency_ = frequency;
  nnet3_rxfilename_ = nnet3_model_filename;
  fst_rxfilename_ = fst_filename;
  word_syms_rxfilename_ = words_filename;
  
  if (ivector_config_filename != "-")
    feature_config_.ivector_extraction_config = ivector_config_filename;
  feature_config_.mfcc_config = mfcc_config_filename;
  feature_config_.feature_type = options.featuretype;

  // DecodableNnet3OnlineOptions configuration
  nnet3_decoding_config_.decodable_opts.acoustic_scale = options.acousticscale;
  nnet3_decoding_config_.decodable_opts.frame_subsampling_factor = options.framesubsamplingfactor;

  // LatticeFasterDecoderConfig configuration
  nnet3_decoding_config_.decoder_opts.max_active = options.maxactive;
  nnet3_decoding_config_.decoder_opts.beam = options.beam;
  nnet3_decoding_config_.decoder_opts.lattice_beam = options.latticebeam;

  lm_scale_ = options.lmscale;
  post_decode_acwt_ = options.postdecodeacwt;

  // Online decoding parametres
  // Uncomment if you are adding online decoding support to the system

  // chunk_length_secs_ = options.chunklengthsecs;
  // max_record_size_seconds_ = options.maxrecordsizesecs;
  // max_lattice_unchanged_interval_seconds_ = options.maxlatticeunchangedintervalsecs;
  // decoding_timeout_seconds_ = options.decodingtimeoutsecs;
}

Nnet3LatgenFasterDecoder::~Nnet3LatgenFasterDecoder() {
  delete decode_fst_;
  delete trans_model_;
  delete nnet_;
  delete feature_info_;
}

Nnet3LatgenFasterDecoder *Nnet3LatgenFasterDecoder::Clone() const {
  return new Nnet3LatgenFasterDecoder(*this);
}


void Nnet3LatgenFasterDecoder::RegisterOptions(kaldi::OptionsItf &so) {
  OnlineDecoder::RegisterOptions(so);

  so.Register("nnet-in", &nnet3_rxfilename_,
                  "Path to nnet");
  so.Register("online", &online_,
              "You can set this to false to disable online iVector estimation "
              "and have all the data for each utterance used, even at "
              "utterance start.  This is useful where you just want the best "
              "results and don't care about online operation.  Setting this to "
              "false has the same effect as setting "
              "--use-most-recent-ivector=true and --greedy-ivector-extractor=true "
              "in the file given to --ivector-extraction-config, and "
              "--chunk-length=-1.");

  feature_config_.Register(&so);
  nnet3_decoding_config_.Register(&so);
  endpoint_config_.Register(&so);
}


bool Nnet3LatgenFasterDecoder::Initialize(kaldi::OptionsItf &so) {
  if (!OnlineDecoder::Initialize(so)) {
    return false;
  }

  if (fst_rxfilename_ == "") {
    return false;
  }

  if (nnet3_rxfilename_ == "") {
    return false;
  }

  feature_info_ = new kaldi::OnlineNnet2FeaturePipelineInfo(feature_config_);

  if (!online_) {
    feature_info_->ivector_extractor_info.use_most_recent_ivector = true;
    feature_info_->ivector_extractor_info.greedy_ivector_extractor = true;
    chunk_length_secs_ = -1.0;
  }

  trans_model_ = new kaldi::TransitionModel();

  // Read the neural net and the language network
  nnet_ = new kaldi::nnet3::AmNnetSimple();
  {
    bool binary;
    kaldi::Input ki(nnet3_rxfilename_, &binary);
    trans_model_->Read(ki.Stream(), binary);
    nnet_->Read(ki.Stream(), binary);
  }

  decode_fst_ = fst::ReadFstKaldi(fst_rxfilename_);

  fst::SymbolTable *word_syms = NULL;
  if (word_syms_rxfilename_ != "")
    if (!(word_syms = fst::SymbolTable::ReadText(word_syms_rxfilename_)))
      ERROR_CRASH(L"Could not read symbol table from file " + util::string2wstring(word_syms_rxfilename_));

  acoustic_scale_ = nnet3_decoding_config_.decodable_opts.acoustic_scale;
 
  return true;
}

void Nnet3LatgenFasterDecoder::InputStarted()
{
  adaptation_state_ = new kaldi::OnlineIvectorExtractorAdaptationState(feature_info_->ivector_extractor_info);

  feature_pipeline_ = new kaldi::OnlineNnet2FeaturePipeline (*feature_info_);
  feature_pipeline_->SetAdaptationState(*adaptation_state_);

  // Initialization of the inner kaldi decoder class with its configuration parametres
  decoder_ = new kaldi::SingleUtteranceNnet3Decoder(nnet3_decoding_config_,
                    *trans_model_,
                    *nnet_,
                    *decode_fst_,
                    feature_pipeline_);

}


void Nnet3LatgenFasterDecoder::CleanUp()
{
  delete decoder_;
  delete adaptation_state_;
  delete feature_pipeline_;

  decoder_ = NULL;
  adaptation_state_ = NULL;
  feature_pipeline_ = NULL;
}

bool Nnet3LatgenFasterDecoder::AcceptWaveform(kaldi::BaseFloat sampling_rate,
    const kaldi::VectorBase<kaldi::BaseFloat> &waveform,
    const bool do_endpointing)
{

  feature_pipeline_->AcceptWaveform(sampling_rate, waveform);

  if (do_endpointing && decoder_->EndpointDetected(endpoint_config_)) {
    return false;
  }

  decoder_->AdvanceDecoding();
  
  return true;
}

void Nnet3LatgenFasterDecoder::InputFinished()
{
  feature_pipeline_->InputFinished();
  decoder_->AdvanceDecoding();
  decoder_->FinalizeDecoding();

}

void Nnet3LatgenFasterDecoder::GetLattice(kaldi::CompactLattice *clat, bool end_of_utterance)
{
  decoder_->GetLattice(end_of_utterance, clat);

  // In an application you might avoid updating the adaptation state if
  // you felt the utterance had low confidence.  See lat/confidence.h
  // feature_pipeline_->GetAdaptationState(adaptation_state_);

  if (acoustic_scale_ != 0) {
    ScaleLattice(fst::AcousticLatticeScale(1.0 / acoustic_scale_), clat);
  }
}

}
