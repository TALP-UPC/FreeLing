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

#include "freeling/morfo/relation.h"

#include <iostream>
using namespace std;


namespace freeling {

  ///----------------------------------------------------------------------
  word_pos::word_pos(const word &w_p, const sentence &s_p, int n_paragraph,
                     int n_sentence, int position) : w(w_p), s(s_p) {
    this->n_paragraph = n_paragraph;
    this->n_sentence = n_sentence;
    this->position = position;
    this->ch_score = 0.0;
  }

  bool word_pos::operator==(const word_pos &other) const {
    return (n_paragraph == other.n_paragraph and 
            n_sentence == other.n_sentence and 
            position == other.position);
  }

  bool word_pos::operator<(const word_pos &other) const {
    return (n_paragraph < other.n_paragraph or
            (n_paragraph == other.n_paragraph and n_sentence < other.n_sentence) or
            (n_paragraph == other.n_paragraph and n_sentence == other.n_sentence and position < other.position));
  }

  bool word_pos::operator>(const word_pos &other) const {
    return (n_paragraph > other.n_paragraph or
            (n_paragraph == other.n_paragraph and n_sentence > other.n_sentence) or
            (n_paragraph == other.n_paragraph and n_sentence == other.n_sentence and position > other.position));
  }

  wstring word_pos::toString() const {
    wstring res = w.get_form() + L" [P" + to_wstring(n_paragraph) + L".S" +
      to_wstring(n_sentence) + L".W" + to_wstring(position) + L"]";
    return res;
  }

  ///----------------------------------------------------------------------
  related_words::related_words(const word_pos &w_p1, const word_pos &w_p2,
                               double relatedness) : w1(w_p1), w2(w_p2) {
    this->relatedness = relatedness;
  }

  wstring related_words::toString() const {
    wstring res = to_wstring(relatedness) + L" " + w1.toString() + L" <-> " + w2.toString();
    return res;
  }

  ///----------------------------------------------------------------------
  int relation::max_distance = 0;
  freeling::regexp relation::re_np(L"^NP");
  freeling::regexp relation::re_nn(L"^N[NC]");

  relation::relation(RelType s, const wstring &t, const wstring &e) : label(s), compatible_tag(t), excluded_lemmas(e) { }

  relation::~relation() {}

  bool relation::is_compatible(const word &w) const {
    return compatible_tag.search(w.get_tag()) and not excluded_lemmas.search(w.get_lemma());
  }

  int relation::matching_word(const word &w1, const word &w2) const { return -1; }

  bool relation::compute_word (const word &w, const sentence &s, const document &doc,
                                int n_paragraph, int n_sentence, int position, 
                                list<word_pos> &words,
                                list<related_words> &relations, 
                                unordered_map<wstring,pair<int,word_pos*> > &unique_words) const {
    // if relation is not applicable to this word, forget it.
    if (not is_compatible(w)) return false;

    // if the new word is equal to any word from in the chain 'words'
    // and within distance range, add it to the chain.
    bool inserted = false;
    word_pos *wp = NULL;
    for (list<word_pos>::const_iterator w2 = words.begin(); w2 != words.end(); w2++) {
      if ((n_sentence - w2->n_sentence) <= max_distance) {
        int x = matching_word(w,w2->w);
        if (x >= 0) {
          if (not inserted) {
            wp = new word_pos(w, s, n_paragraph, n_sentence, position);
            inserted = true;
          }
          related_words rel_w(*wp, *w2, x);
          relations.push_back(rel_w);
        }
      }
    }
    
    if (inserted) {
      // add word to chain
      words.push_back(*wp);
      // insert new entry in unique_words
      auto p = unique_words.insert(make_pair(w.get_lc_form(),make_pair(1,wp)));
      // if insert failed, it was already there => increase count
      if (not p.second) ++p.first->second.first;
    }

    return inserted;
}

  ///----------------------------------------------------------------------
  same_word::same_word(const wstring &expr, const wstring &excluded) : relation(relation::SAME_WORD, expr, excluded) {}

  /// return whether w1 can be assigned to the same chain than w2, and their score
  /// -1: do not add.  >0: add
  int same_word::matching_word(const word &w1, const word &w2) const {
    return (w1.get_lc_form() == w2.get_lc_form() ? 1 : -1); 
  }

  double same_word::get_homogeneity_index(const list<word_pos> &words, const list<related_words> &relations,
                                          const unordered_map<wstring, pair<int, word_pos*> > &unique_words) const {
    return (1.0 - 1.0/words.size());
  }


  list<word_pos> same_word::order_words_by_weight(const unordered_map<wstring,pair<int,word_pos*> > &unique_words) const {
    // In the same_word relation, we return word_pos ordered by its apparition order, so
    // we just need to convert it into a list of word_pos
    wcerr<<L"ordenirg words by weight"<<endl;

    list<word_pos> res;
    for (auto w = unique_words.begin(); w != unique_words.end(); ++w) {
      res.push_back(*(w->second.second));
    }

    return res;
  }

  ///----------------------------------------------------------------------
  hypernymy::hypernymy(const wstring &expr, const wstring &excluded, int dp, double alpha, const wstring &semfile) : relation(relation::HYPERNYMY, expr, excluded) {
    semdb = new semanticDB(semfile);
    depth = dp;
    this->alpha = alpha;
  }

  void hypernymy::set_depth(int d) { depth = d; }
  int hypernymy::get_depth() const { return depth; }

  const word_pos &hypernymy::count_relations(int n, const list<related_words> &relations) const {
    unordered_map<wstring, int> wp_count;
    const word_pos * max_wp=NULL;
    int max_freq = 0;
    for (list<related_words>::const_iterator it = relations.begin(); it != relations.end(); it++) {
      wstring key_1 = to_wstring(it->w1.n_paragraph) + L":" + to_wstring(it->w1.n_sentence) +
        L":" + to_wstring(it->w1.position);
      wstring key_2 = to_wstring(it->w2.n_paragraph) + L":" + to_wstring(it->w2.n_sentence) +
        L":" + to_wstring(it->w2.position);
      int freq1;
      int freq2;

      unordered_map<wstring, int>::iterator word_int2 = wp_count.find(key_2);
      if (word_int2 == wp_count.end()) {
        word_int2 = wp_count.insert(pair<wstring, int>(key_2, 1)).first;
        freq2 = 1;
      }
      else {
        word_int2->second++;
        freq2 = word_int2->second;
      }
      unordered_map<wstring, int>::iterator word_int1 = wp_count.find(key_1);
      if (word_int1 == wp_count.end()) {
        word_int1 = wp_count.insert(pair<wstring, int>(key_1, 1)).first;
        freq1 = 1;
      } else {
        word_int1->second++;
        freq1 = word_int1->second;
      }
      if (freq1 > max_freq && freq1 > freq2) {
        max_freq = freq1;
        max_wp = &(it->w1);
      } else if (freq2 > max_freq) {
        max_freq = freq2;
        max_wp = &(it->w2);
      }
    }
    return *max_wp;
  }

  double hypernymy::get_homogeneity_index(const list<word_pos> &words, const list<related_words> &relations,
                                          const unordered_map<wstring, pair<int, word_pos*> > &unique_words) const {
    int n = words.size();

    // wp_core is the word_pos which appears in most relations
    const word_pos &wp_core = count_relations(n, relations); 

    vector<int> num_words_dist(depth + 1, 0);
    // The first position is also initialized to 0 because we want to
    // give the same homogeinity index than the same_word relation if
    // every word is the same.
    for (list<related_words>::const_iterator it = relations.begin(); it != relations.end(); it++) {
      if (it->w1 == wp_core or it->w2 == wp_core) {
        num_words_dist[int(it->relatedness)]++;
      }
    }

    // We apply the homogeinity index formula for hypernymy relation
    double res = 0;
    double alpha_aux = 1.0;
    for (int i = 0; i < depth + 1; i++) {
      res += alpha_aux * ((double)num_words_dist[i] / n);
      alpha_aux *= alpha;
    }

    return res;
  }

  bool order_by_score (const pair<int, word_pos*> &p1, const pair<int, word_pos*> &p2) {
    return (p1.first >= p2.first);
  }

  list<word_pos> hypernymy::order_words_by_weight(const unordered_map<wstring,pair<int,word_pos*> > &unique_words) const {
    list<pair<int, word_pos*> > lst_to_order;
    for (auto uw = unique_words.begin(); uw != unique_words.end(); ++uw) 
      lst_to_order.push_back(uw->second);

    lst_to_order.sort(order_by_score);

    list<word_pos> res;
    for (auto w = lst_to_order.begin(); w != lst_to_order.end(); ++w) 
      res.push_back(*(w->second));

    return res;
  }

  int hypernymy::hypernymyAux(const wstring &s1, const wstring &s2, int k) const {

    if (k > depth) return -1;
    if (s1 == s2) return k;

    sense_info si = semdb->get_sense_info(s1);
    int kret = -1;
    for (list<wstring>::const_iterator p=si.parents.begin(); p!=si.parents.end() and kret==-1; ++p)
      kret = hypernymyAux(*p, s2, ++k);

    return kret;
  }

  /// return whether w1 can be assigned to the same chain than w2, and their score
  /// -1: do not add.  >0: add
  int hypernymy::matching_word(const word &w1, const word &w2) const {    
    const list<pair<wstring, double>> &ss1 = w1.get_senses();
    const list<pair<wstring, double>> &ss2 = w2.get_senses();
    int lvl = -1;
    if (not ss1.empty() and not ss2.empty()) {
      wstring s1 = ss1.begin()->first;
      wstring s2 = ss2.begin()->first;
      lvl = hypernymyAux(s1, s2, 0);
      if (lvl < 0) 
        lvl = hypernymyAux(s2, s1, 0);
    }
    return lvl;
  }


  ///----------------------------------------------------------------------
  same_coref_group::same_coref_group(const wstring &expr, const wstring &excluded) : relation(relation::SAME_COREF_GROUP, expr, excluded) { }

  double same_coref_group::get_homogeneity_index(const list<word_pos> &words, const list<related_words> &relations,
                                                 const unordered_map<wstring, pair<int, word_pos*> > &unique_words) const {
    int nnc = 0;
    int nnp = 0;
    for (unordered_map<wstring, pair<int, word_pos*> >::const_iterator it = unique_words.begin();
         it != unique_words.end(); it++) {
      if (relation::re_np.search((it->second).second->w.get_tag())) 
        nnp++;
      else if (relation::re_nn.search((it->second).second->w.get_tag())) 
        nnc++;
    }
    double numerator = 0;
    if (nnp > 0) numerator = nnp;
    else if (nnc > 0) numerator = nnc;
    else numerator = unique_words.size();
    return (1.0 - numerator/words.size());
  }

  bool order_by_tag_and_score (const pair<int, word_pos*> &p1, const pair<int, word_pos*> &p2) {
    wstring tag1 = p1.second->w.get_tag();
    wstring tag2 = p2.second->w.get_tag();

    bool np1 = relation::re_np.search(tag1);
    bool np2 = relation::re_np.search(tag2);
    if (np1 and np2) return (p1.first >= p2.first);
    else if (np1) return true;
    else if (np2) return false; 

    bool nn1 = relation::re_nn.search(tag1);
    bool nn2 = relation::re_nn.search(tag2);
    if (nn1 and nn2) return (p1.first >= p2.first);
    else if (nn1) return true;
    else if (nn2) return false; 

    return (p1.first >= p2.first);
  }

  list<word_pos> same_coref_group::order_words_by_weight(const unordered_map<wstring,pair<int,word_pos*> > &unique_words) const {
    list<pair<int, word_pos*> > lst_to_order;
    for (unordered_map<wstring, pair<int, word_pos*> >::const_iterator it = unique_words.begin();
         it != unique_words.end(); it++) {
      lst_to_order.push_back(it->second);
    }
    lst_to_order.sort(order_by_tag_and_score);
    list<word_pos> res;
    for (list<pair<int, word_pos*> >::const_iterator it = lst_to_order.begin();
         it != lst_to_order.end(); it++) {
      res.push_back(*it->second);
    }
    return res;
  }

  bool same_coref_group::compute_word (const word &w, const sentence &s, const document &doc,
                                       int n_paragraph, int n_sentence, int position,
                                       list<word_pos> &words,
                                       list<related_words> &relations, 
                                       unordered_map<wstring,pair<int, word_pos*> > &unique_words) const {

    if (words.empty() or not is_compatible(w)) return false;

    const word_pos &wp2 = *(words.begin());
    const list<int> &grps = doc.get_groups();
    for (list<int>::const_iterator it = grps.begin(); it != grps.end(); it++) {
      list<int> coref_id_mentions = doc.get_coref_id_mentions(*it);
      bool ffound = false;
      bool sfound = false;
      for (list<int>::const_iterator it_cid = coref_id_mentions.begin();
           it_cid != coref_id_mentions.end(); it_cid++) {
        const mention &m = doc.get_mention(*it_cid);
        int s_mention = m.get_n_sentence();
        int pos_mention = m.get_head().get_position();
        if (s_mention == n_sentence && pos_mention == position) ffound = true;
        if (s_mention == wp2.n_sentence && pos_mention == wp2.position) sfound = true;
        if (ffound && sfound) {
          word_pos * wp;
          wp = new word_pos(w, s, n_paragraph, n_sentence, position);
          for (list<word_pos>::const_iterator it_w = words.begin(); it_w != words.end(); it_w++) {
            related_words rel_w(*wp, *it_w, 1);
            relations.push_back(rel_w);
          }
          words.push_back(*wp);
          wstring form = w.get_lc_form();
          unordered_map<wstring, pair<int, word_pos*> >::iterator it_uw = unique_words.find(form);
          if (it_uw == unique_words.end()) unique_words[form] = pair<int, word_pos*>(1, wp);
          else (it_uw->second).first++;
          
          return true;
        }
      }
    }
 
    return false;
  }

} //namespace

