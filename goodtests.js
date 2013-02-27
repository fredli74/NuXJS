  var i = 0 /* comment */ ;   // more comment
 j = 1234.5 /* multi
 line
 comment
 */
   while (   i   <   100.0e+1   )  {  j = j + i ; ++ i  }
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

/*
function f() { z: print("hej"); z: for (var i = 0; i < 10; ++i) { b: while (true) r: for (var j = 0; j < 10; ++j) { if (j >= 5) break b; print(j) }; print("pok") } }
{ a: { for (var x = 1; x < 10; ++x) for (var y = 1; y < 3; ++y) { console.log(x); if (x == 5) break a }; console.log("wow") } }
 
{ x = 0; a: for (var i = 0; i < 10; ++i) { if (++x < 8) continue a; console.log("iter " + i) } }
*/
/*
f=function() { try { var x = 7; print(x); throw "hej" } catch (x) { print(x); throw x } finally { print("final"); }; print("yeah") }
 
	PUSH_CATCH @c
		PUSH_CATCH @a -> push catch address
			blahblah
		POP_CATCH -> pop catch address
		JMP @b
a:			catchblock
b:		POP_CATCH @c
c:
 
 
 
	try { } catch (x) { }
	 
	CATCH @a
		{ try block }
	NO_CATCH @b
a:		WRITE_INDEXED x
		POP
		{ catch block }
b:
 
 
 
	try { } finally { }
	
	CATCH @a
		{ try block }
	PUSH #false
	NO_CATCH @b
a:		PUSH #true
b:		{ finally block }
	JF @c
		THROW
c:
 
 
 
	try { } catch (x) { } finally { }
 
	CATCH @a
	CATCH @d
		{ try block }
	NO_CATCH @e
d:		WRITE_INDEXED x
		POP
		{ catch block }
e:	PUSH #false
	NO_CATCH @b
a:		PUSH #true
b:		{ finally block }
	JF @c
		THROW
c:
*/
