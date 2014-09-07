@ECHO OFF
mkdir build
cd build
cl.exe ..\%1.cpp /EHsc /Od /Zi /IC:/msys64/home/chgl/src/boost-git/ /I..\
cd ..\ 
