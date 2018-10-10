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

#ifndef _RELAX
#define _RELAX

#include <string>
#include <list>
#include <vector>

namespace freeling {


  class constraint_element {
  public:
    constraint_element();
    constraint_element(int, int, double*);
    int var,lab; // target (var,lab) --used for tracing mainly
    double *w;   // pointer to (var,lab) weigth in problem matrix.
  };
 
  
  ////////////////////////////////////////////////////////////////
  ///
  ///  The class constraint implements a constraint for the 
  /// relaxation labelling algorithm.
  ///
  ////////////////////////////////////////////////////////////////

  class constraint : public std::vector<std::vector<constraint_element> > {
  private:
    double compatibility;

  public:
    /// Constructor
    constraint(int);

    /// set/get compatibility value
    void set_compatibility(double);
    double get_compatibility() const;
  };

  ////////////////////////////////////////////////////////////////
  ///
  ///  The class label stores all information related to a 
  /// variable label in the relaxation labelling algorithm.
  ///
  ////////////////////////////////////////////////////////////////

  class label {
    friend class problem;
    friend class relax;

  protected:
    /// label weigth at current and next iterations
    double weight[2];
    /// label name, for user convenience
    std::wstring name;
    /// list of constraints for the label
    std::list<constraint> constraints;

  public:
    /// Constructor
    label();
    double get_weight(int) const;
    void set_weight(int, double);
    std::wstring get_name() const;

  };

  ////////////////////////////////////////////////////////////////
  ///
  ///  The class problem stores the structure of a problem,
  ///  namely, a vector with a position for each variable
  ///  to consider, and for each variable, a list of initial
  ///  weights for each possible label.
  ///   Variables and labels are unnamed, and sequentially 
  ///  numbered. The caller application must keep track of 
  ///  the meaning of each variable and label position.
  ///
  ////////////////////////////////////////////////////////////////

  class problem {
    friend class relax;
  protected:
    /// table with variable-labels in the CLP.
    std::vector<std::vector<label> > vars;
    /// variable names, for user convenience
    std::vector<std::wstring> varnames;
    /// which of both weight sets are we using and which are we computing
    int CURRENT, NEXT;

  public:
    /// Constructor
    problem(int);

    /// set variable name 
    void set_var_name(int, const std::wstring &);
    /// get variable name
    std::wstring get_var_name(int i) const;
    /// get number of labels
    int get_num_labels(int i) const;
    /// get label name
    std::wstring get_label_name(int i, int j) const;

    /// add a label and its weight (and its name if needed) to the given variable
    void add_label(int, double, const std::wstring &lb=L"");
    /// add a new constraint to the problem
    void add_constraint (int, int, const std::list<std::list<std::pair<int,int> > > &, double);
    /// get best label(s) --hopefully only one-- for given variable
    std::list<int> best_label(int) const;
    /// check whether convergence was achieved
    bool there_are_changes(double) const;
    /// Exchange tables, get ready for next iteration
    void next_iteration();
  };



  ////////////////////////////////////////////////////////////////
  ///
  ///  The class relax implements a generic solver for consistent
  /// labelling problems, using relaxation labelling algorithm.
  ///
  ////////////////////////////////////////////////////////////////

  class relax {
  private:
    /// Maximum number of iterations in case of not converging
    int MaxIter;
    /// Scale factor for label supports
    double ScaleFactor;
    /// epsilon value to decide whether or not an iteration has caused relevant weight changes
    double Epsilon;

    /// private methods
    double NormalizeSupport(double) const;

  public:
    /// Constructor
    relax(int, double, double);

    /// solve consistent labelling problem
    void solve(problem &) const;
    /// change scale factor 
    void set_scale_factor(double);
  };

} // namespace

#endif
