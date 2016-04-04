> var x = 0;
>   var innerX = (function() {
>     // If we were to conform strictly to ES spec, the left-hand side of th assigment is a reference to the outer x.
>     x = (eval("var x;"), 1);
>     return x;
>   })();
> print(typeof innerX);
> print(x);
// Strict ES conformance:
// < undefined
// < 1
< number
< 0
-
