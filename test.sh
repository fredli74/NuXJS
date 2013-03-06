echo "### GOOD TESTS ###"
echo
build/Debug/NuXJScript <goodTests.js 2>/dev/null | diff --strip-trailing-cr goodResults.txt -
 v8 goodTests.js | diff --strip-trailing-cr goodResults.txt -
 jsc goodTests.js | diff --strip-trailing-cr goodResults.txt -
echo "### UNCONFORMING TESTS ###"
echo
build/Debug/NuXJScript <unconformTests.js 2>/dev/null | diff --strip-trailing-cr unconformResults.txt -
echo
echo "### BAD TESTS ###"
echo
build/Debug/NuXJScript <badtests.js 2>/dev/null | diff --strip-trailing-cr badresults.txt -
