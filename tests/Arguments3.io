> function f1(a,b,c) { print(a); print(arguments[0]); a = 7; print(a); print(arguments[0]); arguments[0] = 5; print(a); print(arguments[0]); Object.defineProperty(arguments, '0', { value: 'z', writable: false, enumerable: false }); print(a); print(arguments[0]); a = 9; print(a); print(arguments[0]); print('-'); for (i in arguments) print(i); print('-'); }
-
> function f2(a,b,c) { print(a); print(arguments[0]); a = 7; print(a); print(arguments[0]); arguments[0] = 5; print(a); print(arguments[0]); Object.defineProperty(arguments, '0', { value: 'z', writable: true, enumerable: true }); print(a); print(arguments[0]); a = 9; print(a); print(arguments[0]); print('-'); for (i in arguments) print(i); print('-'); }
-
> f1(1234,2345,3456)
< 1234
< 1234
< 7
< 7
< 5
< 5
< 5
< z
< 9
< z
< -
< 1
< 2
< -
-
> f2(1234,2345,3456)
< 1234
< 1234
< 7
< 7
< 5
< 5
< 5
< z
< 9
< z
< -
< 0
< 1
< 2
< -
-
