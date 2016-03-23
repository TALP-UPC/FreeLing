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
//    version 2.1 of the License, or (at your option) any later version.
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

////////////////////////////////////////////////////////////////
//
//  freeling_perlAPI.i
//  This is the SWIG input file, used to generate perl API.
//
////////////////////////////////////////////////////////////////

%module freeling

%{
 // avoid clashes between macros in perl.h and std::algorithm
 // in perl 5.18 and g++ 4.8 
 #undef seed
 #undef do_close
 #undef do_open

 #include "freeling.h"
 #include "freeling/io.h"
 #include "freeling/tree.h"
 #include "freeling/morfo/traces.h"
 using namespace std;
%}

%include ../common/templates.i

### Typemaps ###

%typemap(in) const std::wstring & (std::wstring wtemp)  {
  std::string aux (SvPV($input, PL_na));
  wtemp = freeling::util::wstring_from(aux);
  $1 = &wtemp;
}

%typemap(in) std::wstring (std::wstring wtemp) {
  std::string aux (SvPV($input, PL_na));
  wtemp = freeling::util::wstring_from(aux);
  $1 = wtemp;
}

%typemap(out) const std::wstring & {
  std::string temp;
  temp = freeling::util::wstring_to<std::string>($1);
  $result = sv_2mortal(newSVpv(temp.c_str(), 0));
  argvi++;
  SvUTF8_on ($result);
} 

%typemap(out) std::wstring = const std::wstring &;
%typemap(typecheck) const std::wstring & = char *;

%typemap(out) std::list< std::wstring > {
  std::list<std::wstring>::const_iterator i;
  unsigned int j;
  int len = (& $1)->size();
  SV **svs = new SV*[len];
  for (i=(& $1)->begin(), j=0; i!=(& $1)->end(); i++, j++) {
    std::string ptr = freeling::util::wstring_to<std::string>(*i);
    svs[j] = sv_2mortal(newSVpv(ptr.c_str(), 0));
    SvUTF8_on(svs[j]);
  }
  AV *myav = av_make(len, svs);
  delete[] svs;
  $result = newRV_noinc((SV*) myav);
  sv_2mortal($result);
  argvi++;
}



#define FL_API_PERL
%include ../common/freeling.i
