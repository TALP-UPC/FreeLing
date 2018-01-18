#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>
#include <set>
#include <boost/filesystem.hpp>

#if defined WIN32 
#include "iso646.h"
#endif

using namespace std;
namespace fs = boost::filesystem;

int main(int argc, char *argv[]) {

  if (argc!=3) {
    cerr << "Usage:  " << string(argv[0]) << " variant-MWs general-MWs" << endl;
    exit(-1);
  }
  
  fs::path variant = fs::path(argv[1]);
  if (not is_regular_file(variant)) {
    cerr << "file " << variant << " not found" << endl;
    exit(-1);
  }

  fs::path general = fs::path(argv[2]);
  if (not is_regular_file(general)) {
    cerr << "file " << general << " not found" << endl;
    exit(-1);
  }

  // copy all variant file, except closing </Multiwords> tag
  ifstream vf;
  vf.open(variant.c_str());
  string vline;
  while (getline(vf,vline)) {
    if (vline!="</Multiwords>") cout << vline << endl;
  }
  vf.close();

  // copy general file, skipping headers
  ifstream gf;
  gf.open(general.c_str());
  string gline;
  bool inMW=false;
  while (getline(gf,gline)) {
    if (inMW) cout << gline << endl;
    else if (gline=="<Multiwords>") inMW=true;
  }
  gf.close();

}
