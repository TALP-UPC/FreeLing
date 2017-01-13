/////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#include "freeling/morfo/asr/asr.h"

namespace freeling {

  #define MOD_TRACECODE ASR_TRACE
  #define MOD_TRACENAME L"ASR"

  ////////////////////////////////////////////////////////////////
  /// Constructor
  ////////////////////////////////////////////////////////////////

  asr::asr(const std::wstring &configFile) {

    // Extract configuration data from config file
    enum sections {DNNHMMFILE, WORDSFILE, LANGUAGEMODELFILE, MFCCCONFIGFILE, IVECTORCONFIGFILE, FREQUENCY, FEATURETYPE, FRAMESUBSAMPLINGFACTOR, MAXACTIVE, BEAM, LATTICEBEAM, ACOUSTICSCALE, LMSCALE, POSTDECODEACWT, CHUNKLENGTHSECS, MAXRECORDSIZESECS, MAXLATTICEUNCHANGEDINTERVALSECS, DECODINGTIMEOUTSECS};
    config_file cfg;
    cfg.add_section(L"DnnHmm", DNNHMMFILE);
    cfg.add_section(L"WordsFile", WORDSFILE);
    cfg.add_section(L"LanguageModel", LANGUAGEMODELFILE);
    cfg.add_section(L"MfccConfig", MFCCCONFIGFILE);
    cfg.add_section(L"IvectorExtractorConfig", IVECTORCONFIGFILE);
    cfg.add_section(L"Frequency", FREQUENCY);
    cfg.add_section(L"FeatureType", FEATURETYPE);
    cfg.add_section(L"FrameSubsamplingFactor", FRAMESUBSAMPLINGFACTOR);
    cfg.add_section(L"MaxActive", MAXACTIVE);
    cfg.add_section(L"Beam", BEAM);
    cfg.add_section(L"LatticeBeam", LATTICEBEAM);
    cfg.add_section(L"AcousticScale", ACOUSTICSCALE);
    cfg.add_section(L"LmScale", LMSCALE);
    cfg.add_section(L"PostDecodeAcwt", POSTDECODEACWT);
    cfg.add_section(L"ChunkLengthSecs", CHUNKLENGTHSECS);
    cfg.add_section(L"MaxRecordSizeSecs", MAXRECORDSIZESECS);
    cfg.add_section(L"MaxLatticeUnchangedIntervalSecs", MAXLATTICEUNCHANGEDINTERVALSECS);
    cfg.add_section(L"DecodingTimeoutSecs", DECODINGTIMEOUTSECS);

    if (not cfg.open(configFile))
      ERROR_CRASH(L"Error opening file " + configFile);

    string pathToData = util::wstring2string(configFile.substr(0, configFile.rfind(L"config/")));

    // Variables to be set by the config file
    string fst_filename;
    string nnet3_model_filename;
    string words_filename;
    string mfcc_config_filename;
    string ivector_config_filename;
    string frequency;
    DecoderOptions options;

    string aux;
    wstring line;
    // Read config file
    while (cfg.get_content_line(line)) {
      istringstream sin;
      sin.str(util::wstring2string(line));

      switch (cfg.get_section()) {
        case DNNHMMFILE: {
          sin >> nnet3_model_filename;
          nnet3_model_filename = util::absolute(nnet3_model_filename,pathToData);
          break;
        }
        case WORDSFILE: {
          sin >> words_filename;
          words_filename = util::absolute(words_filename,pathToData);
          break;
        }
        case LANGUAGEMODELFILE: {
          sin >> fst_filename;
          fst_filename = util::absolute(fst_filename,pathToData);
          break;
        }
        case MFCCCONFIGFILE: {
          sin >> mfcc_config_filename;
          mfcc_config_filename = util::absolute(mfcc_config_filename,pathToData);
          break;
        }
        case IVECTORCONFIGFILE: {
          sin >> ivector_config_filename;
          if (ivector_config_filename != "-"){ 
            ivector_config_filename = util::absolute(ivector_config_filename,pathToData);

            // Transform relative paths in the file into absolute paths
            string newfile = "";
            string line;
            std::ifstream fs;
            fs.open(ivector_config_filename, std::ifstream::in);
            
            while (std::getline(fs, line)) {
              if (line.find("/") != std::string::npos && line.find("=/") == std::string::npos) { // change path of file
                line = line.insert(line.find("=")+1, pathToData);
              }
              newfile += line + "\n";
            }
            fs.close();

            std::ofstream ofs(ivector_config_filename, std::fstream::out | std::fstream::trunc );
            ofs << newfile;
            ofs.close();
          }
          break;
        }
        case FREQUENCY: {
          sin >> frequency;
          break;
        } 
        case FEATURETYPE: {
          sin >> options.featuretype;
          break;
        }
        case FRAMESUBSAMPLINGFACTOR: {
          sin >> aux;
          options.framesubsamplingfactor = std::atoi(aux.c_str());
          break;
        }
        case MAXACTIVE: {
          sin >> aux;
          options.maxactive = std::atoi(aux.c_str());
          break;
        }
        case BEAM: {
          sin >> aux;
          options.beam = std::stof(aux.c_str());
          break;
        }
        case LATTICEBEAM: {
          sin >> aux;
          options.latticebeam = std::stof(aux.c_str());
          break;
        }
        case ACOUSTICSCALE: {
          sin >> aux;
          options.acousticscale = std::atoi(aux.c_str());
          break;
        }
        case LMSCALE: {
          sin >> aux;
          options.lmscale = std::atoi(aux.c_str());
          break;
        }
        case POSTDECODEACWT: {
          sin >> aux;
          options.postdecodeacwt = std::atoi(aux.c_str());
          break;
        }
        case CHUNKLENGTHSECS: {
          sin >> aux;
          options.chunklengthsecs = std::atoi(aux.c_str());
          break;
        }
        case MAXRECORDSIZESECS: {
          sin >> aux;
          options.maxrecordsizesecs = std::atoi(aux.c_str());
          break;
        }
        case MAXLATTICEUNCHANGEDINTERVALSECS: {
          sin >> aux;
          options.maxlatticeunchangedintervalsecs = std::atoi(aux.c_str());
          break;
        }
        case DECODINGTIMEOUTSECS: {
          sin >> aux;
          options.decodingtimeoutsecs = std::atoi(aux.c_str());
          break;
        }
        default: 
          break;
      }
    }

    cfg.close();


    frequency_ = atoi(frequency.c_str());

    // Initialize and store decoder
    decoder = new Nnet3LatgenFasterDecoder(frequency_, words_filename, fst_filename, nnet3_model_filename, mfcc_config_filename, ivector_config_filename, options);

    // SimpleOptions kaldi class stores the decoder's configuration properties
    kaldi::SimpleOptions so;
    decoder->RegisterOptions(so);

    // Initialize asr decoder class
    if (!decoder->Initialize(so)) {
      ERROR_CRASH(L"There was an error initializing the decoder class.")
    }

    response = NULL;
    request = NULL;
  }

  ////////////////////////////////////////////////////////////////
  /// Destructor
  ////////////////////////////////////////////////////////////////

  asr::~asr() {
    delete decoder;
    delete response;
    delete request;
  }


  ////////////////////////////////////////////////////////////////
  /// Wrapper decode function: wraps the string in a stringstream and calls the base decode function
  ////////////////////////////////////////////////////////////////

  std::vector< std::pair<std::wstring, float> > asr::decode(const std::string &wav_file, int nbest, int decode_channel, float seconds_to_decode) {

    // Encapsulate string into istreamstring. Binary mode is necessary for kaldi wav reading
    std::istringstream stream(std::ios_base::binary);
    stream.str(wav_file);

    // Call main decoding function
    return decode(stream, nbest, decode_channel, seconds_to_decode);
  }

  ////////////////////////////////////////////////////////////////
  /// Base decode function
  ////////////////////////////////////////////////////////////////

  std::vector< std::pair<std::wstring, float> > asr::decode(std::istream &in_stream, int nbest, int decode_channel, float seconds_to_decode) {
    
    // Check if nbest parameter is valid.
    if (nbest < 1) {
      ERROR_CRASH(L"The nbest parameter must be a positive integer.");
    }

    // Check if seconds to decode parametre is valid
    if (seconds_to_decode < 0.0) {
      ERROR_CRASH(L"The seconds to decode parametre can't be negative.");
    }

    // Create and initialize request class
    request = new Request(in_stream, decode_channel, nbest, seconds_to_decode);

    // Call decode routine, store results in response class
    response = new Response();
    decoder->Decode(*request, *response);

    // Check whether the decode process ended with an error
    if (response->HasError()) {
      ERROR_CRASH(L"Decoding process error: " + response->GetError());
    }

    // Return all the results obtained
    return response->GetAllResults();
  }

  ////////////////////////////////////////////////////////////////
  /// Returns the frequency of the decoder contained in the class
  ////////////////////////////////////////////////////////////////

  int asr::getFrequency() const {
    return frequency_;
  }


} // namespace