@echo off
echo.
echo ### GOOD TESTS ###
echo.
.\Debug\NuXJScript.exe -s goodtests.js >goodcheck.txt
fc goodcheck.txt goodresults.txt
del goodcheck.txt
echo.
echo ### UNCONFORMING TESTS ###
echo.
.\Debug\NuXJScript.exe -s unconformtests.js >unconformcheck.txt
fc unconformcheck.txt unconformresults.txt
del unconformcheck.txt
echo.
echo ### BAD TESTS ###
echo.
.\Debug\NuXJScript.exe -s <badtests.js >badcheck.txt
fc badcheck.txt badresults.txt
del badcheck.txt
