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

function defineProperties(object, attribs, props) {
	var readOnly = attribs['readOnly'];
	var dontEnum = attribs['dontEnum'];
	var dontDelete = attribs['dontDelete'];
	for (p in props) {
		support.defineProperty(object, p, props[p], readOnly, dontEnum, dontDelete);
	}
}

defineProperties(this, { dontEnum: true }, {
	Object: function() { return {}; },
	Function: function(body) {
		var source = '(', n = arguments.length - 1;
		for (var i = 0; i < n; ++i) sources += arguments[i] + (i < n - 1 ? ',' : ')');
		if (n >= 0) source += arguments[n];
		return support.compileFunction(source);
	},
	Boolean: function(v) {
		return support.createWrapper('Boolean', !!v, support.prototypes.Boolean);
	},
	Number: function(v) {
		v = (arguments.length ? +v : 0);
		//if (support.calledAsConstructor()) {
			return support.createWrapper('Number', v, support.prototypes.Number);
	},
	String: function(v) {
		v = (arguments.length ? '' + v : '');
		return support.createWrapper('String', v, support.prototypes.String);
	},
	isNaN: isNaN,
	isFinite: isFinite,
	Math: support.createWrapper('Math', undefined)
});

defineProperties(Object.prototype = support.prototypes.Object, { dontEnum: true }, {
	valueOf: function() {
		return this;
	},
	toString: function() {
		return '[object ' + support.getInternalProperty(this, 'class') + ']'
	},
	isPrototypeOf: function(v) {
		while ((v = support.getInternalProperty(v, 'prototype')) != null) {
			if (v == this) return true;
		}
		return false;
	}
});
defineProperties(Object.prototype, { dontEnum: true }, {
	toLocaleString: Object.prototype.toString
});

defineProperties(Function.prototype = support.prototypes.Function, { dontEnum: true }, {
	toString: function() { // FIX : <- generic, make a factory function
		if (support.getInternalProperty(this, 'class') !== 'Function') throw "TypeError" // FIX
		return support.getInternalProperty(this, 'value');
	}
});

defineProperties(Boolean.prototype = support.prototypes.Boolean, { dontEnum: true }, {
	valueOf: function() {
		if (support.getInternalProperty(this, 'class') !== 'Boolean') throw "TypeError" // FIX
		return support.getInternalProperty(this, 'value');
	},
	toString: function() {
		if (support.getInternalProperty(this, 'class') !== 'Boolean') throw "TypeError" // FIX
		return '' + support.getInternalProperty(this, 'value');
	}
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

defineProperties(Number.prototype = support.prototypes.Number, { dontEnum: true }, {
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

defineProperties(String.prototype = support.prototypes.String, { dontEnum: true }, {
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

