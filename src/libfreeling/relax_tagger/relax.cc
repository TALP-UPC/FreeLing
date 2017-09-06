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
    CURRENT=0; NEXT=1;
  }


  ///////////////////////////////////////////////////////////////
  ///
  ///  Add a label (and its weight) to the i-th variable
  ///
  ///////////////////////////////////////////////////////////////

  void problem::add_label(int i, double w) {
    label lb;
    lb.weight[CURRENT] = lb.weight[NEXT] = w;
    vars[i].push_back(lb);
  }


  ////////////////////////////////////////////////
  ///  Add a new constraint to the problem, afecting 
  /// the (v,l) pair
  ////////////////////////////////////////////////

  void problem::add_constraint(int v, int l, const list<list<pair<int,int> > > &lp, double comp) {
    int i,j;
    list<list<pair<int,int> > >::const_iterator x;
    list<pair<int,int> >::const_iterator y;

    constraint ct(lp.size());

    // translate the given list of coordinates (v,l) to pointers
    // to the actual label weights, to speed later access.
    for (i=0,x=lp.begin();  x!=lp.end();  i++,x++) {

      ct[i] = vector<double*>(x->size(),NULL);

      for (j=0,y=x->begin();  y!=x->end();  j++,y++) {
        TRACE(4,L"added constraint with comp="+util::double2wstring(comp)+L" for ("+util::int2wstring(v)+L","+util::int2wstring(l)+L") pointing to ("+util::int2wstring(y->first)+L","+util::int2wstring(y->second)+L")");

        ct[i][j] = vars[y->first][y->second].weight;
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


  //---------- Class constraint ----------------------------------


  ////////////////////////////////////////////////////////////////
  ///  The class constraint stores all information related to a 
  /// constraint on a label in the relaxation labelling algorithm.
  ////////////////////////////////////////////////////////////////

  constraint::constraint(int sz) :  vector<vector<double*> >(sz, vector<double *>()) {}

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
      TRACE(1,L"Relaxation iteration number "+util::int2wstring(n));
      TRACE(2,L" Max abs change is "+util::double2wstring(change)+
              L" (v,l)=("+util::int2wstring(vch)+L","+util::int2wstring(jch)+L")"+
              L" from "+util::double2wstring(prb.vars[vch][jch].get_weight(prb.NEXT))+
              L" to "+util::double2wstring(prb.vars[vch][jch].get_weight(prb.CURRENT)));

      change=0;

      // for each label of each variable
      vector<vector<label> >::iterator var;
      int v;
      for (v=0,var=prb.vars.begin(); var!=prb.vars.end(); var++,v++) {

        TRACE(3,L"   Variable "+util::int2wstring(v));
        double fnorm=0;
        if (var->size() > 1) { //  only proceed if the word is ambiguous.
        
          double *support = new double[var->size()];
          vector<label>::iterator lab;
          int j;
          for (j=0,lab=var->begin(); lab!=var->end();  j++,lab++) {

            double CurrW = lab->get_weight(prb.CURRENT);
            TRACE(4,L"     Label "+util::int2wstring(j)+L" weight="+util::double2wstring(CurrW));
            if (CurrW>0) { // if weight==0 don't bother to compute supports, since the weight won't change
            
              support[j]=0.0;
              // apply each constraint affecting the label
              list<constraint>::const_iterator r;
              for (r=lab->constraints.begin(); r!=lab->constraints.end(); r++) {
              
                // each constraint is a list of terms to be multiplied
                constraint::const_iterator p;
                double inf;
                for (inf=1, p=r->begin(); p!=r->end(); p++) {
                
                  // each term is a list (of lenght one except on negative or wildcarded conditions) 
                  // of label weights to be added.
                  double tw;
                  vector<double*>::const_iterator wg;
                  for (tw=0, wg=p->begin(); wg!=p->end(); wg++)
                    tw += (*wg)[prb.CURRENT];
                
                  inf *= tw;
                }
              
                // add constraint influence*compatibility to label support
                support[j] += r->get_compatibility() * inf;
                TRACE(6,L"       constraint done (comp:"+util::double2wstring(r->get_compatibility())+L"), inf="+util::double2wstring(inf)+L",  accum.support="+util::double2wstring(support[j]));
              }
            
              // normalize supports to a unified range
              TRACE(4,L"        total support="+util::double2wstring(support[j]));
              support[j] = NormalizeSupport(support[j]);      
              TRACE(4,L"        normalized support="+util::double2wstring(support[j]));
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
