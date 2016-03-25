
#include <boost/test/test_tools.hpp>
#include "freeling/morfo/np.h"
#include "freeling/morfo/numbers.h"
#include "freeling/morfo/tokenizer.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/dictionary.h"
#include "freeling/morfo/dates.h"
#include "freeling/morfo/util.h"

#include "common.h"

using namespace freeling;

void ru_np_test()
{
	std::wstring path;
#if defined WIN32 || defined WIN64
	util::init_locale(L"eng");
	path=L"../../../../windows_bin/freeling/data/en/";
    revert_mode();
#else
	util::init_locale(L"en_US.utf8");
	path=L"share/freeling/en/";
#endif
    
    np npModule(path+L"np.dat");
    numbers num(L"en", L"", L"");
    dates dat(L"ru");
    dictionary dict(L"en", path+L"dicc.src", path+L"afixos.dat", L"");
    tokenizer tk(path+L"tokenizer.dat");
    const std::wstring npTag = L"NP";
    const int ARR_SIZE = 5;
    
    std::list<word> lw;
    sentence::iterator It;
    
    typedef struct wTest
    {
        const std::wstring st;
    } _wTest;
    _wTest tst[] = {
        {L"Банк Возрождение был основан."},
        {L"In April 2011, Chapman was on the runway as a catwalk model for Moscow Fashion Week at the Shiyan & Rudkovskaya show."},
        {L"Therefore, we recommend using a relational database (MySQL, PostgreSQL, Microsoft SQL server, etc.) or a data warehousing\
          system (Netezza, etc.) for storage and processing."} 
    };

    _wTest etalon[][ARR_SIZE] = {{L"банк_возрождение", L"", L"", L"", L""},
                                   {L"chapman", L"moscow_fashion_week", L"shiyan_&_rudkovskaya", L"", L""},
                                   {L"mysql", L"postgresql", L"microsoft_sql", L"netezza", L""}};

    for (size_t i=0; i< sizeof(tst)/sizeof(tst[0]); ++i)
    {
        lw=tk.tokenize(tst[i].st);
        sentence sent(lw);
        num.analyze(sent);
        dat.analyze(sent);
        dict.analyze(sent);
        npModule.analyze(sent);
        word::const_iterator wIt;
        int cnt = 0;
        for (It = sent.begin(); It != sent.end(); ++It)
        {
            word::const_iterator w = It->analysis_begin();
            if (w != It->analysis_end() && (It->get_tag() == npTag))
            {                
                if (cnt == ARR_SIZE)
                    BOOST_ERROR("something wrong");
                std::wstring lem = w->get_lemma();
                std::wstringstream sErr;
                sErr << lem << L" != " << etalon[i][cnt].st;
                BOOST_CHECK_MESSAGE(lem == etalon[i][cnt].st, util::wstring2string(sErr.str()));
                ++cnt;
            }
        }
    }                  
}
