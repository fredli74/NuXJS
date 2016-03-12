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

(function() {
	var o={valueOf:function(){print("valueOf called"); return 1}}
	print(+o)
	print(o+o)
	function f(x) { print("function called"); return x }
	print(o+f(2))
	print(f(2)+o)
})();

i = 0;
f = function(s) { this.valueOf = function() { print(s); return ++i; } };
a = new f('a'); b = new f('b'); c = new f('c');
print(a+b+c);
print(a-b-c);
print(a-(b-c));
print(a+b*c);
print(a*b+c);

