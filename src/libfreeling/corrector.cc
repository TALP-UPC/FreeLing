#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>

#include "freeling/morfo/configfile.h"
#include "freeling/morfo/corrector.h"

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"CORRECTOR"
#define MOD_TRACECODE CORRECTOR_TRACE

  ///////////////////////////////////////////////////////////////
  /// Create a normalization module, loading
  /// appropriate files.
  ///////////////////////////////////////////////////////////////

  corrector::corrector(const std::wstring &cfgFile) {
    
    // default init settings
    search_algorithm  = GENETIC; 
    evaluation_method = PROBABILISTIC;
    
    // configuration file settings
    wstring path = cfgFile.substr(0, cfgFile.find_last_of(L"/\\")+1);
    wstring stopWordsFile;
    wstring modelFile; //language model (Word Embeddings) file
    
    enum sections {LANG, ALGORITHM, EVALUATION, WINDOW, MODEL_FILE};
    config_file cfg;
    cfg.add_section(L"Lang", LANG);
    cfg.add_section(L"Search Algorithm", ALGORITHM);
    cfg.add_section(L"Evaluation Method", EVALUATION);
    cfg.add_section(L"ModelFile", MODEL_FILE);
    
    if (not cfg.open(cfgFile))
      ERROR_CRASH(L"Error opening file " + cfgFile);
    
    // read configuration file
    wstring line;
    while (cfg.get_content_line(line)) {
      wistringstream sin;
      sin.str(line);
    
      switch (cfg.get_section()) {
      case LANG: {
        sin >> lang;
        break;
      }

      case ALGORITHM: {
        std::wstring aux_wstr;
        sin >> aux_wstr;
        if (aux_wstr == L"Exhaustive") {
          search_algorithm = EXHAUSTIVE;
        } else if (aux_wstr == L"Genetic") {
          search_algorithm = GENETIC;
        } else {
          std::wcout << L"In corrector config file, found \"" << aux_wstr << L"\" but search algorithm " 
            << L"can only be [Exhaustive, Genetic]. Setting search_algorithm = GENETIC." << std::endl;
        }
        
        break;
      }
      
      case EVALUATION: {
        std::wstring aux_wstr;
        sin >> aux_wstr;
        if (aux_wstr == L"SimilarityNext") {
          evaluation_method = SIMILARITY_NEXT;
        } else if (aux_wstr == L"SimilarityAll") {
          evaluation_method = SIMILARITY_ALL;
        } else if (aux_wstr == L"ContextAverage") {
          evaluation_method = CONTEXT_AVERAGE;
        } else if (aux_wstr == L"Probabilistic") {
          evaluation_method = PROBABILISTIC;
        } else if (aux_wstr == L"ProbContext"){
          evaluation_method = PROBABILISTIC_CONTEXT;
        } else if (aux_wstr == L"EditDistance") {
          evaluation_method = EDIT_DISTANCE;
        } else {
          std::wcout << L"In corrector config file, found \"" << aux_wstr 
            << L"\" but evaluation method can only be [SimilarityNext, SimilarityAll, ContextAverage, " 
            << L"Probabilistic, ProbContext, EditDistance]. Setting evaluation_method = PROBABILISTIC." << std::endl;
        }
        break;
      }
      
      case MODEL_FILE: {
        sin >> modelFile;
        modelFile = util::absolute(modelFile, path);
        break;
      }

      default: break;
      }
    }
    cfg.close();
    
    // create and read wordVec model
    wordVec = new freeling::word_vector(modelFile);
    
    TRACE(2, L"noisy text normalization module succesfully created");
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Destructor
  /////////////////////////////////////////////////////////////////////////////
  corrector::~corrector() {
    delete wordVec;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// General functions
  /////////////////////////////////////////////////////////////////////////////
  std::wstring corrector::get_language() {return lang;}
  
  // preprocess text after word embeddings have been created
  void corrector::preprocess(std::list<freeling::sentence> &ls) {
    // chat language abreviations
    std::map<std::wstring, std::wstring> chat_lang;
    if (lang == L"es") {
      chat_lang[L"q"]    = L"que";
      chat_lang[L"xq"]   = L"porque";
      chat_lang[L"tq"]   = L"te_quiero";
      chat_lang[L"tk"]   = L"te_quiero";
      chat_lang[L"tkm"]  = L"te_quiero_mucho";
      chat_lang[L"tqm"]  = L"te_quiero_mucho";
      chat_lang[L"bss"]  = L"besos";
      chat_lang[L"tb"]   = L"también";
      chat_lang[L"bye"]  = L"adiós";
      chat_lang[L"k"]    = L"de_acuerdo";
      chat_lang[L"xo"]   = L"pero";
      chat_lang[L"x"]    = L"por";
      chat_lang[L"bn"]   = L"bien";
      chat_lang[L"xa"]   = L"para";
      chat_lang[L"pq"]   = L"porque";
      chat_lang[L"d"]    = L"de";
      chat_lang[L"dnd"]  = L"donde";
      chat_lang[L"xfa"]  = L"por_favor";
      chat_lang[L"plis"] = L"por_favor";
      chat_lang[L"pls"]  = L"por_favor";
      chat_lang[L"sbs"]  = L"sabes";
      chat_lang[L"esk"]  = L"es_que";
    }
    
    for (list<freeling::sentence>::iterator s = ls.begin(); s != ls.end(); s++) {
      for (freeling::sentence::iterator w = s->begin(); w != s->end(); w++) {
        if (chat_lang.count(w->get_lc_form()) == 1) { // if form matches
          w->clear_alternatives();
          w->add_alternative(freeling::alternative(chat_lang[w->get_lc_form()], 75));
        }
      }
    }
  }

  // Move data from sentences to alternatives data structure
  unsigned int corrector::set_alternatives(std::list<freeling::sentence> &ls, alt_t &alternatives) {
    if (DISPLAY_STATS)
      std::wcout << std::endl << L"### CORRECTOR STATS" << std::endl;
    
    // get only X best alternatives
    unsigned int alts_limit        = 30;
    int min_edit_distance          = 15;
    
    // number of incorrect words in the sentences
    unsigned int num_incorrect_words = 0;
    
    // set values for "alternatives" in the desired format from original list of sentences
    for (list<freeling::sentence>::iterator s = ls.begin(); s != ls.end(); s++) {
      for (freeling::sentence::iterator w = s->begin(); w != s->end(); w++) {
        // init next word
        std::pair<std::wstring, std::vector<freeling::alternative>> next_word;
        
        // get form
        next_word.first = std::wstring(w->get_lc_form());
        
        // get valid alternatives
        std::vector<freeling::alternative> alts;
        if (w->has_alternatives()) {
          if (wordVec->word_in_model(w->get_lc_form())) { // add word if not in normal dictionary but in model vocab
            alts.push_back(freeling::alternative(std::wstring(w->get_lc_form()), 100));
          }
          for (list<freeling::alternative>::iterator a = w->alternatives_begin(); a != w->alternatives_end(); a++) {
            if (wordVec->word_in_model(a->get_form())) {
              if (alts.size() >= alts_limit) {
                //try to substitute for a worse alternative
                int dl_dist = (int) DL_distance_recursive(w->get_form(), a->get_form());
                int dist    = std::max(min_edit_distance, std::min(a->get_distance(), dl_dist));
                
                int min = alts[0].get_distance();
                unsigned int min_ind = 0;
                for (unsigned int i = 1; i < alts.size(); i++) {
                  if (alts[i].get_distance() < min) {
                    min = alts[i].get_distance();
                    min_ind = i;
                  }
                }
                if (min > dist) { //add element in min position
                  alts[min_ind] = freeling::alternative(std::wstring(a->get_form()), dist);
                }
              } else {
                int dl_dist = (int) DL_distance_recursive(w->get_lc_form(), a->get_form());
                alts.push_back(freeling::alternative(std::wstring(a->get_form()), std::max(min_edit_distance, std::min(a->get_distance(), dl_dist))));
              }
            }
          }
        }
        
        // sum alternatives, set probability
        double sum = 0.0;
        for (unsigned int i = 0; i < alts.size(); i++) {
          sum += 100.0/alts[i].get_distance();
        }
        for (unsigned int i = 0; i < alts.size(); i++) {
          double inv_dist = 100.0/alts[i].get_distance();
          alts[i].set_probability(inv_dist/sum);
        }
        
        // sort alternatives by descending probability
        std::sort(alts.begin(), alts.end(), 
                       [](const freeling::alternative &left,
                          const freeling::alternative &right){
                         return left.get_probability() > right.get_probability();});
        
        if (DISPLAY_STATS) {
          if (alts.size() > 0)
            std::wcout << L"Alternatives (" << w->get_form() << L") (" << alts.size() << L"): ";
          for (unsigned int i = 0; i < alts.size(); i++) {
            std::wcout << alts[i].get_form() << L"(" << alts[i].get_distance() << L")(" << alts[i].get_probability() << L")";
            if (i < alts.size() - 1) {
              std::wcout << L", ";
            }
          }
          if (alts.size() > 0)
            std::wcout << std::endl;
        }
        
        // keep the word if no alternatives found or needed
        if (alts.size() == 0) {
          freeling::alternative aux_alt(std::wstring(w->get_lc_form()), 1);
          aux_alt.set_probability(1.0f);
          alts.push_back(aux_alt);
        } else if (alts.size() > 1) {
          num_incorrect_words += 1;
        }
        
        // add alternatives to next_word
        next_word.second = std::vector<freeling::alternative>(alts);
        
        // add next_word (with alternatives included) to "alternatives"
        alternatives.push_back(next_word); 
      }
    }
    
    return num_incorrect_words;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Normalize sentence
  /////////////////////////////////////////////////////////////////////////////
  
  void corrector::normalize(std::list<freeling::sentence> &ls) {
    // create words list
    alt_t alternatives;
    
    // preprocessing (if any needed)
    preprocess(ls);
    
    // set values for "alternatives" in the desired format from original list of sentences
    unsigned int num_incorrect_words = set_alternatives(ls, alternatives); 
    
    double total_states = 1;
    int aux_i = 0;
    for (list<freeling::sentence>::iterator s = ls.begin(); s != ls.end(); s++) {
      for (freeling::sentence::iterator w = s->begin(); w != s->end(); w++) {
        if (alternatives[aux_i].second.size() > 1) {
          total_states *= alternatives[aux_i].second.size();
        } else {
        }
        ++aux_i;
      }
    }
    
    // return if there are no words to correct
    if (num_incorrect_words == 0) return;
    
    // create aux algorithm variables
    unsigned int best_solutions[STORED_SOLUTIONS][alternatives.size()];
    for (unsigned int i = 0; i < STORED_SOLUTIONS; i++)
      std::fill_n(best_solutions[i], alternatives.size(), 0);
    
    // set algorithm to EXHAUSTIVE if the number of states to search is low
    search_algorithms selected_algorithm = search_algorithm;
    if (total_states <= 100000) selected_algorithm = EXHAUSTIVE;
    
    if (DISPLAY_STATS) {
      std::wstring aux_wstr = L"???";
      if (selected_algorithm == EXHAUSTIVE) aux_wstr = L"Exhaustive";
      else if (selected_algorithm == GENETIC) aux_wstr = L"Genetic";
      std::wcout << L"Search algorithm:       " << aux_wstr << std::endl;
      
      aux_wstr = L"???";
      if (evaluation_method == SIMILARITY_NEXT) aux_wstr = L"SimilarityNext";
      else if (evaluation_method == SIMILARITY_ALL) aux_wstr = L"SimilarityAll";
      else if (evaluation_method == CONTEXT_AVERAGE) aux_wstr = L"ContextAverage";
      else if (evaluation_method == PROBABILISTIC) aux_wstr = L"Probabilistic";
      else if (evaluation_method == PROBABILISTIC_CONTEXT) aux_wstr = L"ProbContext";
      else if (evaluation_method == EDIT_DISTANCE) aux_wstr = L"EditDistance";
      std::wcout << L"Evaluation algorithm:   " << aux_wstr << std::endl;
    }
    
    switch (selected_algorithm) {
    case EXHAUSTIVE: { //========== EXHAUSTIVE SEARCH ============  
      unsigned int num_states_evaluated = 0;
      float best_results[STORED_SOLUTIONS];
      std::fill_n(best_results, STORED_SOLUTIONS, -1.0);
      unsigned int current_state[alternatives.size()];
      bool state_found = true;
      std::fill_n(current_state, alternatives.size(), 0);
      
      // exhaustive search, classic binary table generation algorithm
      while (state_found) {
        //evaluate current state
        float result = eval_state(alternatives, current_state);
        num_states_evaluated += 1;
        
        //update path if result is better than previous
        unsigned int index = STORED_SOLUTIONS - 1;
        if (result > best_results[index]) {
          // copy to last position
          best_results[index] = result;
          std::copy(current_state, current_state + alternatives.size(), best_solutions[index]);
          
          // swap
          while (index > 0 && result > best_results[index - 1]) {
            // swap array
            unsigned int aux[alternatives.size()];
            memcpy(aux, best_solutions[index - 1], sizeof(unsigned int)*alternatives.size());
            memcpy(best_solutions[index - 1], best_solutions[index], sizeof(unsigned int)*alternatives.size());
            memcpy(best_solutions[index], aux, sizeof(unsigned int)*alternatives.size());
            
            // swap score
            int aux_value           = best_results[index - 1];
            best_results[index - 1] = best_results[index];
            best_results[index]     = aux_value;
            --index;
          }
        }
        
        //go to next state
        state_found = next_state(alternatives, current_state);
      }
      
      if (DISPLAY_STATS) {
        std::wcout << L"Search space size:      " << total_states << L" states" << std::endl;
        std::wcout << L"States evaluated:       " << num_states_evaluated << std::endl;
      }
      break;
    }
    case GENETIC: { //========== GENETIC ALGORITHM ============
      genetic_algorithm(alternatives, num_incorrect_words, &best_solutions[0][0]);
      break;
    }
    }
    
    // modify sentence object (update kbest paths)
    for (unsigned int i = 0; i < STORED_SOLUTIONS; i++) {
      // for each solution
      // this is the (i + 1)nth best solution
      unsigned int index = 0;
      for (list<freeling::sentence>::iterator s = ls.begin(); s != ls.end(); s++) {
        for (freeling::sentence::iterator w = s->begin(); w != s->end(); w++) {
          // if the word doesn't have an analysis (had to be corrected)
          if (w->has_alternatives()) {
            // search its best (i + 1)nth alternative
            std::wstring form = alternatives[index].second[best_solutions[i][index]].get_form();
            
            bool not_found = true;
            auto alt_it = w->alternatives_begin();
            while (not_found and alt_it != w->alternatives_end()) {
              if (alt_it->get_form() == form){
                not_found = false;
                alt_it->add_selection(i + 1);
              }
              alt_it++;
            }
            if (not_found) {
              freeling::alternative alt(alternatives[index].second[best_solutions[i][index]]);
              alt.add_selection(i + 1);
              w->add_alternative(alt);
            }
          }
          ++index;
        }
      }
    }
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Algorithms and evaluation methods
  /////////////////////////////////////////////////////////////////////////////
  
  // return value between [-1.0, 1.0]
  float corrector::eval_state(const alt_t &alternatives, const unsigned int *current_state) {
    float MIN_DISTANCE       = 15.0;
    float embeddings_result  = 0.0;
    
    // calculate distances
    float distance_result = 1.0; //init to 1 to avoid divs by 0
    unsigned int words_counted = 0;
    for (unsigned int i = 0; i < alternatives.size(); i++) {
      // for each corrected word, add distance to total distance result 
      float dist = (float) alternatives[i].second[current_state[i]].get_distance();
      if (dist > 0.0) {
        alternatives[i].second[current_state[i]].get_distance();
        ++words_counted;
      }
    }
    distance_result = (float) distance_result / (float) words_counted;
    distance_result = MIN_DISTANCE/distance_result;
    if (distance_result > 1.0) distance_result = 1.0;
    if (evaluation_method == EDIT_DISTANCE) return distance_result;
    
    if (alternatives.size() <= 1) return 1.0; // embeddings information doesn't help when we only have 1 word
    
    switch (evaluation_method) {
    case SIMILARITY_NEXT: {
      for (unsigned int i = 0; i < alternatives.size(); i++) {
        // for each word, get similarity value with next word
        if (i > 0) {
          std::wstring first_word  = alternatives[i].second[current_state[i]].get_form();
          std::wstring second_word = alternatives[i - 1].second[current_state[i - 1]].get_form();
          embeddings_result += wordVec->cos_similarity(first_word, second_word);
        }
      }
      embeddings_result = (float) embeddings_result / (float) (alternatives.size() - 1);
      break;
    }
    
    case SIMILARITY_ALL: {  
      for (unsigned int i = 0; i < alternatives.size(); i++) {
        // for each word, get similarity value with all other words
        std::wstring first_word  = alternatives[i].second[current_state[i]].get_form();
        for (unsigned int j = i + 1; j < alternatives.size(); j++) {
          std::wstring second_word = alternatives[j].second[current_state[j]].get_form();
          embeddings_result += wordVec->cos_similarity(first_word, second_word);
        }
      }
      float sum = alternatives.size()*(alternatives.size()/2);
      if (alternatives.size() % 2 != 0) {
        sum += (alternatives.size() + 1)/2.0;
      }
      embeddings_result = (float) embeddings_result / (float) sum;
      break;
    }
    
    case PROBABILISTIC: {
      for (unsigned int i = 0; i < alternatives.size(); i++) {        
        // for each word, get probability (similarity with other words*edit distance probability)
        std::wstring first_word  = alternatives[i].second[current_state[i]].get_form();
        float first_prob = alternatives[i].second[current_state[i]].get_probability();
        for (unsigned int j = i + 1; j < alternatives.size(); j++) {
          std::wstring second_word = alternatives[j].second[current_state[j]].get_form();
          float edit_probability   = alternatives[j].second[current_state[j]].get_probability()*first_prob;
          embeddings_result += ((1.0 + wordVec->cos_similarity(first_word, second_word))/2.0)*edit_probability;
        }
      }
      
      float sum = alternatives.size()*(alternatives.size()/2);
      if (alternatives.size() % 2 != 0) {
        sum += (alternatives.size() + 1)/2.0;
      }
      embeddings_result = (float) embeddings_result / (float) sum;
      return embeddings_result;
      break;
    }
    
    case CONTEXT_AVERAGE: {
      for (unsigned int i = 0; i < alternatives.size(); i++) {
        // get vector value of every other word, compare
        std::wstring first_word = alternatives[i].second[current_state[i]].get_form();
        float* first_vec = wordVec->get_vector(first_word);
        if (first_vec == NULL) {
          embeddings_result += -1;
        } else {
          // add vectors in context
          float* sum_vector = new float[wordVec->get_dimensionality()];
          int sum_counter = 0;
          for (unsigned int j = 0; j < alternatives.size(); j++) {
            if (j != i) {
              float* next_word_vec = wordVec->get_vector(alternatives[j].second[current_state[j]].get_form());
              if (next_word_vec != NULL) {
                ++sum_counter;
                wordVec->add_vector(sum_vector, next_word_vec);
              }
            }
          }
          
          wordVec->div_vector(sum_vector, (float) sum_counter);
          embeddings_result += wordVec->cos_similarity(first_vec, sum_vector);
          delete[] sum_vector;
        }
      }
      embeddings_result = (float) embeddings_result / (float) alternatives.size();
      break;
    }
    
    case PROBABILISTIC_CONTEXT: {
      for (unsigned int i = 0; i < alternatives.size(); i++) {
        // get vector value of every other word, compare
        std::wstring first_word = alternatives[i].second[current_state[i]].get_form();
        double edit_probability  = alternatives[i].second[current_state[i]].get_probability();
        float* first_vec = wordVec->get_vector(first_word);
        if (first_vec == NULL) {
          embeddings_result += -1;
        } else {
          // add vectors in context
          float* sum_vector = new float[wordVec->get_dimensionality()];
          int sum_counter = 0;
          for (unsigned int j = 0; j < alternatives.size(); j++) {
            if (j != i) {
              float* next_word_vec = wordVec->get_vector(alternatives[j].second[current_state[j]].get_form());
              edit_probability *= alternatives[j].second[current_state[j]].get_probability();
              if (next_word_vec != NULL) {
                ++sum_counter;
                wordVec->add_vector(sum_vector, next_word_vec);
              }
            }
          }
          
          embeddings_result += (wordVec->cos_similarity(first_vec, sum_vector))*edit_probability;
          delete[] sum_vector;
        }
      }
      embeddings_result = (float) embeddings_result / (float) alternatives.size();
      return embeddings_result;
      break;
    }
    
    default: break;
    } 
    
    embeddings_result = (embeddings_result + 1.0)/2.0; // normalization to [0, 1]
    embeddings_result = embeddings_result*WORD_EMBEDDINGS_WEIGHT;
    distance_result = distance_result*WORD_DISTANCE_WEIGHT;
    return (embeddings_result + distance_result);
  }
  
  bool corrector::next_state(const alt_t &alternatives, unsigned int *current_state) {
    bool state_found = false;
    unsigned int ind = 0;
    
    while (not state_found and ind < alternatives.size()) {
      if (current_state[ind] < (alternatives[ind].second).size() - 1) {
        current_state[ind] += 1;
        state_found = true;
      } else {
        current_state[ind] = 0;
        ind += 1;
      } 
    }
    
    return state_found;
  }
  
  
  /////////////////////////////////////////////////////////////////////////////
  /// Genetic functions
  /////////////////////////////////////////////////////////////////////////////
  
  void corrector::genetic_initialize_pool(unsigned int *relevant_positions, unsigned int num_incorrect_words, const alt_t &alternatives) {
    unsigned int first_half = (int) GEN_POPULATION/2;
    for (unsigned int i = 0; i < first_half; i++) {
      // for every relevant position, set random alternative
      for (unsigned int j = 0; j < num_incorrect_words; j++) {
        unsigned int alt_position = relevant_positions[j];
        unsigned int selection = std::rand() % alternatives[alt_position].second.size();
        gen_states[current_generation][i].second[j] = selection;
      }
    }
    
    for (unsigned int i = first_half; i < GEN_POPULATION; i++) {
      // for every relevant position, select alternative based on roulette selection
      for (unsigned int j = 0; j < num_incorrect_words; j++) {
        //select individual
        unsigned int selected = alternatives[relevant_positions[j]].second.size() - 1;
        bool cont_selection = true;
        float value = (float)std::rand() / (float)RAND_MAX;
        for (unsigned int k = 0; k < alternatives[relevant_positions[j]].second.size() && cont_selection; k++) {
          value -= alternatives[relevant_positions[j]].second[k].get_probability();
          if (value <= 0) {
            selected = k;
            cont_selection = false;
          }
        }
        gen_states[current_generation][i].second[j] = selected;
      }
    }
  }
  
  unsigned int corrector::roulette_selection(float sum_weights) {
    //select individual
    float value = (float)std::rand() / (float)RAND_MAX;
    value *= sum_weights;
    
    for (unsigned int i = 0; i < GEN_POPULATION; i++) {
      value -= gen_states[current_generation][i].first;
      if (value <= 0) return i;
    }
    
    return GEN_POPULATION - 1; // float precision errors
  }
  
  void corrector::genetic_algorithm(const alt_t &alternatives, unsigned int num_incorrect_words, unsigned int *best_solutions) {
    // genetic algorithm basic variables
    // 2 may be changed to keep track of more generations. 2 is the minimum required, the max would be GEN_ITERATIONS
    gen_states = std::vector<std::vector<std::pair<float, unsigned int*>>>(2);
    for (unsigned int i = 0; i < gen_states.size(); i++) {
      gen_states[i] = std::vector<std::pair<float, unsigned int*>>(GEN_POPULATION);
      for (unsigned int j = 0; j < gen_states[i].size(); j++) {
        gen_states[i][j] = std::make_pair(-1.0, new unsigned int[num_incorrect_words]);
      }
    }
    
    current_generation = 0;
    float best_results[STORED_SOLUTIONS];
    std::fill_n(best_results, STORED_SOLUTIONS, -1.0);
    unsigned int num_states_evaluated = 0;
    
    // GENETIC PARAMS
    float mutation_rate   = 0.05;
    float crossover_rate  = 0.70;
    
    unsigned int total_mutations = 0;
    if (DISPLAY_STATS) {
      std::wcout << std::endl << L"Genetic params: " << std::endl;
      std::wcout << L"crossover rate: " << crossover_rate << std::endl;
      std::wcout << L"mutation rate: " << mutation_rate << std::endl;
      std::wcout << L"population: " << GEN_POPULATION << std::endl;
      std::wcout << L"iterations: " << GEN_ITERATIONS << std::endl;
    }
    
    // random generator
    std::srand((unsigned int) time(0));
    
    // modify only words with alternatives
    unsigned int relevant_positions[num_incorrect_words];
    unsigned int relevant_index = 0;
    for (unsigned int i = 0; i < alternatives.size(); i++) {
      if (alternatives[i].second.size() > 1) {
        relevant_positions[relevant_index] = i;
        relevant_index += 1;
      }
    }
    
    // generate initial pool of solutions
    genetic_initialize_pool(relevant_positions, num_incorrect_words, alternatives);
    std::vector<float> probabilities(GEN_POPULATION, 0.0);
    unsigned int evaluation_state[alternatives.size()]; // dummy state for evaluation
    std::fill_n(evaluation_state, alternatives.size(), 0);
    // calculate constants from genetic params:
    unsigned int mutation_rate100 = (unsigned int) (mutation_rate*100.0);
    
    // genetic debug display
    bool GENETIC_DEBUG_DISPLAY = false;
    
    // for every iteration, evaluate and apply operators
    for (unsigned int iter = 0; iter < GEN_ITERATIONS; iter++) {
      
      if (DISPLAY_STATS and GENETIC_DEBUG_DISPLAY) {
        if (iter < 100 and iter % 20 == 0) {
          std::wcout << std::endl << L"GENETIC ITER " << iter << std::endl;
          for (auto element : gen_states[current_generation]) {
            std::wcout << L"[";
            for (unsigned int i = 0; i < num_incorrect_words - 1; i++) {
              std::wcout << element.second[i] << ", ";
            }
            std::wcout << element.second[num_incorrect_words - 1] << L"]" << std::endl;
          }
        }
      }
      
      // EVALUATE SOLUTIONS AND UPDATE BEST RESULTS
      for (unsigned int individual = 0; individual < GEN_POPULATION; individual++) {
        // copy values to be evaluated
        for (unsigned int i = 0; i < num_incorrect_words; i++)
          evaluation_state[relevant_positions[i]] = gen_states[current_generation][individual].second[i];
        
        // evaluate and store result
        gen_states[current_generation][individual].first = eval_state(alternatives, evaluation_state);
        
        // update best results
        unsigned int index = STORED_SOLUTIONS - 1;
        float result = gen_states[current_generation][individual].first;
        if (result > best_results[index]) {
          // copy to last position
          best_results[index] = result;
          // std::copy(evaluation_state, evaluation_state + alternatives.size(), best_solutions[index]);
          memcpy(&best_solutions[index], evaluation_state, sizeof(unsigned int)*alternatives.size());
          
          // swap
          while (index > 0 && result > best_results[index - 1]) {
            // swap array
            unsigned int aux[alternatives.size()];
            memcpy(aux, &best_solutions[index - 1], sizeof(unsigned int)*alternatives.size());
            memcpy(&best_solutions[index - 1], &best_solutions[index], sizeof(unsigned int)*alternatives.size());
            memcpy(&best_solutions[index], aux, sizeof(unsigned int)*alternatives.size());
            
            // swap score
            int aux_value           = best_results[index - 1];
            best_results[index - 1] = best_results[index];
            best_results [index]    = aux_value;
            --index;
          }
        }
      }
      num_states_evaluated += GEN_POPULATION;
      
      // SELECTION AND CROSSOVER
      // calculate weights for roulette selection
      float sum_weights = 0;
      for (unsigned int i = 0; i < GEN_POPULATION; i++) {
        sum_weights += gen_states[current_generation][i].first;
      }
      
      // calculate next generation
      unsigned int next_generation = (current_generation + 1) % gen_states.size();
      
      for (unsigned int i = 0; i < GEN_POPULATION; i++) {
        // select individual
        unsigned int selected = roulette_selection(sum_weights);
        if ((float)std::rand() / (float)RAND_MAX < crossover_rate) {
          // select second individual for crossover
          unsigned int selected_cross = roulette_selection(sum_weights);
          
          // get crossover point
          unsigned int crossover_point = (std::rand() % (num_incorrect_words - 1)) + 1;
          
          // generate new individual
          memcpy(&gen_states[next_generation][i].second[0], &gen_states[current_generation][selected].second[0], sizeof(unsigned int)*(crossover_point + 1));
          if (crossover_point < num_incorrect_words - 1) {
            unsigned int cp = crossover_point + 1;
            memcpy(&gen_states[next_generation][i].second[cp], &gen_states[current_generation][selected_cross].second[cp], sizeof(unsigned int)*(num_incorrect_words - cp));
          }
        } else {
          // copy individual as it is
          memcpy(&gen_states[next_generation][i].second[0], &gen_states[current_generation][selected].second[0], sizeof(unsigned int)*num_incorrect_words);
        }
      }
      
      // APPLY MUTATION OPERATORS
      current_generation = next_generation;
      for (unsigned int individual = 0; individual < GEN_POPULATION; individual++) {
        if (((unsigned int) (std::rand() % 100)) < mutation_rate100) { // generate mutation
          // set random alternative for one position
          unsigned int position = std::rand() % num_incorrect_words;
          unsigned int alt_position = relevant_positions[position];
          unsigned int random_alt = std::rand() % alternatives[alt_position].second.size();
          gen_states[current_generation][individual].second[position] = random_alt;
          ++total_mutations;
        }
      }
    }
    
    if (DISPLAY_STATS) {
      std::wcout << L"Total mutations:      " << total_mutations << std::endl;
      std::wcout << L"Num states evaluated: " << num_states_evaluated << std::endl;
    }
    
    // free memory
    for (unsigned int individual = 0; individual < GEN_POPULATION; individual++) {
      for (unsigned int generation = 0; generation < gen_states.size(); generation++) {
        if (gen_states[generation][individual].second != NULL)
          delete[] gen_states[generation][individual].second;
      }
    }
  }
  
  
  /////////////////////////////////////////////////////////////////////////////
  /// Damerau-Levenshtein edit distance
  /////////////////////////////////////////////////////////////////////////////
  
  unsigned int corrector::DL_distance_rec(const std::wstring &str_A, const std::wstring &str_B, int i, int j) {
    if (std::min(i, j) == -1) {
      return std::max(i + 1, j + 1)*INSERTION_COST;
    } else {
      unsigned int r1, r2, r3;
      r1 = DL_distance_rec(str_A, str_B, i - 1, j) + DELETION_COST;
      r2 = DL_distance_rec(str_A, str_B, i, j - 1) + INSERTION_COST;
      r3 = DL_distance_rec(str_A, str_B, i - 1, j - 1);
      
      if (str_A[i] != str_B[j])
        r3 += SUBSTITUTION_COST;
      
      if (i > 0 and j > 0 and str_A[i] == str_B[j - 1] and str_A[i - 1] == str_B[j]) { // transposition
        unsigned int r4 = DL_distance_rec(str_A, str_B, i - 2, j - 2) + TRANSPOSITION_COST;
        return std::min(std::min(r1, r2), std::min(r3, r4));
      } else {
        return std::min(std::min(r1, r2), r3);
      }
    }
  }

  unsigned int corrector::DL_distance_recursive(const std::wstring &str_A, const std::wstring &str_B) {
    //remove common prefixes/suffixes
    unsigned int min_length = std::min(str_A.length(), str_B.length());
    unsigned int pre, suf;
    pre = suf = 0;
    while (pre < min_length and str_A[pre] == str_B[pre]) pre++;
    while (suf < (min_length - pre) and str_A[str_A.length() - 1 - suf] == str_B[str_B.length() - 1 - suf]) suf++;
    
    std::wstring A = str_A.substr(pre, str_A.length() - suf - pre);
    std::wstring B = str_B.substr(pre, str_B.length() - suf - pre);
    
    //run DL recursively
    return DL_distance_rec(A, B, A.length() - 1, B.length() - 1);
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Damerau-Levenshtein edit distance
  /////////////////////////////////////////////////////////////////////////////
  
  corrector_evaluation_t corrector::evaluate(std::list<freeling::sentence> &to_correct, std::list<std::pair<std::wstring, std::wstring>> &corrections) {
    // original sentence
    std::list<freeling::sentence> original = std::list<freeling::sentence>(to_correct);
    
    // correct sentence
    normalize(to_correct);
    
    // set best alternatives as new forms for easier comparisons
    for (list<freeling::sentence>::iterator s = to_correct.begin(); s != to_correct.end(); s++) {
      for (freeling::sentence::iterator w = s->begin(); w != s->end(); w++) {
        if (w->has_alternatives()) {
          for (list<alternative>::iterator a = w->alternatives_begin(); a != w->alternatives_end(); a++) {
            if (a->is_selected()) {
              w->set_form(a->get_form());
            }
          }
        }
      }
    }
    
    // struct to store results
    corrector_evaluation_t evaluation;
    evaluation.words_to_correct      = corrections.size();
    evaluation.sentence_length       = 0;
    evaluation.words_corrected       = 0;
    evaluation.words_recalled        = 0;
    evaluation.words_well_corrected  = 0;

    // compare sentences and corrections
    std::list<std::pair<std::wstring, std::wstring>>::iterator corrections_it = corrections.begin();
    std::list<freeling::sentence>::iterator original_it = original.begin();
    for (std::list<freeling::sentence>::iterator s = to_correct.begin(); s != to_correct.end(); s++) {
      freeling::sentence::iterator w_original = original_it->begin();
      for (freeling::sentence::iterator w = s->begin(); w != s->end(); w++) {
        evaluation.sentence_length += 1;
        
        // compare with original sentence
        bool corrected = false;
        if (w_original->get_form() != w->get_form()) {
          evaluation.words_corrected += 1;
          corrected = true;
        }
        
        // check if word had to be corrected
        if (w_original->get_form() == corrections_it->first) {
          
          // check if the word existed in the alternatives
          bool recalled = false;
          for (list<alternative>::iterator a = w->alternatives_begin(); a != w->alternatives_end(); a++) {
            if (a->get_form() == corrections_it->second) {
              recalled = true;
            }
          }
          
          if (recalled) evaluation.words_recalled += 1;
          if (recalled and corrected and w->get_form() == corrections_it->second) {
            evaluation.words_well_corrected += 1;
          } else {
            std::wstring a, b, c;
            a = std::wstring(w_original->get_form());
            b = std::wstring(corrections_it->second);
            c = std::wstring(w->get_form());
            missed_corrections.push_back(std::make_tuple(a, b, c));
          }
          
          if (corrections_it != corrections.end()) ++corrections_it;
        }
        
        // increment iterators
        ++w_original;
      }
      ++original_it;
    }
    
    return evaluation;
  }
  
  void corrector::print_evaluation_results(std::vector<corrector_evaluation_t> corrections) {  
    unsigned int num_corrections  = corrections.size();
    unsigned int words_to_correct = 0;
    unsigned int words_corrected  = 0;
    unsigned int words_recalled   = 0;
    unsigned int words_well_corrected = 0;
    
    for (unsigned int i = 0; i < num_corrections; i++) {
      words_to_correct += corrections[i].words_to_correct;
      words_corrected  += corrections[i].words_corrected;
      words_recalled   += corrections[i].words_recalled;
      words_well_corrected += corrections[i].words_well_corrected;
    }
    
    std::wcout << L"### EVALUATION RESULTS #################################" << std::endl;
    std::wcout << L"# Number of sentences evaluated: " << num_corrections << std::endl;
    std::wcout << L" " << std::endl;
    std::wcout << L"### Absolute results: " << std::endl;
    std::wcout << L"# Words corrected:          " << words_corrected << L"/" << words_to_correct << std::endl;
    std::wcout << L"# Precision ceiling:        " << words_recalled << L"/" << words_to_correct << std::endl;
    std::wcout << L"# Precision:                " << words_well_corrected << L"/" << words_to_correct << std::endl;
    std::wcout << L" " << std::endl;
    std::wcout << L"### Percentages: " << std::endl;
    std::wcout << L"# Words corrected:          " << 100.0*((float) words_corrected/(float) words_to_correct) << L"%" << std::endl;
    std::wcout << L"# Words recalled:           " << 100.0*((float) words_recalled/(float) words_to_correct) << L"%" << std::endl;
    std::wcout << L"# Precision:                " << 100.0*((float) words_well_corrected/(float) words_to_correct) << L"%" << std::endl;
    std::wcout << L"# Precision (over ceiling): " << 100.0*((float) words_well_corrected/(float) words_recalled) << L"%" << std::endl;
    std::wcout << L" " << std::endl;
    std::wcout << L"### Missed corrections: " << std::endl;
    int corrections_size = missed_corrections.size();
    for (auto missed : missed_corrections) {
      std::wcout << L"  " << std::get<0>(missed) << L" => " << std::get<1>(missed) << L" (" << std::get<2>(missed) << L")";
      --corrections_size;
      if (corrections_size > 0) {
        std::wcout << ", ";
      } else {
        std::wcout << std::endl;
      }
    }
    missed_corrections.clear();
    std::wcout << L"#########################################################" << std::endl;
  }
} // namespace
