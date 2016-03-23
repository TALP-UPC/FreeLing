#!/bin/bash

# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="freeling"

printf " ------ Checking source directory..."
(test -f $srcdir/configure.ac \
    && test -f $srcdir/autogen.sh \
    && test -d $srcdir/src \
    && echo " Ok.") || {
                        echo " ERROR: Directory '"$srcdir"' does not look like the top-level $PKG_NAME directory."
                        exit 1
                       }

echo " ------ Running autoreconf..."
autoreconf --version >&/dev/null || { echo " ERROR: autoreconf not available."; exit 1; }
autoreconf --install
echo " ------ Running ./configure $*"
./configure $*
make

