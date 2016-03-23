
#include <boost/test/test_tools.hpp>
#include "freeling/morfo/dictionary.h"
#include "freeling/morfo/util.h"
#include "performanceMon.h"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace freeling;

std::wstring path;
dictionary *dict, *dict_p;

void init()
{
  #if defined WIN32 || defined WIN64
    util::init_locale(L"rus");
    path=L"../../../../windows_bin/freeling/data/ru/";
  #else
    util::init_locale(L"ru_RU.utf8");
    path=L"share/freeling/ru/";
  #endif
}

void dictionary_read_test()
{
  init();
  dict = new dictionary(L"ru", path+L"dicc.src", L"", L"", false, false);
}

void dictionary_read_test_pref()
{
  init();
  dict = new dictionary(L"ru", path+L"dicc_p.src", L"", L"", false, false);
}

size_t dictionary_key_find(dictionary *d, int *forms)
{
  std::wifstream fdic;
  util::open_utf8_file(fdic, path+L"dicc.src");
  if (fdic.fail())
    std::wcout << L"Error opening file ";
  
  std::wstring line; 
  getline(fdic, line); //skip
  
  getline(fdic,line);
  std::list<analysis> la;
  size_t cnt = 0;
  do 
    {
      std::wstring::size_type pos = line.find(L" ");
      std::wstring key=line.substr(0,pos);
      la.clear();
      d->search_form(key, la);
      if (!la.empty())
        {
          ++cnt;
          *forms+=la.size();
        }
    } 
  while (getline(fdic,line));
  return cnt;
}

void run_read_test()
{
  {
    int f = 0;
    std::wcout << L"load hash map: " <<_benchmark(dictionary_read_test) << std::endl;
    beginCheckTime(2);
    std::wcout << L"key found hash map count: " << dictionary_key_find(dict, &f) << std::endl;
    std::wcout << L"forms: " << f << std::endl;
    std::wcout << L"read all keys time spend: " ; endCheckTime(2);
    delete dict;
  }
  
  {
    int f = 0;
    std::wcout << L"load pref tree: " <<_benchmark(dictionary_read_test_pref) << std::endl;
    beginCheckTime(3);
    std::wcout << L"key found pref tree count: " << dictionary_key_find(dict_p, &f) << std::endl;
    std::wcout << L"forms: " << f << std::endl;
    std::wcout << L"read all keys time spend: " ; endCheckTime(3);
    delete dict_p;
  }
}
