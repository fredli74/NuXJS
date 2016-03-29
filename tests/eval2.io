> (function f() { eval("var x = 3"); eval("var x = 5"); print(delete x); print(typeof x); })()
< true
< undefined
-
