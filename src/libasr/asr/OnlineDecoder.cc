
#include "asr/OnlineDecoder.h"
#include "freeling/morfo/traces.h"

namespace freeling {

  #define MOD_TRACECODE ASR_TRACE
  #define MOD_TRACENAME L"ASR"

  #define PAD_SIZE 400
  kaldi::BaseFloat padVector[PAD_SIZE];

  struct OnlineDecoder::DecodedData {
    kaldi::LatticeWeight weight;
    std::vector<int32> words;
    std::vector<int32> alignment;
    std::vector<kaldi::LatticeWeight> weights;
  };

  bool wordsEquals(std::vector<int32> &a, std::vector<int32> &b) {
    return (a.size() == b.size()) && (std::equal(a.begin(), a.end(), b.begin()));
  }

  bool getWeightMeasures(const kaldi::Lattice &fst,
                            std::vector<kaldi::LatticeArc::Weight> *weights_out) {
    typedef kaldi::LatticeArc::Label Label;
    typedef kaldi::LatticeArc::StateId StateId;
    typedef kaldi::LatticeArc::Weight Weight;

    std::vector<Weight> weights;

    StateId cur_state = fst.Start();
    if (cur_state == fst::kNoStateId) {  // empty sequence.
      if (weights_out != NULL) weights_out->clear();
      return true;
    }
    while (1) {
      Weight w = fst.Final(cur_state);
      if (w != Weight::Zero()) {  // is final..

        if (w.Value1() != 0 || w.Value2() != 0) {
          weights.push_back(w);
        }
        if (fst.NumArcs(cur_state) != 0) return false;
        if (weights_out != NULL) *weights_out = weights;
        return true;
      } else {
        if (fst.NumArcs(cur_state) != 1) return false;

        fst::ArcIterator<fst::Fst<kaldi::LatticeArc> > iter(fst, cur_state);  // get the only arc.
        const kaldi::LatticeArc &arc = iter.Value();
        if (arc.weight.Value1() != 0 || arc.weight.Value2() != 0) {
          weights.push_back(arc.weight);
        }
        cur_state = arc.nextstate;
      }
    }
  }

  // TODO: change initial parametres
  OnlineDecoder::OnlineDecoder() {
    /*lm_scale_ = 10;
    post_decode_acwt_ = 10;

    // MIRAR SI SON NECESSARIS
    chunk_length_secs_ = 0.18;
    max_record_size_seconds_ = 0;
    max_lattice_unchanged_interval_seconds_ = 0;
    decoding_timeout_seconds_ = 0;*/
  }

  OnlineDecoder::~OnlineDecoder() {
  }

  void OnlineDecoder::RegisterOptions(kaldi::OptionsItf &so) {
    //so.Register("chunk-length", &chunk_length_secs_,
    //            "Length of chunk size in seconds, that we process.");
    so.Register("word-symbol-table", &word_syms_rxfilename_,
                "Symbol table for words [for debug output]");
    so.Register("fst-in", &fst_rxfilename_, "Path to FST model file");
    so.Register("lm-scale", &lm_scale_, "Scaling factor for LM probabilities. "
               "Note: the ratio post-decode-acwt/lm-scale is all that matters.");
    so.Register("post-decode-acwt", &post_decode_acwt_, "Scaling factor for the acoustic probabilities. "
               "Note: the ratio post-decode-acwt/lm-scale is all that matters.");

    so.Register("max-record-length", &max_record_size_seconds_,
        "Max length of record in seconds to be recognised. "
      "All records longer than given value will be truncated. Note: Non-positive value to deactivate.");

    so.Register("max-lattice-unchanged-interval", &max_lattice_unchanged_interval_seconds_,
      "Max interval length in seconds of lattice recognised unchanged. Note: Non-positive value to deactivate.");

    so.Register("decoding-timeout", &decoding_timeout_seconds_,
        "Decoding process timeout given in seconds. Timeout disabled if value is non-positive.");
}


  bool OnlineDecoder::Initialize(kaldi::OptionsItf &so) {
    word_syms_ = NULL;
    if (word_syms_rxfilename_ == "") {
      return false;
    }
    if (!(word_syms_ = fst::SymbolTable::ReadText(word_syms_rxfilename_))) {
      ERROR_CRASH(L"Could not read symbol table from file " + util::string2wstring(word_syms_rxfilename_));
    }
    return true;
  }

  

  void OnlineDecoder::GetRecognitionResult(DecodedData &input, RecognitionResult *output) {
      // TODO move parameters to external file
      output->confidence = std::max(0.0, std::min(1.0, -0.0001466488 * (2.388449*float(input.weight.Value1()) + float(input.weight.Value2())) / (input.words.size() + 1) + 0.956));

      std::wostringstream outss;
      for (size_t i = 0; i < input.words.size(); i++) {
        if (i) {
          outss << L" ";
        }
        std::string s = word_syms_->Find(input.words[i]);

        std::wstring ws = util::string2wstring(s);
        if (ws == L"") {
          WARNING(L"Word-id " + std::to_wstring(input.words[i]) + L" not in symbol table.");
        } else {
          outss << ws;
        }
      }
      output->text = outss.str();

  }

  void OnlineDecoder::GetRecognitionResult(vector<DecodedData> &input, vector<RecognitionResult> *output) {
    for (int i = 0; i < input.size(); i++) {
  
      RecognitionResult result;
      GetRecognitionResult(input.at(i), &result);
      output->push_back(result);
    }
  }

  void OnlineDecoder::Decode(Request &request, Response &response) {
    if (request.Frequency() != decoder_frequency_)
      ERROR_CRASH(L"Audio frequency doesn't match decoder frequency. Audio frequency: " + util::string2wstring(std::to_string(request.Frequency())) + L", decoder frequency: " + util::string2wstring(std::to_string(decoder_frequency_)));
    
    /// INITIALIZE THE DECODER
    InputStarted();

    kaldi::BaseFloat seconds_to_decode = request.SecondsToDecode();

    if (seconds_to_decode == 0.0) {
      kaldi::SubVector<kaldi::BaseFloat>* wave_part = request.GetAudioChunk();
      AcceptWaveform(request.Frequency(), *wave_part, false);
    } else {
      int samp_counter = 0;
      int samples_per_chunk = int(seconds_to_decode * request.Frequency());
      
      kaldi::SubVector<kaldi::BaseFloat>* wave_part = request.NextChunk(samples_per_chunk);
      while (wave_part != NULL) {

        samp_counter += wave_part->Dim();

        AcceptWaveform(request.Frequency(), *wave_part, false);
        wave_part = request.NextChunk(samples_per_chunk);

      }
    }

    InputFinished();
    
    vector<DecodedData> result;

    int32 decoded = GetDecodingResults(request.BestCount(), &result);

    if (decoded == 0) {
      response.SetError(L"Decoding failed");
      WARNING(L"Decoding failed");
    } else {
      vector<RecognitionResult> recognitionResults;
      GetRecognitionResult(result, &recognitionResults);
      response.SetResult(recognitionResults);
    }

    CleanUp();

  }

  int32 OnlineDecoder::GetDecodingResults(int bestCount, vector<DecodedData> *result) {
    kaldi::CompactLattice clat;
    GetLattice(&clat, true);

    if (clat.NumStates() == 0) {
      return 0;
    }

    if (lm_scale_ != 0) {
      fst::ScaleLattice(fst::LatticeScale(lm_scale_, post_decode_acwt_), &clat);
    }

    int32 resultsNumber = 0;

    if (bestCount > 1) {
      kaldi::Lattice _lat;
      fst::ConvertLattice(clat, &_lat);
      kaldi::Lattice nbest_lat;
      fst::ShortestPath(_lat, &nbest_lat, bestCount);
      std::vector<kaldi::Lattice> nbest_lats;
      fst::ConvertNbestToVector(nbest_lat, &nbest_lats);
      if (!nbest_lats.empty()) {
        resultsNumber = static_cast<int32>(nbest_lats.size());
        for (int32 k = 0; k < resultsNumber; k++) {
          kaldi::Lattice &nbest_lat = nbest_lats[k];

          DecodedData decodeData;
          GetLinearSymbolSequence(nbest_lat, &(decodeData.alignment), &(decodeData.words), &(decodeData.weight));
          getWeightMeasures(nbest_lat, &(decodeData.weights));
          result->push_back(decodeData);
        }
      }
    } else {
      kaldi::CompactLattice best_path_clat;
      kaldi::CompactLatticeShortestPath(clat, &best_path_clat);

      kaldi::Lattice best_path_lat;
      fst::ConvertLattice(best_path_clat, &best_path_lat);
      DecodedData decodeData;
      GetLinearSymbolSequence(best_path_lat, &(decodeData.alignment), &(decodeData.words), &(decodeData.weight));
      getWeightMeasures(best_path_lat, &(decodeData.weights));
      result->push_back(decodeData);
      resultsNumber = 1;
    }

    return resultsNumber;
  }

}

