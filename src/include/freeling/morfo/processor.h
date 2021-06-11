////////////////////////////////////////////////////////////////
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

#ifndef _PROCESSOR
#define _PROCESSOR

#include <set>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/analyzer_config.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///
  ///  Abstract class to define the common API of any FreeLing processing module.
  ///  Template parameter is base class that the procesor deals with (e.g. sentence or document).
  ///  Instance need to define basic analyze(T) method, and inherit methods to process 
  ///  list<T> and to return analyzed copies instead of modifying input parameter.
  //////////////////////////////////////////////////////////////////

  class WINDLL processor {
  public:
    /// constructor
    processor() {};
    /// destructor
    virtual ~processor() {};

    /// analyze sentence/document. Pure virtual, must be provided by instance
    virtual void analyze(sentence &) const = 0;

    /// analyze sentence/document with given options. Should be provided by instance if needed.
    /// Default behaviour ignores provided options.
    virtual void analyze(sentence &s, const analyzer_config::invoke_options &opt) const {
      analyze(s);
    };

    /// analyze list of sentences (paragraph)
    virtual void analyze(std::list<sentence> &ls) const {
      std::list<sentence>::iterator is;
      for (is=ls.begin(); is!=ls.end(); is++) {
        analyze(*is);    
      }
    }

    /// analyze list of sentences (paragraph) with given options
    virtual void analyze(std::list<sentence> &ls, const analyzer_config::invoke_options &opt) const {
      std::list<sentence>::iterator is;
      for (is=ls.begin(); is!=ls.end(); is++) {
        analyze(*is, opt);    
      }
    }

    /// analyze document
    virtual void analyze(document &doc) const {
      document::iterator ip;
      for (ip=doc.begin(); ip!=doc.end(); ip++) {
        analyze(*ip);
      }
    }

    /// analyze document with given options
    virtual void analyze(document &doc, const analyzer_config::invoke_options &opt) const {
      document::iterator ip;
      for (ip=doc.begin(); ip!=doc.end(); ip++) {
        analyze(*ip, opt);
      }
    }

    /// analyze sentence, return analyzed copy
    virtual sentence analyze(const sentence &s) const {
      sentence s2=s;
      analyze(s2);    
      return s2;
    }

    /// analyze sentence with given options, return analyzed copy
    virtual sentence analyze(const sentence &s, const analyzer_config::invoke_options &opt) const {
      sentence s2=s;
      analyze(s2, opt);    
      return s2;
    }

    /// analyze list of sentences, return analyzed copy
    virtual std::list<sentence> analyze(const std::list<sentence> &ls) const {
      std::list<sentence> l2=ls;
      analyze(l2);
      return l2;
    }

    /// analyze list of sentences with given options, return analyzed copy
  virtual std::list<sentence> analyze(const std::list<sentence> &ls, const analyzer_config::invoke_options &opt) const {
      std::list<sentence> l2=ls;
      analyze(l2, opt);
      return l2;
    }

    /// analyze document, return analyzed copy
    virtual document analyze(const document &d) const {
      document d2=d;
      analyze(d2);
      return d2;
    }

    /// analyze document with given options, return analyzed copy
    virtual document analyze(const document &d, const analyzer_config::invoke_options &opt) const {
      document d2=d;
      analyze(d2, opt);
      return d2;
    }

  };
} // namespace

#endif
