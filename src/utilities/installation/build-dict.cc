#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>
#include <set>
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;

int main(int argc, char *argv[]) {

  if (argc<3 or argc>5) {
    cerr << "Usage:  " << string(argv[0]) << " header-file entries-dir [add-dir [remove-dir]]" << endl;
    exit(-1);
  }
  
  fs::path header = fs::path(argv[1]);
  if (not is_regular_file(header)) {
    cerr << "file " << header << " not found" << endl;
    exit(-1);
  }

  fs::path entriesdir = fs::path(argv[2]);
  if (not is_directory(entriesdir)) {
    cerr << entriesdir << " is not a directory." << endl;
    exit(-1);
  }
  
  fs::path adddir;
  if (argc>3) {
    adddir = fs::path(argv[3]);
    if (not is_directory(adddir)) {
      cerr << adddir << " is not a directory." << endl;
      exit(-1);
    }
  }

  fs::path removedir;
  if (argc>4) {
    removedir = fs::path(argv[4]);
    if (fs::exists(removedir) and not is_directory(removedir)) {
      cerr << removedir << " is not a directory." << endl;
      exit(-1);
    }
  }
  
  map<string,set<string> > entries;
  for (fs::directory_iterator efile=fs::directory_iterator(entriesdir); efile!=fs::directory_iterator(); ++efile) {
    ifstream ef;
    ef.open(efile->path().c_str());
    string form,lema,tag;
    while (ef>>form>>lema>>tag) {
      map<string,set<string> >::iterator p = entries.find(form);
      if (p==entries.end()) {
	set<string> s;
	s.insert(lema+" "+tag);
	entries.insert(make_pair(form,s));
      }
      else
	p->second.insert(lema+" "+tag);
    }
    ef.close();
  }

  if (removedir!="" and fs::exists(removedir)) {
    for (fs::directory_iterator efile=fs::directory_iterator(removedir); efile!=fs::directory_iterator(); ++efile) {
      ifstream ef;
      ef.open(efile->path().c_str());
      string form,lema,tag;
      while (ef>>form>>lema>>tag) {
	map<string,set<string> >::iterator p = entries.find(form);
	if (p!=entries.end()) p->second.erase(lema+" "+tag);
      }
      ef.close();    
    }
  }
  
  if (adddir!="") {
    for (fs::directory_iterator efile=fs::directory_iterator(adddir); efile!=fs::directory_iterator(); ++efile) {
      ifstream ef;
      ef.open(efile->path().c_str());
      string form,lema,tag;
      while (ef>>form>>lema>>tag) {
	map<string,set<string> >::iterator p = entries.find(form);
	if (p==entries.end()) {
	  set<string> s;
	  s.insert(lema+" "+tag);
	  entries.insert(make_pair(form,s));
	}
	else
	  p->second.insert(lema+" "+tag);
      }
      ef.close();    
    }
  }
    
  ifstream h;
  h.open(header.c_str());
  string line;
  while (getline(h,line)) cout << line << endl;
  h.close();
  for (map<string,set<string> >::const_iterator p=entries.begin(); p!=entries.end(); ++p) {
    cout << p->first;
    for (set<string>::const_iterator a=p->second.begin(); a!=p->second.end(); ++a) cout << " " << (*a) ;
    cout << endl;
  }
  cout << "</Entries>" << endl;
}
