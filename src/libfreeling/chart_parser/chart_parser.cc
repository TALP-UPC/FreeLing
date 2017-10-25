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

#include "freeling/morfo/chart_parser.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"CHART_PARSER"
#define MOD_TRACECODE CHART_TRACE


  ////////////////////////////////////////////////////////////////
  /// Constructor.
  ////////////////////////////////////////////////////////////////

  chart_parser::chart_parser(const wstring &gramFile): gram(gramFile) {}


  ////////////////////////////////////////////////////////////////
  /// query parser about start symbol of current grammar
  ////////////////////////////////////////////////////////////////

  wstring chart_parser::get_start_symbol(void) const {
    return gram.get_start_symbol();
  }


  ////////////////////////////////////////////////////////////////
  /// analyze sentence
  ////////////////////////////////////////////////////////////////

  void chart_parser::analyze(sentence &s) const {
    TRACE(2,L"CHUNKER ");
    // parse each of k-best tag sequences
    for (unsigned int k=0; k<s.num_kbest(); k++) {
      // create chart for this sentence, using given grammar
      chart ch(gram);
   
      TRACE(3,L"LOOP ");
      ch.load_sentence(s,k);
      TRACE(3,L" Sentence loaded.");
      ch.parse();
      TRACE(3,L" Sentence parsed.");
    
      // navigate through the chart and obtain a parse tree for the sentence
      parse_tree tr=ch.get_tree(ch.get_size()-1,0);

      // associate leaf nodes in the tree with the right word in the sentence:
      sentence::iterator w=s.begin();
      for (parse_tree::preorder_iterator n=tr.begin(); n!=tr.end(); ++n) {
        TRACE(4,L" Completing tree: "+n->get_label()+L" children:"+util::int2wstring(n.num_children()));
        if (n.num_children()==0) {
          n->set_word(*w);
          n->set_label(w->get_tag(k));
          w++;
        }
      }

      // assign an id to each node and build an index to access them by id
      TRACE(3,L" Building node index.");
      tr.build_node_index(s.get_sentence_id());
      // include the tree in the sentence object
      TRACE(3,L" Setting sentence parse tree");
      s.set_parse_tree(tr,k);
      TRACE(3,L" Parse tree set");
    }
    TRACE(2,L"CHUNKER DONE");
  }

} // namespace
