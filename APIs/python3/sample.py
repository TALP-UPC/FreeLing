#! /usr/bin/python3

### REQUIRES python 3 !!!!

## Run:  ./sample.py
## Reads from stdin and writes to stdout
## For example:
##     ./sample.py <test.txt >test_out.txt

import pyfreeling
import sys, os

## ------------  output a parse tree ------------
def printTree(ptree, depth):

    node = ptree.begin();

    print(''.rjust(depth*2),end='');
    info = node.get_info();
    if (info.is_head()): print('+',end='');

    nch = node.num_children();
    if (nch == 0) :
        w = info.get_word();
        print ('({0} {1} {2})'.format(w.get_form(), w.get_lemma(), w.get_tag()),end='');

    else :
        print('{0}_['.format(info.get_label()));

        for i in range(nch) :
            child = node.nth_child_ref(i);
            printTree(child, depth+1);

        print(''.rjust(depth*2),end='');
        print(']',end='');
        
    print('');

## ------------  output a parse tree ------------
def printDepTree(dtree, depth):

    node = dtree.begin()

    print(''.rjust(depth*2),end='');

    info = node.get_info();
    link = info.get_link();
    linfo = link.get_info();
    print ('{0}/{1}/'.format(link.get_info().get_label(), info.get_label()),end='');

    w = node.get_info().get_word();
    print ('({0} {1} {2})'.format(w.get_form(), w.get_lemma(), w.get_tag()),end='');

    nch = node.num_children();
    if (nch > 0) :
        print(' [');

        for i in range(nch) :
            d = node.nth_child_ref(i);
            if (not d.begin().get_info().is_chunk()) :
                printDepTree(d, depth+1);

        ch = {};
        for i in range(nch) :
            d = node.nth_child_ref(i);
            if (d.begin().get_info().is_chunk()) :
                ch[d.begin().get_info().get_chunk_ord()] = d;
 
        for i in sorted(ch.keys()) :
            printDepTree(ch[i], depth + 1);

        print(''.rjust(depth*2),end='');
        print(']',end='');

    print('');



## ----------------------------------------------
## -------------    MAIN PROGRAM  ---------------
## ----------------------------------------------

## Check whether we know where to find FreeLing data files
if "FREELINGDIR" not in os.environ :
   if sys.platform == "win32" or sys.platform == "win64" : os.environ["FREELINGDIR"] = "C:\\Program Files"
   else : os.environ["FREELINGDIR"] = "/usr/local"
   print("FREELINGDIR environment variable not defined, trying ", os.environ["FREELINGDIR"], file=sys.stderr)

if not os.path.exists(os.environ["FREELINGDIR"]+"/share/freeling") :
   print("Folder",os.environ["FREELINGDIR"]+"/share/freeling",
         "not found.\nPlease set FREELINGDIR environment variable to FreeLing installation directory",
         file=sys.stderr)
   sys.exit(1)


# Location of FreeLing configuration files.
DATA = os.environ["FREELINGDIR"]+"/share/freeling/";

# Init locales
pyfreeling.util_init_locale("default");

# create language detector. Used just to show it. Results are printed
# but ignored (after, it is assumed language is LANG)
la=pyfreeling.lang_ident(DATA+"common/lang_ident/ident-few.dat");
LANG = "es"

# create options set for maco analyzer. 
op= pyfreeling.analyzer_config();
op.config_opt_Lang = LANG
op.config_opt_MACO_PunctuationFile = DATA + "common/punct.dat"
op.config_opt_MACO_DictionaryFile = DATA + LANG + "/dicc.src"
op.config_opt_MACO_AffixFile = DATA + LANG + "/afixos.dat" 
op.config_opt_MACO_LocutionsFile = DATA + LANG + "/locucions.dat"
op.config_opt_MACO_NPDataFile = DATA + LANG + "/np.dat"
op.config_opt_MACO_QuantitiesFile = DATA + LANG + "/quantities.dat"
op.config_opt_MACO_ProbabilityFile = DATA + LANG + "/probabilitats.dat"

# chose which modules among those available will be used by default
# (can be changed at each all if needed)
op.invoke_opt_MACO_AffixAnalysis = True
op.invoke_opt_MACO_MultiwordsDetection = True
op.invoke_opt_MACO_NumbersDetection = True 
op.invoke_opt_MACO_PunctuationDetection = True 
op.invoke_opt_MACO_DatesDetection = True
op.invoke_opt_MACO_QuantitiesDetection = True
op.invoke_opt_MACO_DictionarySearch = True
op.invoke_opt_MACO_ProbabilityAssignment = True
op.invoke_opt_MACO_NERecognition = True
op.invoke_opt_MACO_RetokContractions = True


# create analyzers
tk=pyfreeling.tokenizer(DATA+LANG+"/tokenizer.dat");
sp=pyfreeling.splitter(DATA+LANG+"/splitter.dat");
sid=sp.open_session();

mf=pyfreeling.maco(op);

# create tagger, sense anotator, and parsers
tg=pyfreeling.hmm_tagger(DATA+LANG+"/tagger.dat",True,2);
sen=pyfreeling.senses(DATA+LANG+"/senses.dat");
parser= pyfreeling.chart_parser(DATA+LANG+"/chunker/grammar-chunk.dat");
dep=pyfreeling.dep_txala(DATA+LANG+"/dep_txala/dependences.dat", parser.get_start_symbol());

# process input text
lin=sys.stdin.readline();

print ("Text language is: "+la.identify_language(lin)+"\n");

while (lin) :
        
    l = tk.tokenize(lin);
    ls = sp.split(sid,l,False);
    
    ls = mf.analyze(ls);
    ls = tg.analyze(ls);
    ls = sen.analyze(ls);
    ls = parser.analyze(ls);
    ls = dep.analyze(ls);

    ## output results
    for s in ls :
       ws = s.get_words();
       for w in ws :
          print(w.get_form()+" "+w.get_lemma()+" "+w.get_tag()+" "+w.get_senses_string());
       print ("");

       tr = s.get_parse_tree();
       printTree(tr, 0);

       dp = s.get_dep_tree();
       printDepTree(dp, 0)

    lin=sys.stdin.readline();
    
# clean up       
sp.close_session(sid);
    
