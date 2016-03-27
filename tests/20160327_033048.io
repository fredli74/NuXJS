> a=[1,2,3,4]
-
> for( i in a) print(i)
< 0
< 1
< 2
< 3
-
> delete a[2]
-
> for( i in a) print(i)
< 3
< 0
< 1
-
> a[2]=5
-
> for( i in a) print(i)
< 0
< 1
< 2
< 3
-
> a.x='y'
-
> for( i in a) print(i)
< x
< 0
< 1
< 2
< 3
-
