var natives = support.natives;

var isNaN = function(v) { return natives.isNaN(+v); };
var isFinite = function(v) { return natives.isFinite(+v); };
function toInteger(v) {
	return (isNaN(v = +v) ? 0.0 : (!isFinite(v) ? v : sign(v) * natives.floor(abs(v))));
};
var abs = function(v) { return ((v = +v) < 0 ? -v : v); }
var sign = function(v) { return ((v = +v) < 0 ? -1 : (v > 0 ? 1 : 0)); }
var alphaDigits = "0123456789abcdefghijklmnopqrstuvwxyz";

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

function isPrimitive(v) {
	var v;
	return (((t = typeof v) !== "object" || v === null) && t !== "function");
}

support.toPrimitive = function(o) {
	var f, v;
	if ((typeof (f = o.valueOf) !== 'function' || !isPrimitive(v = o.valueOf()))
			&& (typeof (f = o.toString) !== 'function' || !isPrimitive(v = o.toString()))) {
		throw("error"); // FIX
	}
	return v;
}

support.toString = function(o) {
	var f, v;
	if ((typeof (f = o.toString) !== 'function' || !isPrimitive(v = o.toString()))
			&& (typeof (f = o.valueOf) !== 'function' || !isPrimitive(v = o.valueOf()))) {
		throw("error"); // FIX
	}
	return '' + v;
}

function defineProperties(object, attribs, props) {
	var readOnly = attribs['readOnly'], dontEnum = attribs['dontEnum'], dontDelete = attribs['dontDelete'];
	for (p in props) support.defineProperty(object, p, props[p], readOnly, dontEnum, dontDelete);
	return object;
}
/*
function createClass(constructorFunction, regularFunction, props) {
	return defineProperties(support.specialConstructor(constructorFunction, regularFunction), { dontEnum }, { ;
}
*/
defineProperties(this, { dontEnum: true }, {
	Object: function(v) {
		switch (typeof v) {
			case "object":
			case "function": if (v !== null) return v;
			case "undefined": return { };
			case "boolean": return new Boolean(v);
			case "number": return new Number(v);
			case "string": return new String(v);
		}
	},
	Function: function(body) {
		var source = '(', n = arguments.length - 1;
		for (var i = 0; i < n; ++i) {
			source += arguments[i];
			if (i < n - 1) s += ',';
		}
		source += ') { ';
		if (n >= 0) source += arguments[n];
		return support.compileFunction(source + ' }', 'anonymous');
	},
	Boolean: support.specialConstructor(function(v) {
			return support.createWrapper('Boolean', !!v, support.prototypes.Boolean);
		}, function(v) {
			return !!v;
		}
	),
	Number: support.specialConstructor(function(v) {
			return support.createWrapper('Number', arguments.length ? +v : 0, support.prototypes.Number);
		}, function(v) {
			return (arguments.length ? +v : 0);
		}
	),
	String: support.specialConstructor(function(v) {
			return support.createWrapper('String', arguments.length ? '' + v : '', support.prototypes.String);
		}, function(v) {
			return (arguments.length ? '' + v : '');
		}
	),
	isNaN: isNaN,
	isFinite: isFinite,
	Math: support.createWrapper('Math', undefined)
});

defineProperties(this, { readOnly: true, dontEnum: true, dontDelete: true }, {
	NaN: support.NaN, Infinity: support.Infinity, undefined: support.undefined
});

defineProperties(Math, { dontEnum: true }, {
	abs: abs,
	floor: function(v) { return natives.floor(+v); },
	sign: sign,
	sqrt: function(v) { return natives.sqrt(+v); }
});

support.defineProperty(Object, 'prototype', support.prototypes.Object, true, true, true);
defineProperties(Object.prototype, { dontEnum: true }, {
	constructor: Object,
	valueOf: function() {
		return this;
	},
	toString: function() {
		return '[object ' + support.getInternalProperty(this, 'class') + ']'
	},
	isPrototypeOf: function(v) {
		while (v = support.getInternalProperty(v, 'prototype')) {
			if (v == this) return true;
		}
		return false;
	}
});
defineProperties(Object.prototype, { dontEnum: true }, {
	toLocaleString: Object.prototype.toString
});

support.defineProperty(Function, 'prototype', support.prototypes.Function, true, true, true);
defineProperties(Function.prototype, { dontEnum: true }, {
	constructor: Function,
	toString: function() { // FIX : <- generic, make a factory function
		if (support.getInternalProperty(this, 'class') !== 'Function') throw "TypeError" // FIX
		return support.getInternalProperty(this, 'value');
	}
});

support.defineProperty(Boolean, 'prototype', support.prototypes.Boolean, true, true, true);
defineProperties(Boolean.prototype, { dontEnum: true }, {
	constructor: Boolean,
	valueOf: function() {
		if (support.getInternalProperty(this, 'class') !== 'Boolean') throw "TypeError" // FIX
		return support.getInternalProperty(this, 'value');
	},
	toString: function() {
		if (support.getInternalProperty(this, 'class') !== 'Boolean') throw "TypeError" // FIX
		return '' + support.getInternalProperty(this, 'value');
	}
});

support.defineProperty(Number, 'prototype', support.prototypes.Number, true, true, true);
defineProperties(Number.prototype, { dontEnum: true }, {
	constructor: Number,
	valueOf: function() {
		if (support.getInternalProperty(this, 'class') !== 'Number') throw "TypeError" // fix
		return support.getInternalProperty(this, 'value');
	},
	toString: function(radix) {
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
	}
});

support.defineProperty(String, 'prototype', support.prototypes.String, true, true, true);
defineProperties(String.prototype, { dontEnum: true }, {
	constructor: String,
	valueOf: function() {
		if (support.getInternalProperty(this, 'class') !== 'String') throw "TypeError" // fix
		return support.getInternalProperty(this, 'value');
	},
	toString: function() {
		if (support.getInternalProperty(this, 'class') !== 'String') throw "TypeError" // fix
		return support.getInternalProperty(this, 'value');
	}
});

defineProperties(String, { dontEnum: true }, {
	fromCharCode: function(v) {
		var l;
		if ((l = arguments.length) === 1) return natives.fromCharCode(v & 65535);
		for (var i = 0, s = ''; i < l; ++i) s += natives.fromCharCode(arguments[i] & 65535);
		return s;
	}
});

