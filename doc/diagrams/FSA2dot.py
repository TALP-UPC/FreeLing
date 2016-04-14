#!/usr/bin/env python
# -*- coding: ISO-8859-1 -*-

# Copyright 2016 Johannes Heinecke <johannes.heinecke@orange.fr>

# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Affero General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Affero General Public License for more details.


# Very simple parsing of the dates_LG.cc and numbers_LG.cc FSA of freeling
# in order to create a dot file to visualize the states and transactions.
# Only works if
#  - #define for states start with "ST_"
#  - #define for tokens start with "TK_"
#  - set<int> for final states is called Final
#  - there is a variable "int initialState" for the initial State

# dot file is written to stdout
# postprocess result with dot -Tpdf -O <filename.dot>



import re
import sys




class CPPTransitions:
    def __init__(self, fn):
        ifp = open(fn)

        self.finals = set()
        self.transitions = {} # state: { token: state}

        self.statenames = {} # ST_A: 1
        self.tokennames = {} # TK_hhmm: 30

        isnotinitialstate = set()
        self.initialstate = "ST_A"
        
        parse_t = re.compile("trans\[([\w_]+)\]\s*\[([\w_]+)\]\s*=\s*([\w_]+)\s*");

        for line in ifp.readlines():
            line = line.strip()
            if line.startswith("//"): continue
            if line.find("Final.insert") >= 0:
                opers = line.split(";")
                for oper in opers:
                    oper = oper.strip()
                    if oper.startswith("Final.insert"):
                        finalstate = oper[13:-1]
                        #print finalstate
                        self.finals.add(finalstate)
            elif line.find("trans[ST_") >= 0:
                opers = line.split(";")
                for oper in opers:
                    oper = oper.strip()
                    if oper.startswith("trans[ST_"):
                        #print "dd", oper
                        mo = parse_t.match(oper)
                        if mo:
                            #print "ee", oper
                            #print mo.group(1),mo.group(2),mo.group(3)
                            if not self.transitions.has_key(mo.group(1)):
                                self.transitions[mo.group(1)] = {}
                            self.transitions[mo.group(1)][mo.group(2)] = mo.group(3)
                            isnotinitialstate.add(mo.group(3))
            elif line.startswith("#define"):
                elems = line.split()
                if elems[1].startswith("ST_"):
                    self.statenames[elems[1]] = elems[2]
                elif elems[1].startswith("TK_"):
                    self.tokennames[elems[1]] = elems[2]
            elif line.startswith("initialState"):
                tmpa = line.split("=")
                tmpb = tmpa[1].split(";")                
                self.initialstate = tmpb[0].strip()
                #print "ee", self.initialstate, line
        #print self.tokennames
        #print self.transitions
        #print isnotinitialstate

        # check whether a state is only initial
        for start,transs in self.transitions.items():
            if not start in isnotinitialstate and start != self.initialstate:
                sys.stderr.write("only initial: %s\n" % start)

        self.out()

    def out(self, out=sys.stdout):
        out.write("digraph rdf {\n")
        #out.write("\trankdir=LR\n")
        out.write("\tnode [margin = 0 shape = circle];\n")
        out.write('\t"%s" [style="filled", fillcolor="green"];\n' % (self.nodename(self.initialstate)))
        #out.write('\t"end" [style="filled", fillcolor="red"];\n\n')
        for start,transs in self.transitions.items():
            for rel,end in transs.items():
                out.write('\t"%s" -> "%s" [label="%s (%s)"];\n' % \
                          (self.nodename(start),
                           self.nodename(end),
                           self.wordtoken(rel), self.tokennames.get(rel)))
        out.write("\n")
        for t in self.finals:
            out.write('\t"%s" [shape="doublecircle"];\n' % self.nodename(t))
        #    out.write('\t"%s (%s)" -> "end";\n' % self.nodename(t))
        out.write("}\n")

    def wordtoken(self, t):
        if t and t.startswith("TK_w_"):
            return '\\"%s\\"' % t[5:]
        return t

    def nodename(self, n):
        return '%s (%s)' % (n, self.statenames.get(n))

if __name__ == "__main__":
    import sys

    if len(sys.argv) > 1:
        dt = CPPTransitions(sys.argv[1])
    else:
        sys.stderr.write("FSA visualisation\nusage: %s dates_LG.cc > dates.dot; dot -Tpdf -O dates.dot\n")


    
