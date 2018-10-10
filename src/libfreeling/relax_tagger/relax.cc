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

#include <cmath>

#include "freeling/morfo/relax.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"RELAX"
#define MOD_TRACECODE RELAX_TRACE


  //---------- Class problem ----------------------------------

  ////////////////////////////////////////////////////////////////
  ///
  /// Constructor: allocate space for nv variables, 
  /// and a label weigth list for each
  ///
  ///////////////////////////////////////////////////////////////

  problem::problem(int nv) {
    // allocate tables for the new sentence. One variable for each word in the sentence
    vars = vector<vector<label> >(nv,vector<label>());
    varnames = vector<wstring>(nv);
    CURRENT=0; NEXT=1;
  }

  ///////////////////////////////////////////////////////////////
  ///
  ///  set a name identifier for a variable (just for user convenience)
  ///
  ///////////////////////////////////////////////////////////////

  void problem::set_var_name(int i, const wstring &vname) {
    varnames[i] = vname;
  }
  
  ///////////////////////////////////////////////////////////////
  ///
  ///  get name identifier for a variable (just for user convenience)
  ///
  ///////////////////////////////////////////////////////////////

  wstring problem::get_var_name(int i) const {
    return varnames[i];
  }

  ///////////////////////////////////////////////////////////////
  ///
  /// get number of labels for a variable (just for user convenience)
  ///
  ///////////////////////////////////////////////////////////////

  int problem::get_num_labels(int i) const {
    return vars[i].size();
  }

  
  ///////////////////////////////////////////////////////////////
  ///
  ///  get name identifier for a variable label (just for user convenience)
  ///
  ///////////////////////////////////////////////////////////////

  wstring problem::get_label_name(int i, int j) const {
    return vars[i][j].get_name();
  }

  
  ///////////////////////////////////////////////////////////////
  ///
  ///  Add a label (and its weight) to the i-th variable
  ///
  ///////////////////////////////////////////////////////////////

  void problem::add_label(int i, double w, const wstring &lbname) {
    label lb;
    lb.weight[CURRENT] = lb.weight[NEXT] = w;
    lb.name = lbname;
    vars[i].push_back(lb);
  }


  ////////////////////////////////////////////////
  ///  Add a new constraint to the problem, afecting 
  /// the (v,l) pair
  ////////////////////////////////////////////////

  void problem::add_constraint(int v, int l, const list<list<pair<int,int> > > &lp, double comp) {

    constraint ct(lp.size());

    // translate the given list of coordinates (v,l) to pointers
    // to the actual label weights, to speed later access.
    list<list<pair<int,int> > >::const_iterator x;
    int i;
    for (i=0,x=lp.begin();  x!=lp.end();  i++,x++) {

      ct[i] = vector<constraint_element>(x->size());
                            
      list<pair<int,int> >::const_iterator y;
      int j;
      for (j=0,y=x->begin();  y!=x->end();  j++,y++) {
        TRACE(4, L"added constraint with comp=" << comp << L" for ("<<v<<L","<<l<<L")["<<varnames[v]<<"="<<vars[v][l].get_name()
              << "] ==> (" << y->first << L"," << y->second << L")["<<varnames[y->first]<<"="<<vars[y->first][y->second].get_name()<<"]");
        
        ct[i][j] = constraint_element(y->first, y->second, vars[y->first][y->second].weight);
      }
    }

    ct.set_compatibility(comp);
    vars[v][l].constraints.push_back(ct);
  }

  ////////////////////////////////////////////////
  /// Once the problem is solved, obtain the solution.
  /// Get the best label(s) for variable v, and return 
  /// them via list<int>&
  ////////////////////////////////////////////////

  list<int> problem::best_label(int v) const {
    vector<label>::const_iterator lab;
    int j;
    double max;
    list<int> best;

    // build list of labels with highest weight
    max=0.0; 
    for (j=0,lab=vars[v].begin(); lab!=vars[v].end(); j++,lab++) {

      if (lab->weight[CURRENT] > max) {
        max=lab->weight[CURRENT];
        // if new maximum, restart list from scratch
        best.clear();
        best.push_back(j);
      }
      else if (lab->weight[CURRENT] == max) {
        // if equals current maximum, add to the list.
        best.push_back(j);
      }
    }

    return(best);
  }


  ////////////////////////////////////////////////
  /// Check whether any weight changed more than Epsilon
  ////////////////////////////////////////////////

  bool problem::there_are_changes(double epsil) const {
    vector<vector<label> >::const_iterator i;
    vector<label>::const_iterator j;

    for (i=vars.begin(); i!=vars.end(); i++) 
      if (i->size() > 1) 
        for (j=i->begin(); j!=i->end(); j++) {
          double ch = fabs(j->weight[NEXT] - j->weight[CURRENT]);
          if (ch >= epsil) {
            TRACE(4,L" Found weight change of "+util::double2wstring(ch)+L", not converging yet.");
            return true;
          }
        }

    return(false);
  }

  ////////////////////////////////////////////////
  /// Exchange tables, get ready for next iteration
  ////////////////////////////////////////////////

  void problem::next_iteration() {
    CURRENT = NEXT;  
    NEXT = 1 - NEXT;    // 'NEXT' becomes 'CURRENT'. A new 'NEXT' will be computed
  }

  //---------- Class label ----------------------------------

  ////////////////////////////////////////////////////////////////
  ///  The class label stores all information related to a 
  /// variable label in the relaxation labelling algorithm.
  ////////////////////////////////////////////////////////////////

  label::label() {}
  double label::get_weight(int which) const { return weight[which]; }
  void label::set_weight(int which, double w) { weight[which]=w; }
  wstring label::get_name() const { return name; }

  //---------- Class constraint ----------------------------------

  constraint_element::constraint_element() {var=-1; lab=-1; w=NULL;}
  constraint_element::constraint_element(int v, int l, double* wgh) {var=v; lab=l; w=wgh;}

  ////////////////////////////////////////////////////////////////
  ///  The class constraint stores all information related to a 
  /// constraint on a label in the relaxation labelling algorithm.
  ////////////////////////////////////////////////////////////////

  constraint::constraint(int sz) :  vector<vector<constraint_element> >(sz, vector<constraint_element>()) {}

  ////////////////////////////////////////////////
  /// set compatibility value
  ////////////////////////////////////////////////

  void constraint::set_compatibility(double c) {
    compatibility=c;
  }

  ////////////////////////////////////////////////
  /// get compatibility value
  ////////////////////////////////////////////////

  double constraint::get_compatibility() const {
    return(compatibility);
  }


  //---------- Class relax ----------------------------------

  ///////////////////////////////////////////////////////////////
  ///  Constructor: Build a relax solver
  ///////////////////////////////////////////////////////////////

  relax::relax(int m, double f, double r) : MaxIter(m), ScaleFactor(f), Epsilon(r) {}


  ////////////////////////////////////////////////
  /// change scale factor 
  ////////////////////////////////////////////////

  void relax::set_scale_factor(double sf) { ScaleFactor = sf; }


  ////////////////////////////////////////////////
  /// Solve the consistent labelling problem
  ////////////////////////////////////////////////

  void relax::solve(problem &prb) const {
  
    // iterate until convercence (no changes)
    int n=0; 
    double change;
    int vch=0;
    int jch=0;
    while ((n==0 or change>=Epsilon) and n<MaxIter) {
      TRACE(1,L"Relaxation iteration number "<<n);
      TRACE(2,L" Max abs change is "<<change
            <<L" (v,l)=("<<vch<<L","<<jch<<L")["<<prb.get_var_name(vch)<<":"<<prb.get_label_name(vch,jch)<<"]"
            <<L" from "<<prb.vars[vch][jch].get_weight(prb.NEXT)
            <<L" to "<<prb.vars[vch][jch].get_weight(prb.CURRENT));

      change=0;

      // for each label of each variable
      vector<vector<label> >::iterator var;
      int v;
      for (v=0,var=prb.vars.begin(); var!=prb.vars.end(); var++,v++) {

        TRACE(3,L"   Variable " << v << L" (" << prb.get_var_name(v) << L")");
        double fnorm=0;

        // variable has only one or no labels. No need to change anything
        if (var->size() == 0) {
          TRACE(4,L"     No labels");
        }
        else if (var->size() == 1) {
          TRACE(4,L"     Label 0 (" << prb.get_label_name(v,0) << L")" << L" weight=" << (*var)[0].get_weight(prb.CURRENT));          
        }
        
        else { //  Variable has more than one option, apply constraints to update weights
        
          double *support = new double[var->size()];
          vector<label>::iterator lab;
          int j;
          for (j=0,lab=var->begin(); lab!=var->end();  j++,lab++) {

            double CurrW = lab->get_weight(prb.CURRENT);
            TRACE(4,L"     Label " << j << L" (" << prb.get_label_name(v,j) << L")" << L" weight=" << CurrW);
            if (CurrW>0) { // if weight==0 don't bother to compute supports, since the weight won't change
            
              support[j]=0.0;
              // apply each constraint affecting the label
              list<constraint>::const_iterator r;
              for (r=lab->constraints.begin(); r!=lab->constraints.end(); r++) {
		TRACE(6,L"      -Checking constraint (comp:" << r->get_compatibility() << L")");

                // each constraint is a list of terms to be multiplied
                constraint::const_iterator p;
                double inf = 1.0;
                for (p=r->begin(); p!=r->end(); p++) {
		  // each term is a list (of lenght one except on negative or wildcarded conditions) 
                  // of label weights to be added.
                  double tw;
                  vector<constraint_element>::const_iterator wg;
                  for (tw=0, wg=p->begin(); wg!=p->end(); wg++) {
                    tw += (wg->w)[prb.CURRENT];
		    TRACE(6,L"         adding constraint element (" << wg->var << L"," << wg->lab << L"," << (wg->w)[prb.CURRENT] << L")");
		  }
                  inf *= tw;
                }
              
                // add constraint influence*compatibility to label support
                support[j] += r->get_compatibility() * inf;
                TRACE(6,L"       constraint done (comp:" << r->get_compatibility() << L"), inf=" << inf << L",  accum.support=" << support[j]);
              }
            
              // normalize supports to a unified range
              TRACE(4,L"        total support=" << support[j]);
              support[j] = NormalizeSupport(support[j]);      
              TRACE(4,L"        normalized support=" << support[j]);
              // compute normalization factor for updating function below
              fnorm += CurrW * (1+support[j]);
            }
          }
        
          // update label weigths, update maximum seen change
          for (j=0,lab=var->begin(); lab!=var->end();  j++,lab++) {
            double CurrW = lab->get_weight(prb.CURRENT);
            double NewW = (CurrW>0 ? CurrW*(1+support[j])/fnorm : 0);
            lab->set_weight(prb.NEXT, NewW);
            if (fabs(NewW-CurrW) > change) {
              change = fabs(NewW-CurrW);
              vch=v; jch=j;
            }
          }
        
          delete[] support;
        }
      }
    
      n++; 

      prb.next_iteration();  // exchange tables to prepare for next iteration
    }
  }





  //--------------- private methods -------------

  ////////////////////////////////////////////////
  /// Normalize support for a label in a fixed range
  ////////////////////////////////////////////////

  double relax::NormalizeSupport(double x) const {

    // normalization disabled, do nothing
    if (ScaleFactor==0) return x;  
    // out of range, return +1 or -1
    else if (fabs(x)>=ScaleFactor) return copysign(1,x);
    // in range, return normalized value
    else return (x/ScaleFactor);
  }



} // namespace
