MyObject = function() { }; o = new MyObject; o.name = "magnus"; o.age = 41;

for (i in o) print(i);
 
o = new MyObject; o.qwerty = "qwerty"; o.zxcv = "zxcv"; o.asdf = "asdf"; for (i in o) print(i);
 
print("");
for (i in o) { if (i >= "m" && i < "s") continue; print(i); }
print("");
 
print("");
for (i in o) { if (i >= "m" && i < "s") break; print(i); }
print("");
 
for (i in o) { print(i); o[i + " 3"] = o[i] + " 3" }
 
for (i in o) { print(i); o[i + " 4"] = o[i] + " 4" }
 
for (i in o) print(i);

print('');
f=function() { arguments["extra"] = null; for (var i in arguments) print(i) }; f('1', '2', '3', '4');
print('');

print('');
f=function() { delete arguments[1]; for (var i in arguments) print(i); arguments[1] = 'back'; print(''); for (var i in arguments) print(i); }; f('1', '2', '3', '4');
print('');

print('');
f=function() { delete arguments.length; arguments.length = 'back'; for (var i in arguments) print(i) }; f('1', '2', '3', '4');
print('');

print("*** regression tests ***");
 
// gc-protect temporary object during method call
(new (function(){this.a=function() { gc(); print(this.b) }; this.b="yeah" })).a()

// non-standard evaluation order for implicit "to primitive" conversions
var v={toString:function(){print("toString()");return "abcd"},valueOf:function(){print("valueOf()");return 1234;}};
print(v[v]=+v);
print(v[v]++);
print(++v[v]);
