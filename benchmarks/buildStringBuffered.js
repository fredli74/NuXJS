function StringBuilder() {
	this.buffers = [ ];
	for (var i = 0; i < 20; ++i) {
		this.buffers[i] = '';
	}
}
StringBuilder.prototype.push = function push(s) {
	var i = 0, n = 256;
	while ((this.buffers[i] += s).length >= n && i + 1 < this.buffers.length) {
		s = this.buffers[i];
		this.buffers[i] = '';
		n <<= 1;
		++i;
	}
}
StringBuilder.prototype.get = function get() {
	var s = this.buffers[0];
	for (var i = 1; i < this.buffers.length; ++i) s += this.buffers[i];
	return s;
}

for (var i = 0; i < 10; ++i) (function() { var builder = new StringBuilder(); for (var i = 0; i < 100000; ++i) builder.push('x'); var s = builder.get(); print(s.length); })();
