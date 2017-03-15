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

  corrector::corrector(const wstring &cfgFile) {
    
    // default init settings
    search_algorithm  = GENETIC; 
    evaluation_method = PROBABILISTIC;

    // init GENETIC PARAMS
    mutation_rate   = 0.05;
    crossover_rate  = 0.70;
    GEN_POPULATION  = 200U;
    GEN_ITERATIONS  = 500U;

    ChatDistance = 75;
    
    // configuration file settings
    wstring path = cfgFile.substr(0, cfgFile.find_last_of(L"/\\")+1);
    wstring stopWordsFile;
    wstring modelFile; //language model (Word Embeddings) file
    
    enum sections {ALGORITHM, EVALUATION, GENETIC_PARAMS, MODEL_FILE, CHAT_LANG};
    config_file cfg;
    cfg.add_section(L"SearchAlgorithm", ALGORITHM);
    cfg.add_section(L"GeneticParameters", GENETIC_PARAMS);
    cfg.add_section(L"EvaluationMethod", EVALUATION);
    cfg.add_section(L"ModelFile", MODEL_FILE, true);
    cfg.add_section(L"ChatLanguage", CHAT_LANG);
    
    if (not cfg.open(cfgFile))
      ERROR_CRASH(L"Error opening file " + cfgFile);
    
    // read configuration file
    wstring line;
    while (cfg.get_content_line(line)) {
      wistringstream sin;
      sin.str(line);
    
      switch (cfg.get_section()) {

      case ALGORITHM: {
        wstring aux_wstr;
        sin >> aux_wstr;
        if (aux_wstr == L"Exhaustive") search_algorithm = EXHAUSTIVE;
        else if (aux_wstr == L"Genetic") search_algorithm = GENETIC;
        else {
          WARNING(L"Invalid SearchAlgorithm \"" << aux_wstr << L"\" in file "<<cfgFile<<". Using default.");
        }
        break;
      }

      case GENETIC_PARAMS : {
        wstring key, value;
        sin >> key >> value;
        if (key==L"MutationRate") mutation_rate = util::wstring2double(value);
        else if (key==L"CrossoverRate") crossover_rate = util::wstring2double(value);
        else if (key==L"PopulationSize") GEN_POPULATION = util::wstring2int(value);
        else if (key==L"Generations") GEN_ITERATIONS = util::wstring2int(value);
        else {
          WARNING(L"Ignoring unexpected line \"" << line << L"\" in file "<<cfgFile<<". Using default.");
        }
        break;
      }
      
      case EVALUATION: {
        wstring aux_wstr;
        sin >> aux_wstr;
        if (aux_wstr == L"SimilarityNext") evaluation_method = SIMILARITY_NEXT;
        else if (aux_wstr == L"SimilarityAll") evaluation_method = SIMILARITY_ALL;
        else if (aux_wstr == L"ContextAverage") evaluation_method = CONTEXT_AVERAGE;
        else if (aux_wstr == L"Probabilistic") evaluation_method = PROBABILISTIC;
        else if (aux_wstr == L"ProbContext") evaluation_method = PROBABILISTIC_CONTEXT;
        else if (aux_wstr == L"EditDistance") evaluation_method = EDIT_DISTANCE;
        else {
          WARNING(L"Invalid EvaluationMethod \"" << aux_wstr << L"\" in file "<<cfgFile<<". Using default.");
        }
        break;
      }
      
      case MODEL_FILE: {
        sin >> modelFile;
        modelFile = util::absolute(modelFile, path);
        break;
      }

      case CHAT_LANG: {
        wstring key, value;
        sin >> key >> value;
        if (key == L"DistanceValue") 
          ChatDistance = util::wstring2int(value);
        else 
          chat_lang.insert(make_pair(key,value));
        break;
      }

      default: break;
      }
    }
    cfg.close();
    
    // create and read wordVec model
    wordVec = new freeling::embeddings(modelFile);
    
    TRACE(2, L"noisy text normalization module succesfully created");
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Destructor
  /////////////////////////////////////////////////////////////////////////////

  corrector::~corrector() {
    delete wordVec;
  }
  
  
  /////////////////////////////////////////////////////////////////////////////
  /// preprocess text after word embeddings have been created
  /////////////////////////////////////////////////////////////////////////////

  void corrector::preprocess(list<freeling::sentence> &ls) {

    if (chat_lang.empty()) return;
    // if we have a chat abbreviations dicctionary, apply it.
    for (list<freeling::sentence>::iterator s = ls.begin(); s != ls.end(); s++) {
      for (freeling::sentence::iterator w = s->begin(); w != s->end(); w++) {
        if (chat_lang.find(w->get_lc_form()) != chat_lang.end()) { // if form matches
          w->clear_alternatives();
          w->add_alternative(freeling::alternative(chat_lang[w->get_lc_form()], ChatDistance));
        }
      }
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Move data from sentences to alternatives data structure
  /////////////////////////////////////////////////////////////////////////////

  unsigned int corrector::set_alternatives(list<freeling::sentence> &ls, alt_t &alternatives) {

    TRACE(6,L"\n### CORRECTOR STATS");
    
    // get only X best alternatives
    unsigned int alts_limit        = 30;
    int min_edit_distance          = 15;
    
    // number of incorrect words in the sentences
    unsigned int num_incorrect_words = 0;
    
    // set values for "alternatives" in the desired format from original list of sentences
    for (list<freeling::sentence>::iterator s = ls.begin(); s != ls.end(); s++) {
      for (freeling::sentence::iterator w = s->begin(); w != s->end(); w++) {
        // init next word
        pair<wstring, vector<freeling::alternative>> next_word;
        
        // get form
        next_word.first = wstring(w->get_lc_form());
        
        // get valid alternatives
        vector<freeling::alternative> alts;
        if (w->has_alternatives()) {
          if (wordVec->word_in_model(w->get_lc_form())) { // add word if not in normal dictionary but in model vocab
            alts.push_back(freeling::alternative(wstring(w->get_lc_form()), 100));
          }
          for (list<freeling::alternative>::iterator a = w->alternatives_begin(); a != w->alternatives_end(); a++) {
            if (wordVec->word_in_model(a->get_form())) {
              if (alts.size() >= alts_limit) {
                //try to substitute for a worse alternative
                int dl_dist = (int) DL_distance_recursive(w->get_form(), a->get_form());
                int dist    = max(min_edit_distance, min(a->get_distance(), dl_dist));
                
                int min = alts[0].get_distance();
                unsigned int min_ind = 0;
                for (unsigned int i = 1; i < alts.size(); i++) {
                  if (alts[i].get_distance() < min) {
                    min = alts[i].get_distance();
                    min_ind = i;
                  }
                }
                if (min > dist) { //add element in min position
                  alts[min_ind] = freeling::alternative(wstring(a->get_form()), dist);
                }
              } else {
                int dl_dist = (int) DL_distance_recursive(w->get_lc_form(), a->get_form());
                alts.push_back(freeling::alternative(wstring(a->get_form()), max(min_edit_distance, min(a->get_distance(), dl_dist))));
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
        sort(alts.begin(), alts.end(), 
                       [](const freeling::alternative &left,
                          const freeling::alternative &right){
                         return left.get_probability() > right.get_probability();});
        
        if (alts.size() > 0) {
          TRACE(4, L"==> Alternatives (" << w->get_form() << L") (" << alts.size() << L"): ");
          for (unsigned int i = 0; i < alts.size(); i++) {
            TRACE(5, L"     "<<alts[i].get_form() << L" (" << alts[i].get_distance() << L"," << alts[i].get_probability() << L")");
          }
        }
        
        // keep the word if no alternatives found or needed
        if (alts.size() == 0) {
          freeling::alternative aux_alt(wstring(w->get_lc_form()), 1);
          aux_alt.set_probability(1.0f);
          alts.push_back(aux_alt);
        }
        else if (alts.size() > 1) {
          num_incorrect_words += 1;
        }
        
        // add alternatives to next_word
        next_word.second = vector<freeling::alternative>(alts);
        
        // add next_word (with alternatives included) to "alternatives"
        alternatives.push_back(next_word); 
      }
    }
    
    return num_incorrect_words;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Normalize sentence
  /////////////////////////////////////////////////////////////////////////////
  
  void corrector::normalize(list<freeling::sentence> &ls) {
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
      fill_n(best_solutions[i], alternatives.size(), 0);
    
    // set algorithm to EXHAUSTIVE if the number of states to search is low
    search_algorithms selected_algorithm = search_algorithm;
    if (total_states <= 100000) selected_algorithm = EXHAUSTIVE;
        
    switch (selected_algorithm) {
    case EXHAUSTIVE: { //========== EXHAUSTIVE SEARCH ============  
      unsigned int num_states_evaluated = 0;
      float best_results[STORED_SOLUTIONS];
      fill_n(best_results, STORED_SOLUTIONS, -1.0);
      unsigned int current_state[alternatives.size()];
      bool state_found = true;
      fill_n(current_state, alternatives.size(), 0);
      
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
          copy(current_state, current_state + alternatives.size(), best_solutions[index]);
          
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

      TRACE(4, L"Search space size:      " << total_states << L" states");
      TRACE(4, L"States evaluated:       " << num_states_evaluated);      
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
            wstring form = alternatives[index].second[best_solutions[i][index]].get_form();
            
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
          wstring first_word  = alternatives[i].second[current_state[i]].get_form();
          wstring second_word = alternatives[i - 1].second[current_state[i - 1]].get_form();
          embeddings_result += wordVec->cos_similarity(first_word, second_word);
        }
      }
      embeddings_result = (float) embeddings_result / (float) (alternatives.size() - 1);
      break;
    }
    
    case SIMILARITY_ALL: {  
      for (unsigned int i = 0; i < alternatives.size(); i++) {
        // for each word, get similarity value with all other words
        wstring first_word  = alternatives[i].second[current_state[i]].get_form();
        for (unsigned int j = i + 1; j < alternatives.size(); j++) {
          wstring second_word = alternatives[j].second[current_state[j]].get_form();
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
        wstring first_word  = alternatives[i].second[current_state[i]].get_form();
        float first_prob = alternatives[i].second[current_state[i]].get_probability();
        for (unsigned int j = i + 1; j < alternatives.size(); j++) {
          wstring second_word = alternatives[j].second[current_state[j]].get_form();
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
        wstring first_word = alternatives[i].second[current_state[i]].get_form();
        freeling::norm_vector first_vec = wordVec->get_vector(first_word);
        if (first_vec.empty()) 
          embeddings_result += -1;

        else {
          // add vectors in context
          freeling::norm_vector sum_vector(wordVec->get_dimensionality());
          int sum_counter = 0;
          for (unsigned int j = 0; j < alternatives.size(); j++) {
            if (j != i) {
              freeling::norm_vector next_word_vec = wordVec->get_vector(alternatives[j].second[current_state[j]].get_form());
              if (not next_word_vec.empty()) {
                ++sum_counter;
                sum_vector = sum_vector + next_word_vec;
              }
            }
          }          
          sum_vector = sum_vector * (1.0/sum_counter);
          embeddings_result += norm_vector::cos_similarity(first_vec, sum_vector);
        }
      }
      embeddings_result = (float) embeddings_result / (float) alternatives.size();
      break;
    }
    
    case PROBABILISTIC_CONTEXT: {
      for (unsigned int i = 0; i < alternatives.size(); i++) {
        // get vector value of every other word, compare
        wstring first_word = alternatives[i].second[current_state[i]].get_form();
        double edit_probability  = alternatives[i].second[current_state[i]].get_probability();
        freeling::norm_vector first_vec = wordVec->get_vector(first_word);
        if (first_vec.empty()) 
          embeddings_result += -1;

        else {
          // add vectors in context
          freeling::norm_vector sum_vector(wordVec->get_dimensionality());
          int sum_counter = 0;
          for (unsigned int j = 0; j < alternatives.size(); j++) {
            if (j != i) {
              freeling::norm_vector next_word_vec = wordVec->get_vector(alternatives[j].second[current_state[j]].get_form());
              edit_probability *= alternatives[j].second[current_state[j]].get_probability();
              if (not next_word_vec.empty()) {
                ++sum_counter;
                sum_vector = sum_vector + next_word_vec;
              }
            }
          }
          
          embeddings_result += norm_vector::cos_similarity(first_vec, sum_vector) * edit_probability;
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
        unsigned int selection = rand() % alternatives[alt_position].second.size();
        gen_states[current_generation][i].second[j] = selection;
      }
    }
    
    for (unsigned int i = first_half; i < GEN_POPULATION; i++) {
      // for every relevant position, select alternative based on roulette selection
      for (unsigned int j = 0; j < num_incorrect_words; j++) {
        //select individual
        unsigned int selected = alternatives[relevant_positions[j]].second.size() - 1;
        bool cont_selection = true;
        float value = (float)rand() / (float)RAND_MAX;
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
    float value = (float)rand() / (float)RAND_MAX;
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
    gen_states = vector<vector<pair<float, unsigned int*>>>(2);
    for (unsigned int i = 0; i < gen_states.size(); i++) {
      gen_states[i] = vector<pair<float, unsigned int*>>(GEN_POPULATION);
      for (unsigned int j = 0; j < gen_states[i].size(); j++) {
        gen_states[i][j] = make_pair(-1.0, new unsigned int[num_incorrect_words]);
      }
    }
    
    current_generation = 0;
    float best_results[STORED_SOLUTIONS];
    fill_n(best_results, STORED_SOLUTIONS, -1.0);
    unsigned int num_states_evaluated = 0;
    
    unsigned int total_mutations = 0;
    
    // random generator
    srand((unsigned int) time(0));
    
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
    vector<float> probabilities(GEN_POPULATION, 0.0);
    unsigned int evaluation_state[alternatives.size()]; // dummy state for evaluation
    fill_n(evaluation_state, alternatives.size(), 0);
    // calculate constants from genetic params:
    unsigned int mutation_rate100 = (unsigned int) (mutation_rate*100.0);
    
    
    // for every iteration, evaluate and apply operators
    for (unsigned int iter = 0; iter < GEN_ITERATIONS; iter++) {
      
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
          // copy(evaluation_state, evaluation_state + alternatives.size(), best_solutions[index]);
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
        if ((float)rand() / (float)RAND_MAX < crossover_rate) {
          // select second individual for crossover
          unsigned int selected_cross = roulette_selection(sum_weights);
          
          // get crossover point
          unsigned int crossover_point = (rand() % (num_incorrect_words - 1)) + 1;
          
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
        if (((unsigned int) (rand() % 100)) < mutation_rate100) { // generate mutation
          // set random alternative for one position
          unsigned int position = rand() % num_incorrect_words;
          unsigned int alt_position = relevant_positions[position];
          unsigned int random_alt = rand() % alternatives[alt_position].second.size();
          gen_states[current_generation][individual].second[position] = random_alt;
          ++total_mutations;
        }
      }
    }
    
    TRACE(4, L"Total mutations:      " << total_mutations);
    TRACE(4, L"Num states evaluated: " << num_states_evaluated);
    
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
  
  unsigned int corrector::DL_distance_rec(const wstring &str_A, const wstring &str_B, int i, int j) {
    if (min(i, j) == -1) {
      return max(i + 1, j + 1)*INSERTION_COST;
    } else {
      unsigned int r1, r2, r3;
      r1 = DL_distance_rec(str_A, str_B, i - 1, j) + DELETION_COST;
      r2 = DL_distance_rec(str_A, str_B, i, j - 1) + INSERTION_COST;
      r3 = DL_distance_rec(str_A, str_B, i - 1, j - 1);
      
      if (str_A[i] != str_B[j])
        r3 += SUBSTITUTION_COST;
      
      if (i > 0 and j > 0 and str_A[i] == str_B[j - 1] and str_A[i - 1] == str_B[j]) { // transposition
        unsigned int r4 = DL_distance_rec(str_A, str_B, i - 2, j - 2) + TRANSPOSITION_COST;
        return min(min(r1, r2), min(r3, r4));
      } else {
        return min(min(r1, r2), r3);
      }
    }
  }

  unsigned int corrector::DL_distance_recursive(const wstring &str_A, const wstring &str_B) {
    //remove common prefixes/suffixes
    unsigned int min_length = min(str_A.length(), str_B.length());
    unsigned int pre, suf;
    pre = suf = 0;
    while (pre < min_length and str_A[pre] == str_B[pre]) pre++;
    while (suf < (min_length - pre) and str_A[str_A.length() - 1 - suf] == str_B[str_B.length() - 1 - suf]) suf++;
    
    wstring A = str_A.substr(pre, str_A.length() - suf - pre);
    wstring B = str_B.substr(pre, str_B.length() - suf - pre);
    
    //run DL recursively
    return DL_distance_rec(A, B, A.length() - 1, B.length() - 1);
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Damerau-Levenshtein edit distance
  /////////////////////////////////////////////////////////////////////////////
  
  corrector_evaluation_t corrector::evaluate(list<freeling::sentence> &to_correct, list<pair<wstring, wstring>> &corrections) {
    // original sentence
    list<freeling::sentence> original = list<freeling::sentence>(to_correct);
    
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
    list<pair<wstring, wstring>>::iterator corrections_it = corrections.begin();
    list<freeling::sentence>::iterator original_it = original.begin();
    for (list<freeling::sentence>::iterator s = to_correct.begin(); s != to_correct.end(); s++) {
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
            wstring a, b, c;
            a = wstring(w_original->get_form());
            b = wstring(corrections_it->second);
            c = wstring(w->get_form());
            missed_corrections.push_back(make_tuple(a, b, c));
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
  
  void corrector::print_evaluation_results(vector<corrector_evaluation_t> corrections) {  
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
    
    wcout << L"### EVALUATION RESULTS #################################" << endl;
    wcout << L"# Number of sentences evaluated: " << num_corrections << endl;
    wcout << L" " << endl;
    wcout << L"### Absolute results: " << endl;
    wcout << L"# Words corrected:          " << words_corrected << L"/" << words_to_correct << endl;
    wcout << L"# Precision ceiling:        " << words_recalled << L"/" << words_to_correct << endl;
    wcout << L"# Precision:                " << words_well_corrected << L"/" << words_to_correct << endl;
    wcout << L" " << endl;
    wcout << L"### Percentages: " << endl;
    wcout << L"# Words corrected:          " << 100.0*((float) words_corrected/(float) words_to_correct) << L"%" << endl;
    wcout << L"# Words recalled:           " << 100.0*((float) words_recalled/(float) words_to_correct) << L"%" << endl;
    wcout << L"# Precision:                " << 100.0*((float) words_well_corrected/(float) words_to_correct) << L"%" << endl;
    wcout << L"# Precision (over ceiling): " << 100.0*((float) words_well_corrected/(float) words_recalled) << L"%" << endl;
    wcout << L" " << endl;
    wcout << L"### Missed corrections: " << endl;
    int corrections_size = missed_corrections.size();
    for (auto missed : missed_corrections) {
      wcout << L"  " << get<0>(missed) << L" => " << get<1>(missed) << L" (" << get<2>(missed) << L")";
      --corrections_size;
      if (corrections_size > 0) {
        wcout << ", ";
      } else {
        wcout << endl;
      }
    }
    missed_corrections.clear();
    wcout << L"#########################################################" << endl;
  }
} // namespace
