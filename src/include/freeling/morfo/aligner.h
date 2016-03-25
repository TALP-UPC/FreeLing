/*
 * String aligner for the Phonetic Distance Package
 * + 
 * + April 2006 pcomas@lsi.upc.edu
 *
 */
#ifndef _aligner_h
#define _aligner_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <string>
#include <math.h>

#include "freeling/morfo/phd.h"

#define GLOBAL    1  // The aligner produces local alignments
#define SEMILOCAL 2  // The aligner produces semilocal alignments
#define LOCAL     3  // The aligner produces global alignments

namespace freeling {

  template<typename T=int> 
    class aligner{

  private:
  phd<T>* sc;
  T score;
  int debug;

  public:

  struct alin {
    T score;   // score of this alignment
    double scoren; // score normalized
    double context;
    int Psubstitutions;
    int Pinserts;
    int Pdeletions;
    int Wsubstitutions;
    int Winserts;
    int Wdeletions;
    unsigned int kword; //The number of keyword involved in this alignment
    int begin; // index in b[] where the alignment starts
    int end;   // index in b[] where the alignment ends
    int beginW; //number of word in b[] where the alignment starts (each ' ' changes the word)
    int endW;   //number of word in b[] where the alignment ends
    wchar_t* seg; // Segment of b[] with the letters
    wchar_t* a;   // Alignment of a
    wchar_t* b;   // Alignment of b
    bool good; // Marks if this alignment is selected as relevant

    ~alin(){
      delete[](a);
      delete[](b);
      delete[](seg);
    }

  alin(T _score, int _begin, int _end, int _beginW, int _endW, int _substitutions, int _inserts, int _deletions, wchar_t* _a, wchar_t* _b) :
    score(_score), Psubstitutions(_substitutions), Pinserts(_inserts), Pdeletions(_deletions), begin(_begin), end(_end), beginW(_beginW), endW(_endW), seg(NULL), a(_a), b(_b) {}
  };


  aligner(const std::wstring &fname, int const _debug = 0){
    sc = new phd<T>(fname);
    debug=_debug;
    // if(debug>3){ sc->show(cerr);}
  } //constructor

  ~aligner(){
    delete(sc);
  }


  /*
   * Alinia la cadena A contra la cadena B. Conceptualment es considera que A és la query
   */
  alin* align(const std::wstring &wa, const std::wstring &wb, const int mode = SEMILOCAL){
    // a is the short string with length tj
    // b is the long string with length ti
    // The algorithm searchs for the best match of a against b in a semi-local or global point of view.
    // Usage of global matching is discouraged for the sake of coherence :-) because it doesn't try
    // to keep the letters together so 'a' will be meaningless scattered through all 'b'

    const wchar_t *a=wa.c_str(); const int tj=wa.size();
    const wchar_t *b=wb.c_str(); const int ti=wb.size();

    /*
    //  Build the align matrix
    */
    int const W = ti+1;

    if( ti==0 || tj == 0 ){ return new alin(0,0,0,0,0,0,0,0,0,0); }
    int i,j;
    int* m = new int[W*(tj+1)+ti+1]; //int m[tj+1][ti+1];

    // Variables for alignment reconstruction
    wchar_t* answerA = new wchar_t[tj+ti];
    wchar_t* answerB = new wchar_t[tj+ti];
    int pA = 0;
    int pB = 0;
    int insertions=0, deletions=0, substitutions=0;
    int spacesA=0;


    // INITIALIZATION STEP

    // Decide the number of word that each character belongs to
    int nwords=0;
    for(int i=0; i<ti; i++){ 
      if( b[i] == L' ' || b[i] == L'_' ){
        // word change at _i_
        nwords++;
      }
    }

    int* words = new int[nwords];
    j=0;
    for(int i=0; i<ti; i++){ 
      if( b[i] == L' ' || b[i] == L'_' ){
        words[j++] = i;
      }
    }

    switch(mode){
    case GLOBAL:  //Start values are skips
    m[0]=0;
    for(int j=1; j<=tj; j++){ m[j*W] = m[(j-1)*W] + sc->dSkip(a[j-1]); }
    for(int i=1; i<=ti; i++){ m[i]   = m[i-1]     + sc->dSkip(b[i-1]); }
    break;
    default: //Start values are 0
    for(int j=0;j<=tj;j++){ m[j*W] = 0; }
    for(int i=1;i<=ti;i++){ m[i] = 0; }
    break;
    }


    /*
    // Throw the scoring process
    */
    int i1, i2, i3, i4, i5, indexInit, indexEnd, bestJ, bestI;
    indexEnd  = 0;
    indexInit = 0;
    int initWord  = 0;
    int endWord   = 0;
    score = -100000000;
    bestJ = tj;
    bestI = ti;

    // FILL THE FIRST COLUMN & ROW
    for(j=1;j<=tj;j++){
      m[j*W+1] = std::max(  m[W*(j-1)]+sc->dSub(a[j-1],b[0]) , m[W*(j-1)+1]+sc->dSkip(a[j-1]) );
      if(a[j-1]==L' '|| a[j-1]==L'_' ){spacesA++;}
      if(score < m[W*j+1]){
        score = m[W*j+1];
        bestJ = j;
        bestI = 0;
      }
    }
    for(i=1;i<=ti;i++){
      m[ W+i ] = std::max(  m[ i-1 ]+sc->dSub(a[0],b[i-1])  ,  m[ W+i-1 ]+sc->dSkip(b[i-1])  );
      if(score < m[W+i]){
        score = m[W+i];
        bestJ = 0;
        bestI = i;
      }
    }

    //FILL THE REST OF THE MATRIX
    //int step = ti/4;
    int lowLimit = 0;
    int upperLimit = 0;
    int threshold = -10000000;
    if( mode == LOCAL ){ threshold = 0; }

    for(j=2;j<=tj;j++){
      //if( mode==GLOBAL && tj/ti> 0.75){
      // // If the scoring is GLOBAL we do not fill the entire matrix
      // lowLimit   = max(  2 , j-step );
      // upperLimit = min( ti , j+step );
      // m[W*j+lowLimit+1] = 0;
      //      } else {
      lowLimit   = 2;
      upperLimit = ti;
      //      }

      for(i=lowLimit;i<=upperLimit;i++){
        i1 = m[W*(j-1)+i-1] + sc->dSub(a[j-1],b[i-1]);
        i2 = m[W*j+i-1] + sc->dSkip(b[i-1]);
        i3 = m[W*(j-1)+i-2] + sc->dExp(a[j-1],b[i-2],b[i-1]);
        i4 = m[W*(j-1)+i] + sc->dSkip(a[j-1]);
        i5 = m[W*(j-2)+i-1] + sc->dExp(b[i-1],a[j-2],a[j-1]);
        m[W*j+i] = std::max(threshold,std::max(i1,std::max(i1,std::max(i2,std::max(i3,std::max(i4,i5))))));

        if(score < m[W*j+i]){
          score = m[W*j+i];
          bestJ = j;
          bestI = i;
        }

      }
    }

    //Prints the full score matrix if needed
    /*if(debug>5){
      cerr << endl << "\t";
      for(i=0;i<ti;i++){ cerr << "\t" << b[i]; }
      cerr << endl << "\t";
      for(j=0;j<=tj;j++){
      if(j>0) cerr << a[j-1] << "\t";
      for(i=0;i<=ti;i++){
      cerr << m[W*j+i] << "\t";
      }
      cerr << endl;
      }
      }*/


    /*
      || Reconstruction Step
    */
    bool final = true;    //Exit condition
    int lastScore = score;
    int steps = 0; //Number of steps uset to align 'a' with 'b'
    int lastChar = 0;


    int ni=0;
    int mymax=0;

    switch(mode){
    case SEMILOCAL:
      /* In semi-local alignment, the best-path reconstructions doesn't start from the 
       * botom-right corner of the matrix but from the best-scoring cell among last
       * column and last row scores.
       * In this implementation only the last row is taken in account since it is 
       * the "long" string
       */
      for(i=0;i<=ti;i++){
        if( mymax < m[W*tj+i] ){
          mymax = m[W*tj+i];
          ni=i;
        }
      }
      score = mymax;
      for(i=ti-1;i>=ni;i--){ // Carry on half of the string for debugging purpouses
        answerA[pA++]= L'-';
        answerB[pB++]= b[i];
        insertions++;
      }
      i=ni; j=tj;
      break;
    case GLOBAL: // now this is global alignment
      i = ti; j = tj;
      break;
    case LOCAL: 
      /* In LOCAL alignment, the best-path reconstruction starts with at the best scoring 
       * position in the whole matrix.
       */
      j = bestJ;
      i = bestI;
      break;
    }


    while( final ){

      /*if(debug>5){
        cerr << "Looking for " << m[ W*j +i] << " in ("<<j<<","<<i<<")"<<endl;
        cerr << "m[j-1][i-1]=" << m[ W*(j-1) +i-1] << ": dSub=" << sc->dSub(a[j-1],b[i-1]) << endl;
        cerr << "m[j][i-1]=" <<   m[ W*j +i-1] << ": dSkipB=" << sc->dSkip(b[i-1]) << endl;
        cerr << "m[j-1][i]=" <<   m[ W*(j-1) +i] << ": dSkipA=" << sc->dSkip(a[j-1]) << endl;
        if(j>1){ cerr << "m[j-1][i-2]=" << m[ W*(j-1) +i-2] << ": dExp=" << sc->dExp(a[j-1],b[i-2],b[i-1]) << endl; }
        if(i>1){ cerr << "m[j-2][i-1]=" << m[ W*(j-2) +i-1] << ": dExp=" << sc->dExp(b[i-1],a[j-2],a[j-1]) << endl; }
        }*/


      if( j>0 && i>0 && m[W*j+i] ==  m[W*(j-1)+i-1] + sc->dSub(a[j-1],b[i-1]) ){
        //Aliniar b[i] amb a[j]
        lastScore = sc->dSub(a[j-1],b[i-1]);
        i--; j--;
        //cerr << "Sub a[" << j << "]=" << a[j] << " b[" << i << "]=" << b[i] << " @ " << pA << endl;
        answerA[pA++] = a[j];
        answerB[pB++] = b[i];
        indexInit= i;
        indexEnd = std::max(i,indexEnd);
        if( a[j]!=b[i] ){ substitutions++;  }
        steps++;
        lastChar = steps;

      } else if ( i>0 && j>0 && m[W*j+i] == m[W*j+i-1] + sc->dSkip(b[i-1]) ) {
        lastScore = sc->dSkip(b[i-1]);
        i--;
        answerA[pA++] = L'-';
        answerB[pB++] = b[i];
        insertions++;
        if(steps==0 && b[i]==L' ' && b[i]==L'_' ){
          score = m[W*j+i];
        } else {
          steps++;
        }

      } else if( j>1 && i>0 && m[W*j+i] ==  m[W*(j-2)+i-1] + sc->dExp(b[i-1],a[j-2],a[j-1]) ){
        lastScore =  sc->dExp(b[i-1],a[j-2],a[j-1]);
        i--; j-=2;
        answerA[pA++] = a[j];
        answerA[pA++] = a[j+1];
        answerB[pB++] = b[i];
        answerB[pB++] = L'+';
        indexInit = i;
        indexEnd = std::max(i,indexEnd);
        deletions++;
        if( a[j]!=b[i] ){ substitutions++; }
        steps+=2;
        lastChar = steps;

      } else if( j>0 && i>0 && m[W*j+i] ==  m[W*(j-1)+i] + sc->dSkip(a[j-1]) ){
        lastScore = sc->dSkip(a[j-1]);
        j--;
        answerA[pA++] = a[j];
        answerB[pB++] = L'-';
        indexInit = i;
        indexEnd = std::max(i,indexEnd);
        deletions++;
        steps++;
        lastChar = steps;

      } else if ( i>1 && j>0 && m[W*j+i] == m[ W*(j-1) +i-2 ] + sc->dExp(a[j-1],b[i-2],b[i-1]) ) {
        lastScore = sc->dExp(a[j-1],b[i-2],b[i-1]);
        j--; i--;
        answerA[pA++] = a[j];
        answerA[pA++] = L'+';
        answerB[pB++] = b[i];
        answerB[pB++] = b[i-1];
        indexInit = i;
        indexEnd = std::max(i,indexEnd);
        insertions++;
        if( a[j]!=b[i] ){ substitutions++; }
        i--;
        steps+=2;
        lastChar = steps;

      } else if ( j==0 ){
        i--;
        answerA[pA++] = L'-';
        answerB[pB++] = b[i];
        insertions++;
        if(steps!=0 || b[i]!=L' ' || b[i]!=L'_' ) steps++;

      } else if ( i==0 ){
        j--;
        answerA[pA++] = a[j];
        answerB[pB++] = L'-';
        deletions++;

        if(steps!=0 || b[i]!=L' ' || b[i]!=L'_' ) steps++;
        lastChar = steps;
        
      } else {
        std::wcerr << L"BOINK! Error at "<< j << L"," << i << std::endl;
        break;
      }
      
      // This is the termination condition
      switch(mode){
      case SEMILOCAL:
      final = j!=0;
      if(!final){
        i--;
        while(i>=0){
          answerA[pA++] = L'-';
          answerB[pB++] = b[i--];
          insertions++;
        }
      }
      break;

      case GLOBAL:
      final = i!=0 || j!=0;
      break;

      case LOCAL:
      final = m[W*j+i]!=0;
      break;
      }
    }


    // Computing of the score
    // Score= avg.points/phoneme
    steps = lastChar;

    switch(mode){
    case GLOBAL:
      // average of similarity per phonem (without spaces?)
      score /= (pA-spacesA);
      break;
    case SEMILOCAL:
      //if(debug>2) cerr << "     Score = " << score << " Last Score = " << lastScore << " steps = " << steps <<  endl;
      //score = (score-lastScore) / steps;
      score = score / steps;
      break;
    case LOCAL:
      score = score / steps;
      break;
    }

    // Print the alignment
    wchar_t* newA = new wchar_t[pA+1];
    wchar_t* newB = new wchar_t[pB+1];
    newA[pA]=0;
    newB[pB]=0;
    --pA;
    --pB;

    for(int i=0; pA>=0; ++i, --pA){
      newA[i] = answerA[pA];
    }
    for(int i=0; pB>=0; ++i, --pB){
      newB[i] = answerB[pB];
    }

    delete[](answerA);
    delete[](answerB);
    delete[] m;

    // Finds in wich word does indexInit and indexEnd fall
    for(int i=0; i<nwords; i++){ 
      if( words[i] == indexInit ){
        initWord = i+1;
        break;
      } else if( words[i] < indexInit ) {
        initWord = i+1;
      } else if( words[i] > indexInit ) {
        initWord = i;
        break;
      }

    }

    for(int i=0; i<nwords; i++){ 
      if( words[i] == indexEnd ){
        endWord = i;
        break;
      } else if( words[i] < indexEnd ) {
        endWord = i+1;
      } else if( words[i] > indexEnd ) {
        endWord = i;
        break;
      }

    }

    delete[] words;

    return new alin(score,indexInit,indexEnd,initWord,endWord,substitutions,insertions,deletions,newA,newB);

  }

  };

} // namespace

#endif
