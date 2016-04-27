EXTRA_DIST = data doc APIs LICENSES freeling.pc msvc
SUBDIRS = src data

### ACLOCAL_AMFLAGS = -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = freeling.pc

dist-hook:
	rm -rf `find $(distdir) -name .svn`; \
	rm -rf `find $(distdir) -name '#*'`; \
	rm -rf `find $(distdir) -name '*~'`; \
	rm -f $(distdir)/doc/userman/userman.[ablo]?? $(distdir)/doc/userman/userman.toc

install-exec-hook:
	chmod -x $(DESTDIR)/$(bindir)/fl_initialize
