#ifndef _RESPONSE_H
#define _RESPONSE_H

#include <string>
#include <vector>
#include <utility>

namespace freeling {

/**
 * Recognition results holder
 */
struct RecognitionResult {
  /**
   * Confidence value given in percents
   */
  float confidence;
  /**
   * Recognition result text
   */
  std::wstring text;
};

/**
 * Interface for recognition data collector
 */
class Response {
  public:
    Response();
    ~Response() {};

    std::pair<std::wstring, float> GetBestResult();

    std::vector< std::pair<std::wstring, float> > GetAllResults();

    /** Set final results */
    void SetResult(std::vector<RecognitionResult> &data);

    // Whether or not the response is an error message
    bool HasError();
    /// Get error message
    std::wstring GetError();
    /** Set error value */
    void SetError(const std::wstring &message);

    static const std::wstring NOT_INTERRUPTED;
    static const std::wstring INTERRUPTED_UNEXPECTED;
    static const std::wstring INTERRUPTED_END_OF_SPEECH;
    static const std::wstring INTERRUPTED_DATA_SIZE_LIMIT;
    static const std::wstring INTERRUPTED_TIMEOUT;

  private:
    std::vector<RecognitionResult> result;
    std::wstring error;
};

} /* namespace apiai */

#endif /* RESPONSE_H_ */
