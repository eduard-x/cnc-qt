#!/bin/bash

CPPCHCK=$(type -p astyle)

if [ ! -f "$CPPCHCK" ]; then
   # not found exit
   echo "please install astyle and restart this script"
   exit 0
fi

set -e
 
ARTISTIC_STYLE_OPTIONS="\
--mode=c \
--style=k&r \
--indent=spaces=4 \
--indent-classes \
--indent-switches \
--indent-col1-comments \
--indent-preprocessor \
--break-blocks \
--pad-oper \
--add-brackets \
--convert-tabs \
--formatted \
--lineend=linux"

astyle $ARTISTIC_STYLE_OPTIONS --suffix=none --recursive  "sources/*.cpp" "sources/*.h";
