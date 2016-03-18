function toObject(v) {
	switch (typeof v) {
		case "object":
		case "function": if (v !== null) return v;
		case "boolean": return new Boolean(v);
		case "number": return new Number(v);
		case "string": return new String(v);
	}
	throw "type error"; // FIX
}

Object = function() { return {}; }
Object.prototype = support.prototypes.Object;
Object.prototype.valueOf = function() { return this; }
Object.prototype.toLocaleString = Object.prototype.toString = function() {
	return '[object ' + support.getInternalProperty(this, 'class') + ']'
};

Object.prototype.isPrototypeOf = function(v) {
	while ((v = support.getInternalProperty(v, 'prototype')) != null) {
		if (v == this) return true;
	}
	return false;
};

Function = function(body) {
	var source = '(', n = arguments.length - 1;
	for (var i = 0; i < n; ++i) sources += arguments[i] + (i < n - 1 ? ',' : ')');
	if (n >= 0) source += arguments[n];
	return support.compileFunction(source);
};
Function.prototype = support.prototypes.Function;
Function.prototype.toString = function() { // FIX : <- generic, make a factory function
	if (support.getInternalProperty(this, 'class') !== 'Function') throw "TypeError" // FIX
	return support.getInternalProperty(this, 'value');
};

Boolean = function(v) {
	return support.createWrapper('Boolean', !!v, support.prototypes.Boolean);
};
Boolean.prototype = support.prototypes.Boolean;
Boolean.prototype.valueOf = function() {
	if (support.getInternalProperty(this, 'class') !== 'Boolean') throw "TypeError" // FIX
	return support.getInternalProperty(this, 'value');
};
Boolean.prototype.toString = function() {
	if (support.getInternalProperty(this, 'class') !== 'Boolean') throw "TypeError" // FIX
	return '' + support.getInternalProperty(this, 'value');
};

var natives = support.natives;
var NaN = this.NaN = support.NaN;
var Infinity = this.Infinity = support.Infinity;
var undefined = this.undefined = support.undefined;
var isNaN = this.isNaN = function(v) { return natives.isNaN(+v); };
var isFinite = this.isFinite = function(v) { return natives.isFinite(+v); };

Math = support.createWrapper('Math', undefined);
var abs = Math.abs = function(v) { return ((v = +v) < 0 ? -v : v); }
Math.floor = function(v) { return natives.floor(+v); }
var sign = Math.sign = function(v) { return ((v = +v) < 0 ? -1 : (v > 0 ? 1 : 0)); }
Math.sqrt = function(v) { return natives.sqrt(+v); }

function toInteger(v) {
	return (isNaN(v = +v) ? 0.0 : (!isFinite(v) ? v : sign(v) * natives.floor(abs(v))));
};

Number = function(v) {
	v = (arguments.length ? +v : 0);
	//if (support.calledAsConstructor()) {
		return support.createWrapper('Number', v, support.prototypes.Number);
};
Number.prototype = support.prototypes.Number;
Number.prototype.valueOf = function() {
	if (support.getInternalProperty(this, 'class') !== 'Number') throw "TypeError" // fix
	return support.getInternalProperty(this, 'value');
};
var alphaDigits = "0123456789abcdefghijklmnopqrstuvwxyz";
Number.prototype.toString = function(radix) {
	if (support.getInternalProperty(this, 'class') !== 'Number') throw "TypeError" // fix
	var n = support.getInternalProperty(this, 'value');
	var r;
	if (radix === void 0 || !natives.isFinite(n)) {
		return '' + n;
	} else if ((r = toInteger(radix)) != radix || r < 2 || r > 36) {
		throw "range error"; // fix
	} else {
		var v = toInteger(abs(n)); // TODO : fraction?
		var s = '';
		do {
			s = alphaDigits[v % r] + s;
			v = natives.floor(v / r);
		} while (v > 0);
		return (n < 0 ? '-' : '') + s;
	}
};

String = function(v) {
	v = (arguments.length ? '' + v : '');
	return support.createWrapper('String', v, support.prototypes.String);
};
String.prototype = support.prototypes.String;
String.prototype.valueOf = function() {
	if (support.getInternalProperty(this, 'class') !== 'String') throw "TypeError" // fix
	return support.getInternalProperty(this, 'value');
}
String.fromCharCode = function() {
	var s = '', l = arguments.length;
	for (var i = 0; i < l; ++i) s += natives.fromCharCode(arguments[i] & 65535);
	return s;
}
String.prototype.toString = function() {
	if (support.getInternalProperty(this, 'class') !== 'String') throw "TypeError" // fix
	return support.getInternalProperty(this, 'value');
}
