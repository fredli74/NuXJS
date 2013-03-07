@echo off
echo.
echo ### GOOD TESTS ###
echo.
.\Debug\NuXJScript.exe -s <goodtests.js >goodcheck.txt
fc goodcheck.txt goodresults.txt
del goodcheck.txt
rem echo.
rem echo ### BAD TESTS ###
rem echo.
rem .\Debug\ImpDTest <badtests.txt >badcheck.txt
rem fc badcheck.txt badresults.txt
rem del badcheck.txt
