/*
 * Phonetic Distance Scorer for the PHAST package
 * + See the features.ALL file for input description
 * + April 2006 pcomas@lsi.upc.edu
 *
 */
#ifndef _phd_h
#define _phd_h

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <math.h>

#include "freeling/morfo/util.h"

namespace freeling {

#define ALPHSIZE 128

  template <typename T=int> 
    class phd {

  private:
  T csub, cexp, cvowel, cskip, cspace;
  T distance[ALPHSIZE][ALPHSIZE];
  std::set<wchar_t> svowels;      // set of vowel phonemes
  std::set<wchar_t> sconsonants;  // set of consonant phonemes
  int debug;

  inline T V(wchar_t a){ return svowels.find(a) != svowels.end() ? cvowel : 0; }

  public:

  phd(const std::wstring &fname){

    debug = 0;
    std::wstring s;
    wchar_t c;
    T t;
    int i;
    std::map<const std::wstring, int> flist;   // set of the features' names with its index
    std::map<const std::wstring, T> fweight; // set of the features' saliences
    std::map<const std::wstring, T> values;  // set of the numerical values of the multivaluated features
    std::set<std::wstring> svfeatures;  // set of attributes for vowel comparison
    std::set<std::wstring> scfeatures;  // set of attributes for other comparisons
    csub = 0;
    cskip = 0;
    cexp = 0;
    cvowel = 0;

    /**************************************************************
     *
     * READ INPUT FILES, BUILD MATRIX OF FEATURES
     *
     **************************************************************/

    T features [ALPHSIZE][ALPHSIZE];

    std::wifstream is;
    util::open_utf8_file(is,fname);
    if (is.fail()) {
      std::wcerr<<L"PHONETIC_DISTANCE: Error opening file "+fname;
      exit(1);
    }

    while(!is.eof()){

      is >> s;

      if( s[0] == L'#'){ 
        getline(is,s);

      } else if( s==L"FON:") {
        is >> c;     // this is the phoneme
        //cerr << "FONEMA "<< c << endl;
        getline(is,s); 
        std::wstringstream ss(s,std::stringstream::in);
        i = 0;
        while(ss>>s){
          if(s==L"+"){
            features[(int)c][i] = 100;
          }else if(s==L"-"){
            features[(int)c][i] = 0;
          }else{  // is a multivaluated feature
            features[(int)c][i] = values[s];
          }
          //cerr << "Posant " << features[c][i] << " a " << i << " (" << s << ")"<< endl;
          i++;
        }

      } else if( s==L"VALUE:") {
        is >> s >> t; // feature value is i
        values[s] = t;
        //cerr << "VALUE ADD: " << s << " <-- " << i << endl;

      } else if( s==L"WEIGHT:") {
        is >> s >> t; // feature s weights i
        fweight[s] = t;

      } else if( s==L"CONSTANT:") {
        is >> s >> t; // s takes value i
        if (s==L"Cskip")   { cskip = t;}
        else if(s==L"Csub"){ csub  = t;}
        else if(s==L"Cexp"){  cexp = t;}
        else if(s==L"Cvowel"){ cvowel = t;}
        else if(s==L"Cspace"){ cspace = t;}
        else{ std::wcerr << L"UNEXPECTED CONSTANT DEFINITION" << s << std::endl; }

      } else if( s==L"VOWELS:") {
        //create a list with the vocalic phonemes
        getline(is,s); 
        std::wstringstream ss(s, std::wstringstream::in);
        while( ss>>c ){  svowels.insert(c); }

      } else if( s==L"CONSONANTS:") {
        //create a set with the consonantic phonemes
        getline(is,s); 
        std::wstringstream ss(s, std::wstringstream::in);
        while( ss>>c ){  sconsonants.insert(c); }

      } else if( s==L"FEATURES:") {
        //create a list with the index inside the matrix for each feature
        getline(is,s); 
        std::wstringstream ss(s, std::wstringstream::in);
        i = 0;
        while( ss>>s ){ flist[s]=i; i++; }

      } else if( s==L"FVOWELS:") {
        //create a set with 
        getline(is,s); 
        std::wstringstream ss(s, std::wstringstream::in);
        while( ss>>s ){ svfeatures.insert(s); }

      } else if( s==L"FOTHER:") {
        //create a set with 
        getline(is,s); 
        std::wstringstream ss(s, std::wstringstream::in);
        while( ss>>s ){ scfeatures.insert(s); }

      } else {
        //skip
      }
      
    }
    
    is.close();


    /**************************************************************
     *
     * BUILD MATRIX OF DISTANCES
     *
     **************************************************************/
    /*
     */
    
    std::set<wchar_t>::iterator it1;
    std::set<wchar_t>::iterator it2;
    std::set<std::wstring>::iterator it3;
    T d;
    int f;

    for(int i=0;i<ALPHSIZE;i++){
      for(int j=0;j<ALPHSIZE;j++){
        distance[i][j]= i==j ? 0 : (T)8000;
      }
    }

    //Build vowels vs vowels

    for( it1 = svowels.begin(); it1!=svowels.end(); ++it1){
      for( it2 = svowels.begin(); it2!=it1; ++it2){
        //calculate distance between it1 and it2 using features in it3
        d=0;
        for(it3 = svfeatures.begin(); it3!=svfeatures.end(); ++it3){
          f = flist[(*it3)];
          d += abs( features[(int)(*it1)][(int)f] - features[(int)(*it2)][(int)f] ) * fweight[(*it3)];
        }
        distance[(int)(*it1)][(int)(*it2)] = d;
        distance[(int)(*it2)][(int)(*it1)] = d;
      }
    }


    //Build vowels vs consonants
    for( it2 = sconsonants.begin(); it2!=sconsonants.end(); ++it2){
      for( it1 = svowels.begin(); it1!=svowels.end(); ++it1){
        //calculate distance between it1 and it2 using features in it3
        d=0;
        for(it3 = scfeatures.begin(); it3!=scfeatures.end(); ++it3){
          f = flist[(*it3)];
          d += abs( features[(int)(*it1)][(int)f] - features[(int)(*it2)][(int)f] ) * fweight[(*it3)];
        }
        distance[(int)(*it1)][(int)(*it2)] = d;
        distance[(int)(*it2)][(int)(*it1)] = d;
      }
    }


    //Build consonants vs consonants
    for( it1 = sconsonants.begin(); it1!=sconsonants.end(); ++it1){
      for( it2 = sconsonants.begin(); it2!=it1; ++it2){
        //calculate distance between it1 and it2 using features in it3
        d=0;
        for(it3 = scfeatures.begin(); it3!=scfeatures.end(); ++it3){
          f = flist[(*it3)];
          d += abs( features[(int)(*it1)][(int)f] - features[(int)(*it2)][(int)f] ) * fweight[(*it3)];
        }
        distance[(int)(*it1)][(int)(*it2)] = d;
        distance[(int)(*it2)][(int)(*it1)] = d;
      }
    }

    if(debug>2){
      std::wcerr << L"\t";
      for( int i=85; i<ALPHSIZE; i++ ){
        std::wcerr << (wchar_t)i << L"\t";
      }
      std::wcerr << std::endl;

      for( int i=85; i<ALPHSIZE; i++ ){
        std::wcerr << (wchar_t)i << L"\t";
        for( int j=85; j<ALPHSIZE; j++ ){
          std::wcerr << distance[i][j] << L"\t";
        }
        std::wcerr << std::endl;
      }

    }


  } //constructor


  void show(std::wostream &o){

    std::set<wchar_t>::iterator it1;
    std::set<wchar_t>::iterator it2;
    std::set<std::wstring>::iterator it3;

    o << L"Distances between phonemes" << std::endl << L"==========================" << std::endl << std::endl;

    o << L"Read values: cskip:" << cskip << L", csub:" << csub << L", cexp:" << cexp << L", cvowel:" << cvowel << std::endl;


    o << L"\t";
    for( it1 = svowels.begin(); it1!=svowels.end(); ++it1) o << (*it1) << L"\t";
    o << std::endl;

    for( it1 = svowels.begin(); it1!=svowels.end(); ++it1){
      o << (*it1) << L"\t";
      for( it2 = svowels.begin(); it2!=it1; ++it2){
        o << distance[(int)(*it1)][(int)(*it2)] << L"\t";
      }
      o << std::endl;
    }

    o << std::endl << L"\t";
    for( it1 = svowels.begin(); it1!=svowels.end(); ++it1) o << (*it1) << L"\t";
    o << std::endl;

    // vowels vs consonants
    for( it2 = sconsonants.begin(); it2!=sconsonants.end(); ++it2){
      o << (*it2) << L"\t";
      for( it1 = svowels.begin(); it1!=svowels.end(); ++it1){
        o << distance[(int)(*it1)][(int)(*it2)] << L"\t";
      }
      o << std::endl;
    }

    o << std::endl << L"\t";
    for( it1 = sconsonants.begin(); it1!=sconsonants.end(); ++it1) o << (*it1) << L"\t";
    o << std::endl;

    // consonants vs consonants
    for( it1 = sconsonants.begin(); it1!=sconsonants.end(); ++it1){
      o << (*it1) << L"\t";
      for( it2 = sconsonants.begin(); it2!=it1; ++it2){
        o << distance[(int)(*it1)][(int)(*it2)] << L"\t";
      }
      o << std::endl;
    }
  }


  T getCskip(){
    return cskip;
  }

  T dSkip(int c){
    return c==L' ' || c==L'_' ? cskip+cspace : cskip;
    //return cskip;
  }

  T dSub(int const a, int const b){
    if( ( (wchar_t)a==L' ' || (wchar_t)a==L'_' ) && ( (wchar_t)b==L' ' || (wchar_t)b==L'_' ) ){ return cspace; }
    return (wchar_t)a==L'_' || (wchar_t)a==L' ' || (wchar_t)b==L' ' || (wchar_t)b==L'_' ? -cspace/2 : csub - distance[a][b] - V(a) - V(b);
  }

  T dExp(int const a, int const b, int const c){
    return cexp - distance[a][b] - distance[a][c] - V(a) - std::max(V(b),V(c));
  }
  
  };

} // namespace

#endif
