> function f() { print(typeof f); var f = 3; print(typeof f); };
-
> f();
< undefined
< number
-
> function g() { print(typeof g); eval("var g = 3"); print(typeof g) };
-
> g();
< function
< number
-
> (function f() { print(typeof f); var f = 3; print(typeof f); })();
< undefined
< number
-
> (function g() { print(typeof g); eval("var g = 3"); print(typeof g) })();
< function
< number
-
> (function f() { print(typeof f); print(delete f); print(typeof f); })();
< function
< false
< function
-
