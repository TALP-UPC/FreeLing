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
 * \file   script-shag.h
 * \brief  A script for parsing with SHAGs 
 * \author Xavier Carreras
 */

#ifndef TREELER_SCRIPT_SHAG
#define TREELER_SCRIPT_SHAG

#include <string>

#include "treeler/control/control.h"
#include "treeler/control/script.h"

#include "treeler/dep/shag1.h"
#include "treeler/dep/shag2.h"
#include "treeler/dep/shagMax.h"

#include "treeler/dep/shagTT.h"
#include "treeler/dep/semiring.h"


namespace treeler {

  namespace control {

    /** 
     *  Extension of Model Types
     *
     */
    enum ModelTypesExt { 
//       SHAG1B = ModelTypes::_EXTENSION, 
//       SHAG1TT,
//       SHAG2G,
//       SHAGMaxTT,
//       SHAGCMaxTT,
//       SHAGCMaxG,
//       SHAGCHyperTT,
//       SHAGCAddTT,
//       SHAGCLogSumTT,
      SHAGTT_MAX = ModelTypes::_EXTENSION,
      SHAGTT_ADD,
      SHAGTT_LOGADD,
      SHAGTT_HYPER
    };
    

    /** 
     *  Definition of SHAG1B
     *
     */
//     template <>
//     class Model<SHAG1B> :  public Model<DEP1> {
//     public:
//       static string name() { return "shag1B"; }
//       static string brief() { return "first-order projective dependency parsing using boxes"; }
//       typedef Sentence X; 
//       typedef PartDep1 R; 
//       typedef Label<R> Y; 
//       typedef Boxes<X,Y,R> I; 
//     };

    /** 
     *  Definition of SHAG1TT
     *
     */
//     template <>
//     class Model<SHAG1TT> :  public Model<DEP1> {
//     public:
//       static string name() { return "shag1TT"; }
//       static string brief() { return "first-order projective dependency parsing using triangles and trapezoids"; }
//       typedef Sentence X; 
//       typedef PartDep1 R; 
//       typedef Label<R> Y; 
//       typedef TriTra<X,Y,R> I; 
//     };

    /** 
     *  Definition of SHAG2G
     *
     */
//     template <>
//     class Model<SHAG2G> :  public Model<DEP2> {
//     public:
//       static string name() { return "shag2G"; }
//       static string brief() { return "second-order grandchildren projective dependency parsing using triangles and trapezoids"; }
//       typedef Sentence X; 
//       typedef PartDep2 R; 
//       typedef Label<R> Y; 
//       typedef Grandchildren<X,Y,R> I; 
//     };

    /** 
     *  Definition of SHAGMaxAdd
     *
     */
//     template <>
//     class Model<SHAGMaxTT> :  public Model<DEP1> {
//     public:
//       static string name() { return "shagMaxTT"; }
//       static string brief() { return "first-order projective dependency parsing using triangles and trapezoids + semiring operators: + -> max and x -> +"; }
//       typedef Sentence X; 
//       typedef PartDep1 R; 
//       typedef Label<R> Y; 
//       typedef MaxTT<X,Y,R> I; 
//     };

    /** 
     *  Definition of SHAGCMaxTT
     *
     */
//     template <>
//     class Model<SHAGCMaxTT> :  public Model<DEP1> {
//     public:
//       static string name() { return "shagCMaxTT"; }
//       static string brief() { return "first-order projective dependency parsing using abstract chart"; }
//       typedef Sentence X; 
//       typedef PartDep1 R; 
//       typedef Label<R> Y; 
//       typedef CMaxTT<X,Y,R> I; 
//     };

    /** 
     *  Definition of SHAGCMaxG
     *
     */
//     template <>
//     class Model<SHAGCMaxG> :  public Model<DEP2> {
//     public:
//       static string name() { return "shagCMaxG"; }
//       static string brief() { return "second-order grandchildren projective dependency parsing using abstract chart"; }
//       typedef Sentence X; 
//       typedef PartDep2 R; 
//       typedef Label<R> Y; 
//       typedef CMaxGrandchildren<X,Y,R> I; 
//     };

    /** 
     *  Definition of SHAGCHyperTT
     *
     */
//     template <>
//     class Model<SHAGCHyperTT> :  public Model<DEP1> {
//     public:
//       static string name() { return "shagCHyperTT"; }
//       static string brief() { return "first-order projective dependency parsing using abstract chart (HYPERGRAPH)"; }
//       typedef Sentence X; 
//       typedef PartDep1 R; 
//       typedef Label<R> Y; 
//       typedef CHyperTT<X,Y,R> I; 
//     };

    /** 
     *  Definition of SHAGCAddTT
     *
     */
//     template <>
//     class Model<SHAGCAddTT> :  public Model<DEP1> {
//     public:
//       static string name() { return "shagCAddTT"; }
//       static string brief() { return "first-order projective dependency parsing using abstract chart (ADD)"; }
//       typedef Sentence X; 
//       typedef PartDep1 R; 
//       typedef Label<R> Y; 
//       typedef CAddTT<X,Y,R> I; 
//     };

    /** 
     *  Definition of SHAGCLogSumTT
     *
     */
//     template <>
//     class Model<SHAGCLogSumTT> :  public Model<DEP1> {
//     public:
//       static string name() { return "shagCLogSumTT"; }
//       static string brief() { return "first-order projective dependency parsing using abstract chart (LOGSUM)"; }
//       typedef Sentence X; 
//       typedef PartDep1 R; 
//       typedef Label<R> Y; 
//       typedef CLogSumTT<X,Y,R> I; 
//     };

    /** 
     *  Definition of SHAGTT_MAX
     *
     */
    template <>
    class Model<SHAGTT_MAX> :  public Model<DEP1> {
    public:
      static string name() { return "shagTT_MAX"; }
      static string brief() { return "first-order projective dependency parsing"; }
      typedef Sentence X; 
      typedef PartDep1 R; 
      typedef Label<R> Y; 
      typedef ShagTT<X,Y,R, MAX> I; 
    };

    /** 
     *  Definition of SHAGTT_ADD
     *
     */
    template <>
    class Model<SHAGTT_ADD> :  public Model<DEP1> {
    public:
      static string name() { return "shagTT_ADD"; }
      static string brief() { return "first-order projective dependency parsing"; }
      typedef Sentence X; 
      typedef PartDep1 R; 
      typedef Label<R> Y; 
      typedef ShagTT<X,Y,R, ADD> I; 
    };

    /** 
     *  Definition of SHAGTT_LOGADD
     *
     */
    template <>
    class Model<SHAGTT_LOGADD> :  public Model<DEP1> {
    public:
      static string name() { return "shagTT_LOGADD"; }
      static string brief() { return "first-order projective dependency parsing"; }
      typedef Sentence X; 
      typedef PartDep1 R; 
      typedef Label<R> Y; 
      typedef ShagTT<X,Y,R, LOGADD> I; 
    };

    /** 
     *  Definition of SHAGTT_HYPER
     *
     */
    template <>
    class Model<SHAGTT_HYPER> :  public Model<DEP1> {
    public:
      static string name() { return "shagTT_HYPER"; }
      static string brief() { return "first-order projective dependency parsing"; }
      typedef Sentence X; 
      typedef PartDep1 R; 
      typedef Label<R> Y; 
      typedef ShagTT<X,Y,R, HYPER> I; 
    };

    /** 
     *  \brief A script for dumping part-factored objects of various kinds
     *  \ingroup control
     */  
    class ScriptSHAG: public Script {
    private:
      typedef ModelSelector<ScriptSHAG, /*DEP1, DEP2, SHAG1B, SHAG1TT, SHAG2G, SHAGMaxTT, SHAGCMaxTT, SHAGCMaxG, SHAGCHyperTT, SHAGCAddTT, SHAGCLogSumTT, */SHAGTT_MAX, SHAGTT_ADD, SHAGTT_LOGADD, SHAGTT_HYPER> GenericModel;
      
    public:
    ScriptSHAG()
      : Script("shag")
	{} 
      ~ScriptSHAG()
	{} 
      
      struct Configuration {
	string ifile;
	bool from_cin;
	int dump_scores; 
      };
      
      static std::string name() { return "shag"; }
      
      void usage(const std::string& msg);

      void run(const std::string& dir, const std::string& data_file);
      
      template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
      static void run(const string& name, bool help);

      template <typename Functor, typename X, typename Y, typename R, typename I, typename S, typename FGEN, typename IO>  
	static void forall(Functor& F, bool help);
      
      template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
	static void parse_old(const string& name, bool help);

      template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
	static void train(const string& name, bool help);

      template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
	static void print_all_derivations(const string& name, bool help);
      

      
      struct DumpMain {
      public:
	template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
	  void run(const string& name, bool help);
      };
      
      template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO> 
      struct DumpFunctor {
	public:
	  void operator()(const X&, const Y&,
			  typename R::Configuration&, 
			  typename I::Configuration&,
			  typename S::Scorer&,
			  IO&); 
	};


      struct ParseMain {
      public:
	template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
	  void run(const string& name, bool help);
      };
      
      template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO> 
      struct ParseFunctor {
	public:
	
	bool dump_scores;
	int correct_dep;
	int total_dep;
	int correct_tree;
	int total_tree;
	
	ParseFunctor() 
	: correct_dep(0), total_dep(0), correct_tree(0), total_tree(0)
	{}

	void operator()(const X&, const Y&,
			typename R::Configuration&, 
			typename I::Configuration&,
			typename S::Scorer&, 
			IO&); 
      };


      struct HyperMain {
      public:
	template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
	  void run(const string& name, bool help);
      };
      
      template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO> 
      struct HyperFunctor {
	public:

	typedef ShagTT<X,Y,R,HYPER> HyperParser;
	
	typename HyperParser::Configuration HPConfig; 

	bool dump_scores;
	int correct_dep;
	int total_dep;
	int correct_tree;
	int total_tree;
	
	HyperFunctor() 
	: correct_dep(0), total_dep(0), correct_tree(0), total_tree(0)
	{}

	void operator()(const X&, const Y&,
			typename R::Configuration&, 
			typename I::Configuration&,
			typename S::Scorer&, 
			IO&); 
      };


      struct HyperParseMain {
      public:
	template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
	  void run(const string& name, bool help);
      };
      
      template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO> 
      struct HyperParseFunctor {
	public:
	
	bool dump_scores;
	int correct_dep;
	int total_dep;
	int correct_tree;
	int total_tree;
	
	HyperParseFunctor() 
	: correct_dep(0), total_dep(0), correct_tree(0), total_tree(0)
	{}

	void operator()(const X&, const Y&,
			typename R::Configuration&, 
			typename I::Configuration&,
			typename S::Scorer&, 
			IO&); 
      };


      struct HyperSumMain {
      public:
	template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
	  void run(const string& name, bool help);
      };
      
      template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO> 
      struct HyperSumFunctor {
	public:
	
	bool dump_scores;
	
	HyperSumFunctor() {}

	void operator()(const X&, const Y&,
			typename R::Configuration&, 
			typename I::Configuration&,
			typename S::Scorer&, 
			IO&); 
      };
      
      
      
      struct SumMain {
      public:
	template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
	  static void run(const string& name, bool help);
      };

      template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO> 
      struct SumFunctor {
      public:
	void operator()(const X&, const Y&,
			typename R::Configuration&, 
			typename I::Configuration&,
			typename S::Scorer&,
			IO&); 
      };
      
      struct MarginalsMain {
      public:
	template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
	  static void run(const string& name, bool help);
      };


      template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO> 
      struct MarginalsFunctor {
      public:
	void operator()(const X&, const Y&,
			typename R::Configuration&, 
			typename I::Configuration&,
			typename S::Scorer&,
			IO&); 	
      };
      
      
    };
  }
}


#endif
