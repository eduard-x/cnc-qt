#!/bin/bash

CMD=$(type -p cmake)

if [ ! -x "$CMD" ]; then
   # not found exit
   echo "please install cmake and restart this script"
   exit 1
fi

CPU_NUM=$(nproc --all)
echo "cpu cores: $CPU_NUM"

DIRECTORY="build"

if [ ! -d "$DIRECTORY" ]; then
  # Control will enter here if $DIRECTORY exists.
    mkdir "$DIRECTORY"
fi

# because of removing of all files in directory
if [ -d "$DIRECTORY" ]; then
    cd "$DIRECTORY"
    rm * -rf
    # options: USE_PROFILER, USE_DEBUGGER, USE_QT5
    cmake -DUSE_DEBUGGER=OFF -DUSE_PROFILER=OFF ..
    make -j$CPU_NUM
    cd ..
    cp "./$DIRECTORY/cnc-qt" .
fi
