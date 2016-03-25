languages = as ca cs cy de en es fr gl hr it nb pt ru sl
variants = es/old-es es/es-ar ca/balear ca/valencia 

EXTRA_DIST = $(languages) common config

pkgdata_DATA =

## configuration files
configdatadir = $(pkgdatadir)/config
configdata_DATA = config/*


install-data-hook:
	-@ldconfig &> /dev/null || echo "Could not run 'ldconfig'. If you get installation errors below, execute 'ldconfig' as root and try 'make install' again.";
	@echo "Indexing language dictionaries. This will take some minutes.";
	@for lg in $(languages); do \
	   echo "Installing files for language '"$$lg"'. Please wait."; \
	   pushd $(top_builddir)/data/$$lg >/dev/null; \
	   find . -name '.svn' -prune -o -name '*~' -prune -o -name 'dictionary' -prune -o -type f -exec install -D {} $(DESTDIR)/$(pkgdatadir)/$$lg/{} \; ; \
	   popd >/dev/null; \
	   if (test -d "$(top_builddir)/data/$$lg/dictionary/entries" ); then \
	     echo "  - Creating "$$lg" dictionary..."; \
	     $(top_builddir)/src/utilities/dicc-management/bin/build-dict.sh $(top_builddir)/data/$$lg/dictionary/header $(top_builddir)/data/$$lg/dictionary/entries/* > $(DESTDIR)/$(pkgdatadir)/$$lg/dicc.src; \
	     for sublg in $(variants); do \
	        if (echo $$sublg | grep -q $$lg/); then \
		   echo "  - Creating "$$sublg" variant dictionary..."; \
		   mkdir -p $(DESTDIR)/$(pkgdatadir)/$$sublg; \
		   $(top_builddir)/src/utilities/dicc-management/bin/build-dict.sh $(top_builddir)/data/$$sublg/dictionary/header $(top_builddir)/data/$$lg/dictionary/entries/* $(top_builddir)/data/$$sublg/dictionary/entries/* > $(DESTDIR)/$(pkgdatadir)/$$sublg/dicc.src; \
	        fi; \
	     done; \
	   fi; \
	   if (test -f "$(DESTDIR)/$(pkgdatadir)/$$lg/alternatives-phon.dat" ); then \
	      echo "  - Creating "$$lg" phonetic dictionary..."; \
              phRul=`cat $(DESTDIR)/$(pkgdatadir)/$$lg/alternatives-phon.dat | grep PhoneticRules | cut -d' ' -f2`; \
              phDic=`cat $(DESTDIR)/$(pkgdatadir)/$$lg/alternatives-phon.dat | grep PhoneticDictionary | cut -d' ' -f2`; \
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
	   find . -name '.svn' -prune -o -name '*~' -prune -o -type f -exec install -D {} $(DESTDIR)/$(pkgdatadir)/$$d/{} \; ; \
	   popd >/dev/null; \
	done

uninstall-hook:
	@rm -rf $(DESTDIR)/$(pkgdatadir)
