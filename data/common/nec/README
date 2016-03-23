
 NEC models for each language are found in folders data/XX/nec (where XX
 is the language code)

 Those folders contain two kinds of models. Configuration files for each module are:

 nec-ab-rich.dat 
              Configuration file for a gazetter-relying NEC model.
              This means that the classifier has been trained with a corpus
              in which between 70-80% of the NE occurrences were found in the gazetter 
              (assuming that the higher frequency of a NE in the train set,
              the higher the chance it is included in the gazetteer).
	      Thus, the model learns to rely in the gazetteer information, and
              performance will degrade if the analyzed corpus contains many 
              unknown entities.

              You should use this model if you can extend the gazetters to
              cover most of the NE occurrences in your input text.
              i.e., this model is intended for users that want to adapt
              the classifier to their domain, by enriching the gazetteer.

              The gazetters are stored in the files gazXXX-[cp].dat, 
              where XXX denotes the class of the lemmas contained in the file 
              (PER, LOC, ORG, MISC), and the suffix "-p" or "-c" denotes 
              whether the name is a complete NE ("-c") or a partial NE ("-p").

              To enlarge the gazetters just add lowercased names of complete
              entities to gazXXX-c.dat, and words that are part of multiword
              entities to gazXXX-p.dat.
              E.g. If "Barak Obama" appears in your corpus, you can add 
              "barak_obama" to gazPER-c.dat, and both "barak" and "obama" to
              gazPER-p.dat.  If "Apple" occurs in your corpus, you will add it
              to gazORG-c.dat, but not to gazORG-p.dat

  nec-ab-poor1.dat 
              Configuration file for a non-gazetter-relying NEC model.
              The model only expects to find in the gazetter about 15% of 
              NE occurrences). Thus, the model relies much less on gazetter
              features.

              The advantadge is that this model offers a more stable performance
              when changing domains, with no need to tune the gazetteers. 
              The price to pay for stability, is a loss of a couple of points in
              performance.

              Use this model if you don't want to bother about enriching 
	      gazetteers to match your domain.



