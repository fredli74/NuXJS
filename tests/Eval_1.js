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
// o.test2()