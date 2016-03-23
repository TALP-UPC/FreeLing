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


#include "freeling/morfo/summarizer.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

using namespace std;


namespace freeling {

#define MOD_TRACECODE SUMMARIZER_TRACE
#define MOD_TRACENAME L"SUMMARIZER"

  ////////////////////////////////////////////////////////////////
  /// Constructor
  ////////////////////////////////////////////////////////////////

  summarizer::summarizer(const wstring &datFile) {

    // Default configuration
    hypernymy_depth = 2;
    alpha = 0.9;
    remove_used_lexical_chains = FALSE;
    only_strong = FALSE;
    this->semdb_path = L"";
    heuristic = FIRST_WORD;
    relation::max_distance = 50;

    wstring path=datFile.substr(0,datFile.find_last_of(L"/\\")+1);

    config_file cfg;
    enum sections {GENERAL, RELATIONS};
    cfg.add_section(L"General", GENERAL);
    cfg.add_section(L"Relations", RELATIONS);
    if (not cfg.open(datFile))
      ERROR_CRASH(L"Error opening file " + datFile);

    wstring line;
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) {

      case GENERAL: {
        wistringstream sin;
        sin.str(line);
        wstring key;
        sin >> key;
        if (key == L"RemoveUsedChains") {
          wstring value;
          sin >> value;
          remove_used_lexical_chains = (value == L"true" or value[0] == L'y'
                                        or value[0] == L'Y' or value == L"1");
        } 
        else if (key == L"OnlyStrong") {
          wstring value;
          sin >> value;
          only_strong = (value == L"true" or value[0] == L'y'
                         or value[0] == L'Y' or value == L"1");
        } 
        else if (key == L"SemDBPath") {
          wstring value;
          sin >> value;
          semdb_path = util::expand_filename(value);
          if (not util::is_absolute(semdb_path))
            semdb_path = util::absolute(value,path);
        } 

        else if (key == L"Heuristic") {
          wstring value;
          sin >> value;
          if (value == L"FirstMostWeightedWord") heuristic = FIRST_MOST_WEIGHT;
          else if (value == L"SumOfChainWeights") heuristic = WEIGHT_SUM;
          else if (value == L"FirstWord") heuristic = FIRST_WORD;
          else WARNING(L"Ignoring invalid value Heuristic="+value+L" in file '"+datFile+L"'. Using default.");
        }

        else if (key == L"MaxDistanceBetweenWords") 
          sin >> relation::max_distance;

        break;
      }

      case RELATIONS: {
        wistringstream sin;
        sin.str(line);
        wstring elem, expr;
        sin >> elem >> expr;
        relation *rel = NULL;
 
       if (elem == L"SameWord") 
          rel = new same_word(expr);

        else if (elem == L"Hypernymy") {
          if (semdb_path.empty()) 
            ERROR_CRASH(L"Hypernymy relation used, but SemDBPath is not defined.\nPlease define SemDBPath in <General> section, placed before <Relations> section.");
          sin >> hypernymy_depth >> alpha;
          rel = new hypernymy(hypernymy_depth, alpha, semdb_path, expr);
        }

        else if (elem == L"SameCoreferenceGroup") 
          rel = new same_coref_group(expr);

        else {
          WARNING(L"Ignoring invalid line '"+line+L"' in file '"+datFile+L"'.");
          continue;
        }

        used_relations.insert(rel);
        break;
      }

      default: break;
      }
    }
    cfg.close();

    // if no relations defined, use default
    if (used_relations.size() == 0) used_relations.insert(new same_word(L"^(NP|VB|NN)"));

    TRACE(1, L"Module sucessfully loaded");
  }

  ////////////////////////////////////////////////////////////////
  /// Destructor
  ////////////////////////////////////////////////////////////////

  summarizer::~summarizer() {
    for (set<relation*>::iterator r=used_relations.begin(); r!=used_relations.end(); r++)
      delete (*r);
  }

  ////////////////////////////////////////////////////////////////
  /// Computes and returns the average scores of the lexical chains.
  ////////////////////////////////////////////////////////////////

  double summarizer::average_scores(map<wstring, list<lexical_chain> > &chains) const {
    double sum = 0;
    int size = 0;
    for (set<relation*>::const_iterator r=used_relations.begin(); r!=used_relations.end(); r++) {
      list<lexical_chain> &lexical_chains = chains[(*r)->label];
      size += lexical_chains.size();
      for (list<lexical_chain>::iterator lc=lexical_chains.begin(); lc!=lexical_chains.end(); lc++) 
        sum += lc->get_score();
    }
    return sum/size;
  }

  ////////////////////////////////////////////////////////////////
  /// Computes and returns the standard deviation of the lexical chains scores.
  ////////////////////////////////////////////////////////////////

  double summarizer::standard_deviation_scores(map<wstring, list<lexical_chain> > &chains,
                                               const double avg) const {
    double sd = 0;
    int size = 0;
    for (set<relation*>::const_iterator r=used_relations.begin(); r!=used_relations.end(); r++) {
      list<lexical_chain> &lexical_chains = chains[(*r)->label];
      size += lexical_chains.size();
      for (list<lexical_chain>::iterator lc=lexical_chains.begin(); lc!=lexical_chains.end(); lc++) 
        sd += pow(lc->get_score() - avg, 2);
    }
    return sqrt(sd/size);
  }

  ////////////////////////////////////////////////////////////////
  /// Concatenate all the lists in the map chains_type into a single list.
  ////////////////////////////////////////////////////////////////

  list<lexical_chain> summarizer::map_to_lists(map<wstring, list<lexical_chain> > &chains) const {
    list<lexical_chain> spliced_lists;
    for (set<relation*>::const_iterator r=used_relations.begin(); r!=used_relations.end(); r++) 
      spliced_lists.splice(spliced_lists.end(), chains[(*r)->label]);

    return spliced_lists;
  }

  ////////////////////////////////////////////////////////////////
  ///  Auxiliar to sort lexical chains by score 
  ////////////////////////////////////////////////////////////////

  bool compare_lexical_chains (lexical_chain &first, lexical_chain &second)  {
    return (first.get_score() >= second.get_score());
  }

  ////////////////////////////////////////////////////////////////
  /// Auxiliar function for first_word and first_most_weighted_word function.
  /// Computes whether a sentence should be included in the summary
  ////////////////////////////////////////////////////////////////

  void summarizer::compute_sentence(const list<word_pos> &wps, list<word_pos> &wp_list,
                                    set<const sentence*> &sent_set, int &acc_n_words,
                                    int num_words) const {
    bool brk = false;
    for (list<word_pos>::const_iterator it_wp = wps.begin(); it_wp != wps.end() and
           acc_n_words < num_words and not brk; it_wp++) {
      const word_pos wp = *it_wp;
      const sentence & s = wp.s;
      if (sent_set.find(&s) == sent_set.end()) {
        // Counting the number of words (here we exclude commas,
        // points, exclamation symbols, etc...)
        int s_size = 0;
        for (sentence::const_iterator w=s.begin(); w!=s.end(); w++) {
          if (w->get_tag()[0] != L'F') 
            s_size++;
        }

        if (acc_n_words <= num_words) {
          sent_set.insert(&s);
          acc_n_words += s_size;
          wp_list.push_back(wp);
          if (remove_used_lexical_chains) brk = true;
        }
      }
    }
  }

  ////////////////////////////////////////////////////////////////
  /// Returns a list of word_pos containing the sentences
  /// which form the summary using the heuristic FirstWord.  
  ////////////////////////////////////////////////////////////////

  list<word_pos> summarizer::first_word(map<wstring, list<lexical_chain> > &chains, int num_words) const {
    list<lexical_chain> lexical_chains = map_to_lists(chains);
    lexical_chains.sort(compare_lexical_chains);
    set<const sentence*> sent_set;
    list<word_pos> wp_list;
    int acc_n_words = 0;
    for (list<lexical_chain>::const_iterator it = lexical_chains.begin(); it != lexical_chains.end(); it++) {
      const list<word_pos> &wps = it->get_words();
      compute_sentence(wps, wp_list, sent_set, acc_n_words, num_words);
    }
    return wp_list;
  }

  ////////////////////////////////////////////////////////////////
  /// Returns a list of word_pos containing the sentences
  /// which form the summary using the heuristic FirstWord.  
  ////////////////////////////////////////////////////////////////

  list<word_pos> summarizer::first_most_weighted_word(map<wstring, list<lexical_chain> > &chains, int num_words) const {
    list<lexical_chain> lexical_chains = map_to_lists(chains);
    lexical_chains.sort(compare_lexical_chains);
    set<const sentence*> sent_set;
    list<word_pos> wp_list;
    int acc_n_words = 0;
    for (list<lexical_chain>::const_iterator it = lexical_chains.begin();
         it != lexical_chains.end(); it++) {
      list<word_pos> wps = it->get_ordered_words();

      compute_sentence(wps, wp_list, sent_set, acc_n_words, num_words);
    }
    return wp_list;
  }

  ////////////////////////////////////////////////////////////////
  /// Auxiliary to sort sentences by score and position
  ////////////////////////////////////////////////////////////////

  bool order_by_scores (const pair<double, const word_pos*> &sc_wp1,
                        const pair<double, const word_pos*> &sc_wp2) {
    if (sc_wp1.first == sc_wp2.first) 
      return sc_wp1.second->n_sentence < sc_wp2.second->n_sentence;
    
    return sc_wp1.first > sc_wp2.first;
  }

  ////////////////////////////////////////////////////////////////
  /// Returns a list of word_pos containing the sentences
  /// which form the summary using the heuristic SumOfChainWeights 
  ////////////////////////////////////////////////////////////////

  list<word_pos> summarizer::sum_of_chain_weights(map<wstring, list<lexical_chain> > &chains, int num_words) const {
    list<lexical_chain> lexical_chains = map_to_lists(chains);
    lexical_chains.sort(compare_lexical_chains);

    // The key is the sentence number, the first value of the pair is the score and the second is the
    // first word_pos that reference the sentence of the key
    unordered_map<int, pair<double, const word_pos*> > sentence_scores;

    // Here we score the sentences that have at least one word in a lexical chain
    for (list<lexical_chain>::iterator lc=lexical_chains.begin(); lc!=lexical_chains.end(); lc++) {
      const list<word_pos> &wps = lc->get_words();

      for (list<word_pos>::const_iterator wp=wps.begin(); wp!=wps.end(); wp++) {
        unordered_map<int, pair<double, const word_pos*> >::iterator ss=sentence_scores.find(wp->n_sentence);
        if (ss!=sentence_scores.end()) 
          (ss->second).first += lc->get_score();
        else 
          sentence_scores[wp->n_sentence] = make_pair(lc->get_score(), &(*wp));
      }
    }

    // We insert every pair in a list and then we order by the score
    list<pair<double, const word_pos*> > score_wp_list;
    for (unordered_map<int, pair<double, const word_pos*> >::const_iterator ss=sentence_scores.begin();
         ss!=sentence_scores.end(); ss++) 
      score_wp_list.push_back(ss->second);

    score_wp_list.sort(order_by_scores);

    // Here we select the sentences with best score and insert them in a list
    // (until the desired number of words is reached)
    int acc_n_words = 0;
    list<word_pos> wp_list;
    for (list<pair<double, const word_pos*> >::const_iterator swp=score_wp_list.begin();
         swp!=score_wp_list.end() and acc_n_words < num_words; swp++) {
      int s_size = 0;
      const word_pos *wp = swp->second;
      const sentence &s = (wp->s);

      // Counting the number of words (here we exclude commas, points,
      // exclamation symbols, etc...)
      for (sentence::const_iterator w=s.begin(); w!=s.end(); w++) {
        if (w->get_tag()[0] != L'F') 
          s_size++;
      }

      if (acc_n_words <= num_words) {
        acc_n_words += s_size;
        wp_list.push_back(*wp);
      }
    }
    return wp_list;
  }

  ////////////////////////////////////////////////////////////////
  /// Builds all the lexical chains.
  ////////////////////////////////////////////////////////////////

  map<wstring, list<lexical_chain>> summarizer::build_lexical_chains(const document &doc) const {
    map<wstring, list<lexical_chain>> chains;
    for (set<relation*>::const_iterator it_t = used_relations.begin(); it_t != used_relations.end(); it_t++) {
      relation *rel = *it_t;
      int i = 0;
      int j = 0;
      for (list<paragraph>::const_iterator par=doc.begin(); par!=doc.end(); par++) {
        for (paragraph::const_iterator sent=par->begin(); sent!=par->end(); sent++) {
          int k = 0;
          for (sentence::const_iterator w=sent->begin(); w!=sent->end(); w++) {
            if (rel->is_compatible(*w)) {
              list<lexical_chain> &lc = chains[rel->label];
              bool inserted = false;
              // check addition of the word to every lexical chain
              for (list<lexical_chain>::iterator ch=lc.begin(); ch!=lc.end(); ch++) 
                inserted = inserted or ch->compute_word(*w, *sent, doc, i, j, k);

              // if the word was not inserted in any chain, then we build a new one
              if (not inserted) {
                lexical_chain new_lc(rel, *w, *sent, i, j, k);
                lc.push_back(new_lc);
              }
            }
            k++;
          }
          j++;
        }
        i++;
      }
    }
    return chains;
  }

  ////////////////////////////////////////////////////////////////
  /// Remove lexical chains with only one word
  ////////////////////////////////////////////////////////////////

  void summarizer::remove_one_word_lexical_chains(map<wstring, list<lexical_chain>> &chains) const {
    for (set<relation*>::const_iterator it_t = used_relations.begin(); it_t != used_relations.end(); it_t++) {
      list<lexical_chain> &lc = chains[(*it_t)->label];
      list<lexical_chain>::iterator it = lc.begin();

      while (it != lc.end()) {
        if (it->get_number_of_words() == 1) 
          it = lc.erase(it);
        else 
          it++;
      }
    }
  }
  
  ////////////////////////////////////////////////////////////////
  /// Remove lexical chains which does not satisfy the strength criterion.
  ////////////////////////////////////////////////////////////////

  void summarizer::remove_weak_lexical_chains(map<wstring, list<lexical_chain>> &chains) const {
    double avg = average_scores(chains);
    double sd = standard_deviation_scores(chains, avg);

    for (set<relation*>::const_iterator it_t = used_relations.begin(); it_t != used_relations.end(); it_t++) {
      list<lexical_chain> &lexical_chains = chains[(*it_t)->label];
      list<lexical_chain>::iterator it = lexical_chains.begin();

      while (it != lexical_chains.end()) {
        if (it->get_score() <= (avg + 1.0 * sd)) 
          it = lexical_chains.erase(it);
        else 
          it++;
      }
    }
  }

  ////////////////////////////////////////////////////////////////
  /// Print lexical chains. Only for debugging.
  ////////////////////////////////////////////////////////////////

  void summarizer::print_lexical_chains(map<wstring, list<lexical_chain>> &chains) const {
    for (set<relation*>::const_iterator it_t = used_relations.begin(); it_t != used_relations.end(); it_t++) {
      list<lexical_chain> &lexical_chains = chains[(*it_t)->label];
      for (list<lexical_chain>::iterator it = lexical_chains.begin(); it != lexical_chains.end(); it++)
        {
          wcerr << L"-------------------------------" << endl;
          wcerr << it->toString();
        }
    }
  }


  ////////////////////////////////////////////////////////////////
  /// Summarizes a document and returns the list of sentences that form the summary.
  ////////////////////////////////////////////////////////////////

  list<const sentence*> summarizer::summarize(const document &doc, int num_words) const {

    // building lexical chains
    map<wstring, list<lexical_chain> > chains = build_lexical_chains(doc);

    // remove one word lexical chains
    remove_one_word_lexical_chains(chains);

    // remove weak lexical chains
    if (only_strong)
      remove_weak_lexical_chains(chains);

    // debug
    // print_lexical_chains(chains);

    // select most relevant sentences using active heuristic
    list<word_pos> wp_res;
    switch (heuristic) {
      case FIRST_WORD: wp_res = first_word(chains, num_words); break;
      case FIRST_MOST_WEIGHT: wp_res = first_most_weighted_word(chains, num_words); break;
      case WEIGHT_SUM: wp_res = sum_of_chain_weights(chains, num_words); break;
    }

    // sorts the sentences by position in the original text
    wp_res.sort();

    /// Transform the list of word_pos to a list of sentences.
    list<const sentence*> res;
    for (list<word_pos>::const_iterator it = wp_res.begin(); it != wp_res.end(); it++) 
      res.push_back(&(it->s));

    /// No sentences were selected (probably too short document, only 1-word chains).
    /// Get sentences from the beggining until we have enough words. 
    if (res.empty()) {
      int n=0;
      int done=false;
      for (document::const_iterator p=doc.begin(); p!=doc.end() and not done; p++) {
        for (paragraph::const_iterator s=p->begin(); s!=p->end() and not done; s++) {
          res.push_back(&(*s));
          n += s->size();
          done = (n>=num_words);
        }
      }          
    }

    return (res);
  }


} // namespace
