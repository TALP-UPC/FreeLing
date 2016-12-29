// Response.h

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

#ifndef _RESPONSE_H
#define _RESPONSE_H

#include <string>
#include <vector>
#include <utility>

namespace freeling {

/// Recognition results holder
struct RecognitionResult {
  /// Confidence value given in percents
  float confidence;
  /// Recognition result text
  std::wstring text;
};

/////////////////////////////////////////////////////////
///
///  Interface for recognition data collector 
///
/////////////////////////////////////////////////////////

class Response {
  public:
    /// Constructor
    Response();
    /// Destructor
    ~Response() {};

    /// Returns only the best result from the decoding process, in pairs of (text, confidence)
    std::pair<std::wstring, float> GetBestResult();

    /// Returns all the results from the decoding process, in pairs of (text, confidence)
    std::vector< std::pair<std::wstring, float> > GetAllResults();

    /// Set final results 
    void SetResult(std::vector<RecognitionResult> &data);

    /// Whether or not the response is an error message
    bool HasError();

    /// Get error message
    std::wstring GetError();
    
    /// Set error value 
    void SetError(const std::wstring &message);

    /// Set of error messages strings. These errors can only happen during the decoding due to
    /// an unexpected decoding event
    static const std::wstring NOT_INTERRUPTED;
    static const std::wstring INTERRUPTED_UNEXPECTED;
    static const std::wstring INTERRUPTED_END_OF_SPEECH;
    static const std::wstring INTERRUPTED_DATA_SIZE_LIMIT;
    static const std::wstring INTERRUPTED_TIMEOUT;

  private:

    /// Holds the results from the decoding process
    std::vector<RecognitionResult> result;

    /// Holds the error message. If it's an empty string, there has not been an error
    std::wstring error;
};

} // namespace

#endif 
