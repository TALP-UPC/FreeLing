 /*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2014   TALP Research Center
 *                       Universitat Politecnica de Catalunya
 *
 *  This file is part of Treeler.
 *
 *  Treeler is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Treeler is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with Treeler.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   parser-projdep2.tcc
 * \brief  Implementation of ProjDep2 template methods
 * \author Xavier Carreras
 */

#include <cmath>
#include <iostream>

using namespace std;

namespace treeler {

  template<typename X, typename S>
  double
  ProjDep2::argmax(const Configuration& config, 
		   const X& x,
		   S& scores,
		   Label<PartDep2>& y) {

    // number of tokens in the sentence
    const int N = x.size();
    
    // dynprog matrices that store optimal scores for every signature
    chart_scores CS(N, config.L);

    // a structure of back-pointers to recover the viterbi tree
    chart_values CV;

    int s,e,l,w;

    /*
     * W is the max width of the spans it controls the outer loop of
     * the viterbi search,
     *
     * when multiple roots are allowed, its value is N+1, so that
     * dependencies from the root are visited from token 0 to token
     * N-1
     *
     */
    int W = N;
    if (config.multiroot) W++;

    /* used for the score of the best structures found in different local optimizations */
    double best_score = 0; 

    /* used to calculate the index of parts */
    PartDep2 part;

    for (w=1; w<W; ++w) {

      int initial_s = config.multiroot ? -1 : 0;

      for (s=initial_s; s<N-w; ++s) {
        e = s+w;

	//cerr << "IT w=" << w << " s=" << s << " e=" << e << endl;

        for (l=0; l<config.L; ++l) {
          int r;                    /* iterator for splitting points between s and e */
          int best_r = -2;          /* best splitting point found, -2 indicates not initialized */
          int best_cs=0, best_ce=0; /* the best of childs, which form the best structure */


          // UNCOMPLETE STRUCTURES HEADED AT S	  

	  part.h = s;
	  part.m = e;
	  part.l = l;

          // consider each possible splitting point
          for (r=s; r<e; ++r) {

            // here we need to find the best cs and ce for the current r

            // CHILDS OF S
	    part.type = PartDep2::SIB;
            int auxb_cs = -2;
            double auxb_ssco=0;
            if (r==s) { // in this case, e is the first child of s; it is indicated by cs=s;

	      //part_id = dpo2_index::encode(N, dpo2_index::CH, s,e,l, -1);
	      part.c = -1;
              auxb_cs = s;	      
              auxb_ssco = scores(part);
            }
            else {
              // this loop considers every possible child of s between s and r
              for (int cs=s+1; cs<=r; ++cs) {
		// part_id = dpo2_index::encode(N, dpo2_index::CH, s,e,l, cs);
		part.c = cs;
                double sco = CS.cscore(s,r,cs) + scores(part);
		//		cerr << "PD2 treeler [U," << s << "," << e << "," << r << "," << cs << "] = " << CS.cscore(s,r,cs) << " + " <<  scores(part) << " = " << sco << endl; 
                if (auxb_cs==-2 or sco>auxb_ssco) {
                  auxb_ssco = sco;
                  auxb_cs = cs;
                }
              }
            }

            // CHILDS OF E
	    part.type = PartDep2::CMI;
            int auxb_ce = -2;
            double auxb_esco=0;
            if (r+1==e) { // in this case, e has no childs to its left; it is indicated by ce=e; le is irrelevant
	      //	      part_id = dpo2_index::encode(N, dpo2_index::CMI, s,e,l, -1);
	      part.c = -1;
              auxb_ce = e;
	      auxb_esco = scores(part);
            }
            else {
              // this loop considers every possible child of e between r+1 and e
              for (int ce=r+1; ce<e; ++ce) {
		// part_id = dpo2_index::encode(N, dpo2_index::CMI, s,e,l, ce);
		part.c = ce;
                double sco = CS.cscore(e,r+1,ce) + scores(part);
                if (auxb_ce==-2 or sco>auxb_esco) {
                  auxb_esco = sco;
                  auxb_ce = ce;
                }
              }
            }

            // DECIDE IF CURRENT R IS BETTER THAN BEST_R
            if (best_r==-2 or (auxb_ssco+auxb_esco > best_score)) {
              best_score = auxb_ssco + auxb_esco;
              best_r = r;
              best_cs = auxb_cs;
              best_ce = auxb_ce;
            }
          }

	  // score for main dependency 
	  part.type = PartDep2::FO;
	  part.c = -1;
	  //part_id = dpo2_index::encode(N, dpo2_index::FO, s,e,l);
          best_score += scores(part);

          CS.uscore_set(s,e,l, best_score);
          CV.set_ubp(s,e,l, best_r, best_cs, best_ce);


          // UNCOMPLETE STRUCTURES HEADED AT E
	  part.h = e;
	  part.m = s;
          best_r = -2;

          if (s!=-1) {
            // consider each possible splitting point
            for (r=s; r<e; ++r) {

              // here we need to find the best cs,ls and ce,le for the current r

              // CHILDS OF S
	      part.type = PartDep2::CMI;
              int auxb_cs = -2;
              double auxb_ssco=0;
              if (r==s) { // in this case, s has no childs to its right; it is indicated by cs=s; ls is irrelevant
		// part_id = dpo2_index::encode(N, dpo2_index::CMI, e,s,l, -1);
		part.c = -1;
                auxb_cs = s;
                auxb_ssco = scores(part);
              }
              else {
                // this loop considers every possible child of s between s and r
                for (int cs=s+1; cs<=r; ++cs) {
		  //part_id = dpo2_index::encode(N, dpo2_index::CMI, e,s,l, cs);
		  part.c = cs;
		  double sco = CS.cscore(s,r,cs) + scores(part);
		  if (auxb_cs==-2 or sco>auxb_ssco) {
		    auxb_ssco = sco;
		    auxb_cs = cs;
		  }
		}
              }

              // CHILDS OF E
	      part.type = PartDep2::SIB;
              int auxb_ce = -2;
              // int auxb_le = 0;
              double auxb_esco=0;
              if (r+1==e) { // e has no childs, i.e. s will be the first child to its left
		// part_id = dpo2_index::encode(N, dpo2_index::CH, e,s,l, -1);
		part.c = -1;
                auxb_ce = e;
                auxb_esco = scores(part);
              }
              else {
                // this loop considers every possible child of e between r+1 and e
                for (int ce=r+1; ce<e; ++ce) {
		  // part_id = dpo2_index::encode(N, dpo2_index::CH, e,s,l, ce);
		  part.c = ce;
                  double sco = CS.cscore(e,r+1,ce) + scores(part);
                  if (auxb_ce==-2 or sco>auxb_esco) {
                    auxb_esco = sco;
                    auxb_ce = ce;
                  }
                }
              }

              // DECIDE IF CURRENT R IS BETTER THAN BEST_R
              if (best_r==-2 or (auxb_ssco+auxb_esco > best_score)) {
                best_score = auxb_ssco + auxb_esco;
                best_r = r;
                best_cs = auxb_cs;
                best_ce = auxb_ce;
              }
            }

	    // part score for score(e,s,l)
	    //part_id = dpo2_index::encode(N, dpo2_index::FO, e,s,l);
	    part.type = PartDep2::FO;
	    part.c = -1;
	    best_score += scores(part);
	    
            CS.uscore_set(e,s,l, best_score);
            CV.set_ubp(e,s,l, best_r, best_ce, best_cs);
          }


        } // for each l

        // COMPLETE STRUCTURES

        // search for best complete dtree with head at s
	part.h = s;
	part.type = PartDep2::CMO;
        for (int m=s+1; m<=e; ++m) {
          int best_l =-1;
          int best_cm=0;
          for (int l=0; l<config.L; ++l) {
            // find the best cm for this label
	    part.m = m;
	    part.l = l;
            int auxb_cm = -2;
            double auxb_sco = 0;
            if (m==e) {
              auxb_cm = m;
	      // null mod child
	      //part_id = dpo2_index::encode(N, dpo2_index::CMO, s,m,l, -1);
	      part.c = -1;
	      auxb_sco = scores(part);
            }
            else {
              for (int cm=m+1; cm<=e; ++cm) {
		//part_id = dpo2_index::encode(N, dpo2_index::CMO, s,m,l, cm);
		part.c = cm;
                double sco = scores(part) + CS.cscore(m,e,cm);
                if ((auxb_cm==-2) or (sco>auxb_sco)) {
                  auxb_cm = cm;
                  auxb_sco = sco;
                }
              }
            }
            auxb_sco += CS.uscore(s,m,l);
            if (best_l==-1 or auxb_sco>best_score) {
              best_l = l;
              best_cm = auxb_cm;
              best_score = auxb_sco;
            }
          }
          CS.cscore_set(s,e,m, best_score);
          CV.set_cbp(s,e,m, best_l,best_cm);
        }
        if (s!=-1) {
          // search for best complete dtree with head at e
	  part.h = e;
          for (int m=s; m<e; ++m) {
            int best_l =-1;
            int best_cm=0;
            for (int l=0; l<config.L; ++l) {
              // find the best cm for this label
	      part.m = m;
	      part.l = l;
              int auxb_cm = -2;
              double auxb_sco = 0;
              if (m==s) {
                auxb_cm = m;
		// null mod child 
		//part_id = dpo2_index::encode(N, dpo2_index::CMO, e,m,l, -1);
		part.c = -1;
		auxb_sco = scores(part);
              }
              else {
                for (int cm=s; cm<m; ++cm) {
		  //part_id = dpo2_index::encode(N, dpo2_index::CMO, e,m,l, cm);
		  part.c = cm;
                  double sco = scores(part) + CS.cscore(m,s,cm);
                  if ((auxb_cm==-2) || (sco>auxb_sco)) {
                    auxb_cm = cm;
                    auxb_sco = sco;
                  }
                }
              }
              auxb_sco += CS.uscore(e,m,l);
              if (best_l==-1 or auxb_sco>best_score) {
                best_l = l;
                best_cm = auxb_cm;
                best_score = auxb_sco;
              }
            }
            CS.cscore_set(e,s,m, best_score);
            CV.set_cbp(e,s,m, best_l,best_cm);
          } // for each m (modifier)
        }

      } // for each s (start point)
    } // for each w (i.e. span width)


   // SEARCH FOR THE BEST STRUCTURE SPANNING THE COMPLETE SENTENCE

    double global_score = 0;

    int best_rootc=-2;
    int best_rootl=-2;
    // Root
    if (config.multiroot) {
      // search for the best root-child and dep label
      for (int c=0; c<N; ++c) {
        if (best_rootc==-2 or CS.cscore(-1,N-1,c)>global_score) {
          global_score = CS.cscore(-1,N-1,c);
          best_rootc = c;
        }
      }
      unravel_tree(N,y, CV, -1, N-1, best_rootc);
    }
    else {
      int best_cl=0;
      int best_cr=0;

      /* consider each possible child of the root with every possible label */

      part.h = -1;

      for (int c=0; c<N; ++c) {
	part.m = c;
        for (int l=0; l<config.L; ++l) {
	  part.l = l;
          // consider each child of c to the left
          // the best values are initialized for cl=c, which means that c has no child (and thus ll is irrelevant)
          int auxb_cl = -2;
          // int auxb_ll;
          double auxb_lsco=0;
          if (c==0) {    // c is the first token and thus has no child to its left
            auxb_cl = 0;
	    //part_id = dpo2_index::encode(N, dpo2_index::CMI, -1,c,l, -1);
	    part.type = PartDep2::CMI;
	    part.c = -1;
            auxb_lsco = scores(part);
          }
          else {
            // this loop considers every possible child of c between 0 and c-1
            for (int cl=0; cl<c; ++cl) {
	      // part_id = dpo2_index::encode(N, dpo2_index::CMI, -1,c,l, cl);
	      part.type = PartDep2::CMI;
	      part.c = cl;
              double sco = CS.cscore(c,0,cl) + scores(part);
              if (auxb_cl==-2 or sco>auxb_lsco) {
                auxb_lsco = sco;
                auxb_cl = cl;
              }
            }
          }

          // consider each child of c to the right
          // the best values are initialized for cl=c, which means that c has no child (and thus lr is irrelevant)
          int auxb_cr = -2;
          double auxb_rsco=0;
          if (c==N-1) {  // the is the last token, and thus has no child to its right
            auxb_cr = c;
	    // part_id = dpo2_index::encode(N, dpo2_index::CMO, -1,c,l, -1);
	    part.type = PartDep2::CMO;
	    part.c = -1;
            auxb_rsco = scores(part);
          }
          else {
            // this loop considers every possible child of c between c+1 and N-1
            for (int cr=c+1; cr<N; ++cr) {
	      // part_id = dpo2_index::encode(N, dpo2_index::CMO, -1,c,l, cr);
	      part.type = PartDep2::CMO;
	      part.c = cr;
              double sco = CS.cscore(c,N-1,cr) + scores(part);
              if (auxb_cr==-2 or sco>auxb_rsco) {
                auxb_rsco = sco;
                auxb_cr = cr;
              }
            }
          }
	  // main dependency
	  part.type = PartDep2::SIB; 
	  part.c = -1; 
	  double sibscore = scores(part);


	  part.type = PartDep2::FO;
	  part.c = -1;
	  //part_id = dpo2_index::encode(N, dpo2_index::FO, -1,c,l);
          double cscore = auxb_lsco + auxb_rsco + scores(part) + sibscore;

	  //	  cerr << "root to " << c << " : " << auxb_lsco << " + " << auxb_rsco << " + " << scores(part) << " = " << cscore  << endl;
	  //	  cerr << "root to " << c << " : tra " << auxb_lsco+scores(part) << " + tri " << auxb_rsco << " = " << cscore  << endl;
	  
          // DECIDE IF CURRENT E IS BETTER THAN BEST_ROOTC
          if (best_rootc==-2 or (cscore > global_score)) {
            global_score = cscore;
            best_rootc = c;
            best_rootl = l;
            best_cl = auxb_cl;
            best_cr = auxb_cr;
          }
        }
      }


      // factors.push_back(o2_model::factor(-1,best_rootc,best_rootl, -1,0, best_cl,-2, best_cr,-2));
      // set part id to factor(-1,best_rootc,best_rootl, -1,0, best_cl,-2, best_cr,-2)

      // main dep
      //part_id = dpo2_index::encode(N, dpo2_index::FO, -1,best_rootc,best_rootl);
      part.type = PartDep2::FO;
      part.h = -1;
      part.m = best_rootc;
      part.c = -1;
      part.l = best_rootl;
      y.push_back(part);
      // head child
      /* TERRY: removed the addition of a null-child CH part, because
	 the null-child CH score is not being included.  Also, since
	 it's single-root parsing, the null-child CH part will always
	 be included, and thus it's not informative to the parser. */
      //      part_id = dpo2_index::encode(N, dpo2_index::CH, -1,best_rootc,best_rootl, -1);
      //      y.push_back(part_id);
      part.type = PartDep2::SIB; 
      part.c = -1; 
      y.push_back(part); 
      global_score += scores(part); 

      // mod child inside
      // part_id = dpo2_index::encode(N, dpo2_index::CMI, -1,best_rootc,best_rootl, best_cl);
      part.type = PartDep2::CMI; 
      part.c = best_cl;
      y.push_back(part);
      // mod child outside
      // part_id = dpo2_index::encode(N, dpo2_index::CMO, -1,best_rootc,best_rootl, best_cr);
      part.type = PartDep2::CMO; 
      part.c = best_cr;
      y.push_back(part);

      //      cerr << "unraveling tree  ..." << flush; 
      unravel_tree(N, y, CV, best_rootc, 0, best_cl);
      unravel_tree(N, y, CV, best_rootc, N-1, best_cr);
      //      cerr << " done" << endl; 
    }

    //    cerr << "P sentence parsed : score is " << global_score << endl;


    // cerr << ">> CHECK" << endl; 
    // cerr << ">>> argmax score " << global_score << endl; 
    // cerr << ">>> argmax parts " << y << endl; 
    // cerr << ">>> argmax parts rescored " << scores.score(y) << endl;
    // {
    //   DepVector<int> t; 
    //   R::compose(x,y, t);
    //   Label<R> yy; 
    //   DepSymbols sym;
    //   R::decompose(sym,x,t,yy);
    //   cerr << ">>> new parts    " << yy << endl;
    //   cerr << ">>> new parts rescored " << scores.score(yy) << endl;
    // }
    // cerr << ">> END OF CHECK" << endl; 

    return global_score;
  }



}
