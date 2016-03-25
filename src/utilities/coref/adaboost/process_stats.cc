//////////////////////////////////////////////////////////////////
//    Simple program to calculate the arithmetic mean of the 10 results.
//////////////////////////////////////////////////////////////////

#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

//////////////////////////////////////////////////////////////////
///    Main process.
//////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
	int num, nummax = 0, i, cont = 0;
	string text;
	double tmp, recall[50], precision[50], fscore[50];

	for(i=0;i<50;i++){
		recall[i] = 0.0;
		precision[i] = 0.0;
		fscore[i] = 0.0;
	}

	while(cin >> num){
		if(num == 0)
			cont++;
		if(num > nummax)
			nummax = num;
		cin >> text;
		cin >> text;
		cin >> tmp;
		recall[num] += tmp;
		cin >> text;
		cin >> tmp;
		precision[num] += tmp;
		cin >> text;
		cin >> tmp;
		fscore[num] += tmp;
	}

	for(i=0;i<=nummax;i++){
		cout << i << ":	Recall: " << (recall[i]/cont) << "	Precisio: " << (precision[i]/cont) << "	F-score: " << (fscore[i]/cont) << endl;

//		cout << (recall[i]/cont) << endl;
//		cout << (precision[i]/cont) << endl;
//		cout << (fscore[i]/cont) << endl;
	}
}
