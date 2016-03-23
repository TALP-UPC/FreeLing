//////////////////////////////////////////////////////////////////
//
//    Treeler - Open-source Structured Prediction for NLP
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
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//                          02111-1307 USA
//
//    contact: Xavier Carreras (carreras@lsi.upc.es)
//             TALP Research Center
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////
#ifndef TREELER_IO_CLASS
#define TREELER_IO_CLASS

#include <iostream>
#include <string>
#include "treeler/base/dataset.h"
#include "treeler/class/class-basic.h"


namespace treeler {

  class IOClass {
  public:
    static void process_options(); 

    //!
    static void read(const std::string& file, DataSet<ClassPattern, Label<PartClass>>& ds);
    
    //!
    static void read(std::istream& iii, DataSet<ClassPattern, Label<PartClass>>& ds);    

    //! \brief Reads a single example from the input stream. If it can
    //  be read, it returns true and x and y point to the pair. Note that 
    //  x and y are references to pointers. 
    //  If it can not be read, it returns false 
    //  and x and y are undetermined.
    static bool read(std::istream& iii, ClassPattern*& x, Label<PartClass>*& y);    


    //! deprecated interface
    static void read_dataset(const std::string& file, DataSet<ClassPattern, Label<PartClass>>& ds) {
      read(file, ds); 
    }
    
    //! deprecated interface
    static void read_dataset(std::istream& iii, DataSet<ClassPattern, Label<PartClass>>& ds) {
      read(iii, ds); 
    }

    static void write(std::ostream& o, const ClassPattern& x); 
    static void write(std::ostream& o, const Label<PartClass>& y); 
    static void write(std::ostream& o, const ClassPattern& x, const Label<PartClass>& y); 
    static void write(std::ostream& o, const ClassPattern& x, const Label<PartClass>& y, const Label<PartClass>& yhat); 
  };



}

#endif
