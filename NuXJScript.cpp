// FIX: all below

#include "assert.h"
#include <cmath>
#include <ctime>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <limits>
#include <map>
#include <vector>
#include "NuXJScript.h"

	bool supressStdErr = false; // FIX : drop

// MSVC (at least 2010) doesn't do fmod correctly with infinite divisor
static double NaN() { return std::numeric_limits<double>::quiet_NaN(); }
#ifdef _MSC_VER
#include <float.h>
static bool isNaN(double d) { return (_isnan(d) != 0); }
static bool isFinite(double d) { return (_finite(d) != 0); }

/*
	Ecma rules
			-inf	-y		0		+y		+inf	NaN
			-------------------------------------------
	-inf   |NaN		NaN		NaN		NaN		NaN		NaN
	-x	   |-x		-x%y	NaN		-x%y	-x		NaN
	0	   |0		0		NaN		0		0		NaN
	+x	   |+x		x%y		NaN		x%y		+x		NaN
	+inf   |NaN		NaN		NaN		NaN		NaN		NaN
	NaN	   |NaN		NaN		NaN		NaN		NaN		NaN

	MSVC fmod
			-inf	-y		0		+y		+inf	NaN
			-------------------------------------------
	-inf   |NaN		NaN		NaN		NaN		NaN		NaN		!_finite
	-x	   |NaN*	-x%y	NaN		-x%y	NaN*	NaN		_finite
	0	   |NaN*	0		NaN		0		NaN*	NaN		_finite
	+x	   |NaN*	x%y		NaN		x%y		NaN*	NaN		_finite
	+inf   |NaN		NaN		NaN		NaN		NaN		NaN		!_finite
	NaN	   |NaN		NaN		NaN		NaN		NaN		NaN		!_finite
			!_f		_f		_f		_f		!_f		!_f
*/
static double modulo(double x, double y) { return (!_finite(y) && !_isnan(y) && _finite(x) ? x : fmod(x, y)); }
#else
static double modulo(double x, double y) { return fmod(x, y); }
// FIX : if c++11 use std::isnan and std::isfinite
static bool isNaN(double d) { return d != d; }
static bool isFinite(double d) { return !isNaN(d) && fabs(d) != std::numeric_limits<double>::infinity(); };
#endif

static double nanToZero(double d) { return (isNaN(d) ? 0.0 : d); }

/* --- String constants --- */

// FIX : sort, check all used, join to few lines, or maybe even do some smart array based construction?
const String EMPTY_STRING("");
const String ANONYMOUS_STRING("anonymous");
const String UNDEFINED_STRING("undefined");
const String OBJECT_STRING("object");
const String O_BJECT_STRING("Object");
const String A_RRAY_STRING("Array");
const String BOOLEAN_STRING("boolean");
const String B_OOLEAN_STRING("Boolean");
const String BRACKET_OBJECT_STRING("[object ");
const String END_BRACKET_STRING("]");
const String NUMBER_STRING("number");
const String N_UMBER_STRING("Number");
const String STRING_STRING("string");
const String S_TRING_STRING("String");
const String FUNCTION_STRING("function");
const String F_UNCTION_STRING("Function");
const String NULL_STRING("null");
const String FALSE_STRING("false");
const String TRUE_STRING("true");
const String NAN_STRING("NaN");
const String INFINITY_STRING("Infinity");
const String MINUS_INFINITY_STRING("-Infinity");
const String LENGTH_STRING("length");
const String ARGUMENTS_STRING("arguments");
const String A_RGUMENTS_STRING("Arguments");
const String CALLEE_STRING("callee");
const String PROTOTYPE_STRING("prototype");
const String CLASS_STRING("class");
const String VALUE_STRING("value");
const String CONSTRUCTOR_STRING("constructor");
const String EVAL_STRING("eval");
const String NATIVE_FUNCTION_STRING("function() { [native code] }");
const String E_RROR_STRING("Error");
const String MESSAGE_STRING("message");
const String NAME_STRING("name");
const String FUNCTION_SPACE("function ");
const String COLON_SPACE(": ");
const String CANNOT_CONVERT_TO_OBJECT_STRING("Cannot convert undefined or null to object");
const String IS_NOT_A_FUNCTION_STRING(" is not a function");
const String IS_NOT_DEFINED_STRING(" is not defined");
const String CANNOT_USE_IN_OPERATOR_STRING("Cannot use 'in' operator on ");
const String ERROR_NAMES[ERROR_TYPE_COUNT] = {
	String("Error"), String("EvalError"), String("RangeError"), String("ReferenceError"), String("SyntaxError")
	, String("TypeError"), String("URIError")
};

/* --- Utilities --- */

// Returns pointer to beginning of string
// End of string is always buffer + 32 (*not* zero-terminated by this routine)
static Char* intToString(Char buffer[32], Int32 i) {
	Char* p = buffer + 32;
	*--p = "9876543210123456789"[9 + i % 10];																			// Mirrored string to handle negative x.
	Int32 x = i;
	while ((x /= 10) != 0) {																							// Mirrored string to handle negative x.
		*--p = "9876543210123456789"[9 + x % 10];
	}
	if (i < 0) {
		*--p = '-';
	}
	assert(p >= buffer);
	return p;
}

const int MAX_EXPONENT = 308;
const int MIN_EXPONENT = -323;

const int QUICK_CONSTANTS_INTEGERS_RANGE = 1000;
struct QuickConstants {
	QuickConstants() {
		for (int i = 0; i < 127; ++i) {
			Char buf[1] = { static_cast<Char>(i) };
			asciiChars[i] = String(buf, buf + 1);
			ascii[i] = Value(&asciiChars[i]);
		}
		for (int i = -QUICK_CONSTANTS_INTEGERS_RANGE; i <= QUICK_CONSTANTS_INTEGERS_RANGE; ++i) {
			Char buffer[32];
			integers[i + QUICK_CONSTANTS_INTEGERS_RANGE] = String(intToString(buffer, i), buffer + 32); // FIX : newHashedString
			integers[i + QUICK_CONSTANTS_INTEGERS_RANGE].calcHash();
		}
		for (int i = MIN_EXPONENT; i <= MAX_EXPONENT; ++i) {
			exp10s[i - MIN_EXPONENT] = pow(10.0, static_cast<double>(i));
		}
	}
	Value ascii[127];
	String asciiChars[127];
	String integers[QUICK_CONSTANTS_INTEGERS_RANGE * 2 + 1];
	double exp10s[MAX_EXPONENT - MIN_EXPONENT + 1];
} QUICK_CONSTANTS;

// FIX : shouldn't need this, used in quick-hash-finders and they should be optimized somehow i think
static int strncmp(const Char* a, const char* b, UInt32 n) {
	UInt32 i = 0;
	while (i < n && a[i] == b[i] && a[i] != 0 && b[i] != 0) {
		++i;
	}
	return (i == n ? 0 : a[i] - b[i]);
}
/* Built with QuickHashMaker.pika */
static int findStatementToken(int n /* string length */, const Char* s /* string (zero termination not required) */) {
	static const char* STRINGS[13] = {
		"var", "if", "while", "do", "for", "return", "continue", "break", "try", 
		"throw", "switch", "function", "with"
	};
	static const int HASH_TABLE[64] = {
		-1, -1, -1, -1, 10, -1, -1, -1, -1, 4, -1, 3, 6, -1, -1, 11, 
		7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, 
		-1, 2, 12, 5, 9, -1, -1, -1, -1, -1, -1, -1, -1, 1, 8, -1
	};
	if (n < 2 || n > 8) return -1;
	int stringIndex = HASH_TABLE[(s[1] - s[0]) & 63];
	return (stringIndex >= 0 && strncmp(s, STRINGS[stringIndex], n) == 0 && STRINGS[stringIndex][n] == 0) ? stringIndex : -1;
}
/* Built with QuickHashMaker.pika */
static int findReservedKeyword(int n /* string length */, const Char* s) {
	static const char* STRINGS[36] = {
		"break", "case", "catch", "continue", "debugger", "default", "delete", "do", 
		"else", "finally", "for", "function", "if", "in", "instanceof", "new", 
		"return", "switch", "this", "throw", "try", "typeof", "var", "void", "while", 
		"with", "class", "const", "enum", "export", "extends", "import", "super", 
		"null", "false", "true"
	};
	static const int QUICK_HASH_TABLE[256] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
		-1, -1, -1, -1, 33, -1, -1, -1, -1, -1, -1, 2, 1, -1, -1, -1, 
		-1, -1, -1, -1, 23, -1, -1, -1, -1, -1, -1, -1, 35, 20, -1, -1, 
		-1, -1, -1, 34, -1, -1, -1, -1, -1, -1, -1, 32, -1, -1, -1, -1, 
		4, 5, 6, -1, -1, -1, -1, -1, -1, -1, 17, -1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
		-1, -1, 21, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
		-1, 9, -1, 26, -1, -1, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
		-1, -1, -1, -1, 8, -1, -1, -1, 3, -1, -1, 27, -1, -1, -1, -1, 
		-1, -1, -1, -1, 28, 15, 7, -1, -1, -1, -1, 0, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, 10, -1, -1, -1, -1, 31, -1, -1, -1, 14, -1, 
		-1, -1, 16, -1, -1, 22, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
		11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 19, 18, -1, -1, -1, 
		-1, 30, 29, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
		-1, -1, -1, 24, -1, -1, -1, -1, -1, -1, -1, -1, 25, -1, -1, -1
	};
	if (n < 2 || n > 10) return -1;
	int stringIndex = QUICK_HASH_TABLE[(((s[1] + s[0]) << 3) - n) & 255];
	return (stringIndex >= 0 && strncmp(s, STRINGS[stringIndex], n) == 0 && STRINGS[stringIndex][n] == 0) ? stringIndex : -1;
};
/* Built with QuickHashMaker.pika */
static int findLiteralToken(int n /* string length */, const Char* s /* zero-terminated string */) {
	static const char* STRINGS[5] = {
		"null", "false", "true", "function", "this"
	};
	static const int QUICK_HASH_TABLE[8] = {
		0, 2, -1, 3, -1, -1, 1, 4
	};
	if (n < 4 || n > 8) return -1;
	int stringIndex = QUICK_HASH_TABLE[((n ^ s[3])) & 7];
	return (stringIndex >= 0 && strncmp(s, STRINGS[stringIndex], n) == 0 && STRINGS[stringIndex][n] == 0) ? stringIndex : -1;
};

// FIX : should first argument be a changable reference or not? we use that convention a lot in the compiler, but not here

static const Char* parseHex(const Char* p, const Char* e, UInt32& i) {
	for (i = 0; p != e && ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F') || (*p >= 'a' && *p <= 'f')); ++p) {
		i = (i << 4) + (*p <= '9' ? *p - '0' : (*p & ~0x20) - ('A' - 10));
	}
	return p;
}

static const Char* parseUnsignedInt(const Char* p, const Char* e, UInt32& i) {
	for (i = 0; p != e && *p >= '0' && *p <= '9'; ++p) {
		i = i * 10 + (*p - '0');
	}
	return p;
}

const Char* parseDouble(const Char* const b, const Char* const e, double& value) {
	Int32 exponent = -1;
	double sign = 1.0;
	const Char* significandBegin = b;
	const Char* numberEnd;

	const Char* p = b;
	if (p != e && (*p == '-' || *p == '+')) {
		sign = (*p == '-' ? -1.0 : 1.0);
		++p;
		significandBegin = p;
	}
	if (e - p >= 8 && strncmp(p, "Infinity", 8) == 0) {
		p += 8;
		value = std::numeric_limits<double>::infinity();
		numberEnd = p;
	} else {
		while (p != e && *p >= '0' && *p <= '9') {
			++exponent;
			++p;
		}
		if (p != e && *p == '.') {
			if (p == significandBegin) {
				++significandBegin;
			}
			++p;
			while (p != e && *p >= '0' && *p <= '9') {
				++p;
			}
		}

		if (p == significandBegin) {
			value = 0.0;
			return b;
		}

		const Char* significandEnd = p;
		numberEnd = p;

		if (e - p >= 2 && (*p == 'e' || *p == 'E')) {
			++p;
			Int32 sign = (*p == '-' ? -1 : 1);
			if (*p == '+' || *p == '-') {
				++p;
			}
			UInt32 ui;
			const Char* q = parseUnsignedInt(p, e, ui);
			if (q != p) {
				exponent += sign * static_cast<Int32>(ui);
				numberEnd = q;
			}
		}
		
		p = significandBegin;
		while (p != significandEnd && (*p == '0' || *p == '.')) {
			if (*p == '0') {
				--exponent;
			}
			++p;
		}
		
		if (p == significandEnd || exponent < MIN_EXPONENT - 1) {
			value = 0.0;
		} else if (exponent > MAX_EXPONENT) {
			value = std::numeric_limits<double>::infinity();
		} else {
			double scale = (exponent < MIN_EXPONENT + 20 ? 0.1 : 1.0);	// An extra scaling step is necessary to reach the lowest denormal (approx 0.5e-324) since 1e-325 is out of range.
			double magnitude = QUICK_CONSTANTS.exp10s[exponent - MIN_EXPONENT + (exponent < MIN_EXPONENT + 20 ? 1 : 0)];
			volatile double compensation = 0.0;
			
			double d = 0.0;
			while (p != significandEnd) {
				if (*p != '.') {
					const Int32 digit = (*p - '0');
					const double x = digit * magnitude * scale;
					const double adjusted = x - compensation;	// Kahan summation for greater accuracy
					volatile double sum = d + adjusted;
					compensation = sum - d;
					compensation -= adjusted;
					d = sum;
					magnitude /= 10.0;
				}
				++p;
			}
			value = d;
		}
	}
	value *= sign;
	return numberEnd;
}

Char* doubleToString(Char buffer[32], const double value) {
	Char* p = buffer;
	if (value == 0.0) {
		*p++ = '0';
		return p;
	}
	if (value < 0) {
		*p++ = '-';
	}

	const double absValue = (value < 0 ? -value : value);
	int base2Exponent;
	frexp(absValue, &base2Exponent);	// frexp is fast and precise and gives log2(x), log10(x) = log2(x) / log2(10)
	Int32 exponent = std::max(static_cast<Int32>(ceil(0.30102999566398119521 * (base2Exponent - 1))) - 1, MIN_EXPONENT - 1);
	if (exponent < MAX_EXPONENT && absValue >= QUICK_CONSTANTS.exp10s[exponent + 1 - MIN_EXPONENT]) {
		++exponent;
	}
	const bool eNotation = (exponent < -6 || exponent >= 21);
	Char* periodPosition = p + (eNotation || exponent < 0 ? 0 : exponent) + 1;
	if (!eNotation && exponent < 0) {
		*p++ = '0';
		*p++ = '.';
		while (p < periodPosition - exponent) {
			*p++ = '0';
		}
	}
	
	const double scale = (exponent < MIN_EXPONENT + 20 ? 0.1 : 1.0);	// An extra scaling step is necessary to reach the lowest denormal (approx 0.5e-324) since 1e-325 is out of range.
	double magnitude = QUICK_CONSTANTS.exp10s[exponent - MIN_EXPONENT + (exponent < MIN_EXPONENT + 20 ? 1 : 0)];
	double residual = absValue;
	double reconstructed = 0.0;
	volatile double compensation = 0.0;
	while (p < buffer + 27 && magnitude != 0.0 && residual != 0.0) {	// p < buffer + 27 is an extra precaution if toNumber(toString(x)) == x is never met (e.g. because of too aggressive optimizations). 27 leaves room for longest exponent.
		if (p == periodPosition) {
			*p++ = '.';
		}
		const Int32 digit = static_cast<Int32>(residual / magnitude / scale);
		*p++ = '0' + digit;
		double x = digit * magnitude * scale;
		if (residual - x < 0 || digit == 10) { 	// When underflowing the residual (because of rounding problems) we drop to a lower digit. Also with some extreme values (e.g. the lowest denormals), rounding can cause residual / magnitude to become 1.0 and we must turn the 10 into a 9.
			p[-1]--;
			x = (digit - 1) * magnitude * scale;
		}
		residual -= x;
		assert(residual >= 0);

		const double adjusted = x - compensation;	// Kahan summation for greater accuracy
		volatile double sum = reconstructed + adjusted;

		if (sum == absValue) {
			break;
		}
		if (digit < 9) {	// Catch shorter rounded solutions by trying one digit higher too (except if last digit was 9 in which case we should have caught the rounded solution in the previous step anyhow).
			const double altAdjusted = (digit + 1) * magnitude * scale - compensation;
			volatile double altSum = reconstructed + altAdjusted;
			if (altSum == absValue) {
				p[-1]++;
				break;
			}
		}

		compensation = sum - reconstructed;
		compensation -= adjusted;
		reconstructed = sum;
		magnitude /= 10.0;
	}
	while (p < periodPosition) {
		*p++ = '0';
	}
	if (eNotation) {
		*p++ = 'e';
		*p++ = (exponent < 0 ? '-' : '+');
		p = std::copy(intToString(buffer, exponent < 0 ? -exponent : exponent), buffer + 32, p);	// intToString fills the buffer from right (buffer + 32) to left and there should always be enough space
	}
	return p;
}

static const Char* eatStringWhite(const Char* p, const Char* e) {
	while (p < e) {
		switch (*p) {
			case ' ': case '\f': case '\n': case '\r': case '\t': case '\v': case 0xA0: case 0x2028: case 0x2029: break;
			default: return p;
		}
		++p;
	}
	return p;
}

static double stringToDouble(const String& s) {
	double v = 0.0;
	const Char* p = s.begin();
	const Char* e = s.end();
	p = eatStringWhite(p, e);
	if (p != e && *p == '0' && p + 1 != e && (p[1] == 'x' || p[1] == 'X')) {
		UInt32 u;
		const Char* b = p + 2;
		p = parseHex(b, e, u);
		if (p == b) {
			return NaN();
		}
		v = u;
	} else {
		p = parseDouble(p, e, v);
	}
	p = eatStringWhite(p, e);
	return (p == e ? v : NaN());
}

/* --- Value --- */

const String* Value::TYPE_STRINGS[5] = { &UNDEFINED_STRING, &OBJECT_STRING, &BOOLEAN_STRING, &NUMBER_STRING, &STRING_STRING };

Function* Value::asFunction() const { return (type == OBJECT_TYPE ? var.object->asFunction() : 0); }

const String* Value::typeOfString() const {
	return (type == OBJECT_TYPE ? var.object->typeOfString() : TYPE_STRINGS[static_cast<int>(type)]);
}

bool Value::toBool() const {
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE:
		case NULL_TYPE: return false;
		case BOOLEAN_TYPE: return var.boolean;
		case NUMBER_TYPE: return nanToZero(var.number) != 0.0;
		case STRING_TYPE: return (!var.string->empty());
		case OBJECT_TYPE: return true;
	}
}

double Value::toDouble() const {
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE:
		case OBJECT_TYPE: return NaN();  // Notice, you shouldn't normally call toDouble on an object as true ToNumber requires conversion to a primitive type
		case NULL_TYPE: return 0.0;
		case BOOLEAN_TYPE: return var.boolean ? 1.0 : 0.0;
		case NUMBER_TYPE: return var.number;
		case STRING_TYPE: return stringToDouble(*var.string);
	}
}

// TODO : toInt32 and toUInt32
Int32 Value::toInt() const { // FIX : could be faster without toDouble I thinkus
	double v = toDouble();
	int i = static_cast<Int32>(v);
	if (i == v) { // FIX : <2^32 but with decimals will now go via slow fmod, right? unneccessary
		return i;
	} else if (!isFinite(v)) {
		return 0;
	} else {
		return (v >= 0) ? static_cast<Int32>(static_cast<UInt32>(fmod(v, 4294967296.0)))
				: -static_cast<Int32>(static_cast<UInt32>(fmod(-v, 4294967296.0)));
	}
}

bool Value::toArrayIndex(UInt32& index) const {
	switch (type) {
		case NUMBER_TYPE: {
			index = static_cast<UInt32>(var.number);
			return index == var.number;
		}
		case STRING_TYPE: {
			const Char* const e = var.string->end();
			const Char* p = var.string->begin();
			if (*p == '0') {
				index = 0;
				++p;
			} else {
				p = parseUnsignedInt(p, e, index);
			}
			return p == e;
		}
		default: return false;
	}
}

const String* Value::toString(GCHeap& heap) const {
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return &UNDEFINED_STRING;
		case NULL_TYPE: return &NULL_STRING;
		case BOOLEAN_TYPE: return var.boolean ? &TRUE_STRING : &FALSE_STRING;
		case NUMBER_TYPE: return String::fromDouble(heap, var.number);
		case STRING_TYPE: return var.string;
		case OBJECT_TYPE: return var.object->toString(heap);
	}
}

Object* Value::toObject(Engine& engine, bool requestExtensible) const {
	GCHeap& heap = engine.getHeap();
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return 0;
		case NULL_TYPE: return 0;
		case BOOLEAN_TYPE: return heap.manage(new (&heap) WrapperObject(&heap, &B_OOLEAN_STRING, *this, &engine.booleanPrototype));
		case NUMBER_TYPE: return heap.manage(new (&heap) WrapperObject(&heap, &N_UMBER_STRING, *this, &engine.numberPrototype));
		// I think const_cast can be accepted here, the string is always immutable when accessed as an Object
		case STRING_TYPE: return (!requestExtensible ? static_cast<Object*>(const_cast<String*>(var.string))
				: heap.manage(new (&heap) StringWrapper(&heap, var.string, &engine.stringPrototype)));
		case OBJECT_TYPE: return var.object;
	}
}

bool Value::operator==(const Value& r) const {
	if (type != r.type) return false;
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE:
		case NULL_TYPE: return true;
		case BOOLEAN_TYPE: return var.boolean == r.var.boolean;
		case NUMBER_TYPE: return var.number == r.var.number;
		case STRING_TYPE: return var.string->isEqualTo(*r.var.string);
		case OBJECT_TYPE: return var.object == r.var.object;
	}
}

bool Value::operator<(const Value& r) const {
	if (type < r.type) return true;
	if (type > r.type) return false;
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE:
		case NULL_TYPE: return false;
		case BOOLEAN_TYPE: return var.boolean < r.var.boolean;
		case NUMBER_TYPE: return var.number < r.var.number;
		case STRING_TYPE: return var.string->isLessThan(*r.var.string);
		case OBJECT_TYPE: return var.object < r.var.object;
	}
}

bool Value::compareStrictly(const Value& r) const {
	assert(r.type == type);
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return true;
		case NULL_TYPE: return true;
		case BOOLEAN_TYPE: return var.boolean == r.var.boolean;
		case NUMBER_TYPE: return var.number == r.var.number;
		case STRING_TYPE: return var.string->isEqualTo(*r.var.string);
		case OBJECT_TYPE: return var.object == r.var.object;
	}
}

bool Value::isStrictlyEqualTo(const Value& r) const { return (type == r.type ? compareStrictly(r) : false); }

/*
				undefined	null	boolean				number			string				object
	undefined	TRUE		TRUE	FALSE				FALSE			FALSE				FALSE
	null		TRUE		TRUE	FALSE				FALSE			FALSE				FALSE
	boolean		FALSE		FALSE	l === r				num(l) === r	num(l) === num(r)	num(l) === num(r)
	number		FALSE		FALSE	l === num(r)		l === r			l === num(r)		l == prim(r)
	string		FALSE		FALSE	num(l) === num(r)	num(l) === r	l === r				l == prim(r)
	object		FALSE		FALSE	num(l) === num(r)	prim(l) == r	prim(l) == r		l === r
*/
bool Value::isEqualTo(const Value& r) const {
	if (type == r.type) {
		return compareStrictly(r);
	} else if (type < r.type) {	// Symmetrical operation, flip for simpler logic.
		return r.isEqualTo(*this);
	} else {
		switch (type) {
			case NULL_TYPE: return true;
			case BOOLEAN_TYPE: return false;
			case NUMBER_TYPE: return (r.type == BOOLEAN_TYPE && var.number == (r.var.boolean ? 1.0 : 0.0));
			case STRING_TYPE: return (r.type >= BOOLEAN_TYPE && stringToDouble(*var.string) == (r.type == BOOLEAN_TYPE ? (r.var.boolean ? 1.0 : 0.0) : r.var.number));
			default: return false;
		}
	}
}

bool Value::isEqualTo(const String& s) const {
	return (type == STRING_TYPE && s.isEqualTo(*var.string));
}

bool Value::isLessThan(const Value& y) const {
	if (type == STRING_TYPE && y.type == STRING_TYPE) {
		return var.string->isLessThan(*y.var.string);
	}
	double thisNumber = toDouble();
	double otherNumber = y.toDouble();
	return thisNumber < otherNumber;
}

bool Value::isLessThanOrEqualTo(const Value& y) const {
	if (type == STRING_TYPE && y.type == STRING_TYPE) {
		return !y.var.string->isLessThan(*var.string);
	}
	double thisNumber = toDouble();
	double otherNumber = y.toDouble();
	return thisNumber <= otherNumber;
}

Value Value::add(GCHeap& heap, const Value& y) const {
	if (type == STRING_TYPE || y.type == STRING_TYPE) {
		const String* ls = toString(heap);
		const String* rs = y.toString(heap);
		if (rs->empty()) return Value(ls);
		if (ls->empty()) return Value(rs);
		return Value(heap.manage(new (&heap) String(*ls, *rs, &heap)));
	}
	return Value(toDouble() + y.toDouble());
}

void Value::gcMark(GCHeap& heap) const {
	switch (type) {
		case STRING_TYPE: heap.mark(var.string); break;
		case OBJECT_TYPE: heap.mark(var.object); break;
		default: break;
	}
}

const Value Value::UNDEFINED(Value::UNDEFINED_TYPE);
const Value Value::NUL(Value::NULL_TYPE);
const Value Value::NOT_A_NUMBER(NaN());
const Value Value::INFINITE_NUMBER(std::numeric_limits<double>::infinity());

/* --- String --- */

String::String() : QArray(0), hash(2166136261U) { assert(isEqualTo(EMPTY_STRING)); }
String::String(const Char* b, const Char* e, Heap* heap) : QArray(e - b, heap), hash(0) { std::copy(b, e, begin()); }
// FIX : needed ? String::String(const String& o) : QArray(o), hash(o.hash) { }
String::String(const std::wstring& s, Heap* heap) : QArray(s.size(), heap), hash(0) { std::copy(s.begin(), s.end(), begin()); }

String::String(const String& l, const String& r, Heap* heap) : QArray(l.size() + r.size(), heap), hash(0) {
	Char* e = std::copy(l.begin(), l.end(), begin());
	std::copy(r.begin(), r.end(), e);
}

const String* String::typeOfString() const { return &STRING_STRING; }
const String* String::getClassName() const { return &S_TRING_STRING; }
const String* String::toString(GCHeap& heap) const { return this; }
Object* String::getPrototype(Engine& engine) const { return &engine.stringPrototype; }

Flags String::getOwnProperty(Engine& engine, const Value& key, Value* v) const {
	if (key.isEqualTo(LENGTH_STRING)) {
		*v = Value(static_cast<Int32>(size()));
		return (READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG | EXISTS_FLAG);
	}
	UInt32 index;
	if (key.toArrayIndex(index) && index < size()) {
		const Char* p = begin() + index;
		GCHeap& heap = engine.getHeap();
		*v = (*p < 127 ? QUICK_CONSTANTS.ascii[*p] : Value(heap.manage(new (&heap) String(p, p + 1, &heap))));
		return (READ_ONLY_FLAG | DONT_DELETE_FLAG | EXISTS_FLAG);
	}
	return NONEXISTENT;
}

// FIX : speed up by skipping when string is really long
// FNV Hash
UInt32 String::calcHash() const {
	if (hash == 0) {
		UInt32 x = 2166136261U;
		for (const Char* p = begin(), *e = end(); p < e; ++p) {
			x *= 16777619;
			x += *p; // FIX : FNV uses xor, not add, why did I do this? Also FNV is really made for byte processing. Also, http://www.isthe.com/chongo/tech/comp/fnv/ recommends xor'ing before multiplying. Better?
		}
		hash = (x == 0 ? x + 1 : x);
	}
	return hash;
}

bool String::isEqualTo(const String& o) const {
	return (this == &o || ( (hash == o.hash || hash == 0 || o.hash == 0)
		&& size() == o.size() && std::equal(begin(), end(), o.begin())));
}

bool String::isLessThan(const String& o) const {
	if (this == &o) {
		return false;
	}
	UInt32 l = std::min(size(), o.size());
	const Char* a = begin();
	const Char* b = o.begin();
	for (UInt32 i = 0; i < l; ++i) {
		if (a[i] != b[i]) {
			return (a[i] < b[i]);
		}
	}
	return size() < o.size();
}

std::wstring String::toStdString() const { return std::wstring(begin(), end()); }

Enumerator* String::getPropertyEnumerator(Engine& engine) const {
	GCHeap& heap = engine.getHeap();
	return heap.manage(new (&heap) RangeEnumerator(heap, 0, size()));
}

// FIX : just a test, solve thread-safe or drop!
static Table _strings;

const String* String::newHashedString(GCHeap& heap, const Char* b, const Char* e) {
	if (b == e) {
		return &EMPTY_STRING;
	}
	String* s = heap.manage(new (&heap) String(b, e, &heap));
	s->calcHash();
	Table::Bucket* bucket = _strings.insert(s);
	if (bucket->valueExists()) {
		return bucket->getValue().getString();
	} else {
		_strings.update(bucket, Value(heap.protect(s)));
		return s;
	}
}

const String* String::fromInt(GCHeap& heap, Int32 i) {
	Char buffer[32];
	return (i >= -QUICK_CONSTANTS_INTEGERS_RANGE && i <= QUICK_CONSTANTS_INTEGERS_RANGE)
		 ? &QUICK_CONSTANTS.integers[i + QUICK_CONSTANTS_INTEGERS_RANGE]
		 : heap.manage(new (&heap) String(intToString(buffer, i), buffer + 32, &heap));
}

const String* String::fromDouble(GCHeap& heap, double d) {
	const String* s;
	Int32 i = static_cast<Int32>(d);
	if (i == d) {
		s = fromInt(heap, i);
	} else if (isNaN(d)) {
		s = &NAN_STRING;
	} else if (d == std::numeric_limits<double>::infinity()) {
		s = &INFINITY_STRING;
	} else if (d == -std::numeric_limits<double>::infinity()) {
		s = &MINUS_INFINITY_STRING;
	} else {
		Char buffer[32];
		s = heap.manage(new (&heap) String(buffer, doubleToString(buffer, d), &heap));
	}
	return s;
}

/* --- GCItem --- */

void* GCItem::operator new(size_t n, Heap* heap) {
	Heap** p = reinterpret_cast<Heap**>(::operator new(n + sizeof (heap), heap));
	*p = heap;
	return p + 1;
}

void GCItem::operator delete(void* ptr) {
	Heap** p = reinterpret_cast<Heap**>(ptr) - 1;
	::operator delete(p, *p);
}

void GCItem::operator delete(void* ptr, Heap* heap) {
	::operator delete(reinterpret_cast<Heap**>(ptr) - 1, heap);
}

GCItem::GCItem() throw() : _gcList(0), _gcPrev(this), _gcNext(this) { }
GCItem::GCItem(const GCItem& copy) throw() : _gcList(0), _gcPrev(0), _gcNext(0) { }
GCItem& GCItem::operator=(const GCItem& copy) throw() { _gcList = 0; _gcPrev = 0; _gcNext = 0; return *this; }
GCHeap* GCItem::gcGetHeap() const { return _gcList == 0 ? (GCHeap*)(0) : &_gcList->getHeap(); }
void GCItem::gcMark(GCHeap& heap) const { }
GCItem::~GCItem() { if (_gcList != 0) _gcList->relinquish(this); }

/* --- GCList --- */

GCList::GCList(GCHeap& heap) : heap(heap), count(0) { }

GCHeap& GCList::getHeap() const { return heap; }

UInt32 GCList::size() const { return count; }

void GCList::relinquish(GCItem* item) throw() {
	assert(item->_gcList == this);
	assert(count > 0);
	count--;
	item->_gcPrev->_gcNext = item->_gcNext;
	item->_gcNext->_gcPrev = item->_gcPrev;
}

void GCList::claim(GCItem* item) throw() {
	if (item->_gcList != 0) {
		item->_gcList->relinquish(item);
	}
	item->_gcPrev = this;
	item->_gcNext = _gcNext;
	_gcNext->_gcPrev = item;
	_gcNext = item;
	item->_gcList = this;
	++count;
}

/* --- Object --- */

static struct EmptyEnumerator : public Enumerator { const String* nextPropertyName() { return 0; } } EMPTY_ENUMERATOR;

Function* Object::asFunction() { return 0; }
const String* Object::typeOfString() const { return &OBJECT_STRING; }
const String* Object::getClassName() const { return &O_BJECT_STRING; }
Object* Object::getPrototype(Engine& engine) const { return &engine.objectPrototype; }
Value Object::getInternalValue(GCHeap& heap) const { return Value::UNDEFINED; }
Flags Object::getOwnProperty(Engine& engine, const Value& key, Value* v) const { return NONEXISTENT; }
bool Object::setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags) { return false; }
bool Object::deleteProperty(Engine& engine, const Value& key) { return false; }
Enumerator* Object::getPropertyEnumerator(Engine& engine) const { return &EMPTY_ENUMERATOR; }
bool Object::isOwnPropertyEnumerable(Engine& engine, const Value& key) const {
	Value dummy;
	return (getOwnProperty(engine, key, &dummy) & (DONT_ENUM_FLAG | EXISTS_FLAG)) == EXISTS_FLAG;
}

const String* Object::toString(GCHeap& heap) const {
	return heap.manage(new (&heap) String(String(BRACKET_OBJECT_STRING, *getClassName(), &heap), END_BRACKET_STRING, &heap));
}

bool Object::hasOwnProperty(Engine& engine, const Value& key) const {
	Value dummy;
	return (getOwnProperty(engine, key, &dummy) != NONEXISTENT);
}

Flags Object::getPropertyDeep(Engine& engine, const Value& key, Value* v) const {
	const Object* o = this;
	do {
		Flags flags = o->getOwnProperty(engine, key, v);
		if (flags != NONEXISTENT) {
			return flags;
		}
		o = o->getPrototype(engine);
	} while (o != 0);
	return NONEXISTENT;
}

bool Object::setPropertyDeep(Engine& engine, const Value& key, const Value& v) {
	Value dummy;
	if (!key.isNumber() && getOwnProperty(engine, key, &dummy) == NONEXISTENT) {
		const Object* o = this;
		while ((o = o->getPrototype(engine)) != 0) {
			if ((o->getOwnProperty(engine, key, &dummy) & READ_ONLY_FLAG) != 0) {
				return false;
			}
		}
	}
	return setOwnProperty(engine, key, v, STANDARD_FLAGS);
}

Value Object::getPropertyDeep(Engine& engine, const char* name) const { // FIX : keep?
	Value v = Value::UNDEFINED;
	String s(name);
	getPropertyDeep(engine, Value(&s), &v);
	return v;
}

/* --- Table --- */

UInt32 Table::calcMaxLoad(UInt32 bucketCount) { return (bucketCount - (bucketCount >> 4) - 1); }

Table::Table(Heap* heap) : n(TABLE_BUILT_IN_N), buckets(1U << TABLE_BUILT_IN_N, heap), loadCount(0) { }

UInt32 Table::getLoadCount() const { return loadCount; }
Table::Bucket* Table::getFirst() const { return getNext(buckets.begin() - 1); }

Table::Bucket* Table::getNext(Bucket* bucket) const {
	assert(bucket != 0);
	Bucket* e = buckets.end();
	while (++bucket != e && !bucket->valueExists()) { }
	return (bucket == e ? 0 : bucket);
}

Table::Bucket* Table::lookup(const String* key) const {
	Bucket* bucket = find(key, key->calcHash());
	return (bucket->valueExists() ? bucket : 0);
}

Table::Bucket* Table::insert(const String* key) {
	UInt32 hash = key->calcHash();
	Bucket* bucket = find(key, hash);
	if (!bucket->keyExists()) {
		if (loadCount >= calcMaxLoad(buckets.size())) {
			rebuild(n + 2);
			assert(loadCount < calcMaxLoad(buckets.size()));
			bucket = find(key, hash);
			assert(!bucket->keyExists());
		}
		++loadCount;
		bucket->flags = 0;
		bucket->hash16 = static_cast<UInt16>(hash);
		bucket->key = key;
	}
	return bucket;
}

bool Table::update(Bucket* bucket, const Value& value, Byte flags) {
	assert(bucket->keyExists());
	if ((bucket->flags & READ_ONLY_FLAG) != 0) {
		return false;
	} else {
		bucket->flags |= EXISTS_FLAG | flags;
		bucket->type = static_cast<Byte>(value.type);
		bucket->var = value.var;
	}
	return true;
}

bool Table::erase(Bucket* bucket) {
	assert(bucket->keyExists());
	if (bucket->valueExists()) {
		if ((bucket->flags & DONT_DELETE_FLAG) != 0) {
			return false;
		}
		bucket->flags = 0;
	}
	return true;
}

void Table::gcMark(GCHeap& heap) const {
	UInt32 rebuildLoadCount = 0;
	for (UInt32 i = 0; i < buckets.size(); ++i) {
		const Bucket& bucket = buckets[i];
		if (bucket.keyExists()) {
			heap.mark(bucket.key); // FIX : Really, doesn't it get cleaned out by rebuild anyhow?
			if (bucket.valueExists()) {
				switch (bucket.type) {
					case Value::STRING_TYPE: heap.mark(bucket.var.string); break;
					case Value::OBJECT_TYPE: heap.mark(bucket.var.object); break;
					default: break;
				}
				rebuildLoadCount++;
			}
		}
	}
	if (rebuildLoadCount != loadCount) {
		UInt32 check = const_cast<Table*>(this)->rebuild((loadCount < calcMaxLoad(1U << (n - 1))) ? n - 1 : n);
		(void)check;
		assert(check == rebuildLoadCount);
	}
}

Table::Bucket* Table::find(const String* key, UInt32 hash) const {
	UInt32 mask = (buckets.size() - 1);
	UInt32 i = hash & mask;
	UInt16 hash16 = static_cast<UInt16>(hash);
	Bucket* bucket;
	while ((bucket = &buckets[i])->keyExists()) {
		if (bucket->hash16 == hash16 && bucket->key->isEqualTo(*key)) {
			if (i != (hash & mask)) { // move found bucket one step closer to search entry, to speed up future lookups
				std::swap(buckets[i], buckets[(i - 1) & mask]);
				i = (i - 1) & mask;
			}
			break;
		}
		i = (i + 1) & mask;
	}
	return &buckets[i];
}

UInt32 Table::rebuild(UInt32 newN) {
	QuickArray<Bucket, 1U << TABLE_BUILT_IN_N> newBuckets(1U << newN, buckets.getHeap());
	UInt32 newLoadCount = 0;
	UInt32 mask = (1U << newN) - 1;
	for (UInt32 i = 0; i < buckets.size(); ++i) {
		const Bucket& bucket = buckets[i];
		if (bucket.valueExists()) {
			UInt32 j = bucket.key->calcHash() & mask;
			while (newBuckets[j].keyExists()) {
				j = (j + 1) & mask;
			}
			newBuckets[j] = bucket;
			++newLoadCount;
		}
	}
	buckets.swap(newBuckets);
	n = newN;
	loadCount = newLoadCount;
	return newLoadCount;
}

/* --- ScriptObject --- */

ScriptObject::ScriptObject(Heap* heap, Object* prototype) : properties(heap), prototype(prototype) { }

Object* ScriptObject::getPrototype(Engine& engine) const { return prototype; }
Table& ScriptObject::accessProperties() { return properties; }

bool ScriptObject::setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags) {
	return properties.update(properties.insert(key.toString(engine.getHeap())), v, flags);
}

Flags ScriptObject::getOwnProperty(Engine& engine, const Value& key, Value* v) const {
	const Table::Bucket* bucket = properties.lookup(key.toString(engine.getHeap()));
	if (bucket != 0) {
		*v = bucket->getValue();
		return bucket->getFlags();
	}
	return NONEXISTENT;
}

bool ScriptObject::deleteProperty(Engine& engine, const Value& key) {
	Table::Bucket* bucket = properties.lookup(key.toString(engine.getHeap()));
	return (bucket == 0 || properties.erase(bucket));
}

static StringListEnumerator* createTableKeyList(GCHeap& heap, const Table& table) {
	StringListEnumerator* list = heap.manage(new (&heap) StringListEnumerator(&heap, table.getLoadCount()));
	for (Table::Bucket* bucket = table.getFirst(); bucket != 0; bucket = table.getNext(bucket)) {
		if (bucket->doEnumerate()) {
			list->add(bucket->getKey());
		}
	}
	return list;
}

Enumerator* ScriptObject::getPropertyEnumerator(Engine& engine) const {
	return createTableKeyList(engine.getHeap(), properties);
}

/* --- WrapperObject --- */

WrapperObject::WrapperObject(Heap* heap, const String* className, const Value& value, Object* prototype)
	: className(className), wrapped(value), super(heap, prototype) {
}

const String* WrapperObject::getClassName() const { return className; }
Value WrapperObject::getInternalValue(GCHeap& heap) const { return wrapped; }

/* --- StringWrapper --- */

StringWrapper::StringWrapper(Heap* heap, const String* string, Object* prototype) : wrapped(string), super(heap, prototype) { }
const String* StringWrapper::getClassName() const { return &S_TRING_STRING; }
const String* StringWrapper::toString(GCHeap& heap) const { return wrapped; }
Value StringWrapper::getInternalValue(GCHeap& heap) const { return Value(wrapped); }

Flags StringWrapper::getOwnProperty(Engine& engine, const Value& key, Value* v) const {
	Flags flags = wrapped->getOwnProperty(engine, key, v);;
	return (flags != NONEXISTENT ? flags : super::getOwnProperty(engine, key, v));
}

bool StringWrapper::setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags) {
	if (wrapped->hasOwnProperty(engine, key)) {
		return false;
	}
	return super::setOwnProperty(engine, key, v, flags);
}

bool StringWrapper::deleteProperty(Engine& engine, const Value& key) {
	if (wrapped->hasOwnProperty(engine, key)) {
		return false;
	} else {
		return super::deleteProperty(engine, key);
	}
}

Enumerator* StringWrapper::getPropertyEnumerator(Engine& engine) const {
	GCHeap& heap = engine.getHeap();
	return heap.manage(new (&heap) JoiningEnumerator(engine, super::getPropertyEnumerator(engine), wrapped));
}

/* --- RangeEnumerator --- */

RangeEnumerator::RangeEnumerator(GCHeap& heap, Int32 from, Int32 count) : heap(heap), index(from), to(from + count) { }
const String* RangeEnumerator::nextPropertyName() { return (index >= to ? 0 : String::fromInt(heap, index++)); }

/* --- StringListEnumerator --- */

StringListEnumerator::StringListEnumerator(Heap* heap, UInt32 initialCapacity) : list(heap), nextIndex(0) { list.reserve(initialCapacity); }
void StringListEnumerator::add(const String* s) { list.append(s); }
const String* StringListEnumerator::nextPropertyName() { return (nextIndex < list.size() ? list[nextIndex++] : 0); }

/* --- JoiningEnumerator --- */

JoiningEnumerator::JoiningEnumerator(Engine& engine, Enumerator* leftEnumerator, const Object* rightObject)
		: engine(engine)
		, leftEnumerator(leftEnumerator)
		, rightObject(rightObject)
		, rightEnumerator(rightObject->getPropertyEnumerator(engine)) {
}

const String* JoiningEnumerator::nextPropertyName() {
	const String* name = rightEnumerator->nextPropertyName();
	if (name == 0) {
		do {
			name = leftEnumerator->nextPropertyName();
			if (name != 0 && !rightObject->isOwnPropertyEnumerable(engine, Value(name))) {
				return name;
			}
		} while (name != 0);
	}
	return name;
}

/* --- CodeDef --- */

CodeDefinition::CodeDefinition(Heap* heap, Constants* constants)
		: code(heap), constants(constants), nameIndexes(heap), varNames(heap), argumentNames(heap), temporaries(heap)
		, name(0), selfName(0), source(0) { assert(constants != 0); }

bool CodeDefinition::lookupNameIndex(const String* name, Int32& index) const {
	const Table::Bucket* bucket = nameIndexes.lookup(name);
	if (bucket == 0) {
		return false;
	}
	assert(!bucket->getValue().isUndefined());
	index = bucket->getValue().toInt();
	return true;
}

/* --- Function --- */

Function* Function::asFunction() { return this; }
const String* Function::typeOfString() const { return &FUNCTION_STRING; }
const String* Function::getClassName() const { return &F_UNCTION_STRING; }
const String* Function::toString(GCHeap& heap) const { return &NATIVE_FUNCTION_STRING; }
Value Function::getInternalValue(GCHeap& heap) const { return Value(&NATIVE_FUNCTION_STRING); }
Object* Function::getPrototype(Engine& engine) const { return &engine.functionPrototype; };

Value Function::construct(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	return call(engine, processor, argc, argv, thisObject);
}

bool Function::hasInstance(Engine& engine, Processor& processor, Object* object) const {
	if (object == 0) {
		return false;
	}
	Value v(Value::UNDEFINED);
	getPropertyDeep(engine, Value(&PROTOTYPE_STRING), &v);
	Object* prototypeObject = v.asObject();
	if (prototypeObject == 0) {
		processor.error(TYPE_ERROR, engine.getHeap().manage(new (&engine.getHeap()) String("Non-object prototype in instanceof check")));
		return false;
	}
	while ((object = object->getPrototype(engine)) != 0) {
		if (object == prototypeObject) {
			return true;
		}
	}
	return false;
}

/* --- Array --- */

Array::Array(Heap* heap) : length(0), denseVector(heap) { }

const String* Array::getClassName() const { return &A_RRAY_STRING; }

Object* Array::getPrototype(Engine& engine) const { // FIX : have a sep. ArrayPrototype object like we have for Function?
	return (this == &engine.arrayPrototype ? static_cast<Object*>(&engine.objectPrototype)
			: static_cast<Object*>(&engine.arrayPrototype));
}

Flags Array::getOwnProperty(Engine& engine, const Value& key, Value* v) const {
	UInt32 index;
	if (key.toArrayIndex(index) && index < denseVector.size()) {
		*v = denseVector[index];
		return STANDARD_FLAGS | EXISTS_FLAG;
	}
	if (key.isEqualTo(LENGTH_STRING)) {
		*v = Value(length);
		return (READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG | EXISTS_FLAG);
	}
	return super::getOwnProperty(engine, key, v);
}

void Array::sliceDenseVector(Engine& engine, const Value& key) {
	UInt32 index;
	if (key.toArrayIndex(index) && index < denseVector.size()) {
		GCHeap& heap = engine.getHeap();
		Table& props = getCompleteObject(engine)->accessProperties();
		for (UInt32 moveIndex = index + 1; moveIndex < denseVector.size(); ++moveIndex) {
			props.update(props.insert(String::fromInt(heap, moveIndex)), denseVector[moveIndex]);
		}
		denseVector.resize(index);
// FIX :		std::cerr << "sliced " << this << " at " << index << " (dense size: " << denseVector.size() << ", table load: " << props.getLoadCount() << ")" << std::endl;
	}
}

bool Array::setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags) {
	UInt32 index;
	if (key.toArrayIndex(index) && index != 0xFFFFFFFFU) {
		if ((flags & (READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG)) != 0) {
			sliceDenseVector(engine, key);
		} else {
			if (index >= length) {
				length = index + 1;
			}
			if (index < denseVector.size()) {
				denseVector[index] = v;
				return true;
			}
			if (index == denseVector.size()) {
				if (completeObject == 0) {
					denseVector.append(v);
				} else {
					GCHeap& heap = engine.getHeap();
					Table& props = completeObject->accessProperties();
					Table::Bucket* bucket = props.lookup(String::fromInt(heap, index));
					if (bucket == 0 || bucket->hasStandardFlags()) {
						UInt32 moveIndex = index + 1;
						denseVector.append(v);
						while ((bucket = props.lookup(String::fromInt(heap, moveIndex))) != 0 && bucket->hasStandardFlags()) {
							denseVector.append(bucket->getValue());
							props.erase(bucket);
							++moveIndex;
						}
	// FIX :					std::cerr << "extended " << this << " at " << index << " (dense size: " << denseVector.size() << ", table load: " << props.getLoadCount() << ")" << std::endl;
					}
				}
				return true;
			}
		}
	} else if (key.isEqualTo(LENGTH_STRING)) {
		UInt32 newLength;
		if (!v.toArrayIndex(newLength)) {
			GCHeap& heap = engine.getHeap();
			throw Exception(heap, Value(heap.manage(new (&heap) Error(RANGE_ERROR, heap.manage(new (&heap) String("Invalid array length")))))); // FIX : how could we do this without exceptions?
		}
		if (newLength < denseVector.size()) {
			denseVector.resize(newLength);
		}
		if (completeObject != 0) {
			Table& props = completeObject->accessProperties();
			for (Table::Bucket* bucket = props.getFirst(); bucket != 0; bucket = props.getNext(bucket)) {
				UInt32 i;
				if (Value(bucket->getKey()).toArrayIndex(i) && i >= newLength && !props.erase(bucket)) {
					return false;
				}
			}
		}
		length = newLength;
		return true;
	}

	return super::setOwnProperty(engine, key, v, flags);
}

bool Array::deleteProperty(Engine& engine, const Value& key) {
	if (key.isEqualTo(LENGTH_STRING)) {
		return false;
	}
	sliceDenseVector(engine, key);
	return super::deleteProperty(engine, key);
}

void Array::constructCompleteObject(Engine& engine) const { }

Enumerator* Array::getPropertyEnumerator(Engine& engine) const {
	GCHeap& heap = engine.getHeap();
	Enumerator* rangeEnumerator = heap.manage(new (&heap) RangeEnumerator(heap, 0, denseVector.size()));
	return (completeObject == 0 ? rangeEnumerator
			: heap.manage(new (&heap) JoiningEnumerator(engine, rangeEnumerator, completeObject)));
}

/* --- LazyObject --- */

template<class SUPER> LazyObject<SUPER>::LazyObject() : completeObject(0) { }

template<class SUPER> Flags LazyObject<SUPER>::getOwnProperty(Engine& engine, const Value& key, Value* v) const {
	return getCompleteObject(engine)->getOwnProperty(engine, key, v);
}

template<class SUPER> bool LazyObject<SUPER>::setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags) {
	return getCompleteObject(engine)->setOwnProperty(engine, key, v, flags);
}

template<class SUPER> bool LazyObject<SUPER>::deleteProperty(Engine& engine, const Value& key) {
	return getCompleteObject(engine)->deleteProperty(engine, key);
}

template<class SUPER> Enumerator* LazyObject<SUPER>::getPropertyEnumerator(Engine& engine) const {
	return getCompleteObject(engine)->getPropertyEnumerator(engine);
}

template<class SUPER> ScriptObject* LazyObject<SUPER>::getCompleteObject(Engine& engine) const {
	if (completeObject == 0) {
		GCHeap& heap = engine.getHeap();
		completeObject = heap.manage(new (&heap) ScriptObject(&heap, SUPER::getPrototype(engine)));
		constructCompleteObject(engine);
	}
	return completeObject;
}

/* --- ExtensibleFunction --- */

void ExtensibleFunction::createPrototypeObject(Engine& engine, Object* forObject, bool dontEnumPrototype) const {
	GCHeap& heap = engine.getHeap();
	ScriptObject* prototype = heap.manage(new (&heap) ScriptObject(&heap, &engine.objectPrototype));
	prototype->setOwnProperty(engine, Value(&CONSTRUCTOR_STRING), Value(const_cast<ExtensibleFunction*>(this)), DONT_ENUM_FLAG);
#if (STRICTLY_ES3)
	forObject->setOwnProperty(engine, Value(&PROTOTYPE_STRING), Value(prototype), (dontEnumPrototype ? (DONT_ENUM_FLAG | DONT_DELETE_FLAG) : DONT_DELETE_FLAG));
#else
	forObject->setOwnProperty(engine, Value(&PROTOTYPE_STRING), Value(prototype), DONT_ENUM_FLAG | DONT_DELETE_FLAG);
#endif
}

/* --- ScriptFunction --- */

ScriptFunction::ScriptFunction(CodeDefinition* codeDef, Frame* environment) : codeDef(codeDef), environment(environment) { }
Value ScriptFunction::getInternalValue(GCHeap& heap) const { return Value(toString(heap)); }
CodeDefinition* ScriptFunction::getDefinition() const { return codeDef; }

const String* ScriptFunction::toString(GCHeap& heap) const {
	const String* s = codeDef->getSource();
	return (s != 0 ? s : Object::toString(heap));
}

Value ScriptFunction::call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	processor.chain(this, argc, argv, thisObject);
	return Value();
}

void ScriptFunction::constructCompleteObject(Engine& engine) const {
	createPrototypeObject(engine, completeObject, false);
#if (STRICTLY_ES3)
	completeObject->setOwnProperty(engine, Value(&NAME_STRING), Value(codeDef->getName()), DONT_ENUM_FLAG);
	completeObject->setOwnProperty(engine, Value(&LENGTH_STRING), Value(codeDef->getArgumentsCount()), READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG);
#else
	completeObject->setOwnProperty(engine, Value(&NAME_STRING), Value(codeDef->getName()), READ_ONLY_FLAG | DONT_ENUM_FLAG);
	completeObject->setOwnProperty(engine, Value(&LENGTH_STRING), Value(codeDef->getArgumentsCount()), READ_ONLY_FLAG | DONT_ENUM_FLAG);
#endif
}

/* --- SpecialConstructorFunction --- */

SpecialConstructorFunction::SpecialConstructorFunction(Function* regularFunction, Function* constructorFunction)
		: regularFunction(regularFunction), constructorFunction(constructorFunction) {
	assert(regularFunction != 0);
}

Value SpecialConstructorFunction::call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	return regularFunction->call(engine, processor, argc, argv, thisObject);
}

Value SpecialConstructorFunction::construct(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	if (constructorFunction == 0) {
		Value v;
		regularFunction->getPropertyDeep(engine, Value(&NAME_STRING), &v);
		GCHeap& heap = engine.getHeap();
		processor.error(TYPE_ERROR, heap.manage(new (&heap) String(*v.toString(heap), String(" is not a constructor"), &heap)));
		return Value();
	} else {
		return constructorFunction->call(engine, processor, argc, argv, thisObject);
	}
}

void SpecialConstructorFunction::constructCompleteObject(Engine& engine) const {
	Object* templateFunction = regularFunction;
	if (constructorFunction != 0) {
		createPrototypeObject(engine, completeObject, true);
		templateFunction = constructorFunction;
	}
	Value v;
	Flags flags = templateFunction->getPropertyDeep(engine, Value(&NAME_STRING), &v);
	if (flags != NONEXISTENT) { // FIX : getName?
		completeObject->setOwnProperty(engine, Value(&NAME_STRING), v, flags);
	}
	flags = templateFunction->getPropertyDeep(engine, Value(&LENGTH_STRING), &v);
	if (flags != NONEXISTENT) { // FIX : in the future I think we should have a getLength
		completeObject->setOwnProperty(engine, Value(&LENGTH_STRING), v, flags);
	}
}

/* --- FunctionPrototypeFunction --- */

Value FunctionPrototypeFunction::call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	return Value::UNDEFINED;
}

Object* FunctionPrototypeFunction::getPrototype(Engine& engine) const { return &engine.objectPrototype; }

void FunctionPrototypeFunction::constructCompleteObject(Engine& engine) const {
#if (STRICTLY_ES3)
	completeObject->setOwnProperty(engine, Value(&NAME_STRING), Value(&EMPTY_STRING), DONT_ENUM_FLAG);
	completeObject->setOwnProperty(engine, Value(&LENGTH_STRING), Value(0.0), READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG);
#else
	completeObject->setOwnProperty(engine, Value(&NAME_STRING), Value(&EMPTY_STRING), READ_ONLY_FLAG | DONT_ENUM_FLAG);
	completeObject->setOwnProperty(engine, Value(&LENGTH_STRING), Value(0.0), READ_ONLY_FLAG | DONT_ENUM_FLAG);
#endif
}

/* --- Error --- */

Error::Error(ErrorType type, const String* message)	: errorType(type), name(&ERROR_NAMES[errorType]), message(message) {
	assert(0 <= errorType && errorType < ERROR_TYPE_COUNT);
}

const String* Error::getClassName() const { return &E_RROR_STRING; }
Value Error::getInternalValue(GCHeap& heap) const { return Value(&ERROR_NAMES[errorType]); }
Object* Error::getPrototype(Engine& engine) const { return engine.errorPrototypes[errorType]; }

const String* Error::toString(GCHeap& heap) const {
	return (message == 0 ? name : heap.manage(new (&heap) String(String(*name, COLON_SPACE, &heap), *message, &heap))); // FIX : we really need operator+ :)
}

void Error::updateReflection(Engine& engine) {
	Value v;
	name = (getPropertyDeep(engine, Value(&NAME_STRING), &v) != NONEXISTENT ? v.toString(engine.getHeap()) : &ERROR_NAMES[errorType]);
	message = (getPropertyDeep(engine, Value(&MESSAGE_STRING), &v) != NONEXISTENT ? v.toString(engine.getHeap()) : 0);
}

bool Error::setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags) {
	const bool result = super::setOwnProperty(engine, key, v, flags);
	updateReflection(engine);
	return result;
}

bool Error::deleteProperty(Engine& engine, const Value& key) {
	const bool result = super::deleteProperty(engine, key);
	updateReflection(engine);
	return result;
}

void Error::constructCompleteObject(Engine& engine) const {
	if (message != 0) {
		completeObject->setOwnProperty(engine, Value(&MESSAGE_STRING), Value(message), DONT_ENUM_FLAG);
	}
}

/* --- ErrorPrototype --- */

ErrorPrototype::ErrorPrototype(ErrorType type) : super(type) { }

Object* ErrorPrototype::getPrototype(Engine& engine) const {
	return (errorType == GENERIC_ERROR ? static_cast<Object*>(&engine.objectPrototype)
			: static_cast<Object*>(engine.errorPrototypes[GENERIC_ERROR]));
}

/* --- Arguments --- */

Arguments::Arguments(Heap* heap, ScriptFrame* frame, UInt32 argumentsCount)
		: frame(frame)
		, argumentsCount(argumentsCount)
		, deletedArguments(argumentsCount, heap) {
	std::fill(deletedArguments.begin(), deletedArguments.end(), false);
}

const String* Arguments::getClassName() const { return &A_RGUMENTS_STRING; }

#if (STRICTLY_ES3)
const String* Arguments::toString(GCHeap& heap) const {
	return heap.manage(new (&heap) String(String(BRACKET_OBJECT_STRING, O_BJECT_STRING, &heap), END_BRACKET_STRING,& heap));
}
#endif

Object* Arguments::getPrototype(Engine& engine) const { return &engine.objectPrototype; }

Value* Arguments::findProperty(const Value& key) const {
	UInt32 i;
	return (key.toArrayIndex(i) && i < argumentsCount && !deletedArguments[i] ? frame->getIndexedPointer() + i : 0);
}

Flags Arguments::getOwnProperty(Engine& engine, const Value& key, Value* v) const {
	const Value* p = findProperty(key);
	return (p == 0 ? super::getOwnProperty(engine, key, v) : (*v = *p
#if (STRICTLY_ES3)
			, (STANDARD_FLAGS | DONT_ENUM_FLAG | EXISTS_FLAG)));
#else
			, (STANDARD_FLAGS | EXISTS_FLAG)));
#endif
}

bool Arguments::setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags) {
	Value* p = findProperty(key);
	if (p != 0 && (flags & (READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG)) != 0) {
		deletedArguments[p - frame->getIndexedPointer()] = true;
		p = 0;
	}
	return (p == 0 ? super::setOwnProperty(engine, key, v, flags) : (*p = v, true));
}

bool Arguments::deleteProperty(Engine& engine, const Value& key) {
	const Value* p = findProperty(key);
	if (p == 0) return super::deleteProperty(engine, key);
	else return (deletedArguments[p - frame->getIndexedPointer()] = true);
}

Enumerator* Arguments::getPropertyEnumerator(Engine& engine) const {
#if (STRICTLY_ES3)
	return (completeObject == 0 ? super::getPropertyEnumerator(engine) : completeObject->getPropertyEnumerator(engine));
#else
	GCHeap& heap = engine.getHeap();
	StringListEnumerator* list = heap.manage(new (&heap) StringListEnumerator(argumentsCount));
	for (UInt32 i = 0; i < argumentsCount; ++i) {
		if (!deletedArguments[i]) {
			list->add(String::fromInt(heap, i));
		}
	}
	return (completeObject == 0 ? static_cast<Enumerator*>(list)
			: heap.manage(new (&heap) JoiningEnumerator(engine, list, completeObject)));
#endif
}

void Arguments::constructCompleteObject(Engine& engine) const {
	completeObject->setOwnProperty(engine, Value(&LENGTH_STRING), Value(argumentsCount), DONT_ENUM_FLAG);
	completeObject->setOwnProperty(engine, Value(&CALLEE_STRING), Value(frame->function), DONT_ENUM_FLAG);
}

/* --- ScriptFrame --- */

static UInt32 calcIndexedVariablesSize(const ScriptFunction* function, UInt32 argc) {
	const CodeDefinition* codeDef = function->getDefinition();
	return codeDef->getVarsCount() + std::max(codeDef->getArgumentsCount(), argc);
}

ScriptFrame::ScriptFrame(Heap* heap, ScriptFunction* function, UInt32 argc, const Value* argv)
		: function(function), passedArgumentsCount(argc), indexed(calcIndexedVariablesSize(function, argc), heap) {
	const CodeDefinition* codeDef = function->getDefinition();
	indexedPointer = indexed.begin() + codeDef->getVarsCount();
	parentFrame = function->environment;
	std::fill(indexed.begin(), indexedPointer, Value::UNDEFINED);
	Value* e = std::copy(argv, argv + argc, indexedPointer);
	if (codeDef->getArgumentsCount() > argc) {
		std::fill(e, indexed.end(), Value::UNDEFINED);
	}
}

// FIX : this one must GO! Q&D crap
void ScriptFrame::updatedCode() {
	const CodeDefinition* codeDef = function->getDefinition();
	const UInt32 oldVarsCount = indexedPointer - indexed.begin();
	const UInt32 oldArgsCount = indexed.end() - indexedPointer;
	const UInt32 newVarsCount = codeDef->getVarsCount();
	const UInt32 newArgsCount = std::max(codeDef->getArgumentsCount(), passedArgumentsCount);
	assert(newVarsCount >= oldVarsCount);
	assert(newArgsCount >= oldArgsCount);
	QuickArray<Value> newIndexed(newVarsCount + newArgsCount, indexed.getHeap());
	std::fill(newIndexed.begin(), newIndexed.begin() + (newVarsCount - oldVarsCount), Value::UNDEFINED);
	std::copy(indexed.begin(), indexed.end(), newIndexed.begin() + (newVarsCount - oldVarsCount));
	std::fill(newIndexed.end() - (newArgsCount - oldArgsCount), newIndexed.end(), Value::UNDEFINED);
	indexed.swap(newIndexed);
	indexedPointer = indexed.begin() + newVarsCount;
}

Flags ScriptFrame::getOwnProperty(Engine& engine, const Value& key, Value* v) const {
	const CodeDefinition* codeDef = function->getDefinition();
	const String* name = &EMPTY_STRING;
	if (key.isString()) { // FIX : <-- unnecessary except for the global frame, cause you can never have variables with numerical names
		name = key.getString();
		Int32 index;
		if (codeDef->lookupNameIndex(name, index)) {
			assert(indexed.begin() <= indexedPointer + index && indexedPointer + index < indexed.end());
			*v = indexedPointer[index];
			return DONT_DELETE_FLAG | EXISTS_FLAG;
		}
	}
	if (parentFrame == 0) {
		return getCompleteObject(engine)->getOwnProperty(engine, key, v);
	}
	if (completeObject != 0 || name->isEqualTo(ARGUMENTS_STRING)) {
		Flags flags = super::getOwnProperty(engine, key, v);
		if (flags != NONEXISTENT) {
			return flags;
		}
	}
	if (codeDef->selfName != 0 && name->isEqualTo(*codeDef->selfName)) {
		*v = Value(function);
		return DONT_DELETE_FLAG | READ_ONLY_FLAG;
	}
	return parentFrame->getOwnProperty(engine, key, v); // FIX: can we do a looping non-recursive solution? faster  AND (stack overflow) SAFER!
}

bool ScriptFrame::setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags) {
	const CodeDefinition* codeDef = function->getDefinition();
	const String* name = &EMPTY_STRING;
	if (key.isString()) { // FIX : <-- unnecessary except for the global frame, cause you can never have variables with numerical names
		name = key.getString();
		Int32 index;
		if (codeDef->lookupNameIndex(name, index)) {
			assert(indexed.begin() <= indexedPointer + index && indexedPointer + index < indexed.end());
			indexedPointer[index] = v;
			return true;
		}
	}
	if (parentFrame == 0) {
		return getCompleteObject(engine)->setOwnProperty(engine, key, v, flags);
	}
	// FIX : make sub that lookup's the bucket, also use in getProperty
	if (completeObject != 0 || name->isEqualTo(ARGUMENTS_STRING)) {
		Table& props = getCompleteObject(engine)->accessProperties();
		Table::Bucket* bucket = props.lookup(name);
		if (bucket != 0) {
			props.update(bucket, v);
			return true;
		}
	}
	if (codeDef->selfName != 0 && name->isEqualTo(*codeDef->selfName)) {
		return false;
	}
	return parentFrame->setOwnProperty(engine, key, v, flags); // FIX : recursion
}

bool ScriptFrame::deleteProperty(Engine& engine, const Value& key) {
	const CodeDefinition* codeDef = function->getDefinition();
	const String* name = &EMPTY_STRING;
	if (key.isString()) { // FIX : <-- unnecessary except for the global frame, cause you can never have variables with numerical names
		name = key.getString();
		Int32 index;
		if (codeDef->lookupNameIndex(name, index)) {
			return false;
		}
	}
	if (parentFrame == 0) {
		return getCompleteObject(engine)->deleteProperty(engine, key);
	}
	if (completeObject != 0 || name->isEqualTo(ARGUMENTS_STRING)) {
		Table& props = getCompleteObject(engine)->accessProperties();
		Table::Bucket* bucket = props.lookup(name);
		if (bucket != 0) {
			return props.erase(bucket);
		}
	}
	if (codeDef->selfName != 0 && name->isEqualTo(*codeDef->selfName)) {
		return false;
	}
	return parentFrame->deleteProperty(engine, key); // FIX : recursion
}

void ScriptFrame::declareVar(Engine& engine, const String* name, const Value& initValue) {
	Int32 index;
	if (!function->getDefinition()->lookupNameIndex(name, index)) {
		Table& props = getCompleteObject(engine)->accessProperties();
		if (props.lookup(name) == 0) { // FIX : lookup and insert only if 0 at once, how?!
			props.update(props.insert(name), Value::UNDEFINED);
		}
	}
	if (!initValue.isUndefined()) {
		setOwnProperty(engine, Value(name), initValue, DONT_DELETE_FLAG);
	}
}

void ScriptFrame::constructCompleteObject(Engine& engine) const {
	if (parentFrame != 0) {
		GCHeap& heap = engine.getHeap();
		completeObject->setOwnProperty(engine, Value(&ARGUMENTS_STRING)
				, Value(heap.manage(new (&heap) Arguments(&heap, const_cast<ScriptFrame*>(this), passedArgumentsCount)))
				, DONT_DELETE_FLAG);
	}
}

Enumerator* ScriptFrame::getPropertyEnumerator(Engine& engine) const {
	Enumerator* keyEnumerator = createTableKeyList(engine.getHeap(), function->getDefinition()->nameIndexes);
	// FIX : add 'manage' and 'new ()' to engine as well, this is so common?
	return engine.getHeap().manage(new (&engine.getHeap()) JoiningEnumerator(engine, keyEnumerator, getCompleteObject(engine)));
}

/* --- EvalFrame --- */

EvalFrame::EvalFrame(Heap* heap, Frame* parentFrame, const CodeDefinition* evalCodeDef)
		: super(parentFrame), evalCodeDef(evalCodeDef), indexed(evalCodeDef->getVarsCount(), heap) {
	indexedPointer = indexed.end();
}
Flags EvalFrame::getOwnProperty(Engine& engine, const Value& key, Value* v) const { return parentFrame->getOwnProperty(engine, key, v); }
bool EvalFrame::setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags) { return parentFrame->setOwnProperty(engine, key, v, flags); }
bool EvalFrame::deleteProperty(Engine& engine, const Value& key) { return parentFrame->deleteProperty(engine, key); }

/* --- CatchFrame --- */

CatchFrame::CatchFrame(Frame* parentFrame, const String* exceptionName, const Value& exceptionValue)
		: super(parentFrame), exceptionName(exceptionName), exceptionValue(exceptionValue) {
	indexedPointer = parentFrame->getIndexedPointer();
}

Flags CatchFrame::getOwnProperty(Engine& engine, const Value& key, Value* v) const {
	// FIX : always isString here right?
	if (key.isEqualTo(*exceptionName)) { *v = exceptionValue; return true; }
	else return parentFrame->getOwnProperty(engine, key, v);
}

bool CatchFrame::setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags) {
	if (key.isEqualTo(*exceptionName)) { exceptionValue = v; return true; }
	else return parentFrame->setOwnProperty(engine, key, v, flags);
}

bool CatchFrame::deleteProperty(Engine& engine, const Value& key) {
	if (key.isEqualTo(*exceptionName)) return false;
	else return parentFrame->deleteProperty(engine, key);
}

/* --- WithFrame --- */

WithFrame::WithFrame(Frame* parentFrame, Object* withObject) : super(parentFrame), withObject(withObject) {
	indexedPointer = parentFrame->getIndexedPointer();
}

const CodeDefinition* WithFrame::getFunctionDef() const { return parentFrame->getFunctionDef(); } // FIX : recursion

Flags WithFrame::getOwnProperty(Engine& engine, const Value& key, Value* v) const {
	Flags flags = withObject->getPropertyDeep(engine, key, v);
	return (flags != NONEXISTENT ? flags : parentFrame->getOwnProperty(engine, key, v));
}

bool WithFrame::setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags) {
	Value dummy;
	// FIX : ok, this is seriously disturbed, climbing up the prototype tree first to check existence, then again to check non-read-only (in setPropertyDeep)
	Flags findFlags = withObject->getPropertyDeep(engine, key, &dummy);
	return (findFlags != NONEXISTENT ? withObject->setPropertyDeep(engine, key, v)
			: parentFrame->setOwnProperty(engine, key, v, flags));
}

bool WithFrame::deleteProperty(Engine& engine, const Value& key) {
	Value dummy;
	Flags findFlags = withObject->getPropertyDeep(engine, key, &dummy);
	return (findFlags != NONEXISTENT ? withObject->deleteProperty(engine, key)
			: parentFrame->deleteProperty(engine, key));
}

/* --- Processor --- */

Processor::Processor(Engine& engine, CStackEntry* cstackStart, CStackEntry* cstackEnd, Value* vstackStart, Value* vstackEnd)
	: engine(engine), cstackStart(cstackStart), cstackEnd(cstackEnd), vstackStart(vstackStart), vstackEnd(vstackEnd)
	, clockCyclesLeft(0x7FFFFFFF), ip(0), vsp(vstackStart - 1), csp(cstackStart - 1), frame(0), constants(0), indexed(0)
	, catchers(&engine.getHeap()) { }

void Processor::push(const Value& v) { assert(vsp + 1 < vstackEnd); *++vsp = v; }
void Processor::poppush(const Value& v) { assert(vsp >= vstackStart); *--vsp = v; }
void Processor::pop(Int32 count) { assert(vsp - count + 1 >= vstackStart); vsp -= count; }
Int32 Processor::operand() { Int32 v = *reinterpret_cast<const Int32*>(ip); ip += OPERAND_SIZE; return v; }
void Processor::resetTimeOut(Int32 clockCycles) { clockCyclesLeft = clockCycles; }

void Processor::error(ErrorType errorType, const String* message) {
// FIX : which? both works!
//	throwVirtualException(Value(errorObject)
	GCHeap& heap = engine.getHeap();
	throw Exception(heap, Value(heap.manage(new (&heap) Error(errorType, message))));
}

void Processor::updateInternals() {
	frame = csp->frame;
	if (frame != 0) { // FIX : use dummy frame?
		constants = frame->getFunctionDef()->getConstantsPointer();
		indexed = frame->getIndexedPointer();
	}
}

void Processor::chain(const CodeWord* stub) {
	++csp; // FIX : overflow check
	assert(csp < cstackEnd);
	csp->returnIP = ip;
	csp->frame = 0;
	csp->thisObject = 0;
	ip = stub;
	updateInternals();
}

void Processor::chain(Function* completion) {
	// FIX : if we decide that CodeWord is int, we could have this array const
	static CodeWord chainCallStub[2 + OPERAND_SIZE] = { CALL_OP };
	*reinterpret_cast<Int32*>(&chainCallStub[1]) = 1;
	chainCallStub[1 + OPERAND_SIZE] = RETURN_OP;
	push(Value(completion));
	chain(chainCallStub);
}

void Processor::chain(ScriptFunction* func, UInt32 argc, const Value* argv, Object* thisObject) {
	GCHeap& heap = engine.getHeap();
	frame = heap.manage(new (&heap) ScriptFrame(&heap, func, argc, argv));
	++csp; // FIX : overflow check
	assert(csp < cstackEnd);
	csp->returnIP = ip;
	csp->frame = frame;
	csp->thisObject = thisObject;
	ip = func->getDefinition()->getCodePointer();
	updateInternals();
	--vsp; // That is right, we pre-compensate for the fact that in comparison to native calls we will end up pushing vsp once for the return value
}

void Processor::chain(const CodeWord* ip, Frame* frame, Object* thisObject) {
	this->frame = frame;
	++csp; // FIX : overflow check
	assert(csp < cstackEnd);
	csp->returnIP = this->ip;
	csp->frame = this->frame;
	csp->thisObject = thisObject;
	this->ip = ip;
	assert(ip != 0);
	updateInternals();
}

void Processor::enterEval(CodeDefinition* codeDef, bool directCall) {
	Frame* f = (directCall ? frame : engine.getGlobals());
	GCHeap& heap = engine.getHeap();
	frame = heap.manage(new (&heap) EvalFrame(&heap, f, codeDef));
	++csp; // FIX : overflow check
	assert(csp < cstackEnd);
	csp->returnIP = ip;
	csp->frame = frame;
	csp->thisObject = csp[-1].thisObject;
	ip = codeDef->getCodePointer();
	updateInternals();
	--vsp;
}

void Processor::throwVirtualException(const Value& exception) {
	if (catchers.empty()) { // FIX: what exception to throw here?
		throw Exception(engine.getHeap(), exception);
	}
	const Catcher& catcher = catchers[catchers.size() - 1];
	ip = catcher.ip;
	assert(ip != 0);
	csp = catcher.csp;
	vsp = catcher.vsp;
	push(exception);
	catchers.resize(catchers.size() - 1);
	updateInternals();
}

bool Processor::privateRun() {
	GCHeap& heap = engine.getHeap();
	assert(csp >= cstackStart);
	updateInternals();
	Object* o;
	Int32 offset;
	
	while (clockCyclesLeft >= 0) {
		--clockCyclesLeft;
		switch (*ip++) {
			case POP_OP:			pop(1); break;
			case REPUSH_OP:			push(vsp[0]); break;
			case REPUSH_2_OP:		push(vsp[-1]); push(vsp[-1]); break;
			case POST_SHUFFLE_OP:	push(vsp[0]); vsp[-1] = vsp[-2]; vsp[-2] = vsp[-3]; vsp[-3] = vsp[0]; break;
			case CONST_OP:			push(constants[operand()]); break;
			case READ_INDEXED_OP:	push(indexed[operand()]); break;
			case WRITE_INDEXED_OP:	indexed[operand()] = vsp[0]; break;
			case READ_NAMED_OP:		{
										const Value& name = constants[operand()];
										if (frame->getOwnProperty(engine, name, ++vsp) == NONEXISTENT) {
											error(REFERENCE_ERROR, heap.manage(new (&heap) String(*name.toString(heap)
													, IS_NOT_DEFINED_STRING, &heap)));
										}
									}
									break;
									
			case WRITE_NAMED_OP:	frame->setOwnProperty(engine, constants[operand()], vsp[0], STANDARD_FLAGS);
									break;
									
			case GET_PROPERTY_OP:	if ((o = vsp[-1].toObject(engine, false)) == 0) {
										error(TYPE_ERROR, &CANNOT_CONVERT_TO_OBJECT_STRING);
										break;
									}
									if (o->getPropertyDeep(engine, vsp[0], vsp - 1) == NONEXISTENT) {
										vsp[-1] = Value::UNDEFINED;
									}
									pop(1);
									break;
			
			case SET_PROPERTY_OP:	if ((o = vsp[-2].toObject(engine, false)) == 0) {
										error(TYPE_ERROR, &CANNOT_CONVERT_TO_OBJECT_STRING);
										break;
									}
									o->setPropertyDeep(engine, vsp[-1], vsp[0]);
									vsp[-2] = vsp[0];
									pop(2);
									break;

			case OBJ_TO_NUMBER_OP:
				if ((o = vsp[0].asObject()) != 0) {
					engine.toNumberFunction->call(engine, *this, 1, vsp, engine.getGlobals());
				}
				break;

			case OBJ_TO_STRING_OP:
				if ((o = vsp[0].asObject()) != 0) {
					engine.toStringFunction->call(engine, *this, 1, vsp, engine.getGlobals());
				}
				break;
				
			case PRE_EQ_OP:
				// FIX : doesn't feel totally efficient this...
				if (vsp[-1].isObject() != vsp[0].isObject()) {
					if (vsp[-1].isObject()) {
						std::swap(vsp[-1], vsp[0]);
					}
					if (vsp[-1].isString() || vsp[-1].isNumber() || vsp[-1].isBoolean()) {
						engine.toNumberFunction->call(engine, *this, 1, vsp, engine.getGlobals());
					}
				}
				break;

			case INC_OP:	vsp[0] = Value(vsp[0].toDouble() + 1.0); break;
			case DEC_OP:	vsp[0] = Value(vsp[0].toDouble() - 1.0); break;
			case ADD_OP:	poppush(vsp[-1].add(heap, vsp[0])); break;
			case SUB_OP:	poppush(Value(vsp[-1].toDouble() - vsp[0].toDouble())); break;
			case MUL_OP:	poppush(Value(vsp[-1].toDouble() * vsp[0].toDouble())); break;
			case DIV_OP:	poppush(Value(vsp[-1].toDouble() / vsp[0].toDouble())); break;
			case MOD_OP:	poppush(Value(modulo(vsp[-1].toDouble(), vsp[0].toDouble()))); break;
			case OR_OP:		poppush(Value(vsp[-1].toInt() | vsp[0].toInt())); break;
			case XOR_OP:	poppush(Value(vsp[-1].toInt() ^ vsp[0].toInt())); break;
			case AND_OP:	poppush(Value(vsp[-1].toInt() & vsp[0].toInt())); break;
			case SHL_OP:	poppush(Value(vsp[-1].toInt() << ((UInt32)vsp[0].toInt() & 0x1F))); break;
			case SHR_OP:	poppush(Value(vsp[-1].toInt() >> ((UInt32)vsp[0].toInt() & 0x1F))); break;
			case USHR_OP:	poppush(Value(((UInt32)(vsp[-1].toInt()) >> ((UInt32)vsp[0].toInt() & 0x1F)))); break;
			case PLUS_OP:	vsp[0] = Value(vsp[0].toDouble()); break;
			case MINUS_OP:	vsp[0] = Value(-vsp[0].toDouble()); break;
			case INV_OP:	vsp[0] = Value(~vsp[0].toInt()); break;
			case NOT_OP:	vsp[0] = Value(!vsp[0].toBool()); break;
			case X_EQ_OP:	poppush(Value(vsp[-1].isStrictlyEqualTo(vsp[0]))); break;
			case X_NEQ_OP:	poppush(Value(!vsp[-1].isStrictlyEqualTo(vsp[0]))); break;
			case EQ_OP:		poppush(Value(vsp[-1].isEqualTo(vsp[0]))); break;
			case NEQ_OP:	poppush(Value(!vsp[-1].isEqualTo(vsp[0]))); break;
			case LT_OP:		poppush(Value(vsp[-1].isLessThan(vsp[0]))); break;
			case LEQ_OP:	poppush(Value(vsp[-1].isLessThanOrEqualTo(vsp[0]))); break;
			case GT_OP:		poppush(Value(vsp[0].isLessThan(vsp[-1]))); break;
			case GEQ_OP:	poppush(Value(vsp[0].isLessThanOrEqualTo(vsp[-1]))); break;
			case JMP_OP:	offset = operand(); ip += offset; break;							
			case JT_OP:		offset = operand(); ip += (vsp[0].toBool() ? offset : 0); pop(1); break;
			case JF_OP:		offset = operand(); ip += (!vsp[0].toBool() ? offset : 0); pop(1); break;

			case JSR_OP:	offset = operand();
							++csp; // FIX : bounds check? or reserve?
							assert(csp < cstackEnd);
							csp->returnIP = ip;
							csp->frame = frame;
							csp->thisObject = csp[-1].thisObject;
							ip += offset;
							break;

			case RETURN_OP:	ip = (csp--)->returnIP;
							assert(ip != 0);
							assert(csp >= cstackStart);							
							updateInternals();
							break;

			case RETURN_TO_HOST_OP: // FIX : I don't like this opcode, is it really necessary?
							ip = (csp--)->returnIP;
							assert(csp >= cstackStart - 1);
							result = vsp[0];
							pop(1);
							// FIX : not necessary now cause we updateInternals at top, should we always? cause updateInterals here will fail if csp == cstackStart - 1
							// updateInternals();
							return false;

			case CALL_METHOD_OP: { // FIX : share more with the others...
				Int32 n = operand();
				Object* thisObject = vsp[-n - 1].toObject(engine, true);
				if (thisObject == 0) {
					error(TYPE_ERROR, &CANNOT_CONVERT_TO_OBJECT_STRING);
					break;
				}
				Value* returnPointer = vsp - n - 1; // Notice that call() will change vsp if it is a script function, so we save this first.
				Value v(Value::UNDEFINED);
				Function* f;
				const Value& name = vsp[-n];
				if (thisObject->getPropertyDeep(engine, name, &v) == NONEXISTENT || (f = v.asFunction()) == 0) {
					error(TYPE_ERROR, heap.manage(new (&heap) String(*name.toString(heap), IS_NOT_A_FUNCTION_STRING, &heap)));
					break;
				}
				*returnPointer = f->call(engine, *this, n, returnPointer + 2, thisObject); // Yeah, this *returnPointer assignment goes to waste if we called a script function (since it returns directly), but so what?
				vsp -= n + 1;
				break;
			}
			case CALL_EVAL_OP: { // FIX : all these are too similar!
				Int32 n = operand();
				Value* returnPointer = vsp - n; // Notice that call() will change vsp if it is a script function, so we save this first.
				Function* f = returnPointer->asFunction();
				if (f == 0) {
					error(TYPE_ERROR, heap.manage(new (&heap) String(*returnPointer->toString(heap), IS_NOT_A_FUNCTION_STRING, &heap)));
					break;
				}
				if (f == &engine.evalFunction) f = &engine.evalDirectFunction; // FIX : can't we implement this check with vm code instead of having a separate OP?
				*returnPointer = f->call(engine, *this, n, returnPointer + 1, engine.getGlobals()); // Yeah, this *returnPointer assignment goes to waste if we called a script function (since it returns directly), but so what?
				vsp -= n;
				break;
			}
			case CALL_OP: {
				Int32 n = operand();
				Value* returnPointer = vsp - n; // Notice that call() will change vsp if it is a script function, so we save this first.
				Function* f = returnPointer->asFunction();
				if (f == 0) {
					error(TYPE_ERROR, heap.manage(new (&heap) String(*returnPointer->toString(heap), IS_NOT_A_FUNCTION_STRING, &heap)));
					break;
				}
				*returnPointer = f->call(engine, *this, n, returnPointer + 1, engine.getGlobals()); // Yeah, this *returnPointer assignment goes to waste if we called a script function (since it returns directly), but so what?
				vsp -= n;
				break;
			}
			
			case NEW_OP: {
				Int32 n = operand(); // fix : top declare all variables in this switch, I seem to recall from GAZL that it is better
				Value* returnPointer = vsp - n; // Notice that call() will change vsp if it is a script function, so we save this first.
				Function* f = returnPointer->asFunction(); // fix: move this logic into f->construct, which means the NEW_RESULT_OP should be a chained function instead
				if (f == 0) { // FIX : sub
					error(TYPE_ERROR, heap.manage(new (&heap) String(*returnPointer->toString(heap), IS_NOT_A_FUNCTION_STRING, &heap)));
					break;
				}
				Value v(Value::UNDEFINED);
				f->getPropertyDeep(engine, Value(&PROTOTYPE_STRING), &v);
				Object* prototype = v.asObject();
				Object* newObject = heap.manage(new (&heap) ScriptObject(&heap, prototype != 0 ? prototype : &engine.objectPrototype));
				returnPointer[1] = f->construct(engine, *this, n, returnPointer + 1, newObject);							// Yeah, this *returnPointer assignment goes to waste if we called a script function (since it returns directly), but so what?
				returnPointer[0] = Value(newObject);
				vsp -= n - 1;
				break;
			}
			
			case NEW_RESULT_OP: poppush(vsp[0].isObject() ? vsp[0] : vsp[-1]); break;
			
			case NEW_OBJECT_OP: {
				push(Value(heap.manage(new (&heap) ScriptObject(&heap, &engine.objectPrototype))));
				break;
			}
			
			case NEW_ARRAY_OP: {
				push(Value(heap.manage(new (&heap) Array(&heap))));
				break;
			}
			
			case NEW_REG_EXP_OP: { // FIX : all these very similar
				Value* returnPointer = vsp - 1; // Notice that call() will change vsp if it is a script function, so we save this first.
				*returnPointer = engine.createRegExpFunction->call(engine, *this, 2, returnPointer, engine.getGlobals()); // Yeah, this *returnPointer assignment goes to waste if we called a script function (since it returns directly), but so what?
				--vsp;
				break;
			}

			case THIS_OP: push(Value(csp->thisObject)); break;
			case VOID_OP: push(Value::UNDEFINED); break;
			case GEN_FUNC_OP: {
				o = constants[operand()].asObject(); // FIX : .getObject?
				assert(o != 0);
				push(Value(heap.manage(new (&heap) ScriptFunction(reinterpret_cast<CodeDefinition*>(o), frame)))); // FIX : lose reinterpret_cast ?
				break;
			}
			case DECLARE_OP: {
				frame->declareVar(engine, constants[operand()].getString(), vsp[0]);
				pop(1);
				break;
			}
			case ADD_PROPERTY_OP: {
				o = vsp[-1].asObject();
				assert(o != 0);
				o->setOwnProperty(engine, constants[operand()], vsp[0], STANDARD_FLAGS);
				pop(1);
				break;
			}
			
			case FRAME_OP: push(Value(frame)); break;
			case DELETE_OP: {
				o = vsp[-1].toObject(engine, true);
				if (o == 0) {
					error(TYPE_ERROR, &CANNOT_CONVERT_TO_OBJECT_STRING);
					break;
				}
				poppush(Value(o->deleteProperty(engine, vsp[0])));
				break;
			}
			case PUSH_FRAME_OP: { // FIX : why don't we use the value stack for pushing / popping frames only (when it isn't a full call)
				++csp;
				assert(csp < cstackEnd);
				csp->returnIP = 0;
				Object* o = vsp[0].toObject(engine, false);
				if (o == 0) { // FIX : so common!
					error(TYPE_ERROR, &CANNOT_CONVERT_TO_OBJECT_STRING);
					break;
				}
				frame = csp->frame = heap.manage(new (&heap) WithFrame(frame, o));
				csp->thisObject = csp[-1].thisObject;
				pop(1);
				break;
			}
			case POP_FRAME_OP: {
				--csp;
				assert(csp >= cstackStart);
				updateInternals();
				break;
			}
			case TRY_OP: {
				offset = operand();
				catchers.append(Catcher(ip + offset, vsp, csp));
				break;
			}
			case TRIED_OP: catchers.resize(catchers.size() - 1); break;
			case CATCH_FRAME_OP: {
				++csp;
				assert(csp < cstackEnd);
				csp->returnIP = 0;
				frame = csp->frame = heap.manage(new (&heap) CatchFrame(frame, constants[operand()].getString(), vsp[0]));
				csp->thisObject = csp[-1].thisObject;
				pop(1);
				break;
			}
			case THROW_OP: throwVirtualException(vsp[0]); break;
			case IN_OP: {
				o = vsp[0].asObject();
				if (o == 0) {
					error(TYPE_ERROR, heap.manage(new (&heap) String(CANNOT_USE_IN_OPERATOR_STRING, *vsp[0].toString(heap), &heap)));
					break;
				}
				Value dummy;
				vsp[-1] = Value(o->getPropertyDeep(engine, vsp[-1], &dummy) != NONEXISTENT);
				pop(1);
				break;
			}
			case INSTANCE_OF_OP: {
				Function* f = vsp[0].asFunction();
				if (f == 0) {
					error(TYPE_ERROR, heap.manage(new (&heap) String(*vsp[0].toString(heap), IS_NOT_A_FUNCTION_STRING, &heap)));
					break;
				}
				vsp[-1] = Value(f->hasInstance(engine, *this, vsp[-1].asObject()));
				pop(1);
				break;
			}
			case TYPEOF_OP: vsp[0] = Value(vsp[0].typeOfString()); break;
			case TYPEOF_NAMED_OP: {
				Value v(Value::UNDEFINED);
				frame->getOwnProperty(engine, constants[operand()], &v);
				push(Value(v.typeOfString()));
				break;
			}
			case GET_ENUMERATOR_OP: {
				o = vsp[0].toObject(engine, false);
			#if (STRICTLY_ES3)
				if (o == 0) { // FIX : so common!
					error(TYPE_ERROR, &CANNOT_CONVERT_TO_OBJECT_STRING);
					break;
				}
			#endif
				// FIX : this is awesome and all, but it *will* become recursive nextPropertyName() calls, and recursive C++ is baaad for sandboxing
				// FIX : also, I don't like the order: prototypes are first (not that it matters much though)
				Enumerator* enumerator = &EMPTY_ENUMERATOR;
				while (o != 0) {
					enumerator = heap.manage(new (&heap) JoiningEnumerator(engine, enumerator, o));
					o = o->getPrototype(engine);
				}
				vsp[0] = Value(enumerator);
				break;
			}
			case NEXT_PROPERTY_OP: {
				o = vsp[0].asObject(); // FIX : .getObject?
				assert(o != 0);
				Int32 offset = operand();
				const String* name = reinterpret_cast<Enumerator*>(o)->nextPropertyName(); // FIX : lose reinterpret_cast<Enumerator*> ? 
				if (name != 0) {
					vsp[0] = Value(name);
				} else {
					ip += offset;
					pop(1);
				}
				break;
			}
			default: assert(0);
		}
	}
	return true;
}

bool Processor::run() {
	try {
		return privateRun();
	}
	catch (const Exception& x) {
		throwVirtualException(x.getValue());
		return true;
	}
}

// FIX : use accessCodeInt32
static Int32 getInt32(const CodeWord* code, UInt32& offset) {
	Int32 i = *reinterpret_cast<const Int32*>(code + offset);
	offset += OPERAND_SIZE;
	return i;
}

static void disassemble(GCHeap& heap, const CodeDefinition& codeDef) {
	const char* OP_STRINGS[] = {
		"!!! INVALID !!!"
		, "POP", "REPUSH", "REPUSH_2", "POST_SHUFFLE"
		, "CONST", "READ_INDEXED", "WRITE_INDEXED", "READ_NAMED", "WRITE_NAMED", "GET_PROPERTY", "SET_PROPERTY", "ADD_PROPERTY"
		, "OBJ_TO_NUMBER", "OBJ_TO_STRING", "PRE_EQ"
		, "INC", "DEC", "ADD", "SUB", "MUL", "DIV", "MOD"
		, "OR", "XOR", "AND"
		, "SHL", "SHR", "USHR"
		, "PLUS", "MINUS"
		, "BIT_NOT", "NOT"
		, "STRICTLY_EQ", "STRICTLY_NEQ", "EQ", "NEQ", "LT", "LEQ", "GT", "GEQ"
		, "JMP", "JSR", "JT", "JF"
		, "CALL", "CALL_METHOD", "CALL_EVAL", "NEW", "NEW_RESULT", "NEW_OBJ", "NEW_ARRAY", "NEW_REG_EXP"
		, "RETURN", "RETURN_TO_HOST"
		, "THIS", "VOID", "FRAME", "DELETE"
		, "GEN_FUNC", "DECLARE"
		, "THROW", "TRY", "TRIED", "CATCH_FRAME", "PUSH_FRAME", "POP_FRAME"
		, "IN", "INSTANCE_OF", "TYPEOF", "TYPEOF_NAMED"
		, "GET_ENUMERATOR", "NEXT_PROPERTY"
	};
	const CodeWord* code = codeDef.getCodePointer();
	const Value* constants = codeDef.getConstantsPointer();
	for (UInt32 offset = 0; offset < codeDef.getCodeSize();) {
		Processor::Opcode opcode = static_cast<Processor::Opcode>(code[offset]);
		if (opcode < 0 || opcode >= Processor::OP_COUNT) opcode = Processor::INVALID_OP;
		std::cerr << "\t" << offset << ':' << OP_STRINGS[opcode - Processor::INVALID_OP];
		++offset;
		switch (opcode) {
			case Processor::JT_OP:
			case Processor::JF_OP:
			case Processor::JMP_OP:
			case Processor::JSR_OP:
			case Processor::TRY_OP:
			case Processor::NEXT_PROPERTY_OP: { Int32 jump = getInt32(code, offset); std::cerr << " @" << (offset + jump); break; }
			case Processor::READ_INDEXED_OP:
			case Processor::WRITE_INDEXED_OP: {
				const Int32 index = getInt32(code, offset);
				const String* name = codeDef.getIndexedName(index);
				if (name != 0) std::wcerr << L" $" << name->toStdString();
				std::wcerr << L" (" << index << L")";
				break;
			}
			case Processor::CALL_OP:
			case Processor::CALL_METHOD_OP:
			case Processor::CALL_EVAL_OP:
			case Processor::NEW_OP: std::cerr << " *" << getInt32(code, offset); break;
			case Processor::CONST_OP:
			case Processor::GEN_FUNC_OP:
			case Processor::CATCH_FRAME_OP: std::wcerr << L" #" << constants[getInt32(code, offset)].toString(heap)->toStdString(); break;
			case Processor::DECLARE_OP:
			case Processor::READ_NAMED_OP:
			case Processor::WRITE_NAMED_OP:
			case Processor::ADD_PROPERTY_OP:
			case Processor::TYPEOF_NAMED_OP: std::wcerr << L" " << constants[getInt32(code, offset)].toString(heap)->toStdString(); break;
			default: break;
		}
		std::cerr << std::endl;
	}
}

/* --- Compiler --- */

const int INVALID_CODE_POINT = -1;

// FIX : sort
enum OperatorType {
	GROUP, UNARY, BINARY, IN_OPERATOR, ABSTRACT_EQUAL, ASSIGNMENT, PRE_INC_DEC, POST_INC_DEC, LOGICAL_AND_OR, PROPERTY_BRACKETS, PROPERTY_DOT, CONDITIONAL
	, FUNCTION_CALL, COMMA, VOID_OPERATOR, DELETE_OPERATOR, NEW_OPERATOR, TYPE_OF
};
enum Associativity { LEFT_TO_RIGHT, RIGHT_TO_LEFT };

struct OperatorInfo {
	const char* token;
	const char* closeToken;
	Precedence prec;
	Associativity assoc;
	OperatorType type;
	Processor::Opcode vmOp;
	bool toPrimitive; // FIX : almost true everywhere, drop and make exception for '!' ?
};

// FIX : sort these two
const OperatorInfo PRE_OPS[] = {
	{ "(", ")", GROUP_PREC, RIGHT_TO_LEFT, GROUP, Processor::INVALID_OP },
	{ "++", 0, PREFIX_PREC, RIGHT_TO_LEFT, PRE_INC_DEC, Processor::INC_OP, true },
	{ "--", 0, PREFIX_PREC, RIGHT_TO_LEFT, PRE_INC_DEC, Processor::DEC_OP, true },
	{ "+", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::PLUS_OP, true },
	{ "-", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::MINUS_OP, true },
	{ "~", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::INV_OP, true },
	{ "!", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::NOT_OP },
	{ "void", 0, PREFIX_PREC, RIGHT_TO_LEFT, VOID_OPERATOR, Processor::VOID_OP },
	{ "delete", 0, PREFIX_PREC, RIGHT_TO_LEFT, DELETE_OPERATOR, Processor::VOID_OP },
	{ "new", 0, NEW_PREC, RIGHT_TO_LEFT, NEW_OPERATOR, Processor::VOID_OP },
	{ "typeof", 0, PREFIX_PREC, RIGHT_TO_LEFT, TYPE_OF, Processor::TYPEOF_OP }
};

const OperatorInfo POST_OPS[] = {
	{ "===", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, Processor::X_EQ_OP },
	{ "==", 0, EQUALITY_PREC, LEFT_TO_RIGHT, ABSTRACT_EQUAL, Processor::EQ_OP },
	{ "=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::INVALID_OP },
	{ ">>>=", 0, SHIFT_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::USHR_OP, true },
	{ ">>=", 0, SHIFT_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::SHR_OP, true },
	{ "<<=", 0, SHIFT_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::SHL_OP, true },
	{ "+=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::ADD_OP, true },
	{ "-=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::SUB_OP, true },
	{ "*=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::MUL_OP, true },
	{ "/=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::DIV_OP, true },
	{ "%=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::MOD_OP, true },
	{ "&=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::AND_OP, true },
	{ "^=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::XOR_OP, true },
	{ "|=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::OR_OP, true },
	{ ">>>", 0, SHIFT_PREC, LEFT_TO_RIGHT, BINARY, Processor::USHR_OP, true },
	{ ">>", 0, SHIFT_PREC, LEFT_TO_RIGHT, BINARY, Processor::SHR_OP, true },
	{ "<<", 0, SHIFT_PREC, LEFT_TO_RIGHT, BINARY, Processor::SHL_OP, true },
	{ "++", 0, POST_INC_DEC_PREC, LEFT_TO_RIGHT, POST_INC_DEC, Processor::INC_OP, true },
	{ "--", 0, POST_INC_DEC_PREC, LEFT_TO_RIGHT, POST_INC_DEC, Processor::DEC_OP, true },
	{ "&&", 0, LOGICAL_AND_PREC, LEFT_TO_RIGHT, LOGICAL_AND_OR, Processor::JF_OP },
	{ "||", 0, LOGICAL_OR_PREC, LEFT_TO_RIGHT, LOGICAL_AND_OR, Processor::JT_OP },
	{ "+", 0, ADD_SUB_PREC, LEFT_TO_RIGHT, BINARY, Processor::ADD_OP, true },
	{ "-", 0, ADD_SUB_PREC, LEFT_TO_RIGHT, BINARY, Processor::SUB_OP, true },
	{ "*", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, Processor::MUL_OP, true },
	{ "/", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, Processor::DIV_OP, true },
	{ "%", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, Processor::MOD_OP, true },
	{ "&", 0, BIT_AND_PREC, LEFT_TO_RIGHT, BINARY, Processor::AND_OP, true },
	{ "^", 0, BIT_XOR_PREC, LEFT_TO_RIGHT, BINARY, Processor::XOR_OP, true },
	{ "|", 0, BIT_OR_PREC, LEFT_TO_RIGHT, BINARY, Processor::OR_OP, true },
	{ "!==", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, Processor::X_NEQ_OP },
	{ "!=", 0, EQUALITY_PREC, LEFT_TO_RIGHT, ABSTRACT_EQUAL, Processor::NEQ_OP },
	{ "<=", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::LEQ_OP, true },
	{ "<", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::LT_OP, true },
	{ ">=", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::GEQ_OP, true },
	{ ">", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::GT_OP, true },
	{ ".", 0, PROPERTY_PREC, LEFT_TO_RIGHT, PROPERTY_DOT, Processor::INVALID_OP },
	{ "[", "]", PROPERTY_PREC, LEFT_TO_RIGHT, PROPERTY_BRACKETS, Processor::INVALID_OP },
	{ "?", ":", CONDITIONAL_PREC, RIGHT_TO_LEFT, CONDITIONAL, Processor::INVALID_OP },
	{ "(", ")", FUNCTION_CALL_PREC, LEFT_TO_RIGHT, FUNCTION_CALL, Processor::INVALID_OP },
	{ ",", 0, COMMA_PREC, LEFT_TO_RIGHT, COMMA, Processor::INVALID_OP },
	{ "instanceof", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::INSTANCE_OF_OP, false },
	{ "in", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, IN_OPERATOR, Processor::IN_OP, true }	// Important: must be last
};

// FIX : sort
enum Statement {
	VAR_STATEMENT, IF_STATEMENT, WHILE_STATEMENT, DO_WHILE_STATEMENT, FOR_STATEMENT, RETURN_STATEMENT
	, CONTINUE_STATEMENT, BREAK_STATEMENT, TRY_STATEMENT, THROW_STATEMENT, SWITCH_STATEMENT, FUNCTION_STATEMENT
	, WITH_STATEMENT
};
enum Literal { NULL_LITERAL, FALSE_LITERAL, TRUE_LITERAL, FUNCTION_LITERAL, THIS_LITERAL };

struct Compiler::Scope {
	enum Type { LABEL_TYPE, ITERATOR_LABEL_TYPE, TRY_TYPE, CATCH_TYPE, FINALLY_TYPE, WITH_TYPE };
	Scope(Heap* heap, Type type, Scope* next) : breaks(heap), continues(heap), finallys(heap), type(type), next(next) { }
	Scope(Heap* heap, const String& label, Scope* next) : breaks(heap), continues(heap), finallys(heap), type(LABEL_TYPE), label(label), next(next) { }
	void makeIteratorScopes(Scope* untilScope) {
		for (Scope* s = this; s != untilScope; s = s->next) s->type = ITERATOR_LABEL_TYPE;
	}
	bool needToPopFrame() const {
		return (type == CATCH_TYPE || type == FINALLY_TYPE || type == WITH_TYPE);
	}
	Type type;
	String label; // empty for automatic while, for, case labels
	Scope* next;
// FIX : feels like a waste with three different vectors here, could we have double Scope entries for continuable and breakable scope types?
	QuickVector<Int32> breaks; // source points for break jmp's
	QuickVector<Int32> continues; // source points for continue jmp's
	QuickVector<Int32> finallys; // source points for finally jsr's
};
				
struct Compiler::ExpressionResult {
	enum Type { NONE, PUSHED, CONSTANT, INDEXED, NAMED, PROPERTY };
	ExpressionResult() : t(NONE), v(Value::UNDEFINED) { }
	ExpressionResult(Type type, const Value& value = Value::UNDEFINED) : t(type), v(value) { }
	Type t;
	Value v;
};

Compiler::Compiler(GCHeap& heap, CodeDefinition* codeDef, Target compileFor) : heap(heap), codeDef(codeDef)
		, compilingFor(compileFor), setupCode(&heap), outputCode(&codeDef->code), acceptInOperator(true), withScopeDepth(0) {
	for (UInt32 i = 0; i < codeDef->constants->size(); ++i) { // FIX : ugly hack to setup the reverse lookup, should share somehow
		definedConstants[(*codeDef->constants)[i]] = i;
	}
}

void Compiler::error(ErrorType errorType, const char* message) {
	throw Exception(heap, Value(heap.manage(new (&heap) Error(errorType, heap.manage(new (&heap) String(message))))));
}

void Compiler::emit(Processor::Opcode op) {
	outputCode->append(op);
}

static Int32& accessCodeInt32(QuickVector<CodeWord>& code, UInt32 offset) {
	return *reinterpret_cast<Int32*>(&code[offset]);
}

void Compiler::emitWithInt32(Processor::Opcode op, Int32 i) {
	outputCode->append(op);
	outputCode->resize(outputCode->size() + OPERAND_SIZE);
	accessCodeInt32(*outputCode, outputCode->size() - OPERAND_SIZE) = i;
}

Int32 Compiler::emitForwardBranch(Processor::Opcode op) {
	outputCode->append(op);
	Int32 point = outputCode->size();
	outputCode->resize(outputCode->size() + OPERAND_SIZE);
	return point;
}

void Compiler::completeForwardBranch(Int32 point) {
	accessCodeInt32(*outputCode, point) = outputCode->size() - point - OPERAND_SIZE;
}

Int32 Compiler::markBackwardBranch() { return outputCode->size(); }

void Compiler::emitBackwardBranch(Processor::Opcode op, Int32 point) {
	emitWithInt32(op, point - (outputCode->size() + OPERAND_SIZE + 1));
}

int Compiler::addConstant(const Value& constant) {
	std::map<Value, Int32>::const_iterator it = definedConstants.find(constant);
	if (it == definedConstants.end()) {
		Int32 index = codeDef->constants->size();
		codeDef->constants->append(constant);
		definedConstants[constant] = index;
		return index;
	} else {
		return it->second;
	}
}

void Compiler::emitWithConstant(Processor::Opcode op, const Value& constant) {
	emitWithInt32(op, addConstant(constant));
}

bool Compiler::isLeadingIdentifierChar(Char c) {
/* TODO:
IdentifierStart

UnicodeLetter
any character in the Unicode categories “Uppercase letter (Lu)”, “Lowercase letter (Ll)”, “Titlecase letter (Lt)”,
“Modifier letter (Lm)”, “Other letter (Lo)”, or “Letter number (Nl)”.

\u HexDigit HexDigit HexDigit HexDigit

+ IdentifierPart:

UnicodeCombiningMark See 7.6
any character in the Unicode categories “Non-spacing mark (Mn)” or “Combining spacing mark (Mc)”
UnicodeDigit See 7.6
any character in the Unicode category “Decimal number (Nd)”
UnicodeConnectorPunctuation See 7.6
any character in the Unicode category “Connector punctuation (Pc)”

*/
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '$' || c == '_');
}

static const Char LINE_TERMINATORS[] = { '\n', 0x2028, 0x2029, '\r' }; // FIX : '\r' must be last for line count to work, but I think we should change line counting algo in the future, a single '\r' without '\n' should count too really

static bool isLineTerminator(Char c) {
	return (std::find(LINE_TERMINATORS, LINE_TERMINATORS + 4, c) != LINE_TERMINATORS + 4);
}

static bool lineTerminatorInRange(const Char* b, const Char* e) {
	return (std::find_first_of(b, e, LINE_TERMINATORS, LINE_TERMINATORS + 4) != e);
}

void Compiler::white() {
	while (!eof()) {
		switch (*p) {
			case ' ': case '\f': case '\n': case '\r': case '\t': case '\v': case 0xA0: case 0x2028: case 0x2029: ++p; break;
			case '/':	if (p + 1 != e && p[1] == '/') {
							p = std::find_first_of(p += 2, e, LINE_TERMINATORS, LINE_TERMINATORS + 4);
							break;
						} else if (p + 1 != e && p[1] == '*') {
							static const Char END_CHARS[] = { '*', '/' };
							p += 2;
							p = std::search(p, e, END_CHARS, END_CHARS + 2);
							if (eof()) {
								error(SYNTAX_ERROR, "Missing */");
							}
							p += 2;
							break;
						}
			default:	return;
		}
	}
}

bool Compiler::whiteNoLF() {
	const Char* b = p;
	white();
	if (lineTerminatorInRange(b, p)) {
		p = b;
		return false;
	}
	return true;
}

bool Compiler::token(const char* t, bool eatLeadingWhite) {
	assert(*t != 0);
	const Char* b = p;
	if (eatLeadingWhite) {
		white();
	}
	while (*t != 0 && !eof() && *p == *t) {
		++p;
		++t;
	}
	if (*t == 0 && (!isLeadingIdentifierChar(t[-1]) || (eof() || !isLeadingIdentifierChar(*p)))) {
		return true;
	} else {
		p = b;
		return false;
	}
}

void Compiler::token() {
	while (!eof() && (isLeadingIdentifierChar(*p) || (*p >= '0' && *p <= '9'))) {
		++p;
	}
}

const Char* Compiler::identifier(bool required, bool acceptReservedKeywords) {
	const Char* b = p;
	if (!eof() && isLeadingIdentifierChar(*p)) {
		++p;
		token();
		if (!acceptReservedKeywords && findReservedKeyword(p - b, b) >= 0) {
			error(SYNTAX_ERROR, "Illegal use of reserved keyword");
		}
	}
	if (required && p == b) {
		error(SYNTAX_ERROR, "Expected identifier");
	}
	return b;
}

static UInt32 unescapedMaxLength(const Char* p, const Char* e) {
	assert(p <= e && (*p == '"' || *p == '\''));
	Char endChar = *p;
	UInt32 l = 0;
	while (++p < e && *p != endChar && !isLineTerminator(*p)) {
		if (*p == '\\' && p + 1 < e) {
			++p;
		}
		++l;
	}
	return l;
}

static const Char ESCAPE_CHARS[] = { '\\', '\"', '\'', 'b',  'f',  'n',  'r',  't',  'v', '0' };
static const Char ESCAPE_CODES[] = { '\\', '\"', '\'', '\b', '\f', '\n', '\r', '\t', '\v', '\0' };
const int ESCAPE_CODE_COUNT = sizeof (ESCAPE_CHARS) / sizeof (*ESCAPE_CHARS);

Char* Compiler::unescape(Char* buffer, const Char* e) {	
	assert(p <= e && (*p == '"' || *p == '\''));
	Char endChar = *p;
	++p;
	const Char* b = p;
	Char* d = buffer;
	while (!eof() && *p != endChar && !isLineTerminator(*p)) {
		if (*p == '\\' && p + 1 < e) {
			UInt32 l = 0;
			d = std::copy(b, p, d);
			++p;
			const Char* f = std::find(ESCAPE_CHARS, ESCAPE_CHARS + ESCAPE_CODE_COUNT, *p);
			if (f != ESCAPE_CHARS + ESCAPE_CODE_COUNT) {
				l = ESCAPE_CODES[f - ESCAPE_CHARS];
			} else if (*p == 'x' || *p == 'u') {
				const int n = (*p == 'x' ? 2 : 4);
				if (e - (p + 1) < n || parseHex(p + 1, p + 1 + n, l) != p + 1 + n) {
					error(SYNTAX_ERROR, "Illegal escape sequence");
				}
				p += n;
			} else if (isLineTerminator(*p)) {
				error(SYNTAX_ERROR, "\\ continuation is not supported");
			} else {
				l = *p;
			}
			b = ++p;
			*d++ = Char(l);
		} else {
			++p;
		}
	}
	if (eof() || *p != endChar) {
		error(SYNTAX_ERROR, "Unterminated string");
	}
	d = std::copy(b, p, d);
	++p;
	return d;
}

void Compiler::maybePrimitive(bool doEmit, bool preferString) {
	if (doEmit) {
		emit(preferString ? Processor::OBJ_TO_STRING_OP : Processor::OBJ_TO_NUMBER_OP);
	}
}

Compiler::ExpressionResult Compiler::makeRValue(const ExpressionResult& xr, bool toPrimitive, bool preferString) {
	switch (xr.t) {
		case ExpressionResult::PUSHED: {
			maybePrimitive(toPrimitive, preferString);
			return xr;
		}
		
		case ExpressionResult::NONE: {
			emit(Processor::VOID_OP);
			break;
		}
		
		case ExpressionResult::CONSTANT: {
			emitWithConstant(Processor::CONST_OP, xr.v);
			break;
		}
		
		case ExpressionResult::INDEXED: {
			emitWithInt32(Processor::READ_INDEXED_OP, xr.v.toInt());
			maybePrimitive(toPrimitive, preferString);
			break;
		}
		
		case ExpressionResult::NAMED: {
			emitWithConstant(Processor::READ_NAMED_OP, xr.v);
			maybePrimitive(toPrimitive, preferString);
			break;
		}
		
		case ExpressionResult::PROPERTY: {
			emit(Processor::GET_PROPERTY_OP);
			maybePrimitive(toPrimitive, preferString);
			break;
		}
		
		default: assert(0);
	}
	return ExpressionResult(ExpressionResult::PUSHED);
}

Compiler::ExpressionResult Compiler::safeKeep(const ExpressionResult& xr) {
	switch (xr.t) {
		case ExpressionResult::PUSHED:
		case ExpressionResult::INDEXED:
		case ExpressionResult::NAMED:
		case ExpressionResult::PROPERTY: {
			makeRValue(xr);
			Int32 tempLocalIndex;
			if (!codeDef->temporaries.empty()) {
				tempLocalIndex = codeDef->temporaries[codeDef->temporaries.size() - 1];
				codeDef->temporaries.resize(codeDef->temporaries.size() - 1);
			} else {
				codeDef->varNames.append(0);
				tempLocalIndex = -static_cast<Int32>(codeDef->varNames.size());
			}
			emitWithInt32(Processor::WRITE_INDEXED_OP, tempLocalIndex);
			emit(Processor::POP_OP);
			return ExpressionResult(ExpressionResult::INDEXED, Value(tempLocalIndex));
		}
		default: return xr;
	}
}

// FIX : why don't we make a special expression result type for safe kept value and put this one in discard / makeRValue ?
void Compiler::returnSafeKept(const ExpressionResult& xr, bool doReset) {
	if (xr.t == ExpressionResult::INDEXED) {
		Int32 index = xr.v.toInt();
		if (doReset) {
			emit(Processor::VOID_OP);
			emitWithInt32(Processor::WRITE_INDEXED_OP, index);
			emit(Processor::POP_OP);
		}
		codeDef->temporaries.append(index);
	}
}

Compiler::ExpressionResult Compiler::makeAssignment(const ExpressionResult& xr) {
	switch (xr.t) {
		default: error(REFERENCE_ERROR, "Illegal l-value"); // FIX : ECMA like the word "reference"
		case ExpressionResult::INDEXED: emitWithInt32(Processor::WRITE_INDEXED_OP, xr.v.toInt()); break;
		case ExpressionResult::NAMED: emitWithConstant(Processor::WRITE_NAMED_OP, xr.v); break;
		case ExpressionResult::PROPERTY: emit(Processor::SET_PROPERTY_OP); break;
	}
	return ExpressionResult(ExpressionResult::PUSHED);
}

Compiler::ExpressionResult Compiler::discard(const ExpressionResult& xr) {
	switch (xr.t) {
		case ExpressionResult::PUSHED: emit(Processor::POP_OP); break;
		case ExpressionResult::NAMED: emitWithConstant(Processor::READ_NAMED_OP, xr.v); emit(Processor::POP_OP); break;
		case ExpressionResult::PROPERTY: emit(Processor::GET_PROPERTY_OP); emit(Processor::POP_OP); break;
		default: break;
	}
	return ExpressionResult(ExpressionResult::NONE);
}

void Compiler::expectToken(const char* token, bool eatLeadingWhite) {
	if (!Compiler::token(token, eatLeadingWhite)) {
		error(SYNTAX_ERROR, (std::string("Expected '") + token + "'").c_str());
	}
}

Compiler::ExpressionResult Compiler::operand(const OperatorInfo& op) {
	ExpressionResult rr(expression(op.closeToken != 0 ? LOWEST_PREC : op.prec));
	if (op.closeToken != 0) {
		expectToken(op.closeToken, true);
	}
	return rr;
}

Compiler::ExpressionResult Compiler::functionCall(Processor::Opcode op) {
	Int32 count = 0;
	white();
	while (!token(")", false)) {
		if (eof()) {
			error(SYNTAX_ERROR, count == 0 ? "Expected ')'" : "Expected ',' or ')'");
		}
		if (count != 0) {
			if (!token(",", false)) {
				error(SYNTAX_ERROR, "Expected ',' or ')'");
			}
			white();
		}
		rvalueExpression(COMMA_PREC);
		white();
		++count;
	}
	emitWithInt32(op, count);
	return ExpressionResult(ExpressionResult::PUSHED);
}

Value Compiler::stringOrNumberConstant() {
	switch (*p) {
		case '0': {
			if (p + 1 == e || (p[1] != '.' && !isLeadingIdentifierChar(p[1]))) {
				++p;
				return Value(0);
			} else if (p[1] == 'x' || p[1] == 'X') {
				UInt32 u;
				const Char* q = parseHex(p + 2, e, u);
				if (q != p + 2) {
					p = q;
					return Value(u);
				}
				break;
			} else if (p[1] != '.' && p[1] != 'e' && p[1] != 'E') {
				break;
			}
			/* fall through */
		}
		case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '.': {
			double d;
			const Char* q = parseDouble(p, e, d);
			if (q != p && (q == e || !isLeadingIdentifierChar(*q))) {
				p = q;
				return Value(d);
			}
			break;
		}
		case '\'': case '"': {
			UInt32 maxLength = unescapedMaxLength(p, e);
			QuickArray<Char, 64> buffer(maxLength, &heap);
			Char* unescapeEnd = unescape(buffer, e);
			assert(unescapeEnd <= buffer + maxLength);
			return Value(String::newHashedString(heap, buffer, unescapeEnd));
		}
	}
	return Value();
}

Compiler::ExpressionResult Compiler::arrayInitialiser() { // FIX : share stuff with functionCall and arrayInitialiser
	emit(Processor::NEW_ARRAY_OP);
	
	Int32 index = 0;
	Int32 lastAddedIndex = 0;
	while (!token("]", true)) {
		Compiler::ExpressionResult xr;
		if (optionalExpression(xr, COMMA_PREC)) {
			makeRValue(xr);
			emitWithConstant(Processor::ADD_PROPERTY_OP, Value(index));
			lastAddedIndex = index;
			if (token("]", true)) {
				break;
			}
		}
		if (!token(",", true)) {
			error(SYNTAX_ERROR, "Expected ',' or ']'");
		}
		++index;
	}
	if (index != lastAddedIndex) {
		emitWithConstant(Processor::CONST_OP, Value(index));
		emitWithConstant(Processor::ADD_PROPERTY_OP, Value(&LENGTH_STRING));
	}
	return ExpressionResult(ExpressionResult::PUSHED);
}

Compiler::ExpressionResult Compiler::objectInitialiser() { // FIX : share stuff with functionCall and arrayInitialiser
	emit(Processor::NEW_OBJECT_OP);
	
	white();
	while (!token("}", false)) {
		const Char* b = p;
		Value key = stringOrNumberConstant();
		if (p == b) {
			b = identifier(false, true);
			if (p == b) {
				error(SYNTAX_ERROR, "Expected property name");
			} else {
				key = Value(String::newHashedString(heap, b, p));
			}
		}
		expectToken(":", true);
		rvalueExpression(COMMA_PREC);
		emitWithConstant(Processor::ADD_PROPERTY_OP, key);
		if (token("}", true)) {
			break;
		}
		if (!token(",", true)) {
			error(SYNTAX_ERROR, "Expected ',' or '}'");
		}
		white();
	}
	return ExpressionResult(ExpressionResult::PUSHED);
}

// FIX : split into pre and post operators anyhow, all pre discards xr now
bool Compiler::operate(ExpressionResult& xr, Precedence precedence, int opCount, const OperatorInfo* opList) {
	const Char* b = p;
    white();
	if (eof()) {
		return false;
	}
	// FIX : use a quick hash solution
	int i = 0;
	while (i < opCount && !token(opList[i].token, false)) {
		++i;
	}
	if (i >= opCount) return false;
	const OperatorInfo& op = opList[i];
	if (precedence < op.prec + (op.assoc == RIGHT_TO_LEFT ? 0 : 1)) {
		p = b;
		return false;
	}
	// FIX : sort
	switch (op.type) {
		case COMMA: discard(xr); xr = makeRValue(operand(op)); break;
		case VOID_OPERATOR: discard(xr); xr = discard(operand(op)); break;
		case GROUP: discard(xr); xr = operand(op); break;
		
		case UNARY: {
			discard(xr);
			xr = makeRValue(operand(op), op.toPrimitive);
			emit(op.vmOp);
			break;
		}
		
		case BINARY: {
			makeRValue(xr, op.toPrimitive);
			xr = makeRValue(operand(op), op.toPrimitive);
			emit(op.vmOp);
			break;
		}
		
		case ABSTRACT_EQUAL: {
			makeRValue(xr, false);
			xr = makeRValue(operand(op), false);
			emit(Processor::PRE_EQ_OP);
			emit(op.vmOp);
			break;
		}

		case IN_OPERATOR: {
			makeRValue(xr, true, true);
			xr = makeRValue(operand(op));
			emit(op.vmOp);
			break;
		}

		case POST_INC_DEC: {
			if (lineTerminatorInRange(b, p)) {
				p = b;
				return false;
			}
			if (xr.t == ExpressionResult::PROPERTY) {
				emit(Processor::REPUSH_2_OP);
			}
			makeRValue(xr, true);
			emit(Processor::PLUS_OP);
			emit(xr.t == ExpressionResult::PROPERTY ? Processor::POST_SHUFFLE_OP : Processor::REPUSH_OP);
			emit(op.vmOp);
			xr = makeAssignment(xr);
			emit(Processor::POP_OP);
			break;
		}
		
		case LOGICAL_AND_OR: {
			xr = makeRValue(xr);
			emit(Processor::REPUSH_OP);
			Int32 point = emitForwardBranch(op.vmOp);
			emit(Processor::POP_OP);
			makeRValue(operand(op));
			completeForwardBranch(point);
			break;
		}
		
		case CONDITIONAL: {
			xr = makeRValue(xr);
			Int32 falsePoint = emitForwardBranch(Processor::JF_OP);
			makeRValue(operand(op));
			Int32 endPoint = emitForwardBranch(Processor::JMP_OP);
			completeForwardBranch(falsePoint);
			xr = rvalueExpression(op.prec);
			completeForwardBranch(endPoint);
			break;			
		}
		
		case PROPERTY_DOT: {
			xr = makeRValue(xr);
			white();
			const Char* b = identifier(true, true);
			emitWithConstant(Processor::CONST_OP, Value(String::newHashedString(heap, b, p)));
			xr = ExpressionResult(ExpressionResult::PROPERTY);
			break;
		}
		
		case FUNCTION_CALL: {
			Processor::Opcode op = Processor::CALL_OP;
			if (xr.t == ExpressionResult::NAMED && xr.v.isEqualTo(EVAL_STRING)) op = Processor::CALL_EVAL_OP;
			if (xr.t == ExpressionResult::PROPERTY) op = Processor::CALL_METHOD_OP;
			else xr = makeRValue(xr);
			xr = functionCall(op);
			break;
		}
		
		case NEW_OPERATOR: {
			discard(xr);
			makeRValue(operand(op));
			if (!token("(", true)) {
				emitWithInt32(Processor::NEW_OP, 0);
				xr = ExpressionResult(ExpressionResult::PUSHED);
			} else {
				white();
				xr = functionCall(Processor::NEW_OP);
			}
			emit(Processor::NEW_RESULT_OP); // FIX : make a chain for script constructors and require returning object for native constructors instead?
			break;
		}
		
		case DELETE_OPERATOR: {
			discard(xr);
			xr = operand(op);
			switch (xr.t) {
				case ExpressionResult::PUSHED: emit(Processor::POP_OP); /* fall through */
				case ExpressionResult::NONE:
				case ExpressionResult::CONSTANT: xr = ExpressionResult(ExpressionResult::CONSTANT, Value(true)); break;
				case ExpressionResult::INDEXED: xr = ExpressionResult(ExpressionResult::CONSTANT, Value(false)); break;
				case ExpressionResult::NAMED: emit(Processor::FRAME_OP); emitWithConstant(Processor::CONST_OP, xr.v);  /* fall through */
				case ExpressionResult::PROPERTY: emit(Processor::DELETE_OP); xr = ExpressionResult(ExpressionResult::PUSHED); break;
				default: assert(0);
			}
			break;
		}
		
		case PRE_INC_DEC: {
			discard(xr);
			xr = operand(op);
			if (xr.t == ExpressionResult::PROPERTY) {
				emit(Processor::REPUSH_2_OP);
			}
			makeRValue(xr, true);
			emit(op.vmOp);
			xr = makeAssignment(xr);
			break;
		}
		
		case TYPE_OF: {
			discard(xr);
			xr = operand(op);
			if (xr.t == ExpressionResult::NAMED) {
				emitWithConstant(Processor::TYPEOF_NAMED_OP, xr.v);
				xr = ExpressionResult(ExpressionResult::PUSHED);
			} else {
				xr = makeRValue(xr);
				emit(op.vmOp);
			}
			break;
		}
		
		case ASSIGNMENT: {
			if (op.vmOp != Processor::INVALID_OP) { // FIX : sep type
				if (xr.t == ExpressionResult::PROPERTY) {
					emit(Processor::REPUSH_2_OP);
				}
				makeRValue(xr, op.toPrimitive);
				makeRValue(operand(op), op.toPrimitive);
				emit(op.vmOp);
			} else {
				makeRValue(operand(op), op.toPrimitive);
			}
			xr = makeAssignment(xr);
			break;
		}
		
		case PROPERTY_BRACKETS: {
			xr = makeRValue(xr);
			makeRValue(operand(op), true, true); // FIX : convert to string const if const, or actually if we allow Value properties and optimize array[double] we shouldn't
			xr = ExpressionResult(ExpressionResult::PROPERTY);
			break;
		}
		
		default: assert(0);
	}
	return true;
}

void Compiler::functionDefinition(const String* functionName, const String* selfName) {
	assert(functionName != 0);
	CodeDefinition* func = heap.manage(new (&heap) CodeDefinition(&heap, codeDef->constants));
	Compiler funcCompiler(heap, func, Compiler::FOR_FUNCTION);
	heap.protect(&funcCompiler);
	try {
		p = funcCompiler.compileFunction(p, e, functionName, selfName);
	}
	catch (const Exception&) { // FIX : a bit Q & D ish
		p = funcCompiler.getStopPoint();
		throw;
	}
	emitWithConstant(Processor::GEN_FUNC_OP, Value(func));
}

bool Compiler::optionalExpression(ExpressionResult& xr, Precedence precedence) {
	if (!operate(xr, precedence, sizeof (PRE_OPS) / sizeof (*PRE_OPS), PRE_OPS)) {
		if (eof()) {
			return false;
		}
		const Char* b = p;
		token();
		int i = findLiteralToken(p - b, &*b);
		if (i >= 0) {
			discard(xr);
			switch (static_cast<Literal>(i)) {
				case NULL_LITERAL: xr = ExpressionResult(ExpressionResult::CONSTANT, Value::NUL); break;
				case FALSE_LITERAL: xr = ExpressionResult(ExpressionResult::CONSTANT, Value(false)); break;
				case TRUE_LITERAL: xr = ExpressionResult(ExpressionResult::CONSTANT, Value(true)); break;
				case THIS_LITERAL: {
					emit(Processor::THIS_OP);
					xr = ExpressionResult(ExpressionResult::PUSHED); break;
					break;
				}
				case FUNCTION_LITERAL: {
					white();
					const Char* b = identifier(false, false);
					const String* name = String::newHashedString(heap, b, p);
					functionDefinition(name, name);
					xr = ExpressionResult(ExpressionResult::PUSHED);
					break;
				}
				default: assert(0);
			}
		} else {
			p = b;
			assert(!eof());
			Value v = stringOrNumberConstant();
			if (p != b) {
				discard(xr);
				xr = ExpressionResult(ExpressionResult::CONSTANT, v);
			} else {
				switch (*p) {
					case '{': {
						++p;
						discard(xr);
						xr = objectInitialiser();
						break;
					}
					
					case '[': {
						++p;
						discard(xr);
						xr = arrayInitialiser();
						break;
					}
					
					case '/': {
						while (++p != e && !isLineTerminator(*p) && *p != '/') {
							if (*p == '\\' && p + 1 != e && !isLineTerminator(p[1])) {
								++p;
							}
						}
						if (p == e || *p != '/') {
							error(SYNTAX_ERROR, "Unterminated regular expression");
						} else {
							discard(xr);
							const String* regExpSource = String::newHashedString(heap, b + 1, p);
							emitWithConstant(Processor::CONST_OP, Value(regExpSource));
							++p;
							const Char* b = p;
							token();
							const String* regExpFlags = String::newHashedString(heap, b, p);
							emitWithConstant(Processor::CONST_OP, Value(regExpFlags));
							emit(Processor::NEW_REG_EXP_OP);
							xr = ExpressionResult(ExpressionResult::PUSHED);
						}
						break;
					}
					
					default: {
						const Char* b = identifier(false, false);
						if (p == b) {
							return false;
						}
						discard(xr);
						const String* name = String::newHashedString(heap, b, p);
						xr = ExpressionResult(ExpressionResult::NAMED, Value(name));
						 // Notice: compilingFor != FOR_EVAL is only for non-strict mode (if we ever support strict-mode in the future)
						if (compilingFor != FOR_EVAL && withScopeDepth == 0) {
							const Table::Bucket* bucket = codeDef->nameIndexes.lookup(name);
							if (bucket != 0) {
								Value index = bucket->getValue();
								if (!index.isUndefined()) {
									xr = ExpressionResult(ExpressionResult::INDEXED, index);
								}
							}
						}
						break;
					}
				}
			}
		}
    }
	const Char* b = p;
    while (operate(xr, precedence, sizeof (POST_OPS) / sizeof (*POST_OPS) - (acceptInOperator ? 0 : 1), POST_OPS)) {
		b = p;
	}
	p = b;
	return true;
}

Compiler::ExpressionResult Compiler::expression(Precedence precedence) {
	ExpressionResult xr;
	if (!optionalExpression(xr, precedence)) {
		error(SYNTAX_ERROR, "Missing / invalid expression");
	}
	return xr;
}

Compiler::ExpressionResult Compiler::rvalueExpression(Precedence precedence) {
	return makeRValue(expression(precedence));
}

/*
	Normally statements discard expression results, but when compiling for eval, statements should return results.
	flushXR is required before any conditional branching and before branch targets to produce the result on the
	vstack in case it has not already been pushed (in other words if it is not an rvalue, e.g. a constant).
*/
void Compiler::flushXR(ExpressionResult& evalXR) {
	if (compilingFor == FOR_EVAL) {
		evalXR = makeRValue(evalXR);
	}
}

// FIX : ok, this is serious mess
Compiler::ExpressionResult Compiler::declareIdentifier(const String* name, bool func) {
	ExpressionResult lxr(ExpressionResult::NAMED, Value(name));
	if (compilingFor == FOR_EVAL) {
		QuickVector<CodeWord>* const outputCodeWas = outputCode;
		outputCode = &setupCode;
		if (!func) {
			emit(Processor::VOID_OP);
		}
		emitWithConstant(Processor::DECLARE_OP, Value(name));
		outputCode = outputCodeWas;
		return lxr;
	} else if (!name->isEqualTo(ARGUMENTS_STRING)) {
		Value index;
		Table& nameIndexes = codeDef->nameIndexes;
		Table::Bucket* bucket = nameIndexes.lookup(name);
		if (bucket != 0) {
			index = bucket->getValue();
		} else {
			codeDef->varNames.append(name);
			index = Value(-static_cast<Int32>(codeDef->varNames.size()));
			nameIndexes.update(nameIndexes.insert(name), index);
		}
		if (withScopeDepth == 0 && !index.isUndefined()) {
			lxr = ExpressionResult(ExpressionResult::INDEXED, index);
		}
	}
	if (func) {
		makeAssignment(lxr);
		emit(Processor::POP_OP);
	}
	return lxr;
}

Compiler::ExpressionResult Compiler::varDeclaration() {
	const Char* b = identifier(true, false);
	ExpressionResult lxr(declareIdentifier(String::newHashedString(heap, b, p), false));
	if (token("=", true)) {
		rvalueExpression(COMMA_PREC);
		makeAssignment(lxr);
		emit(Processor::POP_OP);
	}
	return lxr;
}

void Compiler::varDeclarations() {
	while (true) {
		varDeclaration();
		if (!token(",", true)) {
			break;
		}
		white();
	}
}

void Compiler::completeContinues(const Scope* fromScope, const Scope* untilScope) {
	for (const Scope* s = fromScope; s != untilScope; s = s->next) {
		for (const Int32* it = s->continues.begin(); it != s->continues.end(); ++it) {
			completeForwardBranch(*it);
		}
	}
}

void Compiler::completeBreaks(const Scope* scope) {
	for (const Int32* it = scope->breaks.begin(); it != scope->breaks.end(); ++it) {
		completeForwardBranch(*it);
	}
}

void Compiler::forInStatement(Scope* emptyLabelScope, Scope* scopeLabelsEnd, const QuickVector<CodeWord>& iterCode
		, const ExpressionResult& iterXR, ExpressionResult& evalXR) {
	ExpressionResult enumXR(rvalueExpression());
	emit(Processor::GET_ENUMERATOR_OP);
	enumXR = safeKeep(enumXR);
	expectToken(")", true);
	Int32 loopPoint = markBackwardBranch();
	makeRValue(enumXR);
	Int32 exitLoopPoint = emitForwardBranch(Processor::NEXT_PROPERTY_OP);
	ExpressionResult xr(ExpressionResult::PUSHED);
	if (!iterCode.empty()) { // This is probably extremely rare (left hand side being anything but a directly assignable variable), so I don't mind the extra code of saving away here
		xr = safeKeep(xr);
		outputCode->insert(outputCode->end(), iterCode.begin(), iterCode.end());
		makeRValue(xr);
		returnSafeKept(xr, true);
	}
	discard(makeAssignment(iterXR));
	statement(emptyLabelScope, emptyLabelScope, evalXR);
	flushXR(evalXR);
	completeContinues(emptyLabelScope, scopeLabelsEnd);
	emitBackwardBranch(Processor::JMP_OP, loopPoint);
	completeForwardBranch(exitLoopPoint);
	completeBreaks(emptyLabelScope);
	returnSafeKept(enumXR, true);
}

void Compiler::forStatement(Scope* emptyLabelScope, Scope* scopeLabelsEnd, ExpressionResult& evalXR) {
	expectToken(";", true);
	Int32 loopPoint = markBackwardBranch();
	Int32 exitLoopPoint = INVALID_CODE_POINT;
	ExpressionResult testXR;
	if (optionalExpression(testXR, LOWEST_PREC)) {
		testXR = makeRValue(testXR);
		exitLoopPoint = emitForwardBranch(Processor::JF_OP);
	}
	expectToken(";", true);
	QuickVector<CodeWord> incCode(&heap);
	outputCode = &incCode;
	ExpressionResult incXR;
	if (optionalExpression(incXR, LOWEST_PREC)) {
		discard(incXR);
	}
	expectToken(")", true);
	outputCode = &codeDef->code;
	statement(emptyLabelScope, emptyLabelScope, evalXR);
	flushXR(evalXR);
	completeContinues(emptyLabelScope, scopeLabelsEnd);
	outputCode->insert(outputCode->end(), incCode.begin(), incCode.end());
	emitBackwardBranch(Processor::JMP_OP, loopPoint);
	if (exitLoopPoint != INVALID_CODE_POINT) {
		completeForwardBranch(exitLoopPoint);
	}
	completeBreaks(emptyLabelScope);
}

void Compiler::semicolon(bool acceptLF) {
	const Char* b = p;
	white();
	if (!eof() && *p != '}' && !token(";", false) && (!acceptLF || !lineTerminatorInRange(b, p))) {
		p = b;
		error(SYNTAX_ERROR, "Syntax error");
	}
}

// FIX : make member functions of all the different statements

void Compiler::functionStatement() {
	const Char* b = identifier(true, false);
	const String* name = String::newHashedString(heap, b, p);
	QuickVector<CodeWord>* const outputCodeWas = outputCode;
	outputCode = &setupCode;
	functionDefinition(name);
	declareIdentifier(name, true);
	outputCode = outputCodeWas;
}

void Compiler::withStatement(Scope* currentScope, ExpressionResult& evalXR) {
	expectToken("(", false);
	rvalueExpression();
	expectToken(")", true);
	emit(Processor::PUSH_FRAME_OP);
	++withScopeDepth;
	Scope withScope(&heap, Scope::WITH_TYPE, currentScope);
	statement(&withScope, &withScope, evalXR);
	--withScopeDepth;
	emit(Processor::POP_FRAME_OP);
}

/*
	'scopeLabelsEnd' points to the end of a range of scopes that are all labelled scopes. This is required
	because it is legal to give while loops etc mulitple alternative labels to be used with 'continue'.
	(These should all be converted to 'ITERATOR_LABEL_TYPE' upon entry.)
*/
void Compiler::statement(Scope* currentScope, Scope* scopeLabelsEnd, ExpressionResult& evalXR) {
	white();
	
	if (token("{", false)) {
		statementList(currentScope, evalXR);
		expectToken("}", false);
		return;
	}
	
	const Char* b = p;
	token();
	int statementTokenIndex = findStatementToken(p - b, &*b);
	if (statementTokenIndex < 0) {
		p = b;
		identifier(false, true);
		const Char* q = p;
		if (p != b && token(":", true) && findReservedKeyword(q - b, b) < 0) {
			String label(b, q, &heap);
			for (Scope* s = currentScope; s != 0; s = s->next) {
				if (!s->label.empty() && s->label.isEqualTo(label)) {
					error(SYNTAX_ERROR, "Duplicate label");
				}
			}
			Scope newLabelScope(&heap, label, currentScope);
			statement(&newLabelScope, scopeLabelsEnd, evalXR);
			completeBreaks(&newLabelScope);
		} else {
			p = b;
			optionalExpression(evalXR, LOWEST_PREC);
			if (compilingFor != FOR_EVAL) {
				evalXR = discard(evalXR);
			}
			semicolon();
		}
	} else {
		switch (static_cast<Statement>(statementTokenIndex)) {
			case BREAK_STATEMENT:
			case CONTINUE_STATEMENT: {
				flushXR(evalXR);
				bool isBreak = (statementTokenIndex == BREAK_STATEMENT);
				Scope* s = currentScope;
				const Char* b = (whiteNoLF() ? identifier(false, false) : p);
				String label = String(b, p, &heap);
				// FIX : too complicated!! bone out...
				while (s != 0 && !((s->type == Scope::ITERATOR_LABEL_TYPE
						|| ((isBreak || !label.empty()) && s->type == Scope::LABEL_TYPE))
						&& s->label.isEqualTo(label))) {
					if (s->needToPopFrame()) {
						emit(Processor::POP_FRAME_OP);
					}
					if (s->type == Scope::CATCH_TYPE || s->type == Scope::TRY_TYPE) { // FIX : share with return
						s->finallys.append(emitForwardBranch(Processor::JSR_OP));
					}
					s = s->next;
				}
				if (s == 0) {
					error(SYNTAX_ERROR, !label.empty() ? "Undefined label"
							: (isBreak ? "Illegal break" : "Illegal continue"));
				}
				if (!isBreak && s->type != Scope::ITERATOR_LABEL_TYPE)
					error(SYNTAX_ERROR, "Illegal label for continue");
				(isBreak ? s->breaks : s->continues).append(emitForwardBranch(Processor::JMP_OP));
				semicolon();
				break;
			}
						
			case VAR_STATEMENT: {
				white();
				varDeclarations();
				semicolon();
				break;
			}
			
			case IF_STATEMENT: {
				white();
				flushXR(evalXR);
				expectToken("(", false);
				rvalueExpression();
				expectToken(")", true);
				Int32 falsePoint = emitForwardBranch(Processor::JF_OP);
				statement(currentScope, currentScope, evalXR);
				if (token("else", true)) {
					Int32 endPoint = emitForwardBranch(Processor::JMP_OP);
					completeForwardBranch(falsePoint);
					statement(currentScope, currentScope, evalXR);
					completeForwardBranch(endPoint);
				} else {
					flushXR(evalXR);
					completeForwardBranch(falsePoint);
				}				
				break;
			}

			case WHILE_STATEMENT: {
				white();
				flushXR(evalXR);
				Int32 loopPoint = markBackwardBranch();
				Scope emptyLabelScope(&heap, EMPTY_STRING, currentScope);
				emptyLabelScope.makeIteratorScopes(scopeLabelsEnd);
				expectToken("(", false); // FIX : sub of this
				rvalueExpression();
				expectToken(")", true);
				Int32 exitLoopPoint = emitForwardBranch(Processor::JF_OP);
				statement(&emptyLabelScope, &emptyLabelScope, evalXR);
				flushXR(evalXR);
				completeContinues(&emptyLabelScope, scopeLabelsEnd);
				emitBackwardBranch(Processor::JMP_OP, loopPoint);
				completeForwardBranch(exitLoopPoint);
				completeBreaks(&emptyLabelScope);
				break;
			}

			case DO_WHILE_STATEMENT: {
				white();
				flushXR(evalXR);
				Int32 loopPoint = markBackwardBranch();
				Scope emptyLabelScope(&heap, EMPTY_STRING, currentScope);
				emptyLabelScope.makeIteratorScopes(scopeLabelsEnd);
				statement(&emptyLabelScope, &emptyLabelScope, evalXR);
				flushXR(evalXR);
				completeContinues(&emptyLabelScope, scopeLabelsEnd);
				expectToken("while", true);
				expectToken("(", true);
				rvalueExpression();
				expectToken(")", true);
				emitBackwardBranch(Processor::JT_OP, loopPoint);
				semicolon();
				completeBreaks(&emptyLabelScope);
				break;
			}
			
			case FOR_STATEMENT: {
				white();
				flushXR(evalXR);
				Scope emptyLabelScope(&heap, EMPTY_STRING, currentScope);
				emptyLabelScope.makeIteratorScopes(scopeLabelsEnd);
				expectToken("(", false);
				QuickVector<CodeWord> initCode(&heap);
				if (token("var", true)) {
					white();
					acceptInOperator = false;
					ExpressionResult iterXR = varDeclaration();
					acceptInOperator = true;
					if (token("in", true)) {
						forInStatement(&emptyLabelScope, scopeLabelsEnd, initCode, iterXR, evalXR);
					} else {
						if (token(",", true)) {
							white();
							acceptInOperator = false;
							varDeclarations();
							acceptInOperator = true;
						}
						forStatement(&emptyLabelScope, scopeLabelsEnd, evalXR);
					}
				} else {
					QuickVector<CodeWord>* const outputCodeWas = outputCode;
					outputCode = &initCode;
					acceptInOperator = false;
					ExpressionResult iterXR;
					bool gotExpression = optionalExpression(iterXR, LOWEST_PREC);
					acceptInOperator = true;
					outputCode = outputCodeWas;
					if (gotExpression && token("in", true)) {
						forInStatement(&emptyLabelScope, scopeLabelsEnd, initCode, iterXR, evalXR);
					} else {
						outputCode->insert(outputCode->end(), initCode.begin(), initCode.end());
						discard(iterXR);
						forStatement(&emptyLabelScope, scopeLabelsEnd, evalXR);
					}
				}
				break;
			}
			
			case RETURN_STATEMENT: {
				if (compilingFor != FOR_FUNCTION) {
					error(SYNTAX_ERROR, "Illegal return");
				}
				ExpressionResult returnXR;
				if (whiteNoLF()) {
					optionalExpression(returnXR, LOWEST_PREC);
				}
				Scope* s = currentScope;
				bool safeKept = false;
				while (s != 0) { // FIX : sub
					if (s->type == Scope::CATCH_TYPE || s->type == Scope::TRY_TYPE || s->type == Scope::WITH_TYPE) {
						if (!safeKept) {
							returnXR = safeKeep(returnXR);
							safeKept = true;
						}
					}
					if (s->needToPopFrame()) {
						emit(Processor::POP_FRAME_OP);
					}
					if (s->type == Scope::CATCH_TYPE || s->type == Scope::TRY_TYPE) {
						s->finallys.append(emitForwardBranch(Processor::JSR_OP));
					}
					s = s->next;
				}
				makeRValue(returnXR);
				if (safeKept) {
					returnSafeKept(returnXR, false);
				}
				emit(Processor::RETURN_OP);
				semicolon();
				break;
			}
			
			case TRY_STATEMENT: {
				white();
				flushXR(evalXR);
				Scope tryScope(&heap, Scope::TRY_TYPE, currentScope);
				Int32 catchPoint = emitForwardBranch(Processor::TRY_OP);
				expectToken("{", true); // FIX : sub for block
				statementList(&tryScope, evalXR);
				expectToken("}", false);
				flushXR(evalXR);
				tryScope.finallys.append(emitForwardBranch(Processor::JSR_OP));
				Int32 tryEndPoint = emitForwardBranch(Processor::JMP_OP);
				Int32 catchEndPoint = INVALID_CODE_POINT;
				
				// Catcher
				
				bool gotCatch = false;
				if (token("catch", true)) {
					gotCatch = true;
					expectToken("(", true);
					white();
					const Char* b = identifier(true, false);
					const String* exceptionVarName = String::newHashedString(heap, b, p);
					Value previousLocalIndex;
					Table& nameIndexes = codeDef->nameIndexes;
					bool didExist = false; // FIX : can use NON_EXISTENT type if we choose to keep it
					Table::Bucket* indexBucket = nameIndexes.lookup(exceptionVarName);
					if (indexBucket != 0) {
						didExist = true;
						previousLocalIndex = indexBucket->getValue();
					} else {
						indexBucket = nameIndexes.insert(exceptionVarName);
					}
					nameIndexes.update(indexBucket, Value::UNDEFINED); // prohibit further var accesses or declarations to the capture parameter via fast index binding.
					completeForwardBranch(catchPoint);
					catchPoint = emitForwardBranch(Processor::TRY_OP);
					emitWithConstant(Processor::CATCH_FRAME_OP, Value(exceptionVarName));
					expectToken(")", true);
					Scope catchScope(&heap, Scope::CATCH_TYPE, currentScope);
					expectToken("{", true); // FIX : sub
					statementList(&catchScope, evalXR);
					expectToken("}", false);
					flushXR(evalXR);
					emit(Processor::POP_FRAME_OP); // FIX : try to move POP_FRAME to first in FINALLY block, need different JSR then
					catchScope.finallys.append(emitForwardBranch(Processor::JSR_OP));
					catchEndPoint = emitForwardBranch(Processor::JMP_OP);
					for (const Int32* it = catchScope.finallys.begin(); it != catchScope.finallys.end(); ++it) { // FIX : sub
						completeForwardBranch(*it);
					}
					if (didExist) {
						nameIndexes.update(nameIndexes.insert(exceptionVarName), previousLocalIndex);
					} else {
						nameIndexes.erase(nameIndexes.lookup(exceptionVarName));
					}
				}
				
				// Finally
				
				for (const Int32* it = tryScope.finallys.begin(); it != tryScope.finallys.end(); ++it) // FIX : sub
					completeForwardBranch(*it);
				emit(Processor::TRIED_OP);
				Int32 rethrowPoint = markBackwardBranch();
				if (token("finally", true)) {
					Scope finallyScope(&heap, Scope::FINALLY_TYPE, currentScope);
					expectToken("{", true); // FIX : sub
					statementList(&finallyScope, evalXR);
					expectToken("}", false);
					flushXR(evalXR);
				} else {
					if (!gotCatch) {
						error(SYNTAX_ERROR, "Expected 'catch' or 'finally'");
					}
				}
				emit(Processor::RETURN_OP);
				
				// Re-thrower (FIX : could actually be eliminated if we only have catch and no finally)
				
				completeForwardBranch(catchPoint);
				ExpressionResult thrownXR(ExpressionResult::PUSHED);
				thrownXR = safeKeep(thrownXR);
				emitBackwardBranch(Processor::JSR_OP, rethrowPoint);
				makeRValue(thrownXR);
				returnSafeKept(thrownXR, true);
				emit(Processor::THROW_OP);
				completeForwardBranch(tryEndPoint);
				if (catchEndPoint != INVALID_CODE_POINT) {
					completeForwardBranch(catchEndPoint);
				}
				break;
			}
			
			case THROW_STATEMENT: {
				if (!whiteNoLF()) {
					error(SYNTAX_ERROR, "Illegal newline after throw");
				}
				rvalueExpression();
				emit(Processor::THROW_OP);
				semicolon();
				break;
			}
			
			case SWITCH_STATEMENT: {
				white();
				flushXR(evalXR);
				expectToken("(", true);
				ExpressionResult controlXR(safeKeep(expression(LOWEST_PREC)));
				expectToken(")", true);
				expectToken("{", true);
				Int32 testsPoint = emitForwardBranch(Processor::JMP_OP);
				QuickVector<CodeWord> testsCode(&heap);
				QuickVector<Int32> jumpOffsets(&heap);
				Int32 startOffset = outputCode->size();
				Int32 defaultPoint = INVALID_CODE_POINT;
				Scope emptyLabelScope(&heap, EMPTY_STRING, currentScope);
				bool firstStatement = true;
				white();
				while (!token("}", false)) {
					if (eof()) {
						error(SYNTAX_ERROR, "Expected '}'");
					}
					else if (token("case", false)) {
						flushXR(evalXR);
						QuickVector<CodeWord>* const outputCodeWas = outputCode;
						outputCode = &testsCode;
						makeRValue(controlXR);
						rvalueExpression();
						expectToken(":", true);
						emit(Processor::X_EQ_OP);
						emitWithInt32(Processor::JT_OP, outputCodeWas->size() - startOffset - testsCode.size() - OPERAND_SIZE - 1);
						jumpOffsets.append(testsCode.size() - OPERAND_SIZE);
						outputCode = outputCodeWas;
					} else if (token("default", false)) {
						expectToken(":", true);
						if (defaultPoint != INVALID_CODE_POINT) error(SYNTAX_ERROR, "More than one 'default:'");
						flushXR(evalXR);
						defaultPoint = markBackwardBranch();
					} else if (firstStatement) {
						error(SYNTAX_ERROR, "Expected 'case', 'default' or '}'");
					} else {
						const Char* b = p;
						statement(&emptyLabelScope, &emptyLabelScope, evalXR);
						if (b == p) error(SYNTAX_ERROR, "Expected '}'");
					}
					firstStatement = false;
					white();
				}
				
				flushXR(evalXR);
				Int32 donePoint = emitForwardBranch(Processor::JMP_OP);
				completeForwardBranch(testsPoint);
				for (const Int32* it = jumpOffsets.begin(); it != jumpOffsets.end(); ++it) {
					accessCodeInt32(testsCode, *it) -= outputCode->size() - startOffset;
				}
				outputCode->insert(outputCode->end(), testsCode.begin(), testsCode.end());
				if (defaultPoint != INVALID_CODE_POINT) emitBackwardBranch(Processor::JMP_OP, defaultPoint);
				completeForwardBranch(donePoint);
				completeBreaks(&emptyLabelScope);
				returnSafeKept(controlXR, true);
				break;
			}
			
			case FUNCTION_STATEMENT: {
				white();
				functionStatement();
				break;
			}
			
			case WITH_STATEMENT: {
				white();
				withStatement(currentScope, evalXR);
				break;
			}
			
			default: assert(0);
		}
	}
}

void Compiler::statementList(Scope* currentScope, ExpressionResult& evalXR) {
	const Char* b;
	do {
		b = p;
		statement(currentScope, currentScope, evalXR);
	} while (!eof() && b != p);
}

const Char* Compiler::compile(const Char* b, const Char* e) {
	codeDef->code.resize(0); // FIX : what is the rule, one or many compiles?
	outputCode = &codeDef->code;
	p = b;
	this->e = e;
	acceptInOperator = true;

	ExpressionResult evalXR;
	statementList(0, evalXR);
	if (compilingFor != FOR_EVAL) {
		evalXR = discard(evalXR);
	}
	makeRValue(evalXR);
	emit(Processor::RETURN_OP);
	if (!setupCode.empty()) {
		outputCode->insert(outputCode->begin(), setupCode.begin(), setupCode.end());
	}
	// FIX : shrink code vector (copy to quickarray instead?)
	assert(outputCode == &codeDef->code);
	codeDef->code.shrink();
	codeDef->constants->shrink();
if (!supressStdErr) {
	disassemble(heap, *codeDef);
	std::cerr << "\tCode: " << codeDef->code.size() << std::endl;
	std::cerr << "\tConstants: " << codeDef->constants->size() << std::endl;
/* FIX :	for (const Value* p = codeDef->constants.begin(); p != codeDef->constants.end(); ++p) {
		std::wcerr << "\t\t" << p->toString(heap)->toStdString() << std::endl;
	}*/
	std::cerr << "\tVars: " << codeDef->getVarsCount() << std::endl;
	std::cerr << "\tArguments: " << codeDef->getArgumentsCount() << std::endl;
}
	return p;
}

const Char* Compiler::compileFunction(const Char* b, const Char* e, const String* functionName, const String* selfName) {
	assert(compilingFor == FOR_FUNCTION);  // FIX : messy, why do we have compilerFor if we separate this anyhow? Maybe subclass Compiler instead?
	assert(functionName != 0);
	p = b;
	this->e = e;
	expectToken("(", true);
	white();
	Table& nameIndexes = codeDef->nameIndexes;
	QuickVector<const String*>& argumentNames = codeDef->argumentNames;
	while (!token(")", false)) {
		if (eof()) {
			error(SYNTAX_ERROR, argumentNames.size() == 0 ? "Expected ')'" : "Expected ',' or ')'");
		}
		if (argumentNames.size() != 0) {
			if (!token(",", false)) {
				error(SYNTAX_ERROR, "Expected ',' or ')'");
			}
			white();
		}
		const Char* b = identifier(true, false);
		const String* name = String::newHashedString(heap, b, p);
		nameIndexes.update(nameIndexes.insert(name), Value(static_cast<Int32>(argumentNames.size())));
		argumentNames.append(name);
		white();
	}
	expectToken("{", true);
	compile(p, e); // FIX: ugly as it sets p and e again, although it doesn't hurt
	expectToken("}", false);
	codeDef->name = functionName;
	codeDef->selfName = selfName;
	codeDef->source = heap.manage(new (&heap) String(String(FUNCTION_SPACE, *functionName, &heap), String(b, p, &heap), &heap)); // FIX : operator+ would make things clearer :)
	return p;
}

void Compiler::compile(const String& source) {
	compile(source.begin(), source.end());
	white();
	if (!eof()) {
		error(SYNTAX_ERROR, "Syntax error"); // FIX : better error?
	}
}

GCHeap::GCHeap() : managedListA(*this), managedListB(*this), rootList(*this), currentList(&managedListA), newList(&managedListB), allocated(0) {
	std::fill(pools + 0, pools + 1024 / 16, (void*)(0));
}

void* GCHeap::allocate(size_t size) {
	void* block = 0;
	if (0 < size && size < 1024 - 15) {
		const int poolIndex = (size + 15) / 16;
		size = poolIndex * 16;
		block = pools[poolIndex];
		if (block != 0) {
			pools[poolIndex] = *(static_cast<void**>(block));
		}
	}
	if (block == 0) {
		block = ::operator new(size + sizeof (size_t));
	}
	size_t* p = reinterpret_cast<size_t*>(block);
	*p = size;
	allocated += size;
	return p + 1;
}

void GCHeap::deallocate(void *ptr) {
	size_t* p = reinterpret_cast<size_t*>(ptr) - 1;
	size_t size = *p;
	if (0 < size && size < 1024 - 15) {
		const int poolIndex = (size + 15) / 16;
		*(void **)(p) = pools[poolIndex];
		pools[poolIndex] = p;
	} else {
		::operator delete(p);
	}
	allocated -= size;
	assert(allocated >= 0);
}

void* GCHeap::allocateArray(size_t size) { return allocate(size); }
void GCHeap::deallocateArray(void *ptr) { return deallocate(ptr); }
UInt32 GCHeap::count() const { return currentList->size(); }
size_t GCHeap::size() const { return allocated; }

UInt32 GCHeap::gc() {
	UInt32 sweptCount = 0;

	for (const GCItem* item = rootList._gcNext; item != &rootList; item = item->_gcNext) {
		item->gcMark(*this);
	}
	
	{
		UInt32 n = 0;
		for (const GCItem* i = currentList->_gcNext->_gcNext; i != currentList->_gcNext; i = i->_gcNext) {
			assert(i->_gcPrev->_gcList == currentList);
			delete i->_gcPrev;
			++n;
		}

		currentList->_gcPrev = currentList->_gcNext = currentList;
		sweptCount = n;
	}

	std::swap(currentList, newList);
	return sweptCount;
}

void GCHeap::mark(const GCItem* item) {
	if (item != 0 && item->_gcList == currentList) {
		newList->claim(const_cast<GCItem*>(item));
		item->gcMark(*this);
	}
}

Value EvalFunction::call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	if (argc < 1) return Value::UNDEFINED;
	if (!argv[0].typeOfString()->isEqualTo(STRING_STRING)) return argv[0]; // FIX : better way to check type without involving typeOfString?

	GCHeap& heap = engine.getHeap();
	const String* sourceCode = argv[0].toString(heap);
	CodeDefinition* codeDef;
	
	const Table::Bucket* bucket = functionDefCache.lookup(sourceCode);
	 // FIX : .getObject?
	if (bucket != 0) {
		codeDef = reinterpret_cast<CodeDefinition*>(bucket->getValue().asObject());
	} else {
		codeDef = heap.manage(new (&heap) CodeDefinition(&heap, heap.manage(new (&heap) Constants(&heap)))); // Notice: doesn't share constants with anyone else, so they will be nicely erased when this temporary function gets purged.
		Compiler compiler(heap, codeDef, Compiler::FOR_EVAL);
		heap.protect(&compiler);
		compiler.compile(*sourceCode);
		functionDefCache.update(functionDefCache.insert(sourceCode), Value(codeDef));
	}
	processor.enterEval(codeDef, direct);
	return Value();
}

/* FIX : funny C++11 thing
#include <functional>


typedef std::function<Value (Engine&, Processor&, Int32, const Value*, Object*)> NativeFunction;

		mathObject->setOwnProperty(*this, &SQRT_STRING, newNativeFunction(gcHeap, [](Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
			return (argc >= 1 ? Value(sqrt(argv[0].toDouble())) : Value::NOT_A_NUMBER);
		}));
		
*/

typedef Value (*NativeFunction)(Engine&, Processor&, Int32, const Value*, Object*);

template<class F> struct FunctorAdapter : public Function {
	FunctorAdapter(const F& f) : f(f) { }
	virtual Value call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		return f(engine, processor, argc, argv, thisObject);
	}
	F f;
};

template<class F> struct UnaryMathFunction : public Function {
	UnaryMathFunction(const F& f) : f(f) { }
	virtual Value call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		return (argc >= 1 ? Value(f(argv[0].toDouble())) : Value::NOT_A_NUMBER);
	}
	const F f;
};

// template<class F> Value newNativeFunction(GCHeap& heap, const F& f) { return Value(heap.manage(new FunctorAdapter<F>(f))); }

struct Support {
	static Value getInternalProperty(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 2) {
			Object* o = argv[0].asObject();
			if (o != 0) {
				const String* s = argv[1].toString(engine.getHeap());
				if (s->isEqualTo(PROTOTYPE_STRING)) {
					Object* p = o->getPrototype(engine);
					return (p == 0 ? Value::NUL : Value(p));
				} else if (s->isEqualTo(CLASS_STRING)) {
					return Value(o->getClassName());
				} else if (s->isEqualTo(VALUE_STRING)) {
					return o->getInternalValue(engine.getHeap());
				}
			}
		}
		return Value::UNDEFINED;
	}

	static Value createWrapper(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 2) {
			GCHeap& heap = engine.getHeap();
			const String* const className = argv[0].toString(heap);
			const Value internalValue = argv[1];
			Object* prototype = (argc >= 3 ? argv[2].toObject(engine, false) : 0);
			if (prototype == 0) {
				prototype = &engine.objectPrototype;
			}
			Object* o = 0;
			if (className->isEqualTo(S_TRING_STRING)) {
				o = new (&heap) StringWrapper(&heap, internalValue.toString(heap), prototype);
			} else if (className->isEqualTo(E_RROR_STRING)) {
				for (int i = 0; i < ERROR_TYPE_COUNT; ++i) {
					if (prototype == engine.errorPrototypes[i]) {
						o = new (&heap) Error(static_cast<ErrorType>(i));
						break;
					}
				}
			}
			if (o == 0) {
				o = new (&heap) WrapperObject(&heap, className, internalValue, prototype);
			}
			return Value(heap.manage(o));
		}
		return Value::UNDEFINED;
	}

	static Value specialConstructor(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 1) {
			Function* regularFunction = argv[0].asFunction();
			if (regularFunction != 0) {
				Function* constructorFunction = (argc >= 2 ? argv[1].asFunction() : 0);
				return Value(engine.getHeap().manage(new (&engine.getHeap()) SpecialConstructorFunction(regularFunction, constructorFunction)));
			}
		}
		return Value::UNDEFINED;
	}

	static Value defineProperty(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		bool success = false;
		if (argc >= 2) {
			Object* o = argv[0].asObject();
			if (o != 0) {
				success = o->setOwnProperty(engine, argv[1], (argc >= 3 ? argv[2] : Value::UNDEFINED)
						, (argc >= 4 && argv[3].toBool() ? READ_ONLY_FLAG : 0)
						| (argc >= 5 && argv[4].toBool() ? DONT_ENUM_FLAG : 0)
						| (argc >= 6 && argv[5].toBool() ? DONT_DELETE_FLAG : 0));
			}
		}
		return Value(success);
	}

	static Value compileFunction(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 1) {
			GCHeap& heap = engine.getHeap();
			const String* source = argv[0].toString(heap);
			CodeDefinition* codeDef = heap.manage(new (&heap) CodeDefinition(&heap, heap.manage(new (&heap) Constants(&heap)))); // Notice: doesn't share constants with anyone else, so they will be nicely erased if this function gets deleted.
			Compiler compiler(heap, codeDef, Compiler::FOR_FUNCTION);
			heap.protect(&compiler);
			compiler.compileFunction(source->begin(), source->end(), argc >= 2 ? argv[1].toString(heap) : &ANONYMOUS_STRING);
			return Value(heap.manage(new (&heap) ScriptFunction(codeDef, engine.getGlobals())));
		}
		return Value::UNDEFINED;
	}

	static Value callWithArgs(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		GCHeap& heap = engine.getHeap();
		Function* callFunction = (argc > 0 ? argv[0].asFunction() : 0);
		if (callFunction == 0) {
			processor.error(TYPE_ERROR, heap.manage(new (&heap) String("apply / call used on non-function")));
			return Value();
		} else {
			Object* newThis = (argc > 1 ? argv[1].toObject(engine, true) : 0);
			QuickArray<Value> args(0, &heap);
			Object* arrayObject = (argc > 2 ? argv[2].toObject(engine, false) : 0);
			if (arrayObject != 0) {
				Value v;
				if (arrayObject->getPropertyDeep(engine, Value(&LENGTH_STRING), &v) != NONEXISTENT) { // FIX : in the future I think we should have a getLength
					Int32 offset = (argc > 3 ? argv[3].toInt() : 0);
					UInt32 length = static_cast<UInt32>(std::max(v.toInt() - offset, 0));
					args.resize(length);
					for (UInt32 i = 0; i < length; ++i) {
						args[i] = Value::UNDEFINED;
						arrayObject->getPropertyDeep(engine, Value(i + offset), &args[i]);
					}
				}
			}
			// FIX : we copy all arguments once to args, and then chain will copy them again to a frame object, couldn't we short-cut that somehow?
			// FIX : since only arrays and arguments are really valid here, perhaps even have a new virtual in object for implementing efficient apply with these?
			return callFunction->call(engine, processor, args.size(), args.begin(), (newThis == 0 ? engine.getGlobals() : newThis));
		}
	}
	
	static Value hasOwnProperty(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Object* object = (argc >= 2 ? argv[0].toObject(engine, false) : 0);
		return (object != 0 ? Value(object->hasOwnProperty(engine, argv[1])) : Value(false));
	}

	static Value isPropertyEnumerable(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Object* object = (argc >= 2 ? argv[0].toObject(engine, false) : 0);
		return (object != 0 ? Value(object->isOwnPropertyEnumerable(engine, argv[1])) : Value(false));
	}

	static Value fromCharCode(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Char c = static_cast<Char>(argc > 0 ? argv[0].toInt() : 0);
		GCHeap& heap = engine.getHeap();
		return ((0 <= c && c < 127) ? QUICK_CONSTANTS.ascii[c] : Value(heap.manage(new (&heap) String(&c, &c + 1, &heap))));
	}

	static Value atan2(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		return (argc >= 2 ? Value(std::atan2(argv[0].toDouble(), argv[1].toDouble())) : Value::NOT_A_NUMBER);
	}

	static Value pow(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		return (argc >= 2 ? Value(std::pow(argv[0].toDouble(), argv[1].toDouble())) : Value::NOT_A_NUMBER);
	}

	static Value parseFloat(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Value result = Value::NOT_A_NUMBER;
		if (argc >= 1) {
			const String* s = argv[0].toString(engine.getHeap());
			const Char* e = s->end();
			const Char* b = eatStringWhite(s->begin(), e);
			double d = 0.0;
			if (parseDouble(b, e, d) != b) {
				result = Value(d);
			}
		}
		return result;
	}

	static Value charCodeAt(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 2) {
			const String* s = argv[0].toString(engine.getHeap());
			const double d = nanToZero(argv[1].toDouble());
			if (-1.0 < d && d < s->size()) {
				return Value((*s)[static_cast<UInt32>(d)]);
			}
		}
		return Value::NOT_A_NUMBER;;
	}

	static Value substring(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Value result(&EMPTY_STRING);
		if (argc >= 3) {
			const String* s = argv[0].toString(engine.getHeap());
			const double l = s->size();
			const double from = std::max(nanToZero(argv[1].toDouble()), 0.0);
			const double to = std::min(nanToZero(argv[2].toDouble()), l);
			if (from < to && from < l && to >= 1.0) {
				const Char* b = s->begin();
				GCHeap& heap = engine.getHeap();
				return Value(from < 1.0 && to >= l ? s
						: heap.manage(new (&heap) String(b + static_cast<UInt32>(from), b + static_cast<UInt32>(to), &heap)));
			}
		}
		return Value(&EMPTY_STRING);
	}

	static Value submatch(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 3) {
			const String* const text = argv[0].toString(engine.getHeap());
			const Int32 offset = argv[1].toInt();
			const String* const match = argv[2].toString(engine.getHeap());
			const UInt32 matchSize = match->size();
			if (offset + matchSize <= text->size()
					&& std::equal(text->begin() + offset, text->begin() + offset + matchSize, match->begin())) {
				return Value(true);
			}
		}
		return Value(false);
	}

	static Value getCurrentTime(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		return Value(time(0) * 1000.0 + engine.unixEpochTimeDiff);
	}

	static Value localTimeDifference(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		const time_t t = (argc > 0 ? static_cast<time_t>((argv[0].toDouble() - engine.unixEpochTimeDiff) / 1000.0 + 0.5) : time(0));
		const struct tm localTM = *localtime(&t);
		const struct tm utcTM = *gmtime(&t);
		struct tm newTM;
		memset(&newTM, 0, sizeof (newTM));
		newTM.tm_year = utcTM.tm_year;
		newTM.tm_mon = utcTM.tm_mon;
		newTM.tm_mday = utcTM.tm_mday;
		newTM.tm_hour = utcTM.tm_hour;
		newTM.tm_min = utcTM.tm_min;
		newTM.tm_sec = utcTM.tm_sec;
		newTM.tm_isdst = localTM.tm_isdst;
		const time_t newTime = mktime(&newTM);
		return (newTime == -1 ? Value::NOT_A_NUMBER : Value(t * 1000.0 - newTime * 1000.0));
	}
};

static struct {
	String name;
	UnaryMathFunction<double (*)(double)> func;
} NATIVE_MATH_FUNCTIONS[] = {
	{ String("acos"), acos }, { String("asin"), asin }, { String("atan"), atan }, { String("cos"), cos },
	{ String("exp"), exp }, { String("floor"), floor }, { String("log"), log }, { String("sin"), sin },
	{ String("sqrt"), sqrt }, { String("tan"), tan }
};
// TODO : min, max, random

static struct {
	String name;
	FunctorAdapter<Value (*)(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) > func;
} SUPPORT_FUNCTIONS[] = {
	{ String("getInternalProperty"), Support::getInternalProperty }, { String("createWrapper"), Support::createWrapper },
	{ String("defineProperty"), Support::defineProperty }, { String("compileFunction"), Support::compileFunction },
	{ String("specialConstructor"), Support::specialConstructor }, { String("callWithArgs"), Support::callWithArgs },
	{ String("hasOwnProperty"), Support::hasOwnProperty }, { String("fromCharCode"), Support::fromCharCode },
	{ String("isPropertyEnumerable"), Support::isPropertyEnumerable }, { String("atan2"), Support::atan2 },
	{ String("pow"), Support::pow }, { String("parseFloat"), Support::parseFloat },
	{ String("charCodeAt"), Support::charCodeAt }, { String("substring"), Support::substring },
	{ String("submatch"), Support::submatch }, { String("getCurrentTime"), Support::getCurrentTime }
	, { String("localTimeDifference"), Support::localTimeDifference }
};

static UnaryMathFunction<bool (*)(double)> IS_NAN_FUNCTION(isNaN);
static UnaryMathFunction<bool (*)(double)> IS_FINITE_FUNCTION(isFinite);

Engine::Engine(GCHeap& heap, Frame* globals) : gcHeap(heap), globals(globals), objectPrototype(&heap, 0), evalFunction(&heap, false)
		, evalDirectFunction(&heap, true), toNumberFunction(0), toStringFunction(0), createRegExpFunction(0)
		, booleanPrototype(&heap, &B_OOLEAN_STRING, Value(false), &objectPrototype)
		, numberPrototype(&heap, &N_UMBER_STRING, Value(0.0), &objectPrototype)
		, stringPrototype(&heap, &S_TRING_STRING, Value(&EMPTY_STRING), &objectPrototype)
		, arrayPrototype(&heap), unixEpochTimeDiff(0.0) {
	assert(heap.belongs(globals));

	std::fill(errorPrototypes, errorPrototypes + ERROR_TYPE_COUNT, (Error*)(0));
	
	gcHeap.protect(&objectPrototype);
	gcHeap.protect(&functionPrototype);
	gcHeap.protect(&booleanPrototype);
	gcHeap.protect(&numberPrototype);
	gcHeap.protect(&stringPrototype);
	gcHeap.protect(&arrayPrototype);
	
	for (int i = 0; i < ERROR_TYPE_COUNT; ++i) {
		errorPrototypes[i] = gcHeap.protect(new (&gcHeap) ErrorPrototype(static_cast<ErrorType>(i)));
	}
	
	globals->setOwnProperty(*this, Value(&EVAL_STRING), Value(gcHeap.protect(&evalFunction)), DONT_ENUM_FLAG);
	gcHeap.protect(&evalDirectFunction);
}

void Engine::setupStandardLibrary(const String& librarySourceCode) {
	{	// time() is not guaranteed to return utc time, so we perform some conversions to find the actual implementation
		struct tm refTM;
		memset(&refTM, 0, sizeof (refTM));
		refTM.tm_year = 80;
		refTM.tm_mday = 1;
		time_t refTime = mktime(&refTM);
		time_t refTimeAsUTC = mktime(gmtime(&refTime));
		assert(refTime != -1);
		assert(refTimeAsUTC != -1);
		unixEpochTimeDiff = 315532800000.0 - refTime * 2000.0 + refTimeAsUTC * 1000.0;
	}
	
	ScriptObject* supportObject = gcHeap.protect(new (&gcHeap) ScriptObject(&gcHeap, &objectPrototype));
	// FIX : no new String?
	ScriptObject* prototypesObject = gcHeap.manage(new (&gcHeap) ScriptObject(&gcHeap, &objectPrototype));
	supportObject->setOwnProperty(*this, Value(gcHeap.manage(new (&gcHeap) String("prototypes"))), Value(prototypesObject), STANDARD_FLAGS);
	for (int i = 0; i < sizeof (SUPPORT_FUNCTIONS) / sizeof (*SUPPORT_FUNCTIONS); ++i) {
		supportObject->setOwnProperty(*this, Value(&SUPPORT_FUNCTIONS[i].name), Value(&SUPPORT_FUNCTIONS[i].func), STANDARD_FLAGS);
	}
	supportObject->setOwnProperty(*this, Value(gcHeap.manage(new (&gcHeap) String("isNaN"))), Value(&IS_NAN_FUNCTION), STANDARD_FLAGS);
	supportObject->setOwnProperty(*this, Value(gcHeap.manage(new (&gcHeap) String("isFinite"))), Value(&IS_FINITE_FUNCTION), STANDARD_FLAGS);
	supportObject->setOwnProperty(*this, Value(&UNDEFINED_STRING), Value::UNDEFINED, STANDARD_FLAGS);
	supportObject->setOwnProperty(*this, Value(&NAN_STRING), Value::NOT_A_NUMBER, STANDARD_FLAGS);
	supportObject->setOwnProperty(*this, Value(&INFINITY_STRING), Value::INFINITE_NUMBER, STANDARD_FLAGS);
	supportObject->setOwnProperty(*this, Value(gcHeap.manage(new (&gcHeap) String("maxNumber"))), Value(std::numeric_limits<double>::max()), STANDARD_FLAGS);
	supportObject->setOwnProperty(*this, Value(gcHeap.manage(new (&gcHeap) String("minNumber"))), Value(std::numeric_limits<double>::denorm_min()), STANDARD_FLAGS);
	
	for (int i = 0; i < sizeof (NATIVE_MATH_FUNCTIONS) / sizeof (*NATIVE_MATH_FUNCTIONS); ++i) {
		supportObject->setOwnProperty(*this, Value(&NATIVE_MATH_FUNCTIONS[i].name), Value(&NATIVE_MATH_FUNCTIONS[i].func), STANDARD_FLAGS);
	}
	
	struct { const String* name; Object* object; } PROTOTYPES[] = {
		{ &O_BJECT_STRING, &objectPrototype }, { &F_UNCTION_STRING, &functionPrototype },
		{ &S_TRING_STRING, &stringPrototype }, { &B_OOLEAN_STRING, &booleanPrototype },
		{ &N_UMBER_STRING, &numberPrototype }, { &A_RRAY_STRING, &arrayPrototype }
	};
	for (int i = 0; i < sizeof (PROTOTYPES) / sizeof (*PROTOTYPES); ++i) {
		prototypesObject->setOwnProperty(*this, Value(PROTOTYPES[i].name), Value(PROTOTYPES[i].object), STANDARD_FLAGS);
	}
	for (int i = 0; i < ERROR_TYPE_COUNT; ++i) {
		prototypesObject->setOwnProperty(*this, Value(&ERROR_NAMES[i]), Value(errorPrototypes[i]), STANDARD_FLAGS);
	}
	// FIX : all this boilerplating... build a function object or something instead?

	CodeDefinition* code = gcHeap.manage(new (&gcHeap) CodeDefinition(&gcHeap, gcHeap.manage(new (&gcHeap) Constants(&gcHeap))));
	{
		Compiler compiler(gcHeap, code, Compiler::FOR_EVAL);
		gcHeap.protect(&compiler);
		compiler.compile(librarySourceCode);
	}
	{
		Value vstack[128];
		Processor::CStackEntry cstack[128];
		Processor processor(*this, cstack, cstack + 128, vstack, vstack + 128);
		gcHeap.protect(&processor);
		const CodeWord RETURN_TO_HOST_STUB[] = { Processor::RETURN_TO_HOST_OP };
		processor.chain(RETURN_TO_HOST_STUB);
++processor.vsp; // FIX!!!
		processor.enterEval(code, false);
		bool needsMoreTime = processor.run();
		assert(!needsMoreTime);
		Value v = processor.getResult();
		Function* f = v.asFunction();
		if (f != 0) {
			processor.chain(RETURN_TO_HOST_STUB);
			Value argv[1] = { Value(supportObject) };
++processor.vsp; // FIX!!!
			f->call(*this, processor, 1, argv, getGlobals());
			needsMoreTime = processor.run();
			assert(!needsMoreTime);
		}
	}
	gcHeap.gc();

	// FIX : sub v
	toNumberFunction = supportObject->getPropertyDeep(*this, "toPrimitiveNumber").asFunction();
	if (toNumberFunction == 0) {
	// FIX : all these gcHeap.manage(new String()) ... could be a utility function!
		throw Exception(gcHeap, Value(gcHeap.manage(new (&gcHeap) Error(GENERIC_ERROR, gcHeap.manage(new (&gcHeap) String("Standard library is missing support.toPrimitiveNumber"))))));
	}
	
	toStringFunction = supportObject->getPropertyDeep(*this, "toPrimitiveString").asFunction();
	if (toStringFunction == 0) {
		throw Exception(gcHeap, Value(gcHeap.manage(new (&gcHeap) Error(GENERIC_ERROR, gcHeap.manage(new (&gcHeap) String("Standard library is missing support.toPrimitiveString"))))));
	}

	createRegExpFunction = supportObject->getPropertyDeep(*this, "createRegExp").asFunction();
	if (toStringFunction == 0) {
		throw Exception(gcHeap, Value(gcHeap.manage(new (&gcHeap) Error(GENERIC_ERROR, gcHeap.manage(new (&gcHeap) String("Standard library is missing support.createRegExp"))))));
	}
}

#include <fstream>
#include <memory>

	bool interactive = true;

	std::vector<std::string> ioLines;

	void pushIOLines(char typeChar, const String& s) {
		const Char* p = s.begin();
		const Char* e = s.end();
		do {
			const Char* b = p;
			while (p != e && *p != '\n') {
				++p;
			}
			ioLines.push_back(typeChar + std::string(" ") + std::string(b, p));
			if (p != e) {
				++p;
			}
		} while (p != e);
	}

	void pushIOStop() {
		ioLines.push_back("-");
	}

	class PrintFunction : public Function {
		public:		typedef Function super;
					virtual Value call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
						const String* s = (argc >= 1 ? argv[0].toString(engine.getHeap()) : &EMPTY_STRING);
						std::wcout << s->toStdString().c_str() << std::endl;
						if (interactive) {
							pushIOLines('<', *s);
						}
						return Value::UNDEFINED;
					}
	};
	
	struct GCFunction : public Function {
		virtual Value call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
			UInt32 sweptCount = engine.getHeap().gc();
			return Value(static_cast<Int32>(sweptCount));
		}
	};

int main(int argc, const char* argv[]) {
/*std::cout << sizeof(Value) << std::endl;
std::cout << sizeof(String) << std::endl;
std::cout << sizeof(ScriptObject) << std::endl;
std::cout << sizeof(Array) << std::endl;
std::cout << sizeof(ScriptFunction) << std::endl;*/
supressStdErr = true;
	const String LF_STRING("\n");
	const String PRINT_STRING("print");
/*std::cerr << "Value size: " << sizeof (Value) << std::endl;
std::cerr << "Bucket size: " << sizeof (Table::Bucket) << std::endl;
std::cerr << "Table size: " << sizeof (Table) << std::endl;*/
	GCHeap heap;
	Constants globalConstants(&heap);
	heap.protect(&globalConstants);
	CodeDefinition globalCode(&heap, &globalConstants);
	heap.protect(&globalCode);
	ScriptFunction globalClosure(&globalCode, 0); // FIX : yuck!
	heap.protect(&globalClosure);
	ScriptFrame globalsFrame(&heap, &globalClosure, 0, 0);
	heap.protect(&globalsFrame);
	
	Engine engine(heap, &globalsFrame);

	PrintFunction printFunction;
	globalsFrame.setOwnProperty(engine, Value(&PRINT_STRING), Value(heap.protect(&printFunction)), DONT_ENUM_FLAG);
	GCFunction gcFunction;
	const String GC_STRING("gc");
	globalsFrame.setOwnProperty(engine, Value(&GC_STRING), Value(heap.protect(&gcFunction)), DONT_ENUM_FLAG);
	
	try {
		std::wifstream stdlibStream("stdlib.js");
		String source = String(std::wstring(std::istreambuf_iterator<wchar_t>(stdlibStream), std::istreambuf_iterator<wchar_t>()), &heap);
		stdlibStream.close();
		engine.setupStandardLibrary(source);
	}
	catch (const std::exception& x) {
		std::cerr << "exception setting up standard lib: " << x.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "exception setting up standard lib" << std::endl;
		return 1;
	}
supressStdErr = false;
	
	String source(EMPTY_STRING);
	std::auto_ptr<std::wifstream> inputFileStream;
	std::wistream* inStream = &std::wcin;
	bool doTime = false;
	long gcRate = 256;
	bool autoGCRate = true;
	bool pauseBeforeQuit = false;
	for (int argi = 1; argi < argc; ++argi) {
		if (strcmp(argv[argi], "-t") == 0) doTime = true;
		else if (strcmp(argv[argi], "-s") == 0) supressStdErr = true;
		else if (argi + 1 < argc && strcmp(argv[argi], "-gc") == 0) {
			gcRate = atol(argv[++argi]);
			autoGCRate = false;
		}
		else if (strcmp(argv[argi], "-p") == 0) pauseBeforeQuit = true;
		else if (inputFileStream.get() == 0) {
			inputFileStream.reset(new std::wifstream(argv[argi]));
			inStream = inputFileStream.get();
			interactive = false;
		} else {
			std::cout << "Too many arguments" << std::endl;
			return 1;
		}
	}
	while (inStream->good()) {
		bool execute = false;
		try {
			std::wstring s;
			if (interactive) {
				std::getline(*inStream, s);
				// FIX : add #undo that drops just the last one
				if (s == L"#save" || s.substr(0, 6) == L"#save ") {
					const time_t t = time(0);
					char fn[257];
					if (s.size() >= 6) {
						std::wstring ws = L"tests/";
						ws += s.substr(6);
						ws += L".io";
						strncpy(fn, std::string(ws.begin(), ws.end()).c_str(), 256);
						fn[256] = 0;
					} else {
						strftime(fn, 256, "tests/%Y%m%d_%H%M%S.io", localtime(&t));
					}
					std::ofstream saveStream(fn);
					for (std::vector<std::string>::const_iterator it = ioLines.begin(); it != ioLines.end(); ++it) {
						saveStream << *it << std::endl;
					}
					saveStream.close();
					ioLines.clear();
					std::cout << "saved to " << fn << std::endl;
				} else if (s == L"#purge") {
					ioLines.clear();
					std::cout << "purged" << std::endl;
				} else if (!s.empty()) {
					String line(s, &heap);
					if (!source.empty()) source = String(source, LF_STRING, &heap);
					source = String(source, line, &heap);
				} else {
					if (source.size() > 0 && source[0] == '?') {
						source = String(String(String("print("), String(source.begin() + 1, source.end(), &heap), &heap), String(")"), &heap); // FIX : operator+
					}
					if (interactive) {
						pushIOLines('>', source);
					}
					execute = true;
				}
			} else {
				source = String(std::wstring(std::istreambuf_iterator<wchar_t>(*inStream), std::istreambuf_iterator<wchar_t>()), &heap);
				inStream->get(); // FIX : weird, seems necessary to force !inStream->good
				execute = true;
			}
			
			if (execute) {
				Value vstack[1024];
				Processor::CStackEntry cstack[1024];
				Processor processor(engine, cstack, cstack + 1024, vstack, vstack + 1024);
				heap.protect(&processor);
				const CodeWord RETURN_TO_HOST_STUB[] = { Processor::RETURN_TO_HOST_OP };
				processor.chain(RETURN_TO_HOST_STUB);
				
				Compiler compiler(heap, &globalCode, (interactive ? Compiler::FOR_EVAL : Compiler::FOR_GLOBAL));
				heap.protect(&compiler);
				try {
					compiler.compile(source);
				}
				catch (const Exception&) {
					int lineNumber = 1;
					const Char* stopPoint = compiler.getStopPoint();
					const Char* p = source.begin();
					while (p < stopPoint) {
						assert(p != source.end());
						if (std::find(LINE_TERMINATORS, LINE_TERMINATORS + 3, *p) != LINE_TERMINATORS + 3) {
							++lineNumber;
						}
						++p;
					}
					std::wstring ws;
					{
						std::wstringstream ss;
						ss << std::wstring(L"!!!! Line: ") << lineNumber;
						ws = ss.str();
					}
					std::wcout << ws << std::endl;
					if (interactive) {
						pushIOLines('!', String(ws, &heap));
					}
					throw;
				}
				globalsFrame.updatedCode();
if (!supressStdErr) {
	for (const Value* p = globalConstants.begin(); p != globalConstants.end(); ++p) {
		std::wcerr << "\t\t" << p->toString(heap)->toStdString() << std::endl;
	}
}
					processor.chain(globalCode.getCodePointer(), &globalsFrame, &globalsFrame);
				
				bool done = false;
				clock_t start = ::clock();	
				do {
					processor.resetTimeOut(gcRate);
					done = !processor.run();
					{
						long beforeSize = static_cast<long>(heap.size());
						UInt32 sweptCount = heap.gc();
						long nowSize = static_cast<long>(heap.size());;
						if (!supressStdErr) {
							std::cerr << "\tGC rate: " << gcRate << std::endl;
							std::cerr << "\tGC swept: " << sweptCount << std::endl;
							std::cerr << "\tGC kept: " << heap.count() << std::endl;
							std::cerr << "\tHeap size: " << beforeSize / (1024.0 * 1024.0) << "MiB -> " << nowSize / (1024.0 * 1024.0) << "MiB" << std::endl;
						}
						if (autoGCRate) {
							if (beforeSize - nowSize >= 128L * 1024 * 1024) {
								gcRate = 256;
							} else if (gcRate < 16 * 1024 * 1024) {
								gcRate *= 2;
							}
						}
					}
				
					if (done) {
						if (!supressStdErr) {
							std::cerr << "\tOn stack: " << (processor.vsp - vstack + 1) << std::endl;
							assert(processor.vsp - vstack + 1 == 0);
							Value v = processor.getResult();
							std::wcerr << L"\t=" << (v.toString(heap))->toStdString().c_str() << std::endl;
						}
						if (doTime) {
							clock_t end = ::clock();
							const double divisor = (CLOCKS_PER_SEC / 1000.0);
							std::cout << "\ttime: " << (static_cast<double>(end - start) / divisor) << std::endl;
						}
					}
				} while (!done);
				source = EMPTY_STRING;
			}
		}
		catch (const Exception& x) {
			source = EMPTY_STRING;
			std::wstring ws = x.getValue().toString(heap)->toStdString();
			ws = L"!!!! " + ws;
			std::wcout << ws << std::endl;
			if (!interactive) {
				return 1;
			} else {
				pushIOLines('!', String(ws, &heap));
			}
		}
		catch (const std::exception& x) {
			source = EMPTY_STRING;
			std::cout << x.what() << std::endl;
			if (!interactive) {
				return 1;
			} else {
				pushIOLines('!', String(x.what()));
			}
		}
		catch (...) {
			source = EMPTY_STRING;
			std::cout << "Unknown exception" << std::endl;
			if (!interactive) {
				return 1;
			} else {
				pushIOLines('!', String("Unknown exception"));
			}
		}
		if (execute && interactive) {
			pushIOStop();
		}
	}
	if (pauseBeforeQuit) std::wcin.get();
	return 0;
}
