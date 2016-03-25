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

#include <sstream>
#include <fstream>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/senses.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"SENSES"
#define MOD_TRACECODE SENSES_TRACE


  ///////////////////////////////////////////////////////////////
  ///  Create the sense annotator
  ///////////////////////////////////////////////////////////////

  senses::senses(const wstring & wsdFile) {
 
    duplicate=false;
    semdb = new semanticDB(wsdFile);

    enum sections {DUP_ANALYSIS};
    config_file cfg(true);  
    cfg.add_section(L"DuplicateAnalysis",DUP_ANALYSIS);

    if (not cfg.open(wsdFile))
      ERROR_CRASH(L"Error opening file "+wsdFile);

    // read sense configuration file 
    wstring line; 
    while (cfg.get_content_line(line)) {
      switch (cfg.get_section()) {
      case DUP_ANALYSIS: {
        wistringstream sin;
        sin.str(line);
        wstring key; 
        sin>>key;
        if (key==L"yes") duplicate=true;  
        break;
      }
      default: break;
      }
    }
    cfg.close();
 
    TRACE(1,L"analyzer succesfully created");
  }

  ///////////////////////////////////////////////////////////////
  ///  Destructor
  ///////////////////////////////////////////////////////////////

  senses::~senses() {
    delete semdb;
  }

  ///////////////////////////////////////////////////////////////
  ///  Analyze given sentences.
  ///////////////////////////////////////////////////////////////  

  void senses::analyze(sentence &s) const {
    list<wstring> lsen;  

    // anotate with all possible senses, no disambiguation
    sentence::iterator w;
    for (w=s.begin(); w!=s.end(); w++) {
    
      list<analysis> newla;
      word::iterator a;
      for (a=w->begin(); a!=w->end(); a++) {
      
        lsen=semdb->get_word_senses(w->get_lc_form(), a->get_lemma(), a->get_tag());

        if (lsen.size()==0) {
          // no senses found for that lemma.
          if (duplicate) newla.push_back(*a);
        }
        else {
          // senses found for that lemma
          if (duplicate) {
            list<wstring> ss;
            list<wstring>::iterator s=lsen.begin(); 
            double newpr= a->get_prob()/lsen.size();
          
            for (s=lsen.begin(); s!=lsen.end(); s++) {
              // create a copy of the analysis for each sense
              analysis *newan = new analysis(*a);
              // add current sense to new analysis, overwriting.
              ss.clear(); ss.push_back(*s);  
            
              list<wstring>::iterator lsi;
              list<pair<wstring, double> > lsen_noranks;
              for (lsi = ss.begin(); lsi != ss.end(); lsi++) {
                lsen_noranks.push_back(make_pair(*lsi, 0.0));
              }
              newan->set_senses(lsen_noranks); 
            
              newan->set_prob(newpr);
              // add new analysis to the new list
              newla.push_back(*newan);
            
              TRACE(3, L"  Duplicating analysis for sense "+(*s));
            }
          }
          else {
            // duplicate not set. Add the the whole sense list to current analysis
            list<wstring>::iterator lsi;
            list<pair<wstring, double> > lsen_noranks;
            for (lsi = lsen.begin(); lsi != lsen.end(); lsi++) {
              lsen_noranks.push_back(make_pair(*lsi, 0.0));
            }
            a->set_senses(lsen_noranks);
          }
        }
      }
    
      // if duplicate is on, use the newly rebuild analysis list
      if (duplicate) w->set_analysis(newla);
    }

    TRACE(1,L"Sentences annotated by the senses module.");
  }

} // namespace
