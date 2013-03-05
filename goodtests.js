// function print(x) { console.log(x) }
// function time(f) { var start = Date.now(); f(); print("elapsed: " + (Date.now() - start)); }

  var i = 0 /* comment */ ;   // more comment
 j = 1234.5 /* multi
 line
 comment
 */

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
/*
var x=3,y=9,z=5
print(x+y*z == 48)
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
print(1.0E-3 == 0.001)
print(1.0E+3 == 1000.0)
print(0.000000000001 == 0.000000000001)
print(0.99999999999999 == 1.0)
print(x == 3)
print(x == 3.0)
print(x == 3.0e0)
print(x == 0.003e3)
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
*/

var i = 0 ;   while (   i   <   100.0e+1   )  {  j = j + i ; ++ i  }
 do { j = j - i * 0.5 } while ( i ++ < 2000 )
 print (  i + ", " + j )  ;

var i=0;for(j=1234.5;1E3>i;)j+=i,++i;do j-=0.5*i;while(2E3>i++);print(i+", "+j);

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

print(f.a+0) // NaN
garlicNan=f.a+0
cheeseNan=f.b+0
print(garlicNan<cheeseNan) // NaN comparisons are always false
print(garlicNan<=cheeseNan) // NaN comparisons are always false
print(garlicNan==cheeseNan) // NaN comparisons are always false
print(garlicNan===cheeseNan) // NaN comparisons are always false
print(garlicNan>=cheeseNan) // NaN comparisons are always false
print(garlicNan>cheeseNan) // NaN comparisons are always false
print(garlicNan!=cheeseNan) // but this is true
print(garlicNan!==cheeseNan) // but this is true

if (false && false || true && true) print("ok"); else print("oops");
if (!(false && true || true && false)) print("ok"); else print("oops")
if (!(true && false || false && true)) print("ok"); else print("oops");
if (false && true || true && false || !(false && false)) print("ok"); else print("oops")
if ((3<5 && 5<3 || 2==3 && 2==2) || !(3>4 || 7<5)) print("ok"); else print("oops");
if ((3<5 && 5<3 || 2==3 && 2==2) || !(3>4 || 5<7)) print("oops"); else print("ok");
print ((3<5 && 5<3 || 2==3 && 2==2) || !(3>4 || 7<5) ? ("ok") : ("oops"));

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

x=9;(function() { var y=3,x=5 })(); print(x)

delete z;
print(function() { z=3; return delete z }())
print(function() { return delete f.z }())
print(function() { return delete "i"[0] }())
print(function() { var z=3; return delete z }())
print(function() { var z=3; var g=function() {  return delete z; } ; return g; }()())
print(delete "constant is always ok")

f=function(a, b) { this.a=a; this.b=b; this.c="cee"; this.dump=function() { print("a: " + this.a  + ", b: " + this.b + ", c: " + this.c); } }
x = new f
x.dump()
x = new f ( "aah" , "bee" )
x.dump()

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
try { (function() { try { throw "x" } finally { try { throw "y" } catch (x) { print(x) } } })() } catch (x) { print(x) }

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

for(var x=-3;x<=3;++x)switch(x+3){case 1:case 2:case 1+2:print(x)}

var f = function(n) { print('test:' + n); };
for (var x = -3; x <= 3; ++x) switch (  x + 3  )   { case 1  : case 2  : case f ( x )   , 1 + 3  : print(x) }

switch (3) { case 2: print("NO"); default: print("HEJ"); case 3: print("3") }

switch ("1") { case 1: print('1'); case "1": print('"1"'); case 2.0: print("continues to 2"); break; case "3": print("but not to 3"); case "1": print('Second "1" is not executed'); default: print("Neither is default") }

switch (1.0) { case 0: print('0'); default: print('default'); case "1.0": print('1'); case "1": print('"1"'); case 2.0: print("continues to 2"); break; case "3": print("but not to 3"); case "1": print('Second "1" is not executed'); }

for (var i = 0; i < 10; ++i) { switch (i) { case 5: continue }; print(i) }

print("hejsan".length);

s = "0123456789"; print(s.length); s.length = 0; print(s.length); // both output 10, string lengths are not writable

MyObject = function() { }; o = new MyObject; o.name = "magnus"; o.age = 41;
print("name" in o);
print("age" in o);
print("nope" in o);
print(o in o);
print("o" in this);

