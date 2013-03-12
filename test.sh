testOne()
{
	if [[ -e $(which $2) ]]; then
		echo
		echo --- $1 ---
		echo
		$2 goodTests.js | diff --strip-trailing-cr goodResults.txt -
	fi
}

if [ "$1" == "all" ]; then
	echo
	echo --- NuX ---
	echo
	build/Debug/NuXJScript -s goodTests.js | diff --strip-trailing-cr goodResults.txt -
	testOne V8 v8
	testOne JavaScriptCore jsc
	testOne SpiderMonkey js
	testOne Rhino rhino
elif [ -n "$1" ]; then
	testOne "$1" "$1"
else
	echo "### GOOD TESTS ###"
	export DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib
	build/Debug/NuXJScript -s goodTests.js | diff --strip-trailing-cr goodResults.txt -
	# build/Debug/NuXJScript -s -gc 1 goodTests.js | diff --strip-trailing-cr goodResults.txt -
	export -n DYLD_INSERT_LIBRARIES
	echo "### UNCONFORMING TESTS ###"
	echo
	build/Debug/NuXJScript <unconformTests.js 2>/dev/null | diff --strip-trailing-cr unconformResults.txt -
	echo
	echo "### BAD TESTS ###"
	echo
	build/Debug/NuXJScript <badtests.js 2>/dev/null | diff --strip-trailing-cr badresults.txt -
fi
