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
//  freeling_javaAPI.i
//  This is the SWIG input file, used to generate java APIs.
//
////////////////////////////////////////////////////////////////

%module Jfreeling
%{
 #include "freeling.h"
 #include "freeling/io.h"
 #include "freeling/tree.h"
 #include "freeling/morfo/traces.h"
 using namespace std;
%}

%include std_wstring.i

%rename(operator_equal) operator==;
%rename(operator_notequal) operator!=;
%rename(operator_increment) operator++;
%rename(operator_decrement) operator--;
%rename(operator_lessthan) operator<;
%rename(operator_morethan) operator>;
%rename(operator_access) operator[];
%rename(operator_content) operator*;
%rename(operator_deref) operator->;

%rename get_info getInformation;
%rename set_inverse_dict setInverseDictionary;
%rename set_retok_contractions setRetokenizeContractions;

%rename("%(lowercamelcase)s", %$isfunction) "";
%rename("%(camelcase)s", %$isclass) "";
%rename("%(camelcase)s", %$isenum) "";

#define FL_API_JAVA
%include ../common/templates.i
%include ../common/freeling.i
