#! /bin/sh

## Fusion of given sources to create a single dictionary
## with one line per word form
##
##  Usage:
##         bin/build-dict [source-files]
##
##  Examples:
##         bin/build-dict es/*.txt  >dicc.src
##         bin/build-dict nouns.dat verbs.dat  >dicc.src 

# Find out script location
path=$(dirname $0)

header=$1
shift
entries=$@

# fuse sources, sort, remove duplicates, and join entries for the same form
export LC_ALL="C"
cat $entries | awk 'NF>0' | sort | uniq | awk -f $path/unique-key.awk | cat $header - 
echo "</Entries>"
