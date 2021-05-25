//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#include <cmath>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/numbers_modules.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"NUMBERS"
#define MOD_TRACECODE NUMBERS_TRACE


  //---------------------------------------------------------------------------
  //        Czech number recognizer
  //---------------------------------------------------------------------------
  // TOKENS
#define TK_other 0
#define TK_code 1
#define TK_ord 2
#define TK_millrd 3
#define TK_mill 4
#define TK_thous 5
#define TK_minus 6
#define TK_zero 7
#define TK_unit 8
#define TK_unit_teen 9
#define TK_decimal 10
#define TK_decimal_unit_alternative 11
#define TK_hundred 12
#define TK_special 13
#define TK_num 14
  // STATES
#define ST_num_init_start 0
#define ST_minus_loaded_start 1
#define ST_zero_loaded_start 2
#define ST_unit_loaded_start 3
#define ST_unit_teen_loaded_start 4
#define ST_decimal_loaded_start 5
#define ST_decimal_unit_start 6
#define ST_hundred_loaded_start 7
#define ST_special_start 8
#define ST_number_start 9
#define ST_hundred_zero_start 10
#define ST_hundred_unit_start 11
#define ST_hundred_decimal_start 12
#define ST_hundred_decimal_unit_start 13
#define ST_num_init_millrd 14
#define ST_zero_loaded_millrd 15
#define ST_unit_loaded_millrd 16
#define ST_unit_teen_loaded_millrd 17
#define ST_decimal_loaded_millrd 18
#define ST_decimal_unit_millrd 19
#define ST_hundred_loaded_millrd 20
#define ST_special_millrd 21
#define ST_number_millrd 22
#define ST_hundred_zero_millrd 23
#define ST_hundred_unit_millrd 24
#define ST_hundred_decimal_millrd 25
#define ST_hundred_decimal_unit_millrd 26
#define ST_num_init_mill 27
#define ST_zero_loaded_mill 28
#define ST_unit_loaded_mill 29
#define ST_unit_teen_loaded_mill 30
#define ST_decimal_loaded_mill 31
#define ST_decimal_unit_mill 32
#define ST_hundred_loaded_mill 33
#define ST_special_mill 34
#define ST_number_mill 35
#define ST_hundred_zero_mill 36
#define ST_hundred_unit_mill 37
#define ST_hundred_decimal_mill 38
#define ST_hundred_decimal_unit_mill 39
#define ST_num_init_thous 40
#define ST_zero_loaded_thous 41
#define ST_unit_loaded_thous 42
#define ST_unit_teen_loaded_thous 43
#define ST_decimal_loaded_thous 44
#define ST_decimal_unit_thous 45
#define ST_hundred_loaded_thous 46
#define ST_special_thous 47
#define ST_number_thous 48
#define ST_hundred_zero_thous 49
#define ST_hundred_unit_thous 50
#define ST_hundred_decimal_thous 51
#define ST_hundred_decimal_unit_thous 52
#define ST_num_init 53
#define ST_zero_loaded 54
#define ST_unit_loaded 55
#define ST_unit_teen_loaded 56
#define ST_decimal_loaded 57
#define ST_decimal_unit 58
#define ST_hundred_loaded 59
#define ST_special 60
#define ST_number 61
#define ST_hundred_zero 62
#define ST_hundred_unit 63
#define ST_hundred_decimal 64
#define ST_hundred_decimal_unit 65
#define ST_STOP 66

  ///////////////////////////////////////////////////////////////
  ///  Create a numbers recognizer for English
  ///////////////////////////////////////////////////////////////

  numbers_cs::numbers_cs(const std::wstring &dec, const std::wstring &thou): numbers_module(dec,thou) {
 
    value.insert(make_pair(L"nula", 0.0f));    tok.insert(make_pair(L"nula",TK_zero));
    value.insert(make_pair(L"nultý", 0.0f));   tok.insert(make_pair(L"nultý",TK_zero));
    value.insert(make_pair(L"jeden", 1.0f));   tok.insert(make_pair(L"jeden",TK_unit));
    value.insert(make_pair(L"první", 1.0f));   tok.insert(make_pair(L"první",TK_unit));
    value.insert(make_pair(L"prvý", 1.0f));    tok.insert(make_pair(L"prvý",TK_unit));
    value.insert(make_pair(L"dva", 2.0f));     tok.insert(make_pair(L"dva",TK_unit));
    value.insert(make_pair(L"druhý", 2.0f));   tok.insert(make_pair(L"druhý",TK_unit));
    value.insert(make_pair(L"tři", 3.0f));     tok.insert(make_pair(L"tři",TK_unit));
    value.insert(make_pair(L"třetí", 3.0f));   tok.insert(make_pair(L"třetí",TK_unit));
    value.insert(make_pair(L"čtyři", 4.0f));   tok.insert(make_pair(L"čtyři",TK_unit));
    value.insert(make_pair(L"čtvrtý", 4.0f));  tok.insert(make_pair(L"čtvrtý",TK_unit));
    value.insert(make_pair(L"pět", 5.0f));     tok.insert(make_pair(L"pět",TK_unit));
    value.insert(make_pair(L"pátý", 5.0f));    tok.insert(make_pair(L"pátý",TK_unit));
    value.insert(make_pair(L"šest", 6.0f));    tok.insert(make_pair(L"šest",TK_unit));
    value.insert(make_pair(L"šestý", 6.0f));   tok.insert(make_pair(L"šestý",TK_unit));
    value.insert(make_pair(L"sedm", 7.0f));    tok.insert(make_pair(L"sedm",TK_unit));
    value.insert(make_pair(L"sedmý", 7.0f));   tok.insert(make_pair(L"sedmý",TK_unit));
    value.insert(make_pair(L"osm", 8.0f));     tok.insert(make_pair(L"osm",TK_unit));
    value.insert(make_pair(L"osmý", 8.0f));    tok.insert(make_pair(L"osmý",TK_unit));
    value.insert(make_pair(L"devět", 9.0f));   tok.insert(make_pair(L"devět",TK_unit));
    value.insert(make_pair(L"devátý", 9.0f));  tok.insert(make_pair(L"devátý",TK_unit));

    value.insert(make_pair(L"deset", 10.0f));       tok.insert(make_pair(L"deset",TK_unit_teen));
    value.insert(make_pair(L"desátý", 10.0f));      tok.insert(make_pair(L"desátý",TK_unit_teen));
    value.insert(make_pair(L"jedenáct", 11.0f));    tok.insert(make_pair(L"jedenáct",TK_unit_teen));
    value.insert(make_pair(L"jedenáctý", 11.0f));   tok.insert(make_pair(L"jedenáctý",TK_unit_teen));
    value.insert(make_pair(L"dvanáct", 12.0f));     tok.insert(make_pair(L"dvanáct",TK_unit_teen));
    value.insert(make_pair(L"dvanáctý", 12.0f));    tok.insert(make_pair(L"dvanáctý",TK_unit_teen));
    value.insert(make_pair(L"třináct", 13.0f));     tok.insert(make_pair(L"třináct",TK_unit_teen));
    value.insert(make_pair(L"třináctý", 13.0f));    tok.insert(make_pair(L"třináctý",TK_unit_teen));
    value.insert(make_pair(L"čtrnáct", 14.0f));     tok.insert(make_pair(L"čtrnáct",TK_unit_teen));
    value.insert(make_pair(L"čtrnáctý", 14.0f));    tok.insert(make_pair(L"čtrnáctý",TK_unit_teen));
    value.insert(make_pair(L"patnáct", 15.0f));     tok.insert(make_pair(L"patnáct",TK_unit_teen));
    value.insert(make_pair(L"patnáctý", 15.0f));    tok.insert(make_pair(L"patnáctý",TK_unit_teen));
    value.insert(make_pair(L"šestnáct", 16.0f));    tok.insert(make_pair(L"šestnáct",TK_unit_teen));
    value.insert(make_pair(L"šestnáctý", 16.0f));   tok.insert(make_pair(L"šestnáctý",TK_unit_teen));
    value.insert(make_pair(L"sedmnáct", 17.0f));    tok.insert(make_pair(L"sedmnáct",TK_unit_teen));
    value.insert(make_pair(L"sedmnáctý", 17.0f));   tok.insert(make_pair(L"sedmnáctý",TK_unit_teen));
    value.insert(make_pair(L"osmnáct", 18.0f));     tok.insert(make_pair(L"osmnáct",TK_unit_teen));
    value.insert(make_pair(L"osmnáctý", 18.0f));    tok.insert(make_pair(L"osmnáctý",TK_unit_teen));
    value.insert(make_pair(L"devatenáct", 19.0f));  tok.insert(make_pair(L"devatenáct",TK_unit_teen));
    value.insert(make_pair(L"devatenáctý", 19.0f)); tok.insert(make_pair(L"devatenáctý",TK_unit_teen));

    value.insert(make_pair(L"dvacet", 20.0f));      tok.insert(make_pair(L"dvacet",TK_decimal));
    value.insert(make_pair(L"dvacátý", 20.0f));     tok.insert(make_pair(L"dvacátý",TK_decimal));
    value.insert(make_pair(L"třicet", 30.0f));      tok.insert(make_pair(L"třicet",TK_decimal));
    value.insert(make_pair(L"třicátý", 30.0f));     tok.insert(make_pair(L"třicátý",TK_decimal));
    value.insert(make_pair(L"čtyřicet", 40.0f));    tok.insert(make_pair(L"čtyřicet",TK_decimal));
    value.insert(make_pair(L"čtyřicátý", 40.0f));   tok.insert(make_pair(L"čtyřicátý",TK_decimal));
    value.insert(make_pair(L"padesát", 50.0f));     tok.insert(make_pair(L"padesát",TK_decimal));
    value.insert(make_pair(L"padesátý", 50.0f));    tok.insert(make_pair(L"padesátý",TK_decimal));
    value.insert(make_pair(L"šedesát", 60.0f));     tok.insert(make_pair(L"šedesát",TK_decimal));
    value.insert(make_pair(L"šedesátý", 60.0f));    tok.insert(make_pair(L"šedesátý",TK_decimal));
    value.insert(make_pair(L"sedmdesát", 70.0f));   tok.insert(make_pair(L"sedmdesát",TK_decimal));
    value.insert(make_pair(L"sedmdesátý", 70.0f));  tok.insert(make_pair(L"sedmdesátý",TK_decimal));
    value.insert(make_pair(L"osmdesát", 80.0f));    tok.insert(make_pair(L"osmdesát",TK_decimal));
    value.insert(make_pair(L"osmdesátý", 80.0f));   tok.insert(make_pair(L"osmdesátý",TK_decimal));
    value.insert(make_pair(L"devadesát", 90.0f));   tok.insert(make_pair(L"devadesát",TK_decimal));
    value.insert(make_pair(L"devadesátý", 90.0f));  tok.insert(make_pair(L"devadesátý",TK_decimal));

    value.insert(make_pair(L"jednaadvacet", 21.0f));   tok.insert(make_pair(L"jednaadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníadvacet", 21.0f));   tok.insert(make_pair(L"prvníadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýadvacet", 21.0f));    tok.insert(make_pair(L"prvýadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaadvacet", 22.0f));     tok.insert(make_pair(L"dvaadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýadvacet", 22.0f));   tok.insert(make_pair(L"druhýadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiadvacet", 23.0f));     tok.insert(make_pair(L"třiadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíadvacet", 23.0f));   tok.insert(make_pair(L"třetíadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiadvacet", 24.0f));   tok.insert(make_pair(L"čtyřiadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýadvacet", 24.0f));  tok.insert(make_pair(L"čtvrtýadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětadvacet", 25.0f));     tok.insert(make_pair(L"pětadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýadvacet", 25.0f));    tok.insert(make_pair(L"pátýadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestadvacet", 26.0f));    tok.insert(make_pair(L"šestadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýadvacet", 26.0f));   tok.insert(make_pair(L"šestýadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmadvacet", 27.0f));    tok.insert(make_pair(L"sedmadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýadvacet", 27.0f));   tok.insert(make_pair(L"sedmýadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmadvacet", 28.0f));     tok.insert(make_pair(L"osmadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýadvacet", 28.0f));    tok.insert(make_pair(L"osmýadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětadvacet", 29.0f));   tok.insert(make_pair(L"devětadvacet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýadvacet", 29.0f));  tok.insert(make_pair(L"devátýadvacet",TK_decimal_unit_alternative));

    value.insert(make_pair(L"jednaatřicet", 31.0f));   tok.insert(make_pair(L"jednaatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníatřicet", 31.0f));   tok.insert(make_pair(L"prvníatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýatřicet", 31.0f));    tok.insert(make_pair(L"prvýatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaatřicet", 32.0f));     tok.insert(make_pair(L"dvaatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýatřicet", 32.0f));   tok.insert(make_pair(L"druhýatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiatřicet", 33.0f));     tok.insert(make_pair(L"třiatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíatřicet", 33.0f));   tok.insert(make_pair(L"třetíatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiatřicet", 34.0f));   tok.insert(make_pair(L"čtyřiatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýatřicet", 34.0f));  tok.insert(make_pair(L"čtvrtýatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětatřicet", 35.0f));     tok.insert(make_pair(L"pětatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýatřicet", 35.0f));    tok.insert(make_pair(L"pátýatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestatřicet", 36.0f));    tok.insert(make_pair(L"šestatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýatřicet", 36.0f));   tok.insert(make_pair(L"šestýatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmatřicet", 37.0f));    tok.insert(make_pair(L"sedmatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýatřicet", 37.0f));   tok.insert(make_pair(L"sedmýatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmatřicet", 38.0f));     tok.insert(make_pair(L"osmatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýatřicet", 38.0f));    tok.insert(make_pair(L"osmýatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětatřicet", 39.0f));   tok.insert(make_pair(L"devětatřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýatřicet", 39.0f));  tok.insert(make_pair(L"devátýatřicet",TK_decimal_unit_alternative));

    value.insert(make_pair(L"jednaačtyřicet", 41.0f));  tok.insert(make_pair(L"jednaačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníačtyřicet", 41.0f));  tok.insert(make_pair(L"prvníačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýačtyřicet", 41.0f));   tok.insert(make_pair(L"prvýačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaačtyřicet", 42.0f));    tok.insert(make_pair(L"dvaačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýačtyřicet", 42.0f));  tok.insert(make_pair(L"druhýačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiačtyřicet", 43.0f));    tok.insert(make_pair(L"třiačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíačtyřicet", 43.0f));  tok.insert(make_pair(L"třetíačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiačtyřicet", 44.0f));  tok.insert(make_pair(L"čtyřiačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýačtyřicet", 44.0f)); tok.insert(make_pair(L"čtvrtýačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětačtyřicet", 45.0f));    tok.insert(make_pair(L"pětačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýačtyřicet", 45.0f));   tok.insert(make_pair(L"pátýačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestačtyřicet", 46.0f));   tok.insert(make_pair(L"šestačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýačtyřicet", 46.0f));  tok.insert(make_pair(L"šestýačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmačtyřicet", 47.0f));   tok.insert(make_pair(L"sedmačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýačtyřicet", 47.0f));  tok.insert(make_pair(L"sedmýačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmačtyřicet", 48.0f));    tok.insert(make_pair(L"osmačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýačtyřicet", 48.0f));   tok.insert(make_pair(L"osmýačtyřicet",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětačtyřicet", 49.0f));  tok.insert(make_pair(L"devětačtyřicet",TK_decimal_unit_alternative)); 
    value.insert(make_pair(L"devátýačtyřicet", 49.0f)); tok.insert(make_pair(L"devátýačtyřicet",TK_decimal_unit_alternative));
 
    value.insert(make_pair(L"jednaapadesát", 51.0f));    tok.insert(make_pair(L"jednaapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníapadesát", 51.0f));    tok.insert(make_pair(L"prvníapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýapadesát", 51.0f));     tok.insert(make_pair(L"prvýapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaapadesát", 52.0f));      tok.insert(make_pair(L"dvaapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýapadesát", 52.0f));    tok.insert(make_pair(L"druhýapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiapadesát", 53.0f));      tok.insert(make_pair(L"třiapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíapadesát", 53.0f));    tok.insert(make_pair(L"třetíapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiapadesát", 54.0f));    tok.insert(make_pair(L"čtyřiapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýapadesát", 54.0f));   tok.insert(make_pair(L"čtvrtýapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětapadesát", 55.0f));      tok.insert(make_pair(L"pětapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýapadesát", 55.0f));     tok.insert(make_pair(L"pátýapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestapadesát", 56.0f));     tok.insert(make_pair(L"šestapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýapadesát", 56.0f));    tok.insert(make_pair(L"šestýapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmapadesát", 57.0f));     tok.insert(make_pair(L"sedmapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýapadesát", 57.0f));    tok.insert(make_pair(L"sedmýapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmapadesát", 58.0f));      tok.insert(make_pair(L"osmapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýapadesát", 58.0f));     tok.insert(make_pair(L"osmýapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětapadesát", 59.0f));    tok.insert(make_pair(L"devětapadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýapadesát", 59.0f));   tok.insert(make_pair(L"devátýapadesát",TK_decimal_unit_alternative));

    value.insert(make_pair(L"jednaašedesát", 61.0f));    tok.insert(make_pair(L"jednaašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníašedesát", 61.0f));    tok.insert(make_pair(L"prvníašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýašedesát", 61.0f));     tok.insert(make_pair(L"prvýašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaašedesát", 62.0f));      tok.insert(make_pair(L"dvaašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýašedesát", 62.0f));    tok.insert(make_pair(L"druhýašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiašedesát", 63.0f));      tok.insert(make_pair(L"třiašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíašedesát", 63.0f));    tok.insert(make_pair(L"třetíašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiašedesát", 64.0f));    tok.insert(make_pair(L"čtyřiašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýašedesát", 64.0f));   tok.insert(make_pair(L"čtvrtýašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětašedesát", 65.0f));      tok.insert(make_pair(L"pětašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýašedesát", 65.0f));     tok.insert(make_pair(L"pátýašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestašedesát", 66.0f));     tok.insert(make_pair(L"šestašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýašedesát", 66.0f));    tok.insert(make_pair(L"šestýašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmašedesát", 67.0f));     tok.insert(make_pair(L"sedmašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýašedesát", 67.0f));    tok.insert(make_pair(L"sedmýašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmašedesát", 68.0f));      tok.insert(make_pair(L"osmašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýašedesát", 68.0f));     tok.insert(make_pair(L"osmýašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětašedesát", 69.0f));    tok.insert(make_pair(L"devětašedesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýašedesát", 69.0f));   tok.insert(make_pair(L"devátýašedesát",TK_decimal_unit_alternative));

    value.insert(make_pair(L"jednaasedmdesát", 71.0f));  tok.insert(make_pair(L"jednaasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníasedmdesát", 71.0f));  tok.insert(make_pair(L"prvníasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýasedmdesát", 71.0f));   tok.insert(make_pair(L"prvýasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaasedmdesát", 72.0f));    tok.insert(make_pair(L"dvaasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýasedmdesát", 72.0f));  tok.insert(make_pair(L"druhýasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiasedmdesát", 73.0f));    tok.insert(make_pair(L"třiasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíasedmdesát", 73.0f));  tok.insert(make_pair(L"třetíasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiasedmdesát", 74.0f));  tok.insert(make_pair(L"čtyřiasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýasedmdesát", 74.0f)); tok.insert(make_pair(L"čtvrtýasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětasedmdesát", 75.0f));    tok.insert(make_pair(L"pětasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýasedmdesát", 75.0f));   tok.insert(make_pair(L"pátýasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestasedmdesát", 76.0f));   tok.insert(make_pair(L"šestasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýasedmdesát", 76.0f));  tok.insert(make_pair(L"šestýasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmasedmdesát", 77.0f));   tok.insert(make_pair(L"sedmasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýasedmdesát", 77.0f));  tok.insert(make_pair(L"sedmýasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmasedmdesát", 78.0f));    tok.insert(make_pair(L"osmasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýasedmdesát", 78.0f));   tok.insert(make_pair(L"osmýasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětasedmdesát", 79.0f));  tok.insert(make_pair(L"devětasedmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýasedmdesát", 79.0f)); tok.insert(make_pair(L"devátýasedmdesát",TK_decimal_unit_alternative));
 
    value.insert(make_pair(L"jednaaosmdesát", 81.0f));   tok.insert(make_pair(L"jednaaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaaosmdesát", 82.0f));     tok.insert(make_pair(L"dvaaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiaosmdesát", 83.0f));     tok.insert(make_pair(L"třiaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiaosmdesát", 84.0f));   tok.insert(make_pair(L"čtyřiaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětaosmdesát", 85.0f));     tok.insert(make_pair(L"pětaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestaosmdesát", 86.0f));    tok.insert(make_pair(L"šestaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmaosmdesát", 87.0f));    tok.insert(make_pair(L"sedmaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmaosmdesát", 88.0f));     tok.insert(make_pair(L"osmaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětaosmdesát", 89.0f));   tok.insert(make_pair(L"devětaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníaosmdesát", 81.0f));   tok.insert(make_pair(L"prvníaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýaosmdesát", 81.0f));    tok.insert(make_pair(L"prvýaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýaosmdesát", 82.0f));   tok.insert(make_pair(L"druhýaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíaosmdesát", 83.0f));   tok.insert(make_pair(L"třetíaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýaosmdesát", 84.0f));  tok.insert(make_pair(L"čtvrtýaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýaosmdesát", 85.0f));    tok.insert(make_pair(L"pátýaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýaosmdesát", 86.0f));   tok.insert(make_pair(L"šestýaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýaosmdesát", 87.0f));   tok.insert(make_pair(L"sedmýaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýaosmdesát", 88.0f));    tok.insert(make_pair(L"osmýaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýaosmdesát", 89.0f));  tok.insert(make_pair(L"devátýaosmdesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"jednaadevadesát", 91.0f));  tok.insert(make_pair(L"jednaadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaadevadesát", 92.0f));    tok.insert(make_pair(L"dvaadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiadevadesát", 93.0f));    tok.insert(make_pair(L"třiadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiadevadesát", 94.0f));  tok.insert(make_pair(L"čtyřiadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětadevadesát", 95.0f));    tok.insert(make_pair(L"pětadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestadevadesát", 96.0f));   tok.insert(make_pair(L"šestadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmadevadesát", 97.0f));   tok.insert(make_pair(L"sedmadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmadevadesát", 98.0f));    tok.insert(make_pair(L"osmadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětadevadesát", 99.0f));  tok.insert(make_pair(L"devětadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníadevadesát", 91.0f));  tok.insert(make_pair(L"prvníadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýadevadesát", 91.0f));   tok.insert(make_pair(L"prvýadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýadevadesát", 92.0f));  tok.insert(make_pair(L"druhýadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíadevadesát", 93.0f));  tok.insert(make_pair(L"třetíadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýadevadesát", 94.0f)); tok.insert(make_pair(L"čtvrtýadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýadevadesát", 95.0f));   tok.insert(make_pair(L"pátýadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýadevadesát", 96.0f));  tok.insert(make_pair(L"šestýadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýadevadesát", 97.0f));  tok.insert(make_pair(L"sedmýadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýadevadesát", 98.0f));   tok.insert(make_pair(L"osmýadevadesát",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýadevadesát", 99.0f)); tok.insert(make_pair(L"devátýadevadesát",TK_decimal_unit_alternative));

    value.insert(make_pair(L"jednaadvacátý", 21.0f));  tok.insert(make_pair(L"jednaadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaadvacátý", 22.0f));    tok.insert(make_pair(L"dvaadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiadvacátý", 23.0f));    tok.insert(make_pair(L"třiadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiadvacátý", 24.0f));  tok.insert(make_pair(L"čtyřiadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětadvacátý", 25.0f));    tok.insert(make_pair(L"pětadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestadvacátý", 26.0f));   tok.insert(make_pair(L"šestadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmadvacátý", 27.0f));   tok.insert(make_pair(L"sedmadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmadvacátý", 28.0f));    tok.insert(make_pair(L"osmadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětadvacátý", 29.0f));  tok.insert(make_pair(L"devětadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníadvacátý", 21.0f));  tok.insert(make_pair(L"prvníadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýadvacátý", 21.0f));   tok.insert(make_pair(L"prvýadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýadvacátý", 22.0f));  tok.insert(make_pair(L"druhýadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíadvacátý", 23.0f));  tok.insert(make_pair(L"třetíadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýadvacátý", 24.0f)); tok.insert(make_pair(L"čtvrtýadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýadvacátý", 25.0f));   tok.insert(make_pair(L"pátýadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýadvacátý", 26.0f));  tok.insert(make_pair(L"šestýadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýadvacátý", 27.0f));  tok.insert(make_pair(L"sedmýadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýadvacátý", 28.0f));   tok.insert(make_pair(L"osmýadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýadvacátý", 29.0f)); tok.insert(make_pair(L"devátýadvacátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"jednaatřicátý", 31.0f));  tok.insert(make_pair(L"jednaatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaatřicátý", 32.0f));    tok.insert(make_pair(L"dvaatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiatřicátý", 33.0f));    tok.insert(make_pair(L"třiatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiatřicátý", 34.0f));  tok.insert(make_pair(L"čtyřiatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětatřicátý", 35.0f));    tok.insert(make_pair(L"pětatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestatřicátý", 36.0f));   tok.insert(make_pair(L"šestatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmatřicátý", 37.0f));   tok.insert(make_pair(L"sedmatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmatřicátý", 38.0f));    tok.insert(make_pair(L"osmatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětatřicátý", 39.0f));  tok.insert(make_pair(L"devětatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníatřicátý", 31.0f));  tok.insert(make_pair(L"prvníatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýatřicátý", 31.0f));   tok.insert(make_pair(L"prvýatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýatřicátý", 32.0f));  tok.insert(make_pair(L"druhýatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíatřicátý", 33.0f));  tok.insert(make_pair(L"třetíatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýatřicátý", 34.0f)); tok.insert(make_pair(L"čtvrtýatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýatřicátý", 35.0f));   tok.insert(make_pair(L"pátýatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýatřicátý", 36.0f));  tok.insert(make_pair(L"šestýatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýatřicátý", 37.0f));  tok.insert(make_pair(L"sedmýatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýatřicátý", 38.0f));   tok.insert(make_pair(L"osmýatřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýatřicátý", 39.0f)); tok.insert(make_pair(L"devátýatřicátý",TK_decimal_unit_alternative));

    value.insert(make_pair(L"jednaačtyřicátý", 41.0f)); tok.insert(make_pair(L"jednaačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaačtyřicátý", 42.0f));   tok.insert(make_pair(L"dvaačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiačtyřicátý", 43.0f));   tok.insert(make_pair(L"třiačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiačtyřicátý", 44.0f)); tok.insert(make_pair(L"čtyřiačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětačtyřicátý", 45.0f));   tok.insert(make_pair(L"pětačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestačtyřicátý", 46.0f));  tok.insert(make_pair(L"šestačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmačtyřicátý", 47.0f));  tok.insert(make_pair(L"sedmačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmačtyřicátý", 48.0f));   tok.insert(make_pair(L"osmačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětačtyřicátý", 49.0f)); tok.insert(make_pair(L"devětačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníačtyřicátý", 41.0f)); tok.insert(make_pair(L"prvníačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýačtyřicátý", 41.0f));  tok.insert(make_pair(L"prvýačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýačtyřicátý", 42.0f)); tok.insert(make_pair(L"druhýačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíačtyřicátý", 43.0f)); tok.insert(make_pair(L"třetíačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýačtyřicátý", 44.0f)); tok.insert(make_pair(L"čtvrtýačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýačtyřicátý", 45.0f));   tok.insert(make_pair(L"pátýačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýačtyřicátý", 46.0f));  tok.insert(make_pair(L"šestýačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýačtyřicátý", 47.0f));  tok.insert(make_pair(L"sedmýačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýačtyřicátý", 48.0f));   tok.insert(make_pair(L"osmýačtyřicátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýačtyřicátý", 49.0f)); tok.insert(make_pair(L"devátýačtyřicátý",TK_decimal_unit_alternative));

    value.insert(make_pair(L"jednaapadesátý", 51.0f));  tok.insert(make_pair(L"jednaapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaapadesátý", 52.0f));    tok.insert(make_pair(L"dvaapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiapadesátý", 53.0f));    tok.insert(make_pair(L"třiapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiapadesátý", 54.0f));  tok.insert(make_pair(L"čtyřiapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětapadesátý", 55.0f));    tok.insert(make_pair(L"pětapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestapadesátý", 56.0f));   tok.insert(make_pair(L"šestapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmapadesátý", 57.0f));   tok.insert(make_pair(L"sedmapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmapadesátý", 58.0f));    tok.insert(make_pair(L"osmapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětapadesátý", 59.0f));  tok.insert(make_pair(L"devětapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníapadesátý", 51.0f));  tok.insert(make_pair(L"prvníapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýapadesátý", 51.0f));   tok.insert(make_pair(L"prvýapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýapadesátý", 52.0f));  tok.insert(make_pair(L"druhýapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíapadesátý", 53.0f));  tok.insert(make_pair(L"třetíapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýapadesátý", 54.0f)); tok.insert(make_pair(L"čtvrtýapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýapadesátý", 55.0f));   tok.insert(make_pair(L"pátýapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýapadesátý", 56.0f));  tok.insert(make_pair(L"šestýapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýapadesátý", 57.0f));  tok.insert(make_pair(L"sedmýapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýapadesátý", 58.0f));   tok.insert(make_pair(L"osmýapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýapadesátý", 59.0f)); tok.insert(make_pair(L"devátýapadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"jednaašedesátý", 61.0f));  tok.insert(make_pair(L"jednaašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaašedesátý", 62.0f));    tok.insert(make_pair(L"dvaašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiašedesátý", 63.0f));    tok.insert(make_pair(L"třiašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiašedesátý", 64.0f));  tok.insert(make_pair(L"čtyřiašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětašedesátý", 65.0f));    tok.insert(make_pair(L"pětašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestašedesátý", 66.0f));   tok.insert(make_pair(L"šestašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmašedesátý", 67.0f));   tok.insert(make_pair(L"sedmašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmašedesátý", 68.0f));    tok.insert(make_pair(L"osmašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětašedesátý", 69.0f));  tok.insert(make_pair(L"devětašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníašedesátý", 61.0f));  tok.insert(make_pair(L"prvníašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýašedesátý", 61.0f));   tok.insert(make_pair(L"prvýašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýašedesátý", 62.0f));  tok.insert(make_pair(L"druhýašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíašedesátý", 63.0f));  tok.insert(make_pair(L"třetíašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýašedesátý", 64.0f)); tok.insert(make_pair(L"čtvrtýašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýašedesátý", 65.0f));   tok.insert(make_pair(L"pátýašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýašedesátý", 66.0f));  tok.insert(make_pair(L"šestýašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýašedesátý", 67.0f));  tok.insert(make_pair(L"sedmýašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýašedesátý", 68.0f));   tok.insert(make_pair(L"osmýašedesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýašedesátý", 69.0f)); tok.insert(make_pair(L"devátýašedesátý",TK_decimal_unit_alternative));

    value.insert(make_pair(L"jednaasedmdesátý", 71.0f));  tok.insert(make_pair(L"jednaasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaasedmdesátý", 72.0f));    tok.insert(make_pair(L"dvaasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiasedmdesátý", 73.0f));    tok.insert(make_pair(L"třiasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiasedmdesátý", 74.0f));  tok.insert(make_pair(L"čtyřiasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětasedmdesátý", 75.0f));    tok.insert(make_pair(L"pětasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestasedmdesátý", 76.0f));   tok.insert(make_pair(L"šestasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmasedmdesátý", 77.0f));   tok.insert(make_pair(L"sedmasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmasedmdesátý", 78.0f));    tok.insert(make_pair(L"osmasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětasedmdesátý", 79.0f));  tok.insert(make_pair(L"devětasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníasedmdesátý", 71.0f));  tok.insert(make_pair(L"prvníasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýasedmdesátý", 71.0f));   tok.insert(make_pair(L"prvýasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýasedmdesátý", 72.0f));  tok.insert(make_pair(L"druhýasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíasedmdesátý", 73.0f));  tok.insert(make_pair(L"třetíasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýasedmdesátý", 74.0f)); tok.insert(make_pair(L"čtvrtýasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýasedmdesátý", 75.0f));   tok.insert(make_pair(L"pátýasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýasedmdesátý", 76.0f));  tok.insert(make_pair(L"šestýasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýasedmdesátý", 77.0f));  tok.insert(make_pair(L"sedmýasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýasedmdesátý", 78.0f));   tok.insert(make_pair(L"osmýasedmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýasedmdesátý", 79.0f)); tok.insert(make_pair(L"devátýasedmdesátý",TK_decimal_unit_alternative));

    value.insert(make_pair(L"jednaaosmdesátý", 81.0f));  tok.insert(make_pair(L"jednaaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaaosmdesátý", 82.0f));    tok.insert(make_pair(L"dvaaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiaosmdesátý", 83.0f));    tok.insert(make_pair(L"třiaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiaosmdesátý", 84.0f));  tok.insert(make_pair(L"čtyřiaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětaosmdesátý", 85.0f));    tok.insert(make_pair(L"pětaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestaosmdesátý", 86.0f));   tok.insert(make_pair(L"šestaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmaosmdesátý", 87.0f));   tok.insert(make_pair(L"sedmaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmaosmdesátý", 88.0f));    tok.insert(make_pair(L"osmaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětaosmdesátý", 89.0f));  tok.insert(make_pair(L"devětaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníaosmdesátý", 81.0f));  tok.insert(make_pair(L"prvníaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýaosmdesátý", 81.0f));   tok.insert(make_pair(L"prvýaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýaosmdesátý", 82.0f));  tok.insert(make_pair(L"druhýaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíaosmdesátý", 83.0f));  tok.insert(make_pair(L"třetíaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýaosmdesátý", 84.0f)); tok.insert(make_pair(L"čtvrtýaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýaosmdesátý", 85.0f));   tok.insert(make_pair(L"pátýaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýaosmdesátý", 86.0f));  tok.insert(make_pair(L"šestýaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýaosmdesátý", 87.0f));  tok.insert(make_pair(L"sedmýaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýaosmdesátý", 88.0f));   tok.insert(make_pair(L"osmýaosmdesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýaosmdesátý", 89.0f)); tok.insert(make_pair(L"devátýaosmdesátý",TK_decimal_unit_alternative));

    value.insert(make_pair(L"jednaadevadesátý", 91.0f));  tok.insert(make_pair(L"jednaadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"dvaadevadesátý", 92.0f));    tok.insert(make_pair(L"dvaadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třiadevadesátý", 93.0f));    tok.insert(make_pair(L"třiadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtyřiadevadesátý", 94.0f));  tok.insert(make_pair(L"čtyřiadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pětadevadesátý", 95.0f));    tok.insert(make_pair(L"pětadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestadevadesátý", 96.0f));   tok.insert(make_pair(L"šestadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmadevadesátý", 97.0f));   tok.insert(make_pair(L"sedmadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmadevadesátý", 98.0f));    tok.insert(make_pair(L"osmadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devětadevadesátý", 99.0f));  tok.insert(make_pair(L"devětadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvníadevadesátý", 91.0f));  tok.insert(make_pair(L"prvníadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"prvýadevadesátý", 91.0f));   tok.insert(make_pair(L"prvýadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"druhýadevadesátý", 92.0f));  tok.insert(make_pair(L"druhýadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"třetíadevadesátý", 93.0f));  tok.insert(make_pair(L"třetíadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"čtvrtýadevadesátý", 94.0f)); tok.insert(make_pair(L"čtvrtýadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"pátýadevadesátý", 95.0f));   tok.insert(make_pair(L"pátýadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"šestýadevadesátý", 96.0f));  tok.insert(make_pair(L"šestýadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"sedmýadevadesátý", 97.0f));  tok.insert(make_pair(L"sedmýadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"osmýadevadesátý", 98.0f));   tok.insert(make_pair(L"osmýadevadesátý",TK_decimal_unit_alternative));
    value.insert(make_pair(L"devátýadevadesátý", 99.0f));  tok.insert(make_pair(L"devátýadevadesátý",TK_decimal_unit_alternative));

    value.insert(make_pair(L"půltucet", 6.0f));     tok.insert(make_pair(L"půltucet",TK_special));
    value.insert(make_pair(L"tucet", 12.0f));       tok.insert(make_pair(L"tucet",TK_special));
    value.insert(make_pair(L"mandel", 15.0f));      tok.insert(make_pair(L"mandel",TK_special));
    value.insert(make_pair(L"kopa", 60.0f));        tok.insert(make_pair(L"kopa",TK_special));
    value.insert(make_pair(L"veletucet", 144.0f));  tok.insert(make_pair(L"veletucet",TK_special));
    value.insert(make_pair(L"velekopa", 3600.0f));  tok.insert(make_pair(L"velekopa",TK_special));
    value.insert(make_pair(L"minus", -1.0f));       tok.insert(make_pair(L"minus",TK_minus));
  
    tok.insert(make_pair(L"sto",TK_hundred));
    tok.insert(make_pair(L"tisíc",TK_thous));
    tok.insert(make_pair(L"milión",TK_mill));
    tok.insert(make_pair(L"milion",TK_mill));
    tok.insert(make_pair(L"miliarda",TK_millrd));
    /* tok.insert(make_pair(L"bilión",TK_bill));
       tok.insert(make_pair(L"bilion",TK_bill));
       tok.insert(make_pair(L"biliarda",TK_billrd));
       tok.insert(make_pair(L"trilión",TK_trill));
       tok.insert(make_pair(L"trilion",TK_trill));
       tok.insert(make_pair(L"triliarda",TK_trillrd));
       tok.insert(make_pair(L"kvadrilión",TK_kvrill));
       tok.insert(make_pair(L"kvadrilion",TK_kvrill));
       tok.insert(make_pair(L"kvadriliarda",TK_kvrillrd));
       tok.insert(make_pair(L"sextilion",TK_sext));
    */

    // Initializing power map
    // long scale
    power.insert(make_pair(TK_hundred, 100.0));
    power.insert(make_pair(TK_thous, 1000.0));
    power.insert(make_pair(TK_mill,  1000000.0));
    power.insert(make_pair(TK_millrd,1000000000.0));
    /* power.insert(make_pair(TK_bill, pow(10.0, 12)));
       power.insert(make_pair(TK_billrd, pow(10.0, 15)));
       power.insert(make_pair(TK_trill, pow(10.0, 18)));
       power.insert(make_pair(TK_trillrd, pow(10.0, 21)));
       power.insert(make_pair(TK_kvrill, pow(10.0, 24)));
       power.insert(make_pair(TK_kvrillrd, pow(10.0, 27)));
       power.insert(make_pair(TK_sext, pow(10.0, 36)));
    */  

    // Initialize special state attributes
    initialState=ST_num_init_start; stopState=ST_STOP;

    Final.insert(ST_minus_loaded_start);
    Final.insert(ST_zero_loaded_start);
    Final.insert(ST_unit_loaded_start);
    Final.insert(ST_unit_teen_loaded_start);
    Final.insert(ST_decimal_loaded_start);
    Final.insert(ST_decimal_unit_start);
    Final.insert(ST_hundred_loaded_start);
    Final.insert(ST_hundred_zero_start);
    Final.insert(ST_hundred_unit_start);
    Final.insert(ST_hundred_decimal_start);
    Final.insert(ST_hundred_decimal_unit_start);
    Final.insert(ST_special_start);
    Final.insert(ST_number_start);
    Final.insert(ST_num_init_millrd);
    Final.insert(ST_zero_loaded_millrd);
    Final.insert(ST_unit_loaded_millrd);
    Final.insert(ST_unit_teen_loaded_millrd);
    Final.insert(ST_decimal_loaded_millrd);
    Final.insert(ST_decimal_unit_millrd);
    Final.insert(ST_hundred_loaded_millrd);
    Final.insert(ST_hundred_zero_millrd);
    Final.insert(ST_hundred_unit_millrd);
    Final.insert(ST_hundred_decimal_millrd);
    Final.insert(ST_hundred_decimal_unit_millrd);
    Final.insert(ST_special_millrd);
    Final.insert(ST_number_millrd);
    Final.insert(ST_num_init_mill);
    Final.insert(ST_zero_loaded_mill);
    Final.insert(ST_unit_loaded_mill);
    Final.insert(ST_unit_teen_loaded_mill);
    Final.insert(ST_decimal_loaded_mill);
    Final.insert(ST_decimal_unit_mill);
    Final.insert(ST_hundred_loaded_mill);
    Final.insert(ST_hundred_zero_mill);
    Final.insert(ST_hundred_unit_mill);
    Final.insert(ST_hundred_decimal_mill);
    Final.insert(ST_hundred_decimal_unit_mill);
    Final.insert(ST_special_mill);
    Final.insert(ST_number_mill);
    Final.insert(ST_num_init_thous);
    Final.insert(ST_zero_loaded_thous);
    Final.insert(ST_unit_loaded_thous);
    Final.insert(ST_unit_teen_loaded_thous);
    Final.insert(ST_decimal_loaded_thous);
    Final.insert(ST_decimal_unit_thous);
    Final.insert(ST_hundred_loaded_thous);
    Final.insert(ST_hundred_zero_thous);
    Final.insert(ST_hundred_unit_thous);
    Final.insert(ST_hundred_decimal_thous);
    Final.insert(ST_hundred_decimal_unit_thous);
    Final.insert(ST_special_thous);
    Final.insert(ST_number_thous);
    Final.insert(ST_num_init);
    Final.insert(ST_zero_loaded);
    Final.insert(ST_unit_loaded);
    Final.insert(ST_unit_teen_loaded);
    Final.insert(ST_decimal_loaded);
    Final.insert(ST_decimal_unit);
    Final.insert(ST_hundred_loaded);
    Final.insert(ST_hundred_zero);
    Final.insert(ST_hundred_unit);
    Final.insert(ST_hundred_decimal);
    Final.insert(ST_hundred_decimal_unit);
    Final.insert(ST_special);
    Final.insert(ST_number);

    // Initialize transitions table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;
  
    // auto generated from numbers_transitions_dfa.rb
  
    // num_init_start - don't know if there will be quantifier or not yet
    trans[ST_num_init_start][TK_minus]=ST_minus_loaded_start;
    trans[ST_num_init_start][TK_zero]=ST_zero_loaded_start;
    trans[ST_num_init_start][TK_unit]=ST_unit_loaded_start;
    trans[ST_num_init_start][TK_unit_teen]=ST_unit_teen_loaded_start;
    trans[ST_num_init_start][TK_decimal]=ST_decimal_loaded_start;
    trans[ST_num_init_start][TK_decimal_unit_alternative]=ST_decimal_unit_start;
    trans[ST_num_init_start][TK_hundred]=ST_hundred_loaded_start;
    trans[ST_num_init_start][TK_special]=ST_special_start;
    trans[ST_num_init_start][TK_num]=ST_number_start;
    trans[ST_minus_loaded_start][TK_zero]=ST_zero_loaded_start;
    trans[ST_minus_loaded_start][TK_unit]=ST_unit_loaded_start;
    trans[ST_minus_loaded_start][TK_unit_teen]=ST_unit_teen_loaded_start;
    trans[ST_minus_loaded_start][TK_decimal]=ST_decimal_loaded_start;
    trans[ST_minus_loaded_start][TK_decimal_unit_alternative]=ST_decimal_unit_start;
    trans[ST_minus_loaded_start][TK_hundred]=ST_hundred_loaded_start;
    trans[ST_zero_loaded_start][TK_hundred]=ST_hundred_loaded_start;
    trans[ST_unit_loaded_start][TK_hundred]=ST_hundred_loaded_start;
    trans[ST_unit_teen_loaded_start][TK_hundred]=ST_hundred_loaded_start;
    trans[ST_decimal_loaded_start][TK_unit]=ST_decimal_unit_start;
    trans[ST_hundred_loaded_start][TK_zero]=ST_hundred_zero_start;
    trans[ST_hundred_loaded_start][TK_unit]=ST_hundred_unit_start;
    trans[ST_hundred_loaded_start][TK_unit_teen]=ST_hundred_unit_start;
    trans[ST_hundred_loaded_start][TK_decimal]=ST_hundred_decimal_start;
    trans[ST_hundred_decimal_start][TK_unit]=ST_hundred_decimal_unit_start;
    trans[ST_number_start][TK_num]=ST_number_start;

    // load next segment (number) after quantifier (except of "hundred")
    // for all quantifier words
    trans[ST_num_init_millrd][TK_zero]=ST_zero_loaded_millrd;
    trans[ST_num_init_millrd][TK_unit]=ST_unit_loaded_millrd;
    trans[ST_num_init_millrd][TK_unit_teen]=ST_unit_teen_loaded_millrd;
    trans[ST_num_init_millrd][TK_decimal]=ST_decimal_loaded_millrd;
    trans[ST_num_init_millrd][TK_decimal_unit_alternative]=ST_decimal_unit_millrd;
    trans[ST_num_init_millrd][TK_hundred]=ST_hundred_loaded_millrd;
    trans[ST_num_init_millrd][TK_special]=ST_special_millrd;
    trans[ST_num_init_millrd][TK_num]=ST_number_millrd;
    trans[ST_zero_loaded_millrd][TK_hundred]=ST_hundred_loaded_millrd;
    trans[ST_unit_loaded_millrd][TK_hundred]=ST_hundred_loaded_millrd;
    trans[ST_unit_teen_loaded_millrd][TK_hundred]=ST_hundred_loaded_millrd;
    trans[ST_decimal_loaded_millrd][TK_unit]=ST_decimal_unit_millrd;
    trans[ST_hundred_loaded_millrd][TK_zero]=ST_hundred_zero_millrd;
    trans[ST_hundred_loaded_millrd][TK_unit]=ST_hundred_unit_millrd;
    trans[ST_hundred_loaded_millrd][TK_unit_teen]=ST_hundred_unit_millrd;
    trans[ST_hundred_loaded_millrd][TK_decimal]=ST_hundred_decimal_millrd;
    trans[ST_hundred_decimal_millrd][TK_unit]=ST_hundred_decimal_unit_millrd;
    trans[ST_number_millrd][TK_num]=ST_number_millrd;

    trans[ST_num_init_mill][TK_zero]=ST_zero_loaded_mill;
    trans[ST_num_init_mill][TK_unit]=ST_unit_loaded_mill;
    trans[ST_num_init_mill][TK_unit_teen]=ST_unit_teen_loaded_mill;
    trans[ST_num_init_mill][TK_decimal]=ST_decimal_loaded_mill;
    trans[ST_num_init_mill][TK_decimal_unit_alternative]=ST_decimal_unit_mill;
    trans[ST_num_init_mill][TK_hundred]=ST_hundred_loaded_mill;
    trans[ST_num_init_mill][TK_special]=ST_special_mill;
    trans[ST_num_init_mill][TK_num]=ST_number_mill;
    trans[ST_zero_loaded_mill][TK_hundred]=ST_hundred_loaded_mill;
    trans[ST_unit_loaded_mill][TK_hundred]=ST_hundred_loaded_mill;
    trans[ST_unit_teen_loaded_mill][TK_hundred]=ST_hundred_loaded_mill;
    trans[ST_decimal_loaded_mill][TK_unit]=ST_decimal_unit_mill;
    trans[ST_hundred_loaded_mill][TK_zero]=ST_hundred_zero_mill;
    trans[ST_hundred_loaded_mill][TK_unit]=ST_hundred_unit_mill;
    trans[ST_hundred_loaded_mill][TK_unit_teen]=ST_hundred_unit_mill;
    trans[ST_hundred_loaded_mill][TK_decimal]=ST_hundred_decimal_mill;
    trans[ST_hundred_decimal_mill][TK_unit]=ST_hundred_decimal_unit_mill;
    trans[ST_number_mill][TK_num]=ST_number_mill;

    trans[ST_num_init_thous][TK_zero]=ST_zero_loaded_thous;
    trans[ST_num_init_thous][TK_unit]=ST_unit_loaded_thous;
    trans[ST_num_init_thous][TK_unit_teen]=ST_unit_teen_loaded_thous;
    trans[ST_num_init_thous][TK_decimal]=ST_decimal_loaded_thous;
    trans[ST_num_init_thous][TK_decimal_unit_alternative]=ST_decimal_unit_thous;
    trans[ST_num_init_thous][TK_hundred]=ST_hundred_loaded_thous;
    trans[ST_num_init_thous][TK_special]=ST_special_thous;
    trans[ST_num_init_thous][TK_num]=ST_number_thous;
    trans[ST_zero_loaded_thous][TK_hundred]=ST_hundred_loaded_thous;
    trans[ST_unit_loaded_thous][TK_hundred]=ST_hundred_loaded_thous;
    trans[ST_unit_teen_loaded_thous][TK_hundred]=ST_hundred_loaded_thous;
    trans[ST_decimal_loaded_thous][TK_unit]=ST_decimal_unit_thous;
    trans[ST_hundred_loaded_thous][TK_zero]=ST_hundred_zero_thous;
    trans[ST_hundred_loaded_thous][TK_unit]=ST_hundred_unit_thous;
    trans[ST_hundred_loaded_thous][TK_unit_teen]=ST_hundred_unit_thous;
    trans[ST_hundred_loaded_thous][TK_decimal]=ST_hundred_decimal_thous;
    trans[ST_hundred_decimal_thous][TK_unit]=ST_hundred_decimal_unit_thous;
    trans[ST_number_thous][TK_num]=ST_number_thous;

    trans[ST_num_init][TK_zero]=ST_zero_loaded;
    trans[ST_num_init][TK_unit]=ST_unit_loaded;
    trans[ST_num_init][TK_unit_teen]=ST_unit_teen_loaded;
    trans[ST_num_init][TK_decimal]=ST_decimal_loaded;
    trans[ST_num_init][TK_decimal_unit_alternative]=ST_decimal_unit;
    trans[ST_num_init][TK_hundred]=ST_hundred_loaded;
    trans[ST_num_init][TK_special]=ST_special;
    trans[ST_num_init][TK_num]=ST_number;
    trans[ST_zero_loaded][TK_hundred]=ST_hundred_loaded;
    trans[ST_unit_loaded][TK_hundred]=ST_hundred_loaded;
    trans[ST_unit_teen_loaded][TK_hundred]=ST_hundred_loaded;
    trans[ST_decimal_loaded][TK_unit]=ST_decimal_unit;
    trans[ST_hundred_loaded][TK_zero]=ST_hundred_zero;
    trans[ST_hundred_loaded][TK_unit]=ST_hundred_unit;
    trans[ST_hundred_loaded][TK_unit_teen]=ST_hundred_unit;
    trans[ST_hundred_loaded][TK_decimal]=ST_hundred_decimal;
    trans[ST_hundred_decimal][TK_unit]=ST_hundred_decimal_unit;
    trans[ST_number][TK_num]=ST_number;

    // transitions from initial state to quantifiers
    trans[ST_num_init_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_minus_loaded_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_zero_loaded_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_unit_loaded_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_unit_teen_loaded_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_decimal_loaded_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_decimal_unit_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_hundred_loaded_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_hundred_zero_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_hundred_unit_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_hundred_decimal_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_hundred_decimal_unit_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_special_start][TK_millrd]=ST_num_init_millrd;
    trans[ST_number_start][TK_millrd]=ST_num_init_millrd;

    trans[ST_num_init_start][TK_mill]=ST_num_init_mill;
    trans[ST_minus_loaded_start][TK_mill]=ST_num_init_mill;
    trans[ST_zero_loaded_start][TK_mill]=ST_num_init_mill;
    trans[ST_unit_loaded_start][TK_mill]=ST_num_init_mill;
    trans[ST_unit_teen_loaded_start][TK_mill]=ST_num_init_mill;
    trans[ST_decimal_loaded_start][TK_mill]=ST_num_init_mill;
    trans[ST_decimal_unit_start][TK_mill]=ST_num_init_mill;
    trans[ST_hundred_loaded_start][TK_mill]=ST_num_init_mill;
    trans[ST_hundred_zero_start][TK_mill]=ST_num_init_mill;
    trans[ST_hundred_unit_start][TK_mill]=ST_num_init_mill;
    trans[ST_hundred_decimal_start][TK_mill]=ST_num_init_mill;
    trans[ST_hundred_decimal_unit_start][TK_mill]=ST_num_init_mill;
    trans[ST_special_start][TK_mill]=ST_num_init_mill;
    trans[ST_number_start][TK_mill]=ST_num_init_mill;

    trans[ST_num_init_start][TK_thous]=ST_num_init_thous;
    trans[ST_minus_loaded_start][TK_thous]=ST_num_init_thous;
    trans[ST_zero_loaded_start][TK_thous]=ST_num_init_thous;
    trans[ST_unit_loaded_start][TK_thous]=ST_num_init_thous;
    trans[ST_unit_teen_loaded_start][TK_thous]=ST_num_init_thous;
    trans[ST_decimal_loaded_start][TK_thous]=ST_num_init_thous;
    trans[ST_decimal_unit_start][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_loaded_start][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_zero_start][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_unit_start][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_decimal_start][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_decimal_unit_start][TK_thous]=ST_num_init_thous;
    trans[ST_special_start][TK_thous]=ST_num_init_thous;
    trans[ST_number_start][TK_thous]=ST_num_init_thous;

    // glue transitions between quantifiers, number AFTER quantifier word
    // milliard glues to million, thousand and unit, milion to thousand and unit, ...
    // for "millrd"
    // millrd -> mill
    trans[ST_num_init_millrd][TK_mill]=ST_num_init_mill;
    trans[ST_zero_loaded_millrd][TK_mill]=ST_num_init_mill;
    trans[ST_unit_loaded_millrd][TK_mill]=ST_num_init_mill;
    trans[ST_unit_teen_loaded_millrd][TK_mill]=ST_num_init_mill;
    trans[ST_decimal_loaded_millrd][TK_mill]=ST_num_init_mill;
    trans[ST_decimal_unit_millrd][TK_mill]=ST_num_init_mill;
    trans[ST_hundred_loaded_millrd][TK_mill]=ST_num_init_mill;
    trans[ST_hundred_zero_millrd][TK_mill]=ST_num_init_mill;
    trans[ST_hundred_unit_millrd][TK_mill]=ST_num_init_mill;
    trans[ST_hundred_decimal_millrd][TK_mill]=ST_num_init_mill;
    trans[ST_hundred_decimal_unit_millrd][TK_mill]=ST_num_init_mill;
    trans[ST_special_millrd][TK_mill]=ST_num_init_mill;
    trans[ST_number_millrd][TK_mill]=ST_num_init_mill;

    // millrd -> thous
    trans[ST_num_init_millrd][TK_thous]=ST_num_init_thous;
    trans[ST_zero_loaded_millrd][TK_thous]=ST_num_init_thous;
    trans[ST_unit_loaded_millrd][TK_thous]=ST_num_init_thous;
    trans[ST_unit_teen_loaded_millrd][TK_thous]=ST_num_init_thous;
    trans[ST_decimal_loaded_millrd][TK_thous]=ST_num_init_thous;
    trans[ST_decimal_unit_millrd][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_loaded_millrd][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_zero_millrd][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_unit_millrd][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_decimal_millrd][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_decimal_unit_millrd][TK_thous]=ST_num_init_thous;
    trans[ST_special_millrd][TK_thous]=ST_num_init_thous;
    trans[ST_number_millrd][TK_thous]=ST_num_init_thous;

    // for "mill"
    // mill -> thous
    trans[ST_num_init_mill][TK_thous]=ST_num_init_thous;
    trans[ST_zero_loaded_mill][TK_thous]=ST_num_init_thous;
    trans[ST_unit_loaded_mill][TK_thous]=ST_num_init_thous;
    trans[ST_unit_teen_loaded_mill][TK_thous]=ST_num_init_thous;
    trans[ST_decimal_loaded_mill][TK_thous]=ST_num_init_thous;
    trans[ST_decimal_unit_mill][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_loaded_mill][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_zero_mill][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_unit_mill][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_decimal_mill][TK_thous]=ST_num_init_thous;
    trans[ST_hundred_decimal_unit_mill][TK_thous]=ST_num_init_thous;
    trans[ST_special_mill][TK_thous]=ST_num_init_thous;
    trans[ST_number_mill][TK_thous]=ST_num_init_thous;

    // for "thous"
    // for ""
    // nothing else expected

    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int numbers_cs::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
    wstring form;
    int token;
    map<wstring,int>::const_iterator im;
 
    form = j->get_lemma();
    if(form == L"")
      form = j->get_lc_form();
    
    token = TK_other;
    im = tok.find(form);
    if (im!=tok.end()) {
      token = (*im).second;
    }
  
    TRACE(3,L"Next word form is: ["+form+L"] token="+util::int2wstring(token));     
    // if the token was in the table, we're done
    if (token != TK_other) return (token);

    // Token not found in translation table, let's have a closer look.

    // check to see if it is a number or an alphanumeric code
    // negative number (-17 000) is signed only in the beginning
    if ((state == ST_num_init_start) && (RE_number_neg.search(form))) token = TK_num;
    else if (RE_number.search(form)) token = TK_num;
    else if (RE_code.search(form)) token = TK_code;

    TRACE(3,L"Leaving state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)); 
    return (token);
  }



  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state, update current 
  ///  nummerical value.
  ///////////////////////////////////////////////////////////////

  void numbers_cs::StateActions(int origin, int state, int token, sentence::const_iterator j, numbers_status *st) const {
    wstring form;
    size_t i;
    long double num;
    int is_negative = 0;
  
    form = j->get_lemma();
    if(form == L"")
      form = j->get_lc_form();

    TRACE(3,L"Reaching state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)+L" for word ["+form+L"]");

    // get token numerical value, if any
    num=0;
    if (token==TK_num) {
      // erase thousand points
      while ((i=form.find_first_of(MACO_Thousand))!=wstring::npos) {
        form.erase(i,1);
      }
      TRACE(3,L"after erasing thousand "+form);
      // make sure decimal point is L"."
      if ((i=form.find_last_of(MACO_Decimal))!=wstring::npos) {
        form[i]=L'.';
        TRACE(3,L"after replacing decimal "+form);
      }
      num = util::wstring2longdouble(form);
    }
  
    // BY ME FROM HERE

    if(st->units < 0 || st->milion < 0 || st->bilion < 0 || origin==ST_minus_loaded_start)
      {
        is_negative = 1;
        TRACE(3,L"number is negative");
      }
  
    st->units = fabs(st->units);
    st->milion = fabs(st->milion);
    st->bilion = fabs(st->bilion);

    // multiply by 1000 (like "17 000")
    if ((origin==ST_number_start || origin==ST_number_millrd || origin==ST_number_mill ||
         origin==ST_number_thous || origin==ST_number) && token == TK_num)
      {
        st->units *= power.find(TK_thous)->second;
      }

    // Numerical representation, add to units
    if (token==TK_num) st->units += num;
  
    // quantifier in the very beginning of the number or after minus word, silent 'one'
    if ((origin==ST_num_init_start || origin==ST_minus_loaded_start || origin==ST_num_init_millrd ||
         origin==ST_num_init_mill || origin==ST_num_init_thous ||
         origin==ST_num_init ) && 
        (token==TK_hundred || token == TK_thous || token == TK_mill || token == TK_millrd)) {
      st->units += 1;
      TRACE(3,L"Silent 'one' detected...");
    }
  
    if (token == TK_hundred || token==TK_thous || token==TK_mill || token==TK_millrd) {
      long long to_multiply = (long long) st->units % (long long) power.find(token)->second;
    
      st->units -= to_multiply;
      st->units += to_multiply * (int) power.find(token)->second;
      TRACE(3,L"Quantifier value="+util::longdouble2wstring(power.find(token)->second));
      TRACE(3,L"Quantifier processed. bilion="+util::longdouble2wstring(st->bilion)+L" milion="+util::longdouble2wstring(st->milion)+L" units="+util::longdouble2wstring(st->units));
    }
  
    // Milliard first
    // split number if bigger than billiard (long scale)
    if (((int) st->units / (int) power.find(TK_millrd)->second) > 0) {
      st->bilion += ((int) st->units / (int) power.find(token)->second) * (int) power.find(token)->second;
      st->units -=  st->bilion;
      TRACE(3,L"Milliard (long scale) split...");
    }
    // split number if bigger than million
    if (((int) st->units / (int) power.find(TK_mill)->second) > 0) {
      st->milion += ((int) st->units / (int) power.find(token)->second) * (int) power.find(token)->second;
      st->units -=  st->milion;
      TRACE(3,L"Million split...");
    }
  
    // other standard numbers
    if (token==TK_unit || token==TK_unit_teen || token==TK_decimal || token==TK_decimal_unit_alternative
        || token==TK_special)
      {
        st->units += value.find(form)->second;
      }
  
    // don't want negative 0
    if (is_negative) {
      if (st->units != 0) st->units *= -1;
      if (st->milion != 0) st->milion *= -1;
      if (st->bilion != 0) st->bilion *= -1;
    }
  

    /* TODO:
       case ST_COD:
       TRACE(3,L"Actions for COD state");    
       if (token==TK_code) st->iscode=CODE;
       else if (token==TK_ord) st->iscode=ORD;
       break;
       // ---------------------------------
       default: break;
       }
    */
    TRACE(3,L"State actions completed. bilion="+util::longdouble2wstring(st->bilion)+L" milion="+util::longdouble2wstring(st->milion)+L" units="+util::longdouble2wstring(st->units));
  }


  ///////////////////////////////////////////////////////////////
  ///   Set the appropriate lemma and tag for the 
  ///   new multiword.
  ///////////////////////////////////////////////////////////////

  void numbers_cs::SetMultiwordAnalysis(sentence::iterator i, int fstate, const numbers_status *st) const {
    wstring lemma, tag;

    tag=L"Z";

    // Setting the analysis for the nummerical expression
    if (st->iscode==CODE) {
      lemma=i->get_form();
    }
    else {
      // compute nummerical value as lemma
      lemma=util::longdouble2wstring(st->bilion + st->milion + st->units);
    }

    i->set_analysis(analysis(lemma,tag));
    TRACE(3,L"Analysis set to: "+lemma+L" "+tag);

    // record this word was analyzed by this module
    i->set_analyzed_by(word::NUMBERS);    

  }
} // namespace
