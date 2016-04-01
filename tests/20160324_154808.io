> SyntaxError.name
> print(SyntaxError.name)
< SyntaxError
-
> print(SyntaxError.name=3)
< 3
-
> print(SyntaxError.name)
< 3
-
> print(delete SyntaxError.name)
< true
-
> print(SyntaxError.name)
< Error
-
> function test() { }
> print(test.name)
< test
-
> test.name=1345
> print(test.name)
< test
-
> print(delete test.name)
< true
-
> print(test.name)
> test.name=3
> print(test.name)
< 
< 3
-
