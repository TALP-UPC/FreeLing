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

#ifndef _ASR_H
#define _ASR_H

#include "freeling.h"

#include "freeling/morfo/util.h"
#include "Request.h"
#include "Response.h"
#include "Nnet3LatgenFasterDecoder.h"
#include "util/simple-options.h"
#include "base/kaldi-types.h"
#include "matrix/kaldi-vector.h"
#include <sstream>
#include <fstream>
#include <utility>

namespace freeling {

/////////////////////////////////////////////////////////
///
///  asr class receives an audio and returns
///  its transcription as an string
///
/////////////////////////////////////////////////////////


class asr {
  public:

  	/// constructor
  	asr(const wstring &configFile);
  	/// destructor
  	~asr();

    /// Decode function with in the wavfile in the form of a string
    std::vector< std::pair<std::wstring, float> > decode(const string &wav_file, int nbest, int decode_channel, float seconds_to_decode);
    
    /// Decode function with a istream as the input. The istream must be opened in binary mode
    std::vector< std::pair<std::wstring, float> > decode(std::istream &in_stream, int nbest, int decode_channel, float seconds_to_decode);

    // Returns the frequency of the decoder contained in the class 
    int getFrequency() const;

  private:
    /// Reference to the decoder contained in the class, which will do all the decoding work
    Decoder *decoder;

    /// Reference to the wrapper request class, used by the decoder 
    Request* request;

    /// Reference to the wrapper response class, used by the decoder, which will contain the results
    Response* response;

    /// Frequency of the decoder contained in the class
    int frequency_;
};

} // namespace

#endif