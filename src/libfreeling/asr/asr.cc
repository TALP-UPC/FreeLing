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

    wstring pathToData = configFile.substr(0,configFile.find_last_of(L"/\\")+1);

    // Variables to be set by the config file
    wstring fst_filename;
    wstring nnet3_model_filename;
    wstring words_filename;
    wstring mfcc_config_filename;
    wstring ivector_config_filename;
    wstring frequency;
    DecoderOptions options;

    string aux;
    wstring line;
    // Read config file
    while (cfg.get_content_line(line)) {
      wistringstream sin;
      sin.str(line);

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
          break;
        }
        case FREQUENCY: {
          sin >> frequency_;
          break;
        } 
        case FEATURETYPE: {
          sin >> options.featuretype;
          break;
        }
        case FRAMESUBSAMPLINGFACTOR: {
          sin >> options.framesubsamplingfactor;
          break;
        }
        case MAXACTIVE: {
          sin >> options.maxactive;
          break;
        }
        case BEAM: {
          sin >> options.beam;
          break;
        }
        case LATTICEBEAM: {
          sin >> options.latticebeam;
          break;
        }
        case ACOUSTICSCALE: {
          sin >> options.acousticscale;
          break;
        }
        case LMSCALE: {
          sin >> options.lmscale;
          break;
        }
        case POSTDECODEACWT: {
          sin >> options.postdecodeacwt;
          break;
        }
        case CHUNKLENGTHSECS: {
          sin >> options.chunklengthsecs;
          break;
        }
        case MAXRECORDSIZESECS: {
          sin >> options.maxrecordsizesecs;
          break;
        }
        case MAXLATTICEUNCHANGEDINTERVALSECS: {
          sin >> options.maxlatticeunchangedintervalsecs;
          break;
        }
        case DECODINGTIMEOUTSECS: {
          sin >> options.decodingtimeoutsecs;
          break;
        }
        default: 
          break;
      }
    }
    cfg.close();


    // if Ivector was given, make sure paths are absolute, making a temp copy of the original file
    wstring tmp_ivector;
    if (not ivector_config_filename.empty()) { 
      ivector_config_filename = util::absolute(ivector_config_filename,pathToData);
      
      // Transform relative paths in the file into absolute paths
      wifstream fs;
      util::open_utf8_file(fs, ivector_config_filename);

      tmp_ivector = util::new_tempfile_name();
      wofstream ofs;
      util::open_utf8_file(ofs, tmp_ivector);
      
      wstring line;      
      while (getline(fs, line)) {
        util::find_and_replace(line, L"${INSTALLPATH}", pathToData);
        ofs << line << endl;
      }
      fs.close();
      ofs.close();

      TRACE(4,L"Created TMP copy of ivector config at "<<tmp_ivector);
    }
    
    TRACE(3,L"Creating decoder with params:"<<frequency_<<L"\n"<<words_filename<<L"\n"<<fst_filename<<L"\n"<<nnet3_model_filename<<L"\n"<<mfcc_config_filename<<L"\n"<<tmp_ivector);

    // Initialize and store decoder
    decoder = new Nnet3LatgenFasterDecoder(frequency_, words_filename, fst_filename, nnet3_model_filename, mfcc_config_filename, tmp_ivector, options);

    // SimpleOptions kaldi class stores the decoder's configuration properties
    kaldi::SimpleOptions so;
    decoder->RegisterOptions(so);

    // Initialize asr decoder class
    if (not decoder->Initialize(so)) {
      ERROR_CRASH(L"There was an error initializing the decoder class.")
   }

    // remove ivector config temporal copy if it had been created
    // if (not tmp_ivector.empty()) remove(util::wstring2string(tmp_ivector).c_str());

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

    TRACE(2,"Creating request");
    // Create and initialize request class
    request = new Request(in_stream, decode_channel, nbest, seconds_to_decode);

    TRACE(2,"Creating response");
    // Call decode routine, store results in response class
    response = new Response();

    TRACE(2,"decoding " << decoder);
    decoder->Decode(*request, *response);
    TRACE(2,"done");

    // Check whether the decode process ended with an error
    if (response->HasError()) {
      ERROR_CRASH(L"Decoding process error: " + response->GetError());
    }

    // Return all the results obtained
    TRACE(2,"getting results");
    return response->GetAllResults();
  }

  ////////////////////////////////////////////////////////////////
  /// Returns the frequency of the decoder contained in the class
  ////////////////////////////////////////////////////////////////

  int asr::getFrequency() const {
    return frequency_;
  }


} // namespace
