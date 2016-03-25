> print(Error.name)
< Error
> print(SyntaxError.name)
< SyntaxError
> delete SyntaxError.name
> print(SyntaxError.name)
< 
> print(Error.prototype.name)
< Error
> print(Error.isPrototypeOf(SyntaxError))
< false
> print(Error.prototype.isPrototypeOf(SyntaxError))
< false
