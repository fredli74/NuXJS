testOne()
{
	if [[ -e $(which $2) ]]; then
		echo
		echo --- $1 ---
		echo
		$2 goodTests.js | diff --strip-trailing-cr goodResults.txt -
	fi
}

echo "### GOOD TESTS ###"
echo
echo --- NuX ---
echo
export DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib
build/Debug/NuXJScript -s goodTests.js | diff --strip-trailing-cr goodResults.txt -
# build/Debug/NuXJScript -s -gc 1 goodTests.js | diff --strip-trailing-cr goodResults.txt -
export DYLD_INSERT_LIBRARIES=
#testOne V8 v8
#testOne JavaScriptCore jsc
#testOne SpiderMonkey js
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
