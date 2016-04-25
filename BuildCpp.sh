#!/bin/bash

COMPILER=g++
COMPILER=/opt/local/bin/g++-mp-5

if [ "$1" == "debug" ]; then
	OPTIONS="-O0 -DDEBUG -g"
	shift
elif [ "$1" == "beta" ]; then
	OPTIONS="-Os -DDEBUG -g"
	shift
elif [ "$1" == "release" ]; then
	OPTIONS="-Os -DNDEBUG -fno-rtti"
	shift
else
	OPTIONS="-Os -DNDEBUG -fno-rtti"
fi

if [ "$1" == "64" ]; then
	MODEL="-m64"
	shift
elif [ "$1" == "32" ]; then
	MODEL="-m32"
	shift
else
	MODEL="-m32"
fi

if [ $# -lt 2 ]; then
	echo "BuildCpp.sh [debug|beta|release] [32|64 (bit)] <output> <compiler options and source files>"
	exit 1
fi

echo Compiling $1...
$COMPILER -pipe $OPTIONS $MODEL -Wno-trigraphs -Wreturn-type -Wunused-variable -fmessage-length=0 -fstrict-aliasing -fvisibility=hidden -fvisibility-inlines-hidden -fno-threadsafe-statics -o $@ >/var/tmp/buildlog.txt 2>&1
if [ $? -ne 0 ]; then
	cat /var/tmp/buildlog.txt
	echo Compilation of $1 failed
	exit 1
else
	echo Compiled $1
	exit 0
fi
