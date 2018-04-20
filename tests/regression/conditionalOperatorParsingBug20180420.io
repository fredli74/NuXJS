> print(true?3:false?7:9)
< 3
-
> print((true?3:false)?7:9)
< 7
-
> print(true?3:(false?7:9))
< 3
-
> print(true?3:false?true?2:5:9)
< 3
-
> print(false?3:false?true?2:5:9)
< 9
-
> print(false?3:true?true?2:5:9)
< 2
-
> false?a=3:a=6; print(a)
< 6
-
> false?a=3:a=6,true?b=9:b=12; print(a); print(b)
< 6
< 9
-
> true?a=3:a=6,false?b=9:b=12; print(a); print(b)
< 3
< 12
-
> false?a=c=3:a=c=6,true?b=d=9:b=d=12; print(c); print(d)
< 6
< 9
-
> true?a=c=3:a=c=6,false?b=d=9:b=d=12; print(c); print(d)
< 3
< 12
-
