#!/bin/bash

mkdir -p tmp
rm -rf tmp/*

echo "Parsing AnCora"
echo

for fileIn in Ancora_ES/AnCora_CO_ES/*
do
	dir2=${fileIn%/*}
	dir2=${dir2%/*}"/CESS_ES/"
	fileCess=${fileIn##*/}
	fileCess=${fileCess%.*}
	fileCess=${fileCess%.*}
	fileCess=$dir2$fileCess"*.tbf"
	cat $fileIn > tmp/1.xml
	cat $fileCess > tmp/1.tbl
	echo "	file: "$fileIn
	# Parse all the Samples
	#./parseAnCora -onlyhead -limit 0 tmp/1.xml tmp/1.tbl tmp/parseout.txt
	# Parse all the Samples limited at distance 20
	#./parseAnCora -onlyhead -limit 20 tmp/1.xml tmp/1.tbl tmp/parseout.txt

	#Parse with all negatives at distance 10
	#./parseAnCora -onlyhead -allneg 10 tmp/1.xml tmp/1.tbl tmp/parseout.txt

	#eliminate false negatives with all the positives
	#./parseAnCora -elimFN -onlyhead -allpos -limit 10 tmp/1.xml tmp/1.tbl tmp/parseout.txt

	#All the positives and all the negatives at distance 10
	./parseAnCora -allpos -onlyhead -allneg 10 tmp/1.xml tmp/1.tbl tmp/parseout.txt > /dev/null

	#Generates the same proportion of negatives and positives.
	#./parseAnCora -onlyhead -limit 0 -re 1.0 /usr/tmp/1.xml /usr/tmp/1.tbl /usr/tmp/out1.txt
done

echo "Encoding samples"
./encodeAnCora -2 -dist -ipron -jpron -ipronm -jpronm -strmatch -defnp -demnp -number -gender -semclass -propname -alias -appos tmp/parseout.txt tmp/encoderout.

echo "Training ten models"
for num in {1,2,3,4,5,6,7,8,9,10};
do
	echo "	Model: "$num
 	cat tmp/encoderout.$num.learn | train tmp/model_$num
done

echo "Training complete model"
cat tmp/encoderout.1.learn tmp/encoderout.1.test | train tmp/model

echo "Testing models"

for num in {1,2,3,4,5,6,7,8,9,10};
do
	echo $num
 	./classify tmp/model_$num.abm < tmp/encoderout.$num.test >> tmp/test.txt
done

./process_stats < tmp/test.txt > tmp/testAll.txt
