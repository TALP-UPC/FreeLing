default: parseAnCora encodeAnCora train classify classify_pcl process_stats

parseAnCora: parseAnCora.cc 
	g++ -O3 -Wall `xml2-config --cflags --libs` -ldb_cxx -lmorfo -lfries -lomlet -lpcre -o parseAnCora parseAnCora.cc

encodeAnCora: encodeAnCora.cc
	g++ -O3 -Wall `xml2-config --cflags --libs` -ldb_cxx -lmorfo -lfries -lomlet -lpcre -o encodeAnCora encodeAnCora.cc

train: train.cc
	g++ -O3 -Wall `xml2-config --cflags --libs` -ldb_cxx -lmorfo -lfries -lomlet -lpcre -o train train.cc

classify: classify.cc
	g++ -O3 -Wall `xml2-config --cflags --libs` -ldb_cxx -lmorfo -lfries -lomlet -lpcre -o classify classify.cc

classify_pcl: classify_pcl.cc
	g++ -O3 -Wall `xml2-config --cflags --libs` -ldb_cxx -lmorfo -lfries -lomlet -lpcre -o classify_pcl classify_pcl.cc

process_stats: process_stats.cc
	g++ -O3 -Wall -o process_stats process_stats.cc

clean:
	$(RM) -r classify classify_pcl encodeAnCora parseAnCora process_stats train
