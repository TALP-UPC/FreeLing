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
//  freeling_rubyAPI.i
//  This is the SWIG input file, used to generate ruby APIs.
//
////////////////////////////////////////////////////////////////

%module freeling
%{
 #include "freeling.h"
 #include "freeling/io.h"
 #include "freeling/tree.h"
 #include "freeling/morfo/traces.h"
 using namespace std;
%}

%include std_wstring.i
%include std_set.i

%include <typemaps/cwstring.swg>
%include <typemaps/std_wstring.swg>

// `SetString` is also defined in `freeling.i`, but it needs to be defined earlier, otherwise
// SWIG will complain about `make_set_nonconst_iterator` not having been defined yet.
%template(SetString) std::set<std::wstring>;

#define FL_API_RUBY
%include ../common/templates.i
%include ../common/freeling.i
