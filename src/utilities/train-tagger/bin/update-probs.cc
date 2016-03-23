
#include <iostream>
#include "freeling.h"

using namespace std;
using namespace freeling;

probabilities *prob;
hmm_tagger *tagger;

void process_sentence(sentence &s) {

  prob->analyze(s);
  
  list<sentence> ls; ls.push_back(s);
  
  list<sentence>::iterator f=ls.begin();
  sentence::iterator w;
  
  for (w=f->begin(); w!=f->end(); w++) {
    wcout<<w->get_form()<<L" "<<w->user[0]<<L" "<<w->user[1]<<" #";
    
    for(word::iterator ait=w->selected_begin(); ait!=w->selected_end(); ait++){
      if (ait->is_retokenizable()) {
	list<word> rtk=ait->get_retokenizable();
	list<word>::iterator r;
	wstring lem,par;
	for (r=rtk.begin(); r!= rtk.end(); r++) {
	  lem=lem+L"+"+r->get_lemma();
	  par=par+L"+"+r->get_tag();
	}
	wcout<<L" "<<lem.substr(1)<<L" "<<par.substr(1)<<L" "<<ait->get_prob();
      }
      else {
	wcout<<L" "<<ait->get_lemma()<<L" "<<ait->get_tag()<<L" "<<ait->get_prob();
      }
    }
    wcout<<endl;
  }
  
  wcout<<endl;
}

int main(int argc, char **argv) {

  if (argc!=5) {
    wcerr<<L"\n This program changes the probabilitites of the given corpus according to the\n given probabilities file and threshold.\n";
    wcerr<<L"\n Usage: "<<util::string2wstring(argv[0])<<L" lang threshold probfile locale <corpus >corpus.new \n"<<endl;
    wcerr<<L"  e.g.: "<<util::string2wstring(argv[0])<<L" es 0.001 myprob.dat locale <corpus >corpus.new \n"<<endl;
    exit(0);
  }

  util::init_locale(util::string2wstring(argv[4]));

  wstring lang=util::string2wstring(argv[1]);
  double thresh=util::wstring2double(util::string2wstring(argv[2]));
  wstring pfile=util::string2wstring(argv[3]);

  prob=new probabilities(pfile, thresh);

  wstring line;
  sentence s;
  while (getline(wcin,line)) {

    if (line.empty()) { // sentence end. Anotate and output it.
      process_sentence(s);
      s.clear();
    }
    else {
      wistringstream sin;
      sin.str(line);
      
      wstring form,lemmaOK,tagOK,dummy;
      sin>>form>>lemmaOK>>tagOK>>dummy;
      word w(form);
      w.user.push_back(lemmaOK);
      w.user.push_back(tagOK);
      
      wstring lemma,tag;
      double pr;
      while (sin>>lemma>>tag>>pr) {
	analysis a(lemma,tag);
	a.set_prob(pr);
	w.add_analysis(a);
      }
      
      w.set_found_in_dict(w.get_n_analysis()>0);
      s.push_back(w);
    }
  }

  if (not s.empty())    
    process_sentence(s);

}
