> function f(x) { var y = (x > 0 ? f(x - 1) : 0); return x * 10 + y; }
> print(f(1000))
< 5005000
-
> print(f(10000))
< 500050000
-
> print(f(100000))
< 50000500000
-
> print(f(1000000))
< 5000005000000
-
