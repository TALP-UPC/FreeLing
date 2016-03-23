
SUBDIRS = . main

AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -I$(top_srcdir)/src/libtreeler
AM_CXXFLAGS = -I$(top_srcdir)/src/libtreeler

#-----------------------------------------------------------------
#--  library libtreeler

lib_LTLIBRARIES = libtreeler.la

libtreeler_la_SOURCES = treeler/base/token.cc treeler/base/fidx.cc treeler/base/dictionary.cc treeler/util/options.cc treeler/util/char-utils.cc treeler/util/timer.cc treeler/io/io-basic.cc treeler/control/factory-base.cc treeler/dep/part-dep1.cc treeler/dep/parser-projdep1.cc treeler/dep/part-dep2.cc treeler/dep/part-dep2-index.cc treeler/dep/parser-projdep2.cc treeler/dep/pos-symbols.cc treeler/dep/dependency_parser.cc treeler/tag/part-tag.cc treeler/tag/tuple-seq.cc treeler/tag/io-tag.cc treeler/class/class-basic.cc treeler/class/io-class.cc treeler/class/fgen-class.cc treeler/class/model-mc.cc treeler/srl/part-srl0.cc treeler/srl/simple-parser.cc treeler/srl/srl_parser.cc  

librarydir=$(includedir)
nobase_library_HEADERS = treeler/base/windll.h treeler/base/treeler.h treeler/base/exception.h treeler/base/dataset.h treeler/base/example.h treeler/base/fidx.h treeler/base/feature-idx-v0.h treeler/base/feature-vector.h treeler/base/label.h treeler/base/parameters.h treeler/base/base-parameters.h treeler/base/sparse-parameters.h treeler/base/scores.h treeler/base/wf-scores.h treeler/base/oracle-scores.h treeler/base/score-dumper.h treeler/base/basic-sentence.h treeler/base/sentence.h treeler/base/token.h treeler/base/dictionary.h treeler/class/class-basic.h treeler/class/fgen-class.h treeler/class/io-class.h treeler/class/model-mc.h treeler/control/control.h treeler/control/trainer.h treeler/control/forall.h treeler/control/factory-base.h treeler/control/factory-scores.h treeler/control/factory-dep.h treeler/control/factory-tag.h treeler/control/factory-ioconll.h treeler/control/models.h treeler/control/model-selector.h treeler/dep/dep.h treeler/dep/dep-symbols.h treeler/dep/pos-symbols.h treeler/dep/dep-tree.h treeler/dep/dep-eval.h treeler/dep/fgen-dep-v1.h treeler/dep/ftemplates-dep1-v1.h treeler/dep/fgen-basic-dep1.h treeler/dep/fgen-basic-dep1.tcc treeler/dep/fgen-basic-dep2.h treeler/dep/fgen-basic-dep2.tcc treeler/dep/fgen-dep-v0.h treeler/dep/fgen-ftemplates-dep1.h treeler/dep/fgen-ftemplates-dep1.tcc treeler/dep/fgen-ftemplates-dep2.h treeler/dep/fgen-ftemplate-types.h treeler/dep/parser-projdep1.h treeler/dep/parser-projdep1.tcc treeler/dep/parser-projdep2.h treeler/dep/parser-projdep2.tcc treeler/dep/part-dep1.h treeler/dep/part-dep2.h treeler/dep/part-dep2-index.h treeler/dep/dependency_parser.h treeler/dep/score-dumper.h treeler/io/io-basic.h treeler/io/conllstream.h treeler/io/io-conll.h treeler/io/io-sentence.h treeler/io/io-dep.h treeler/io/io-fvec.h treeler/algo/decoder.h treeler/algo/learn-utils.h treeler/algo/learn-utils.tcc treeler/algo/perceptron.h treeler/algo/perceptron.tcc treeler/algo/objectives.h treeler/algo/sgd-trainer.h treeler/tag/tag-eval.h treeler/tag/fgen-tag.h treeler/tag/ftemplates-tag-v1.h treeler/tag/fgen-ttag.h treeler/tag/io-tag.h treeler/tag/io-ttag.h treeler/tag/part-tag.h treeler/tag/tag.h treeler/tag/tag-symbols.h treeler/tag/tag-seq.h treeler/tag/tuple-seq.h treeler/tag/viterbi.h treeler/util/char-utils.h treeler/util/math-utils.h treeler/util/options.h treeler/util/simple-hash-table.h treeler/util/simple-hash-table.tcc treeler/util/timer.h treeler/srl/srl.h treeler/srl/sentence.h treeler/srl/paths-container.h treeler/srl/part-srl0.h treeler/srl/simple-parser.h treeler/srl/io.h treeler/srl/fgen-srl-v1.h treeler/srl/fgen-srl-types.h treeler/srl/ftemplates-srl.h treeler/srl/fidx-path.h treeler/srl/srl-eval.h treeler/srl/factory-srl.h treeler/srl/paths-defs.h treeler/srl/srl_parser.h treeler/srl/scorers.h

libtreeler_la_LIBADD = -lz
libtreeler_la_LDFLAGS = -release 0.4


