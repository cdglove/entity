#!/bin/sh
CMD='/usr/bin/include-what-you-use -stdlib=libstdc++ -std=c++11 -O3 -DNDEBUG -I/home/chris/src/local/entity/_target/clang/test -I/home/chris/src/local/entity -I/home/chris/src/local/entity/_target/clang -I/home/chris/src/cmn/boost -I/home/chris/src/local/entity/contrib/daily'
echo "" > iwyu.txt
for f in `find ../../ -name "*.hpp"`; do
	CPP=${f%.*}.cpp
	echo "#include \"$f\"" > $CPP
	CMD2="$CMD -c $CPP"
	#echo $CMD2
	$CMD2 2>> iwyu.txt
	rm $CPP
done