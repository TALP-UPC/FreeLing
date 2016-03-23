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

#ifndef _CHART
#define _CHART

#include <list>
#include <vector>
#include <string>

#include "freeling/morfo/language.h"
#include "freeling/morfo/grammar.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///   Class edge stores all information in a chart edge.
  ////////////////////////////////////////////////////////////////

  class edge : public rule {

  private:
    /// Part of the rule already matched
    std::list<std::wstring> matched;
    // list of cells that matched the currently solved part of the edge.
    // (-1,-1) stands for unary rules producing cell self-references.
    std::list<std::pair<int,int> > backpath;

  public:
    /// Constructors of the subclass edge
    edge(const std::wstring&, const std::list<std::wstring> &, const int posgov);
    edge();
    ~edge();

   /// get matched part of the rule.
    const std::list<std::wstring> get_matched() const;
    /// get list of cells used to satisfy edge rule
    const std::list<std::pair<int,int> > get_backpath() const;
    /// Check if the edge is complete (inactive) or not
    bool active() const;
    /// Advance the edge one position
    void shift(int,int);
   
  };

  ////////////////////////////////////////////////////////////////
  ///   Class cell stores all information in a chart cell.
  ////////////////////////////////////////////////////////////////

  typedef std::list<edge> cell; 


  ////////////////////////////////////////////////////////////////
  ///   Class chart contains an array of cells 
  ///  that constitute a chart.
  ////////////////////////////////////////////////////////////////

  class chart {

  private:

    /// chart table
    std::vector<cell> table;
    /// dimension of the chart table (length of the sentece to parse)
    int size;
    /// grammar to use
    const grammar &gram;

    /// compare two edges when extracting a tree
    bool better_edge(const edge &, const edge&) const;
    /// obtain a list of cells that cover the subtree under cell (a,b)
    std::list<std::pair<int,int> > cover (int a, int b) const;
    /// compute position of the cell inside the vector
    int index(int i, int j) const;
    /// find out whether the cell (i,j) has some inactive edge
    /// whose head is the given category 
    bool can_extend(const std::wstring &, int, int) const;
    /// Complete edges in a cell (ce) after inserting a terminal or an inactive edge,
    ///  using rules whose right part starts with the right token (which may be wildcarded)
    void find_all_rules(const edge &, cell &, int, int) const;
    /// check match between a (possibly) wildcarded string and a literal.
    bool check_match(const std::wstring &, const std::wstring &) const;

    void dump() const;

  public:
    /// constructor
    chart(const grammar &);
    /// destructor
    ~chart();

    /// Get size of the table
    int get_size() const;

    /// load sentece and init parsing (fill up first row of chart)
    void load_sentence(const sentence &, int k=0);

    /// Do the parsing
    void parse();

    /// navigate through the chart and obtain a parse tree.
    parse_tree get_tree(int, int, const std::wstring & =L"") const;
  };

} // namespace

#endif

