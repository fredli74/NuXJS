@echo off
echo.
echo ### GOOD TESTS ###

echo.
.\Debug\ImpDTest <goodtests.txt >goodcheck.txt
fc goodcheck.txt goodresults.txt
del goodcheck.txt
echo.
echo ### BAD TESTS ###

echo.
.\Debug\ImpDTest <badtests.txt >badcheck.txt
fc badcheck.txt badresults.txt
del badcheck.txt
