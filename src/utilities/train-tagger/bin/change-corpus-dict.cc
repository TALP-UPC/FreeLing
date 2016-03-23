
#include <iostream>
#include "freeling.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;
using namespace freeling;


list<analysis> printRetokenizable(const list<word> &rtk, list<word>::const_iterator w, const wstring &lem, const wstring &tag) {
  
  list<analysis> s;
  if (w==rtk.end()) 
    s.push_back(analysis(lem.substr(1),tag.substr(1)));
		
  else {
    list<analysis> s1;
    list<word>::const_iterator w1=w; w1++;
    for (word::const_iterator a=w->begin(); a!=w->end(); a++) {
      s1=printRetokenizable(rtk, w1, lem+L"+"+a->get_lemma(), tag+L"+"+a->get_tag());
      s.splice(s.end(),s1);
    }
  }
  return s;
}  


int main(int argc, char *argv[]) {

  util::init_locale(L"default");

  wstring form,lemma,tag,dummy;
  wstring lang(util::string2wstring(argv[1]));
  wstring path(util::string2wstring(argv[2]));
  path=path+lang+L"/";

  traces::TraceLevel=0;
  traces::TraceModule=0xFFFFF;

  // morphological analysis has a lot of options, and for simplicity they are packed up
  // in a maco_options object. First, create the maco_options object with default values.
  maco_options opt(lang);  
  // and provide files for morphological submodules. Note that it is not necessary
  // to set opt.QuantitiesFile, since Quantities module was deactivated.
  opt.UserMapFile=L"";
  opt.LocutionsFile=L""; opt.AffixFile=path+L"";
  opt.ProbabilityFile=path+L"probabilitats.dat"; opt.DictionaryFile=path+L"dicc.src";
  opt.NPdataFile=L""; opt.PunctuationFile=L"";

  // create the analyzer with the just build set of maco_options
  maco morfo(opt); 
  // then, set required options on/off  
  morfo.set_active_options (false,// UserMap
                            true, // NumbersDetection,
                            false, //  PunctuationDetection,
                            false, //  DatesDetection,
                            true, //  DictionarySearch,
                            false, //  AffixAnalysis,
                            false, //  CompoundAnalysis,
                            false, //  RetokContractions,
                            false, //  MultiwordsDetection,
                            false, //  NERecognition,
                            false, //  QuantitiesDetection,
                            true);  //  ProbabilityAssignment

  wstring line;
  while (getline(wcin,line)) {

    if (line.empty()) 
      // empty line, keep it (sentence split)
      wcout<<endl;

    else {

      wistringstream sin;
      sin.str (line);
      
      sin >> form >> lemma >> tag >> dummy; 	// get word form, plus "#"
      
      if (tag[0]==L'Z' or tag[0]==L'W' or tag[0]==L'F' or tag.substr(0,2)==L"NP" or form.find(L"_")!=wstring::npos)
	// Z, W, F, NP, or multiword:  keep as is.
	wcout<<line<<endl;
      
      else {
	// other words, analyze
	
	word w(form);
	sentence s; s.push_back(w);
	list<sentence> ls; ls.push_back(s);
	
	morfo.analyze(ls);
	wcout<<form<<L" "<<lemma<<L" "<<tag<<L" "<<dummy;
	for (word::iterator a=ls.begin()->begin()->begin(); a!=ls.begin()->begin()->end(); a++) {

	  if (a->is_retokenizable ()) {
	    list <word> rtk = a->get_retokenizable ();
	    list <analysis> la=printRetokenizable(rtk, rtk.begin(), L"", L"");
	    for (list<analysis>::iterator x=la.begin(); x!=la.end(); x++) {
	      wcout << L" " << x->get_lemma() << L" " << x->get_tag() << L" " << a->get_prob()/la.size();
	    }
	  }
	  else 
	    wcout<<L" "<<a->get_lemma()<<L" "<<a->get_tag()<<L" "<<a->get_prob();
	}
	wcout<<endl;      
      } 
    }
  }
}
