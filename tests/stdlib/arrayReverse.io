> var a = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
> print(a.toString())
< 1,2,3,4,5,6,7,8,9,10
-
> a.reverse()
> print(a.toString())
< 10,9,8,7,6,5,4,3,2,1
-
> var a = [ 1, 2, 3, 4, 5, 6, 7, 8, 9 ];
> print(a.toString())
< 1,2,3,4,5,6,7,8,9
-
> a.reverse()
> print(a.toString())
< 9,8,7,6,5,4,3,2,1
-
> var a = [ ]
> a.reverse()
> print(a.toString())
< 
-
> var a = [ 1,,,3,,,,,,undefined,'end' ];
> print(a.toString())
< 1,,,3,,,,,,,end
-
> a.reverse()
-
> print(a.toString())
< end,,,,,,,3,,,1
-
> print(typeof a[1])
< undefined
-
> print(2 in a)
< false
-
> print(a.reverse().toString())
< 1,,,3,,,,,,,end
-
