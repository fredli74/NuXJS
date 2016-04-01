> function f() { }
> print(f.name)
< f
> print(delete f.name)
< false
> print(f.name)
> print('name' in f)
< f
< true
> print(f.hasOwnProperty('name'))
< true
> print('name' in Function.prototype)
< true
> print(Function.prototype.name)
> print(Function.prototype.name==='')
< 
< true
> print(Math.sin.name)
< sin
> print(typeof Math.sin)
< function
> print(SyntaxError.name)
