echo "### GOOD TESTS ###"
echo
build/Debug/NuXJScript <goodtests.js 2>/dev/null | diff --strip-trailing-cr goodresults.txt -
# v8 goodtests.js | diff --strip-trailing-cr goodresults.txt -
# jsc goodtests.js | diff --strip-trailing-cr goodresults.txt -
echo
echo "### BAD TESTS ###"
echo
build/Debug/NuXJScript <badtests.js 2>/dev/null | diff --strip-trailing-cr badresults.txt -
