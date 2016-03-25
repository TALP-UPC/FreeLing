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

#ifndef _FEX_RULE
#define _FEX_RULE

#include <map>
#include <set>
#include "freeling/morfo/language.h"
#include "freeling/morfo/tagset.h"
#include "freeling/regexp.h"

#define OP_NONE 0
#define OP_AND 1
#define OP_OR 2

namespace freeling {

  class fex_rulepack;  // predeclaration

  ////////////////////////////////////////////////////////////////
  /// Store status about extracted features
  ////////////////////////////////////////////////////////////////

  class fex_status : public processor_status {
  public:
    /// for each rule id, store list of features extracted 
    /// for each word in sentence
    std::map<std::wstring,std::map<int,std::list<std::wstring> > > features;
    /// for each condition id that requires it, store substrings
    /// matching latest regex application
    std::map<std::wstring,std::vector<std::wstring> > re_result;
  };

  ////////////////////////////////////////////////////////////////
  ///  Class fex_condition stores a condition to be 
  ///  checked on the target.
  ////////////////////////////////////////////////////////////////

  class fex_condition {
  private:
    /// condition id
    std::wstring cid;
    /// function to perform (check Regex, search a file, etc) 
    std::wstring function;
    /// item on which perform the check (word, lemma, tag, any-tag, etc)
    std::wstring focus;
    /// substring to use as a separator in splits 
    std::wstring split;
    /// literal to compare against in "is" operations. 
    /// For "match" and set operations, it holds the string form of
    /// the parameter, just for debugging purposes
    std::wstring literal;
    /// set file contents (if needed by function)
    std::set<std::wstring> *fileset;
    /// regexp (if needed by function)
    freeling::regexp match_re;

    /// whether the function has a negation 
    bool negated;
    /// remember if the rule is trivial
    bool cond_true;

    /// auxiliar regexs to parse rules
    static const freeling::regexp split_re;

    ///  Obtain the target(s) of a condition 
    std::list<std::wstring> get_target(const word &, const tagset&) const;    

  public:
    // constructor
    fex_condition();
    /// constructor, given id, function, focus, and filename/regex
    fex_condition(const std::wstring&,const std::wstring&,const std::wstring&,const std::wstring&, 
                  std::map<std::wstring,std::set<std::wstring> > &);
    /// Copy constructor
    fex_condition(const fex_condition &);
    /// assignment
    fex_condition& operator=(const fex_condition&);

    /// evaluate whether a word meets the condition.    
    bool check(const word&, const tagset&, fex_status *) const;
    /// check whether the condition is "true" (literally) and will match any words.
    bool is_true() const;
    /// get i-th subexpression match of last RE application
    std::wstring get_match(int, fex_status *) const;
    /// print condition to stderr in the given tracelevel (debug purposes only)
    void trace(int) const;
  };

  ////////////////////////////////////////////////////////////////
  ///  Class feature_function is an abstrac class to enable
  /// calling module to define user-custom feature functions
  ////////////////////////////////////////////////////////////////

  class feature_function {  
  public: 
    virtual void extract (const sentence &, int, std::list<std::wstring> &) const =0;
    /// Destructor
    virtual ~feature_function() {};
  };


  ////////////////////////////////////////////////////////////////
  ///  Class fex_rule stores a feature extraction rule.
  ////////////////////////////////////////////////////////////////

  class fex_rule {  
  private:
    /// rule id
    std::wstring rid;
    /// rule pattern to build feature
    std::wstring pattern;
    /// range around target where rule should be applied
    int left,right;
    /// additional condition to be met by the target.
    std::list<fex_condition> conds;
    /// whether conditions should be joined with AND or OR
    int operation;
    const std::map<std::wstring,const feature_function*> & feat_functs;

    /// auxiliar regexs to parse rules
    static const freeling::regexp rulepat;
    static const freeling::regexp rulepat_anch;
    static const freeling::regexp subexpr;
    static const freeling::regexp featfun;

    /// replace marked chunks in a rule pattern (e.g.: $t(0), $l(-1),...)
    /// with appropriate instance for given word
    void pattern_instance(const sentence &, int, const tagset &, std::list<std::wstring> &) const;
    void get_replacements(const std::wstring &, const word &, const tagset &, std::list<std::wstring> &) const;

  public:
    /// Constructor, given id, pattern, rang, and condition:(focus, function, param)
    fex_rule (const std::wstring &, const std::wstring &, const std::wstring &, int, 
              const std::list<fex_condition> &, const std::map<std::wstring,const feature_function*> &);
    /// Copy constructor
    fex_rule(const fex_rule &);
    /// assignment
    fex_rule& operator=(const fex_rule&);

    // get rule id
    std::wstring get_id() const;
    /// check whether a word matches the rule, precompute the 
    /// feature, and store it.
    void precompute(const sentence&, int, const tagset&) const;
    /// Use precomputed features to extract actual features for
    /// word "i" as seen form word "anch".
    void extract(const sentence&, int, int, const tagset&, std::list<std::wstring> &) const;
    /// get left limit of range
    int get_left() const;
    /// get right limit of range
    int get_right() const;

    /// check a list of conditions with and/or on a word.
    static bool check_conds(const std::list<fex_condition> &, int, const word &, const tagset &, fex_status*);

    /// print rule to stderr in the given tracelevel (debug purposes only)
    void trace(int) const;
  };


  ////////////////////////////////////////////////////////////////
  ///  Class fex_rulepack stores a batch of feature rules
  ///  to be applied to the same kind of targets.
  ////////////////////////////////////////////////////////////////

  class fex_rulepack  {

  public:
    /// condition required for a word to be considered a target for this pack
    std::list<fex_condition> conds;
    /// operation to combine conditions (and/or)
    int operation;
    /// rules in the pack.
    std::list<fex_rule> rules;

    /// Constructor
    fex_rulepack();
    /// Copy constructor
    fex_rulepack(const fex_rulepack &);
    /// assignment
    fex_rulepack& operator=(const fex_rulepack&);

    /// print rule to stderr in the given tracelevel (debug purposes only)
    void trace(int) const;
  };

} // namespace

#endif

