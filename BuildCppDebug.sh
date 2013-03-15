#!/bin/bash

if [ $# -lt 2 ]; then
	echo BuildCpp.sh \<output\> \<compiler options and source files\>
	exit 1
fi

echo Compiling $1...
g++ -x c++ -pipe -Wno-trigraphs -O0 -g -Wreturn-type -Wunused-variable -DDEBUG -fmessage-length=0 -fstrict-aliasing -m32 -fvisibility=hidden -fvisibility-inlines-hidden -fno-threadsafe-statics -o $@ >/var/tmp/buildlog.txt 2>&1
if [ $? -ne 0 ]; then
	cat /var/tmp/buildlog.txt
	echo Compilation of $1 failed
	exit 1
else
	echo Compiled $1
	exit 0
fi
