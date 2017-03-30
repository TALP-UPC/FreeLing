#ifndef _CORRECTOR
#define _CORRECTOR

#include "freeling/morfo/language.h"
#include "freeling/morfo/embeddings.h"

typedef std::vector<std::pair<std::wstring, std::vector<freeling::alternative>>> alt_t;

struct corrector_evaluation_t {
  unsigned int sentence_length;
  unsigned int words_to_correct;
  unsigned int words_corrected;
  unsigned int words_recalled; // (precision ceiling) precision_over_ceiling = words_to_correct/words_recalled
  unsigned int words_well_corrected;
};

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class corrector performs noisy text normalization using
  ///  the analyzer, alternatives modules and word embeddings
  ///  language model.
  ////////////////////////////////////////////////////////////////

  class corrector {
  private: 
    
    // word embeddings
    freeling::embeddings *wordVec;    
    // number of best solutions stored on the alternatives kbest
    static const unsigned int STORED_SOLUTIONS = 10U;
    
    // original incorrect => correct (actual correction)
    std::list<std::tuple<std::wstring, std::wstring, std::wstring>> missed_corrections;

    //-------------------------------
    // Evaluation weights
    //-------------------------------
    static constexpr float WORD_EMBEDDINGS_WEIGHT = 0.3;
    static constexpr float WORD_DISTANCE_WEIGHT   = 0.7;

    // chat language abreviations
    std::map<std::wstring, std::wstring> chat_lang;
    int ChatDistance;

    //-------------------------------
    // Algorithms
    //-------------------------------
    
    // search algorithm being used
    enum search_algorithms {EXHAUSTIVE, GENETIC};
    search_algorithms search_algorithm;
    
    // evaluation function to be used
    enum eval_methods {SIMILARITY_NEXT, SIMILARITY_ALL, CONTEXT_AVERAGE, PROBABILISTIC, PROBABILISTIC_CONTEXT, EDIT_DISTANCE};
    eval_methods evaluation_method;
    
    //-------------------------------
    // Damerau-Levenshtein distance
    //-------------------------------
    static const unsigned int TRANSPOSITION_COST      = 60U;
    static const unsigned int DELETION_COST           = 100U;
    static const unsigned int INSERTION_COST          = 100U;
    static const unsigned int SUBSTITUTION_COST       = 100U;
    static const unsigned int INSERT_REPETITION_COST  = 30U;
    static const unsigned int SUBST_REPETITION_COST   = 100U;
    static const unsigned int VOWEL_DELETION_COST     = 80U;
    static const unsigned int VOWEL_INSERTION_COST    = 80U;
    static const unsigned int VOWEL_SUBSTITUTION_COST = 80U;
    static const unsigned int SEPARATION_COST         = 40U;
    
    unsigned int DL_distance_recursive(const std::wstring &str_A, const std::wstring &str_B);
    unsigned int DL_distance_rec(const std::wstring &str_A, const std::wstring &str_B, int i, int j);
    
    //-------------------------------
    // Genetic algorithm params
    //-------------------------------
    double mutation_rate;
    double crossover_rate;
    unsigned int GEN_POPULATION;
    unsigned int GEN_ITERATIONS;
    std::vector<std::vector<std::pair<float, unsigned int*>>> gen_states;
    unsigned int current_generation;
    
    void genetic_algorithm(const alt_t &alternatives, unsigned int num_incorrect_words, unsigned int *best_solutions);
    void genetic_initialize_pool(unsigned int *relevant_positions, unsigned int num_incorrect_words, const alt_t &alternatives);
    unsigned int roulette_selection(float sum_weights);
    
    //-------------------------------
    // Functions
    //-------------------------------
    
    // returns true if current_state has been modified, false otherwise (next state based on current search_algorithm)
    bool next_state(const alt_t &alternatives, unsigned int *current_state);
    
    // returns current state evaluation value (calculations based on current evaluation_method)
    float eval_state(const alt_t &alternatives, const unsigned int *current_state);
    
    // move data from sentences to alternatives data structure, returns number of incorrect words found
    unsigned int set_alternatives(std::list<freeling::sentence> &ls, alt_t &alternatives);
    
    // preprocess text after word embeddings have been created
    void preprocess(std::list<freeling::sentence> &ls);
    
  public:
    // constructor
    corrector(const std::wstring &cfgFile);
    
    // destructor
    ~corrector();

    // normalize given sentence
    void normalize(std::list<freeling::sentence> &ls);
    
    // evaluate a sentence normalization, given the correct version of the sentence
    corrector_evaluation_t evaluate(std::list<freeling::sentence> &to_correct, std::list<std::pair<std::wstring, std::wstring>> &corrections);
    
    // takes a vector of corrections and prints the results in a readable format
    void print_evaluation_results(std::vector<corrector_evaluation_t> corrections);
    
  };
} // namespace

#endif

