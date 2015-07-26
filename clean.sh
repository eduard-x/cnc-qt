#!/bin/bash

astyle.sh

if [ -f 'Makefile' ]; then
   make clean
   rm Makefile
fi

cd build
if [ -f 'Makefile' ]; then
   make clean
   rm * -rf
fi
cd ..

rm "build-stamp"
rm "install_manifest.txt"
rm "version.h"

#make clean
for i in $(find . -type f \( -name "*.orig" -or -name "*~" -or -name "moc_*" \));
do
    rm -if "$i";
done

rm -if $(find ./sources/ -name "*.cmake")
rm -if $(find ./sources/ -name "*.qrc.depends")
rm -if $(find . -name "Makefile")
rm -if $(find . -name "CMakeCache.txt")
rm -ifr $(find . -name "CMakeFiles")


