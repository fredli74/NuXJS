> function f() { print(arguments[0]); print(arguments["0"]); print(arguments[" 0 "]); print(arguments["0.0"]); }
> f(1234)
< 1234
< 1234
< undefined
< undefined
-
