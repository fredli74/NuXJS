var support = { };

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

/*
function extend(object, definitions) {
	for (var property in definitions) {
		object[property] = definitions[property];
	}
    return target;
}*/


Object = support.createClass(
	'Object',
	function(value) {
		return (value == null ? { } : toObject(value));
	},
	{
		toString: function() {
			return "[object " + support.getObjectClass(this) + "]";
		}
	}
);

Array = support.createClass(
	'Array',
	function(len) {
		var args, argsLen, newArray = [ ];
		if ((argsLen = (args = arguments).length) === 1 && typeof len === "number") {
			newArray.length = len;
		} else {
			for (var i = argsLen; --i >= 0;) {
				newArray[i] = args[i];
			}
		}
		return newArray;
	}
);

Number = support.createClass(
	'Number',
	function(v) {
		v = (arguments.length ? +v : 0);
		if (this) {
			v;
		}
		return v;
	}
}

Number = support.createConstructor(function(v) {
	v = (arguments.length ? +v : 0);
	if (support.calledAsConstructor()) {
		return support.createWrapper('Number', v, support.prototypes.Number);
	} else {
		return v;
	}
}
Number.prototype = support.prototypes.Number;
support.addHiddenProperties(Number.prototype, {
	constructor: Number,
//	valueOf: function() { 

support.toPrimitiveValue = function(o) {
	var v;
	function convertWith(m) {
		var t;
		return (typeof (o[m]) === "function" && ((v = o[m]()) === null || ((t = typeof(v)) !== 'object' && t !== 'function')));
	}
	if (!convertWith("valueOf") && !convertWith("toString")) {
		throw("error");
	}
	return v;
};

Math = support.createWrapper('Math', '[object Math]', support.prototypes.Object);

support.addHiddenProperties(Math, {
	abs: function(v) { return ((v = +v) < 0 ? -v : v) },
	floor: function(v) { return support.natives.floor(+v) },
	sqrt: function(v) { return support.natives.sqrt(+v) }
});
