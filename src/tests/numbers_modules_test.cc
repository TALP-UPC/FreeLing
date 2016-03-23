
#include <boost/test/test_tools.hpp>
#include "freeling/morfo/tokenizer.h"
#include "freeling/morfo/numbers.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/util.h"
#include "common.h"

using namespace freeling;

void en_numbers_test()
{
	std::wstring path;
#if defined WIN32 || defined WIN64
	util::init_locale(L"eng");
	path=L"../../../../windows_bin/freeling/data/en/";
#else
	util::init_locale(L"en_US.utf8");
	path=L"share/freeling/en/";
#endif
    
    tokenizer tk(path+L"tokenizer.dat");
    
    numbers num(L"en", L"", L"");
    std::list<word> lw;
    sentence::iterator It;
    
    typedef struct wTest
    {
        const std::wstring st;
        const std::wstring chk;
    } _wTest;
    _wTest tst[] = {
                      {L"one hundred ninety-two", L"192"},
                      {L"five", L"5"},
                      {L"twenty-six", L"26"},
                  };
                  
    for (size_t i=0; i< sizeof(tst)/sizeof(tst[0]); ++i)
    {
        lw=tk.tokenize(tst[i].st);
        sentence sent(lw);
        num.analyze(sent);
        word::const_iterator wIt;
        for (It = sent.begin(); It != sent.end(); ++It)
        {
            word::const_iterator w = It->selected_begin();
            std::string chkStr = util::wstring2string(tst[i].chk);
            if (w != It->selected_end())
            {
                std::wstring lem = w->get_lemma();
                BOOST_CHECK_MESSAGE (lem == tst[i].chk, chkStr);
            }
            else
                BOOST_FAIL("not defined :" << chkStr);
        }
    }                  
}


void ru_numbers_test()
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
    
    numbers num(L"ru", L"", L"");
    std::list<word> lw;
    sentence::iterator It;
    
    typedef struct wTest
    {
        const std::wstring st;
        const std::wstring chk;
    } _wTest;
    _wTest tst[] = {
                      {L"второй", L"2"},
                      {L"пятый", L"5"},
                      {L"двадцать второй", L"22"},
                      {L"тридцатый", L"30"},
                      {L"сто пятьдесят седьмой", L"157"},
                      {L"двухсотый", L"200"},

                      {L"четверть миллиона", L"250000"},
                      {L"четверть сотни", L"25"},
                      {L"четверть тысячи", L"250"},
                      {L"четверть миллиарда", L"250000000"},
                      //{L"четверть", L""},

                      {L"два с половиной", L"2.5"},
                      {L"два с половиной миллиарда", L"2500000000"},
                      {L"две с половиной тысячи", L"2500"},
                      {L"две с половиной сотни", L"250"},

                      {L"два", L"2"},
                      {L"двадцать", L"20"},
                      {L"восемнадцать с половиной", L"18.5"},
                      {L"двадцать два", L"22"},
                      {L"сто", L"100"},
                      {L"сто двадцать два", L"122"},
                      {L"двести", L"200"},
                      {L"тысяча", L"1000"},
                      {L"три тысячи", L"3000"},
                      {L"три тысячи двадцать два", L"3022"},
                      {L"3 тысячи двадцать два", L"3022"},
                      {L"3 тысячи двадцать два с половиной", L"3022.5"},
                      {L"миллион", L"1000000"},
                      {L"миллиард", L"1000000000"},

                      {L"полсотни", L"50"},
                      {L"полтысячи", L"500"},
                      {L"полмиллиона", L"500000"},
                      {L"полмиллиарда", L"500000000"},
                      {L"2 млн", L"2000000"},
                      {L"3 тыс", L"3000"},
                             
                      {L"3022", L"3022"},
                      {L"30.22", L"30.22"},
                      {L"30,22", L"30.22"},                      

                      {L"12-й", L"12"},
                      {L"22-й", L"22"},
                      {L"3-й", L"3"},
                      {L"5-я", L"5"},
                      {L"20-ть", L"20"},
                  };
                  
    for (size_t i=0; i< sizeof(tst)/sizeof(tst[0]); ++i)
    {
        lw=tk.tokenize(tst[i].st);
        sentence sent(lw);
        num.analyze(sent);
        word::const_iterator wIt;
        for (It = sent.begin(); It != sent.end(); ++It)
        {
            word::const_iterator w = It->selected_begin();
            std::string chkStr(tst[i].chk.length(), L' '); 
            std::copy(tst[i].chk.begin(), tst[i].chk.end(), chkStr.begin());
            if (w != It->selected_end())
            {
                std::wstring lem = w->get_lemma();
                BOOST_CHECK_MESSAGE (lem == tst[i].chk, chkStr);
            }
            else
                BOOST_FAIL("not defined :" << chkStr);
        }
    }                  
}
