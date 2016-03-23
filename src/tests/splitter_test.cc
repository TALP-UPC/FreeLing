#include <boost/test/test_tools.hpp>
#include "freeling/morfo/tokenizer.h"
#include "freeling/morfo/splitter.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/language.h"
#include "common.h"

//#include "C:\Program Files (x86)\Visual Leak Detector\include\vld.h"
#include <list>

using namespace freeling;

void ru_splitter_test()
{
    std::wstring path;
#if defined WIN32 || defined WIN64
	util::init_locale(L"rus");
	path=L"../../../../windows_bin/freeling/data/ru/";
    revert_mode();
#else
	util::init_locale(L"ru_RU.utf8");
	path=L"share/freeling/ru/";
#endif
  tokenizer tk(path+L"tokenizer.dat");
  splitter sp(path+L"splitter.dat");

    typedef struct wTest
    {
        const std::wstring st;
        const size_t sent_num;
        const std::wstring tag;
    } _wTest;
    _wTest tst[] = {
					  {L"Мама мыла раму, а Маша мыла маму. Мама мыла раму, а Маша мыла маму.", 2, L""},
					  {L"Ты! Ты смеешь ТАК говорить про Анечку! Которая рискуя всем работала ради \
                        безопасности своей страны, в то время как гламурные кисы сидели в своих бложиках и потягивали ягу! ГОРДИСЬ АНЕЙ! ГОРДИСЬ АНЕЙ, #УКА!", 5, L""},
                      {L"Джек Потрошитель (англ. Jack The Ripper, маш. Поднимите домкратом прекрасного Человека) — один из самых известных Анонимусов, серийный убийца. \
                        По праву считается самым успешным аноном всех времён — несмотря на явные следы, оставленные им, так и не был деанонимизирован. Любил шлюх, \
                        однако ответной приязни так и не испытал.", 3, L""},
                      {L"Мэри-Энн Николс («Полли»), 42 года. Ночью 31 августа 1888 года, спьяну забив на совет подруги: «Иди, проспись, овца, еле стоишь же», — отправилась \
                        на заработки. Через полтора часа её остывающий труп нашёл рабочий-мимокрокодил. Впотёмках он не разглядел, что Мэри-Энн улыбалась от уха до уха и, \
                        справедливо решив, что она забухала и спит, пошёл за констеблем, дабы он препроводил её в ЛТП. Пришедший констебель принёс «тёмный фонарь», увидел не \
                        алкаша, но труп, и послал за доктором. Вскрытие производилось совершенно через жопу, но из рапортов мы знаем, что на шее Полли было две раны, \
                        нанесённые слева направо, одна из них была глубиной ажно до позвоночника. Помимо всего прочего, живот проститутки был взрезан, но все органы были на месте. \
                        Народ слегка пошумел по этому поводу, но быстро успокоился. Пока ничего из ряда вон выходящего не произошло.", 9, L""},
                      {L"Видимо, в Тайсоне проснулся скрытый хохол[1] — «як не з’їм, то хоч понадкушую», и в третьем раунде он откусил Холифилду кусок правого уха.", 1, L""},
                      {L"Женя Духовникова (aka Духовникова Евгения Викторовна; 29 марта 1986) — никому не известная поэтесса из Иркутска, продемонстрировавшая в борьбе с Википедией \
                        уровень ЧСВ, достойный самой Кати Гордон (даже превосходя её), а уровень неадеквата — сопоставимый с Lingqiyan в треде про официалки.", 1, L""},
                      {L"Постановление Правительства РФ от 24 апреля 2003 г. № 238 \"ОБ ОРГАНИЗАЦИИ НЕЗАВИСИМОЙ ТЕХНИЧЕСКОЙ ЭКСПЕРТИЗЫ ТРАНСПОРТНЫХ СРЕДСТВ\".\
                        Пункт 9. Срок проведения экспертизы устанавливается экспертом-техником (экспертной организацией) по согласованию со страховщиком (потерпевшим) с учетом \
                        требований статей 12 и 13 Федерального закона \"Об обязательном страховании гражданской ответственности владельцев транспортных средств\"." , 3, L""}
    };

    std::list<word> av;
    std::list<sentence> ls;

    splitter::session_id ses = sp.open_session();

    for (size_t i=0; i< sizeof(tst)/sizeof(tst[0]); ++i)
    {
        av.clear();
        ls.clear();
        av = tk.tokenize(tst[i].st);
        ls = sp.split(ses, av, true);
        std::wstringstream sErr;
        sErr << i << L" sentence need to be splitted into: " << tst[i].sent_num << " sentences, get: " << ls.size();
        BOOST_CHECK_MESSAGE(ls.size()==tst[i].sent_num, util::wstring2string(sErr.str()));
    }

    sp.close_session(ses);
}

void en_splitter_test()
{
    std::wstring path;
#if defined WIN32 || defined WIN64
	util::init_locale(L"default");
	path=L"../../../../windows_bin/freeling/data/en/";
    revert_mode();
#else
	util::init_locale(L"default");
	path=L"share/freeling/en/";
#endif
  tokenizer tk(path+L"tokenizer.dat");
  splitter sp(path+L"splitter.dat");

    typedef struct wTest
    {
        const std::wstring st;
        const size_t sent_num;
        const std::wstring tag;
    } _wTest;
    _wTest tst[] = {
					  {L"Anna Vasil’yevna Chapman (Russian: Анна Васильевна Чапман; born Anna Vasil’yevna Kushchyenko Russian: Анна Васильевна Кущенко; 23 February 1982) is a Russian national, \
                        who was residing in New York when she was arrested along with nine others on 27 June 2010, on suspicion of working for the Illegals Program spy ring under the Russian \
                        Federation's external intelligence agency, the SVR (Sluzhba Vneshney Razvedki).[2][5] Chapman pleaded guilty to a charge of conspiracy to act as an agent of a foreign \
                        government without notifying the U.S. Attorney General, and was deported back to Russia on 8 July 2010, as part of a prisoner swap.", 1, L""},
                      {L"Magazines and blogs detailed her fashion style and dress sense, while tabloids displayed her action figure dolls. \
                        Chapman was described by local media in New York as a regular of exclusive bars and restaurants. \
                        US Vice-President Joe Biden, when jokingly asked by Jay Leno on NBC's The Tonight Show with Jay Leno, \"Do we have any spies that hot?\", \
                        replied in a mock serious tone, \"Let me be clear. It was not my idea to send her back.\"", 3, L""},
                      {L"The  name  \"word\"  is  a Perl extension, and \"blank\" is a GNU extension from Perl 5.8. Another Perl extension is negation, which is indicated \
                        by a ^ character after the colon. For example, [12[:^digit:]] matches  \"1\", \"2\", or any non-digit. PCRE (and Perl) also recognize the \
                        POSIX syntax [.ch.] and [=ch=] where \"ch\" is a \"collating element\", but these are not supported, and an error is given if they are encountered.", 4, L""}
    };

    std::list<word> av;
    std::list<sentence> ls;

    splitter::session_id ses = sp.open_session();

    for (size_t i=0; i< sizeof(tst)/sizeof(tst[0]); ++i)
    {
        av.clear();
        ls.clear();
        av = tk.tokenize(tst[i].st);
        ls = sp.split(ses, av, true);
        std::wstringstream sErr;
        sErr << i << L" sentence need to be splitted into: " << tst[i].sent_num << L" sentences, get: " << ls.size();
        BOOST_CHECK_MESSAGE(ls.size()==tst[i].sent_num, util::wstring2string(sErr.str()));
    }

    sp.close_session(ses);
}
