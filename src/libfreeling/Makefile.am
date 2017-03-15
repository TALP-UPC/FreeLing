
EXTRA_DIST=windll.cc

AUTOMAKE_OPTIONS = subdir-objects

include $(top_srcdir)/src/automake_options.am

AM_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/libfoma -I$(top_srcdir)/src/libtreeler $(KALDI_CPPFLAGS) $(MY_CPPFLAGS)
AM_CXXFLAGS = $(MY_CXXFLAGS)

lib_LTLIBRARIES = libfreeling.la

libfreeling_la_SOURCES = version.cc util.cc regexp.cc traces.cc language.cc configfile.cc analyzer.cc tokenizer.cc splitter.cc processor.cc RE_map.cc dictionary.cc suffixes.cc accents/accents.cc accents/accents_default.cc accents/accents_es.cc accents/accents_gl.cc prefTree.cc database.cc punts.cc automat.cc numbers/numbers.cc numbers/numbers_default.cc numbers/numbers_ca.cc numbers/numbers_cs.cc numbers/numbers_de.cc numbers/numbers_en.cc numbers/numbers_es.cc numbers/numbers_gl.cc numbers/numbers_pt.cc numbers/numbers_ru.cc numbers/numbers_it.cc dates/dates.cc dates/dates_default.cc dates/dates_ca.cc dates/dates_de.cc dates/dates_fr.cc dates/dates_gl.cc dates/dates_pt.cc dates/dates_en.cc dates/dates_es.cc dates/dates_ru.cc locutions.cc ner.cc ner_module.cc np.cc bioner.cc quantities/quantities.cc quantities/quantities_default.cc quantities/quantities_ca.cc quantities/quantities_en.cc quantities/quantities_es.cc quantities/quantities_gl.cc quantities/quantities_pt.cc quantities/quantities_ru.cc probabilities.cc maco.cc maco_options.cc compounds.cc alternatives.cc corrector.cc foma_FSM.cc phonetics.cc tagset.cc tagger.cc hmm_tagger.cc lexer.cc relax_tagger/relax_tagger.cc relax_tagger/relax.cc relax_tagger/constraint_grammar.cc nec.cc senses.cc semdb.cc chart_parser/chart_parser.cc chart_parser/chart.cc chart_parser/grammar.cc dependency_parsing/dep_rules.cc dependency_parsing/dep_txala.cc dependency_parsing/dep_treeler.cc ukb.cc csr_kb.cc embeddings.cc lang_ident/idioma.cc lang_ident/lang_ident.cc fex/fex_rule.cc fex/fex_lexicon.cc fex/fex.cc fex/nerc_features.cc omlet/classifier.cc omlet/adaboost.cc omlet/dataset.cc omlet/example.cc omlet/weakrule.cc omlet/viterbi.cc omlet/svm.cc omlet/libsvm.cc coref/mention_detector.cc coref/relaxcor/relaxcor_model.cc coref/relaxcor/relaxcor_modelDT.cc coref/relaxcor/relaxcor_fex.cc coref/relaxcor/relaxcor.cc output/output.cc output/io_handler.cc output/output_handler.cc output/output_freeling.cc output/output_train.cc output/output_conll.cc output/output_xml.cc output/output_naf.cc output/output_json.cc output/input_handler.cc output/input_conll.cc output/input_freeling.cc output/conll_handler.cc semgraph/semgraph.cc semgraph/ent_extract.cc semgraph/rel_extract.cc semgraph/rel_extract_SPR.cc semgraph/rel_extract_SRL.cc semgraph/semgraph_extract.cc summarizer/lexical_chain.cc summarizer/relation.cc summarizer/summarizer.cc

libfreeling_la_LIBADD = $(FREELING_DEPS) -lz -ltreeler -lfoma

libfreeling_la_LDFLAGS = -release $(PACKAGE_VERSION) -L$(top_srcdir)/src/libtreeler -L$(top_srcdir)/src/libfoma
