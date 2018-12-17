require './freeling'

Freeling::Util.init_locale('default')

FLDIR = '/usr/local/share/freeling'.freeze
LANG = 'en'.freeze

tokenizer = Freeling::Tokenizer.new("#{FLDIR}/#{LANG}/tokenizer.dat")
splitter = Freeling::Splitter.new("#{FLDIR}/#{LANG}/splitter.dat")
session = splitter.open_session

maco_options = Freeling::Maco_options.new(LANG)
maco_options.set_data_files(
  '',
  "#{FLDIR}/common/punct.dat",
  "#{FLDIR}/#{LANG}/dicc.src",
  "#{FLDIR}/#{LANG}/afixos.dat",
  '',
  "#{FLDIR}/#{LANG}/locucions.dat",
  "#{FLDIR}/#{LANG}/np.dat",
  "#{FLDIR}/#{LANG}/quantities.dat",
  "#{FLDIR}/#{LANG}/probabilitats.dat"
)

morphological_analyzer = Freeling::Maco.new(maco_options)

# activate mmorpho odules to be used in next call
morphological_analyzer.set_active_options(
  false, # umap - User Map
  true,  # num -  Number Detection
  true,  # pun -  Punctuation Detection
  true,  # dat -  Dates Detection
  true,  # dic -  Dictionary Search (also splits words)
  true,  # aff -  Affix (?)
  false, # comp - Compounds (?)
  true,  # rtk -  Retokenization (?)
  true,  # mw -   Multiword Recognition
  true,  # ner -  Named Entity Recognition
  true,  # qt -   Quantity Recognition
  true   # prb -  Probability Assignment and Unknown Word Guesser
)

# create tagger and sense anotator,
tagger = Freeling::Hmm_tagger.new("#{FLDIR}/#{LANG}/tagger.dat", true, 2)
sense_labeler = Freeling::Senses.new("#{FLDIR}/#{LANG}/senses.dat")

while (input = gets)
  input.chomp

  tokens = tokenizer.tokenize(input)
  sentences = splitter.split(session, tokens, true)
  sentences = morphological_analyzer.analyze(sentences)
  sentences = tagger.analyze(sentences)
  sentences = sense_labeler.analyze(sentences)

  sentences.each do |sentence|
    sentence.each do |word|
      puts word.get_form + ' ' + word.get_lemma + ' ' + word.get_tag + ' ' + word.get_senses_string
    end
    puts ' '
  end
end

splitter.close_session(session)
