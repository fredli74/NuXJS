var o = {}; 
f=function(n) { for (var i = 0; i < n; ++i) o[i] = "" + i * 1234; };
print("1st phase");
for (var j = 0; j < 2; ++j) f(100000);
for (var i = 0; i < 100000; i += 10000) print(i + ': ' + o[i]);
var b = []; 
f=function(n) { for (var i = 0; i < n; ++i) b[i] = "" + i * 1234; };
print("2nd phase");
for (var j = 0; j < 200; ++j) f(10000);
