> (function() { var i = 123; try { throw "x" } catch (x) { print(i) } })()
< 123
-
> (function() { var i = 123; try { throw "x" } catch (x) { (function() { print(i) })() } })()
< 123
-
> (function() { var j = 345; (function() { var i = 123; try { throw "x" } catch (x) { (function() { print(i); print(j) })() } })() })()
< 123
< 345
-
> (function() { var j = 345; (function() { var i = 123; with ({}) { (function() { print(i); print(j) })() } })() })()
< 123
< 345
-
> (function() { var j = 345; (function() { var i = 123; try { return; } finally { (function() { print(i); print(j) })() } })() })()
< 123
< 345
-
> (function() { var j = 345; (function() { var i = 123; try { return; } finally { (function() { print(i); print(j); return })() } })() })()
< 123
< 345
-
> (function() { var j = 345; (function() { var i = 123; try { return; } finally { (function() { print(i); print(j); return; print("no") })() } })() })()
< 123
< 345
-
> (function() { var j = 345; (function() { var i = 123; try { return; } finally { (function() { print(i); print(j) })(); return; print("no") } })() })()
< 123
< 345
-
