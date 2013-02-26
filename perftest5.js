f=function(n) { var i = 0; while (i < n) { f[i] = "" + i * 1234; ++i } };
for (var a = 0; a < 2; ++a) f(1000000);
