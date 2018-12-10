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


%fragment("SWIG_AsWCharPtrAndSize","header",fragment="<wchar.h>",fragment="SWIG_pwchar_descriptor") {
SWIGINTERN int SWIG_AsWCharPtrAndSize(VALUE obj, wchar_t **cptr, size_t *psize, int *alloc)
{
  if (TYPE(obj) == T_STRING) {
    %#if defined(StringValuePtr)
                  char *cstr = StringValuePtr(obj);
    %#else
       char *cstr = STR2CSTR(obj);
    %#endif

       std::wstring tempStr = freeling::util::string2wstring(cstr);

    size_t size = tempStr.size() + 1;
    if (cptr)  {
      if (alloc) {                
        *cptr = %new_copy_array(tempStr.c_str(), size, wchar_t);
        *alloc = SWIG_NEWOBJ;
      }
    }
    if (psize) *psize = size;
    return SWIG_OK;
  } else {
    swig_type_info* pwchar_descriptor = SWIG_pwchar_descriptor();
    if (pwchar_descriptor) {
      void* vptr = 0;
      if (SWIG_ConvertPtr(obj, &vptr, pwchar_descriptor, 0) == SWIG_OK) {
        if (cptr) *cptr = (wchar_t *)vptr;
        if (psize) *psize = vptr ? (wcslen((wchar_t*)vptr) + 1) : 0;
        if (alloc) *alloc = SWIG_OLDOBJ;
        return SWIG_OK;
      }
    }
  }  
  return SWIG_TypeError;
}
 }

%fragment("SWIG_FromWCharPtrAndSize","header",fragment="<wchar.h>",fragment="SWIG_pwchar_descriptor") {
SWIGINTERNINLINE VALUE
  SWIG_FromWCharPtrAndSize(const wchar_t * carray, size_t size)
{
  if (carray) {
    std::string tempStr(freeling::util::wstring2string(carray));

    if (tempStr.size() > LONG_MAX) {
      swig_type_info* pwchar_descriptor = SWIG_pwchar_descriptor();
      return pwchar_descriptor ?
        SWIG_NewPointerObj(%const_cast(carray,wchar_t *), pwchar_descriptor, 0) : Qnil;
    } else {
      return rb_str_new(tempStr.c_str(), %numeric_cast(tempStr.size(),long));
    }
  } else {
    return Qnil;
  }
}
 }

%include <typemaps/cwstring.swg>
%include <typemaps/std_wstring.swg>

%include ../common/templates.i
%include ../common/freeling.i
