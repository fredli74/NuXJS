echo "### GOOD TESTS ###"
echo
echo --- NuX ---
echo
export DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib
build/Debug/NuXJScript -s goodTests.js | diff --strip-trailing-cr goodResults.txt -
export DYLD_INSERT_LIBRARIES=
if [[ -e $(which v8) ]]; then
	echo
	echo --- V8 ---
	echo
	v8 goodTests.js | diff --strip-trailing-cr goodResults.txt -
fi
if [[ -e $(which jsc) ]]; then
	echo
	echo --- JSC ---
	echo
	jsc goodTests.js | diff --strip-trailing-cr goodResults.txt -
fi
if [[ -e $(which js) ]]; then
	echo
	echo --- SpiderMonkey ---
	echo
	js goodTests.js | diff --strip-trailing-cr goodResults.txt -
fi
#
# Seems like Rhino doesn't enumerate string characters according to standard: http://www.ecma-international.org/ecma-262/5.1/#sec-15.5.5.2
#
#if [[ -e $(which rhino) ]]; then
#	echo Rhino
#	rhino goodTests.js | diff --strip-trailing-cr goodResults.txt -
#fi
echo "### UNCONFORMING TESTS ###"
echo
build/Debug/NuXJScript <unconformTests.js 2>/dev/null | diff --strip-trailing-cr unconformResults.txt -
echo
echo "### BAD TESTS ###"
echo
build/Debug/NuXJScript <badtests.js 2>/dev/null | diff --strip-trailing-cr badresults.txt -
