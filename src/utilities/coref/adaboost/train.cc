//////////////////////////////////////////////////////////////////
//    Train. Train a file with samples encoded and outputs a model.
//////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <sstream>

#include <string.h>
#include "omlet.h"

using namespace std;

//////////////////////////////////////////////////////////////////
///    Main process
//////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
	map<string,int> lexicon;
	map<string,string> codes;
	char snam[512];
	string num,name,wr_type,line;
	int nlab = 2;
 
	// create dataset to store examples;
	dataset ds(nlab);

	// read std input examples (one per line) into train data set
	while (std::getline(std::cin, line)) {
		int clas,feat;
		istringstream sin;

		sin.str(line);
		// first field in line is the class for the example
		sin >> clas;
		// create new example with that class
		example ex(nlab);
		ex.set_label(clas,true,0,0);

		// following fields are feature codes
		while (sin >> feat)
			ex.add_feature(feat);

		// add complete example to dataset
		ds.add_example(ex);
	}

	// Set weak rule type to be used.  Note that the
	// wr_type must correspond to a registered weak rule type.
	// "mlDTree" is preregistered in libomlet, but you can write
	// code for your own WR and register it without recompiling the
	// library.  Check documentation for details on how to do this.
	wr_type="mlDTree";

	// Set parameters for WRs, nlab and epsilon are generic for all WRs.
	// Third parameter is max_depth, specific to mlDTree.
	mlDTree_params wp(nlab,0.001,4);

	// create and learn adaboost model
	adaboost learner(nlab, wr_type);

	// open model output file
	strcpy(snam, argv[1]);
	ofstream abm(strcat(snam, ".abm"));

	// write class descriptions on first line
	abm<<"0 Negative 1 Positive"<<endl;
	// write Weak rule type on second line
	abm<<wr_type<<endl;
		
	// learn the model, up to 500 weak rules, incrementally writting it to the file.
	learner.set_output((ostream*)&abm);
	learner.learn(ds, 200, true, (wr_params *)&wp);

	abm.close();
}
