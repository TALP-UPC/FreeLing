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

#ifdef WIN32
  #include <io.h>
  #include <fcntl.h>
#else
  #include <unistd.h>
#endif 
#include <boost/thread.hpp>
#include "freeling.h"

#ifdef WIN32
  #define pipe(p1) _pipe(p1, 1024, O_BINARY);
  #define close(p1) _close(p1);
  #define write(p1, p2, p3) _write(p1, p2, p3);
  #define read(p1, p2, p3) _read(p1, p2, p3); 
#endif

using namespace freeling;

//////////////////////////////////////////////////////////////////////
/// Class FL_pipe is used to pass objects from one thread to another. 
/// The passed objects are pointers to actual data in shared memory, 
/// to avoid repeated copying.
//////////////////////////////////////////////////////////////////////

class FL_pipe {
private:
  static const int READ=0;
  static const int WRITE=1;
  int fd[2];

public:
  /// constructor
  FL_pipe() {
    int x= pipe(fd);
    if (x == -1) {
      std::wcerr<<L"Error creating pipe."<<std::endl;
      exit(1);
	}
  }
  /// destructor
  ~FL_pipe() { }

  /// read from pipe. Return NULL if EOF
  void* receive() {
    void* x;
    int n=read(fd[READ],(void*)&x,sizeof(void*));
    if (n==0) x=NULL;
    return x;
  }

  // write to pipe
  size_t send(void* x) { return write(fd[WRITE], &x, sizeof(void*));}
  // close write end of the pipe
  void close_write() { close(fd[WRITE]); }
  // close read end of the pipe
  void close_read() { close(fd[READ]); }
};


//////////////////////////////////////////////////////////////////////
/// Class threaded_processor wraps into a thread any FreeLing module
//////////////////////////////////////////////////////////////////////

template<class T,class D=sentence>
class threaded_processor {
private:
  // processor to use for the actual work
  T *proc;
  // piped I/O channels for this thread 
  FL_pipe &ch_in;
  FL_pipe &ch_out;

  /// Get data from pipe "i", process them using 
  /// module "mod", send results to pipe "o".
  static void do_analysis(T *mod, FL_pipe &i, FL_pipe &o);
  
public: 
  /// constructor: Store channels and processor, and 
  /// launch a thread matching the processor type.
  threaded_processor<T,D> (T *mod, FL_pipe &i, FL_pipe &o) : proc(mod), ch_in(i), ch_out(o) { 
    boost::thread worker(do_analysis,proc,ch_in,ch_out);  
  }

};


//////// thread functions for each kind of threaded_processor<T> //////////////

/// ---------------------------------------------
/// Thread function for T=tokenizer.
/// Reads wstring from ch_in, tokenizes them, and
/// sends resulting list<word> to ch_out.
/// ---------------------------------------------

template<>
void threaded_processor<tokenizer>::do_analysis(tokenizer *proc, FL_pipe &ch_in, FL_pipe &ch_out) {
  std::wstring *s;
  
  // read pipe until we get a NULL.
  while ( (s = (std::wstring*) ch_in.receive()) ) {
    std::list<word> *y = new std::list<word>();
    proc->tokenize(*s,*y);
    ch_out.send((void*)y);
    delete s; 
  }
  // no more to read
  ch_in.close_read();
  // let the next module know
  ch_out.close_write();
}

/// ---------------------------------------------
/// Thread function for T=splitter.
/// Reads list<word> from ch_in, splits them, and
/// sends resulting list<sentence> to ch_out.
/// ---------------------------------------------

template<>
void threaded_processor<splitter>::do_analysis(splitter *proc, FL_pipe &ch_in, FL_pipe &ch_out) {
  std::list<word>* x;
  std::list<sentence>* y;
  
  // open splitter session for this thread
  splitter::session_id ses = proc->open_session();

  // read pipe until we get a NULL.
  while ( (x = (std::list<word>*)ch_in.receive()) ) {
    y = new std::list<sentence>();
    proc->split(ses, *x, false, *y);
    ch_out.send((void*)y);
    delete x;
  }
  // no more to read
  ch_in.close_read();
  // empty splitter buffer
  y = new std::list<sentence>();
  proc->split(ses, std::list<word>(), true, *y);
  // send results
  ch_out.send((void*)y);
  // let the next module know we're done.
  ch_out.close_write();

  // close splitter session for this thread
  proc->close_session(ses);
}

/// ---------------------------------------------
/// Thread function for T=processor<D>, and D=data type (sentence/document).
/// Reads list<sentence> from ch_in, analyzes them, 
/// and sends updated list<sentence> to ch_out.
/// ---------------------------------------------

template<class T,class D>
void threaded_processor<T,D>::do_analysis(T *proc, FL_pipe &ch_in, FL_pipe &ch_out) {
  std::list<D>* x;
  
  // read pipe until we get a NULL.
  while ( (x = (std::list<sentence>*)ch_in.receive()) ) {
    proc->analyze(*x);
    ch_out.send((void*)x);
  }
  // no more to read
  ch_in.close_read();
  // let the next module know
  ch_out.close_write();
}

