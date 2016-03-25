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

#ifndef _CONSTR_GRAMMAR
#define _CONSTR_GRAMMAR

#include <string>
#include <list>
#include <map>
#include <set>

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///   Class condition implements a condition of a CG rule 
  ////////////////////////////////////////////////////////////////

  class condition {
  private:
    /// is it a negative condition?
    bool neg;
    /// position to check for
    int pos;
    /// star in the position
    bool starpos;
    /// terms ORed in the condition (each term is a <lemma>, (form), or TAG)
    std::list<std::wstring> terms;
    /// terms in barrier (if any)
    std::list <std::wstring> barrier;

  public:
    /// constructor
    condition();

    /// empty condition
    void clear();
    /// set negative condition flag
    void set_neg(bool);
    /// set position value
    void set_pos(int,bool);
    /// set terms list
    void set_terms(const std::list<std::wstring>&);
    /// set barrier terms list
    void set_barrier(const std::list<std::wstring>&);
    /// find out whether it is a negative condition
    bool is_neg() const;
    /// get position
    int get_pos() const;
    /// find out whethe position has a "*"
    bool has_star() const;
    /// get terms to check
    std::list<std::wstring> get_terms() const;
    /// find out if there are barrier terms
    bool has_barrier() const;
    /// get barrier terms
    std::list<std::wstring> get_barrier() const;
  };


  ////////////////////////////////////////////////////////////////
  ///   Class rule implements a rule of a CG.
  ////////////////////////////////////////////////////////////////

  class ruleCG : public std::list<condition> {
  protected:
    /// compatibility value
    double weight;
    /// Head of the rule
    std::wstring head;

  public:
    /// Constructors of the subclass rule 
    ruleCG();

    /// set rule head.
    void set_head(const std::wstring &);
    /// set rule compatibility.
    void set_weight(double);
    /// get rule head.
    std::wstring get_head() const;
    /// set rule compatibility.
    double get_weight() const;
  };

  ////////////////////////////////////////////////////////////////
  /// auxiliary class to store sets defined in the CG
  ////////////////////////////////////////////////////////////////


  class setCG : public std::set<std::wstring> {
  public:
    typedef enum {FORM_SET, LEMMA_SET, SENSE_SET, CATEGORY_SET} setType;
    setType type;
  };



  ////////////////////////////////////////////////////////////////
  ///   Class constraint_grammar implements a pseudo CG, ready 
  /// to be used from a relax PoS tagger.
  ////////////////////////////////////////////////////////////////

  class constraint_grammar : public std::multimap<std::wstring,ruleCG> {
  public:
    /// flag to remember if rules affecting senses where used
    bool senses_used;

    /// map to store -by name- all sets defined in the CG
    std::map<std::wstring,setCG> sets;

    /// Create a grammar loading it from a file
    constraint_grammar(const std::wstring &);

    /// add to the given list all rules with a head starting with the given string
    void get_rules_head(const std::wstring &, std::list<ruleCG> &) const;
  };

} // namespace

#endif
