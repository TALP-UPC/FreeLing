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

///////////////////////////////////////////////
//
//   Author: Jordi Turmo (turmo@lsi.upc.edu)
//
//   This class is an implementation based on the work done by Emili Sapena 
//   in his PhD. thesis on coreference resolution
//
///////////////////////////////////////////////

#include <fstream>
#include <sstream>
#include <algorithm> //sort
#include <ctime>

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/relaxcor.h"
#include "freeling/morfo/relax.h"
#include "freeling/morfo/configfile.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"RELAXCOR"
#define MOD_TRACECODE COREF_TRACE

  //////////////////////////////////////////////////////////////
  /// Destructor
  ///////////////////////////////////////////////////////////////

  relaxcor::~relaxcor() {
    delete model;
    delete detector;
    delete extractor;
  }

  ///////////////////////////////////////////////////////////////
  /// Create a relaxcor module, loading appropriate files.
  ///////////////////////////////////////////////////////////////

  relaxcor::relaxcor(const wstring &filename) {
    // load regular config file

    // by default: singletons will not be provided
    provide_singletons = false;

    wstring language;
    wstring fmention_detector; // mention detector file
    wstring ffeature_extractor; // feature extractor file
    wstring fmodel; // relaxcor model file

    enum sections {LANGUAGE, MENTION_DETECTOR, FEATURE_EXTRACTOR, MODEL, 
                   MAX_ITER, SCALE_FACTOR, EPSILON, SINGLE_FACTOR, N_PRUNE};

    // read configuration file and store information.
    // do not allow undeclared sections.
    config_file cfg(false, L"%");  

    cfg.add_section(L"Language",LANGUAGE,true);
    cfg.add_section(L"MentionDetector",MENTION_DETECTOR,true);
    cfg.add_section(L"FeatureExtractor",FEATURE_EXTRACTOR,true);
    cfg.add_section(L"Model",MODEL,true);
    cfg.add_section(L"MaxIter",MAX_ITER,true);
    cfg.add_section(L"ScaleFactor",SCALE_FACTOR,true);
    cfg.add_section(L"Epsilon",EPSILON,true);
    cfg.add_section(L"SingleFactor",SINGLE_FACTOR,true);
    cfg.add_section(L"Nprune",N_PRUNE,true); 

    if (not cfg.open(filename)) ERROR_CRASH(L"Error opening file "+filename);

    map<unsigned int, bool> exists_section;
    wstring path=filename.substr(0,filename.find_last_of(L"/\\")+1);
    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;  
      sin.str(line);
      
      switch (cfg.get_section()) { 

      case LANGUAGE: {
	// Read the language
        sin>>language;
	break;
      }
      case MENTION_DETECTOR: {
	// load mention detector data file
	wstring fname;
	sin>>fname;
	fmention_detector = util::absolute(fname,path);
	break;
      }
      case FEATURE_EXTRACTOR: {
	// load feature extractor data file
	wstring fname;
	sin>>fname;
	ffeature_extractor = util::absolute(fname,path);
	break;
      }
      case MODEL: {
	// load the model files
	wstring fname;
	sin>>fname;
	fmodel = util::absolute(fname,path);
	break;
      }
      case MAX_ITER: {
        sin>>_Max_iter;
	break;
      }
      case SCALE_FACTOR: {
	sin>>_Scale_factor;
	break;
      }
      case EPSILON: {
	sin>>_Epsilon;
	break;
      }
      case SINGLE_FACTOR: {
	sin>>_Single_factor;
	break;
      }
      case N_PRUNE: {
	sin>>_Nprune;
	break;
      }
      default: break;
      }
    }
    cfg.close();
  
    // load model
    model = new coref_model(fmodel);
    // create helper modules
    detector = new mention_detector(fmention_detector);
    extractor = new relaxcor_fex(ffeature_extractor, (*model) );

    TRACE(3,L"analyzer succesfully created");
  }

  ///////////////////////////////////////////////////////////////
  /// Create a relaxcor module for tuning the weights of a model..  
  /// DEPRECATED
  ///////////////////////////////////////////////////////////////

  //relaxcor::relaxcor(const wstring &filename, int Nprune, int Balance, const wstring &fconstraints) {
  //  // load config file in tuning mode (no Nprune required)
  //  load_config_file(filename,true);
  //
  //  // set _Nprune to given parameter
  //  _Nprune = Nprune;
  //  // load constraints from fconstraint, overwriting existing constraints (if any)
  //  model->load_constraints(fconstraints,Balance);
  //
  //  TRACE(3,L"analyzer succesfully created");
  // }


  ///////////////////////////////////////////////////////////
  /// Auxiliary for constructors
  /// DEPRECATED

  // void relaxcor::load_config_file(const wstring &filename, bool tuning) {
  //   wstring language;
  //   wstring fmention_detector; // mention detector file
  //   wstring ffeature_extractor; // feature extractor file
  //   wstring fmodel; // relaxcor model file

  //   enum sections {LANGUAGE, MENTION_DETECTOR, FEATURE_EXTRACTOR, MODEL, 
  //                  MAX_ITER, SCALE_FACTOR, EPSILON, SINGLE_FACTOR, N_PRUNE};

  //   // read configuration file and store information   
  //   // if tuning, allow and ignore undeclared sections (e.g. Nprune)
  //   // if not tuning, do not allow undeclared sections.
  //   config_file cfg(tuning, L"%");  

  //   cfg.add_section(L"Language",LANGUAGE,true);
  //   cfg.add_section(L"MentionDetector",MENTION_DETECTOR,true);
  //   cfg.add_section(L"FeatureExtractor",FEATURE_EXTRACTOR,true);
  //   cfg.add_section(L"Model",MODEL,true);
  //   cfg.add_section(L"MaxIter",MAX_ITER,true);
  //   cfg.add_section(L"ScaleFactor",SCALE_FACTOR,true);
  //   cfg.add_section(L"Epsilon",EPSILON,true);
  //   cfg.add_section(L"SingleFactor",SINGLE_FACTOR,true);
  //   if (not tuning)
  //     cfg.add_section(L"Nprune",N_PRUNE,true); 

  //   if (not cfg.open(filename)) ERROR_CRASH(L"Error opening file "+filename);

  //   map<unsigned int, bool> exists_section;
  //   wstring path=filename.substr(0,filename.find_last_of(L"/\\")+1);
  //   wstring line;
  //   while (cfg.get_content_line(line)) {

  //     wistringstream sin;  
  //     sin.str(line);
      
  //     switch (cfg.get_section()) { 

  //     case LANGUAGE: {
  //       // Read the language
  //       sin>>language;
  //       break;
  //     }
  //     case MENTION_DETECTOR: {
  //       // load mention detector data file
  //       wstring fname;
  //       sin>>fname;
  //       fmention_detector = util::absolute(fname,path);
  //       break;
  //     }
  //     case FEATURE_EXTRACTOR: {
  //       // load feature extractor data file
  //       wstring fname;
  //       sin>>fname;
  //       ffeature_extractor = util::absolute(fname,path);
  //       break;
  //     }
  //     case MODEL: {
  //       // load the model files
  //       wstring fname;
  //       sin>>fname;
  //       fmodel = util::absolute(fname,path);
  //       break;
  //     }
  //     case MAX_ITER: {
  //       sin>>_Max_iter;
  //       break;
  //     }
  //     case SCALE_FACTOR: {
  //       sin>>_Scale_factor;
  //       break;
  //     }
  //     case EPSILON: {
  //       sin>>_Epsilon;
  //       break;
  //     }
  //     case SINGLE_FACTOR: {
  //       sin>>_Single_factor;
  //       break;
  //     }
  //     case N_PRUNE: {
  //       sin>>_Nprune;
  //       break;
  //     }
  //     default: break;
  //     }
  //   }
  //   cfg.close();
    
  //   // load model
  //   model = new coref_model(fmodel,0,tuning);
  //   //    detector = new mention_detector(language, fmention_detector);
  //   detector = new mention_detector(fmention_detector, language);
  //   extractor = new relaxcor_fex(ffeature_extractor, model, language);
  // } 


  /////////////////////////////////////////////////////////////////////////////
  // set provide_singletons
  /////////////////////////////////////////////////////////////////////////////
 
  void relaxcor::set_provide_singletons(bool value) {
    provide_singletons = value;
  }

  /////////////////////////////////////////////////////////////////////////////
  // get provide_singletons
  /////////////////////////////////////////////////////////////////////////////

  bool relaxcor::get_provide_singletons() const{
    return provide_singletons;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// return the offset of a mention
  /////////////////////////////////////////////////////////////////////////////

  int relaxcor::offset(const mention &m, unsigned int side) {
    if (side == 0)
      return m.get_n_sentence()*1000 + m.get_pos_end();
    else 
      return m.get_n_sentence()*1000 + m.get_pos_begin();
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Not much sense in using this, but it is virtual so we need to define it
  /////////////////////////////////////////////////////////////////////////////

  void relaxcor::analyze(sentence &s) const {
    ERROR_CRASH (L"Coreference solver requires a document, not a sentence. Please call relaxcor::analyze(document &d).");
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Detect coreference chains in a given document.
  /////////////////////////////////////////////////////////////////////////////

  void relaxcor::analyze(document &doc) const {

    TRACE(3,L"Detecting mentions");

    // searching for mentions
    clock_t t0 = clock();  // initial time
    vector<mention> mentions = detector->detect(doc);
    clock_t t1 = clock();  // final time
    TRACE(3,L"detection time: "+util::double2wstring(double(t1-t0)/double(CLOCKS_PER_SEC)));

    // extracting features of mention-pairs
    TRACE(3,L"Extracting features");
    t0 = clock();  // initial time
    relaxcor_fex_abs::Mfeatures M = extractor->extract(mentions);
    t1 = clock();  // final time
    TRACE(3,L"extraction time: "+util::double2wstring(double(t1-t0)/double(CLOCKS_PER_SEC)));
   
    // print features
    //print(M, mentions.size());
    
    TRACE(3,L"Ready to create chains");

    // coreferent chains, useful when removing singletons
    map<int, vector<int> > chains;

    if (mentions.size() < 2) {
      mentions[0].set_group(0);
      chains[0] = vector<int>({0});
    }
    else {

    ///////////////////////
    // building the problem
    ///////////////////////
 
    problem coref_problem(mentions.size());

    t0 = clock();  // initial time

    // first, they have to be sorted: proper nouns -> noun phrases+composites -> pronouns
    vector<vector<mention>::const_iterator> sorted_mentions;
    sorted_mentions.reserve(mentions.size());

    for (vector<mention>::const_iterator it=mentions.begin(); it!=mentions.end(); it++) {
      if (it->is_type(mention::PROPER_NOUN))
        sorted_mentions.push_back(it);
    }
    for (vector<mention>::const_iterator it=mentions.begin(); it!=mentions.end(); it++) {
      if (it->is_type(mention::NOUN_PHRASE) or it->is_type(mention::COMPOSITE))
        sorted_mentions.push_back(it);
    }
    for (vector<mention>::const_iterator it=mentions.begin(); it!=mentions.end(); it++) {
      if (it->is_type(mention::PRONOUN))
        sorted_mentions.push_back(it);
    }
 
    t1 = clock();  // final time
    TRACE(3,L"building problem: sort mentions time: "+util::double2wstring(double(t1-t0)/double(CLOCKS_PER_SEC)));

    ////////////////////////////////////
    // adding vextexes (mentions)
    // add one vertex per sorted mention and create the set of its labels (ie, all the previous mentions)
    ////////////////////////////////////

    t0 = clock();  // initial time

    for (unsigned int m=0; m<sorted_mentions.size(); m++) {

      TRACE(3, L"building problem: adding vertex " + util::int2wstring(m) +
               L" = mention " + util::int2wstring(sorted_mentions[m]->get_id()) + 
               L" (" +sorted_mentions[m]->value() + L")");

      double prob;      
      double mult;
      if (sorted_mentions[m]->is_type(mention::PRONOUN)) {
        // if not a pronoun, all m+1 labels have uniform probability
        prob = 1.0/(m+1);
        mult = 1;
      }
      else {
        // if not a pronoun, last label will have twice the probability than the others
        prob = 1.0/(m+2);
        mult = 2;
      }

      // adding labels with equiprobable initial probabilities for each vertex 
      for (unsigned int l=0; l<m; l++) {
        coref_problem.add_label(m,prob);
        TRACE(3,L"   adding label " + util::int2wstring(l) + 
                L" to vertex "+ util::int2wstring(m) +
                L" prob=" + util::double2wstring(prob));
      }
      // last label may have twice the probability if it was not a pronoun
      coref_problem.add_label(m,mult*prob);
      TRACE(3,L"   adding label " + util::int2wstring(m) + 
              L" to vertex "+ util::int2wstring(m) +
              L" prob=" + util::double2wstring(mult*prob));
    }

    t1 = clock();  // final time
    TRACE(3,L"building problem: adding vertexes time: "+util::double2wstring(double(t1-t0)/double(CLOCKS_PER_SEC)));

    //////////////////////////////////////////////////////////////////////////////////
    // creating edges
    // create one edge from mention m and a previous one, m_ant, using sorted_mentions
    //////////////////////////////////////////////////////////////////////////////////

    t0 = clock();  // initial time

    typedef vector< pair<pair<int, int >, double> > Tadjacents;
    int nedges=0;
    int nconstr=0;
        
    for (unsigned int m=0; m<sorted_mentions.size(); m++) {
      unsigned int Npos=0; // number of positive edges (used for prunning)
      unsigned int Nneg=0; // number of negative edges (used for prunning)
      Tadjacents adjacents;
      adjacents.reserve(sorted_mentions.size()*(sorted_mentions.size()-1)/2);

      TRACE(4, L"Looking for edges for mention " + 
               util::int2wstring(sorted_mentions[m]->get_id()) + 
               L" (" + sorted_mentions[m]->value() + L")" );

      for (unsigned int m_ant=0; m_ant<m; m_ant++) {
	  
	int anaphor = max (sorted_mentions[m]->get_id(), sorted_mentions[m_ant]->get_id());
	int antecedent = min (sorted_mentions[m]->get_id(), sorted_mentions[m_ant]->get_id());
	if (anaphor == antecedent)
	  ERROR_CRASH(L"Two different mentions with the same identifier");
	
	// computing the weight of the edge as the sum of compatibilities of the satisfied constraints
	wstring mp = util::int2wstring(anaphor) + L":" + util::int2wstring(antecedent);
        TRACE(7, L"  Checking constraints for pair " << mp << 
              L" (" << sorted_mentions[m]->value() << L"," << sorted_mentions[m_ant]->value() << L")");
        TRACE(7,L"     Pair features:  ");
        TRACE(7,L"        active  : " << model->print(M[mp],true) );
        TRACE(7,L"        inactive: " << model->print(M[mp],false) );
	double w = model->weight(M[mp]);

	if (w>0) Npos++;
	if (w<0) Nneg++;
	// create an adjacent beween m and m_ant (only if w!=0)
	if (w!=0) adjacents.push_back(make_pair(make_pair(m,m_ant), w));	
      }
          
      // wcerr << "before:" << "pos=" << Npos << " neg=" << Nneg << endl;
      // for (unsigned int i=0; i<adjacents.size(); i++) 
      //   wcerr << L"(" << adjacents[i].first.first << L"," <<  adjacents[i].first.second << L") - "  << adjacents[i].second << endl;
      
      TRACE(4,L"  Found "+util::int2wstring(adjacents.size())+L" edges for mention "+util::int2wstring(sorted_mentions[m]->get_id())+L" ("+sorted_mentions[m]->value()+L")");

      // If only negative arcs for this mention, it will end up a singleton. 
      // Leave it alone (no edges) to speed up solving.
      if (Npos==0 and Nneg>0) {
        TRACE(4,L"  All edges are negative. Removing.");
        adjacents.clear();
      }        
      
      ///////////////////////////////////////
      // Prunning edges if required
      // Keep only the N most relevant edges.
      // (the N/2 most relevant positive edges)
      ///////////////////////////////////////

      if (_Nprune > 0 and adjacents.size() > _Nprune) {
	sort(adjacents.begin(), adjacents.end(), util::ascending_second<pair<int,int>,double>);

	unsigned int sel_Pos = (Npos > _Nprune/2) ? _Nprune/2 : Npos;
	unsigned int sel_Neg = (Nneg > _Nprune-sel_Pos) ? _Nprune-sel_Pos : Nneg;
	if (sel_Pos+sel_Neg < _Nprune) sel_Pos=_Nprune-sel_Neg;

	Tadjacents::iterator begin_it = adjacents.begin(); 
	begin_it += sel_Neg;
	Tadjacents::iterator end_it = adjacents.end();
	end_it = (sel_Pos==0)? end_it : end_it-sel_Pos;   

	adjacents.erase(begin_it,end_it);
	
	// wcerr << "after:" << "selNeg=" << sel_Neg << " selPos=" << sel_Pos << endl;
	// for (unsigned int i=0; i<adjacents.size(); i++) {wcerr << L"(" << adjacents[i].first.first << L"," <<  adjacents[i].first.second << L") - "  << adjacents[i].second << " ";}
	//  wcerr << endl;	
      }

      TRACE(4,L"  "+util::int2wstring(adjacents.size())+L" edges remaining after pruning with Nprune="+util::int2wstring(_Nprune));
    
      ////////////////////////////////////
      // Adding constraints to the problem from the edges 
      // Add constraints (m_ant, label, weight) to each (m, label)
      ////////////////////////////////////

      for (Tadjacents::const_iterator it=adjacents.begin(); it!=adjacents.end(); it++) {
	int m = it->first.first;
	int m_ant = it->first.second;
	double w = it->second;
        TRACE(4,L"   Creating constraints for edge ("+util::int2wstring(sorted_mentions[m]->get_id())+L","+util::int2wstring(sorted_mentions[m_ant]->get_id())+L") = "+util::double2wstring(w));
        nedges++;
        
	for (int l=0; l<=m_ant; l++) {
	  list<pair<int,int> > constraint;
	  list<list<pair<int,int> > > lconstraint;

	  // backward constraint
	  constraint.push_back(make_pair(m_ant,l));
	  lconstraint.push_back(constraint);
	  coref_problem.add_constraint(m,l,lconstraint,w);

          nconstr+=1;
	}
      }
    }

    t1 = clock();  // final time
    TRACE(3,L"building problem. Added "+util::int2wstring(nconstr)+L" constraints for "+util::int2wstring(nedges)+L" edges");
    TRACE(3,L"building problem: adding constraints time: "+util::double2wstring(double(t1-t0)/double(CLOCKS_PER_SEC)));

    /////////////////////////////////
    // Solving the coreference chains
    /////////////////////////////////

    t0 = clock();  // initial time

    relax coref_solver(_Max_iter, _Scale_factor, _Epsilon);
    coref_solver.solve(coref_problem);

    t1 = clock();  // final time
    TRACE(3,L"solving time: "+util::double2wstring(double(t1-t0)/double(CLOCKS_PER_SEC)));

    ////////////////////////////////////////////
    // Adding coreference chains to the document
    ////////////////////////////////////////////

    t0 = clock();  // initial time

    for (unsigned int m=0; m<sorted_mentions.size(); m++) {

      int id, group;
      list<int> best_sorted_groups = coref_problem.best_label(m);

      if (best_sorted_groups.size()==1) {
	int best = best_sorted_groups.front();
	id = sorted_mentions[m]->get_id();
	group = sorted_mentions[best]->get_id();
      }
      else {

      // from best_groups, find the group which includes the mention closest to m, preferable by which is closer to the left
	list<int>::const_iterator it = best_sorted_groups.begin();
	int best_left = -1;
	int best_right = -1;
	int dist_best_left = 100000;
	int dist_best_right = 100000;

	id = sorted_mentions[m]->get_id();
	
	for (it= best_sorted_groups.begin(); it != best_sorted_groups.end(); it++) {
	  
	  int dist_left =  ((*it) == m)? 0 : offset(mentions[id], 1) - offset(mentions[sorted_mentions[(*it)]->get_id()], 0);
	  int dist_right = ((*it) == m)? 0 : offset(mentions[sorted_mentions[(*it)]->get_id()], 1) - offset(mentions[id], 0);

	  if (dist_left >= 0 and dist_left <= dist_best_left) {
	    best_left = sorted_mentions[(*it)]->get_id();
	    dist_best_left = dist_left;
	  }
	  else
	    if (dist_right > 0 and dist_right < dist_best_right) {
	      best_right = sorted_mentions[(*it)]->get_id();
	      dist_best_right = dist_right;
	    }
	}

	group = dist_best_left>dist_best_right ? best_right : best_left;
      }

      mentions[id].set_group(group);

      if (chains.find(group) == chains.end())
	chains[group] = vector<int>({id});
      else
	chains[group].push_back(id);
     }

    }

    t1 = clock();  // final time
    TRACE(3,L"adding chains to doc time: "+util::double2wstring(double(t1-t0)/double(CLOCKS_PER_SEC)));

    for (unsigned int m=0; m<mentions.size(); m++) 
      // ignore the mention when it is a singleton and provide_singletons=false 
      if (provide_singletons or chains[mentions[m].get_group()].size()>1) 
	doc.add_mention(mentions[m]);
  }

} // namespace


