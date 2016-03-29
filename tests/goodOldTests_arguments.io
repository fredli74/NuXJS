> (function() { print(delete arguments); print(typeof arguments); })();
< true
< object
-
> f=function() { var a = arguments; var arguments; var b = arguments; var arguments = 3; var c = arguments; print(a==b); print(typeof a); print(c) }; f();
< true
< object
< 3
-
> f=function() { print(arguments[0]); print(arguments[1]); print(arguments[2]); }; f(1,'b',true);
< 1
< b
< true
-
> f=function(a,b,c,d,e) { print(arguments[0]); print(arguments[1]); print(arguments[2]); }; f(1,'b',true);
< 1
< b
< true
-
> f=function() { for (var i = 0, l = arguments.length; i < l; ++i) print(arguments[i]) }; f('ole', 'dole', 'doff');
< ole
< dole
< doff
-
> f=function() { g = function () { print(arguments[0]); print(arguments[1]); }; return g; }; f('a')('b');
< b
< undefined
-
> print('');
> f=function() { for (var i in arguments) print(i) }; f('1', '2', '3', '4');
> print('');
< 
< 0
< 1
< 2
< 3
< 
-
> f=function(a,b,c,d,e) {
> print(arguments[0]);
> a = 111;
> print(arguments[0]);
> arguments[0] = 222;
> print(a);
> delete arguments[0];
> print(arguments[0]);
> arguments[0] = 333;
> print(arguments[0]);
> print(a);
> print (arguments.length);
> arguments.length = 100;
> print(arguments.length);
> arguments.length = 0;
> print(arguments.length);
> print(arguments[1]);
> };
> f(1,'b',true);
< 1
< 111
< 222
< undefined
< 333
< 222
< 3
< 100
< 0
< b
-
> print('');
> f = function() { for (arguments in "abcd") print(arguments); }; f();
> print('');
< 
< 0
< 1
< 2
< 3
< 
-
> f = function() { for (var arguments = 3 in "abcd") print(arguments); }; f();
> print('');
< 0
< 1
< 2
< 3
< 
-
> (function() { print(typeof arguments); function arguments(a,b,c,d) { }; print(arguments.length) })();
< function
< 4
-
> (function() { print(typeof arguments); eval("function arguments(a,b,c,d) { }"); print(arguments.length) })();
< object
< 4
-
