#!/bin/sh
CMD='/home/chris/src/build/llvm/_target/gcc/bin/include-what-you-use -Xiwyu --mapping_file=boost-mappings/boost.imp -stdlib=libstdc++ -std=c++11 -O3 -DNDEBUG -I/home/chris/src/local/entity/_target/clang/test -I/home/chris/src/local/entity -I/home/chris/src/local/entity/_target/clang -isystem/home/chris/src/cmn/boost -I/home/chris/src/local/entity/contrib/daily'
echo "" > iwyu.txt
for f in `find ../../ -name "*.hpp"`; do
	CPP=${f%.*}.cpp
	echo "#include \"$f\"" > $CPP
	CMD2="$CMD -c $CPP"
	#echo $CMD2
	$CMD2 2>> iwyu.txt
	rm $CPP
done