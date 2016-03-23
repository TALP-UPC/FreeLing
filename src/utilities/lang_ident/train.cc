//////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public License
//    (GNU GPL) as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License 
//    along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Muntsa Padro (mpadro@lsi.upc.edu)
//             TALP Research Center
//             despatx Omega.S107 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>

#include "freeling/morfo/util.h"
#include "freeling/morfo/lang_ident.h"


using namespace std;

int main(int argc, char* argv[]){

  freeling::util::init_locale(L"default");
	
  if (argc<2 or argc>3) {
    wcerr<<L"Usage:  train lang_code [model_file] <text" << endl; 
    exit(1);
  }

  // iso language code
  wstring code = freeling::util::string2wstring(argv[1]); 
  // model file
  wstring outfile = code+L".dat";
  if (argc==3) 
    outfile = freeling::util::string2wstring(argv[2]); 

  // create language model, and train it.
  freeling::idioma id;
  id.train(wcin, outfile, code);
}
