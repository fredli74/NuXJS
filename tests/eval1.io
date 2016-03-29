> (function() { var x = 3; eval("var x = 5"); print(delete x); print(typeof x); print(x); })();
< false
< number
< 5
-
> (function() { eval("var x = 3"); eval("var x = 5"); print(delete x); print(typeof x); })();
< true
< undefined
-
> (function x() { eval("var x = 3"); eval("var x = 5"); print(typeof x); print(delete x); print(typeof x); })();
< number
< true
< function
-
