//////////////////////////////////////////////////////////////////
//    encodeAncora - Encodes a file with samples and generates a new file with en encoded features
//////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

#include "freeling.h"
#include "freeling/traces.h"

using namespace std;

struct RESULT {
	int positive;
	std::vector<int> result;
};
list<struct RESULT *> allRes;

//////////////////////////////////////////////////////////////////
///    Adds a sample vector to the list
//////////////////////////////////////////////////////////////////

void add(int positive, std::vector<int> &result){
	struct RESULT *res = new RESULT;
	res->positive = positive;
	res->result = result;
	allRes.push_back(res);
}

//////////////////////////////////////////////////////////////////
///    Outputs a sample vector to the file
//////////////////////////////////////////////////////////////////

void save(ofstream *outfile, int positive, std::vector<int> &result){
	std::vector<int>::iterator vit;
	
	(*outfile) << positive << " ";
	vit = result.begin();
	while(vit != result.end()){
		(*outfile) << (*vit);
		++vit;
		if(vit != result.end())
			(*outfile) << " ";
	}
	(*outfile) << endl;
}

//////////////////////////////////////////////////////////////////
///    Process all the file. Read, encode and save all the samples.
///    Generates 10 groups (for crossvalidation) with two files (learn and text)
//////////////////////////////////////////////////////////////////

void process(coref_fex &cFex, char *filename, char *outputfile){
	char buffer[1000];
	int countp = 0, countn = 0;
	int positive;
	ifstream myfile (filename, ios::in|ios::binary);
	int statsV[50], statsVP[50], i;
	string strTmp;

	for(i=0; i<50;i++){
		statsV[i] = 0;
		statsVP[i] = 0;
	}
	if (myfile.good()){
		while(!myfile.eof()){
			myfile.getline(buffer, 1000);
			if(!myfile.eof()){
				struct EXAMPLE ex;
				if(buffer[0] == '+'){
					positive = 1;
					countp++;
				}else{
					positive = 0;
					countn++;
				}
				myfile.getline(buffer, 1000);
				ex.sample1.sent = atoi(buffer);
				myfile.getline(buffer, 1000);
				ex.sample1.posbegin = atoi(buffer);
				myfile.getline(buffer, 1000);
				ex.sample1.posend = atoi(buffer);
				myfile.getline(buffer, 1000);
				if(buffer[strlen(buffer)-1] == ' ')
					buffer[strlen(buffer)-1] = 0;
				ex.sample1.text = buffer;
				transform(ex.sample1.text.begin(), ex.sample1.text.end(), ex.sample1.text.begin(), (int (*)(int))std::tolower);
				ex.sample1.texttok = cFex.tokenize(ex.sample1.text.c_str()," ");
				myfile.getline(buffer, 1000);
				if(buffer[strlen(buffer)-1] == ' ')
					buffer[strlen(buffer)-1] = 0;
				strTmp = buffer;
				transform(strTmp.begin(), strTmp.end(), strTmp.begin(), (int (*)(int))std::tolower);
				ex.sample1.tags = cFex.tokenize(strTmp.c_str()," ");
				myfile.getline(buffer, 1000);
				ex.sample2.sent = atoi(buffer);
				ex.sent = ex.sample2.sent - ex.sample1.sent;
				myfile.getline(buffer, 1000);
				ex.sample2.posbegin = atoi(buffer);
				myfile.getline(buffer, 1000);
				ex.sample2.posend = atoi(buffer);
				myfile.getline(buffer, 1000);
				if(buffer[strlen(buffer)-1] == ' ')
					buffer[strlen(buffer)-1] = 0;
				ex.sample2.text = buffer;
				transform(ex.sample2.text.begin(), ex.sample2.text.end(), ex.sample2.text.begin(), (int (*)(int))std::tolower);
				ex.sample2.texttok = cFex.tokenize(ex.sample2.text.c_str()," ");
				myfile.getline(buffer, 1000);
				if(buffer[strlen(buffer)-1] == ' ')
					buffer[strlen(buffer)-1] = 0;
				strTmp = buffer;
				transform(strTmp.begin(), strTmp.end(), strTmp.begin(), (int (*)(int))std::tolower);
				ex.sample2.tags = cFex.tokenize(strTmp.c_str()," ");

				std::vector<int> result;
				cFex.extract(ex, result);
				add(positive, result);

				std::vector<int>::iterator vit;
				vit = result.begin();
				i=0;
				while(vit != result.end()){
					if((*vit) == 1)
						statsV[i]++;
					if((*vit) == 1 && positive == 1)
						statsVP[i]++;
					++vit;
					i++;
				}
			}
		}
	}
	int num = allRes.size();
	for(i = 1; i<=10; i++){
		int curr = 0;
		char outputfileT[100], outputfileL[100];
		list<struct RESULT *>::iterator itRes;
		sprintf(outputfileT, "%s%d.test", outputfile, i);
		sprintf(outputfileL, "%s%d.learn", outputfile, i);
		ofstream *outfileT = new ofstream(outputfileT, ios::out|ios::binary);
		ofstream *outfileL = new ofstream(outputfileL, ios::out|ios::binary);
		itRes = allRes.begin();
		while(itRes != allRes.end()){
			if( curr < (num/10)*i && curr > (num/10)*(i-1))
				save(outfileT, (*itRes)->positive, (*itRes)->result);
			else
				save(outfileL, (*itRes)->positive, (*itRes)->result);
			++itRes;
			curr++;
		}
		outfileT->close();
		outfileL->close();
	}
/*
	for(i=0;i<50;i++){
		if(statsV[i] > 0){
			cerr << "Vector: " << i << ": " << statsV[i] << "/" << (countn+countp) << " : " << (statsV[i]*100.0)/(countn+countp) << "%" << endl;
			cerr << "Vector aciertos: " << i << ": " << statsVP[i] << "/" << statsV[i] << " : " << (statsVP[i]*100.0)/statsV[i] << "%" << endl;
		}
	}
*/
	cout << "Positivos: " << countp << endl;
	cout << "Negativos: " << countn << endl;
	cout << "Positivos/Negativos %: " << (countp*100.0)/(countn+countp) << endl;

}

//////////////////////////////////////////////////////////////////
///    Main process.
//////////////////////////////////////////////////////////////////

int main(int argc, char **argv){
	coref_fex cFex(COREFEX_TYPE_TWO, 0, "/usr/local/share/FreeLing/es/senses16.db", "/usr/local/share/FreeLing/common/wn16.db");
	int pos=1, vectors = 0;
	char infile[256], outputfile[256];

	if(argc < 3) {
		cout << "encodeSamples -3 samplesFile OutputFile" << endl;
		cout << "	-2: Output two values (0 no match or unknow, 1 match). (Default)" << endl;
		cout << "	-3: Output three values (0 no match, 1 match, 2 Unknow)." << endl;
		cout << "	-dist: Output the distance between i and j in sentences (0 the same, ...)." << endl;
		cout << "	-ipron: Output is i element is a pronoum." << endl;
		cout << "	-jpron: Output is j element is a pronoum." << endl;
		cout << "	-ipronm: Output is i element is a pronoum and his type." << endl;
		cout << "	-jpronm: Output is j element is a pronoum and his type." << endl;
		cout << "	-strmatch: Output is i and j elements match." << endl;
		cout << "	-defnp: ." << endl;
		cout << "	-demnp: ." << endl;
		cout << "	-number: Output if the number match." << endl;
		cout << "	-gender: Output if the gender match." << endl;
		cout << "	-semclass: Output if the semanthic class match (with wordnet)." << endl;
		cout << "	-propname: Output if i and j are proper noums." << endl;
		cout << "	-alias: Output if j is an alias of i." << endl;
		cout << "	-appos: Output if i is an apposition of j." << endl;
		cout << "	samplesFile: File with the samples to encode." << endl;
		cout << "	OutputFile: File to output the samples encoded." << endl;
		return(1);
	}

	while(strncmp(argv[pos], "-", 1) == 0){
		if(strncmp(argv[pos], "-2", 2) == 0)
			cFex.typeVector = COREFEX_TYPE_TWO;
		else if(strncmp(argv[pos], "-3", 2) == 0)
			cFex.typeVector = COREFEX_TYPE_THREE;
		else if(strncmp(argv[pos], "-dist", 5) == 0)
			vectors |= COREFEX_DIST;
		else if(strncmp(argv[pos], "-ipronm", 7) == 0)
			vectors |= COREFEX_IPRONM;
		else if(strncmp(argv[pos], "-jpronm", 7) == 0)
			vectors |= COREFEX_JPRONM;
		else if(strncmp(argv[pos], "-ipron", 6) == 0)
			vectors |= COREFEX_IPRON;
		else if(strncmp(argv[pos], "-jpron", 6) == 0)
			vectors |= COREFEX_JPRON;
		else if(strncmp(argv[pos], "-strmatch", 9) == 0)
			vectors |= COREFEX_STRMATCH;
		else if(strncmp(argv[pos], "-defnp", 6) == 0)
			vectors |= COREFEX_DEFNP;
		else if(strncmp(argv[pos], "-demnp", 6) == 0)
			vectors |= COREFEX_DEMNP;
		else if(strncmp(argv[pos], "-number", 7) == 0)
			vectors |= COREFEX_NUMBER;
		else if(strncmp(argv[pos], "-gender", 7) == 0)
			vectors |= COREFEX_GENDER;
		else if(strncmp(argv[pos], "-semclass", 9) == 0)
			vectors |= COREFEX_SEMCLASS;
		else if(strncmp(argv[pos], "-propname", 9) == 0)
			vectors |= COREFEX_PROPNAME;
		else if(strncmp(argv[pos], "-alias", 6) == 0)
			vectors |= COREFEX_ALIAS;
		else if(strncmp(argv[pos], "-appos", 6) == 0)
			vectors |= COREFEX_APPOS;
		else
			cout << "Option unknow: " << argv[pos] << endl;
		pos++;
	}
	cFex.setVectors(vectors);
	strcpy(infile, argv[pos++]);
	strcpy(outputfile, argv[pos]);
	process(cFex, infile, outputfile);

	return 0;
}
