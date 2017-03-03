// OnlineDecoder.cc

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

#include "freeling/morfo/asr/OnlineDecoder.h"
#include "freeling/morfo/traces.h"

namespace freeling {

  #define MOD_TRACECODE ASR_TRACE
  #define MOD_TRACENAME L"ASR"

  /// DecodedData struct holds the results of the decoding process 
  /// extracted from kaldi's structure
  struct OnlineDecoder::DecodedData {
    kaldi::LatticeWeight weight;
    std::vector<int32> words;
    std::vector<int32> alignment;
    std::vector<kaldi::LatticeWeight> weights;
  };

  ////////////////////////////////////////////////////////////////
  /// Constructor, initialize audio properties
  ////////////////////////////////////////////////////////////////

  bool wordsEquals(std::vector<int32> &a, std::vector<int32> &b) {
    return (a.size() == b.size()) && (std::equal(a.begin(), a.end(), b.begin()));
  }

  ////////////////////////////////////////////////////////////////
  /// Extracts weights information from the lattice
  ////////////////////////////////////////////////////////////////

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


  ////////////////////////////////////////////////////////////////
  /// Destructor
  ////////////////////////////////////////////////////////////////

  OnlineDecoder::~OnlineDecoder() {
  }

  ////////////////////////////////////////////////////////////////
  /// Register custom options of the decoder
  ////////////////////////////////////////////////////////////////

  void OnlineDecoder::RegisterOptions(kaldi::OptionsItf &so) {

    so.Register("word-symbol-table", &word_syms_rxfilename_,
                "Symbol table for words [for debug output]");
    so.Register("fst-in", &fst_rxfilename_, "Path to FST model file");
    so.Register("lm-scale", &lm_scale_, "Scaling factor for LM probabilities. "
               "Note: the ratio post-decode-acwt/lm-scale is all that matters.");
    so.Register("post-decode-acwt", &post_decode_acwt_, "Scaling factor for the acoustic probabilities. "
               "Note: the ratio post-decode-acwt/lm-scale is all that matters.");
    
    /** Properties of the decoder for the online funcionality
     ** Uncomment if online funcionality is to be implemented

    so.Register("chunk-length", &chunk_length_secs_,
                "Length of chunk size in seconds, that we process.");
    so.Register("max-record-length", &max_record_size_seconds_,
        "Max length of record in seconds to be recognised. "
      "All records longer than given value will be truncated. Note: Non-positive value to deactivate.");
    so.Register("max-lattice-unchanged-interval", &max_lattice_unchanged_interval_seconds_,
      "Max interval length in seconds of lattice recognised unchanged. Note: Non-positive value to deactivate.");
    so.Register("decoding-timeout", &decoding_timeout_seconds_,
        "Decoding process timeout given in seconds. Timeout disabled if value is non-positive.");

    **/
}

  ////////////////////////////////////////////////////////////////
  /// Initialize the decoder: loads all necessary files
  ////////////////////////////////////////////////////////////////

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

  
  ////////////////////////////////////////////////////////////////
  /// Converts a result from the decoding process into readable text
  ////////////////////////////////////////////////////////////////

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


  ////////////////////////////////////////////////////////////////
  /// Converts a set of results from the decoding process into readable text
  ////////////////////////////////////////////////////////////////

  void OnlineDecoder::GetRecognitionResult(vector<DecodedData> &input, vector<RecognitionResult> *output) {
    for (int i = 0; i < input.size(); i++) {
  
      RecognitionResult result;
      GetRecognitionResult(input.at(i), &result);
      output->push_back(result);
    }
  }


  ////////////////////////////////////////////////////////////////
  /// Decode main routine
  ////////////////////////////////////////////////////////////////

  void OnlineDecoder::Decode(Request &request, Response &response) {

    TRACE(4,"Decoding - HERE");
      
    // Check if the audio request frequency matches the decoder frequency
    if (request.Frequency() != decoder_frequency_) {
      ERROR_CRASH(L"Audio frequency doesn't match decoder frequency. Audio frequency: " << request.Frequency() << L", decoder frequency: " << decoder_frequency_ );
    }

    TRACE(4,"Decoding - Starting input")
    
    // Prepare the decoder for the decoding call
    InputStarted();

    TRACE(4,"Decoding - input started");

    kaldi::BaseFloat seconds_to_decode = request.SecondsToDecode();

    TRACE(4,"Decoding seconds="<<seconds_to_decode);

    if (seconds_to_decode == 0.0) {   // Decode all the audio chunk at once
      TRACE(4,"Get audio");
      kaldi::SubVector<kaldi::BaseFloat>* wave_part = request.GetAudioChunk();

      TRACE(4,"Audio. Size="<<wave_part->Dim());
      AcceptWaveform(request.Frequency(), *wave_part, false);
    } 
    else {                          // Decode the audio chunk in parts
      int samp_counter = 0;
      int samples_per_chunk = int(seconds_to_decode * request.Frequency());
      
      kaldi::SubVector<kaldi::BaseFloat>* wave_part = request.NextChunk(samples_per_chunk);
      while (wave_part != NULL) {

        samp_counter += wave_part->Dim();

        AcceptWaveform(request.Frequency(), *wave_part, false);
        wave_part = request.NextChunk(samples_per_chunk);

      }
    }

    // Prepare the decoder to get results
    InputFinished();
    TRACE(4,"input finished");
    
    // Get decoding results
    vector<DecodedData> result;
    int32 decoded = GetDecodingResults(request.BestCount(), &result);

    if (decoded == 0) {
      response.SetError(L"Decoding failed");
      WARNING(L"Decoding failed");
    } else {
      // Store result in response wrapper class
      vector<RecognitionResult> recognitionResults;
      GetRecognitionResult(result, &recognitionResults);
      response.SetResult(recognitionResults);
    }

    CleanUp();

  }


  ////////////////////////////////////////////////////////////////
  /// Extracts and converts results from kaldi's decoding process
  ////////////////////////////////////////////////////////////////

  int32 OnlineDecoder::GetDecodingResults(int bestCount, vector<DecodedData> *result) {
    // Extract lattice
    kaldi::CompactLattice clat;
    GetLattice(&clat, true);

    if (clat.NumStates() == 0) {
      return 0;
    }

    if (lm_scale_ != 0) {
      fst::ScaleLattice(fst::LatticeScale(lm_scale_, post_decode_acwt_), &clat);
    }

    int32 resultsNumber = 0;

    // Convert lattice into DecodedData struct
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

} // namespace
