> var a = [ 1, 2, 3, 4, 5, 6, 7 ];
> print(a.shift());
< 1
-
> print(a.toString());
< 2,3,4,5,6,7
-
> while (a.length > 0) print(a.shift());
< 2
< 3
< 4
< 5
< 6
< 7
-
> print(a.toString());
< 
-
> a = [ 1,,,3,,undefined,5 ];
> a.shift(); print(a.shift());
< undefined
-
> print(a.toString());
< ,3,,,5
-
> print(0 in a)
< false
-
> print((a.length - 1) in a)
< true
-
> print(a[a.length - 1])
< 5
-
> print((a.length - 2) in a)
< true
-
> print(a[a.length - 2])
< undefined
-
> while (a.length > 0) print(a.shift());
< undefined
< 3
< undefined
< undefined
< 5
-
> print(a.shift());
< undefined
-
> print(a.length);
< 0
-
