languages = as ca cs cy de en es fr gl hr it nb pt ru sl
variants = es/es-old es/es-ar es/es-cl ca/balear ca/valencia

EXTRA_DIST = $(languages) common config

pkgdata_DATA =

if KALDI
  LD_LIBRARY_PATH := $(KALDI_ROOT)/src/lib
  ASR=yes
else
  ASR=
endif

## configuration files
configdatadir = $(pkgdatadir)/config
configdata_DATA = config/*.cfg

install-data-hook:
	-@ldconfig &> /dev/null || echo "Could not run 'ldconfig'. If you get installation errors below, execute 'ldconfig' as root and try 'make install' again.";
	@echo "Indexing language dictionaries. This will take some minutes.";
	@for lg in $(languages); do \
	   echo "Installing files for language '"$$lg"'. Please wait."; \
	   pushd $(top_builddir)/data/$$lg >/dev/null; \
           find . -name '.svn' -prune -o -name '*~' -prune -o -name 'dictionary' -prune -o -type d -exec install -d {} $(DESTDIR)/$(pkgdatadir)/$$lg/{} \; ; \
	   find . -name '.svn' -prune -o -name '*~' -prune -o -name 'dictionary' -prune -o -type f -exec install {} $(DESTDIR)/$(pkgdatadir)/$$lg/{} \; ; \
	   popd >/dev/null; \
	   if (test -d "$(top_builddir)/data/$$lg/dictionary/entries" ); then \
	     echo "  - Creating "$$lg" dictionary..."; \
	     $(top_builddir)/src/utilities/dicc-management/bin/build-dict.sh $(top_builddir)/data/$$lg/dictionary/header $(top_builddir)/data/$$lg/dictionary/entries/* > $(DESTDIR)/$(pkgdatadir)/$$lg/dicc.src; \
	     for sublg in $(variants); do \
	        if (echo $$sublg | grep -q $$lg/); then \
		   echo "  - Creating "$$sublg" variant dictionary..."; \
		   mkdir -p $(DESTDIR)/$(pkgdatadir)/$$sublg; \
		   cat $(top_builddir)/data/$$sublg/dictionary/remove/* >$(DESTDIR)/$(pkgdatadir)/$$sublg/remove.tmp 2>/dev/null; \
		   cat $(top_builddir)/data/$$lg/dictionary/entries/* | grep -v -F -x -f $(DESTDIR)/$(pkgdatadir)/$$sublg/remove.tmp >$(DESTDIR)/$(pkgdatadir)/$$sublg/base.tmp; \
		   $(top_builddir)/src/utilities/dicc-management/bin/build-dict.sh $(top_builddir)/data/$$sublg/dictionary/header $(DESTDIR)/$(pkgdatadir)/$$sublg/base.tmp $(top_builddir)/data/$$sublg/dictionary/entries/* > $(DESTDIR)/$(pkgdatadir)/$$sublg/dicc.src; \
		   rm -f $(DESTDIR)/$(pkgdatadir)/$$sublg/remove.tmp $(DESTDIR)/$(pkgdatadir)/$$sublg/base.tmp; \
		   if (test -f "$(top_builddir)/data/$$sublg/locucions.dat"); then \
		      cat $(top_builddir)/data/$$lg/locucions.dat | tail -n +5 > $(DESTDIR)/$(pkgdatadir)/$$sublg/mws.tmp; \
		      cat $(top_builddir)/data/$$sublg/locucions.dat | grep -v '</Multiwords>' | cat - $(DESTDIR)/$(pkgdatadir)/$$sublg/mws.tmp > $(DESTDIR)/$(pkgdatadir)/$$sublg/locucions.dat; \
		      rm -f $(DESTDIR)/$(pkgdatadir)/$$sublg/mws.tmp; \
		   fi; \
	        fi; \
	     done; \
	   fi; \
	   if (test -f "$(DESTDIR)/$(pkgdatadir)/$$lg/alternatives-phon.dat" ); then \
	      echo "  - Creating "$$lg" phonetic dictionary..."; \
              phRul=`cat $(DESTDIR)/$(pkgdatadir)/$$lg/alternatives-phon.dat | grep PhoneticRules | cut -d' ' -f2`; \
              phDic=`cat $(DESTDIR)/$(pkgdatadir)/$$lg/alternatives-phon.dat | grep PhoneticDictionary | cut -d' ' -f2`; \
              echo $LD_LIBRAY_PATH \
	      cat $(DESTDIR)/$(pkgdatadir)/$$lg/dicc.src | LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(DESTDIR)/$(libdir) $(DESTDIR)/$(bindir)/dicc2phon $(DESTDIR)/$(pkgdatadir)/$$lg/$$phRul > $(DESTDIR)/$(pkgdatadir)/$$lg/$$phDic; \
           fi; \
	   nfil=`ls -1 $(DESTDIR)/$(pkgdatadir)/$$lg/dep_treeler/srl/parameters.*.gz.aa 2>/dev/null | wc -l`; \
	   if (test $$nfil -gt 0 ); then \
	     pfname=`basename $(DESTDIR)/$(pkgdatadir)/$$lg/dep_treeler/srl/parameters.*.gz.aa .aa`; \
	     cat $(DESTDIR)/$(pkgdatadir)/$$lg/dep_treeler/srl/parameters.*.gz.* > $(DESTDIR)/$(pkgdatadir)/$$lg/dep_treeler/srl/$$pfname; \
	     rm -f $(DESTDIR)/$(pkgdatadir)/$$lg/dep_treeler/srl/parameters.*.gz.*; \
	   fi; \
	   nfil=`ls -1 $(DESTDIR)/$(pkgdatadir)/$$lg/dep_treeler/dep/parameters.*.gz.aa 2>/dev/null | wc -l`; \
	   if (test $$nfil -gt 0 ); then \
	     pfname=`basename $(DESTDIR)/$(pkgdatadir)/$$lg/dep_treeler/dep/parameters.*.gz.aa .aa`; \
	     cat $(DESTDIR)/$(pkgdatadir)/$$lg/dep_treeler/dep/parameters.*.gz.* > $(DESTDIR)/$(pkgdatadir)/$$lg/dep_treeler/dep/$$pfname; \
	     rm -f $(DESTDIR)/$(pkgdatadir)/$$lg/dep_treeler/dep/parameters.*.gz.*; \
	   fi; \
	done;
	@for d in common config; do \
	   echo "Installing "$$d" files."; \
	   pushd $(top_builddir)/data/$$d >/dev/null; \
	   find . -name '.svn' -prune -o -name '*~' -prune -o -type d -exec install -d {} $(DESTDIR)/$(pkgdatadir)/$$d/{} \; ; \
	   find . -name '.svn' -prune -o -name '*~' -prune -o -type f -exec install {} $(DESTDIR)/$(pkgdatadir)/$$d/{} \; ; \
	   popd >/dev/null; \
	done;
	@if !(test -z '$(ASR)'); then \
	   echo "Installing asr data. Please wait."; \
	   echo "  - Downloading data from 'http://freeling.com'."; \
	   cp /media/guillem/Dades/TFG/Project/data/freeling_asr_data.tar.gz $(top_builddir)/freeling_asr_data.tar.gz; \
	   echo "  - Untaring the asr data."; \
	   tar -xvzf $(top_builddir)/freeling_asr_data.tar.gz; \
	   rm $(top_builddir)/freeling_asr_data.tar.gz; \
	fi;

uninstall-hook:
	@rm -rf $(DESTDIR)/$(pkgdatadir)
	@if !(test -z '$(ASR)'); then \
	   rm -rf $(top_builddir)/data/config/asr; \
	   rm -rf $(top_builddir)/data/en/asr; \
	   rm -rf $(top_builddir)/data/es/asr; \
	fi;
