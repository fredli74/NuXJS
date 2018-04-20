> var first = { n: null }, o = first; for (var i = 0; i < 1000000; ++i) { o.n = { n: null }; o = o.n; };
> gc();
> var o = first, count = 0; while (o.n) { ++count; o = o.n; }; print(count)
< 1000000
-
