var o = function() { }
var a = new o;
for (var i = 0; i < 100000; ++i) {
	a[i] = new o;
	for (var j = 0; j < 100; ++j) {
		a[i][j] = new o;
	}
}
