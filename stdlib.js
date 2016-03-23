/*
	support: {
		prototypes: {	// built-in prototype objects
			object, function, string, boolean, number, array
		},
		natives: {		// built-in native functions (mainly math etc)
			asin(), atan() etc..
		},
		defineProperty(object: object, property: string, value: any, readOnly: boolean, dontEnum: boolean, dontDelete: boolean): boolean
		compileFunction(sourceCode: string, name: string): function
		createWrapper(className: string, internalValue: any, prototype: object): object
		specialConstructor(regularCall: function): function									// = exception on construction and no .prototype either
		specialConstructor(regularCall: function, constructorCall: function): function
		getInternalProperty(object: object, 'class'|'value'|'prototype'): any
		undefined
		NaN
		Infinity
	}
*/

var global = this;
var natives = support.natives;
var unconstructable = support.specialConstructor; // these are the same now, but not guaranteed in the future

var isNaN = function(v) { return natives.isNaN(+v); };
var isFinite = function(v) { return natives.isFinite(+v); };
function toInteger(v) {
	return (isNaN(v = +v) ? 0.0 : (!isFinite(v) ? v : sign(v) * natives.floor(abs(v))));
};
var abs = function(v) { return ((v = +v) < 0 ? -v : v); }
var sign = function(v) { return ((v = +v) < 0 ? -1 : (v > 0 ? 1 : 0)); }
var alphaDigits = "0123456789abcdefghijklmnopqrstuvwxyz";

function isPrimitive(v) {
	var v;
	return (((t = typeof v) !== "object" || v === null) && t !== "function");
}

support.toPrimitive = function(o) {
	var f, v;
	if ((typeof (f = o.valueOf) !== 'function' || !isPrimitive(v = o.valueOf()))
			&& (typeof (f = o.toString) !== 'function' || !isPrimitive(v = o.toString()))) {
		throw TypeError("Error converting object to primitive type");
	}
	return v;
}

support.toString = function(o) {
	var f, v;
	if ((typeof (f = o.toString) !== 'function' || !isPrimitive(v = o.toString()))
			&& (typeof (f = o.valueOf) !== 'function' || !isPrimitive(v = o.valueOf()))) {
		throw TypeError("Error converting object to primitive type");
	}
	return '' + v;
}

function defineProperties(object, attribs, props) {
	var readOnly = attribs.readOnly, dontEnum = attribs.dontEnum, dontDelete = attribs.dontDelete;
	for (var p in props) support.defineProperty(object, p, props[p], readOnly, dontEnum, dontDelete);
	return object;
}

function checkClass(object, expectedClass, forFunction) {
	if (support.getInternalProperty(object, 'class') !== expectedClass) {
		throw TypeError(expectedClass + ".prototype." + forFunction + " is not generic");
	}
}

/* --- Object --- */

var Object = function(v) {
	switch (typeof v) {
		case "object":
		case "function": if (v !== null) return v;
		case "undefined": return { };
		case "boolean": return new Boolean(v);
		case "number": return new Number(v);
		case "string": return new String(v);
	}
};
defineProperties(Object, { dontEnum: true, readOnly: true, dontDelete: true }, { prototype: support.prototypes.Object });
defineProperties(Object.prototype, { dontEnum: true }, {
	constructor: Object,
	valueOf: unconstructable(function() {
		return this;
	}),
	toString: unconstructable(function() {
		return '[object ' + support.getInternalProperty(this, 'class') + ']'
	}),
	isPrototypeOf: unconstructable(function(v) {
		while (v = support.getInternalProperty(v, 'prototype')) {
			if (v == this) return true;
		}
		return false;
	})
});
defineProperties(Object.prototype, { dontEnum: true }, {
	toLocaleString: Object.prototype.toString
});

/* --- Function --- */

var Function = function(body) {
	var source = '(', n = arguments.length - 1;
	for (var i = 0; i < n; ++i) {
		source += arguments[i];
		if (i < n - 1) s += ',';
	}
	source += ') { ';
	if (n >= 0) source += arguments[n];
	return support.compileFunction(source + ' }', 'anonymous');
};
defineProperties(Function, { dontEnum: true, readOnly: true, dontDelete: true }, { prototype: support.prototypes.Function });
defineProperties(Function.prototype, { dontEnum: true }, {
	constructor: Function,
	apply: unconstructable(function(thisArg, argArray) { // FIX : <- 100% native version in the future I think
		var theClass;
		if (argArray == null) argArray = [ ];
		else if ((theClass = support.getInternalProperty(argArray, 'class')) !== 'Array' && theClass !== 'Arguments') {
			throw TypeError("argument list for apply() has wrong type");
		};
		return support.callWithArgs(this, thisArg, argArray, 0);
	}),
	call: unconstructable(function(thisArg) { // FIX : <- 100% native version in the future I think
		return support.callWithArgs(this, thisArg, arguments, 1);
	}),
	toString: unconstructable(function() { // FIX : <- generic, make a factory function
		checkClass(this, 'Function', 'toString');
		return support.getInternalProperty(this, 'value');
	})
});

/* --- Boolean --- */

var Boolean = support.specialConstructor(function(v) {
		return !!v;
	}, function(v) {
		return support.createWrapper('Boolean', !!v, support.prototypes.Boolean);
	}
);
defineProperties(Boolean, { dontEnum: true, readOnly: true, dontDelete: true }, { prototype: support.prototypes.Boolean });
defineProperties(Boolean.prototype, { dontEnum: true }, {
	constructor: Boolean,
	valueOf: unconstructable(function() {
		checkClass(this, 'Boolean', 'valueOf');
		return support.getInternalProperty(this, 'value');
	}),
	toString: unconstructable(function() {
		checkClass(this, 'Boolean', 'toString');
		return '' + support.getInternalProperty(this, 'value');
	})
});

/* --- Number --- */

var Number = support.specialConstructor(function(v) {
		return (arguments.length ? +v : 0);
	}, function(v) {
		return support.createWrapper('Number', arguments.length ? +v : 0, support.prototypes.Number);
	}
);
defineProperties(Number, { dontEnum: true, readOnly: true, dontDelete: true }, {
	prototype: support.prototypes.Number,
	MAX_VALUE: support.maxNumber,
	MIN_VALUE: support.minNumber,
	NaN: support.NaN,
	NEGATIVE_INFINITY: -support.Infinity,
	POSITIVE_INFINITY: support.Infinity
});
defineProperties(Number.prototype, { dontEnum: true }, {
	constructor: Number,
	valueOf: unconstructable(function() {
		checkClass(this, 'Number', 'valueOf');
		return support.getInternalProperty(this, 'value');
	}),
	toString: unconstructable(function(radix) {
		checkClass(this, 'Number', 'toString');
		var n = support.getInternalProperty(this, 'value');
		var r;
		if (radix === void 0 || !natives.isFinite(n)) {
			return '' + n;
		} else if ((r = toInteger(radix)) != radix || r < 2 || r > 36) {
			throw RangeError("illegal radix for toString()");
		} else {
			var v = toInteger(abs(n)); // TODO : fraction?
			var s = '';
			do {
				s = alphaDigits[v % r] + s;
				v = natives.floor(v / r);
			} while (v > 0);
			return (n < 0 ? '-' : '') + s;
		}
	})
});

/* --- String --- */

var String = support.specialConstructor(function(v) {
		return (arguments.length ? '' + v : '');
	}, function(v) {
		return support.createWrapper('String', arguments.length ? '' + v : '', support.prototypes.String);
	}
);
defineProperties(String, { dontEnum: true, readOnly: true, dontDelete: true }, { prototype: support.prototypes.String });
defineProperties(String, { dontEnum: true }, {
	fromCharCode: unconstructable(function(v) {
		var l;
		if ((l = arguments.length) === 1) return natives.fromCharCode(v & 65535);
		for (var i = 0, s = ''; i < l; ++i) s += natives.fromCharCode(arguments[i] & 65535);
		return s;
	})
});
defineProperties(String.prototype, { dontEnum: true }, {
	constructor: String,
	valueOf: unconstructable(function() {
		checkClass(this, 'String', 'valueOf');
		return support.getInternalProperty(this, 'value');
	}),
	toString: unconstructable(function() {
		checkClass(this, 'String', 'toString');
		return support.getInternalProperty(this, 'value');
	})
});

/* --- Array --- */

var Array = function(v) {
	var a = [ ], args, l;
	if ((l = (args = arguments).length) === 1 && typeof v === 'number') {
		if ((v >>> 0) !== v) throw RangeError("Invalid array length");
		a.length = v;
	} else {
		for (var i = 0; i < l; ++i) a[i] = args[i];
	}
	return a;
}
defineProperties(Array, { dontEnum: true, readOnly: true, dontDelete: true }, { prototype: support.prototypes.Array });
defineProperties(Array.prototype, { dontEnum: true }, {
	constructor: Array,
	push: unconstructable(function(item) {
		var args, offset = this.length, end = (args = arguments).length + offset;
		for (var i = offset; i < end; ++i) this[i] = args[i - offset];
		return end;
	}),
	pop: unconstructable(function() {
		var v = void 0, l;
		if ((l = this.length) > 0) {
			v = this[l - 1];
			--this.length;
		}
		return v;
	}),
	join: unconstructable(function(separator) {
		separator = (separator === void 0 ? ',' : '' + separator);
		var s = (this[0] == void 0 ? '' : this[0]);
		var l = this.length;
		for (var i = 1; i < l; ++i) s += separator + (this[i] == null ? '' : this[i]);
		return s;
	}),
	toString: unconstructable(function() {
		checkClass(this, 'Array', 'toString');
		return this.join();
	})
});

/* --- Set up globals --- */

defineProperties(global, { dontEnum: true }, {
	Object: Object,
	Function: Function,
	Boolean: Boolean,
	Number: Number,
	String: String,
	Array: Array,
	isNaN: unconstructable(isNaN),
	isFinite: unconstructable(isFinite),
	Math: support.createWrapper('Math', void 0)
});

defineProperties(global, { readOnly: true, dontEnum: true, dontDelete: true }, {
	NaN: support.NaN, Infinity: support.Infinity, undefined: support.undefined
});

/* --- Math --- */

defineProperties(Math, { dontEnum: true }, {
	abs: unconstructable(abs),
	floor: unconstructable(function(v) { return natives.floor(+v); }),
	sign: unconstructable(sign),
	sqrt: unconstructable(function(v) { return natives.sqrt(+v); })
});

/* --- Errors --- */

function createErrorConstructor(name, prototype) {
	return function(message) {
		var e;
		support.defineProperty(e = support.createWrapper('Error', name, prototype), 'message'
				, (message !== void 0 ? '' + message : ''), false, true, false);
		return e
	}
};

(function() {
	var ERROR_NAMES = [ 'Error', 'RangeError', 'ReferenceError', 'SyntaxError', 'TypeError' ];

	for (var i = ERROR_NAMES.length - 1; i >= 0; --i) {
		var name, constructor, prototype;
		support.defineProperty(global, name = ERROR_NAMES[i]
				, constructor = createErrorConstructor(name, prototype = support.prototypes[name]), false, true, false);
		support.defineProperty(constructor, 'prototype', prototype, true, true, true);
		defineProperties(prototype, { dontEnum: true }, { constructor: constructor, name: name });
	}

	defineProperties(Error.prototype, { dontEnum: true }, {
		message: "",
		toString: unconstructable(function() { return this.name + (this.message ? (': ' + this.message) : ''); })
	});
})();
