f=function(n) { var i = 0; while (i < n) { f[i] = i * 1234; ++i } }
f(1000000)
for (var i = 0; i < 1000000; i += 100000) print(i + ': ' + f[i]);
