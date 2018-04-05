function StringBuilder() {
	this.buffers = [ ];
	for (var i = 0; i < 20; ++i) {
		this.buffers[i] = '';
	}
}
StringBuilder.prototype.append = function append(s) {
	for (var i = 0, n = 256; (this.buffers[i] += s).length >= n && i + 1 < this.buffers.length; n <<= 1, ++i) {
		s = this.buffers[i];
		this.buffers[i] = '';
	}
}
StringBuilder.prototype.toString = function toString() {
	var s = this.buffers[0];
	for (var i = 1; i < this.buffers.length; ++i) s += this.buffers[i];
	return s;
}

var s = new StringBuilder
s.append('a');
var t = '';
for (i = 0; i < 512; ++i) t += 'b';
s.append(t);
s.append('c');
print(''+s);
