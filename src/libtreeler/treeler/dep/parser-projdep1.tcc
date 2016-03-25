//////////////////////////////////////////////////////////////////
//
//    Treeler - Open-source Structured Prediction for NLP
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//                          02111-1307 USA
//
//    contact: Xavier Carreras (carreras@lsi.upc.es)
//             TALP Research Center
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////
#include <cmath>
#include <iostream>

using namespace std;

namespace treeler {

  /* Back pointers to recover the best deptree from DynProg
   * optimization matrix
   *
   * The pointers are indexed by head and modifier
   * Internally, pointers for head=-1 (root) are stored in the [m,m] position
   *
   */
  struct ProjDep1::backp {
    int _N;   /* number of words */
    int *_RC;  /* optimal splitting points for complete signatures */
    int *_RI;  /* optimal splitting points for incomplete signatures */
    int *_K;   /* optimal labels for incomplete signatures */
    backp(int n0) {
      _N = n0;
      _RC = new int[_N*_N];
      _RI = new int[_N*_N];
      _K = new int[_N*_N];
    }

    ~backp() {
      delete [] _RC;
      delete [] _RI;
      delete [] _K;
    }

    void set_r_com(int h, int m, int r) {
      if (h==-1) h=m;
      _RC[m*_N + h] = r;
    }
    int get_r_com(int h, int m) const {
      if (h==-1) h=m;
      return _RC[m*_N + h];
    }
    void set_r_inc(int h, int m, int r) {
      if (h==-1) h=m;
      _RI[m*_N + h] = r;
    }
    int get_r_inc(int h, int m) const {
      if (h==-1) h=m;
      return _RI[m*_N + h];
    }
    void set_k(int h, int m, int k) {
      if (h==-1) h=m;
      _K[m*_N + h] = k;
    }
    int get_k(int h, int m) const {
      if (h==-1) h=m;
      return _K[m*_N + h];
    }
  };



  template <typename X, typename S>
  double ProjDep1::argmax(const Configuration& c,
			  const X& x,
			  S& scores,
			  Label<PartDep1>& y) {
    MyScores<X,S> myscores(x,scores);
    return argmax_impl(c, x, myscores, y); 
  }


  template <typename X, typename S>
  double ProjDep1::argmax_impl(const Configuration& c, const X& x, S& score,
			       Label<PartDep1>& y) {

    const int N = x.size();
    // const int NN = N*N;
    // const int NNK = NN*_K;
    // assert(NNK == x->nparts());

    // Chart structure
    // matrices of scores
    // SCom : for complete structures
    // SInc : for incomplete structures
    // 1st/2nd dimension : head/modifier positions
    double** SCom = new double*[N];
    double** SInc = new double*[N];
    for(int i = 0; i < N; ++i) {
      SCom[i] = new double[N];
      SInc[i] = new double[N];
    }

    double* SRCom = new double[N];
    double* SRInc = new double[N];

    // a structure of back-pointers to recover the viterbi tree
    backp BP(N);

    int s,e,w;

    // initialization
    for (s=0;s<N;++s) {
      SCom[s][s] = 0.0;
      SInc[s][s] = 0.0;
    }

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
    if (c.multiroot) W++;

    for (w=1; w<W; ++w) {

      /*
       * The code in this conditional treats the structures headed at the root.
       * The operations are the same than with regular tokens, except for two
       * issues: the root has its own dyn_prog matrix, and the root only can be
       * head, not modifier
      */
      if (c.multiroot) {
	e = -1+w;

	int r;                         /* iterator for splitting points between -1 and e */
	int opt_r = -1;                /* optimal splitting point, initialized to -1 */
	double opt_sco = SCom[e][0];   /* score the optimal splitting point */

	// search for best incomplete dtree
	for (r=0; r<e; ++r) {
	  if (SRCom[r] + SCom[e][r+1] > opt_sco) {
	    opt_r = r;
	    opt_sco = SRCom[r] + SCom[e][r+1];
	  }
	}
	int opt_k = 0;
	double opt_ksco =  score(-1,e,0); // S[e + N*e];/*scores.score(-1,e,0);*/
	for (int k=1; k<c.L; ++k) {
	  const double sc = score(-1,e,k); // S[e + N*e + NN*k];/*scores.score(-1,e,k)*/
	  if (sc > opt_ksco) {
	    opt_k = k;
	    opt_ksco = sc;
	  }
	}
	SRInc[e] = opt_sco + opt_ksco;
	BP.set_r_inc(-1,e,opt_r);
	BP.set_k(-1,e,opt_k);

	// complete structure
	opt_r = -1;
	for (r=0; r<=e; ++r) {
	  if ((opt_r==-1) || (SRInc[r] + SCom[r][e] > opt_sco)) {
	    opt_r = r;
	    opt_sco = SRInc[r] + SCom[r][e];
	  }
	}
	SRCom[e] = opt_sco;
	BP.set_r_com(-1,e,opt_r);
      }

      for (s=0; s<N-w; ++s) {
	e = s+w;

	int r;                           /* iterator for splitting points between s and e */
	int opt_r = s;                   /* optimal splitting point, initialized to r=s */
	double opt_sco = SCom[e][s+1];   /* score the optimal splitting point */

	// search for best incomplete dtree
	for (r=s+1; r<e; ++r) {
	  if (SCom[s][r] + SCom[e][r+1] > opt_sco) {
	    opt_r = r;
	    opt_sco = SCom[s][r] + SCom[e][r+1];
	  }
	}


	// Dependency headed at s

	int k;                  /* iterator for classes */
	int opt_k;              /* optimal class */
	double opt_ksco = 0;    /* score of the optimal class */

	opt_k = 0;
	opt_ksco = score(s,e,0); // S[s + N*e];/*scores.score(s,e,0);*/
	for (k=1; k<c.L; ++k) {
	  const double sc = score(s,e,k); //S[s + N*e + NN*k];/*scores.score(s,e,k)*/
	  if (sc > opt_ksco) {
	    opt_k = k;
	    opt_ksco = sc;
	  }
	}
	SInc[s][e] = opt_sco + opt_ksco;
	BP.set_r_inc(s,e,opt_r);
	BP.set_k(s,e,opt_k);


	// Dependency headed at e

	opt_k = 0;
	opt_ksco = score(e,s,0); // S[e + N*s];/*scores.score(e,s,0);*/
	for (k=1; k<c.L; ++k) {
	  const double sc = score(e,s,k); // S[e + N*s + NN*k];/*scores.score(e,s,k)*/
	  if (sc>opt_ksco) {
	    opt_k = k;
	    opt_ksco = sc;
	  }
	}
	SInc[e][s] = opt_sco + opt_ksco;
	BP.set_r_inc(e,s,opt_r);
	BP.set_k(e,s,opt_k);


	// search for best complete dtree with head at s
	opt_r = -1;
	for (r=s+1; r<=e; ++r) {
	  if ((opt_r==-1) || (SInc[s][r] + SCom[r][e] > opt_sco)) {
	    opt_r = r;
	    opt_sco = SInc[s][r] + SCom[r][e];
	  }
	}
	SCom[s][e] = opt_sco;
	BP.set_r_com(s,e,opt_r);

	// search for best complete dtree with head at e
	opt_r = -1;
	for (r=s; r<e; ++r) {
	  if ((opt_r==-1) || (SCom[r][s] + SInc[e][r] > opt_sco)) {
	    opt_r = r;
	    opt_sco = SCom[r][s] + SInc[e][r];
	  }
	}
	SCom[e][s] = opt_sco;
	BP.set_r_com(e,s,opt_r);
      }
    }

    double global_score = 0;


    // Root
    if (c.multiroot) {
      /* here, the work has already been done before */
      global_score = SRCom[N-1];
      bool complete = true;
      unravel_tree(y, &BP, -1, N-1, complete, N);
    }
    else {
      /* consider the dependencies that go from the root to any of the
	 tokens, and choose the optimal */
      int opt_root = -1;
      int opt_rootK = -1;

      for (s=0; s<N; ++s) {
	int k;                /* iterator for classes */
	int opt_k = 0;        /* optimal class */
	double opt_sco = score(-1,s,0); // S[s + N*s];/*scores.score(-1,s,0);*/ /* score of the optimal class */
	for (k=1; k<c.L; ++k) {
	  const double sc = score(-1,s,k); //S[s + N*s + NN*k];/*scores.score(-1,s,k)*/
	  if (sc > opt_sco) {
	    opt_k = k;
	    opt_sco = sc;
	  }
	}

	opt_sco += SCom[s][0] + SCom[s][N-1];

	if (opt_root==-1 or global_score < opt_sco) {
	  opt_root = s;
	  opt_rootK = opt_k;
	  global_score = opt_sco;
	}
      }

      /*dtree.set_dependency(-1, opt_root, opt_rootK);*/
      y.push_back(PartDep1(-1, opt_root, opt_rootK)); //   opt_root + N*opt_root + NN*opt_rootK);
      unravel_tree(y, &BP, opt_root, 0, true, N);
      unravel_tree(y, &BP, opt_root, N-1, true, N);
    }

    //   write_chart(cerr, x, SCom, SInc, SRCom, SRInc, BP);

    for(int i = 0; i < N; ++i) {
      delete [] SCom[i];
      delete [] SInc[i];
    }
    delete [] SCom;
    delete [] SInc;
    delete [] SRCom;
    delete [] SRInc;
    return global_score;
  }

  template <typename X>
  void ProjDep1::write_chart(ostream& o, const X& x, 
			     double** SCom, double** SInc, 
			     double* SRCom, double* SRInc, 
			     const struct backp& bp){    
    int N = x.size();
    for (int i=0; i<N; ++i) {
      o << "CHART " << x.id() << " " << -1 << " " << i 
	<< " C " << SRCom[i] << " " << bp.get_r_com(-1, i)  
	<< " I " << SRInc[i] << " " << bp.get_r_inc(-1, i) << " " << bp.get_k(-1, i) << endl;
    }
    for (int i=0; i<N; ++i) {
      for (int j=0; j<N; ++j) {
        o << "CHART " << x.id() << " " << i << " " << j 
	<< " C " << SCom[i][j] << " " << bp.get_r_com(i, j)  
	<< " I " << SInc[i][j] << " " << bp.get_r_inc(i, j) << " " << bp.get_k(i, j) << endl;
      }
    }
  }


  /****************************************************/


  /* a set of part-wise marginal probabilities, represented in the log-domain */
  class ProjDep1::ChartIO {
  private:
    int _N, _K;
    double *_ICroot, *_OCroot, *_IIroot, *_OIroot;
    double *_IC, *_II, *_OC, *_OI;

  public:
    ChartIO(int k0, int n0)
      : _N(n0), _K(k0)
    {
      _IC = new double[_N*_N];
      _II = new double[_N*_N*_K];
      _OC = new double[_N*_N];
      _OI = new double[_N*_N];
      _IIroot = new double[_N*_K];
      _ICroot = new double[_N];
      _OIroot = new double[_N];
      _OCroot = new double[_N];

      int i;
      double l = log(1.0);
      for (i=0; i<_N; i++) {
	_ICroot[i] = _OCroot[i] = _OIroot[i] = l;
      }
      for (i=0; i<_N*_K; i++) {
	_IIroot[i] = l;
      }

      for (i=0; i<_N*_N; i++) {
	_IC[i] = _OC[i] = _OI[i] = l;
      }
      for (i=0; i<_N*_N*_K; i++) {
	_II[i] = l;
      }
    }

    ~ChartIO() {
      delete [] _IC;
      delete [] _OC;
      delete [] _II;
      delete [] _OI;
      delete [] _ICroot;
      delete [] _OCroot;
      delete [] _IIroot;
      delete [] _OIroot;
    }


    // i,j range from -1 to n
    static double log_add(double a, double b)
    {
      if (a>b)
	return a+log(1+exp(b-a));
      else
	return b+log(1+exp(a-b));
    }

    int N() const { return _N; }
    int K() const { return _K; }

    void inside_C_add(int h, int m, double w) { _IC[h*_N + m] = log_add(_IC[h*_N + m],w); }
    void outside_C_add(int h, int m, double w) { _OC[h*_N + m] = log_add(_OC[h*_N + m],w); }

    void inside_I_add(int h, int m, int k, double w) { _II[h*_N*_K + m*_K + k] = log_add(_II[h*_N*_K + m*_K + k], w); }
    void outside_I_add(int h, int m, double w) { _OI[h*_N + m] = log_add(_OI[h*_N + m], w); }

    void inside_C_set(int h, int m, double w) { _IC[h*_N + m] = w; }
    void outside_C_set(int h, int m, double w) { _OC[h*_N + m] = w; }

    void inside_I_set(int h, int m, int k, double w) { _II[h*_N*_K + m*_K + k] = w; }
    void outside_I_set(int h, int m, double w) { _OI[h*_N + m] = w; }

    double inside_C(int h, int m) const { return _IC[h*_N + m]; }
    double outside_C(int h, int m) const { return _OC[h*_N + m]; }
    double inside_I(int h, int m, int k) const { return _II[h*_N*_K + m*_K + k]; }
    double outside_I(int h, int m) const { return _OI[h*_N + m ]; }

    double inside_C_root(int m) const { return _ICroot[m]; }
    double inside_I_root(int i, int k) const { return _IIroot[i*_K + k]; }
    double outside_C_root(int m) const { return _OCroot[m]; }
    double outside_I_root(int i) const { return _OIroot[i]; }

    void inside_C_root_set(int m, double w) { _ICroot[m] = w; }
    void inside_C_root_add(int m, double w) { _ICroot[m] = log_add(_ICroot[m], w); }
    void inside_I_root_set(int m, int k, double w) { _IIroot[m*_K +k] = w; }
    void outside_C_root_set(int m, double w) { _OCroot[m] = w; }
    void outside_C_root_add(int m, double w) { _OCroot[m] = log_add(_OCroot[m], w); }
    void outside_I_root_set(int m, double w) { _OIroot[m] = w; }
    void outside_I_root_add(int m, double w) { _OIroot[m] = log_add(_OIroot[m], w); }

    double log_Z() const { return _ICroot[_N-1]; }

    double log_marginal(int h, int m, int l) const {
      if (h==-1)
	return inside_I_root(m,l) + outside_I_root(m) - _ICroot[_N-1];
      else
	return inside_I(h,m,l) + outside_I(h,m) - _ICroot[_N-1];
    }


    void write(ostream & out, bool E) const {
      int o,i,j,k;
      E = true;
      // INSIDES
      if (E)
	out << "I C [" << -1 << "," << _N-1 << "] :: " << exp(this->inside_C_root(_N-1)) << endl;
      else
	out << "I log C [" << -1 << "," << _N-1 << "] :: " << this->inside_C_root(_N-1) << endl;

      for (i=0; i<_N; ++i) {
	if (E) {
	  out << "I [" << -1 << "," << i << "] ::";
	  for (k=0; k<_K; ++k) {
	    out << "  (ul_" << k << ") " << exp(this->inside_I_root(i,k));
	  }
	  out << endl;
	}
	else {
	  out << "I log [" << -1 << "," << i << "] ::";
	  for (k=0; k<_K; ++k) {
	    out << "  (ul_" << k << ") " << this->inside_I_root(i,k);
	  }
	  out << endl;
	}
      }

      for (o=1; o<_N; ++o) {
	for (i=0; i<_N-o; ++i) {
	  j = i + o;
	  if (E) {
	    out << "I [" << i << "," << j << "] ::";
	    out << "  (cl) " << exp(this->inside_C(i,j)) << "  (cr) " << exp(this->inside_C(j,i));
	    for (k=0; k<_K; ++k) {
	      out << "  (ul_" << k << ") " << exp(this->inside_I(i,j,k)) << "  (ur_" << k << ") " << exp(this->inside_I(j,i,k));
	    }
	    out << endl;
	  }
	  else {
	    out << "I log [" << i << "," << j << "] ::";
	    out << "  (cl) " << this->inside_C(i,j) << "  (cr) " << this->inside_C(j,i);
	    for (k=0; k<_K; ++k) {
	      out << "  (ul_" << k << ") " << this->inside_I(i,j,k) << "  (ur_" << k << ") " << this->inside_I(j,i,k);
	    }
	    out << endl;
	  }
	}
      }

      // OUTSIDES
      if (E)
	out << "O C [" << -1 << "," << _N-1 << "] :: " << exp(this->outside_C_root(_N-1)) << endl;
      else
	out << "O log C [" << -1 << "," << _N-1 << "] :: " << this->outside_C_root(_N-1) << endl;

      for (i=0; i<_N; ++i) {
	if (E) {
	  out << "O [" << -1 << "," << i << "] ::";
	  out << "  (cl) " << exp(this->outside_I_root(i)) << endl;
	}
	else {
	  out << "O log [" << -1 << "," << i << "] ::";
	  out << "  (cl) " << this->outside_I_root(i) << endl;
	}
      }

      for (o=1; o<_N; ++o) {
	for (i=0; i<_N-o; ++i) {
	  j = i + o;
	  if (E) {
	    out << "O [" << i << "," << j << "] ::";
	    out << "  (cl) " << exp(this->outside_C(i,j)) << "  (cr) " << exp(this->outside_C(j,i));
	    out << "  (ul) " << exp(this->outside_I(i,j)) << "  (ur) " << exp(this->outside_I(j,i)) << endl;
	  }
	  else {
	    out << "O log [" << i << "," << j << "] ::";
	    out << "  (cl) " << this->outside_C(i,j) << "  (cr) " << this->outside_C(j,i);
	    out << "  (ul) " << this->outside_I(i,j) << "  (ur) " << this->outside_I(j,i) << endl;
	  }
	}
      }

    }
  }; /* end marg class definition */


  template <typename X, typename S>
  double ProjDep1::partition(const Configuration& c, const X& x, S& scores) {
    const int N = x.size();
    ChartIO chart(c.L, N);
    inside(c, scores, chart);
    return chart.log_Z();
  }


  template <typename X, typename S, typename M>
  double ProjDep1::marginals(const Configuration& c, const X& x, S& scores, M& mu) {
    mu.initialize(R::Configuration(c.L), x, 0); 
    const int N = x.size();    
    ChartIO chart(c.L, N);
    inside(c, scores, chart);
    outside(c, scores, chart);
    /* copy the log-marginals to the output structure of marginals */
    PartDep1 r; 
    for(r.l = 0; r.l < c.L; ++r.l) {
      for(r.m = 0; r.m < N; ++r.m) {
	for(r.h = -1; r.h < N; ++r.h) {
	  if (r.m==r.h) continue;	  
	  mu(r) = exp(chart.log_marginal(r.h,r.m,r.l));
	}
      }
    }
    return chart.log_Z();
  }



  template <typename S>
  void ProjDep1::inside(const Configuration& c, S& score, ChartIO& chart) {
    const int N = chart.N();
    PartDep1 r; 
    
    int W = N;    // max width
    if (c.multiroot) W++;

    for (int w=1; w<W; ++w) {

      /*
       * The code in this conditional treats the structures headed at the root.
       * The operations are the same than with regular tokens, except for two
       * issues: the root has its own inside calls, and the root only can be
       * head, not modifier
       */
      if (c.multiroot) {
	int e = -1+w;
	r.h = -1; 
	r.m = e; 

	double psco = chart.inside_C(e,0);   /*  */

	for (int k=0; k<e; ++k) {
	  psco = ChartIO::log_add(psco, chart.inside_C_root(k) + chart.inside_C(e,k+1));
	}

	// uncomplete structures
	for (r.l=0; r.l<c.L; ++r.l) {
	  chart.inside_I_root_set(e, r.l, psco + score(r));
	}

	// complete structure
	for (int k=0; k<=e; ++k) {
	  double cl = chart.inside_C(k,e);
	  for (int l=0; l<c.L; ++l) {
	    if (k==0 && l==0)
	      chart.inside_C_root_set(e, chart.inside_I_root(k,l) + cl);
	    else
	      chart.inside_C_root_add(e, chart.inside_I_root(k,l) + cl);
	  }
	}
      }

      for (int s=0; s<N-w; ++s) {
	int e = s+w;

	double psco = chart.inside_C(s,s) + chart.inside_C(e,s+1);
	for (int k=s+1; k<e; ++k) {
	  psco = ChartIO::log_add(psco, chart.inside_C(s,k) + chart.inside_C(e,k+1));
	}

	// headed at s
	r.h = s; 
	r.m = e; 
	for (r.l=0; r.l<c.L; ++r.l) {
	  chart.inside_I_set(s,e,r.l, psco + score(r));
	}

	// headed at e
	r.h = e; 
	r.m = s; 
	if (s!=-1) {
	for (r.l=0; r.l<c.L; ++r.l) {
	  chart.inside_I_set(e,s,r.l, psco + score(r));
	  }
	}

	// Complete spans

	for(int k=s+1; k<=e; ++k) {
	  double cl = chart.inside_C(k,e);
	  for (int l=0; l<c.L; ++l) {
	    if (k==s+1 && l==0)
	      chart.inside_C_set(s,e, chart.inside_I(s,k,l) + cl);
	    else
	      chart.inside_C_add(s,e, chart.inside_I(s,k,l) + cl);
	  }
	  //	  cout << "INSIDE C Left " << s << " " << e << " " << r << " .... " << chart.inside_C(s,e) << endl;
	}
	//	cout << "INSIDE C Left " << s << " " << e << " is " << chart.inside_C(s,e) << endl;
	
	for(int k=s; k<e; ++k) {
	  double cr = chart.inside_C(k,s);
	  for (int l=0; l<c.L; ++l) {
	    if (k==s && l==0)
	      chart.inside_C_set(e,s, cr + chart.inside_I(e,k,l));
	    else
	      chart.inside_C_add(e,s, cr + chart.inside_I(e,k,l));
	  }
	}
      }
    }

    if (!c.multiroot) {
      // STRUCTURES AT ROOT WORD
      r.h = -1; 
      for (int s=0; s<N; ++s) {
	r.m = s;
	for (r.l=0; r.l<c.L; ++r.l) {
	  double aux = score(r) + chart.inside_C(s,0);
	  chart.inside_I_root_set(s, r.l, aux );

	  if (s==0 && r.l==0)
	    chart.inside_C_root_set(N-1, aux + chart.inside_C(s,N-1) );
	  else
	    chart.inside_C_root_add(N-1, aux + chart.inside_C(s,N-1) );
	}
      }
    }

  }



  template <typename S>
  void ProjDep1::outside(const Configuration& config, S& score, ChartIO& chart) {
    const int N = chart.N();

    int w,s,e,k,r;
    double aux, auxL, auxR;
    PartDep1 part; 

    // Root
    chart.outside_C_root_set(N-1, (double) log(1.0));
    if (!config.multiroot) {
      for (s=0; s<N; ++s) {
	chart.outside_I_root_set(s, chart.outside_C_root(N-1) + chart.inside_C(s,N-1));
      }
    }

    int W = config.multiroot ? N : N-1;
    for (w=W; w>0; --w) {

      /* Structures headed at the root token, treated specially when multiple roots are possible*/
      if (config.multiroot) {
	e = -1+w;
	part.h = -1; 

	// complete signature
	int c = 0;   /* initialized? */
	for (r=e+1; r<N; ++r) {
	  aux = chart.outside_I_root(r) + chart.inside_C(r,e+1);
	  part.m = r; 
	  for (part.l=0; part.l<config.L; ++part.l) {
	    if (!c++)
	      chart.outside_C_root_set(e, aux + score(part));
	    else
	      chart.outside_C_root_add(e, aux + score(part));
	  }
	}

	// incomplete
	/* initialize with r=e */
	chart.outside_I_root_set(e, chart.outside_C_root(e) + chart.inside_C(e,e));
	for (r=e+1; r<N; ++r) {
	  chart.outside_I_root_add(e, chart.outside_C_root(r) + chart.inside_C(e,r) );
	}
      }

      /* Structures spanning from s to e */
      for (s=0; s<N-w; ++s) {
	e = s+w;
	
	/// COMPLETE signatures

	// when c==0 the cell is initialized (with set), otherwise new values are added to the existing ones
	int c = 0;

	// LEFT
	// first the structures coming from the root;
	if (config.multiroot || e==N-1) {
	  // initialization with k=0
	  c = 1;
	  chart.outside_C_set(s,e, chart.outside_C_root(e) + chart.inside_I_root(s,0));
	  for (k=1; k<config.L; ++k) {
	    chart.outside_C_add(s,e, chart.outside_C_root(e) + chart.inside_I_root(s,k));
	  }
	}

	for (r=0; r<s; ++r) {
	  for (k=0; k<config.L; ++k) {
	    if (!c++)
	      chart.outside_C_set(s,e, chart.outside_C(r,e) + chart.inside_I(r,s,k) );
	    else
	      chart.outside_C_add(s,e, chart.outside_C(r,e) + chart.inside_I(r,s,k) );
	  }
	}
	
	for (r=e+1; r<N; ++r) {
	  auxL = chart.outside_I(s,r) + chart.inside_C(r,e+1);

	  part.h = s; 
	  part.m = r; 	  
	  for (part.l=0; part.l<config.L; ++part.l) {
	    if (!c++)
	      chart.outside_C_set(s,e, auxL + score(part));
	    else
	      chart.outside_C_add(s,e, auxL + score(part));
	  }

	  part.h = r; 
	  part.m = s; 	  
	  auxR = chart.outside_I(r,s) + chart.inside_C(r,e+1);
	  for (part.l=0; part.l<config.L; ++part.l) {
	    chart.outside_C_add(s,e, auxR + score(part));
	  }
	}


	// RIGHT
	c = 0;
	// first the structures coming from the root;
	if (s==0 || config.multiroot) {
	  // initialization with k=0
	  c = 1;
	  aux = chart.outside_I_root(e);
	  if (s>0) aux += chart.inside_C_root(s-1);
	  chart.outside_C_set(e,s, aux + score(-1,e,0));
	  for (k=1; k<config.L; ++k) {
	    chart.outside_C_add(e,s, aux +  score(-1,e,k));
	  }
	}

	for (r=e+1; r<N; ++r) {
	  for (k=0; k<config.L; ++k) {
	    if (!c++)
	      chart.outside_C_set(e,s, chart.outside_C(r,s) + chart.inside_I(r,e,k) );
	    else
	      chart.outside_C_add(e,s, chart.outside_C(r,s) + chart.inside_I(r,e,k) );
	  }
	}

	for (r=0; r<s; ++r) {
	  auxL = chart.outside_I(r,e) + chart.inside_C(r,s-1);
	  for (k=0; k<config.L; ++k) {
	    if (!c++)
	      chart.outside_C_set(e,s, auxL + score(r,e,k));
	    else
	      chart.outside_C_add(e,s, auxL + score(r,e,k));
	  }

	  auxR = chart.outside_I(e,r) + chart.inside_C(r,s-1);
	  for (k=0; k<config.L; ++k) {
	    chart.outside_C_add(e,s, auxR + score(e,r,k));
	  }

	}


	/// uncomplete signatures

	// LEFT part : initialize with r=e
	chart.outside_I_set(s,e, chart.outside_C(s,e) + chart.inside_C(e,e));

	// rest of splitting points
	for (r=e+1; r<N; ++r) {
	  chart.outside_I_add(s,e, chart.outside_C(s,r) + chart.inside_C(e,r));
	}

	// RIGHT part : initialize with r=s
	chart.outside_I_set(e,s, chart.outside_C(e,s) + chart.inside_C(s,s));

	// rest of splitting points
	for (r=0; r<s; ++r) {
	  chart.outside_I_add(e,s, chart.outside_C(e,r) + chart.inside_C(s,r));
	}
      }

    }


  }



 

}
