/*
	support: {
		prototypes: {	// built-in prototype objects
			object, function, string, boolean, number, array
		},
		asin(), atan() etc..
		isNaN(), isFinite()
		defineProperty(object: object, property: string, value: any, readOnly: boolean, dontEnum: boolean, dontDelete: boolean): boolean
		compileFunction(sourceCode: string, name: string): function
		createWrapper(className: string, internalValue: any, prototype: object): object
		specialConstructor(regularCall: function): function									// = exception on construction and no .prototype either
		specialConstructor(regularCall: function, constructorCall: function): function
		getInternalProperty(object: object, 'class'|'value'|'prototype'): any
		hasOwnProperty(object: object, name: string): boolean
		isPropertyEnumerable(object: object, name: string): boolean
		undefined
		NaN
		Infinity
	}
*/

var global = this;
var unconstructable = support.specialConstructor; // these are the same now, but not guaranteed in the future

var isNaN = function isNaN(v) { return support.isNaN(+v); };
var isFinite = function isFinite(v) { return support.isFinite(+v); };
function toInteger(v) {
	return (isNaN(v = +v) ? 0.0 : (!isFinite(v) ? v : sign(v) * support.floor(abs(v))));
};
var abs = function abs(v) { return ((v = +v) < 0 ? -v : v); }
var sign = function sign(v) { return ((v = +v) < 0 ? -1 : (v > 0 ? 1 : 0)); }
var alphaDigits = "0123456789abcdefghijklmnopqrstuvwxyz";

function isPrimitive(v) {
	var v;
	return (((t = typeof v) !== "object" || v === null) && t !== "function");
}

// FIX : join these two maybe?
support.toPrimitiveNumber = function toPrimitiveNumber(o) {
	var f, v;
	if ((typeof (f = o.valueOf) !== 'function' || !isPrimitive(v = o.valueOf()))
			&& (typeof (f = o.toString) !== 'function' || !isPrimitive(v = o.toString()))) {
		throw TypeError("Error converting object to primitive type");
	}
	return v;
}

support.toPrimitiveString = function toPrimitiveString(o) {
	var f, v;
	if ((typeof (f = o.toString) !== 'function' || !isPrimitive(v = o.toString()))
			&& (typeof (f = o.valueOf) !== 'function' || !isPrimitive(v = o.valueOf()))) {
		throw TypeError("Error converting object to primitive type");
	}
	return v;
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

var Object = function Object(v) {
	switch (typeof v) {
		case "object":
		case "function": if (v !== null) return v;
		case "undefined": return { };
		case "boolean": return new Boolean(v);
		case "number": return new Number(v);
		case "string": return new String(v);
	}
};
defineProperties(Object, { dontDelete: true }, { prototype: support.prototypes.Object });
defineProperties(Object.prototype, { dontEnum: true }, {
	constructor: Object,
	valueOf: unconstructable(function valueOf() { return this; }),
	toString: unconstructable(function toString() {
		var s;
		return '[object ' + (((s = support.getInternalProperty(this, 'class')) === 'Arguments') ? 'Object' : s) + ']'
	}),
	hasOwnProperty: unconstructable(function hasOwnProperty(name) { return support.hasOwnProperty(this, '' + name) }),
	propertyIsEnumerable: unconstructable(function propertyIsEnumerable(name) { return support.isPropertyEnumerable(this, '' + name) }),
	isPrototypeOf: unconstructable(function isPrototypeOf(v) {
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

var Function = function Function(body) {
	var source = '(', n = arguments.length - 1;
	for (var i = 0; i < n; ++i) {
		source += arguments[i];
		if (i < n - 1) s += ',';
	}
	source += ') { ';
	if (n >= 0) source += arguments[n];
	return support.compileFunction(source + ' }', 'anonymous');
};
defineProperties(Function, { dontDelete: true }, { prototype: support.prototypes.Function });
defineProperties(Function.prototype, { dontEnum: true }, {
	constructor: Function,
	apply: unconstructable(function apply(thisArg, argArray) { // FIX : <- 100% native version in the future I think
		var theClass;
		if (argArray == null) argArray = [ ];
		else if ((theClass = support.getInternalProperty(argArray, 'class')) !== 'Array' && theClass !== 'Arguments') {
			throw TypeError("Argument list has wrong type");
		};
		return support.callWithArgs(this, thisArg, argArray, 0);
	}),
	call: unconstructable(function call(thisArg) { // FIX : <- 100% native version in the future I think
		return support.callWithArgs(this, thisArg, arguments, 1);
	}),
	toString: unconstructable(function toString() { // FIX : <- generic, make a factory function
		checkClass(this, 'Function', 'toString');
		return support.getInternalProperty(this, 'value');
	})
});

/* --- Boolean --- */

var Boolean = support.specialConstructor(function Boolean(v) {
		return !!v;
	}, function Boolean(v) {
		return support.createWrapper('Boolean', !!v, support.prototypes.Boolean);
	}
);
defineProperties(Boolean, { dontDelete: true }, { prototype: support.prototypes.Boolean });
defineProperties(Boolean.prototype, { dontEnum: true }, {
	constructor: Boolean,
	valueOf: unconstructable(function valueOf() {
		checkClass(this, 'Boolean', 'valueOf');
		return support.getInternalProperty(this, 'value');
	}),
	toString: unconstructable(function toString() {
		checkClass(this, 'Boolean', 'toString');
		return '' + support.getInternalProperty(this, 'value');
	})
});

/* --- Number --- */

var Number = support.specialConstructor(function Number(v) {
		return (arguments.length ? +v : 0);
	}, function Number(v) {
		return support.createWrapper('Number', arguments.length ? +v : 0, support.prototypes.Number);
	}
);
defineProperties(Number, { dontDelete: true }, { prototype: support.prototypes.Number });
defineProperties(Number, { dontEnum: true, readOnly: true, dontDelete: true }, {
	MAX_VALUE: support.maxNumber,
	MIN_VALUE: support.minNumber,
	NaN: support.NaN,
	NEGATIVE_INFINITY: -support.Infinity,
	POSITIVE_INFINITY: support.Infinity
});
defineProperties(Number.prototype, { dontEnum: true }, {
	constructor: Number,
	valueOf: unconstructable(function valueOf() {
		checkClass(this, 'Number', 'valueOf');
		return support.getInternalProperty(this, 'value');
	}),
	toString: unconstructable(function toString(radix) {
		checkClass(this, 'Number', 'toString');
		var n = support.getInternalProperty(this, 'value');
		var r;
		if (radix === void 0 || !support.isFinite(n)) {
			return '' + n;
		} else if ((r = toInteger(radix)) != radix || r < 2 || r > 36) {
			throw RangeError("illegal radix for toString()");
		} else {
			var v = toInteger(abs(n)); // TODO : fraction?
			var s = '';
			do {
				s = alphaDigits[v % r] + s;
				v = support.floor(v / r);
			} while (v > 0);
			return (n < 0 ? '-' : '') + s;
		}
	})
});

/* --- String --- */

var String = support.specialConstructor(function String(v) {
		return (arguments.length ? '' + v : '');
	}, function String(v) {
		var s;
		return defineProperties(
				support.createWrapper('String', (s = arguments.length ? '' + v : ''), support.prototypes.String)
				,  { readOnly: true, dontEnum: true, dontDelete: true }, { length: s.length });
	}
);
defineProperties(String, { dontDelete: true }, { prototype: support.prototypes.String });
defineProperties(String, { dontEnum: true }, {
	fromCharCode: unconstructable(function fromCharCode(v) {
		var l;
		if ((l = arguments.length) === 1) return support.fromCharCode(v & 65535);
		for (var i = 0, s = ''; i < l; ++i) s += support.fromCharCode(arguments[i] & 65535);
		return s;
	})
});
defineProperties(String.prototype, { dontEnum: true }, {
	constructor: String,
	valueOf: unconstructable(function valueOf() {
		checkClass(this, 'String', 'valueOf');
		return support.getInternalProperty(this, 'value');
	}),
	toString: unconstructable(function toString() {
		checkClass(this, 'String', 'toString');
		return support.getInternalProperty(this, 'value');
	})
});

/* --- Array --- */

var Array = function Array(v) {
	var a = [ ], args, l;
	if ((l = (args = arguments).length) === 1 && typeof v === 'number') {
		if ((v >>> 0) !== v) throw RangeError("Invalid array length");
		a.length = v;
	} else {
		for (var i = 0; i < l; ++i) a[i] = args[i];
	}
	return a;
}
defineProperties(Array, { dontDelete: true }, { prototype: support.prototypes.Array });
defineProperties(Array.prototype, { dontEnum: true }, {
	constructor: Array,
	push: unconstructable(function push(item) {
		var args, offset = this.length, end = (args = arguments).length + offset;
		for (var i = offset; i < end; ++i) this[i] = args[i - offset];
		return end;
	}),
	pop: unconstructable(function pop() {
		var v = void 0, l;
		if ((l = this.length) > 0) {
			v = this[l - 1];
			--this.length;
		}
		return v;
	}),
	join: unconstructable(function join(separator) {
		var s, s2, l = this.length;
		separator = (separator === void 0 ? ',' : '' + separator);
		if ((s = this[0]) == void 0) s = '';
		for (var i = 1; i < l; ++i) {
			s += separator;
			if ((s2 = this[i]) != null) s += s2;
		}
		return s;
	}),
	toString: unconstructable(function toString() {
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

defineProperties(global, { dontEnum: true, dontDelete: true }, {
	NaN: support.NaN, Infinity: support.Infinity, undefined: support.undefined
});

/* --- Math --- */

// FIX : more decimals needed? these are from ECMA-262
defineProperties(Math, { readOnly: true, dontEnum: true, dontDelete: true }, {
	E: 2.7182818284590452354,
	LN10: 2.302585092994046,
	LN2: 0.6931471805599453,
	LOG2E: 1.4426950408889634,
	LOG10E: 0.4342944819032518,
	PI: 3.1415926535897932,
	SQRT1_2: 0.7071067811865476,
	SQRT2: 1.4142135623730951
});

defineProperties(Math, { dontEnum: true }, { // FIX : construction from array instead?
	abs: unconstructable(abs),
	acos: unconstructable(function acos(v) { return support.acos(+v); }),
	asin: unconstructable(function asin(v) { return support.asin(+v); }),
	atan: unconstructable(function atan(v) { return support.atan(+v); }),
	ceil: unconstructable(function ceil(v) { return -support.floor(-v); }),
	cos: unconstructable(function cos(v) { return support.cos(+v); }),
	exp: unconstructable(function exp(v) { return support.exp(+v); }),
	floor: unconstructable(function floor(v) { return support.floor(+v); }),
	log: unconstructable(function log(v) { return support.log(+v); }),
	round: unconstructable(function round(v) { return support.floor(v + 0.5); }), // FIX : -0 rule in ES262
	sin: unconstructable(function sin(v) { return support.sin(+v); }),
	sign: unconstructable(sign), // FIX : not ES3 standard??
	sqrt: unconstructable(function sqrt(v) { return support.sqrt(+v); }),
	tan: unconstructable(function tan(v) { return support.tan(+v); })

	// trunc etc from ES6?
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
    /* FIX : not necessary for strict es3, where .name isn't included at all in the official specs but is my own extension using standard flags
		delete constructor.name; // ugly trick to set the read-only, but not undeletable .name on the anonymous function
    */
		constructor.name = name;
		defineProperties(constructor, { dontDelete: true }, { prototype: prototype });
		defineProperties(prototype, { dontEnum: true }, { constructor: constructor });
		prototype.name = name;
	}

	defineProperties(Error.prototype, { dontEnum: true }, {
		message: "",
		toString: unconstructable(function toString() {
			return (this.name === void 0 ? 'Error' : this.name) + (this.message ? (': ' + this.message) : '');
		})
	});
})();

/* --- ES >3 polyfills --- */

// These are not guaranteed to be 100% compatible

defineProperties(Object, { dontEnum: true }, {
	defineProperty: unconstructable(function defineProperty(o, p, d) {
		support.defineProperty(o, '' + p, d.value, !d.writable, !d.enumerable, !d.configurable);
	}),
	getPrototypeOf: unconstructable(function getPrototypeOf(o) {
		return support.getInternalProperty(o, 'prototype');
	})
});
