> callbackTest(1,2,3,function() { print("cb 4") },5)
< 1
< 2
< 3
< cb 4
< 5
-
> callbackTest(1,2,3,function() { print("cb 4") },5,callbackTest)
-
> callbackTest(1,2,3,function() { print("cb 4") },5,function f() { callbackTest(f) })
-
