if (!print) print = function(x) { console.log(x) }
// function time(f) { var start = Date.now(); f(); print("elapsed: " + (Date.now() - start)); }

print("*** parser ***");

  var           
      i = 0 /* comment */ ; print(i);  // more comment
 j = 1234.5 /* multi
 line
 comment
 */

print(j);

doThis=false;do.3;while(doThis);

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
print(4294967295.0|0); // Should be -1
print((-4294967295.0)|0); // Should be 1
print(-4294967295.0|0); // Should be 1
print(2147483648.0|0); // Should be -2147483648
print(-2147483648.0|0); // Should be -2147483648
print(4294967295>>>0); // Should be 4294967295
print((-4294967295)>>>0); // Should be 1

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

// FIX : test null, undefined etc

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
/*print("multi\
line string");*/
print("line\nfeed\ttab\'single quotes\'\n\"double quotes\" back\\slash  \b\b xxx \f \x40 \rcr");
print("\u1234"[0] === String.fromCharCode(1234));
print("\u1234"[0] === String.fromCharCode(0x1234));

// FIX : charCodeAt instead

print("*** objects ***")

emptyObject = {};
print(typeof emptyObject);

filledObject =   {  a  :   "b"   , 3  :   (5,10+23)   , ","  :   emptyObject != null ? "y":"n"  };
print(filledObject.a);
print(filledObject[3]);
print(filledObject[","]);

deepObject = { left : { l : "ll" , r:"lr" },right:{l:"rl",r:"rr"} };
print(deepObject.left.l); print(deepObject.left.r); print(deepObject.right.l); print(deepObject.right.r);

objectFromLiteral = { a : "this" , c : "is" , };
print(objectFromLiteral.a);
print(objectFromLiteral.c);

objectFromCall = Object();
objectFromCall.f = "expected";
objectFromCall.g = "behavior";
print(objectFromCall.f);
print(objectFromCall.g);

objectFromConstructor = new Object();
objectFromConstructor.h = "... and";
objectFromConstructor.i = "this too";
print(objectFromConstructor.h);
print(objectFromConstructor.i);

MyObject = function() { }; objectFromCustomConstructor = new MyObject; objectFromCustomConstructor.name = "magnus"; objectFromCustomConstructor.age = 41;
print("name" in objectFromCustomConstructor);
print("age" in objectFromCustomConstructor);
print("nope" in objectFromCustomConstructor);
// FIX : enable once we have toString in object.prototype
// print(objectFromCustomConstructor in objectFromCustomConstructor);
print("objectFromCustomConstructor" in this);

/* FIX :
Object.prototype.patch = "patched";
print(emptyObject.patch);
print(deepObject.patch);
print(objectFromLiteral.patch);
print(objectFromCall.patch);
print(objectFromConstructor.patch);
print(objectFromCustomConstructor.patch);

print(delete Object.prototype.patch);
*/

// had a bug with this...
(function() {
	var o = { a: 1 };
	o.a += 5;
	print(o.a++);
	print(++o.a);

	var a = {b:0};
	a.b=1;
	a.b+=1
	function fo(a,b) { var o = { }; o[a] = b; return o; };
	function i(j) { return j; };
	print(fo('q','g')[i('q')]);
	(o = fo('q','g'))[i('q')] = 5;
	print(o.q);
	(o = fo('q',3))[i('q')] += fo('z',5)[i('z')];
	print(o.q);
})();

print("*** arrays ***")

emptyArray = [   ]  ;
print(emptyArray.length);
print(emptyArray[0]);

var x = 100;
bigArray = [
	  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
	, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
	, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
	, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
	, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
	, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
 	, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1000 / x ];
(function() { for (var i = 0; i < bigArray.length; ++i) bigArray[i] = bigArray[i] * 1234; })();
print(bigArray.length);
print(bigArray[279]);
print(bigArray[280]);
bigArray[12345] = null;
print(bigArray.length);
bigArray[' 12346'] = 1;
print(bigArray.length);
print(bigArray[' 12346']);
print(bigArray[12346]);
bigArray['12346'] = "hola";
print(bigArray[12346]);
bigArray[-1.234] = 55;
print(bigArray[-1]);
print(bigArray[-1.234]);
bigArray.length = 3;
print(bigArray.length);
print(bigArray[2]);
print(bigArray[3]);
print(bigArray[12345]);

newedArray = new Array(1,2,3,4);
print(newedArray.length);
print(newedArray[0]);
print(newedArray[3]);

newedArray = new Array();
print(newedArray.length);

/* FIX : should throw
newedArray = new Array(-1);
print(newedArray.length);
print(newedArray[0]);
*/

newedArray = new Array(50);
print(newedArray.length);

/* FIX : should throw
newedArray = new Array(50.5);
print(newedArray.length);
print(newedArray[0]);
*/

print(Array.prototype.length);
print(delete Array.prototype.length);
Array.prototype.patch = "patched";
print(bigArray.patch);
print(delete Array.prototype.patch);

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
print("f.length=" + f.length);
print("delete f.length=" + delete f.length);
print("f.length=" + f.length);

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

(function f () { print ( typeof f ) } ) ();
(function f(n) { print(n); if (n < 10) f(n + 1); print(n) })(0)

print("del: " + delete f); // true
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
(function f() { print("del: " + delete f); f = 27; print("del: " + delete f); function f(a,b) { }; print(typeof f); })(); // del: false, del: false, number
print("hasf: " + ("f" in this)); // hasf: false
(function f() { print(f.length); function f(a,b,c,d) { }; print(f.length); })(); // 4, 4

print("typeof(function(){}): "+typeof(function(){}));
print("typeof(Function.prototype): "+typeof(Function.prototype));
print("Function.prototype.isPrototypeOf(function(){}): "+Function.prototype.isPrototypeOf(function(){}));
print("Function.prototype.isPrototypeOf(Function.prototype): "+Function.prototype.isPrototypeOf(Function.prototype));
// fix : print("Function instanceof Object: "+(Function instanceof Object));
// fix : print("Function instanceof Function: "+(Function instanceof Function));
// fix : print("(function(){}) instanceof Function: "+((function(){}) instanceof Function));
print("Function.prototype.length: "+Function.prototype.length);

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

print('delete "asdf".qwer: ' + (delete "asdf".qwer));
print('delete (123).qwer: ' + (delete (123).qwer));
print('delete (false).qwer: ' + (delete (false).qwer));

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

print("*** arguments ***");

(function() { print(delete arguments); print(typeof arguments); })();

f=function() { var a = arguments; var arguments; var b = arguments; var arguments = 3; var c = arguments; print(a==b); print(typeof a); print(c) }; f();

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

(function() { print(typeof arguments); eval("function arguments(a,b,c,d) { }"); print(arguments.length) })();

print("*** value conversion ***");

// Valids:
print(+"");
print(+"0");
print(+"000");
print(+"0123");
print(+"12345");
print(+"+12345");
print(+"-12345");
print(+"+12345.57685");
print(+"+12345.57685e10");
print(+"+12345.57685e+10");
print(+"+12345.57685e-10");
print(+"+12345.57685E-10");
print(+" +12345.57685E-10");
print(+"+12345.57685E-10 ");
print(+"  \n  \r +12345.57685E-10   \v \t ");
print(+"  \n  \r 0x0   \v \t ");
print(+"  \n  \r 0xDE   \v \t ");
print(+"  \n  \r 0xAA55AA55   \v \t ");
print(+"Infinity");
print(+"+Infinity");
print(+"-Infinity");

// Invalids:
print(+"  \n  \r -0xAA55AA55   \v \t ");
print(+"k");
print(+"5k");
print(+"+ 5");

function ValueObject(v) {
	this.valueOf = function() { return v };
};

print((new ValueObject(33)) + (new ValueObject(55)));
print((new ValueObject(33)) + (new ValueObject("55")));
print((new ValueObject("62456")) + (new ValueObject(23452)));
print((new ValueObject(76)) - (new ValueObject(23)));
print((new ValueObject(35)) * (new ValueObject("123")));
print((new ValueObject("1574.4")) / (new ValueObject(12.3)));
print((new ValueObject(329329)) % (new ValueObject(127)));
print((new ValueObject(55)) & (new ValueObject(27)));
print((new ValueObject(-439)) | (new ValueObject(33)));
print((new ValueObject(123)) ^ (new ValueObject(243)));
print((new ValueObject(4398)) << (new ValueObject(5)));
print((new ValueObject(129384)) >> (new ValueObject(3)));
print((new ValueObject(-348234)) >>> (new ValueObject(11)));
print((new ValueObject(1234)) < (new ValueObject(1235)));
print((new ValueObject(1234)) < (new ValueObject(1234)));
print((new ValueObject(1234)) <= (new ValueObject(1235)));
print((new ValueObject(1234)) <= (new ValueObject("1234.0")));
print((new ValueObject(1235)) > (new ValueObject(1234)));
print((new ValueObject(1234)) > (new ValueObject(1234)));
print((new ValueObject(1235)) >= (new ValueObject(1234)));
print((new ValueObject("1234.0")) >= (new ValueObject(1234)));
print((new ValueObject("1234")) > (new ValueObject("1234.0")));
print((new ValueObject("1234")) >= (new ValueObject("1234.0")));
print((new ValueObject("1234")) < (new ValueObject("1234.0")));
print((new ValueObject("1234")) <= (new ValueObject("1234.0")));
print((new ValueObject("1234")) == (new ValueObject(1234.0)));
print((new ValueObject("1234")) == (new ValueObject(1234.1)));
print((new ValueObject("1234")) != (new ValueObject("1234.0")));
print((new ValueObject(1234)) != (new ValueObject(1234.0)));
var v = new ValueObject(1234);
print(v == v);
print(v != v);

print(33 + (new ValueObject(55)));
print(33 + (new ValueObject("55")));
print("62456" + (new ValueObject(23452)));
print(76 - (new ValueObject(23)));
print(35 * (new ValueObject("123")));
print("1574.4" / (new ValueObject(12.3)));
print(329329 % (new ValueObject(127)));
print(55 & (new ValueObject(27)));
print(-439 | (new ValueObject(33)));
print(123 ^ (new ValueObject(243)));
print(4398 << (new ValueObject(5)));
print(129384 >> (new ValueObject(3)));
print(-348234 >>> (new ValueObject(11)));
print(1234 < (new ValueObject(1235)));
print(1234 < (new ValueObject(1234)));
print(1234 <= (new ValueObject(1235)));
print(1234 <= (new ValueObject("1234.0")));
print(1235 > (new ValueObject(1234)));
print(1234 > (new ValueObject(1234)));
print(1235 >= (new ValueObject(1234)));
print("1234.0" >= (new ValueObject(1234)));
print("1234.0" >= (new ValueObject(1234)));
print("1234" > (new ValueObject("1234.0")));
print("1234" >= (new ValueObject("1234.0")));
print("1234" < (new ValueObject("1234.0")));
print("1234" <= (new ValueObject("1234.0")));
print("1234" == (new ValueObject(1234.0)));
print("1234" == (new ValueObject(1234.1)));
print("1234" != (new ValueObject("1234.0")));
print(1234 != (new ValueObject(1234.0)));

print((new ValueObject(33)) + 55);
print((new ValueObject(33)) + "55");
print((new ValueObject("62456")) + 23452);
print((new ValueObject(76)) - 23);
print((new ValueObject(35)) * "123");
print((new ValueObject("1574.4")) / 12.3);
print((new ValueObject(329329)) % 127);
print((new ValueObject(55)) & 27);
print((new ValueObject(-439)) | 33);
print((new ValueObject(123)) ^ 243);
print((new ValueObject(4398)) << 5);
print((new ValueObject(129384)) >> 3);
print((new ValueObject(-348234)) >>> 11);
print((new ValueObject(1234)) < 1235);
print((new ValueObject(1234)) < 1234);
print((new ValueObject(1234)) <= 1235);
print((new ValueObject(1234)) <= "1234.0");
print((new ValueObject(1235)) > 1234);
print((new ValueObject(1234)) > 1234);
print((new ValueObject(1235)) >= 1234);
print((new ValueObject("1234.0")) >= 1234);
print((new ValueObject("1234")) > "1234.0");
print((new ValueObject("1234")) >= "1234.0");
print((new ValueObject("1234")) < "1234.0");
print((new ValueObject("1234")) <= "1234.0");
print((new ValueObject("1234")) == 1234.0);
print((new ValueObject("1234")) == 1234.1);
print((new ValueObject("1234")) != "1234.0");
print((new ValueObject(1234)) != 1234.0);

print(+new ValueObject(239423.2349));
print(+new ValueObject("239423.2349"));
print(-new ValueObject(238742));
print(-new ValueObject("238742"));
print(~new ValueObject(-3492384));
print(~new ValueObject("-3492384"));

var v = new ValueObject(123);
print(typeof v);
print(v++);
print(typeof v);
print(v);
v = new ValueObject(v);
print(typeof v);
print(++v);
print(typeof v);

v = new ValueObject(v);
print(typeof v);
print(v--);
print(typeof v);
print(v);
v = new ValueObject(v);
print(typeof v);
print(--v);
print(typeof v);

o = new ValueObject(12345); print(typeof o); print(o += 6789); print(typeof o); print(o); o = new ValueObject(o); print(typeof o); print(o += (new ValueObject(6789))); print(typeof o); print(o);
o = new ValueObject(12345); print(typeof o); print(o -= 6789); print(typeof o); print(o); o = new ValueObject(o); print(typeof o); print(o -= (new ValueObject(6789))); print(typeof o); print(o);
o = new ValueObject(12345); print(typeof o); print(o *= 6789); print(typeof o); print(o); o = new ValueObject(o); print(typeof o); print(o *= (new ValueObject(6789))); print(typeof o); print(o);
o = new ValueObject(15744); print(typeof o); print(o /= 123); print(typeof o); print(o); o = new ValueObject(o); print(typeof o); print(o /= (new ValueObject(64))); print(typeof o); print(o);
o = new ValueObject(12345); print(typeof o); print(o %= 6789); print(typeof o); print(o); o = new ValueObject(o); print(typeof o); print(o %= (new ValueObject(1748))); print(typeof o); print(o);
o = new ValueObject(12345); print(typeof o); print(o &= 6789); print(typeof o); print(o); o = new ValueObject(o); print(typeof o); print(o &= (new ValueObject(374))); print(typeof o); print(o);
o = new ValueObject(12345); print(typeof o); print(o |= 6789); print(typeof o); print(o); o = new ValueObject(o); print(typeof o); print(o |= (new ValueObject(13764))); print(typeof o); print(o);
o = new ValueObject(12345); print(typeof o); print(o ^= 6789); print(typeof o); print(o); o = new ValueObject(o); print(typeof o); print(o ^= (new ValueObject(34857))); print(typeof o); print(o);
o = new ValueObject(12345); print(typeof o); print(o >>= 5); print(typeof o); print(o); o = new ValueObject(o); print(typeof o); print(o >>= (new ValueObject(5))); print(typeof o); print(o);
o = new ValueObject(12345); print(typeof o); print(o <<= 5); print(typeof o); print(o); o = new ValueObject(o); print(typeof o); print(o <<= (new ValueObject(5))); print(typeof o); print(o);
o = new ValueObject(-12345); print(typeof o); print(o >>>= 5); print(typeof o); print(o); o = new ValueObject(o); print(typeof o); print(o >>>= (new ValueObject(5))); print(typeof o); print(o);


o=new (function() { this.valueOf = function() { return 33 } } )
print(+o);

OneHundredEleven = function() { this.valueOf = function() { print("click"); return 111 }; this.toString = null; }; o = new OneHundredEleven; print(+o)
TestObject = function() { };
TestObject.prototype = new OneHundredEleven;
o2 = new TestObject; print(+o2)
 
AString = function() { this.valueOf = null; this.toString = function() { print("good toString"); return "stringy"; } };
o = new AString; print(+o)
 
StringAgain = function() { this.valueOf = function() { print("bad valueOf"); return this }; this.toString = function() { print("good toString"); return "stringy"; } };
o = new StringAgain; print(+o);

(function() {
	var v={toString:void null,valueOf:function(){print("valueOf = 1234");return 1234;}};
	var o=new Object();
	o[v]=5678;
	print(o[1234]);
	delete v.toString;
	o[v]=9012;
	print(o[1234]);
	print(o["[object Object]"]);
	v.toString=function(){print('toString = "abcd"');return "abcd";};
	o[v]="efgh";
	print(o.abcd);
})();

var v = { valueOf: function() { return 'a' } };
var x = { a: 55 };
print(delete x[v]);
print(typeof x.a);

var v = { toString: function() { return "koko" } };
var o={koko:"koko"};
print("koko" in o);
print(v in o);

var a = [];
a["5.0"]=2384;
a["5"]=9812;
print(a[new String("5.0")]);
print(a[new Number("5.0")]);

print("*** eval ***");

print(eval("3"))

print(eval("3+3"))

print(eval("3+3;;;;"))

print(eval("3+3;;;{}"))

print(eval(";;{3+3};;"))

print(eval("aGlobal=1"))

print(eval("aGlobal"))

print(typeof eval("aGlobal=new (function(){this.a='a string'})"))

print(eval("aGlobal.a"))

print("- if");

print(eval("if (true) 3"))

print(eval("if (false) 3"))

print(eval("if (true) 3+3"))

print(eval("if (false) 3+3"))

print(eval("if (true) 3+3; else 6-6"))

print(eval("if (false) 3+3; else 6-6"))

print(typeof eval("aGlobal; if (false) 3"))

print("- break");

print(eval("b: { 1; break b }"))

print(eval("b: if (true) { 1; break b; 2 }"))

print(eval("3; b: if (false) { 123; if (false) { 1; break b; 2 }}"))

print(eval("3; b: if (true) { 123; if (false) { 1; break b; 2 }}"))

print(eval("3; b: if (false) { 123; if (true) { 1; break b; 2 }}"))

print(eval("3; b: if (false) { 123; if (false) { 1; break b; 2 }}"))

print("- while");

print(eval("while (false) 7;"))

print(eval("5; while (false) 7;"))

print(eval("5; while (true) { 3; break }"))

print(eval("var i = 0; 234; while (i++ == 0) i * 100"))

print(eval("var i = 0; 234; while (i++ == 0) { if (i == 1) continue; i * 100 }"))

print(eval("var i = 0; 234; while (++i < 10) { 23; continue }"))

print(eval("var i = 3; 234; while (++i < 10) { j = 5; while (++j < i) j * 3 }"))

print("- do while");

i = 0; print(eval("do { ; } while (++i < 1000)"))

i = 0; print(eval("do { ++i } while (i < 1000)"))

i = 0; print(eval("234; do { ++i; if (i < 10) continue; i * 100 } while (i < 1000)"))

i = 0; print(eval("234; do { ++i; if (i < 10) continue; 123 } while (i < 1000)"));

print("- for");

print(eval("; for (var i = 0; i < -1; ++i) 123"))

print(eval("1; for (var i = 0; i < -1; ++i) 123"))

print(eval("; for (var i = 0; i < 1; ++i) 123"))

print(eval("1; for (var i = 0; i < 1; ++i) 123"))

print(eval("for (var i = 0; i < 100; ++i) { ++i; if (i < 10) continue; 123 }"));

print(eval("for (var i = 0; i < 100; ++i) { ++i; if (i >= 10) break; 123 }"));

print("- for in");

o = new (function() { });

print(eval("; for (var i in o) 123"))

print(eval("1; for (var i in o) 123"))

o["a"] = "b";

print(eval("; for (var i in o) 123"))

print(eval("1; for (var i in o) 123"))

for (var i = 0; i < 100; ++i) o[i] = i * 1234;

print("built");

print(eval("for (var i in o) { ++i; if (i == 55) continue; 123 }"));

print("first");

print(eval("for (var i in o) { ++i; if (i == 55) continue; 123 }"));

print(eval("for (var i in o) { ++i; if (i == 55) { print('found'); break; }; 123 }"));

print("second");

print("- switch");

print(eval("; switch (3+3) { case 7: 123 }"));
print(eval("; switch (3+3) { case 7: 123; default: 5656 }"));
print(eval("345; switch (3+3) { case 7: 123; default: 5656 }"));
print(eval("345; switch (3+3) { case 7: 123 }"));
print(eval("; switch (3+3) { case 6: 123; case 7: 256; }"));
print(eval("; switch (3+3+1) { case 6: 123; case 7: 256; }"));
print(eval("; switch (3+3) { case 6: 123; break; case 7: 256; }"));
print(eval("for (var i = 0; i < 10; ++i) switch (3+3) { case 6: 123; continue; case 7: 256; }"));

print("- try catch");

print(eval("; try { 234 } catch ( x ) { 555 }"));
print(eval("1234; try { 234 } catch ( x ) { 555 }"));
print(eval("1234; try { throw 55; 234 } catch ( x ) { 555 }"));
print(eval("1234; try { throw 55; 234 } catch ( x ) { }"));
print(eval("1234; try { throw 55; 234 } catch ( x ) { } finally { 9876 }"));
print(eval("try { throw 2 } catch (x) { try { throw 3 } catch (y) { print(y) } } finally { try { } catch (y) { } }"));
for (x = 0; x < 2; ++x) {
	for (y = 1; y < 4; ++y) {
		print(eval("switch (x) { case 0: 0; break; case 1: switch (y) { case 3: 3; break; case 2: try { throw 2 } catch (z) { z } } }"))
	}
}

print("- more");

print((function() { var x = 33; eval("x = 55"); return x })());

print((function() { var x = 33; eval("var x = 55"); return x })());

print((function() { eval("if (false) var x = 23"); return x })());

(function() { var x = 88; try { throw "k" } catch (x) { eval("print(x); var x = 5"); print(x) }; print(x) })();

(function() { var x = 88; try { throw "k" } catch (x) { eval("print(x); x = 5"); print(x) }; print(x) })();

aGlobal = "ok"; (function() { eval("var aGlobal = 5"); print(delete aGlobal); print(aGlobal) })(); print("aGlobal" in this);
print("");

aGlobal = "ok"; (function() { eval("var aGlobal = 5"); print(delete aGlobal); print(delete aGlobal); })(); print("aGlobal" in this);
print("");

(function() { eval("var x = 5"); print(delete x); })();
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
mandelbrot();

(function(){
	print("*** operation matrix ***");

	var valueStrings = [
		'undefined', 'null', 'false', 'true', '0', '1', '-128', '1234.5678', '-1.0e+7', 'NaN', 'Infinity', '""', '"str"'
		, '"-128"', '"1.0"', '"  +1234  "', '"string"', '"5tring"'
		, '({valueOf:function(){return "-128"}})','({valueOf:function(){return null}})'
	];

	var values = [ ];
	for (var i = 0; i < valueStrings.length; ++i) {
		values[i] = eval(valueStrings[i]);
	}

	var tests = [
		"==", "!=", "===", "!==", "<", "<=", ">", ">=", "+", "-", "*", "/", "%", "&", "^", "|", "<<", ">>", ">>>"
	];

	for (var t = 0; t < tests.length; ++t) {
		var test = tests[t];
		print(test);
		for (var i = 0; i < values.length; ++i) {
			var a = values[i];
			var results = "";
			for (var j = 0; j < values.length; ++j) {
				var b = values[j];
				var r;
				try {
					r = eval('a' + test + 'b');
					if (typeof r === 'number') {
						r = Math.floor(r * 100000 + 0.5) / 100000;
					}
				}
				catch (e) {
					r = e;
				}
				if (results !== "") {
					results += " ";
				}
				results += r;
			}
			results = valueStrings[i] + ": " + results;
			print(results);
		}
		print("");
	}
})();

(function() {
	print("*** prototypes ***");

	function sort(a) {
		for (i = 0; i < a.length - 1; ++i) {
			for (j = i + 1; j < a.length; ++j) {
				if (a[j] < a[i]) {
					a[i] = [ a[j], a[j] = a[i] ][0];
				}
			}
		}
		return a;
	}
	var a = sort([ "pok", "adf", "viusdh", "oqritj", "qthreguajf", "avpok", "pok" ]);
	for (var i = 0; i < a.length; ++i) print(a[i]);

	function contents(o) {
		var keys = [ ];
		for (var i in o) {
			keys[keys.length] = i;
		}
		sort(keys);
		var s = "{ ";
		for (var i = 0; i < keys.length; ++i) {
			s += keys[i] + ": " + o[keys[i]] + (i < keys.length - 1 ? ", " : " }"); 
		}
		return s;
	}

	print(contents({ "kzxcjvn": "oaierjf", "goiegjf": "odisjv", "kjscnv": "oeriuf", "asdf": "poqker" }));

	function ChainA(a, b, c, d) { this.shadowProperty = "ChainA value"; this.toString = function() { return this.shadowProperty; }; };
	ChainA.prototype.rootProperty = "Root value";
	print("typeof ChainA.prototype.constructor: " + typeof ChainA.prototype.constructor);
	ChainA.prototype.constructor = 12345;
	print("ChainA.prototype.constructor: " + ChainA.prototype.constructor);
	print("contents(ChainA.prototype): " + contents(ChainA.prototype));
	print("delete ChainA.prototype.constructor: " + delete ChainA.prototype.constructor);
	print("delete ChainA.prototype: " + delete ChainA.prototype);
	ChainA.prototype.constructor = 12345;
	print("contents(ChainA.prototype): " + contents(ChainA.prototype));

	function ChainB() { this.chainAProperty = this.shadowProperty = "ChainB value"; };
	ChainB.prototype = new ChainA();
	function ChainC() { this.chainBProperty = this.shadowProperty = "ChainC value"; };
	ChainC.prototype = new ChainB();
	
/* FIX : reenable
	function Damaged() { }
	Damaged.prototype = 12345;
	var damaged = new Damaged;
	Object.prototype.sdfnkdan = 'o3htieofd';
	print("damaged.sdfnkdan="+damaged.sdfnkdan);
	print("delete Object.prototype.sdfnkdan="+(delete Object.prototype.sdfnkdan));
*/
	var chain = new ChainC();
	print("chain.shadowProperty: " + chain.shadowProperty);
	print("chain.chainBProperty: " + chain.chainBProperty);
	print("chain.chainAProperty: " + chain.chainAProperty);
	print("chain.rootProperty: " + chain.rootProperty);
	print('rootProperty' in chain);
	print("chain: " + chain);
	print("'shadowProperty' in chain: " + ('shadowProperty' in chain));
	print("delete chain.shadowProperty: " + delete chain.shadowProperty);
	print("'shadowProperty' in chain: " + ('shadowProperty' in chain));
	print("chain.shadowProperty: " + chain.shadowProperty);
	print("delete chain.shadowProperty: " + delete chain.shadowProperty);
	print("'shadowProperty' in chain: " + ('shadowProperty' in chain));
	print("chain.shadowProperty: " + chain.shadowProperty);
	print("chain: " + chain);
	
	var chain = new ChainB();
	print("chain.shadowProperty: " + chain.shadowProperty);
	print("chain.chainBProperty: " + chain.chainBProperty);
	print("chain.chainAProperty: " + chain.chainAProperty);
	print("chain.rootProperty: " + chain.rootProperty);
	print("chain: " + chain);
	print("delete chain.shadowProperty: " + delete chain.shadowProperty);
	print("chain.shadowProperty: " + chain.shadowProperty);
	print("delete chain.shadowProperty: " + delete chain.shadowProperty);
	print("chain.shadowProperty: " + chain.shadowProperty);
	print("chain: " + chain);
	print("'shadowProperty' in chain: " + ('shadowProperty' in chain));
	print("delete ChainB.prototype.shadowProperty: " + delete ChainB.prototype.shadowProperty);
	print("'shadowProperty' in chain: " + ('shadowProperty' in chain));
	print("chain.shadowProperty: " + chain.shadowProperty);
	print("chain: " + chain);

	ChainB.prototype.toString = undefined;
	chain = new ChainC;
	print(contents(chain));
	print(delete ChainC.prototype.shadowProperty);
	print(contents(chain));
	print(delete chain.shadowProperty);
	print(contents(chain));

})();

(function() {
	print("*** wrappers ***");

	if (true) {
		var constants = [
			undefined, null, false, true, 1.2345, 12, +Infinity, NaN, '', 'xyzzy'
			, { "i am": "object" }, [ 'i', 'am', 'array' ]
		];
		var constructorNames = [ 'Boolean', 'Number', 'String', 'Object'/*, 'Array'*/ ];
		var testArguments = [ 'true', '3.4567', -99.99, 123.4, 5.678 ];
		for (var j = 0; j < constructorNames.length; ++j) {
			var name = constructorNames[j];
			var constructor = this[name];
			for (var k = 0; k < 2; ++k) {
				var s = '';
				for (var i = -1; i < constants.length; ++i) {
					if (s) s += ', ';
					try {
						var o = (k === 0 ?
								(i < 0 ? new constructor() : new constructor(constants[i]))
								: (i < 0 ? constructor() : constructor(constants[i])));
						try {
							s += (typeof o)[0] + '_' + o.valueOf();
						}
						catch (x) {
							s += 'error';
						}
						s += "/";
						try {
							s += (typeof o)[0] + '_' + o.toString();
						}
						catch (x) {
							s += 'error';
						}
					}
					catch (x) {
						s += 'error';
					}
				}
				print((k === 0 ? 'new ' : '') + name + '(): ' + s);
			}
		}
	}

	{
		var o0 = new Object();
		var o1 = new Object();
		print("o0 === o1: " + (o0 === o1));
		o0 = new Object();
		o1 = new Object(o0);
		print("o0 === o1: " + (o0 === o1));
	}

	/*
		According to 11.2.1 (and 8.7.1), any property access may result in a conversion of the target base (i.e. the
		left-hand side of '.' or '[]') to an object (if it is a "primitive type"). A string is a "primitive type". This
		means that, in theory, any subscript access ('[]') on a string requires the creation of a temporary full-blown
		String object (which is then just thrown away).

		Workaround in NuXJScript: strings are "shallow" objects meaning that they can't be extended but they have
		properties that can be accessed without any object conversion / creation. Only member calls (on a member in
		String.prototype) will actually convert "this" to a full-blown "deep" String object. Any such member call
		*might* indirectly use "this" for extending the String so this is necessary to obtain 100% EcmaScript
		compatibility.
	*/

	if (true) {
		var breakOuts = [ ];
		String.prototype.test = function(i, s) { breakOuts[i] = this; breakOuts[i].s = s; };
		var s = "zxcv";
		s.test(0, 'asdf');
		s.test(1, 'qwerty');
		print('delete String.prototype.test: ' + delete String.prototype.test);
		print('typeof breakOuts[0]: ' + typeof breakOuts[0]);
		print('typeof breakOuts[0].s: ' + typeof breakOuts[0].s);
		print('breakOuts[0]: ' + breakOuts[0]);
		print('breakOuts[0].s: ' + breakOuts[0].s);
		print('breakOuts[1]: ' + breakOuts[1]);
		print('breakOuts[1].s: ' + breakOuts[1].s);
	}

	/*
		According to 15.7.4.4 Number.valueOf can't be transferred.
	*/

	{
		var MyBoolean = function() { }
		var MyNumber = function() { }
		var MyString = function() { }

		MyBoolean.prototype = Boolean.prototype;
		MyNumber.prototype = Number.prototype;
		MyString.prototype = String.prototype;

		var myBoolean = new MyBoolean();
		var myNumber = new MyNumber();
		var myString = new MyString();
		try { print("myBoolean: " + (myBoolean.valueOf())); } catch (x) { print("exception: " + x); }
		try { print("myNumber: " + (+myNumber)); } catch (x) { print("exception: " + x); }
		try { print("myString: " + (''+myString)); } catch (x) { print("exception: " + x); }
	}
	
	{
/*
FIX:

String comparison can be forced by: "" + a == "" + b.
Numeric comparison can be forced by: +a == +b.
Boolean comparison can be forced by: !a == !b.
NOTE 2 The equality operators maintain the following invariants:
 
A != B is equivalent to !(A == B).
A == B is equivalent to B == A, except in the order of evaluation of A and B.
 
new String("a") == "a" and "a" == new String("a")are both true.
new String("a") == new String("a") is false.
 
*/
		print('3 in new String("pokpokpko"): '+(3 in new String("pokpokpko")));
		// FIX : (new String("pokpokpokok")).hasOwnProperty(3)
		var s = '';
		for (var i in new String("abcdefghijklmn")) s += i + ",";
		print('for (var i in new String("abcdefghijklmn")): '+s);
 	}

 	{
 		for (i in { valueOf: 'test' }) print(i); // should print 'valueOf'
 		for (i in { somethingElse: 'test' }) print(i); // should print 'somethingElse'
 	}
	
})();

(function() {
	print("*** stdlib ***");
	
// FIX : drop
/*	function printAndEval(s) {
		var v;
		print(s + ": " + (v = eval(s)));
		return v;
	};
*/
	function analyzeFlags(object, property) {
		var dontEnum = true;
		for (var p in object) {
			if (p === property) {
				dontEnum = false;
				break;
			}
		}
		var valueWas = object[property];
		var testObject = { };
		object[property] = testObject;
		var valueIs = object[property];
		var readOnly = (valueIs !== testObject);
		var deleted = delete object[property];
		var dontDelete = (property in object);
		print(property + " : " + (dontEnum ? "dontEnum," : "") + (readOnly ? "readOnly," : "")
				+ (dontDelete ? "dontDelete," : "") + " (deleted:" + deleted + ')');
		object[property] = valueWas;
	};
	
	var readOnlyConstants = [
		[ this, "NaN" ], [ this, "Infinity" ], [ this, "undefined" ]
		, [ Number, "MAX_VALUE" ], [ Number, "MIN_VALUE" ], [ Number, "NaN" ], [ Number, "NEGATIVE_INFINITY" ], [ Number, "POSITIVE_INFINITY" ]
	];
	for (var i = 0; i < readOnlyConstants.length; ++i) {
		var base = readOnlyConstants[i][0];
		var constant = readOnlyConstants[i][1];
		print(constant + ": " + base[constant]);
		analyzeFlags(base, constant);
	/*	printAndEval(constant);
		printAndEval("typeof " + constant);
		printAndEval("delete " + constant);
		printAndEval("typeof " + constant);
		printAndEval(constant + " = 192837465");
		printAndEval(constant);*/
	};

	var builtInConstructors = [ "Object", "Boolean", "Number", "String", "Function", "Error" ];
	for (var i = 0; i < builtInConstructors.length; ++i) {
		var constructor = builtInConstructors[i];
/* FIX : drop
		print(constructor);
		printAndEval("typeof " + constructor);
		var copy = eval(constructor);
		printAndEval(constructor + " = 192837465");
		printAndEval(constructor);
		printAndEval("delete " + constructor);
		printAndEval("typeof " + constructor);
		eval(constructor + " = copy");
		printAndEval("typeof " + constructor);
		printAndEval(constructor + ".length");
		printAndEval("delete " + constructor + ".length");
		printAndEval(constructor + ".length");
		eval(constructor + ".length = 938");
		printAndEval(constructor + ".length");
		var prototypeWas = eval(constructor + ".prototype");
		eval(constructor + ".prototype = { 'wrong': 'prototype' }");
		printAndEval(constructor + ".prototype === prototypeWas");
		printAndEval("delete " + constructor + ".prototype");
		printAndEval(constructor + ".prototype === prototypeWas");
		printAndEval("typeof " + constructor + ".prototype.constructor");
		printAndEval(constructor + ".prototype.constructor === " + constructor);*/
		print(constructor + ": " + typeof this[constructor]);
		analyzeFlags(this, constructor);
		print(constructor + ".prototype: " + this[constructor].prototype);
		analyzeFlags(this[constructor], 'prototype');
		print(constructor + ".prototype.constructor: " + typeof this[constructor].prototype.constructor);
		analyzeFlags(this[constructor].prototype, 'constructor');
		print(this[constructor].prototype.constructor === this[constructor]);
		print(constructor + ".length: " + this[constructor].length);
		analyzeFlags(this[constructor], 'length');
	}

	// FIX : test that all functions in all standard library objects are not constructable and that they don't have prototype fields
//	var globalObjects = [ "Object", "Boolean", "Number", "String", "Function", "Error", "Math" ];

	{
		var x = Object(true);
		print("x: " + x);
		print("x instanceof Boolean: " + (x instanceof Boolean));
		var realBoolean = Boolean;
		Boolean = function(b) { print("custom Boolean call"); return realBoolean(!b) };
		x = new Boolean(true);
		print("x: " + x);
		print("x instanceof Boolean: " + (x instanceof Boolean));
		print("x instanceof realBoolean: " + (x instanceof realBoolean));
		realBoolean.prototype.capture = function() { return this; };
		Boolean.prototype.capture = function() { return this; };
		x = (true).capture();
		delete realBoolean.prototype.capture;
		delete Boolean.prototype.capture;
		print("x: " + x);
		print("x instanceof Boolean: " + (x instanceof Boolean));
		print("x instanceof realBoolean: " + (x instanceof realBoolean));
		x = Object(true);
		print("x: " + x);
		print("x instanceof Boolean: " + (x instanceof Boolean));
		print("x instanceof realBoolean: " + (x instanceof realBoolean));
	
		var x = Object(1.2345);
		print("x: " + x);
		print("x instanceof Number: " + (x instanceof Number));
		var realNumber = Number;
		Number = function(b) { print("custom Number call"); return realNumber(!b) };
		x = new Number(1.2345);
		print("x: " + x);
		print("x instanceof Number: " + (x instanceof Number));
		print("x instanceof realNumber: " + (x instanceof realNumber));
		realNumber.prototype.capture = function() { return this; };
		Number.prototype.capture = function() { return this; };
		x = (1.2345).capture();
		delete realNumber.prototype.capture;
		delete Number.prototype.capture;
		print("x: " + x);
		print("x instanceof Number: " + (x instanceof Number));
		print("x instanceof realNumber: " + (x instanceof realNumber));
		x = Object(1.2345);
		print("x: " + x);
		print("x instanceof Number: " + (x instanceof Number));
		print("x instanceof realNumber: " + (x instanceof realNumber));
	
		var x = Object("strongbad");
		print("x: " + x);
		print("x instanceof String: " + (x instanceof String));
		var realString = String;
		String = function(b) { print("custom String call"); return realString(!b) };
		x = new String("strongbad");
		print("x: " + x);
		print("x instanceof String: " + (x instanceof String));
		print("x instanceof realString: " + (x instanceof realString));
		realString.prototype.capture = function() { return this; };
		String.prototype.capture = function() { return this; };
		x = ("strongbad").capture();
		delete realString.prototype.capture;
		delete String.prototype.capture;
		print("x: " + x);
		print("x instanceof String: " + (x instanceof String));
		print("x instanceof realString: " + (x instanceof realString));
		x = Object("strongbad");
		print("x: " + x);
		print("x instanceof String: " + (x instanceof String));
		print("x instanceof realString: " + (x instanceof realString));
	}
})();

