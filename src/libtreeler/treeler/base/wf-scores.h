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
 * \file   wf-scores.h
 * \brief  Score components computed by linear inner products with features
 * \author Xavier Carreras
 */

#include "treeler/base/scores.h"

#ifndef TREELER_WFSCORES_H
#define TREELER_WFSCORES_H

namespace treeler {

  template <typename Symbols, typename X, typename R, typename FGen, template <typename,typename> class P = Parameters>
  class WFScorer;
  
  /** 
   * \brief  Scores calculated as inner products between features of parts and a parameter vector
   * \author Xavier Carreras
   * \ingroup base
   * 
   * \tparam X The type of input structures
   * 
   * \tparam R The type of parts
   * 
   * \tparam FGen The type of feature generator. It is expected that \c
   * F::FIdx indicates the type of feature indices that the feature
   * generator produces.
   *
   * This class defines the following subtypes:
   * - \c W : The type of parameter vector
   * - \c Scorer : The type of scorers associate to these scores
   *
   */
  template <typename X, typename R, typename FGen, template <typename,typename> class P = Parameters>
  class WFScores : public BaseScores<WFScores<X,R,FGen,P>> {
  public:
    typedef typename FGen::FIdx FIdx; 
    typedef P<FIdx,double> W;
    typedef typename P<FIdx,double>::FVec FVec; 
  private:
    W* _w;
    typename FGen::Features _f; 
    const X* _x;
  public:
    WFScores() : _w(NULL), _x(NULL) {}
    WFScores(const W& w, const X& x) : _w(&w), _x(&x) {}
    ~WFScores() {}
    
    void set_w(W* w) { assert(_w==NULL); _w = w; }
    W& w() { return *_w; }
    typename FGen::Features& f() { return _f; }

    void new_x(const X& x) { 
      assert(_w!=NULL);
      assert(_x==NULL);
      if (_x!=NULL) {
	_f.clear();
      }
      _x = &x;
    }

    void new_xy(const X& x, const Label<R>& y) { 
      new_x(x); 
    }
    void new_xy(const Example<X,Label<R>>& xy) { 
      new_x(xy.x()); 
    }

    void clear() {
      assert(_x!=NULL);
      _f.clear();
      _x = NULL;
    }
    
    /**
     * \brief Returns the score of part \c r
     * \param r The part to score
     */
    inline double operator()(const R& r) {
      const FVec* f = _f.phi(r);
      // cerr << "FVec " << typeid(FGen).name()  << " (" << r << ") : ";
      // IOFVec::write(cerr, *f); 
      // cerr << endl;
      double s = _w->dot(*f);       
      _f.discard(f);
      return s;
    }

    //! \brief Scores given part 
    template <typename ...A>
    inline double operator()(A&&... args) {
      const FVec* f = _f(args...);
      double s = _w->dot(*f);       
      _f.discard(f);
      return s;      
    }

    inline vector<double> score_block(const R& r, const std::list<int>& offsets) {
      vector<double> s(offsets.size());
      FVec* f = _f(r);
      size_t i = 0; 
      for (int o : offsets) {
	_f.label_offset(f, o); 
	s[i] = _w->dot(*f);
	++i;
      }
      _f.discard(f);
      return std::move(s);
    }


  };

  /** 
   * \brief  A scorer component that computes \c WFScores
   * \author Xavier Carreras
   * \ingroup base
   */
  template <typename Symbols, typename X, typename R, typename FGen, template <typename,typename> class P>
  class WFScorer {
  public:
    typedef WFScores<X,R,FGen,P> Scores;
    typedef typename Scores::W W;
  private:
    W* _w; 
    FGen* _f; 
  public:
    WFScorer(const Symbols& sym) : _w(NULL), _f(NULL)  {}
    WFScorer(const Symbols& sym, const W& w, const FGen& f) : _w(&w), _f(&f)  {}
    ~WFScorer() {
      delete _w;
      delete _f;
    }
    
    void set_w(W* w) { assert(_w==NULL); _w = w; }
    void set_f(FGen* f) { assert(_f==NULL); _f = f; }    
    void unset_w() { _w = NULL; }
    void unset_f() { _f = NULL; }    

    FGen& f() { return *_f; }
    W& w() { return *_w; }

    void scores(const X& x, Scores& s) const {
      s.set_w(_w); 
      s.new_x(x); 
      _f->features(x, s.f()); 
    }

    template <typename Y>
    void scores(const X& x, const Y& y, Scores& s) const {
      scores(x, s); 
    }

    template <typename Y>
    void scores(const Example<X,Y>& xy, Scores& s) const {
      scores(xy.x(), s); 
    }
  };


  /** 
   * \brief  A feature generator that returns an empty feature vector for any part
   * \author Xavier Carreras
   * \ingroup base
   */
  template <typename X, typename R, typename FIdx0=FIdxBits> 
  class NullFeatures {
  public:    
    typedef FIdx0 FIdx; 
    typedef FeatureVector<FIdx> FVec; 
    
    class Features {
    public:  
      const FVec* phi(const X& x, const R& r) const {
	FVec* f = new FVec(); 
	f->n = 0; 
	f->idx = NULL; 
	f->val = NULL;
	f->offset = 0;
	f->next = NULL;
	return f; 
	
      }
      void discard(const FVec* f, const X& x, const R& r) const {
	delete f; 
      }
      void clear() {}     
    };

    int spaces() const { return 1; }
    void features(const X&x, Features& features) {}

    void phi_start_pattern(const X& x) {};
    void phi_end_pattern(const X& x) {};

    const FVec* phi(const X&x, const R& r) {
      FVec* f = new FVec(); 
      f->n = 0; 
      f->idx = NULL; 
      f->val = NULL;
      f->offset = 0;
      f->next = NULL;
      return f; 
    }

    void discard(const FVec* f, const X& x, const R& r) {
      delete f; 
    }
  };

  // forward declaration of NullFScorer
  template <typename X, typename R, typename FGen>
  class NullFScorer;
  

  /** 
   * \brief  Null Scores with an FGen component (for development)
   * \author Xavier Carreras
   * \ingroup base
   * 
   * \tparam X The type of input structures
   * 
   * \tparam R The type of parts
   * 
   * \tparam FGen The type of feature generator. It is expected that \c
   * F::FIdx indicates the type of feature indices that the feature
   * generator produces.
   *
   * This class defines the following subtypes:
   * - \c Scorer : The type of scorers associate to these scores
   */
  template <typename X, typename R, typename FGen>
  class NullFScores {
  public:
    typedef typename FGen::FIdx FIdx; 
    typedef NullFScorer<X,R,FGen> Scorer;

    struct Configuration {};

  private:
    typename FGen::Features _f; 
    const X* _x;
  public:
    NullFScores() : _x(NULL) {}
    NullFScores(const X& x) : _x(&x) {}
    ~NullFScores() {}
    
    typename FGen::Features& f() { return _f; }

    void new_x(const X& x) { 
      assert(_x==NULL);
      if (_x!=NULL) {
	_f.clear();
      }
      _x = &x;
    }

    void new_xy(const X& x, const Label<R>& y) { 
      new_x(x); 
    }
    void new_xy(const Example<X,Label<R>>& xy) { 
      new_x(xy.x()); 
    }

    void clear() {
      assert(_x!=NULL);
      _f.clear();
      _x = NULL;
    }
    
    /**
     * \brief Returns the score of part \c r
     * 
     * \param r The part to score
     */
    inline double operator()(const R& r) {
      //      const FVec* f = _f.phi(*_x,r);
      double s = 0;
      // _f.discard(f, *_x, r);
      return s;
    }
  };

  /** 
   * \brief  A scorer component that computes \c NullFScores
   * \author Xavier Carreras
   * \ingroup base
   *
   * 
   */
  template <typename X, typename R, typename FGen>
  class NullFScorer {
  public:
    typedef NullFScores<X,R,FGen> Scores;
  private:
    FGen* _f; 
  public:
    NullFScorer() : _f(NULL)  {}
    NullFScorer(const FGen& f) : _f(&f)  {}
    ~NullFScorer() {
      delete _f;
    }
    
    void set_f(FGen* f) { assert(_f==NULL); _f = f; }    
    void unset_f() { _f = NULL; }    

    FGen& f() { return *_f; }

    void scores(const X& x, Scores& s) const {
      s.new_x(x); 
      _f->features(x, s.f()); 
    }

    void scores(const Example<X,Label<R>>& xy, Scores& s) const {
      scores(xy.x(), s); 
    }
  };



}

#endif
