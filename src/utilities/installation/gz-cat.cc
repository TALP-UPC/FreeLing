#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <boost/filesystem.hpp>

#if defined WIN32 
#include "iso646.h"
#endif

using namespace std;
namespace fs = boost::filesystem;

int main(int argc, char *argv[]) {

  if (argc!=3) {
    cerr << "Usage:  " << string(argv[0]) << " /path/to/parameters.XX.gz.aa /path/to/result-file" << endl;
    exit(-1);
  }

  // input filename
  fs::path aafile = fs::path(argv[1]);
  if (not is_regular_file(aafile)) {
    cerr << "file " << aafile << " not found" << endl;
    exit(-1);
  }

  // result file name
  fs::path gzfile = fs::path(argv[2]); 

  fs::path base = aafile.filename().stem();   // get basename without ".aa"
  fs::path dir = aafile.parent_path(); // get directory
  
  // open destination file for writting
  ofstream dest(gzfile.c_str(), ios::out | ios::binary);
  if (dest.fail()) {
    cerr << "Error opening output file " << gzfile << endl;
    exit(-1);
  }    

  // get files in the same directory than input
  vector<fs::path> gzparts;
  copy(fs::directory_iterator(dir), fs::directory_iterator(), back_inserter(gzparts));  
  sort(gzparts.begin(), gzparts.end());

  // add files in the input directory with the same prefix than given .aa file
  const size_t LEN=65536;
  for (vector<fs::path>::const_iterator p=gzparts.begin(); p!=gzparts.end(); ++p) {
    if (p->filename().stem() == base) {
      // the file matches the base name, append it to dest
      char buffer[LEN];
      ifstream src(p->c_str(), ios::in | ios::binary);
      if (src.fail()) {
        cerr << "Error opening input file " << *p << endl;
        exit(-1);
      }    
      while (!src.eof()) {
	src.read(buffer, LEN);
	dest.write(buffer, src.gcount());
      }
      src.close();
    }
  }
  dest.close();

}
