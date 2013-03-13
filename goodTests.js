// function print(x) { console.log(x) }
// function time(f) { var start = Date.now(); f(); print("elapsed: " + (Date.now() - start)); }

print("*** comments ***")

  var           
      i = 0 /* comment */ ; print(i);  // more comment
 j = 1234.5 /* multi
 line
 comment
 */

print(j);

print("*** numbers ***")

print(1+1 == 2)
print(1.2+1.5 == 2.7)
print(1-1 == 0)
print(1-2 == -1)
print(1-2.5 == -1.5)
print(8*6==48)
print(-8*6==-48)
print(8*-6==-48)
print(-8*-6==48)
print(-(-8*-6)==-48)
print(10/2 == 5)
print(-9/2 == -4.5)
print(6.4%4.2 == 2.2)
print( 5 * 7 % 3  == 2)
print( (5 * 7) % 3  == 2)
print( 5 * (7 % 3)  == 5)
print(3*(9-3) == 18)
print(3*9-3 == 24)
print(3-9*3 == -24)
print(-3-(-9)*-3 == -30)
print(  -3  -  -9  *  -3   == -30)
print( +   55  == 55)

print("*** more numbers ***")

var x=3
print(x+9*5 == 48)
print(0 == 0)
print(0.0 == 0)
print(.0 == 0)
print(0. == 0)
print(0.e0 == 0)
print(0.0e0 == 0)
print(.0e0 == 0)
print(.0E0 == 0)
print(.0E+0 == 0)
print(.0E-0 == 0)
// print(1.0E-3 == 0.001) // FIX : accuracy problem
print(1.0E+3 == 1000.0)
print(0.000000000001 == 0.000000000001)
print(x == 3)
print(x == 3.0)
print(x == 3.0e0)
// print(x == 0.003e3) // FIX : accuracy problem
print(!(x == 2.9))
print(!(x != 3))
print(!(x != 3.0))
print(x != 3.1)
print(!(x < 3))
print(x < 3.1)
print(x <= 3)
print(x <= 3.1)
print(!(x <= 2.9))
print(!(x > 3))
print(x > 2.9)
print(x >= 3)
print(!(x >= 3.1))
print(x >= 2.9)
print((x == 3 ? 'a' : 'b') == 'a')
print((x == 3.1 ? 'a' : 'b') == 'b')
print((x == 3.0 ? (x >= 3.0 ? 'a' : 'b') : 'c') == 'a')
print((x != 3.0 ? (x >= 3.0 ? 'a' : 'b') : (x > 3.0 ? 'c' : 'd')) == 'd')

print("*** integers ***")

print(1 << 5 == 32)
print(1 << 37 == 32)
print(32 >> 5 == 1)
print(32 >> 37 == 1)
print(32 << -23 == 16384)
print(16384 >> -23 == 32)
print(-1 << 5 == -32)
print(-1 << 37 == -32)
print(-32 >> 5 == -1)
print(-32 >> 37 == -1)
print(-32 << -23 == -16384)
print(-16384 >> -23 == -32)
print(-16384 >>> 23 == 511)
print(-16384 >>> -23 == 8388576)
print(~~16384.9 == 16384);
print(~~-16384.9 == -16384);
print((-16384.9)>>0 == -16384);
print(1<<31 == -2147483648);
print(0xCFCFCFCF == 3486502863);
print(-0xCFCFCFCF == -3486502863);
print(~~0xCFCFCFCF == -808464433);
print((80189565849)>>0 == -1414812775);
print(-808464433>>>0 == 0xCFCFCFCF);
print(1.0/0.0); // Should be Infinity
print(-1.0/0.0);  // Should be -Infinity
print(~~(1.0/0.0) == 0);
print(~~(-1.0/0.0) == 0);
print(+"q"); // Should be NaN
print(~~(+"q") == 0);

var i = 0 ;   while (   i   <   100.0e+1   )  {  j = j + i ; ++ i  }
 do { j = j - i * 0.5 } while ( i ++ < 2000 )
 print (  i + ", " + j )  ;

var i=0;for(j=1234.5;1E3>i;)j+=i,++i;do j-=0.5*i;while(2E3>i++);print(i+", "+j);

print("*** special numbers ***")

// FIX : infinity etc

o = new (function(){});
print(o.a+0) // NaN
garlicNan=o.a+0
cheeseNan=o.b+0
print(garlicNan<cheeseNan) // NaN comparisons are always false
print(garlicNan<=cheeseNan) // NaN comparisons are always false
print(garlicNan==cheeseNan) // NaN comparisons are always false
print(garlicNan===cheeseNan) // NaN comparisons are always false
print(garlicNan>=cheeseNan) // NaN comparisons are always false
print(garlicNan>cheeseNan) // NaN comparisons are always false
print(garlicNan!=cheeseNan) // but this is true
print(garlicNan!==cheeseNan) // but this is true

print("*** booleans ***")

if (false && false || true && true) print("ok"); else print("oops");
if (!(false && true || true && false)) print("ok"); else print("oops")
if (!(true && false || false && true)) print("ok"); else print("oops");
if (false && true || true && false || !(false && false)) print("ok"); else print("oops")
if ((3<5 && 5<3 || 2==3 && 2==2) || !(3>4 || 7<5)) print("ok"); else print("oops");
if ((3<5 && 5<3 || 2==3 && 2==2) || !(3>4 || 5<7)) print("oops"); else print("ok");
print ((3<5 && 5<3 || 2==3 && 2==2) || !(3>4 || 7<5) ? ("ok") : ("oops"));

print("*** strings ***")

print("abcdefgh"[0]);
print("abcdefgh"[4]);
print("abcdefgh"[8]);
print("abcdefgh"[-1]);
print("abcdefgh".length);
s = "abcdefgh"; s = s[0] + s[3] + s[7] + s[1]; print(s);
s = "0123456789"; print(s.length); s.length = 0; print(s.length); // both output 10, string lengths are not writable

print("*** objects ***")

MyObject = function() { }; o = new MyObject; o.name = "magnus"; o.age = 41;
print("name" in o);
print("age" in o);
print("nope" in o);
print(o in o);
print("o" in this);

print.extended = "added to built-in";
print(print.extended);

print("*** types ***")

print((function() { print(x); var x; print(x); x = void print("turn to void"); return x })());

(function() { var f = function() {
	var a;
	print(typeof a);
	print(typeof null);
	print(typeof false);
	print(typeof 3.5);
	print(typeof "zzz");
	print(typeof f);
	print(typeof "zzz"[1]);
	print(typeof "zzz"[5]);
	print(typeof doesNotExist);
	doesExist = "I exist";
	print(typeof doesExist);
}; f(); })();

print("*** functions ***")

f = function() { }
var a = 1 , b = 2 , c , d = 1 in f ; print(a + ", " + b + ", " + c + ", " + d);

f = function ( a , b , c ) { print("a:" + a + ", b:" + b + ", c:" + c) }
f ( 'first', 2, true )

 var fib = function ( y ) { if ( y >= 2 ) return fib ( y - 1 ) + fib ( y - 2 ) ; else return y ; };
print(fib(15));

var fib=function(a){return 2<=a?fib(a-1)+fib(a-2):a};print(fib(15));

f=function(n) { var i = 0; while (i < n) { f[i] = i * 1234; ++i } }
f(1000)
print(f[0])
print(f[999])
print(f[1000])
print(f[-1])

x=9;(function() { var y=3,x=5 })(); print(x);

f = function(a,b,c) { }
print(f.length)
f.length = 0
print(f.length);

f = function() { }
print(f.length);

(function() { function  f (  )  {  print (  "hej"  ) }; f() })();
(function() { print(typeof g); function g() { } })(); // function
(function() { var g; print(typeof g); function g() { } })(); // function
(function() { print(typeof g); var g = 23; print(typeof g); function g() { } })(); // function, number
(function() { print(g.length); function g(a,b) { }; function g(a,b,c,d) { } })(); // 4
// FIX : js doesn't like -> (function() { print(g.length); if (false) function g(a,b) { }; if (false) function g(a,b,c,d) { } })(); // 4
print("del: " + delete g);
(function() { g = "qwerty"; function g(a,b,c,d) { }; print(g); print("del: " + delete g); })(); // qwerty, del: false
print("hasg: " + ("g" in this));

/*print("del: " + delete f); // true
print("hasf: " + ("f" in this)); // hasf: false
(function f() { f=3; print(typeof f); })(); // function
(function f() { eval("f=3"); print(typeof f); })(); // function
print("hasf: " + ("f" in this)); // hasf: false
(function() { f=3; print(typeof f); })(); // number
print("hasf: " + ("f" in this)); // hasf: true
print("del: " + delete f); // true
(function f() { print(typeof f); var f=3; print(typeof f); })(); // undefined, number
(function f() { print("del: " + delete f); print(typeof f); })(); // del: false, function
(function f() { print("del: " + delete f); var f = 27; print("del: " + delete f); print(typeof f); })(); // del: false, del: false, number
(function f() { print("del: " + delete f); f = 27; print("del: " + delete f); print(typeof f); })(); // del: false, del: false, function
print("hasf: " + ("f" in this)); // hasf: false
(function f() { function f(a,b) { }; print(f.length); })(); // 2
(function f() { function f(a,b) { }; print("del: " + delete f); print(f.length); })(); // del: false, 2
(function f() { print("del: " + delete f); f = 27; print("del: " + delete f); function f(a,b) { }; print(f.length); })(); // del: false, del: false, undefined
print("hasf: " + ("f" in this)); // hasf: false
(function f() { print("del: " + delete f); f = 27; print("del: " + delete f); function f(a,b) { }; print(typeof f); })(); // del: false, del: false, number
print("hasf: " + ("f" in this)); // hasf: false
(function f() { print(f.length); function f(a,b,c,d) { }; print(f.length); })(); // 4, 4*/

print("*** for statement ***")

s = "";
for ( x = 0 ; x < 10 ; ++ x ) s += x + ","
print(s)
s = "";
for ( ; x < 20 ;) { s += x + ","; ++x };
print(s)

s = "";
for (var x = 3, y = 5 ; x < 100 && y < 15 ; ++y) { s += y + "," };
print(s)

s = "";
for (var x = 1, y = 1 ; x < 100 ; x += (x < 50 ? 10 : 20), ++y) { if (x > 20) s += x + "," + y + "," };
print(s)

print("*** new statement ***")

f=function(a, b) { this.a=a; this.b=b; this.c="cee"; this.dump=function() { print("a: " + this.a  + ", b: " + this.b + ", c: " + this.c); } }
x = new f
x.dump()

g=function(c) { this.c=c }
g.prototype = x;
y = new g;
y.dump();
y = new g("hey")
y.dump();

x = new f ( "aah" , "bee" )
x.dump()

g=function(c) { this.c=c; this.d="dee" }
g.prototype = x;
y = new g("hey")
y.dump();

print("a" in x);
print("b" in x);
print("c" in x);
print("d" in x);

print("a" in y);
print("b" in y);
print("c" in y);
print("d" in y);
print("e" in y);

h=function(e) { this.e=e; }
h.prototype = y;
z = new h("yo!")
x.a="changed"
z.dump();

print("a" in z);
print("b" in z);
print("c" in z);
print("d" in z);
print("e" in z);

g=function(c) { this.c=c; this.d="dee" }
g.prototype = "not valid";
y = new g("hey")
print("a" in y);
print("b" in y);
print("c" in y);
print("d" in y);


print("*** delete statement ***")

delete z;
print(function() { z=3; return delete z }())
print(function() { return delete f.z }())
print(function() { return delete "i"[0] }())
print(function() { var z=3; return delete z }())
print(function() { var z=3; var g=function() {  return delete z; } ; return g; }()())
print(delete "constant is always ok");

(function() { var x=new(function(){}); x.a="a"; var b=function(){}; b.prototype=x; var c=new b; c.a="b"; print(c.a); delete c.a; print(c.a) })();

print("*** break & continue ***")

{ var i = 0; while (++i < 5) { print("before " + i); if (i > 2) continue; print("after " + i) } }
{ var i = 0; while (++i < 5) { print("before " + i); if (i > 2) break; print("after " + i) } }
{ var i = 0; a : while (++i < 5) { print("before " + i); if (i > 2) continue a; print("after " + i) } }
{ var i = 0; a : while (++i < 5) { print("before " + i); if (i > 2) break a; print("after " + i) } }
{ var i = 0; a : b:while (++i < 5) { print("before " + i); if (i > 2) continue a; print("after " + i) } }
{ var i = 0; a : b:while (++i < 5) { print("before " + i); if (i > 2) continue b; print("after " + i) } }
{ var i = 0; a : { b:while (++i < 5) { print("before " + i); if (i > 2) break a; print("after " + i) } } }
{ var i = 0; a : { b:while (++i < 5) { print("before " + i); if (i > 2) break b; print("after " + i) } } }
{ x = 0; var i = 0; a: while (i < 5) { ++i; j=0; b:while(j++ < 3) { print("iter " + i + "," + j); if (++x < 3) continue b; } } }
{ x = 0; var i = 0; a: while (i < 5) { ++i; j=0; b:while(j++ < 3) { print("iter " + i + "," + j); if (++x < 3) continue a; } } }
{ x = 0; var i = 0; a: while (i < 5) { ++i; j=0; b:while(j++ < 3) { print("iter " + i + "," + j); if (++x > 4) break b; } } }
{ x = 0; var i = 0; a: while (i < 5) { ++i; j=0; b:while(j++ < 3) { print("iter " + i + "," + j); if (++x > 4) break a; } } }
{ x = 0; b: do { y = 0; a: do { print('before ' + y + ',' + x); if (y > 2) continue b; print('after ' + y + ',' + x);  } while (++y < 5); } while (++x < 5) }
{ x = 0; b: do { y = 0; a: do { print('before ' + y + ',' + x); if (y > 2) continue a; print('after ' + y + ',' + x);  } while (++y < 5); } while (++x < 5) }

f = function() { z: print("hej"); z: for (var i = 0; i < 10; ++i) { b: while (true) r: for (var j = 0; j < 10; ++j) { if (j >= 5) break b; print(j) }; print("thonk") } }
f()

{ a: { for (var x = 1; x < 10; ++x) for (var y = 1; y < 3; ++y) { print(x); if (x == 5) break a }; print("not here") } }

{ x = 0; a: for (var i = 0; i < 10; ++i) { if (++x < 8) continue a; print("iter " + i) } }

print("*** try, catch, finally ***")

a: try { print("a0"); b: { try { print("b0"); break b; print("b1") } finally { print("bf") } }; print("a1"); } finally { print("af") }
a: try { print("a0"); b: { try { print("b0"); break a; print("b1") } finally { print("bf") } }; print("a1"); } finally { print("af") }
var s = ''; for (var i = 0; i < 10; ++i) { try { if (i == 8) break; if (i > 5) continue; s += i } finally { s += "." } s += ','; }; print(s);
a: try { print("a"); } finally { break a; print("b") }
a: try { print("a"); } finally { try { break a; } finally { print("c") }; print("b") }
a: try { print("a"); } finally { try { } finally { break a; print("c") }; print("b") }
print((function() { try { return "du"; } finally { print("hej") } })());
print((function() { try { print("a"); } finally { try { return "r"; } finally { print("c") }; print("b") } })());
print((function() { try { print("a"); } finally { try { } finally { return "r"; print("c") }; print("b") } })());
print((function() { a: try { return 1 } finally { break a; print(2) } })());
print((function() { try { return 1 } finally { return 3; print(2) } })());
print(function() { var x = 0; try { return ++x; } finally { x++; print(x) } }())
print(function() { var x = 0; try { return ++x; } finally { x++; return(x) } }())
print(function() { var x = 0; try { return ++x; } finally { return ++x } }())
try { (function() { try { throw "x" } finally { try { throw "y" } catch ( x ) { print(x) } } })() } catch (x) { print(x) }

thrower=function(a) { print("throwing: " + a); throw ("thrown: " + a); print("not here"); }
print((function() { b: try { thrower("throw me") } finally { print("fake catcher") ; break b }; return("ok then...") })());
print((function() { try { try { thrower("throw me") } finally { print("fake catcher") ; thrower("throwed again"); }; } finally { return("ok then...") } })());

(function() { var y = null; var x = 3; try { throw 7 } catch (x) { print(x); try { thrower(9) } catch (x) { print(x); }; x=5; print(x); print(delete x); y = function() { print(x) } }; y(); print(x) })();

((function() { try { x: { try { throw "oops" } finally { print("finally") } } } catch (x) { print("caught: " + x) } })());
// finally
// caught: oops

((function() { try { x: { try { break x; thrower("oops") } finally { print("finally") } } } catch (x) { print("caught: " + x) } })());
// finally

((function() { try { x: { try { throw "oops" } finally { print("finally"); break x } }; print("broken") } catch (x) { print("caught: " + x) } })());
// finally
// broken

((function() { try { x: while (true) { try { break x; thrower("oops") } finally { print("finally") }; print("nope") } } catch (x) { print("caught: " + x) } })());
// finally

print((function() { try { return "hej" } finally { print("finally") } })());
// finally
// "hej"

((function() { b: try { return "hej" } finally { print("finally"); break b }; print("didn't return") })());
// finally
// didn't return

print((function() { var x = 1; b: try { return ++x } finally { print("finally " + x) } })());
// finally 2
// 2

print((function() { var x = 1; b: try { return x++ } finally { print("finally " + x) } })());
// finally 2
// 1

print("*** switch ***")

for(var x=-3;x<=3;++x)switch(x+3){case 1:case 2:case 1+2:print(x)}

f = function(n) { print('test:' + n); };
for (var x = -3; x <= 3; ++x) switch (  x + 3  )   { case 1  : case 2  : case f ( x )   , 1 + 3  : print(x) }

switch (3) { case 2: print("NO"); default: print("HEJ"); case 3: print("3") }

switch ("1") { case 1: print('1'); case "1": print('"1"'); case 2.0: print("continues to 2"); break; case "3": print("but not to 3"); case "1": print('Second "1" is not executed'); default: print("Neither is default") }

switch (1.0) { case 0: print('0'); default: print('default'); case "1.0": print('1'); case "1": print('"1"'); case 2.0: print("continues to 2"); break; case "3": print("but not to 3"); case "1": print('Second "1" is not executed'); }

for (var i = 0; i < 10; ++i) { switch (i) { case 5: continue }; print(i) }

print("*** for in ***")

s = ''; for ( a in "abcdefgh" ) s += a + ','; print(s);
s = ''; for ( var a in "abcdefgh" ) s += a + ','; print(s);
s = ''; for ( var a = 100 in "abcdefgh" ) s += a + ','; print(s);
s = ''; for ( var a = 100 in "" ) ; print(a);
a = new MyObject; for (a.x in "abcd") print(a.x)
a = new MyObject; x = 0; for (a[x++] in "abcd") { }; print(x); for (var i = 0; i < x; ++i) print(a[i]);
a = new MyObject; f = function() { print("tick "); return 0 }; for (a[f()] in "abcd") { print (a[0]) }

print("*** closures ***")

a=function() { b = function() { c = function() { return d++ }; return c; }; var d = 23 ; return b }
b=a()()
print(b());
print(b());
print(b());
print(b());

print("*** arguments ***")

f=function() { var a = arguments; var arguments; var b = arguments; var arguments = 3; var c = arguments; print(a==b); print(a); print(c) }; f();

f=function() { print(arguments[0]); print(arguments[1]); print(arguments[2]); }; f(1,'b',true);

f=function(a,b,c,d,e) { print(arguments[0]); print(arguments[1]); print(arguments[2]); }; f(1,'b',true);

f=function() { for (var i = 0, l = arguments.length; i < l; ++i) print(arguments[i]) }; f('ole', 'dole', 'doff');

f=function() { g = function () { print(arguments[0]); print(arguments[1]); }; return g; }; f('a')('b');

print('');
f=function() { for (var i in arguments) print(i) }; f('1', '2', '3', '4');
print('');

f=function(a,b,c,d,e) {
	print(arguments[0]);
	a = 111;
	print(arguments[0]);
	arguments[0] = 222;
	print(a);
	delete arguments[0];
	print(arguments[0]);
	arguments[0] = 333;
	print(arguments[0]);
	print(a);
	print (arguments.length);
	arguments.length = 100;
	print(arguments.length);
	arguments.length = 0;
	print(arguments.length);
	print(arguments[1]);
};
f(1,'b',true);

print('');
f = function() { for (arguments in "abcd") print(arguments); }; f();
print('');

f = function() { for (var arguments = 3 in "abcd") print(arguments); }; f();
print('');

(function() { print(typeof arguments); function arguments(a,b,c,d) { }; print(arguments.length) })();

print("*** value conversion ***");
/*
String comparison can be forced by: "" + a == "" + b.
Numeric comparison can be forced by: +a == +b.
Boolean comparison can be forced by: !a == !b.
NOTE 2 The equality operators maintain the following invariants:
 
A != B is equivalent to !(A == B).
A == B is equivalent to B == A, except in the order of evaluation of A and B.
 
new String("a") == "a" and "a" == new String("a")are both true.
new String("a") == new String("a") is false.
 
*/

o=new (function() { this.valueOf = function() { return 33 } } )
print(+o)

/*
OneHundredEleven = function() { this.valueOf = function() { print("click"); return 111 }; this.toString = null; }; o = new OneHundredEleven; print(+o)
TestObject = function() { };
TestObject.prototype = new OneHundredEleven;
o2 = new TestObject; print(+o2)
 
AString = function() { this.valueOf = null; this.toString = function() { print("good toString"); return "stringy"; } };
o = new AString; print(+o)
 
StringAgain = function() { this.valueOf = function() { print("bad valueOf"); return this }; this.toString = function() { print("good toString"); return "stringy"; } };
o = new StringAgain; print(+o)
 */
/*
i = 0;
f = function(s) { this.valueOf = function() { print(s); return ++i; } };
a = new f('a'); b = new f('b'); c = new f('c');
print(a+b+c);
print(a-b-c);
print(a-(b-c));
print(a+b*c);
print(a*b+c);
*/

print("*** eval ***");

print(eval("3"))

print(eval("3+3"))

print(eval("3+3;;;;"))

print(eval("3+3;;;{}"))

print(eval(";;{3+3};;"))

print(eval("aGlobal=1"))

print(eval("aGlobal"))

print(eval("aGlobal=new (function(){this.a='a string'})"))

print(eval("aGlobal.a"))

print(eval("if (true) 3"))

print(eval("if (false) 3"))

print(eval("if (true) 3+3"))

print(eval("if (false) 3+3"))

print(eval("if (true) 3+3; else 6-6"))

print(eval("if (false) 3+3; else 6-6"))

print(eval("aGlobal; if (false) 3"))

print(eval("b: { 1; break b }"))

print(eval("b: if (true) { 1; break b; 2 }"))

print(eval("3; b: if (false) { 123; if (false) { 1; break b; 2 }}"))

print(eval("3; b: if (true) { 123; if (false) { 1; break b; 2 }}"))

print(eval("3; b: if (false) { 123; if (true) { 1; break b; 2 }}"))

print(eval("3; b: if (false) { 123; if (false) { 1; break b; 2 }}"))

print(eval("5; while (false) 7;"))

print(eval("5; while (true) { 3; break }"))

print(eval("var i = 0; 234; while (i++ == 0) i * 100"))

print(eval("var i = 0; 234; while (i++ == 0) { if (i == 1) continue; i * 100 }"))

print(eval("var i = 0; 234; while (++i < 10) { 23; continue }"))

print(eval("var i = 3; 234; while (++i < 10) { j = 5; while (++j < i) j * 3 }"))

print((function() { var x = 33; eval("x = 55"); return x })());

print((function() { var x = 33; eval("var x = 55"); return x })());

print((function() { eval("if (false) var x = 23"); return x })());

(function() { var x = 88; try { throw "k" } catch (x) { eval("print(x); var x = 5"); print(x) }; print(x) })();

(function() { var x = 88; try { throw "k" } catch (x) { eval("print(x); x = 5"); print(x) }; print(x) })();

aGlobal = "ok"; (function() { eval("var aGlobal = 5"); print(delete aGlobal); print(aGlobal) })(); print("aGlobal" in this);
print("");

aGlobal = "ok"; (function() { eval("var aGlobal = 5"); print(delete aGlobal); print(delete aGlobal); })(); print("aGlobal" in this);
print("");

(function() { var x = 77; eval("var x = 5"); print(delete x); print(x) })();
print("");

(function() { var f = 33; eval("function f() { print('ohboy') }"); f(); })();

(function() { var f = 33; eval("f=18; function f() { print('ohboy') }"); print(f); })();

(function() { var f = 33; eval("function f() { print('ohboy') }; f=18"); print(f); })();

print("anotherGlobal in this: " + ("anotherGlobal" in this));
eval("if (false) var anotherGlobal = 'this creates a global'");
print("anotherGlobal in this: " + ("anotherGlobal" in this));

var y = -100; var a=(function() { var z = 3; var y = 2; var x = eval; z = x("var y = 100; (function() { print('tick ' + ++y); return y })"); print(y); return z })(); print(a()); print(a()); print(y); print("");

var y = -100;
var a=(function() { var z = 3; var y = 2; z = eval("var y = 100; (function() { print('tick ' + ++y); return y })"); print(y); return z })();
print(a());
print(a());
print(y);
print("");

var y = -100;
var a=(function() { var z = 3; var y = 2; z = eval("y = 100; (function() { print('tick ' + ++y); return y })"); print(y); return z })();
print(a());
print(a());
print(y);
print("");

var y = -100;
var a=(function() { var z = 3; var y = 2; var x = eval; z = x("y = 100; (function() { print('tick ' + ++y); return y })"); print(y); return z })();
print(a());
print(a());
print(y);
print("");

this.isGlobal = "yeah, is global";
var o = new (function() { this.eval = eval; this.isGlobal = "no, is not global"; this.test1 = function() { this.eval("print(this.isGlobal)")
		this.test2 = function() { eval("print(this.isGlobal)") } } });
// o.eval("print(this.isGlobal)"); // FIX : jsc doesn't like this
// o.test1(); // FIX : jsc doesn't like this
// o.test2();

print("*** fun stuff ***");

mandelbrot = function() {
	var I, R, s, b, n;
	for(var i=-1, s=''; i<1; i+=0.06, print(s), s='') {
		for (var r=-2;(I=i,(R=r)<1);r+=0.03,s+=String.fromCharCode(n+31)) {
			for(n=0;(b=I*I,26>n++&&R*R+b<4);I=2*R*I+i,R=R*R-b+r);
		}
	}
}
mandelbrot()

