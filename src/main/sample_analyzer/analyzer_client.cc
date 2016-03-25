//////////////////////////////////////////////////////////////////
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

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "socket.h"

using namespace std;

//////////////////////////////////////////////////////////
// process a file (or stdin) sending it all to the server

void process_stream(istream &sin, socket_CS &sock) {

  // process each line from input
  string s,r;
  while (getline(sin,s)) { 
    sock.write_message(s); // send line to server
    sock.read_message(r);  // get response
    if (r!="FL-SERVER-READY") cout<<r; // output response, if any
  }
  
  // input ended. Make sure to flush server's buffer
  sock.write_message("FLUSH_BUFFER");
  sock.read_message(r);
  if (r!="FL-SERVER-READY") cout<<r;
}


//------------------------------------------
int main(int argc, char *argv[]) {

  if (argc < 2) {
    cerr<<"usage: "<<endl;
    cerr<<"   Connect to server in local host:   "<<string(argv[0])<<" port [input-files]"<<endl;
    cerr<<"   Connect to server in remote host:  "<<string(argv[0])<<" host:port [input-files]"<<endl;
    cerr<<"   (if no input-files are provided, stdin is read and processed)"<<endl;
    exit(0);
  }

  // extract host and port from argv[1]
  string p(argv[1]); 
  string host="localhost";
  int port;
  size_t i=p.find(":");
  if (i!=string::npos) {
    host = p.substr(0,i);
    p = p.substr(i+1);
  }
  istringstream ss(p);
  ss>>port;
  
  // connect to server  
  socket_CS sock(host,port);

  // send a sync message to make sure the worker is initialized in the server side
  sock.write_message("RESET_STATS");

  // wait for server answer, so we know it is ready
  string r;
  sock.read_message(r);
  if (r!="FL-SERVER-READY") {
    cerr<<"Server not ready?"<<endl;
    exit(0);
  };

  if (argc==2) 
    // no input files given. Use stdin as a single file
    process_stream(cin,sock);
  
  else {
    // input files provided, process each one in sequence, using the same socket.
    for (int i=2; i<argc; i++) {
      cout << "<OUTPUT SRCFILE=\"" << string(argv[i]) << "\">" << endl;
      ifstream fin;
      fin.open(argv[i]);
      process_stream(fin,sock);
      fin.close();
      cout << "</OUTPUT>" << endl;
    }
  }

  // terminate connection
  sock.close_connection();

  return 0;
}
