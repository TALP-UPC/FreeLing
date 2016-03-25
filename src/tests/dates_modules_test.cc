

#include <boost/test/test_tools.hpp>
#include "freeling/morfo/tokenizer.h"
#include "freeling/morfo/numbers.h"
#include "freeling/morfo/dates.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/dictionary.h"
#include <iostream>

#include "common.h"

using namespace freeling;

void ru_dates_test()
{
	std::wstring path;
 #if defined WIN32 || defined WIN64
	util::init_locale(L"rus");
	path=L"../../../../windows_bin/freeling/data/ru/";
    revert_mode();
#else
	util::init_locale(L"ru_RU.UTF-8");
	path=L"share/freeling/ru/";
#endif

    numbers num(L"ru", L".", L",");
    dates dat(L"ru");

    tokenizer tk(path+L"tokenizer.dat");
    std::list<word> lw;
    sentence::iterator It;

    typedef struct wTest
    {
        const std::wstring st;
        const std::wstring chk;
        const std::wstring tag;
    } _wTest;
    _wTest tst[] = {
					  {L"12минут" ,L"[??:?\?/?\?/??:??.12:??]", L"W"},
					  {L"12мин."  ,L"[??:?\?/?\?/??:??.12:??]", L"W"},
                      {L"12мин"   ,L"[??:?\?/?\?/??:??.12:??]", L"W"},

                      {L"09ч24мин"  , L"[??:?\?/?\?/??:9.24:??]", L"W"},
                      {L"09ч.24мин.", L"[??:?\?/?\?/??:9.24:??]", L"W"},
                      {L"09:24"     , L"[??:?\?/?\?/??:9.24:??]", L"W"},
                      {L"09:24минут", L"[??:?\?/?\?/??:9.24:??]", L"W"},
                      {L"02:54", L"[??:?\?/?\?/??:2.54:??]", L"W"},
                      
                      {L"23 ч 12 мин.", L"[??:?\?/?\?/??:23.12:??]", L"W"},           //*
                      {L"23 ч. 12 мин.", L"[??:?\?/?\?/??:23.12:??]", L"W"},          //*

                      {L"15 май", L"[??:15/5/??:??.??:??]", L"W"},                    //*
                      {L"15 май 1981 год", L"[??:15/5/1981:??.??:??]", L"W"},         //*

                      {L"15 мая 1981 года", L"[??:15/5/1981:??.??:??]", L"W"},

                      {L"1981 год", L"[??:?\?/?\?/1981:??.??:??]", L"W"},             //*
                      {L"1981 год 6 октября", L"[??:6/10/1981:??.??:??]", L"W"},
                      {L"6 октября 1981 года", L"[??:6/10/1981:??.??:??]", L"W"},
                      {L"1981 год 6 октябрь", L"[??:6/10/1981:??.??:??]", L"W"},      //*

                      {L"15.05.1981", L"[??:15/5/1981:??.??:??]", L"W"},
                      {L"12 часов 32 минуты", L"[??:?\?/?\?/??:12.32:??]", L"W"},
                      {L"12 ч 32 мин", L"[??:?\?/?\?/??:12.32:??]", L"W"},              //*
                      {L"12 часов 32 мин", L"[??:?\?/?\?/??:12.32:??]", L"W"},
                      {L"12 часов", L"[??:?\?/?\?/??:12.??:??]", L"W"},
                      {L"12 ч", L"[??:?\?/?\?/??:12.??:??]", L"W"},                     //*
                      {L"15 мая 1981 года", L"[??:15/5/1981:??.??:??]", L"W"},
                      {L"15 май 1981 год", L"[??:15/5/1981:??.??:??]", L"W"},           //*
                      {L"15 мая 1981 года 12 часов 32 минуты", L"[??:15/5/1981:12.32:??]", L"W"},
                      {L"15 май 1981 год 12 ч 32 мин", L"[??:15/5/1981:12.32:??]", L"W"},   //*
                      {L"15 мая 1981 года в 12 часов 32 минуты", L"[??:15/5/1981:12.32:??]", L"W"},
                      {L"15 май 1981 год в 12 ч 32 мин", L"[??:15/5/1981:12.32:??]", L"W"},  //*
                      {L"в 1981 году 15 мая", L"[??:15/5/1981:??.??:??]", L"W"},
                      {L"в 1981 год 15 май", L"[??:15/5/1981:??.??:??]", L"W"},   //*
                      {L"1981 году", L"[??:?\?/?\?/1981:??.??:??]", L"W"},
                      {L"два часа дня", L"[??:?\?/?\?/?\?:14.??:??]", L"W"}, // TODO !!!
                      {L"в два часа дня", L"[??:?\?/?\?/?\?:14.??:??]", L"W"}, // TODO !!!
                      {L"в семь часов вечера", L"[??:?\?/?\?/?\?:19.??:??]", L"W"},
                      /*bad formed*/
                      //{L"два года назад", L"2", L"Z"},
                      //{L"второй час", L"2", L"Z"},
                      //{L"73 года", L"73", L"Z"},
                   };

    for (size_t i=0; i< sizeof(tst)/sizeof(tst[0]); ++i)
    {
        lw=tk.tokenize(tst[i].st);
        sentence sent(lw);
        num.analyze(sent);
        dat.analyze(sent);
        word::const_iterator wIt;

        It = sent.begin();
        word::const_iterator w = It->selected_begin();
        if (w != It->selected_end())
        {
            std::wstring lem = w->get_lemma();
            if ((lem != tst[i].chk) || w->get_tag() != tst[i].tag)
                BOOST_CHECK_MESSAGE (lem == tst[i].chk, util::wstring2string(lem));
        }
        else
            BOOST_FAIL("not defined :" << util::wstring2string(tst[i].st));
    }
}

