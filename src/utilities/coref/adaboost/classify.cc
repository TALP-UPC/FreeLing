//////////////////////////////////////////////////////////////////
//    Classify. Gets a model and a set of classified examples and outputs:
//    precision, recall and f1.
//////////////////////////////////////////////////////////////////

#include <fstream>
#include <sstream>

#include "omlet.h"

#define MAX_NUM_VECTOR 100
using namespace std;

int main(int argc, char* argv[]) {
	int j;
	// create AdaBoost classifier loading given model file
	adaboost *classifier = new adaboost(string(argv[1]));

	// allocate prediction array (reused for all examples)
	double pred[classifier->get_nlabels()];

	int pe[MAX_NUM_VECTOR];
	int pm[MAX_NUM_VECTOR];
	int pok[MAX_NUM_VECTOR];
	int vect[MAX_NUM_VECTOR];
	for (j=0; j<MAX_NUM_VECTOR; j++) {
		pe[j] = 0;
		pm[j] = 0;
		pok[j] = 0;
		vect[j] = 0;
	}
	int vect_max;
	string line;

	while (std::getline(std::cin,line)) {
		int clas, feat;
		istringstream sin; sin.str(line);
		// first field in line is the class for the example (only useful
		// here to compute classifier performace)
		sin>>clas;
		// create new example.
		example ex(classifier->get_nlabels());
		// following fields are feature codes
		vect_max = 0;
		while (sin>>feat){
			vect[vect_max++] = feat;
			ex.add_feature(feat);
		}

		// classify current example
		classifier->classify(ex,pred);

		// find out which class has highest weight (alternatively, we
		// could select all classes with positive weight, and propose
		// more than one class per example)
		double max=pred[0];
		string tag=classifier->get_label(0);
		for (j=1; j<classifier->get_nlabels(); j++) {
			if (pred[j]>max) {
				max=pred[j];
				tag=classifier->get_label(j);
			}
		}

		// if no label has a positive prediction and <others> is defined, select <others> label.
		string def = classifier->default_class();
		if (max<0 && def!="") tag = def;

		if(classifier->get_label(clas)=="Positive"){
			pe[0]++;
			for(j=0;j<vect_max;j++){
				if(vect[j] == 1)
					pe[j+1]++;
			}
		}
		if(tag=="Positive"){
			pm[0]++;
			for(j=0;j<vect_max;j++){
				if(vect[j] == 1)
					pm[j+1]++;
			}
		}
		if(classifier->get_label(clas)=="Positive" && classifier->get_label(clas)==tag){
//			cout << vect[0] << " " << vect[1] << endl;
			pok[0]++;
			for(j=0;j<vect_max;j++){
				if(vect[j] == 1)
					pok[j+1]++;
			}
		}
	}

	for(j=0;j<1;j++){
//	for(j=0;j<vect_max+1;j++){
		double recall;
		double precisio;
		double fscore;
		if(pok[j] == 0 || pm[j] == 0) {
			recall = 0;
			precisio = 0;
			fscore = 0;
		} else {
			recall = ((double)pok[j] / pe[j]);
			precisio = ((double)pok[j] / pm[j]);
			fscore = (2.0 * precisio * recall) / (precisio + recall);
		}
		cout << j << ":	Recall: " << recall << "	Precisio: " << precisio << "	F-score: " << fscore << endl;
	}

}


