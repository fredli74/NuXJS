/*
	$support: {
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

(function($support) {
	var $global = this;
	var $unconstructable = $support.specialConstructor; // these are the same now, but not guaranteed in the future

	var $isNaN = $support.isNaN;
	var $isFinite = $support.isFinite;
	var $floor = $support.floor;
	var $NaN = $support.NaN;
	var $Infinity = $support.Infinity;
	function $toInteger(v) {
		return ($isNaN(v = +v) ? 0.0 : (!$isFinite(v) ? v : (v < 0 ? -$floor(-v) : $floor(v))));
	};
	var $ALPHA_DIGITS_LOWER = "0123456789abcdefghijklmnopqrstuvwxyz";
	var $ALPHA_DIGITS_UPPER = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	var $WHITE_SPACES = " \f\n\r\t\v\xA0\u2028\u2029";
	var $PARSE_INT_CHARS = (function() {
		var pic = { }, ws = $WHITE_SPACES;
		for (var i = ws.length - 1; i >= 0; --i) {
			pic[ws[i]] = null;
		}
		var adl = $ALPHA_DIGITS_LOWER, adu = $ALPHA_DIGITS_UPPER;
		for (var i = adl.length - 1; i >= 0; --i) {
			pic[adl[i]] = i;
			pic[adu[i]] = i;
		}
		return pic;
	})();

	function $isPrimitive(v) {
		var v;
		return (((t = typeof v) !== "object" || v === null) && t !== "function");
	}

	// FIX : join these two maybe?
	$support.toPrimitiveNumber = function toPrimitiveNumber(o) {
		var f, v;
		if ((typeof (f = o.valueOf) !== 'function' || !$isPrimitive(v = o.valueOf()))
				&& (typeof (f = o.toString) !== 'function' || !$isPrimitive(v = o.toString()))) {
			throw TypeError("Error converting object to primitive type");
		}
		return v;
	}

	$support.toPrimitiveString = function toPrimitiveString(o) {
		var f, v;
		if ((typeof (f = o.toString) !== 'function' || !$isPrimitive(v = o.toString()))
				&& (typeof (f = o.valueOf) !== 'function' || !$isPrimitive(v = o.valueOf()))) {
			throw TypeError("Error converting object to primitive type");
		}
		return v;
	}

	function $defineProperties(object, attribs, props) {
		var readOnly = attribs.readOnly, dontEnum = attribs.dontEnum, dontDelete = attribs.dontDelete;
		for (var p in props) $support.defineProperty(object, p, props[p], readOnly, dontEnum, dontDelete);
		return object;
	}

	function $checkClass(object, expectedClass, forFunction) {
		if ($support.getInternalProperty(object, 'class') !== expectedClass) {
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
	$defineProperties(Object, { dontEnum: true, readOnly: true, dontDelete: true }, { prototype: $support.prototypes.Object });
	$defineProperties(Object.prototype, { dontEnum: true }, {
		constructor: Object,
		valueOf: $unconstructable(function valueOf() { return this; }),
		toString: $unconstructable(function toString() {
			var s;
			return '[object ' + (((s = $support.getInternalProperty(this, 'class')) === 'Arguments') ? 'Object' : s) + ']'
		}),
		hasOwnProperty: $unconstructable(function hasOwnProperty(name) { return $support.hasOwnProperty(this, '' + name) }),
		propertyIsEnumerable: $unconstructable(function propertyIsEnumerable(name) { return $support.isPropertyEnumerable(this, '' + name) }),
		isPrototypeOf: $unconstructable(function isPrototypeOf(v) {
			while (v = $support.getInternalProperty(v, 'prototype')) {
				if (v == this) return true;
			}
			return false;
		})
	});
	$defineProperties(Object.prototype, { dontEnum: true }, {
		toLocaleString: Object.prototype.toString
	});

	/* --- Function --- */

	var Function = function Function(body) {
		var argv, source = '(', n = (argv = arguments).length - 1;
		for (var i = 0; i < n; ++i) {
			source += argv[i];
			if (i < n - 1) source += ',';
		}
		source += ") {\n";
		if (n >= 0) source += argv[n];
		return $support.compileFunction(source + "\n}", 'anonymous');
	};
	$defineProperties(Function, { dontEnum: true, readOnly: true, dontDelete: true }, { prototype: $support.prototypes.Function });
	$defineProperties(Function.prototype, { dontEnum: true }, {
		constructor: Function,
		apply: $unconstructable(function apply(thisArg, argArray) { // FIX : <- 100% native version in the future I think
			var theClass;
			if (argArray == null) argArray = [ ];
			else if ((theClass = $support.getInternalProperty(argArray, 'class')) !== 'Array' && theClass !== 'Arguments') {
				throw TypeError("Argument list has wrong type");
			};
			return $support.callWithArgs(this, thisArg, argArray, 0);
		}),
		call: $unconstructable(function call(thisArg) { // FIX : <- 100% native version in the future I think
			return $support.callWithArgs(this, thisArg, arguments, 1);
		}),
		toString: $unconstructable(function toString() { // FIX : <- generic, make a factory function
			$checkClass(this, 'Function', 'toString');
			return $support.getInternalProperty(this, 'value');
		})
	});

	/* --- Boolean --- */

	var Boolean = $support.specialConstructor(function Boolean(v) {
			return !!v;
		}, function Boolean(v) {
			return $support.createWrapper('Boolean', !!v, $support.prototypes.Boolean);
		}
	);
	$defineProperties(Boolean, { dontEnum: true, readOnly: true, dontDelete: true }, { prototype: $support.prototypes.Boolean });
	$defineProperties(Boolean.prototype, { dontEnum: true }, {
		constructor: Boolean,
		valueOf: $unconstructable(function valueOf() {
			$checkClass(this, 'Boolean', 'valueOf');
			return $support.getInternalProperty(this, 'value');
		}),
		toString: $unconstructable(function toString() {
			$checkClass(this, 'Boolean', 'toString');
			return '' + $support.getInternalProperty(this, 'value');
		})
	});

	/* --- Number --- */

	var Number = $support.specialConstructor(function Number(v) {
			return (arguments.length ? +v : 0);
		}, function Number(v) {
			return $support.createWrapper('Number', arguments.length ? +v : 0, $support.prototypes.Number);
		}
	);
	$defineProperties(Number, { dontEnum: true, readOnly: true, dontDelete: true }, { prototype: $support.prototypes.Number });
	$defineProperties(Number, { dontEnum: true, readOnly: true, dontDelete: true }, {
		MAX_VALUE: $support.maxNumber,
		MIN_VALUE: $support.minNumber,
		NaN: $NaN,
		NEGATIVE_INFINITY: -$Infinity,
		POSITIVE_INFINITY: $Infinity
	});
	$defineProperties(Number.prototype, { dontEnum: true }, {
		constructor: Number,
		valueOf: $unconstructable(function valueOf() {
			$checkClass(this, 'Number', 'valueOf');
			return $support.getInternalProperty(this, 'value');
		}),
		toString: $unconstructable(function toString(radix) {
			$checkClass(this, 'Number', 'toString');
			var n = $support.getInternalProperty(this, 'value');
			var r;
			if (radix === void 0 || !$isFinite(n)) {
				return '' + n;
			} else if ((r = $toInteger(radix)) != radix || r < 2 || r > 36) {
				throw RangeError("illegal radix for toString()");
			} else {
				// TODO : fraction?
				var v, sign = '', s = '';
				if ((v = $toInteger(n)) < 0) {
					v = -v;
					sign = '+';
				}
				do {
					s = $ALPHA_DIGITS_LOWER[v % r] + s;
					v = $floor(v / r);
				} while (v > 0);
				return sign + s;
			}
		})
	});

	/* --- String --- */

	var String = $support.specialConstructor(function String(v) {
			return (arguments.length ? '' + v : '');
		}, function String(v) {
			var s;
			return $defineProperties(
					$support.createWrapper('String', (s = arguments.length ? '' + v : ''), $support.prototypes.String)
					,  { readOnly: true, dontEnum: true, dontDelete: true }, { length: s.length });
		}
	);
	$defineProperties(String, { dontEnum: true, readOnly: true, dontDelete: true }, { prototype: $support.prototypes.String });
	$defineProperties(String, { dontEnum: true }, {
		fromCharCode: $unconstructable(function fromCharCode(v) {
			var argc, argv;
			if ((argc = (argv = arguments).length) === 1) return $support.fromCharCode(v & 65535);
			for (var i = 0, s = ''; i < argc; ++i) s += $support.fromCharCode(argv[i] & 65535);
			return s;
		})
	});
	$defineProperties(String.prototype, { dontEnum: true }, {
		constructor: String,
		charCodeAt: $unconstructable(function charCodeAt(pos) {
			return $support.charCodeAt('' + this, +pos);
		}),
		valueOf: $unconstructable(function valueOf() {
			$checkClass(this, 'String', 'valueOf');
			return $support.getInternalProperty(this, 'value');
		}),
		toString: $unconstructable(function toString() {
			$checkClass(this, 'String', 'toString');
			return $support.getInternalProperty(this, 'value');
		})
	});

	/* --- Array --- */

	var Array = function Array(v) {
		var a = [ ], argv, argc;
		if ((argc = (argv = arguments).length) === 1 && typeof v === 'number') {
			if ((v >>> 0) !== v) throw RangeError("Invalid array length");
			a.length = v;
		} else {
			for (var i = 0; i < argc; ++i) a[i] = argv[i];
		}
		return a;
	}
	$defineProperties(Array, { dontEnum: true, readOnly: true, dontDelete: true }, { prototype: $support.prototypes.Array });
	$defineProperties(Array.prototype, { dontEnum: true }, {
		constructor: Array,
		concat: $unconstructable(function concat(item1) {
			var a = [ ], argv, argc = (argv = arguments).length, n = 0, v = this;
			for (var i = -1; i < argc; v = argv[++i]) {
				if ($support.getInternalProperty(v, 'class') !== 'Array') {
					a[n++] = v;
				} else {
					for (var j = 0, e = v.length; j < e; ++j) if (j in v) a[n + j] = v[j];
					a.length = (n += j);
				}
			}
			return a;
		}),
		join: $unconstructable(function join(separator) {
			var s, s2, l = this.length;
			separator = (separator === void 0 ? ',' : '' + separator);
			if ((s = this[0]) == void 0) s = '';
			for (var i = 1; i < l; ++i) {
				s += separator;
				if ((s2 = this[i]) != null) s += s2;
			}
			return s;
		}),
		pop: $unconstructable(function pop() {
			var v = void 0, l;
			if ((l = this.length) > 0) {
				v = this[l - 1];
				--this.length;
			}
			return v;
		}),
		push: $unconstructable(function push(item) {
			var argv, offset = this.length, end = (argv = arguments).length + offset;
			for (var i = offset; i < end; ++i) this[i] = argv[i - offset];
			return end;
		}),
		reverse: $unconstructable(function reverse() {
			var len, mid = $floor((len = this.length) / 2);
			--len;
			for (var leftIndex = 0; leftIndex < mid; ++leftIndex) {
				var rightIndex = len - leftIndex;
				var right = this[rightIndex];
				if (leftIndex in this) this[rightIndex] = this[leftIndex]; else delete this[rightIndex];
				if (rightIndex in this) this[leftIndex] = right; else delete this[leftIndex];
			}
			return this;
		}),
		shift: $unconstructable(function shift() {
			var len, elementZero;
			if (len = this.length) {
				elementZero = this[0];
				for (var i = 1; i < len; ++i) {
					if (i in this) this[i - 1] = this[i]; else delete this[i - 1];
				}
				--this.length;
			}
			return elementZero;
		}),
		toString: $unconstructable(function toString() {
			$checkClass(this, 'Array', 'toString');
			return this.join();
		})
	});

	/* --- Set up globals --- */

	$defineProperties($global, { dontEnum: true }, {
		Array: Array,
		Boolean: Boolean,
		Function: Function,
		Math: $support.createWrapper('Math', void 0),
		Number: Number,
		Object: Object,
		String: String,
		isFinite: $unconstructable(function isFinite(v) { return $isFinite(+v) }),
		isNaN: $unconstructable(function isNaN(v) { return $isNaN(+v) }),
		parseFloat: $unconstructable(function parseFloat(string) { return $support.parseFloat('' + string); }),
		parseInt: $unconstructable(function parseInt(string, radix) {
			string = '' + string;
			var pic = $PARSE_INT_CHARS, ch, i = -1;
			while (pic[string[++i]] === null) ;
			var sign = 1;
			switch (ch = string[i]) {
				case '-': sign = -1;
				case '+': ++i;
			}
			if (((radix = $toInteger(radix)) === 0 || radix === 16) && (string[i] === '0' && string[i + 1] === 'x')) {
				i += 2;
				radix = 16;
			}
			if (radix === 0) {
				radix = 10;
			} else if (radix < 2 || radix > 36) {
				return $NaN;
			}
			var v = 0, b, e = string.length, n;
			for (b = i; i < e && (n = pic[string[i]]) != null && n < radix; ++i) {
				v = v * radix + n;
			}
			return (b === i ? $NaN : v * sign);
		})
	});

	$defineProperties($global, { dontEnum: true, dontDelete: true }, {
		NaN: $NaN, Infinity: $Infinity, undefined: $support.undefined
	});

	/* --- Math --- */

	// FIX : more decimals needed? these are from ECMA-262
	$defineProperties(Math, { readOnly: true, dontEnum: true, dontDelete: true }, {
		E: 2.7182818284590452354,
		LN10: 2.302585092994046,
		LN2: 0.6931471805599453,
		LOG2E: 1.4426950408889634,
		LOG10E: 0.4342944819032518,
		PI: 3.1415926535897932,
		SQRT1_2: 0.7071067811865476,
		SQRT2: 1.4142135623730951
	});

	$defineProperties(Math, { dontEnum: true }, { // FIX : construction from array instead?
		abs: $unconstructable(function abs(v) { return ((v = +v) < 0 ? -v : v) }),
		acos: $unconstructable(function acos(v) { return $support.acos(+v); }),
		asin: $unconstructable(function asin(v) { return $support.asin(+v); }),
		atan: $unconstructable(function atan(v) { return $support.atan(+v); }),
		atan2: $unconstructable(function atan2(y, x) { return $support.atan2(+y, +x); }),
		ceil: $unconstructable(function ceil(v) { return -$floor(-v); }),
		cos: $unconstructable(function cos(v) { return $support.cos(+v); }),
		exp: $unconstructable(function exp(v) { return $support.exp(+v); }),
		floor: $unconstructable(function floor(v) { return $floor(+v); }),
		log: $unconstructable(function log(v) { return $support.log(+v); }),
		max: $unconstructable(function max(x, y) { var m = -$Infinity, v; for (var i = (argv = arguments).length - 1; i >= 0; --i) if ((v = +argv[i]) > m || $isNaN(v)) m = v; return m; }),
		min: $unconstructable(function min(x, y) { var m = $Infinity, v; for (var i = (argv = arguments).length - 1; i >= 0; --i) if ((v = +argv[i]) < m || $isNaN(v)) m = v; return m; }),
		pow: $unconstructable(function pow(x, y) { return $support.pow(+x, +y); }),
		round: $unconstructable(function round(v) { return $floor(v + 0.5); }), // FIX : -0 rule in ES262
		sin: $unconstructable(function sin(v) { return $support.sin(+v); }),
		sign: $unconstructable(function sign(v) { return ((v = +v) < 0 ? -1 : (v > 0 ? 1 : 0)) }), // FIX : not ES3 standard??
		sqrt: $unconstructable(function sqrt(v) { return $support.sqrt(+v); }),
		tan: $unconstructable(function tan(v) { return $support.tan(+v); })
		// trunc etc from ES6?
	});

	/* --- Errors --- */

	function createErrorConstructor(name, prototype) {
		return function(message) {
			var e;
			$support.defineProperty(e = $support.createWrapper('Error', name, prototype), 'message'
					, (message !== void 0 ? '' + message : ''), false, true, false);
			return e
		}
	};

	(function() {
		var ERROR_NAMES = [ 'Error', 'RangeError', 'ReferenceError', 'SyntaxError', 'TypeError' ];

		for (var i = ERROR_NAMES.length - 1; i >= 0; --i) {
			var name, constructor, prototype;
			$support.defineProperty($global, name = ERROR_NAMES[i]
					, constructor = createErrorConstructor(name, prototype = $support.prototypes[name]), false, true, false);
	    /* FIX : not necessary for strict es3, where .name isn't included at all in the official specs but is my own extension using standard flags
			delete constructor.name; // ugly trick to set the read-only, but not undeletable .name on the anonymous function
	    */
			constructor.name = name;
			$defineProperties(constructor, { dontEnum: true, readOnly: true, dontDelete: true }, { prototype: prototype });
			$defineProperties(prototype, { dontEnum: true }, { constructor: constructor });
			prototype.name = name;
		}

		$defineProperties(Error.prototype, { dontEnum: true }, {
			message: "",
			toString: $unconstructable(function toString() {
				return (this.name === void 0 ? 'Error' : this.name) + (this.message ? (': ' + this.message) : '');
			})
		});
	})();

	/* --- ES >3 polyfills --- */

	// These are not guaranteed to be 100% compatible

	$defineProperties(Array, { dontEnum: true }, {
		isArray: $unconstructable(function isArray(o) {
			return $support.getInternalProperty(o, 'class') === 'Array';
		}),
		getPrototypeOf: $unconstructable(function getPrototypeOf(o) {
			return $support.getInternalProperty(o, 'prototype');
		})
	});

	$defineProperties(Object, { dontEnum: true }, {
		defineProperty: $unconstructable(function defineProperty(o, p, d) {
			$support.defineProperty(o, '' + p, d.value, !d.writable, !d.enumerable, !d.configurable);
		}),
		getPrototypeOf: $unconstructable(function getPrototypeOf(o) {
			return $support.getInternalProperty(o, 'prototype');
		})
	});
})
