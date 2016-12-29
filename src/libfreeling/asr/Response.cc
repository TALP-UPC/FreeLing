// Response.cc

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

#include "freeling/morfo/asr/Response.h"

namespace freeling {
  
  #define MOD_TRACECODE ASR_TRACE
  #define MOD_TRACENAME L"ASR"

  ////////////////////////////////////////////////////////////////
  /// Constructor
  ////////////////////////////////////////////////////////////////

  Response::Response() {
    error = L"";
  }

  ////////////////////////////////////////////////////////////////
  /// Returns only the best result from the decoding process, in pairs of (text, confidence)
  ////////////////////////////////////////////////////////////////

  std::pair<std::wstring, float> Response::GetBestResult() {
    // Convert first result info from custom class RecognitionResult into stl vector and return it
    return std::make_pair(result.at(0).text, result.at(0).confidence);
  }

  ////////////////////////////////////////////////////////////////
  /// Returns all the results from the decoding process, in pairs of (text, confidence)
  ////////////////////////////////////////////////////////////////

  std::vector< std::pair<std::wstring, float> > Response::GetAllResults() {
    // Convert info from custom class RecognitionResult into stl vector and return it
    std::vector< std::pair<std::wstring, float> > res;
    for (int i = 0; i < result.size(); i++)
        res.push_back(std::make_pair(result.at(i).text, result.at(i).confidence)); 
    return res;
  }

  ////////////////////////////////////////////////////////////////
  /// Set final results
  ////////////////////////////////////////////////////////////////

  void Response::SetResult(std::vector<RecognitionResult> &data) {
    result = data;
  }

  ////////////////////////////////////////////////////////////////
  /// Whether or not the response is an error message
  ////////////////////////////////////////////////////////////////

  bool Response::HasError() {
    return error != L"";
  }

  ////////////////////////////////////////////////////////////////
  /// Get error message
  ////////////////////////////////////////////////////////////////

  std::wstring Response::GetError() {
    return error;
  }

  ////////////////////////////////////////////////////////////////
  /// Set error value
  ////////////////////////////////////////////////////////////////
  
  void Response::SetError(const std::wstring &message) {
    error = message;
  }

} // namespace