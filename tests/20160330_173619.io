> (function() { var x = 7; try { throw "x" } catch (x) { x = 9; eval("var x = 11, y = 5"); print(x) }; print(x); print(y) })()
< 11
< 7
< 5
-
