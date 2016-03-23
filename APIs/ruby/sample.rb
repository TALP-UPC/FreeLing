
require './freeling'

 Freeling::Util.init_locale("default")

 FLDIR="/home/padro/Software/share/freeling"
 LANG="es"

 tk = Freeling::Tokenizer.new(FLDIR+"/"+LANG+"/tokenizer.dat")
 sp = Freeling::Splitter.new(FLDIR+"/"+LANG+"/splitter.dat")
 sid = sp.open_session

 op = Freeling::Maco_options.new(LANG);
 op.set_data_files( "", 
                   FLDIR+"/"+"common/punct.dat",
                   FLDIR+"/"+LANG + "/dicc.src",
                   FLDIR+"/"+LANG + "/afixos.dat",
                   "",
                   FLDIR+"/"+LANG + "/locucions.dat", 
                   FLDIR+"/"+LANG + "/np.dat",
                   FLDIR+"/"+LANG + "/quantities.dat",
                   FLDIR+"/"+LANG + "/probabilitats.dat")

 mf = Freeling::Maco.new(op)

 # activate mmorpho odules to be used in next call
 mf.set_active_options(false, true, true, true,  # select which among created 
                       true, true, false, true,  # submodules are to be used. 
                       true, true, true, true ); # default: all created submodules are used

 # create tagger and sense anotator,
 tg = Freeling::Hmm_tagger.new(FLDIR+"/"+LANG+"/tagger.dat",true,2);
 sen = Freeling::Senses.new(FLDIR+"/"+LANG+"/senses.dat");

 while (line=gets)
   line.chomp
   s = tk.tokenize(line)
   ls = sp.split(sid,s,false)
   ls = mf.analyze(ls)
   ls = tg.analyze(ls)
   ls = sen.analyze(ls)
  
   ls.each { |s| 
      s.each { |w| 
          puts w.get_form + " " + w.get_lemma + " " +  w.get_tag + " " + w.get_senses_string
      }
    puts " "
   }
 end

 sp.close_session(sid)
