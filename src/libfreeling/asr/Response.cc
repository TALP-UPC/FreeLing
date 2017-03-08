#include "freeling/morfo/asr/Response.h"

namespace freeling {

  Response::Response() {
    error = L"";
  }

  std::pair<std::wstring, float> Response::GetBestResult() {
    return std::make_pair(result.at(0).text, result.at(0).confidence);
  }

  std::vector< std::pair<std::wstring, float> > Response::GetAllResults() {
    std::vector< std::pair<std::wstring, float> > res;
    for (int i = 0; i < result.size(); i++)
        res.push_back(std::make_pair(result.at(i).text, result.at(i).confidence)); 
    return res;
  }

  void Response::SetResult(std::vector<RecognitionResult> &data) {
    result = data;
  }

  bool Response::HasError() {
    return error != L"";
  }

  std::wstring Response::GetError() {
    return error;
  }
  
  void Response::SetError(const std::wstring &message) {
    error = message;
  }

}
