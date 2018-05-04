#pragma GCC optimize ("no-finite-math-only")

#ifdef __FAST_MATH__
	#error This code requires IEEE compliant floating point handling. Avoid -Ofast / -ffast-math etc, at least for this source file.
#endif

// FIX: all below

#include "assert.h"
#include <cmath>
#include <ctime>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <iomanip>      // std::setw
#include <sstream>
#include <limits>
#include "NuXJScript.h"

	bool suppressStdErr = false; // FIX : drop

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
static bool isFinite(double d) { return !isNaN(d) && fabs(d) != std::numeric_limits<double>::infinity(); }
#endif

static double nanToZero(double d) { return (isNaN(d) ? 0.0 : d); }

/* --- String constants --- */

// FIX : sort, check all used, join to few lines, or maybe even do some smart array based construction?
const String EMPTY_STRING;
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
const String STACK_OVERFLOW_STRING("Stack overflow");
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
			asciiChars[i] = String(buf, buf + 1, 0);
			ascii[i] = Value(&asciiChars[i]);
		}
		for (int i = -QUICK_CONSTANTS_INTEGERS_RANGE; i <= QUICK_CONSTANTS_INTEGERS_RANGE; ++i) {
			Char buffer[32];
			integers[i + QUICK_CONSTANTS_INTEGERS_RANGE] = String(intToString(buffer, i), buffer + 32, 0); // FIX : newHashedString
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
static int strncmp(const Char* a, const char* b, size_t n) {
	size_t i = 0;
	while (i < n && a[i] == b[i] && a[i] != 0 && b[i] != 0) {
		++i;
	}
	return (i == n ? 0 : a[i] - b[i]);
}
/* Built with QuickHashMaker.pika */
static int findStatementToken(size_t n /* string length */, const Char* s /* string (zero termination not required) */) {
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
static int findReservedKeyword(size_t n /* string length */, const Char* s) {
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
}
/* Built with QuickHashMaker.pika */
static int findLiteralToken(size_t n /* string length */, const Char* s /* zero-terminated string */) {
	static const char* STRINGS[5] = {
		"null", "false", "true", "function", "this"
	};
	static const int QUICK_HASH_TABLE[8] = {
		0, 2, -1, 3, -1, -1, 1, 4
	};
	if (n < 4 || n > 8) return -1;
	int stringIndex = QUICK_HASH_TABLE[((n ^ s[3])) & 7];
	return (stringIndex >= 0 && strncmp(s, STRINGS[stringIndex], n) == 0 && STRINGS[stringIndex][n] == 0) ? stringIndex : -1;
}

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
		case OBJECT_TYPE: return NaN();  // Notice, you shouldn't normally call toDouble on an object as proper ToNumber requires conversion to a primitive type
		case NULL_TYPE: return 0.0;
		case BOOLEAN_TYPE: return var.boolean ? 1.0 : 0.0;
		case NUMBER_TYPE: return var.number;
		case STRING_TYPE: return stringToDouble(*var.string);
	}
}

// TODO : toInt32 and toUInt32
Int32 Value::toInt() const { // FIX : could be faster without toDouble I thinkus
	const double v = toDouble();
	const Int32 i = static_cast<Int32>(v);
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

const String* Value::toString(Heap& heap) const {
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

// FIX : requestExtensible is only used once! do we need it?
Object* Value::toObject(Runtime& rt, bool requestExtensible) const {
	Heap& heap = rt.getHeap();
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return 0;
		case NULL_TYPE: return 0;
		case BOOLEAN_TYPE: return heap.govern(new(heap) WrapperObject(heap, &B_OOLEAN_STRING, *this, &rt.booleanPrototype));
		case NUMBER_TYPE: return heap.govern(new(heap) WrapperObject(heap, &N_UMBER_STRING, *this, &rt.numberPrototype));
		// I think const_cast can be accepted here, the string is always immutable when accessed as an Object
		case STRING_TYPE: return (!requestExtensible ? static_cast<Object*>(const_cast<String*>(var.string))
				: heap.govern(new(heap) StringWrapper(heap, var.string, &rt.stringPrototype)));
		case OBJECT_TYPE: return var.object;
	}
}

bool Value::compareStrictly(const Value& r) const {
	assert(r.type == type);
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
			case STRING_TYPE: return (r.type >= BOOLEAN_TYPE && stringToDouble(*var.string)
					== (r.type == BOOLEAN_TYPE ? (r.var.boolean ? 1.0 : 0.0) : r.var.number));
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
	double thatNumber = y.toDouble();
	return thisNumber < thatNumber;
}

bool Value::isLessThanOrEqualTo(const Value& y) const {
	if (type == STRING_TYPE && y.type == STRING_TYPE) {
		return !y.var.string->isLessThan(*var.string);
	}
	double thisNumber = toDouble();
	double thatNumber = y.toDouble();
	return thisNumber <= thatNumber;
}

Value Value::add(Heap& heap, const Value& y) const {
	if (type == STRING_TYPE || y.type == STRING_TYPE) {
		const String* ls = toString(heap);
		const String* rs = y.toString(heap);
		if (rs->empty()) return Value(ls);
		if (ls->empty()) return Value(rs);
		return Value(String::concat(heap, *ls, *rs));
	}
	return Value(toDouble() + y.toDouble());
}

void Value::gcMark(Heap& heap) const {
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

String::String() : QArray(0, 0), hash(2166136261), bloom(0x10000020) {
	assert(String("").calcHash() == hash && String("").createBloomCode() == bloom);
}

String::String(const Char* b, const Char* e, Heap* heap) : QArray(static_cast<UInt32>(e - b), heap), hash(0), bloom(0) {
	std::copy(b, e, begin());
}

String::String(const String& l, const String& r, Heap* heap) : QArray(l.size() + r.size(), heap), hash(0), bloom(0) {
	Char* e = std::copy(l.begin(), l.end(), begin());
	std::copy(r.begin(), r.end(), e);
}

String::String(const std::wstring& s, Heap* heap) : QArray(static_cast<UInt32>(s.size()), heap), hash(0), bloom(0) {
	std::copy(s.begin(), s.end(), begin());
}

String::String(const char* s, Heap* heap) : QArray(static_cast<UInt32>(strlen(s)), heap), hash(0), bloom(0) {
	std::copy(s, s + size(), begin());
	calcHash();
	createBloomCode();
}

const String* String::typeOfString() const { return &STRING_STRING; }
const String* String::getClassName() const { return &S_TRING_STRING; }
const String* String::toString(Heap& heap) const { return this; }
Object* String::getPrototype(Runtime& rt) const { return &rt.stringPrototype; }

Flags String::getOwnProperty(Runtime& rt, const Value& key, Value* v) const {
	if (key.isEqualTo(LENGTH_STRING)) {
		*v = Value(static_cast<Int32>(size()));
		return (READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG | EXISTS_FLAG);
	}
	UInt32 index;
	if (key.toArrayIndex(index) && index < size()) {
		const Char* p = begin() + index;
		Heap& heap = rt.getHeap();
		*v = (*p < 127 ? QUICK_CONSTANTS.ascii[*p] : Value(heap.govern(new(heap) String(p, p + 1, &heap))));
		return (READ_ONLY_FLAG | DONT_DELETE_FLAG | EXISTS_FLAG);
	}
	return NONEXISTENT;
}

// Modified FNV Hash
UInt32 String::calcHash() const {
	if (hash == 0) {
		UInt32 x = 2166136261U;
		for (const Char* p = begin(), *e = end(); p < e; ++p) {
			x = (x << 3) | (x >> (32 - 3));
			x = (x + *p) * 16777619;
		}
		hash = (x + (x == 0 ? 1 : 0));
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

UInt32 String::createBloomCode() const {
	if (bloom == 0) {
		const UInt32 hash = calcHash();
		bloom = (static_cast<UInt32>(1) << (hash & 31)) | (static_cast<UInt32>(1) << ((hash >> 16) & 31));
	}
	return bloom;
}

Enumerator* String::getPropertyEnumerator(Runtime& rt) const {
	Heap& heap = rt.getHeap();
	return heap.govern(new(heap) RangeEnumerator(heap, 0, size()));
}

const String* String::newString(Heap& heap, const char* s) {
	return heap.govern(new(heap) String(s, &heap));
}

const String* String::newHashedString(Heap& heap, const Char* b, const Char* e) {
	if (b == e) {
		return &EMPTY_STRING;
	}
	String* s = heap.govern(new(heap) String(b, e, &heap));
	s->calcHash();
	s->createBloomCode();
	return s;
}

const String* String::concat(Heap& heap, const String& l, const String& r) {
	return heap.govern(new(heap) String(l, r, &heap));
}

const String* String::fromInt(Heap& heap, Int32 i) {
	Char buffer[32];
	return (i >= -QUICK_CONSTANTS_INTEGERS_RANGE && i <= QUICK_CONSTANTS_INTEGERS_RANGE)
		 ? &QUICK_CONSTANTS.integers[i + QUICK_CONSTANTS_INTEGERS_RANGE]
		 : heap.govern(new(heap) String(intToString(buffer, i), buffer + 32, &heap));
}

const String* String::fromDouble(Heap& heap, double d) {
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
		s = heap.govern(new(heap) String(buffer, doubleToString(buffer, d), &heap));
	}
	return s;
}

/* --- GCItem --- */

void* GCItem::operator new(size_t n, Heap& heap) {
	Heap** p = reinterpret_cast<Heap**>(::operator new(sizeof (Heap*) + n, &heap));
	*p = &heap;
	return p + 1;
}

void GCItem::operator delete(void* ptr) { Heap** p = reinterpret_cast<Heap**>(ptr) - 1; ::operator delete(p, *p); }
void GCItem::operator delete(void* ptr, Heap& heap) { ::operator delete(reinterpret_cast<Heap**>(ptr) - 1, &heap); }
GCItem::GCItem() throw() : _gcList(0), _gcPrev(this), _gcNext(this) { }
GCItem::GCItem(Heap& heap) throw() : _gcList(0), _gcPrev(this), _gcNext(this) { heap.include(*this); }
GCItem::GCItem(const GCItem& copy) throw() { *this = copy; }
GCItem& GCItem::operator=(const GCItem& copy) throw() { _gcList = 0; _gcPrev = this; _gcNext = this; if (copy._gcList != 0) copy._gcList->heap.include(*this); return *this; }
Heap* GCItem::getGCHeap() const { return (_gcList == 0 ? 0 : &_gcList->heap); }
void GCItem::gcMark(Heap& heap) const { }
GCItem::~GCItem() { if (_gcList != 0) _gcList->relinquish(this); }

/* --- GCItemList --- */

GCItemList::GCItemList(Heap& heap) : heap(heap), count(0) { }

void GCItemList::relinquish(GCItem* item) throw() {
	assert(item->_gcList == this);
	assert(count > 0);	// If this assert triggers it could mean that you've forgot to call include() on a GCItem and it has been both garbage collected and destructed (e.g. by going out of scope).
	--count;
	item->_gcPrev->_gcNext = item->_gcNext;
	item->_gcNext->_gcPrev = item->_gcPrev;
}

void GCItemList::claim(GCItem* item) throw() {
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

UInt32 GCItemList::deleteAll() throw() {
	const UInt32 n = count;
	for (const GCItem* i = _gcNext->_gcNext; i != _gcNext; i = i->_gcNext) {
		assert(i->_gcPrev->_gcList == this);
		delete i->_gcPrev;
	}
	assert(_gcPrev == _gcNext && _gcPrev == this);
	return n;
}

GCItemList::~GCItemList() {
	deleteAll();
}

/* --- Heap --- */

Heap::Heap() : allocated(0), governedListA(*this), governedListB(*this), rootList(*this), currentList(&governedListA)
		, newList(&governedListB) {
	std::fill(pooled + 0, pooled + MAX_POOLED_SIZE / POOL_SIZE_GRANULARITY, (void*)(0));
}

int Heap::calcPoolIndex(const size_t size) {
	return (0 < size && size <= MAX_POOLED_SIZE - POOL_SIZE_GRANULARITY)
			? static_cast<int>((size + (POOL_SIZE_GRANULARITY - 1)) / POOL_SIZE_GRANULARITY)
			: -1;
}

void* Heap::allocate(size_t size) {
	void* block = 0;
	const int poolIndex = calcPoolIndex(size);
	if (poolIndex >= 0) {
		size = poolIndex * POOL_SIZE_GRANULARITY;
		assert(0 <= poolIndex && poolIndex < MAX_POOLED_SIZE / POOL_SIZE_GRANULARITY);
		block = pooled[poolIndex];
		if (block != 0) {
			pooled[poolIndex] = *(static_cast<void**>(block));
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

void Heap::deallocate(void *ptr) {
	size_t* p = reinterpret_cast<size_t*>(ptr) - 1;
	size_t size = *p;
	const int poolIndex = calcPoolIndex(size);
	if (poolIndex >= 0) {
		*(void **)(p) = pooled[poolIndex];
		assert(0 <= poolIndex && poolIndex < MAX_POOLED_SIZE / POOL_SIZE_GRANULARITY);
		pooled[poolIndex] = p;
	} else {
		::operator delete(p);
	}
	allocated -= size;
	assert(allocated >= 0);
}

void* Heap::allocateArray(size_t size) { return allocate(size); }
void Heap::deallocateArray(void *ptr) { return deallocate(ptr); }
UInt32 Heap::count() const { return currentList->count; }
size_t Heap::size() const { return allocated; }

UInt32 Heap::gc() {
	for (const GCItem* item = rootList._gcNext; item != &rootList; item = item->_gcNext) {
		item->gcMark(*this);
	}
	for (const GCItem* item = newList->_gcPrev; item != newList; item = item->_gcPrev) {
		item->gcMark(*this);
	}
	std::swap(currentList, newList);
	return newList->deleteAll();
}

void Heap::purgePooled() {
	for (int poolIndex = 0; poolIndex < MAX_POOLED_SIZE / POOL_SIZE_GRANULARITY; ++poolIndex) {
		void* p = pooled[poolIndex];
		while (p != 0) {
			void* n = *(void **)(p);
			::operator delete(p);
			p = n;
		}
		pooled[poolIndex] = 0;
	}
}

Heap::~Heap() { purgePooled(); }

/* --- Object --- */

static struct EmptyEnumerator : public Enumerator { const String* nextPropertyName() { return 0; } } EMPTY_ENUMERATOR;

Function* Object::asFunction() { return 0; }
const String* Object::typeOfString() const { return &OBJECT_STRING; }
const String* Object::getClassName() const { return &O_BJECT_STRING; }
Object* Object::getPrototype(Runtime& rt) const { return &rt.objectPrototype; }
Value Object::getInternalValue(Heap& heap) const { return Value::UNDEFINED; }
Flags Object::getOwnProperty(Runtime& rt, const Value& key, Value* v) const { return NONEXISTENT; }
bool Object::setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags) { return false; }
bool Object::deleteProperty(Runtime& rt, const Value& key) { return false; }
Enumerator* Object::getPropertyEnumerator(Runtime& rt) const { return &EMPTY_ENUMERATOR; }

bool Object::updateProperty(Runtime& rt, const Value& key, const Value& v) {
	return (hasOwnProperty(rt, key) && ((void)(setOwnProperty(rt, key, v, STANDARD_FLAGS)), true));
}

bool Object::isOwnPropertyEnumerable(Runtime& rt, const Value& key) const {
	Value dummy;
	return (getOwnProperty(rt, key, &dummy) & (DONT_ENUM_FLAG | EXISTS_FLAG)) == EXISTS_FLAG;
}

const String* Object::toString(Heap& heap) const {
	return String::concat(heap, String(BRACKET_OBJECT_STRING, *getClassName(), &heap), END_BRACKET_STRING);
}

bool Object::hasOwnProperty(Runtime& rt, const Value& key) const {
	Value dummy;
	return (getOwnProperty(rt, key, &dummy) != NONEXISTENT);
}

Flags Object::getPropertyDeep(Runtime& rt, const Value& key, Value* v) const {
	const Object* o = this;
	do {
		Flags flags = o->getOwnProperty(rt, key, v);
		if (flags != NONEXISTENT) {
			return flags;
		}
		o = o->getPrototype(rt);
	} while (o != 0);
	return NONEXISTENT;
}

bool Object::setPropertyDeep(Runtime& rt, const Value& key, const Value& v) {
	if (updateProperty(rt, key, v)) {
		return true;
	}
	const Object* o = this;
	while ((o = o->getPrototype(rt)) != 0) {
		Value dummy;
		if ((o->getOwnProperty(rt, key, &dummy) & READ_ONLY_FLAG) != 0) {
			return false;
		}
	}
	return setOwnProperty(rt, key, v, STANDARD_FLAGS);
}

Value Object::getPropertyDeep(Runtime& rt, const char* name) const { // FIX : keep?
	Value v = Value::UNDEFINED;
	String s(name);
	getPropertyDeep(rt, Value(&s), &v);
	return v;
}

/* --- Table --- */

UInt32 Table::calcMaxLoad(UInt32 bucketCount) { return (bucketCount - (bucketCount >> 4) - 1); }

Table::Table(Heap* heap) : buckets(1U << TABLE_BUILT_IN_N, heap), loadCount(0) { }

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
			rebuild(buckets.size() << 2);	// expand 4 times on overload
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

void Table::update(Bucket* bucket, const Int32 index) {
	assert(bucket->keyExists());
	bucket->flags = EXISTS_FLAG | INDEX_TYPE_FLAG;
	bucket->index = index;
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

void Table::gcMark(Heap& heap) const {
	UInt32 rebuildLoadCount = 0;
	const UInt32 n = buckets.size();
	for (UInt32 i = 0; i < n; ++i) {
		const Bucket& bucket = buckets[i];
		if (bucket.keyExists()) {
			heap.mark(bucket.key);
			if (bucket.valueExists()) {
				switch (((bucket.flags & INDEX_TYPE_FLAG) != 0) ? Value::NUMBER_TYPE : bucket.type) {
					case Value::STRING_TYPE: heap.mark(bucket.var.string); break;
					case Value::OBJECT_TYPE: heap.mark(bucket.var.object); break;
					default: break;
				}
				rebuildLoadCount++;
			}
		}
	}
	if (rebuildLoadCount != loadCount) {
		UInt32 check = const_cast<Table*>(this)->rebuild(loadCount < calcMaxLoad(n >> 1) ? (n >> 1) : n);
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

UInt32 Table::rebuild(UInt32 newSize) {
	QuickArray<Bucket, 1U << TABLE_BUILT_IN_N> newBuckets(newSize, buckets.getAssociatedHeap());
	UInt32 newLoadCount = 0;
	UInt32 mask = newSize - 1;
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
	loadCount = newLoadCount;
	return newLoadCount;
}

/* --- ScriptObject --- */

ScriptObject::ScriptObject(Heap& heap, Object* prototype) : super(heap), properties(&heap), prototype(prototype) { }

Object* ScriptObject::getPrototype(Runtime& rt) const { return prototype; }
Table& ScriptObject::accessProperties() { return properties; }

bool ScriptObject::setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags) {
	return properties.update(properties.insert(key.toString(rt.getHeap())), v, flags);
}

bool ScriptObject::updateProperty(Runtime& rt, const Value& key, const Value& v) {
	Table::Bucket* bucket = properties.lookup(key.toString(rt.getHeap()));
	return (bucket != 0 && properties.update(bucket, v));
}

Flags ScriptObject::getOwnProperty(Runtime& rt, const Value& key, Value* v) const {
	const Table::Bucket* bucket = properties.lookup(key.toString(rt.getHeap()));
	if (bucket != 0) {
		*v = bucket->getValue();
		return bucket->getFlags();
	}
	return NONEXISTENT;
}

bool ScriptObject::deleteProperty(Runtime& rt, const Value& key) {
	Table::Bucket* bucket = properties.lookup(key.toString(rt.getHeap()));
	return (bucket == 0 || properties.erase(bucket));
}

static StringListEnumerator* createTableKeyList(Heap& heap, const Table& table) {
	StringListEnumerator* list = heap.govern(new(heap) StringListEnumerator(table.getLoadCount(), &heap));
	for (Table::Bucket* bucket = table.getFirst(); bucket != 0; bucket = table.getNext(bucket)) {
		if (bucket->doEnumerate()) {
			list->add(bucket->getKey());
		}
	}
	return list;
}

Enumerator* ScriptObject::getPropertyEnumerator(Runtime& rt) const {
	return createTableKeyList(rt.getHeap(), properties);
}

/* --- WrapperObject --- */

WrapperObject::WrapperObject(Heap& heap, const String* className, const Value& value, Object* prototype)
	: super(heap, prototype), className(className), wrapped(value) {
}

const String* WrapperObject::getClassName() const { return className; }
Value WrapperObject::getInternalValue(Heap& heap) const { return wrapped; }

/* --- StringWrapper --- */

StringWrapper::StringWrapper(Heap& heap, const String* string, Object* prototype) : super(heap, prototype), wrapped(string) { }
const String* StringWrapper::getClassName() const { return &S_TRING_STRING; }
const String* StringWrapper::toString(Heap& heap) const { return wrapped; }
Value StringWrapper::getInternalValue(Heap& heap) const { return Value(wrapped); }

Flags StringWrapper::getOwnProperty(Runtime& rt, const Value& key, Value* v) const {
	Flags flags = wrapped->getOwnProperty(rt, key, v);
	return (flags != NONEXISTENT ? flags : super::getOwnProperty(rt, key, v));
}

bool StringWrapper::setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags) {
	if (wrapped->hasOwnProperty(rt, key)) {
		return false;
	}
	return super::setOwnProperty(rt, key, v, flags);
}

bool StringWrapper::deleteProperty(Runtime& rt, const Value& key) {
	if (wrapped->hasOwnProperty(rt, key)) {
		return false;
	} else {
		return super::deleteProperty(rt, key);
	}
}

Enumerator* StringWrapper::getPropertyEnumerator(Runtime& rt) const {
	Heap& heap = rt.getHeap();
	return heap.govern(new(heap) JoiningEnumerator(rt, super::getPropertyEnumerator(rt), wrapped));
}

/* --- RangeEnumerator --- */

RangeEnumerator::RangeEnumerator(Heap& heap, Int32 from, Int32 count) : heap(heap), index(from), to(from + count) { }
const String* RangeEnumerator::nextPropertyName() { return (index >= to ? 0 : String::fromInt(heap, index++)); }

/* --- StringListEnumerator --- */

StringListEnumerator::StringListEnumerator(UInt32 initialCapacity, Heap* heap) : list(heap), nextIndex(0) { list.reserve(initialCapacity); }
void StringListEnumerator::add(const String* s) { list.push(s); }
const String* StringListEnumerator::nextPropertyName() { return (nextIndex < list.size() ? list[nextIndex++] : 0); }

/* --- JoiningEnumerator --- */

JoiningEnumerator::JoiningEnumerator(Runtime& rt, Enumerator* leftEnumerator, const Object* rightObject)
		: rt(rt), leftEnumerator(leftEnumerator), rightObject(rightObject)
		, rightEnumerator(rightObject->getPropertyEnumerator(rt)) {
}

const String* JoiningEnumerator::nextPropertyName() {
	const String* name = rightEnumerator->nextPropertyName();
	if (name == 0) {
		do {
			name = leftEnumerator->nextPropertyName();
			if (name != 0 && !rightObject->isOwnPropertyEnumerable(rt, Value(name))) {
				return name;
			}
		} while (name != 0);
	}
	return name;
}

/* --- Code --- */

Code::Code(Heap& heap, Constants* constants)
	: super(heap), code(0, &heap), constants(constants), nameIndexes(&heap), varNames(&heap), argumentNames(&heap)
	, name(0), selfName(0), source(0), bloomSet(0), maxStackDepth(0)
#if (VERIFY_STACK_LOGIC)
	, stackDepths(&heap)
#endif
{
	assert(constants != 0);
}

bool Code::lookupNameIndex(const String* name, Int32& index) const {
	const Table::Bucket* bucket = nameIndexes.lookup(name);
	if (bucket == 0) {
		return false;
	}
	index = bucket->getIndexValue();
	return true;
}

/* --- Function --- */

Function* Function::asFunction() { return this; }
const String* Function::typeOfString() const { return &FUNCTION_STRING; }
const String* Function::getClassName() const { return &F_UNCTION_STRING; }
const String* Function::toString(Heap& heap) const { return &NATIVE_FUNCTION_STRING; }
Value Function::getInternalValue(Heap& heap) const { return Value(&NATIVE_FUNCTION_STRING); }
Object* Function::getPrototype(Runtime& rt) const { return &rt.functionPrototype; }

Value Function::construct(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	return call(rt, processor, argc, argv, thisObject);
}

bool Function::hasInstance(Runtime& rt, Processor& processor, Object* object) const {
	if (object == 0) {
		return false;
	}
	Value v(Value::UNDEFINED);
	getPropertyDeep(rt, Value(&PROTOTYPE_STRING), &v);
	Object* prototypeObject = v.asObject();
	if (prototypeObject == 0) {
		Heap& heap = rt.getHeap();
		Exception::throwError(heap, TYPE_ERROR, "Non-object prototype in instanceof check");
		return false;
	}
	while ((object = object->getPrototype(rt)) != 0) {
		if (object == prototypeObject) {
			return true;
		}
	}
	return false;
}

/* --- Array --- */

Array::Array(Heap& heap) : super(heap), length(0), denseVector(&heap) { }

const String* Array::getClassName() const { return &A_RRAY_STRING; }

Object* Array::getPrototype(Runtime& rt) const { // FIX : have a sep. ArrayPrototype object like we have for Function?
	return (this == &rt.arrayPrototype ? static_cast<Object*>(&rt.objectPrototype)
			: static_cast<Object*>(&rt.arrayPrototype));
}

Flags Array::getOwnProperty(Runtime& rt, const Value& key, Value* v) const {
	UInt32 index;
	if (key.toArrayIndex(index) && index < denseVector.size()) {
		*v = denseVector[index];
		return STANDARD_FLAGS | EXISTS_FLAG;
	}
	if (key.isEqualTo(LENGTH_STRING)) {
		*v = Value(length);
		return (READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG | EXISTS_FLAG);
	}
	return super::getOwnProperty(rt, key, v);
}

void Array::sliceDenseVector(Runtime& rt, const Value& key) {
	UInt32 index;
	if (key.toArrayIndex(index) && index < denseVector.size()) {
		Heap& heap = rt.getHeap();
		Table& props = getCompleteObject(rt)->accessProperties();
		for (UInt32 moveIndex = index + 1; moveIndex < denseVector.size(); ++moveIndex) {
			props.update(props.insert(String::fromInt(heap, moveIndex)), denseVector[moveIndex]);
		}
		denseVector.resize(index);
// FIX :		std::cerr << "sliced " << this << " at " << index << " (dense size: " << denseVector.size() << ", table load: " << props.getLoadCount() << ")" << std::endl;
	}
}

bool Array::setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags) {
	UInt32 index;
	if (key.toArrayIndex(index) && index != 0xFFFFFFFFU) {
		if ((flags & (READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG)) != 0) {
			sliceDenseVector(rt, key);
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
					denseVector.push(v);
				} else {
					Heap& heap = rt.getHeap();
					Table& props = completeObject->accessProperties();
					Table::Bucket* bucket = props.lookup(String::fromInt(heap, index));
					if (bucket == 0 || bucket->hasStandardFlags()) {
						UInt32 moveIndex = index + 1;
						denseVector.push(v);
						while ((bucket = props.lookup(String::fromInt(heap, moveIndex))) != 0 && bucket->hasStandardFlags()) {
							denseVector.push(bucket->getValue());
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
			Heap& heap = rt.getHeap();
			Exception::throwError(heap, RANGE_ERROR, "Invalid array length"); // FIX : how could we do this without exceptions?
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

	return super::setOwnProperty(rt, key, v, flags);
}

bool Array::updateProperty(Runtime& rt, const Value& key, const Value& v) {
	UInt32 index;
	if (key.toArrayIndex(index) && index != 0xFFFFFFFFU) {
		return setOwnProperty(rt, key, v, STANDARD_FLAGS);
	} else {
		return super::updateProperty(rt, key, v);
	}
}

bool Array::deleteProperty(Runtime& rt, const Value& key) {
	if (key.isEqualTo(LENGTH_STRING)) {
		return false;
	}
	sliceDenseVector(rt, key);
	return super::deleteProperty(rt, key);
}

void Array::constructCompleteObject(Runtime& rt) const { }

Enumerator* Array::getPropertyEnumerator(Runtime& rt) const {
	Heap& heap = rt.getHeap();
	Enumerator* rangeEnumerator = heap.govern(new(heap) RangeEnumerator(heap, 0, denseVector.size()));
	return (completeObject == 0 ? rangeEnumerator
			: heap.govern(new(heap) JoiningEnumerator(rt, rangeEnumerator, completeObject)));
}

/* --- LazyScriptObject --- */

template<class SUPER> Flags LazyScriptObject<SUPER>::getOwnProperty(Runtime& rt, const Value& key, Value* v) const {
	return getCompleteObject(rt)->getOwnProperty(rt, key, v);
}

template<class SUPER> bool LazyScriptObject<SUPER>::setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags) {
	return getCompleteObject(rt)->setOwnProperty(rt, key, v, flags);
}

template<class SUPER> bool LazyScriptObject<SUPER>::deleteProperty(Runtime& rt, const Value& key) {
	return getCompleteObject(rt)->deleteProperty(rt, key);
}

template<class SUPER> Enumerator* LazyScriptObject<SUPER>::getPropertyEnumerator(Runtime& rt) const {
	return getCompleteObject(rt)->getPropertyEnumerator(rt);
}

template<class SUPER> ScriptObject* LazyScriptObject<SUPER>::getCompleteObject(Runtime& rt) const {
	if (completeObject == 0) {
		Heap& heap = rt.getHeap();
		completeObject = heap.govern(new(heap) ScriptObject(heap, SUPER::getPrototype(rt)));
		constructCompleteObject(rt);
	}
	return completeObject;
}

/* --- ExtensibleFunction --- */

void ExtensibleFunction::createPrototypeObject(Runtime& rt, Object* forObject, bool dontEnumPrototype) const {
	Heap& heap = rt.getHeap();
	ScriptObject* prototype = heap.govern(new(heap) ScriptObject(heap, &rt.objectPrototype));
	prototype->setOwnProperty(rt, Value(&CONSTRUCTOR_STRING), Value(const_cast<ExtensibleFunction*>(this)), DONT_ENUM_FLAG);
	forObject->setOwnProperty(rt, Value(&PROTOTYPE_STRING), Value(prototype), (dontEnumPrototype ? (DONT_ENUM_FLAG | DONT_DELETE_FLAG) : DONT_DELETE_FLAG));
}

/* --- ScriptFunction --- */

ScriptFunction::ScriptFunction(Heap& heap, const Code* code, Scope* closure)
		: super(heap), code(code), closure(closure) { }

Value ScriptFunction::getInternalValue(Heap& heap) const { return Value(toString(heap)); }
const Code* ScriptFunction::getCode() const { return code; }

const String* ScriptFunction::toString(Heap& heap) const {
	const String* s = code->getSource();
	return (s != 0 ? s : Object::toString(heap));
}

Value ScriptFunction::call(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	processor.chain(this, argc, argv, thisObject);
	return Value::UNDEFINED;
}

void ScriptFunction::constructCompleteObject(Runtime& rt) const {
	createPrototypeObject(rt, completeObject, false);
	completeObject->setOwnProperty(rt, Value(&NAME_STRING), Value(code->getName()), DONT_ENUM_FLAG);
	completeObject->setOwnProperty(rt, Value(&LENGTH_STRING), Value(code->getArgumentsCount()), READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG);
}

/* --- SpecialConstructorFunction --- */

SpecialConstructorFunction::SpecialConstructorFunction(Heap& heap, Function* regularFunction, Function* constructorFunction)
		: super(heap), regularFunction(regularFunction), constructorFunction(constructorFunction) {
	assert(regularFunction != 0);
}

Value SpecialConstructorFunction::call(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	return regularFunction->call(rt, processor, argc, argv, thisObject);
}

Value SpecialConstructorFunction::construct(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	if (constructorFunction == 0) {
		Value v;
		regularFunction->getPropertyDeep(rt, Value(&NAME_STRING), &v);
		Heap& heap = rt.getHeap();
		Exception::throwError(heap, TYPE_ERROR, String::concat(heap, *v.toString(heap), String(" is not a constructor")));
	}
	return constructorFunction->call(rt, processor, argc, argv, thisObject);
}

void SpecialConstructorFunction::constructCompleteObject(Runtime& rt) const {
	Object* templateFunction = regularFunction;
	if (constructorFunction != 0) {
		createPrototypeObject(rt, completeObject, true);
		templateFunction = constructorFunction;
	}
	Value v;
	Flags flags = templateFunction->getPropertyDeep(rt, Value(&NAME_STRING), &v);
	if (flags != NONEXISTENT) { // FIX : getName?
		completeObject->setOwnProperty(rt, Value(&NAME_STRING), v, flags);
	}
	flags = templateFunction->getPropertyDeep(rt, Value(&LENGTH_STRING), &v);
	if (flags != NONEXISTENT) { // FIX : in the future I think we should have a getLength
		completeObject->setOwnProperty(rt, Value(&LENGTH_STRING), v, flags);
	}
}

/* --- FunctionPrototypeFunction --- */

Value FunctionPrototypeFunction::call(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	return Value::UNDEFINED;
}

Object* FunctionPrototypeFunction::getPrototype(Runtime& rt) const { return &rt.objectPrototype; }

void FunctionPrototypeFunction::constructCompleteObject(Runtime& rt) const {
	completeObject->setOwnProperty(rt, Value(&NAME_STRING), Value(&EMPTY_STRING), DONT_ENUM_FLAG);
	completeObject->setOwnProperty(rt, Value(&LENGTH_STRING), Value(0.0), READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG);
}

/* --- Error --- */

Error::Error(Heap& heap, ErrorType type, const String* message)
		: super(heap), errorType(type), name(&ERROR_NAMES[errorType]), message(message) {
	assert(0 <= errorType && errorType < ERROR_TYPE_COUNT);
}

const String* Error::getClassName() const { return &E_RROR_STRING; }
Value Error::getInternalValue(Heap& heap) const { return Value(&ERROR_NAMES[errorType]); }
Object* Error::getPrototype(Runtime& rt) const { return rt.errorPrototypes[errorType]; }

const String* Error::toString(Heap& heap) const {
	return (message == 0 ? name : String::concat(heap, String(*name, COLON_SPACE, &heap), *message));
}

void Error::updateReflection(Runtime& rt) {
	Value v;
	name = (getPropertyDeep(rt, Value(&NAME_STRING), &v) != NONEXISTENT ? v.toString(rt.getHeap()) : &ERROR_NAMES[errorType]);
	message = (getPropertyDeep(rt, Value(&MESSAGE_STRING), &v) != NONEXISTENT ? v.toString(rt.getHeap()) : 0);
}

bool Error::setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags) {
	const bool result = super::setOwnProperty(rt, key, v, flags);
	updateReflection(rt);
	return result;
}

bool Error::deleteProperty(Runtime& rt, const Value& key) {
	const bool result = super::deleteProperty(rt, key);
	updateReflection(rt);
	return result;
}

void Error::constructCompleteObject(Runtime& rt) const {
	if (message != 0) {
		completeObject->setOwnProperty(rt, Value(&MESSAGE_STRING), Value(message), DONT_ENUM_FLAG);
	}
}

/* --- ErrorPrototype --- */

ErrorPrototype::ErrorPrototype(Heap& heap, ErrorType type) : super(heap, type) { }

Object* ErrorPrototype::getPrototype(Runtime& rt) const {
	return (errorType == GENERIC_ERROR ? static_cast<Object*>(&rt.objectPrototype)
			: static_cast<Object*>(rt.errorPrototypes[GENERIC_ERROR]));
}

/* --- Arguments --- */

Arguments::Arguments(Heap& heap, FunctionScope* scope, UInt32 argumentsCount)
		: super(heap), scope(scope), argumentsCount(argumentsCount), deletedArguments(argumentsCount, &heap) {
	std::fill(deletedArguments.begin(), deletedArguments.end(), false);
}

const String* Arguments::getClassName() const { return &A_RGUMENTS_STRING; }

const String* Arguments::toString(Heap& heap) const {
	return heap.govern(new(heap) String(String(BRACKET_OBJECT_STRING, O_BJECT_STRING, &heap), END_BRACKET_STRING, &heap));
}

Object* Arguments::getPrototype(Runtime& rt) const { return &rt.objectPrototype; }

Value* Arguments::findProperty(const Value& key) const {
	UInt32 i;
	return (key.toArrayIndex(i) && i < argumentsCount && !deletedArguments[i] ? scope->getLocalsPointer() + i : 0);
}

Flags Arguments::getOwnProperty(Runtime& rt, const Value& key, Value* v) const {
	const Value* p = findProperty(key);
	return (p == 0 ? super::getOwnProperty(rt, key, v) : ((void)(*v = *p)
			, (STANDARD_FLAGS | DONT_ENUM_FLAG | EXISTS_FLAG)));
}

bool Arguments::setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags) {
	Value* p = findProperty(key);
	if (p != 0 && (flags & (READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG)) != 0) {
		deletedArguments[p - scope->getLocalsPointer()] = true;
		p = 0;
	}
	return (p == 0 ? super::setOwnProperty(rt, key, v, flags) : ((void)(*p = v), true));
}

bool Arguments::deleteProperty(Runtime& rt, const Value& key) {
	const Value* p = findProperty(key);
    return (p == 0 ? super::deleteProperty(rt, key) : (deletedArguments[p - scope->getLocalsPointer()] = true));
}

Enumerator* Arguments::getPropertyEnumerator(Runtime& rt) const {
	return (completeObject == 0 ? super::getPropertyEnumerator(rt) : completeObject->getPropertyEnumerator(rt));
}

void Arguments::constructCompleteObject(Runtime& rt) const {
	completeObject->setOwnProperty(rt, Value(&LENGTH_STRING), Value(argumentsCount), DONT_ENUM_FLAG);
	completeObject->setOwnProperty(rt, Value(&CALLEE_STRING), Value(scope->function), DONT_ENUM_FLAG);
}

/* --- Scope --- */

Scope::Scope(Heap& heap, Scope* parentScope)
		: super(heap), parentScope(parentScope), localsPointer(parentScope != 0 ? parentScope->localsPointer : 0) { }

Flags Scope::readVar(Runtime& rt, const String* name, Value* v) const {
	assert(parentScope != 0);
	return parentScope->readVar(rt, name, v);
}

void Scope::writeVar(Runtime& rt, const String* name, const Value& v) {
	assert(parentScope != 0);
	return parentScope->writeVar(rt, name, v);
}

bool Scope::deleteVar(Runtime& rt, const String* key) {
	assert(parentScope != 0);
	return parentScope->deleteVar(rt, key);
}

void Scope::declareVar(Runtime& rt, const String* name, const Value& initValue, bool dontDelete) {
	assert(parentScope != 0);
	return parentScope->declareVar(rt, name, initValue, dontDelete);
}

Value* Scope::getLocalsPointer() const { return localsPointer; }
Scope* Scope::getParentScope() const { return parentScope; }

/* --- FunctionScope --- */

static UInt32 calcLocalsVariablesSize(const ScriptFunction* function, UInt32 argc) {
	const Code* code = function->getCode();
	return code->getVarsCount() + std::max(code->getArgumentsCount(), argc);
}

FunctionScope::FunctionScope(Heap& heap, ScriptFunction* function, UInt32 argc, const Value* argv)
		: super(heap, function->closure), function(function), passedArgumentsCount(argc)
		, locals(calcLocalsVariablesSize(function, argc), &heap), dynamicVars(0), bloomSet(function->code->bloomSet) {
	const Code* code = function->getCode();
	localsPointer = locals.begin() + code->getVarsCount();
	std::fill(locals.begin(), localsPointer, Value::UNDEFINED);
	Value* e = std::copy(argv, argv + argc, localsPointer);
	if (code->getArgumentsCount() > argc) {
		std::fill(e, locals.end(), Value::UNDEFINED);
	}
}

ScriptObject* FunctionScope::getDynamicVars(Runtime& rt) const {
	if (dynamicVars == 0) {
		Heap& heap = rt.getHeap();
		dynamicVars = heap.govern(new(heap) ScriptObject(heap, 0));
		dynamicVars->setOwnProperty(rt, Value(&ARGUMENTS_STRING)
				, Value(heap.govern(new(heap) Arguments(heap, const_cast<FunctionScope*>(this), passedArgumentsCount)))
				, DONT_DELETE_FLAG);
	}
	return dynamicVars;
}

Flags FunctionScope::readVar(Runtime& rt, const String* name, Value* v) const {
	const UInt32 bloomCode = name->createBloomCode();
	if ((bloomSet & bloomCode) == bloomCode) {
		Int32 index;
		const Code* code = function->getCode();
		if (code->lookupNameIndex(name, index)) {
			assert(locals.begin() <= localsPointer + index && localsPointer + index < locals.end());
			*v = localsPointer[index];
			return DONT_DELETE_FLAG | EXISTS_FLAG;
		}
		if (dynamicVars != 0 || name->isEqualTo(ARGUMENTS_STRING)) {
			Flags flags = getDynamicVars(rt)->getOwnProperty(rt, Value(name), v);
			if (flags != NONEXISTENT) {
				return flags;
			}
		}
		if (code->selfName != 0 && name->isEqualTo(*code->selfName)) {
			*v = Value(function);
			return DONT_DELETE_FLAG | READ_ONLY_FLAG | EXISTS_FLAG;
		}
	}
	return parentScope->readVar(rt, name, v);
}

void FunctionScope::writeVar(Runtime& rt, const String* name, const Value& v) {
	const UInt32 bloomCode = name->createBloomCode();
	if ((bloomSet & bloomCode) == bloomCode) {
		Int32 index;
		const Code* code = function->getCode();
		if (code->lookupNameIndex(name, index)) {
			assert(locals.begin() <= localsPointer + index && localsPointer + index < locals.end());
			localsPointer[index] = v;
			return;
		}
		// FIX : make sub that lookup's the bucket, also use in getProperty
		if (dynamicVars != 0 || name->isEqualTo(ARGUMENTS_STRING)) {
			Table& props = getDynamicVars(rt)->accessProperties();
			Table::Bucket* bucket = props.lookup(name);
			if (bucket != 0) {
				props.update(bucket, v);
				return;
			}
		}
		if (code->selfName != 0 && name->isEqualTo(*code->selfName)) {
			return;
		}
	}
	parentScope->writeVar(rt, name, v); // FIX : recursion
}

bool FunctionScope::deleteVar(Runtime& rt, const String* name) {
	const UInt32 bloomCode = name->createBloomCode();
	if ((bloomSet & bloomCode) == bloomCode) {
		Int32 index;
		const Code* code = function->getCode();
		if (code->lookupNameIndex(name, index)) {
			return false;
		}
		if (dynamicVars != 0 || name->isEqualTo(ARGUMENTS_STRING)) {
			Table& props = getDynamicVars(rt)->accessProperties();
			Table::Bucket* bucket = props.lookup(name);
			if (bucket != 0) {
				return props.erase(bucket);
			}
		}
		if (code->selfName != 0 && name->isEqualTo(*code->selfName)) {
			return false;
		}
	}
	return parentScope->deleteVar(rt, name); // FIX : recursion
}

void FunctionScope::declareVar(Runtime& rt, const String* name, const Value& initValue, bool dontDelete) {
	Int32 index;
	if (!function->getCode()->lookupNameIndex(name, index)) {
		Table& props = getDynamicVars(rt)->accessProperties();
		Table::Bucket* bucket = props.insert(name);
		if (!bucket->valueExists()) {
			bloomSet |= name->createBloomCode();
			props.update(bucket, Value::UNDEFINED, dontDelete ? DONT_DELETE_FLAG : 0);
		}
	}
	if (!initValue.isUndefined()) {
		writeVar(rt, name, initValue);
	}
}

/* --- EvalScope --- */

EvalScope::EvalScope(Heap& heap, Scope* parentScope) : super(heap, parentScope) { }

void EvalScope::declareVar(Runtime& rt, const String* name, const Value& initValue, bool dontDelete) {
	parentScope->declareVar(rt, name, initValue, false);
}

/* --- GlobalScope --- */

GlobalScope::GlobalScope(Heap& heap) : super(heap, 0) { }
Flags GlobalScope::readVar(Runtime& rt, const String* name, Value* v) const {
	return rt.getGlobals()->getPropertyDeep(rt, Value(name), v);
}
void GlobalScope::writeVar(Runtime& rt, const String* name, const Value& v) {
	rt.getGlobals()->setPropertyDeep(rt, Value(name), v);
}
bool GlobalScope::deleteVar(Runtime& rt, const String* name) { return rt.getGlobals()->deleteProperty(rt, Value(name)); }
void GlobalScope::declareVar(Runtime& rt, const String* name, const Value& initValue, bool dontDelete) {
	Value v;
	// FIX : deep or not?!
	const Flags flags = rt.getGlobals()->getOwnProperty(rt, Value(name), &v);
	if (flags == NONEXISTENT || !initValue.isUndefined()) {
		rt.getGlobals()->setOwnProperty(rt, Value(name), initValue, dontDelete ? DONT_DELETE_FLAG : 0);
	}
}

/* --- CatchScope --- */

CatchScope::CatchScope(Heap& heap, Scope* parentScope, const String* exceptionName, const Value& exceptionValue)
		: super(heap, parentScope), exceptionName(exceptionName), exceptionValue(exceptionValue) { }

Flags CatchScope::readVar(Runtime& rt, const String* name, Value* v) const {
	if (name->isEqualTo(*exceptionName)) {
		*v = exceptionValue;
		return DONT_DELETE_FLAG | EXISTS_FLAG;
	} else {
		return parentScope->readVar(rt, name, v);
	}
}

void CatchScope::writeVar(Runtime& rt, const String* name, const Value& v) {
	if (name->isEqualTo(*exceptionName)) {
		exceptionValue = v;
	} else {
		parentScope->writeVar(rt, name, v);
	}
}

bool CatchScope::deleteVar(Runtime& rt, const String* name) {
	if (name->isEqualTo(*exceptionName)) {
		return false;
	} else {
		return parentScope->deleteVar(rt, name);
	}
}

/* --- WithScope --- */

WithScope::WithScope(Heap& heap, Scope* parentScope, Object* withObject) : super(heap, parentScope), withObject(withObject) { }

Flags WithScope::readVar(Runtime& rt, const String* name, Value* v) const {
	Flags flags = withObject->getPropertyDeep(rt, Value(name), v);
	return (flags != NONEXISTENT ? flags : parentScope->readVar(rt, name, v));
}

void WithScope::writeVar(Runtime& rt, const String* name, const Value& v) {
	const Value key(name);
	for (Object* o = withObject; o != 0; o = o->getPrototype(rt)) {
		Value dummy;
		const Flags flags = o->getOwnProperty(rt, key, &dummy);
		if (flags != NONEXISTENT) {
			if ((flags & READ_ONLY_FLAG) == 0) {
				withObject->setOwnProperty(rt, key, v, STANDARD_FLAGS);
			}
			return;
		}
	}
	parentScope->writeVar(rt, name, v);
}

bool WithScope::deleteVar(Runtime& rt, const String* name) {
	Value dummy;
	Flags findFlags = withObject->getPropertyDeep(rt, Value(name), &dummy);
	return (findFlags != NONEXISTENT ? withObject->deleteProperty(rt, Value(name)) : parentScope->deleteVar(rt, name));
}

/* --- Processor --- */

const Int32 MIN_OPERAND_VALUE = -(1 << 23);
const Int32 MAX_OPERAND_VALUE = (1 << 23) - 1;

// This list must be in enum order
const Processor::OpcodeInfo Processor::opcodeInfo[Processor::OP_COUNT] = {
	{ CONST_OP                   , "CONST"                   , +1     , 0 },
	{ READ_LOCAL_OP              , "READ_LOCAL"              , +1     , 0 },
	{ WRITE_LOCAL_OP             , "WRITE_LOCAL"             , 0      , 0 },
	{ WRITE_LOCAL_POP_OP         , "WRITE_LOCAL_POP"         , -1     , 0 },
	{ READ_NAMED_OP              , "READ_NAMED"              , 1      , 0 },
	{ WRITE_NAMED_OP             , "WRITE_NAMED"             , 0      , 0 },
	{ WRITE_NAMED_POP_OP         , "WRITE_NAMED_POP"         , -1     , 0 },
	{ GET_PROPERTY_OP            , "GET_PROPERTY"            , -1     , 0 },
	{ SET_PROPERTY_OP            , "SET_PROPERTY"            , -2     , 0 },
	{ SET_PROPERTY_POP_OP        , "SET_PROPERTY_POP"        , -3     , 0 },
	{ ADD_PROPERTY_OP            , "ADD_PROPERTY"            , -1     , 0 },
	{ OBJ_TO_NUMBER_OP           , "OBJ_TO_NUMBER"           , 0      , 0 },
	{ OBJ_TO_STRING_OP           , "OBJ_TO_STRING"           , 0      , 0 },
	{ PRE_EQ_OP                  , "PRE_EQ"                  , 0      , 0 },
	{ INC_OP                     , "INC"                     , 0      , 0 },
	{ DEC_OP                     , "DEC"                     , 0      , 0 },
	{ ADD_OP                     , "ADD"                     , -1     , 0 },
	{ SUB_OP                     , "SUB"                     , -1     , 0 },
	{ MUL_OP                     , "MUL"                     , -1     , 0 },
	{ DIV_OP                     , "DIV"                     , -1     , 0 },
	{ MOD_OP                     , "MOD"                     , -1     , 0 },
	{ OR_OP                      , "OR"                      , -1     , 0 },
	{ XOR_OP                     , "XOR"                     , -1     , 0 },
	{ AND_OP                     , "AND"                     , -1     , 0 },
	{ SHL_OP                     , "SHL"                     , -1     , 0 },
	{ SHR_OP                     , "SHR"                     , -1     , 0 },
	{ USHR_OP                    , "USHR"                    , -1     , 0 },
	{ PLUS_OP                    , "PLUS"                    , 0      , 0 },
	{ MINUS_OP                   , "MINUS"                   , 0      , 0 },
	{ INV_OP                     , "INV"                     , 0      , 0 },
	{ NOT_OP                     , "NOT"                     , 0      , 0 },
	{ X_EQ_OP                    , "X_EQ"                    , -1     , 0 },
	{ X_NEQ_OP                   , "X_NEQ"                   , -1     , 0 },
	{ EQ_OP                      , "EQ"                      , -1     , 0 },
	{ NEQ_OP                     , "NEQ"                     , -1     , 0 },
	{ LT_OP                      , "LT"                      , -1     , 0 },
	{ LEQ_OP                     , "LEQ"                     , -1     , 0 },
	{ GT_OP                      , "GT"                      , -1     , 0 },
	{ GEQ_OP                     , "GEQ"                     , -1     , 0 },
	{ JMP_OP                     , "JMP"                     , 0      , OpcodeInfo::TERMINAL },
	{ JSR_OP                     , "JSR"                     , 0      , 0 },
	{ JT_OP                      , "JT"                      , -1     , 0 },
	{ JF_OP                      , "JF"                      , -1     , 0 },
	{ JT_OR_POP_OP               , "JT_OR_POP"               , -1     , OpcodeInfo::NO_POP_ON_BRANCH },
	{ JF_OR_POP_OP               , "JF_OR_POP"               , -1     , OpcodeInfo::NO_POP_ON_BRANCH },
	{ POP_OP                     , "POP"                     , 0      , OpcodeInfo::POP_OPERAND },
	{ PUSH_BACK_OP               , "PUSH_BACK"               , 0      , OpcodeInfo::POP_OPERAND },
	{ REPUSH_OP                  , "REPUSH"                  , 1      , 0 },
	{ SWAP_OP                    , "SWAP"                    , 0      , 0 },
	{ REPUSH_2_OP                , "REPUSH_2"                , +2     , 0 },
	{ POST_SHUFFLE_OP            , "POST_SHUFFLE"            , +1     , 0 },
	{ CALL_OP                    , "CALL"                    , 0      , OpcodeInfo::POP_OPERAND },
	{ CALL_METHOD_OP             , "CALL_METHOD"             , -1     , OpcodeInfo::POP_OPERAND },
	{ CALL_EVAL_OP               , "CALL_EVAL"               , 0      , OpcodeInfo::POP_OPERAND },
	{ NEW_OP                     , "NEW"                     , +1     , OpcodeInfo::POP_OPERAND },
	{ NEW_RESULT_OP              , "NEW_RESULT"              , -1     , 0 },
	{ NEW_OBJECT_OP              , "NEW_OBJECT"              , +1     , 0 },
	{ NEW_ARRAY_OP               , "NEW_ARRAY"               , +1     , 0 },
	{ NEW_REG_EXP_OP             , "NEW_REG_EXP"             , -1     , 0 },
	{ RETURN_OP                  , "RETURN"                  , -1     , OpcodeInfo::TERMINAL },
	{ THIS_OP                    , "THIS"                    , +1     , 0 },
	{ VOID_OP                    , "VOID"                    , +1     , 0 },
	{ DELETE_OP                  , "DELETE"                  , -1     , 0 },
	{ DELETE_NAMED_OP            , "DELETE_NAMED"            , 1      , 0 },
	{ GEN_FUNC_OP                , "GEN_FUNC"                , +1     , 0 },
	{ DECLARE_OP                 , "DECLARE"                 , -1     , 0 },
	{ CATCH_SCOPE_OP             , "CATCH_SCOPE"             , -1     , 0 },
	{ WITH_SCOPE_OP              , "WITH_SCOPE"              , -1     , 0 },
	{ POP_FRAME_OP               , "POP_FRAME"               , 0      , 0 },
	{ TRY_OP                     , "TRY"                     , 0      , OpcodeInfo::NO_POP_ON_BRANCH },
	{ TRIED_OP                   , "TRIED"                   , 0      , 0 },
	{ THROW_OP                   , "THROW"                   , -1     , OpcodeInfo::TERMINAL },
	{ IN_OP                      , "IN"                      , -1     , 0 },
	{ INSTANCE_OF_OP             , "INSTANCE_OF"             , -1     , 0 },
	{ TYPEOF_OP                  , "TYPEOF"                  , 0      , 0 },
	{ TYPEOF_NAMED_OP            , "TYPEOF_NAMED"            , 1      , 0 },
	{ GET_ENUMERATOR_OP          , "GET_ENUMERATOR"          , 0      , 0 },
	{ NEXT_PROPERTY_OP           , "NEXT_PROPERTY"           , 0      , OpcodeInfo::POP_ON_BRANCH }
};

const Processor::OpcodeInfo& Processor::getOpcodeInfo(const Opcode opcode) {
	assert(0 <= opcode && opcode < OP_COUNT);
	assert(opcodeInfo[opcode].opcode == opcode); // source code check
	return opcodeInfo[opcode];
}

Processor::Processor(Runtime& rt) : super(rt.getHeap()), rt(rt), currentFrame(0), firstCatcher(0)
		, cyclesLeft(0x7FFFFFFF), stack(rt.getStackSize(), &rt.getHeap()) {
	stack[0] = Value::UNDEFINED;
	reset();
}

CodeWord Processor::packInstruction(const Opcode opcode, const Int32 operand) {
	assert(0 <= opcode && static_cast<int>(opcode) < 256);
	assert(MIN_OPERAND_VALUE < operand && operand < MAX_OPERAND_VALUE);
	return (operand << 8) | opcode;
}

std::pair<Processor::Opcode, Int32> Processor::unpackInstruction(const CodeWord codeWord) {
	return std::make_pair(static_cast<Opcode>(codeWord & 255), static_cast<Int32>(codeWord) >> 8);
}

void Processor::push(const Value& v) { assert(sp + 1 < stack.end()); *++sp = v; }
void Processor::pop2push1(const Value& v) { assert(sp >= stack.begin()); *--sp = v; }
void Processor::pop(Int32 count) { assert(sp - count + 1 >= stack.begin()); sp -= count; }
void Processor::resetTimeOut(Int32 clockCycles) { cyclesLeft = clockCycles; }

void Processor::chain(const Code* code, Scope* scope, Object* thisObject) {
	Heap& heap = rt.getHeap();
	// We could grow stack here *but* then existing pointers into the stack might become invalid and we would need to copy arguments for native calls somewhere (e.g. to C++ stack).
	if (sp + code->getMaxStackDepth() > stack.end()) {
		error(RANGE_ERROR, &STACK_OVERFLOW_STRING);
	} else {
		currentFrame = heap.govern(new(heap) Frame(ip, code, scope, thisObject, currentFrame));
		ip = code->getCodeWords();
	}
}

void Processor::chain(ScriptFunction* func, UInt32 argc, const Value* argv, Object* thisObject) {
	Heap& heap = rt.getHeap();
	chain(func->getCode(), heap.govern(new(heap) FunctionScope(heap, func, argc, argv)), thisObject);
}

void Processor::enterEval(const Code* code, bool directCall) {
	Heap& heap = rt.getHeap();
	assert(!directCall || currentFrame != 0); // FIX : what to do with global scope?
	chain(code, heap.govern(new(heap) EvalScope(heap, directCall ? currentFrame->scope : rt.getGlobalScope()))
			, directCall ? currentFrame->thisObject : rt.getGlobals());
}

void Processor::reset() {
	cyclesLeft = 0x7FFFFFFF;
	currentFrame = 0;
	firstCatcher = 0;
	ip = 0;
	sp = stack.begin();
}

void Processor::popFrame() {
	Frame* killMe = currentFrame;
	currentFrame = currentFrame->previousFrame;
	delete killMe;
}

void Processor::throwVirtualException(const Value& exception) {
	if (firstCatcher == 0) { // FIX: what exception to throw here?
		reset();
		throw Exception(rt.getHeap(), exception);
	}
	ip = firstCatcher->ip;
	assert(ip != 0);
	currentFrame = firstCatcher->frame;
	sp = firstCatcher->sp;
	push(exception);
	Catcher* killMe = firstCatcher;
	firstCatcher = firstCatcher->nextCatcher;
	delete killMe;
}

void Processor::error(ErrorType errorType, const String* message) {
	Heap& heap = rt.getHeap();
	throwVirtualException(Value(heap.govern(new(heap) Error(heap, errorType, message))));
}

Object* Processor::convertToObject(const Value& v, const bool requestExtensible) {
	Object* const object = v.toObject(rt, requestExtensible);
	if (object == 0) {
		error(TYPE_ERROR, &CANNOT_CONVERT_TO_OBJECT_STRING);
	}
	return object;
}

bool Processor::privateRun() {
	Heap& heap = rt.getHeap();
	assert(currentFrame != 0);
	Scope* scope = currentFrame->scope;
	assert(scope != 0);
	const Code* code = currentFrame->code;
	assert(ip >= code->getCodeWords() && ip < code->getCodeWords() + code->getCodeSize());
	const Value* constants = currentFrame->code->getConstantsPointer();
	Value* locals = scope->getLocalsPointer();
	Object* thisObject = currentFrame->thisObject;

	while (cyclesLeft >= 0) {
		--cyclesLeft;
		const CodeWord instruction = *ip++;
		const Opcode opcode = static_cast<Opcode>(instruction & 255);
		const Int32 im = static_cast<Int32>(instruction) >> 8;
		switch (opcode) {
			case CONST_OP:				push(constants[im]); break;
			case READ_LOCAL_OP:			assert(locals != 0); push(locals[im]); break;
			case WRITE_LOCAL_OP:		assert(locals != 0); locals[im] = sp[0]; break;
			case WRITE_LOCAL_POP_OP:	assert(locals != 0); locals[im] = sp[0]; pop(1); break;
			case READ_NAMED_OP:			{
											const String* name = constants[im].getString();
											if (scope->readVar(rt, name, ++sp) == NONEXISTENT) {
												error(REFERENCE_ERROR, heap.govern(new(heap) String(*name, IS_NOT_DEFINED_STRING, &heap)));
												return true;
											}
										}
										break;
									
			case WRITE_NAMED_OP:		scope->writeVar(rt, constants[im].getString(), sp[0]);
										break;
				
			case WRITE_NAMED_POP_OP:	scope->writeVar(rt, constants[im].getString(), sp[0]);
										pop(1);
										break;

			case GET_PROPERTY_OP: {
				const Object* o = convertToObject(sp[-1], false);
				if (o == 0) {
					return true;
				}
				if (o->getPropertyDeep(rt, sp[0], sp - 1) == NONEXISTENT) {
					sp[-1] = Value::UNDEFINED;
				}
				pop(1);
				break;
			}
			
			case SET_PROPERTY_OP: {
				Object* o = convertToObject(sp[-2], false);
				if (o == 0) {
					return true;
				}
				o->setPropertyDeep(rt, sp[-1], sp[0]);
				sp[-2] = sp[0];
				pop(2);
				break;
			}
			
			case SET_PROPERTY_POP_OP: {
				Object* o = convertToObject(sp[-2], false);
				if (o == 0) {
					return true;
				}
				o->setPropertyDeep(rt, sp[-1], sp[0]);
				pop(3);
				break;
			}

			case OBJ_TO_NUMBER_OP: {
				const Object* o = sp[0].asObject();
				if (o != 0) {
					rt.toNumberFunction->call(rt, *this, 1, sp, rt.getGlobals());
					return true;
				}
				break;
			}

			case OBJ_TO_STRING_OP: {
				const Object* o = sp[0].asObject();
				if (o != 0) {
					rt.toStringFunction->call(rt, *this, 1, sp, rt.getGlobals());
					return true;
				}
				break;
			}
				
			case PRE_EQ_OP:
				// FIX : doesn't feel totally efficient this...
				if (sp[-1].isObject() != sp[0].isObject()) {
					if (sp[-1].isObject()) {
						std::swap(sp[-1], sp[0]);
					}
					if (sp[-1].isString() || sp[-1].isNumber() || sp[-1].isBoolean()) {
						rt.toNumberFunction->call(rt, *this, 1, sp, rt.getGlobals());
						return true;
					}
				}
				break;

			case INC_OP:	sp[0] = Value(sp[0].toDouble() + 1.0); break;
			case DEC_OP:	sp[0] = Value(sp[0].toDouble() - 1.0); break;
			case ADD_OP:	pop2push1(sp[-1].add(heap, sp[0])); break;
			case SUB_OP:	pop2push1(Value(sp[-1].toDouble() - sp[0].toDouble())); break;
			case MUL_OP:	pop2push1(Value(sp[-1].toDouble() * sp[0].toDouble())); break;
			case DIV_OP:	pop2push1(Value(sp[-1].toDouble() / sp[0].toDouble())); break;
			case MOD_OP:	pop2push1(Value(modulo(sp[-1].toDouble(), sp[0].toDouble()))); break;
			case OR_OP:		pop2push1(Value(sp[-1].toInt() | sp[0].toInt())); break;
			case XOR_OP:	pop2push1(Value(sp[-1].toInt() ^ sp[0].toInt())); break;
			case AND_OP:	pop2push1(Value(sp[-1].toInt() & sp[0].toInt())); break;
			case SHL_OP:	pop2push1(Value(sp[-1].toInt() << ((UInt32)sp[0].toInt() & 0x1F))); break;
			case SHR_OP:	pop2push1(Value(sp[-1].toInt() >> ((UInt32)sp[0].toInt() & 0x1F))); break;
			case USHR_OP:	pop2push1(Value(((UInt32)(sp[-1].toInt()) >> ((UInt32)sp[0].toInt() & 0x1F)))); break;
			case PLUS_OP:	sp[0] = Value(sp[0].toDouble()); break;
			case MINUS_OP:	sp[0] = Value(-sp[0].toDouble()); break;
			case INV_OP:	sp[0] = Value(~sp[0].toInt()); break;
			case NOT_OP:	sp[0] = Value(!sp[0].toBool()); break;
			case X_EQ_OP:	pop2push1(Value(sp[-1].isStrictlyEqualTo(sp[0]))); break;
			case X_NEQ_OP:	pop2push1(Value(!sp[-1].isStrictlyEqualTo(sp[0]))); break;
			case EQ_OP:		pop2push1(Value(sp[-1].isEqualTo(sp[0]))); break;
			case NEQ_OP:	pop2push1(Value(!sp[-1].isEqualTo(sp[0]))); break;
			case LT_OP:		pop2push1(Value(sp[-1].isLessThan(sp[0]))); break;
			case LEQ_OP:	pop2push1(Value(sp[-1].isLessThanOrEqualTo(sp[0]))); break;
			case GT_OP:		pop2push1(Value(sp[0].isLessThan(sp[-1]))); break;
			case GEQ_OP:	pop2push1(Value(sp[0].isLessThanOrEqualTo(sp[-1]))); break;
			case JMP_OP:	ip += im; break;

							// FIX : make a new super-class to both with- and catch-scope and re-use here
			case JSR_OP: {
				currentFrame = heap.govern(new(heap) Frame(ip, code, scope, thisObject, currentFrame));
				ip += im;
				return true;
			}

			case JT_OP:		ip += (sp[0].toBool() ? im : 0); pop(1); break;
			case JF_OP:		ip += (!sp[0].toBool() ? im : 0); pop(1); break;
			case JT_OR_POP_OP:	if (sp[0].toBool()) ip += im; else pop(1); break;
			case JF_OR_POP_OP:	if (!sp[0].toBool()) ip += im; else pop(1); break;

			case POP_OP:			assert(im >= 0); pop(im); break;
			case PUSH_BACK_OP:		assert(im >= 0); sp[-im] = sp[0]; pop(im); break;
			case REPUSH_OP:			assert(im <= 0); push(sp[im]); break;
			case SWAP_OP:			std::swap(sp[-1], sp[0]); break;
			case REPUSH_2_OP:		push(sp[-1]); push(sp[-1]); break;
			case POST_SHUFFLE_OP:	push(sp[0]); sp[-1] = sp[-2]; sp[-2] = sp[-3]; sp[-3] = sp[0]; break;

			case CALL_OP: {
				Function* const f = sp[-im].asFunction();
				if (f == 0) {
					error(TYPE_ERROR, heap.govern(new(heap) String(*sp[-im].toString(heap), IS_NOT_A_FUNCTION_STRING, &heap)));
					return true;
				}
				sp[-im] = f->call(rt, *this, im, sp - im + 1, rt.getGlobals());
				pop(im);
				return true;
			}
			
			case CALL_METHOD_OP: { // FIX : share more with the others...
				Object* const thisObject = convertToObject(sp[-im - 1], true);
				if (thisObject == 0) {
					return true;
				}
				Value v(Value::UNDEFINED);
				Function* f;
				const Value& name = sp[-im];
				if (thisObject->getPropertyDeep(rt, name, &v) == NONEXISTENT || (f = v.asFunction()) == 0) {
					error(TYPE_ERROR, heap.govern(new(heap) String(*name.toString(heap), IS_NOT_A_FUNCTION_STRING, &heap)));
					return true;
				}
				sp[-(im + 1)] = f->call(rt, *this, im, sp - im + 1, thisObject);
				pop(im + 1);
				return true;
			}
			
			case CALL_EVAL_OP: { // FIX : all these are too similar!
				Function* f = sp[-im].asFunction();
				if (f == 0) {
					error(TYPE_ERROR, heap.govern(new(heap) String(*sp[-im].toString(heap), IS_NOT_A_FUNCTION_STRING, &heap)));
					return true;
				}
				if (f == &rt.evalFunction) {
					f = &rt.evalDirectFunction; // FIX : can't we implement this check with vm code instead of having a separate OP?
				}
				sp[-im] = f->call(rt, *this, im, sp - im + 1, rt.getGlobals());
				pop(im);
				return true;
			}

			case NEW_OP: {
				Function* f = sp[-im].asFunction(); // fix: move this logic into f->construct, which means the NEW_RESULT_OP should be a chained function instead
				if (f == 0) { // FIX : sub
					error(TYPE_ERROR, heap.govern(new(heap) String(*sp[-im].toString(heap), IS_NOT_A_FUNCTION_STRING, &heap)));
					return true;
				}
				Value v(Value::UNDEFINED);
				f->getPropertyDeep(rt, Value(&PROTOTYPE_STRING), &v);
				Object* prototype = v.asObject();
				Object* newObject = heap.govern(new(heap) ScriptObject(heap, prototype != 0 ? prototype : &rt.objectPrototype));
				sp[-(im - 1)] = f->construct(rt, *this, im, sp - im + 1, newObject);
				sp[-(im - 1) - 1] = Value(newObject);
				pop(im - 1);
				return true;
			}
			
			case NEW_RESULT_OP: pop2push1(sp[0].isObject() ? sp[0] : sp[-1]); break;
			
			case NEW_OBJECT_OP: {
				push(Value(heap.govern(new(heap) ScriptObject(heap, &rt.objectPrototype))));
				break;
			}
			
			case NEW_ARRAY_OP: {
				push(Value(heap.govern(new(heap) Array(heap))));
				break;
			}
			
			case NEW_REG_EXP_OP: { // FIX : all these very similar
				sp[-1] = rt.createRegExpFunction->call(rt, *this, 2, sp - 1, rt.getGlobals());
				pop(1);
				return true;
			}

			case RETURN_OP:	{
				ip = currentFrame->returnIP;
				if (ip == 0) {
					assert(firstCatcher == 0 && sp == stack.begin());
					reset();
					return false;
				} else {
					popFrame();
					assert(currentFrame != 0);
					return true;
				}
			}
			
			case THIS_OP: push(Value(thisObject)); break;
			case VOID_OP: push(Value::UNDEFINED); break;
			
			case GEN_FUNC_OP: {
				const Object* o = constants[im].getObject();
				assert(dynamic_cast<const Code*>(o) != 0);
				push(Value(heap.govern(new(heap) ScriptFunction(heap, reinterpret_cast<const Code*>(o), scope))));
				break;
			}
			
			case DECLARE_OP: {
				scope->declareVar(rt, constants[im].getString(), sp[0], true);
				pop(1);
				break;
			}
			
			case ADD_PROPERTY_OP: {
				Object* o = sp[-1].getObject();
				o->setOwnProperty(rt, constants[im], sp[0], STANDARD_FLAGS);
				pop(1);
				break;
			}
			
			case DELETE_NAMED_OP: {
				push(Value(scope->deleteVar(rt, constants[im].getString())));
				break;
			}
			
			case DELETE_OP: {
				Object* o = convertToObject(sp[-1], true);
				if (o == 0) {
					return true;
				}
				pop2push1(Value(o->deleteProperty(rt, sp[0])));
				break;
			}
			
			case CATCH_SCOPE_OP: {
				currentFrame = heap.govern(new(heap) Frame(0, code, heap.govern(new(heap) CatchScope(heap, scope, constants[im].getString(), sp[0])), thisObject, currentFrame));
				pop(1);
				return true;
			}
			
			case WITH_SCOPE_OP: {
				Object* o = convertToObject(sp[0], false);
				if (o == 0) {
					return true;
				}
				currentFrame = heap.govern(new(heap) Frame(0, code, heap.govern(new(heap) WithScope(heap, scope, o)), thisObject, currentFrame));
				pop(1);
				return true;
			}
			
			case POP_FRAME_OP: {
				popFrame();
				return true;
			}
			
			case TRY_OP: {
				firstCatcher = heap.govern(new(heap) Catcher(ip + im, sp, currentFrame, firstCatcher));
				break;
			}
			case TRIED_OP: assert(firstCatcher != 0); firstCatcher = firstCatcher->nextCatcher; break;
			case THROW_OP: throwVirtualException(sp[0]); return true;
			case IN_OP: {
				const Object* o = sp[0].asObject();
				if (o == 0) {
					error(TYPE_ERROR, heap.govern(new(heap) String(CANNOT_USE_IN_OPERATOR_STRING, *sp[0].toString(heap), &heap)));
					return true;
				}
				Value dummy;
				sp[-1] = Value(o->getPropertyDeep(rt, sp[-1], &dummy) != NONEXISTENT);
				pop(1);
				break;
			}
			case INSTANCE_OF_OP: {
				Function* f = sp[0].asFunction();
				if (f == 0) {
					error(TYPE_ERROR, heap.govern(new(heap) String(*sp[0].toString(heap), IS_NOT_A_FUNCTION_STRING, &heap)));
					return true;
				}
				sp[-1] = Value(f->hasInstance(rt, *this, sp[-1].asObject()));
				pop(1);
				break;
			}
			case TYPEOF_OP: sp[0] = Value(sp[0].typeOfString()); break;
			case TYPEOF_NAMED_OP: {
				Value v(Value::UNDEFINED);
				scope->readVar(rt, constants[im].getString(), &v);
				push(Value(v.typeOfString()));
				break;
			}
			
			case GET_ENUMERATOR_OP: {
				const Object* o = convertToObject(sp[0], false);
				// FIX : this is awesome and all, but it *will* become recursive nextPropertyName() calls, and recursive C++ is baaad for sandboxing
				// FIX : also, I don't like the order: prototypes are first (not that it matters much though)
				if (o == 0) {
					return true;
				}
				Enumerator* enumerator = &EMPTY_ENUMERATOR;
				do {
					enumerator = heap.govern(new(heap) JoiningEnumerator(rt, enumerator, o));
					o = o->getPrototype(rt);
				} while (o != 0);
				sp[0] = Value(enumerator);
				break;
			}
			
			case NEXT_PROPERTY_OP: {
				Object* o = sp[0].getObject();
				assert(dynamic_cast<Enumerator*>(o) != 0);
				const String* name = reinterpret_cast<Enumerator*>(o)->nextPropertyName();
 				if (name != 0) {
					sp[0] = Value(name);
				} else {
					ip += im;
					pop(1);
				}
				break;
			}
				
			default: assert(0);
		}
	}
	return true;
}

Value Processor::getResult() const {
	assert(ip == 0 && sp == stack.begin()); // make sure you've called run() until it returns false
	return sp[0];
}

bool Processor::run() {
	if (ip == 0) {
		return false;
	}
	try {
		while (cyclesLeft >= 0) {
			if (!privateRun()) {
				return false;
			}
		}
	}
	catch (const Exception& x) {
		throwVirtualException(x.getValue());
		return true;
	}
	return true;
}

static int recursiveStackCheck(const Code& code, QuickArray<Int32>& stackDepths
		, Int32 offset, Int32 currentStackDepth, Int32& maxStackDepth) {
	const CodeWord* codeWords = code.getCodeWords();
	const UInt32 codeSize = code.getCodeSize();
	
	int errors = 0;
	for (; offset < codeSize; ++offset) {
		if (stackDepths[offset] != DEAD_CODE_STACK_DEPTH) {
			if (stackDepths[offset] != currentStackDepth) {
				std::cerr << "Conflicting stack depth @" << offset << ": " << currentStackDepth << " & " << stackDepths[offset] << std::endl;
				++errors;
			}
			return errors;
		}

		const std::pair<Processor::Opcode, Int32> pair = Processor::unpackInstruction(codeWords[offset]);
		const Processor::Opcode opcode = pair.first;
		const Int32 operand = pair.second;
		if (opcode < 0 || opcode >= Processor::OP_COUNT) {
			std::cerr << "Invalid opcode " << static_cast<int>(opcode) << "@" << offset << std::endl;
			++errors;
		}
		stackDepths[offset] = currentStackDepth;
		if (currentStackDepth > code.getMaxStackDepth()) {
			std::cerr << "Stack overflow (code max is " << code.getMaxStackDepth() << ", stack depth @" << offset << " is " << currentStackDepth << ")" << std::endl;
			++errors;
		}
		maxStackDepth = std::max(maxStackDepth, currentStackDepth);
#if (VERIFY_STACK_LOGIC)
	if (stackDepths[offset] != code.stackDepths[offset]) {
		std::cerr << "Invalid compile-time stack depth @" << offset << " (" << code.stackDepths[offset] << " vs " << stackDepths[offset] << ")" << std::endl;
		++errors;
	}
#endif
		const Processor::OpcodeInfo& info = Processor::getOpcodeInfo(opcode);
		const Int32 thisStackUse = info.stackUse + (((info.flags & Processor::OpcodeInfo::POP_OPERAND) != 0) ? -operand : 0);
		currentStackDepth += thisStackUse;
		maxStackDepth = std::max(maxStackDepth, currentStackDepth);
		if (currentStackDepth < 0) {
			std::cerr << "Negative stack depth @" << offset << std::endl;
			++errors;
		}
		switch (opcode) {
			case Processor::JT_OP:
			case Processor::JF_OP:
			case Processor::JT_OR_POP_OP:
			case Processor::JF_OR_POP_OP:
			case Processor::JMP_OP:
			case Processor::JSR_OP:
			case Processor::TRY_OP:
			case Processor::NEXT_PROPERTY_OP: {
				const Int32 targetOffset = offset + 1 + operand;
				if (targetOffset < 0 || targetOffset >= codeSize) {
					std::cerr << "Invalid target @" << offset << ": @" << targetOffset << std::endl;
					++errors;
					return errors;
				}
				const Int32 targetStackDepth = currentStackDepth + (((info.flags & Processor::OpcodeInfo::NO_POP_ON_BRANCH) != 0) ? 1 : 0)
						+ (((info.flags & Processor::OpcodeInfo::POP_ON_BRANCH) != 0) ? -1 : 0);
				errors += recursiveStackCheck(code, stackDepths, targetOffset, targetStackDepth, maxStackDepth);
				if (opcode == Processor::JMP_OP) {
					return errors;
				}
				break;
			}
			case Processor::RETURN_OP: // fix: check all returns from jsr end with the same stack pointer as entry (keep track with extra parameter that only updates with JSR)
			case Processor::THROW_OP: {
				return errors;
			}
	
			default: break;
		}
	}

	return errors;
}

static void disassemble(Heap& heap, const Code& code) {
	const CodeWord* codeWords = code.getCodeWords();
	const Value* constants = code.getConstantsPointer();
	const UInt32 codeSize = code.getCodeSize();
	
	QuickArray<Int32> stackDepths(codeSize, &heap);
	std::fill(stackDepths.begin(), stackDepths.end(), DEAD_CODE_STACK_DEPTH);
	
	Int32 maxStackDepth = 0;
	int errors = recursiveStackCheck(code, stackDepths, 0, 1, maxStackDepth);
	if (maxStackDepth != code.getMaxStackDepth()) {
		std::cerr << "Notice: code definition max stack depth is " << code.getMaxStackDepth() << " but max stack depth actually used is " << maxStackDepth << std::endl;
	}
	
	for (UInt32 offset = 0; offset < codeSize; ++offset) {
		const std::pair<Processor::Opcode, Int32> pair = Processor::unpackInstruction(codeWords[offset]);
		const Processor::Opcode opcode = pair.first;
		const Int32 operand = pair.second;
		if (opcode < 0 || opcode >= Processor::OP_COUNT) {
			break;
		}

		if (stackDepths[offset] == DEAD_CODE_STACK_DEPTH) {
			std::cerr << "?";
		} else {
			std::cerr << stackDepths[offset];
		}
		std::cerr << "\t";
#if (VERIFY_STACK_LOGIC)
		if (code.stackDepths[offset] == DEAD_CODE_STACK_DEPTH) {
			std::cerr << "?";
		} else {
			std::cerr << code.stackDepths[offset];
		}
		std::cerr << "\t";
#endif
		std::cerr << "@" << offset << ":\t" << Processor::getOpcodeInfo(opcode).mnemonic;
		
		switch (opcode) {
			case Processor::JT_OP:
			case Processor::JF_OP:
			case Processor::JT_OR_POP_OP:
			case Processor::JF_OR_POP_OP:
			case Processor::JMP_OP:
			case Processor::JSR_OP:
			case Processor::TRY_OP:
			case Processor::NEXT_PROPERTY_OP: {
				const Int32 target = offset + 1 + operand;
				std::cerr << " @" << target;
				break;
			}
			case Processor::READ_LOCAL_OP:
			case Processor::WRITE_LOCAL_OP:
			case Processor::WRITE_LOCAL_POP_OP: {
				const Int32 index = operand;
				const String* name = code.getLocalName(index);
				if (name != 0) std::wcerr << L" $" << name->toStdString();
				std::wcerr << L" (" << index << L")";
				break;
			}
			case Processor::CALL_OP:
			case Processor::CALL_METHOD_OP:
			case Processor::CALL_EVAL_OP:
			case Processor::NEW_OP:
			case Processor::POP_OP:
			case Processor::PUSH_BACK_OP: std::cerr << " *" << operand; break;
			case Processor::REPUSH_OP: std::cerr << " " << operand; break;
			case Processor::CONST_OP:
			case Processor::GEN_FUNC_OP:
			case Processor::CATCH_SCOPE_OP: std::wcerr << L" #" << constants[operand].toString(heap)->toStdString(); break;
			case Processor::DECLARE_OP:
			case Processor::READ_NAMED_OP:
			case Processor::WRITE_NAMED_OP:
			case Processor::WRITE_NAMED_POP_OP:
			case Processor::ADD_PROPERTY_OP:
			case Processor::DELETE_NAMED_OP:
			case Processor::TYPEOF_NAMED_OP: std::wcerr << L" " << constants[operand].toString(heap)->toStdString(); break;
			default: break;
		}
		std::cerr << std::endl;
	}
	
	assert(errors == 0);
}

/* --- Compiler --- */

// FIX : sort
enum OperatorType {
	GROUP, UNARY, BINARY, IN_OPERATOR, ABSTRACT_EQUAL, ASSIGNMENT, COMPOUND_ASSIGNMENT, PRE_INC_DEC, POST_INC_DEC
	, LOGICAL_AND_OR, PROPERTY_BRACKETS, PROPERTY_DOT, CONDITIONAL, FUNCTION_CALL, COMMA, VOID_OPERATOR
	, DELETE_OPERATOR, NEW_OPERATOR, TYPE_OF
};
enum Associativity { LEFT_TO_RIGHT, RIGHT_TO_LEFT };

struct OperatorInfo {
	const char* token;
	const char* closeToken;
	Precedence prec;
	Associativity assoc;
	OperatorType type;
	Processor::Opcode vmOp;
	bool primitiveInput;
	bool primitiveOutput;
};

// FIX : sort these two
const OperatorInfo PRE_OPS[] = {
	{ "(", ")", GROUP_PREC, RIGHT_TO_LEFT, GROUP, Processor::INVALID_OP, false, false },
	{ "++", 0, PREFIX_PREC, RIGHT_TO_LEFT, PRE_INC_DEC, Processor::INC_OP, true, true },
	{ "--", 0, PREFIX_PREC, RIGHT_TO_LEFT, PRE_INC_DEC, Processor::DEC_OP, true, true },
	{ "+", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::PLUS_OP, true, true },
	{ "-", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::MINUS_OP, true, true },
	{ "~", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::INV_OP, true, true },
	{ "!", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::NOT_OP, false, true },
	{ "void", 0, PREFIX_PREC, RIGHT_TO_LEFT, VOID_OPERATOR, Processor::VOID_OP, false, true },
	{ "delete", 0, PREFIX_PREC, RIGHT_TO_LEFT, DELETE_OPERATOR, Processor::VOID_OP, false, true },
	{ "new", 0, NEW_PREC, RIGHT_TO_LEFT, NEW_OPERATOR, Processor::VOID_OP, false, false },
	{ "typeof", 0, PREFIX_PREC, RIGHT_TO_LEFT, TYPE_OF, Processor::TYPEOF_OP, false, true }
};

const OperatorInfo POST_OPS[] = {
	{ "===", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, Processor::X_EQ_OP, false, true },
	{ "==", 0, EQUALITY_PREC, LEFT_TO_RIGHT, ABSTRACT_EQUAL, Processor::EQ_OP, false, true },
	{ "=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::INVALID_OP, false, false },
    { ">>>=", 0, SHIFT_PREC, RIGHT_TO_LEFT, COMPOUND_ASSIGNMENT, Processor::USHR_OP, true, true },
	{ ">>=", 0, SHIFT_PREC, RIGHT_TO_LEFT, COMPOUND_ASSIGNMENT, Processor::SHR_OP, true, true },
	{ "<<=", 0, SHIFT_PREC, RIGHT_TO_LEFT, COMPOUND_ASSIGNMENT, Processor::SHL_OP, true, true },
	{ "+=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, COMPOUND_ASSIGNMENT, Processor::ADD_OP, true, true },
	{ "-=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, COMPOUND_ASSIGNMENT, Processor::SUB_OP, true, true },
	{ "*=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, COMPOUND_ASSIGNMENT, Processor::MUL_OP, true, true },
	{ "/=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, COMPOUND_ASSIGNMENT, Processor::DIV_OP, true, true },
	{ "%=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, COMPOUND_ASSIGNMENT, Processor::MOD_OP, true, true },
	{ "&=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, COMPOUND_ASSIGNMENT, Processor::AND_OP, true, true },
	{ "^=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, COMPOUND_ASSIGNMENT, Processor::XOR_OP, true, true },
	{ "|=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, COMPOUND_ASSIGNMENT, Processor::OR_OP, true, true },
	{ ">>>", 0, SHIFT_PREC, LEFT_TO_RIGHT, BINARY, Processor::USHR_OP, true, true },
	{ ">>", 0, SHIFT_PREC, LEFT_TO_RIGHT, BINARY, Processor::SHR_OP, true, true },
	{ "<<", 0, SHIFT_PREC, LEFT_TO_RIGHT, BINARY, Processor::SHL_OP, true, true },
	{ "++", 0, POST_INC_DEC_PREC, LEFT_TO_RIGHT, POST_INC_DEC, Processor::INC_OP, true, true },
	{ "--", 0, POST_INC_DEC_PREC, LEFT_TO_RIGHT, POST_INC_DEC, Processor::DEC_OP, true, true },
	{ "&&", 0, LOGICAL_AND_PREC, RIGHT_TO_LEFT, LOGICAL_AND_OR, Processor::JF_OR_POP_OP, false, false },
	{ "||", 0, LOGICAL_OR_PREC, RIGHT_TO_LEFT, LOGICAL_AND_OR, Processor::JT_OR_POP_OP, false, false },
	{ "+", 0, ADD_SUB_PREC, LEFT_TO_RIGHT, BINARY, Processor::ADD_OP, true, true },
	{ "-", 0, ADD_SUB_PREC, LEFT_TO_RIGHT, BINARY, Processor::SUB_OP, true, true },
	{ "*", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, Processor::MUL_OP, true, true },
	{ "/", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, Processor::DIV_OP, true, true },
	{ "%", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, Processor::MOD_OP, true, true },
	{ "&", 0, BIT_AND_PREC, LEFT_TO_RIGHT, BINARY, Processor::AND_OP, true, true },
	{ "^", 0, BIT_XOR_PREC, LEFT_TO_RIGHT, BINARY, Processor::XOR_OP, true, true },
	{ "|", 0, BIT_OR_PREC, LEFT_TO_RIGHT, BINARY, Processor::OR_OP, true, true },
	{ "!==", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, Processor::X_NEQ_OP, false, true },
	{ "!=", 0, EQUALITY_PREC, LEFT_TO_RIGHT, ABSTRACT_EQUAL, Processor::NEQ_OP, false, true },
	{ "<=", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::LEQ_OP, true, true },
	{ "<", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::LT_OP, true, true },
	{ ">=", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::GEQ_OP, true, true },
	{ ">", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::GT_OP, true, true },
	{ ".", 0, PROPERTY_PREC, LEFT_TO_RIGHT, PROPERTY_DOT, Processor::INVALID_OP, false, false },
	{ "[", "]", PROPERTY_PREC, LEFT_TO_RIGHT, PROPERTY_BRACKETS, Processor::INVALID_OP, false, false },
	{ "?", ":", CONDITIONAL_PREC, RIGHT_TO_LEFT, CONDITIONAL, Processor::INVALID_OP, false, false },
	{ "(", ")", FUNCTION_CALL_PREC, LEFT_TO_RIGHT, FUNCTION_CALL, Processor::INVALID_OP, false, false },
	{ ",", 0, COMMA_PREC, LEFT_TO_RIGHT, COMMA, Processor::INVALID_OP, false, false },
	{ "instanceof", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::INSTANCE_OF_OP, false, true },
	{ "in", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, IN_OPERATOR, Processor::IN_OP, false, true }
};

// FIX : sort
enum Statement {
	VAR_STATEMENT, IF_STATEMENT, WHILE_STATEMENT, DO_WHILE_STATEMENT, FOR_STATEMENT, RETURN_STATEMENT
	, CONTINUE_STATEMENT, BREAK_STATEMENT, TRY_STATEMENT, THROW_STATEMENT, SWITCH_STATEMENT, FUNCTION_STATEMENT
	, WITH_STATEMENT
};
enum Literal { NULL_LITERAL, FALSE_LITERAL, TRUE_LITERAL, FUNCTION_LITERAL, THIS_LITERAL };

struct Compiler::SemanticScope {
	enum Type { ROOT_TYPE, LABEL_TYPE, ITERATOR_LABEL_TYPE, TRY_TYPE, CATCH_TYPE, FINALLY_TYPE, WITH_TYPE };
	SemanticScope(Heap& heap, Type type, Int32 stackDepthOnEntry, SemanticScope* next)
			: breaks(&heap), continues(&heap), finallys(&heap), type(type), stackDepthOnEntry(stackDepthOnEntry), next(next) { }
	SemanticScope(Heap& heap, const String& label, Int32 stackDepthOnEntry, SemanticScope* next)
			: breaks(&heap), continues(&heap), finallys(&heap), type(LABEL_TYPE), label(label)
			, stackDepthOnEntry(stackDepthOnEntry), next(next) { }
	void makeIteratorScopes(SemanticScope* untilScope) {
		for (SemanticScope* s = this; s != untilScope; s = s->next) {
			s->type = ITERATOR_LABEL_TYPE;
		}
	}
	
	Int32 popStack(Compiler& compiler, Int32 currentDepth, Processor::Opcode popOpcode) {
		if (!compiler.currentSection->inDeadCode()) {
			const Int32 stackDistance = currentDepth - stackDepthOnEntry;
			if (stackDistance > 0) {
				compiler.emit(popOpcode, stackDistance);
				currentDepth -= stackDistance;
			}
		}
		return currentDepth;
	}
	
	Int32 rollback(Compiler& compiler, Int32 rollbackStackDepth, Processor::Opcode popOpcode) {
		switch (type) {
			case CATCH_TYPE:
			case WITH_TYPE:
			case FINALLY_TYPE: compiler.emit(Processor::POP_FRAME_OP); break;
			default: break;
		}
		switch (type) {
			case TRY_TYPE:
			case CATCH_TYPE:
			case ROOT_TYPE: rollbackStackDepth = popStack(compiler, rollbackStackDepth, popOpcode); break;
			default: break;
		}
		switch (type) {
			case TRY_TYPE:
			case CATCH_TYPE: finallys.push(compiler.emitForwardBranch(Processor::JSR_OP)); break;
			default: break;
		}
		return rollbackStackDepth;
	}
	
	Type type;
	const String label; // empty for automatic while, for, case labels
	SemanticScope* const next;
	Int32 stackDepthOnEntry;
	QuickVector<BranchPoint> breaks; // source points for break jmp's
	QuickVector<BranchPoint> continues; // source points for continue jmp's
	QuickVector<BranchPoint> finallys; // source points for finally jsr's
};

Compiler::Compiler(Heap& heap, Code* code, Target compileFor)
		: super(heap), heap(heap), code(code), compilingFor(compileFor), setupSection(heap, 1)
		, mainSection(heap, 1), p(0), e(0), currentSection(0), acceptInOperator(true), withScopeCounter(0) { }

void Compiler::error(ErrorType errorType, const char* message) { Exception::throwError(heap, errorType, message); }

void Compiler::CodeSection::emit(Processor::Opcode opcode, Int32 operand) {
	if (inDeadCode()) {	// unknown stack depth = dead code
		return;
	}
#if (VERIFY_STACK_LOGIC)
	stackDepths.push(stackDepth);
#endif
	const Processor::OpcodeInfo& opcodeInfo = Processor::getOpcodeInfo(opcode);
	stackDepth += opcodeInfo.stackUse + (((opcodeInfo.flags & Processor::OpcodeInfo::POP_OPERAND) != 0) ? -operand : 0);
	assert(stackDepth >= 0);
	maxStackDepth = std::max(maxStackDepth, stackDepth);		// Yeah, if we eliminate a (void/const/repush, pop) pair this might be one more than necessary, but it never seems to happen from emperical tests and it doesn't really matter in the first place.
	if ((opcodeInfo.flags & Processor::OpcodeInfo::TERMINAL) != 0) {
		stackDepth = DEAD_CODE_STACK_DEPTH;
	}
	if (opcode == Processor::POP_OP && operand == 1) {
		Processor::Opcode replacementOpcode = Processor::INVALID_OP;
		switch (lastEmitted) {
			case Processor::WRITE_LOCAL_OP: replacementOpcode = Processor::WRITE_LOCAL_POP_OP; break;
			case Processor::WRITE_NAMED_OP: replacementOpcode = Processor::WRITE_NAMED_POP_OP; break;
			case Processor::SET_PROPERTY_OP: replacementOpcode = Processor::SET_PROPERTY_POP_OP; break;
			case Processor::REPUSH_OP:
			case Processor::CONST_OP:
			case Processor::VOID_OP: {
				code.pop();
#if (VERIFY_STACK_LOGIC)
	stackDepths.pop(2);
#endif
				lastEmitted = Processor::INVALID_OP;
				return;
			}
			default: break;
		}
		if (replacementOpcode != Processor::INVALID_OP) {
			const Int32 oldOperand = Processor::unpackInstruction(code.end()[-1]).second;
			code.pop();
#if (VERIFY_STACK_LOGIC)
	stackDepths.pop();
#endif
			code.push(Processor::packInstruction(replacementOpcode, oldOperand));
			lastEmitted = replacementOpcode;
			return;
		}
	}
	code.push(Processor::packInstruction(opcode, operand));
	lastEmitted = opcode;
}

void Compiler::CodeSection::insertSection(const CodeSection& section) {
	const Int32 stackAdjust = stackDepth - section.initialStackDepth;
	lastEmitted = Processor::INVALID_OP;
	code.insert(code.end(), section.code.begin(), section.code.end());
#if (VERIFY_STACK_LOGIC)
	for (int i = 0; i < section.stackDepths.size(); ++i) {
		stackDepths.push(section.stackDepths[i] + stackAdjust);
	}
#endif
	stackDepth = section.stackDepth + stackAdjust;
	maxStackDepth = std::max(maxStackDepth, section.maxStackDepth + stackAdjust);
}

void Compiler::emit(Processor::Opcode opcode, Int32 operand) {
	if (operand < MIN_OPERAND_VALUE || operand > MAX_OPERAND_VALUE) { // Highly hypothetical, but correctness!
		error(GENERIC_ERROR, "Internal compiler limitations reached. Reduce code complexity.");
	}
	currentSection->emit(opcode, operand);
}

Compiler::CodeSection* Compiler::changeSection(CodeSection* newOutputSection) {
	std::swap(newOutputSection, currentSection);
	return newOutputSection;
}

Compiler::BranchPoint Compiler::emitForwardBranch(Processor::Opcode opcode) {
// FIX : share code somehow
	const Processor::OpcodeInfo& opcodeInfo = Processor::getOpcodeInfo(opcode);
	const Int32 operand = 0;
	const Int32 targetStackDepth = (currentSection->inDeadCode() ? DEAD_CODE_STACK_DEPTH
			: currentSection->stackDepth
			+ opcodeInfo.stackUse + (((opcodeInfo.flags & Processor::OpcodeInfo::POP_OPERAND) != 0) ? -operand : 0)
			+ (((opcodeInfo.flags & Processor::OpcodeInfo::NO_POP_ON_BRANCH) != 0) ? 1 : 0)
			+ (((opcodeInfo.flags & Processor::OpcodeInfo::POP_ON_BRANCH) != 0) ? -1 : 0));
	emit(opcode);
	return BranchPoint(currentSection->code.size() - 1, targetStackDepth);
}

void Compiler::completeForwardBranch(const BranchPoint& point) {
	assert(currentSection->inDeadCode() || point.inDeadCode() || point.stackDepth == currentSection->stackDepth);
	if (!point.inDeadCode()) {
		currentSection->lastEmitted = Processor::INVALID_OP;
		currentSection->stackDepth = point.stackDepth;
		currentSection->maxStackDepth = std::max(currentSection->maxStackDepth, point.stackDepth);
		const std::pair<Processor::Opcode, Int32> instruction = Processor::unpackInstruction(currentSection->code[point.codeOffset]);
		currentSection->code[point.codeOffset] = Processor::packInstruction(instruction.first, currentSection->code.size() - point.codeOffset - 1);
	}
}

void Compiler::completeForwardBranches(const BranchPoint* begin, const BranchPoint* end) {
	for (const BranchPoint* it = begin; it != end; ++it) {
		completeForwardBranch(*it);
	}
}

Compiler::BranchPoint Compiler::markBackwardBranch() {
	currentSection->lastEmitted = Processor::INVALID_OP;
	return BranchPoint(currentSection->code.size(), currentSection->stackDepth);
}

void Compiler::emitBackwardBranch(Processor::Opcode opcode, const BranchPoint& point) {
	if (!currentSection->inDeadCode()) {
		assert(!point.inDeadCode() && 0 <= point.codeOffset && point.codeOffset < currentSection->code.size());
		emit(opcode, point.codeOffset - 1 - currentSection->code.size());
	}
}

UInt32 Compiler::addConstant(const Value& constant) {
	Constants* constants = code->constants;
	for (UInt32 index = constants->size(); index > 0;) {
		if ((*constants)[--index].isStrictlyEqualTo(constant)) {
			return index;
		}
	}
	constants->push(constant);
	return constants->size() - 1;
}

void Compiler::emitWithConstant(Processor::Opcode opcode, const Value& constant) {
	emit(opcode, addConstant(constant));
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

Compiler::ExpressionResult Compiler::makeRValue(const ExpressionResult& xr, bool toPrimitive, bool preferString) {
	switch (xr.t) {
		case ExpressionResult::PUSHED: break;
		case ExpressionResult::PUSHED_PRIMITIVE: return ExpressionResult(ExpressionResult::PUSHED_PRIMITIVE);
		case ExpressionResult::NONE: emit(Processor::VOID_OP); return ExpressionResult(ExpressionResult::PUSHED_PRIMITIVE);
		case ExpressionResult::CONSTANT: emitWithConstant(Processor::CONST_OP, xr.v); return ExpressionResult(ExpressionResult::PUSHED_PRIMITIVE);
		case ExpressionResult::LOCAL: emit(Processor::READ_LOCAL_OP, xr.v.toInt()); break;
		case ExpressionResult::NAMED: emitWithConstant(Processor::READ_NAMED_OP, xr.v); break;
		case ExpressionResult::PROPERTY: emit(Processor::GET_PROPERTY_OP); break;
		case ExpressionResult::SAFEKEPT: emit(Processor::REPUSH_OP, xr.v.toInt() - currentSection->stackDepth + 1); break;
		default: assert(0);
	}
	if (toPrimitive) {
		emit(preferString ? Processor::OBJ_TO_STRING_OP : Processor::OBJ_TO_NUMBER_OP);
		return ExpressionResult(ExpressionResult::PUSHED_PRIMITIVE);
	} else {
		return ExpressionResult(ExpressionResult::PUSHED);
	}
}

Processor::Opcode Compiler::evalPopOpcode() const {
	return compilingFor == FOR_EVAL ? Processor::PUSH_BACK_OP : Processor::POP_OP;
}

Compiler::ExpressionResult Compiler::safeKeep() {
	Int32 stackOffset = 1;
	if (compilingFor == FOR_EVAL) {
		emit(Processor::SWAP_OP);				// evaluation result should be up front always
		stackOffset = 2;
	}
	assert(currentSection->inDeadCode() || currentSection->stackDepth >= stackOffset);
	return ExpressionResult(ExpressionResult::SAFEKEPT, Value(currentSection->stackDepth - stackOffset));
}

void Compiler::returnSafeKept(const ExpressionResult& xr) {
	assert(xr.t == ExpressionResult::SAFEKEPT);
	assert(currentSection->inDeadCode() || xr.v.toInt() == currentSection->stackDepth - (compilingFor == FOR_EVAL ? 2 : 1));
	emit(evalPopOpcode(), 1);
}

Compiler::ExpressionResult Compiler::makeAssignment(const ExpressionResult& xr) {
	switch (xr.t) {
		default: error(REFERENCE_ERROR, "Illegal l-value"); // FIX : ECMA like the word "reference"
		case ExpressionResult::LOCAL: emit(Processor::WRITE_LOCAL_OP, xr.v.toInt()); break;
		case ExpressionResult::NAMED: emitWithConstant(Processor::WRITE_NAMED_OP, xr.v); break;
		case ExpressionResult::PROPERTY: emit(Processor::SET_PROPERTY_OP); break;
	}
	return ExpressionResult(ExpressionResult::PUSHED);
}

Compiler::ExpressionResult Compiler::discard(const ExpressionResult& xr) {
	switch (xr.t) {
		case ExpressionResult::PUSHED:
		case ExpressionResult::PUSHED_PRIMITIVE: emit(Processor::POP_OP, 1); break;
		case ExpressionResult::NAMED: emitWithConstant(Processor::READ_NAMED_OP, xr.v); emit(Processor::POP_OP, 1); break;
		case ExpressionResult::PROPERTY: emit(Processor::GET_PROPERTY_OP); emit(Processor::POP_OP, 1); break;
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

Compiler::ExpressionResult Compiler::functionCall(Processor::Opcode opcode) {
	const bool didAcceptInOperator = acceptInOperator;
	acceptInOperator = true;
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
	acceptInOperator = didAcceptInOperator;
	emit(opcode, count);
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
		ExpressionResult xr;
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

int Compiler::parseOperator(Precedence precedence, int opCount, const OperatorInfo* opList) {
    white();
	if (eof()) {
		return -1;
	}
	int i = 0;
	while (i < opCount && !token(opList[i].token, false)) {
		++i;
	}
	if (i >= opCount) {
		return -1;
	}
	const OperatorInfo& op = opList[i];
	if (precedence < op.prec + (op.assoc == RIGHT_TO_LEFT ? 0 : 1)) {
		return -1;
	}
	return i;
}

bool Compiler::preOperate(ExpressionResult& xr, Precedence precedence) {
	const int opIndex = parseOperator(precedence, sizeof (PRE_OPS) / sizeof (*PRE_OPS), PRE_OPS);
	if (opIndex < 0) {
		return false;
	}
	xr = discard(xr);
	const OperatorInfo& op = PRE_OPS[opIndex];
	switch (op.type) {
        case VOID_OPERATOR: {
            xr = discard(operand(op));
            break;
        }
		
        case GROUP: {
			const bool didAcceptInOperator = acceptInOperator;
			acceptInOperator = true;
            xr = operand(op);
			acceptInOperator = didAcceptInOperator;
            break;
        }
		
		case UNARY: {
			makeRValue(operand(op), op.primitiveInput);
			emit(op.vmOp);
            xr = (op.primitiveOutput ? ExpressionResult::PUSHED_PRIMITIVE : ExpressionResult::PUSHED);
			break;
		}

		case NEW_OPERATOR: {
			makeRValue(operand(op), false);
			if (!token("(", true)) {
				emit(Processor::NEW_OP, 0);
			} else {
				white();
				functionCall(Processor::NEW_OP);
			}
			emit(Processor::NEW_RESULT_OP); // FIX : make a chain for script constructors and require returning object for native constructors instead?
            assert(!op.primitiveOutput);
            xr = (op.primitiveOutput ? ExpressionResult::PUSHED_PRIMITIVE : ExpressionResult::PUSHED);
			break;
		}
		
		case DELETE_OPERATOR: {
            assert(!op.primitiveInput);
            assert(op.primitiveOutput);
			xr = operand(op);
			switch (xr.t) {
				case ExpressionResult::PUSHED:
				case ExpressionResult::PUSHED_PRIMITIVE: emit(Processor::POP_OP, 1); /* fall through */
				case ExpressionResult::NONE:
				case ExpressionResult::CONSTANT: xr = ExpressionResult(ExpressionResult::CONSTANT, Value(true)); break;
				case ExpressionResult::LOCAL: xr = ExpressionResult(ExpressionResult::CONSTANT, Value(false)); break;
				case ExpressionResult::NAMED: emitWithConstant(Processor::DELETE_NAMED_OP, xr.v); xr = ExpressionResult(ExpressionResult::PUSHED_PRIMITIVE); break;
				case ExpressionResult::PROPERTY: emit(Processor::DELETE_OP); xr = ExpressionResult(ExpressionResult::PUSHED_PRIMITIVE); break;
				default: assert(0);
			}
			break;
		}
		
		case PRE_INC_DEC: {
            assert(op.primitiveInput);
            assert(op.primitiveOutput);
			xr = operand(op);
			if (xr.t == ExpressionResult::PROPERTY) {
				emit(Processor::REPUSH_2_OP);
			}
			makeRValue(xr, true);
			emit(op.vmOp);
			makeAssignment(xr);
            xr = ExpressionResult::PUSHED_PRIMITIVE;
			break;
		}
		
		case TYPE_OF: {
            assert(!op.primitiveInput);
            assert(op.primitiveOutput);
			xr = operand(op);
			if (xr.t == ExpressionResult::NAMED) {
				emitWithConstant(Processor::TYPEOF_NAMED_OP, xr.v);
			} else {
				makeRValue(xr, false);
				emit(op.vmOp);
			}
            xr = ExpressionResult::PUSHED_PRIMITIVE;
			break;
		}
		
		default: assert(0);
	}
	return true;
}

bool Compiler::postOperate(ExpressionResult& xr, Precedence precedence) {
	const Char* b = p;
	const int opIndex = parseOperator(precedence, sizeof (POST_OPS) / sizeof (*POST_OPS), POST_OPS);
	if (opIndex < 0) {
		return false;
	}
	const OperatorInfo& op = POST_OPS[opIndex];
	switch (op.type) {
        case COMMA: {
            xr = discard(xr);
            xr = makeRValue(operand(op));
            break;
        }
            
		case BINARY: {
			makeRValue(xr, op.primitiveInput);
			makeRValue(operand(op), op.primitiveInput);
			emit(op.vmOp);
            xr = (op.primitiveOutput ? ExpressionResult::PUSHED_PRIMITIVE : ExpressionResult::PUSHED);
			break;
		}
		
		case ABSTRACT_EQUAL: {
			assert(!op.primitiveInput);
            assert(op.primitiveOutput);
			makeRValue(xr, false);
			makeRValue(operand(op), op.primitiveInput);
			emit(Processor::PRE_EQ_OP);
			emit(op.vmOp);
            xr = ExpressionResult::PUSHED_PRIMITIVE;
			break;
		}

		case IN_OPERATOR: {
			if (!acceptInOperator) {
				return false;
			}
			assert(!op.primitiveInput);
            assert(op.primitiveOutput);
			makeRValue(xr, true, true); // left should be primitive, right doesn't have to
			makeRValue(operand(op), false);
			emit(op.vmOp);
            xr = ExpressionResult::PUSHED_PRIMITIVE;
			break;
		}

		case POST_INC_DEC: {
            assert(op.primitiveInput);
            assert(op.primitiveOutput);
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
			makeAssignment(xr);
			emit(Processor::POP_OP, 1);
            xr = ExpressionResult::PUSHED_PRIMITIVE;
			break;
		}
		
		case LOGICAL_AND_OR: {
			assert(!op.primitiveInput);
            assert(!op.primitiveOutput);
			makeRValue(xr, false);
			const BranchPoint point = emitForwardBranch(op.vmOp);
			makeRValue(operand(op), false);
			completeForwardBranch(point);
            xr = ExpressionResult::PUSHED;
			break;
		}
		
		case CONDITIONAL: {
			assert(!op.primitiveInput);
            assert(!op.primitiveOutput);
			makeRValue(xr, false);
			const BranchPoint falsePoint = emitForwardBranch(Processor::JF_OP);
			makeRValue(operand(op), false);
			const BranchPoint endPoint = emitForwardBranch(Processor::JMP_OP);
			completeForwardBranch(falsePoint);
			rvalueExpression(ASSIGN_PREC);
			completeForwardBranch(endPoint);
            xr = ExpressionResult::PUSHED;
			break;
		}
		
		case PROPERTY_DOT: {
			assert(!op.primitiveInput);
			makeRValue(xr, op.primitiveInput);
			white();
			const Char* b = identifier(true, true);
			emitWithConstant(Processor::CONST_OP, Value(String::newHashedString(heap, b, p)));
			xr = ExpressionResult(ExpressionResult::PROPERTY);
			break;
		}
		
		case FUNCTION_CALL: {
            assert(!op.primitiveInput);
            assert(!op.primitiveOutput);
			Processor::Opcode callOp = Processor::CALL_OP;
			if (xr.t == ExpressionResult::NAMED && xr.v.isEqualTo(EVAL_STRING)) {
				callOp = Processor::CALL_EVAL_OP;
			}
			if (xr.t == ExpressionResult::PROPERTY) {
				callOp = Processor::CALL_METHOD_OP;
			} else {
				makeRValue(xr, false);
			}
			functionCall(callOp);
            xr = ExpressionResult::PUSHED;
			break;
		}
		
        case ASSIGNMENT: {
            assert(!op.primitiveInput);
            assert(!op.primitiveOutput);
            const ExpressionResult rxr = makeRValue(operand(op), false);
            makeAssignment(xr);
            xr = rxr;
            break;
        }
            
        case COMPOUND_ASSIGNMENT: {
            assert(op.primitiveInput);
            assert(op.primitiveOutput);
            if (xr.t == ExpressionResult::PROPERTY) {
                emit(Processor::REPUSH_2_OP);
            }
            makeRValue(xr, true);
            makeRValue(operand(op), true);
            emit(op.vmOp);
            makeAssignment(xr);
            xr = ExpressionResult::PUSHED_PRIMITIVE;
            break;
        }
            
		case PROPERTY_BRACKETS: {
			assert(!op.primitiveInput);
			makeRValue(xr, false);
			const bool didAcceptInOperator = acceptInOperator;
			acceptInOperator = true;
			makeRValue(operand(op), true, true); // left doesn't need to be primitive, but right does (and preferred string!)
			acceptInOperator = didAcceptInOperator;
			xr = ExpressionResult(ExpressionResult::PROPERTY);
			break;
		}
		
		default: assert(0);
	}
	return true;
}

void Compiler::functionDefinition(const String* functionName, const String* selfName) {
	assert(functionName != 0);
	Code* func = heap.govern(new(heap) Code(heap, code->constants));
	Compiler funcCompiler(heap, func, Compiler::FOR_FUNCTION);
	try {
		p = funcCompiler.compileFunction(p, e, functionName, selfName);
	}
	catch (const Exception&) { // FIX : a bit Q & D ish
		p = funcCompiler.getStopPoint();
		throw;
	}
	emitWithConstant(Processor::GEN_FUNC_OP, Value(func));
}

const Int32 CATCH_PARAMETER = 0x7FFFFFFF;

bool Compiler::optionalExpression(ExpressionResult& xr, Precedence precedence) {
	if (!preOperate(xr, precedence)) {
		if (eof()) {
			return false;
		}
		const Char* b = p;
		token();
		int i = findLiteralToken(p - b, &*b);
		if (i >= 0) {
			xr = discard(xr);
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
				xr = discard(xr);
				xr = ExpressionResult(ExpressionResult::CONSTANT, v);
			} else {
				switch (*p) {
					case '{': {
						++p;
						xr = discard(xr);
						xr = objectInitialiser();
						break;
					}
					
					case '[': {
						++p;
						xr = discard(xr);
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
							xr = discard(xr);
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
						xr = discard(xr);
						const String* name = String::newHashedString(heap, b, p);
						xr = ExpressionResult(ExpressionResult::NAMED, Value(name));
						 // Notice: compilingFor != FOR_EVAL and checking for "arguments" is only for non-strict mode (if we ever support strict-mode in the future)
						if (!name->isEqualTo(ARGUMENTS_STRING) && compilingFor == FOR_FUNCTION && withScopeCounter == 0) {
							const Table::Bucket* bucket = code->nameIndexes.lookup(name);
							if (bucket != 0) {
								const Int32 index = bucket->getIndexValue();
								if (index != CATCH_PARAMETER) {
									xr = ExpressionResult(ExpressionResult::LOCAL, Value(index));
									break;
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
    while (postOperate(xr, precedence)) {
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

void Compiler::rvalueGroup() {
	expectToken("(", true);
	rvalueExpression();
	expectToken(")", true);
}

// FIX : ok, this is serious mess
Compiler::ExpressionResult Compiler::declareIdentifier(const String* name, bool func) {
	ExpressionResult lxr(ExpressionResult::NAMED, Value(name));
	if (compilingFor != FOR_FUNCTION) {
		CodeSection* previousSection = changeSection(&setupSection);
		if (!func) {
			emit(Processor::VOID_OP);
		}
		emitWithConstant(Processor::DECLARE_OP, Value(name));
		changeSection(previousSection);
		return lxr;
	} else if (!name->isEqualTo(ARGUMENTS_STRING)) {
		Int32 index;
		Table& nameIndexes = code->nameIndexes;
		Table::Bucket* bucket = nameIndexes.lookup(name);
		if (bucket != 0) {
			index = bucket->getIndexValue();
		} else {
			code->varNames.push(name);
			index = -static_cast<Int32>(code->varNames.size());
			nameIndexes.update(nameIndexes.insert(name), index);
			code->bloomSet |= name->createBloomCode();
		}
		if (withScopeCounter == 0 && index != CATCH_PARAMETER) {
			lxr = ExpressionResult(ExpressionResult::LOCAL, Value(index));
		}
	}
	if (func) {
		makeAssignment(lxr);
		emit(Processor::POP_OP, 1);
	}
	return lxr;
}

Compiler::ExpressionResult Compiler::varDeclaration() {
	const Char* b = identifier(true, false);
	ExpressionResult lxr(declareIdentifier(String::newHashedString(heap, b, p), false));
	if (token("=", true)) {
		rvalueExpression(COMMA_PREC);
		makeAssignment(lxr);
		emit(Processor::POP_OP, 1);
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

void Compiler::completeIteratorContinues(const SemanticScope* fromScope, const SemanticScope* untilScope) {
	for (const SemanticScope* s = fromScope; s != untilScope; s = s->next) {
		assert(s->type == SemanticScope::ITERATOR_LABEL_TYPE);
		completeForwardBranches(s->continues.begin(), s->continues.end());
	}
}

void Compiler::completeIteratorBreaks(const SemanticScope* fromScope, const SemanticScope* untilScope) {
	for (const SemanticScope* s = fromScope; s != untilScope; s = s->next) {
		assert(s->type == SemanticScope::ITERATOR_LABEL_TYPE);
		completeForwardBranches(s->breaks.begin(), s->breaks.end());
	}
}

void Compiler::completeBreaks(const SemanticScope* ofScope) {
	completeForwardBranches(ofScope->breaks.begin(), ofScope->breaks.end());
}

void Compiler::forInStatement(SemanticScope* emptyLabelScope, SemanticScope* scopeLabelsEnd
		, const CodeSection& iterationSection, const ExpressionResult& iterationXR) {
	ExpressionResult enumXR(rvalueExpression());
 	emit(Processor::GET_ENUMERATOR_OP);
	enumXR = safeKeep();
	expectToken(")", true);
	for (SemanticScope* s = emptyLabelScope; s != scopeLabelsEnd; s = s->next) {	// we must change stack depth of all labels that point to this scope, otherwise we'll pop the enumerator on continue
		s->stackDepthOnEntry = currentSection->stackDepth;
	}
	const BranchPoint loopPoint = markBackwardBranch();
	makeRValue(enumXR);
	const BranchPoint exitLoopPoint = emitForwardBranch(Processor::NEXT_PROPERTY_OP);
	ExpressionResult xr(ExpressionResult::PUSHED);
	if (!iterationSection.code.empty()) { // This is probably extremely rare (left hand side being anything but a directly assignable variable), so I don't mind the extra code of safe keeping here
		xr = safeKeep();
		currentSection->insertSection(iterationSection);
		makeRValue(xr);
		discard(makeAssignment(iterationXR));
		returnSafeKept(xr);
	} else {
		discard(makeAssignment(iterationXR));
	}
	statement(emptyLabelScope, emptyLabelScope);
	completeIteratorContinues(emptyLabelScope, scopeLabelsEnd);
	emitBackwardBranch(Processor::JMP_OP, loopPoint);
	completeForwardBranch(exitLoopPoint);
	completeIteratorBreaks(emptyLabelScope, scopeLabelsEnd);
	returnSafeKept(enumXR);
}

void Compiler::forStatement(SemanticScope* emptyLabelScope, SemanticScope* scopeLabelsEnd) {
	expectToken(";", true);
	const BranchPoint loopPoint = markBackwardBranch();
	BranchPoint exitLoopPoint;
	ExpressionResult testXR;
	if (optionalExpression(testXR, LOWEST_PREC)) {
		makeRValue(testXR);
		exitLoopPoint = emitForwardBranch(Processor::JF_OP);
	}
	expectToken(";", true);
	CodeSection incrementSection(heap, currentSection->stackDepth);
	{
		CodeSection* previousSection = changeSection(&incrementSection);
		ExpressionResult incXR;
		if (optionalExpression(incXR, LOWEST_PREC)) {
			discard(incXR);
		}
		expectToken(")", true);
		changeSection(previousSection);
	}
	statement(emptyLabelScope, emptyLabelScope);
	completeIteratorContinues(emptyLabelScope, scopeLabelsEnd);
	currentSection->insertSection(incrementSection);
	emitBackwardBranch(Processor::JMP_OP, loopPoint);
	completeForwardBranch(exitLoopPoint);
	completeIteratorBreaks(emptyLabelScope, scopeLabelsEnd);
}

void Compiler::semicolon(bool acceptLF) {
	const Char* b = p;
	white();
	if (!eof() && *p != '}' && !token(";", false) && (!acceptLF || !lineTerminatorInRange(b, p))) {
		p = b;
		error(SYNTAX_ERROR, "Syntax error");
	}
}

// FIX : sort all statement funcs

void Compiler::varStatement() {
	white();
	varDeclarations();
	semicolon();
}

void Compiler::functionStatement() {
	white();
	const Char* b = identifier(true, false);
	const String* name = String::newHashedString(heap, b, p);
	CodeSection* previousSection = changeSection(&setupSection);
	functionDefinition(name);
	declareIdentifier(name, true);
	changeSection(previousSection);
}

void Compiler::withStatement(SemanticScope* currentScope) {
	rvalueGroup();
	emit(Processor::WITH_SCOPE_OP);
	{
		++withScopeCounter;
		SemanticScope withScope(heap, SemanticScope::WITH_TYPE, currentSection->stackDepth, currentScope);
		statement(&withScope, &withScope);
		--withScopeCounter;
	}
	emit(Processor::POP_FRAME_OP);
}

void Compiler::breakStatement(SemanticScope* currentScope) {
	const String label = String((whiteNoLF() ? identifier(false, false) : p), p, &heap);
	Int32 rollbackStackDepth = currentSection->stackDepth;
	for (SemanticScope* s = currentScope; s != 0; s = s->next) {
		if (s->label.isEqualTo(label)
				&& (s->type == SemanticScope::ITERATOR_LABEL_TYPE || s->type == SemanticScope::LABEL_TYPE)) {
			s->popStack(*this, rollbackStackDepth, evalPopOpcode());
			s->breaks.push(emitForwardBranch(Processor::JMP_OP));
			semicolon();
			return;
		}
		rollbackStackDepth = s->rollback(*this, rollbackStackDepth, evalPopOpcode());
	}
	error(SYNTAX_ERROR, !label.empty() ? "Undefined label" : "Illegal break");
}

void Compiler::continueStatement(SemanticScope* currentScope) {
	const String label = String((whiteNoLF() ? identifier(false, false) : p), p, &heap);
	Int32 rollbackStackDepth = currentSection->stackDepth;
	for (SemanticScope* s = currentScope; s != 0; s = s->next) {
		if (s->label.isEqualTo(label) && (s->type == SemanticScope::ITERATOR_LABEL_TYPE
				|| (s->type == SemanticScope::LABEL_TYPE && !label.empty()))) {
			if (s->type != SemanticScope::ITERATOR_LABEL_TYPE) {
				error(SYNTAX_ERROR, "Illegal label for continue");
			}
			s->popStack(*this, rollbackStackDepth, evalPopOpcode());
			s->continues.push(emitForwardBranch(Processor::JMP_OP));
			semicolon();
			return;
		}
		rollbackStackDepth = s->rollback(*this, rollbackStackDepth, evalPopOpcode());
	}
	error(SYNTAX_ERROR, !label.empty() ? "Undefined label" : "Illegal continue");
}

void Compiler::forOrForInStatement(SemanticScope* currentScope, SemanticScope* scopeLabelsEnd) {
	SemanticScope emptyLabelScope(heap, EMPTY_STRING, currentSection->stackDepth, currentScope);
	emptyLabelScope.makeIteratorScopes(scopeLabelsEnd);
	expectToken("(", true);
	CodeSection initSection(heap, currentSection->stackDepth);
	if (token("var", true)) {
		white();
		acceptInOperator = false;
		ExpressionResult iterationXR = varDeclaration();
		acceptInOperator = true;
		if (token("in", true)) {
			forInStatement(&emptyLabelScope, scopeLabelsEnd, initSection, iterationXR);
		} else {
			if (token(",", true)) {
				white();
				acceptInOperator = false;
				varDeclarations();
				acceptInOperator = true;
			}
			forStatement(&emptyLabelScope, scopeLabelsEnd);
		}
	} else {
		ExpressionResult iterationXR;
		bool isForIn = false;
		{
			CodeSection* previousSection = changeSection(&initSection);
			acceptInOperator = false;
			bool gotExpression = optionalExpression(iterationXR, LOWEST_PREC);
			acceptInOperator = true;
			isForIn = (gotExpression && token("in", true));
			if (!isForIn) {
				discard(iterationXR);
			}
			changeSection(previousSection);
		}
		if (isForIn) {
			forInStatement(&emptyLabelScope, scopeLabelsEnd, initSection, iterationXR);
		} else {
			currentSection->insertSection(initSection);
			forStatement(&emptyLabelScope, scopeLabelsEnd);
		}
	}
}

void Compiler::ifStatement(SemanticScope* currentScope) {
	rvalueGroup();
	const BranchPoint falsePoint = emitForwardBranch(Processor::JF_OP);
	statement(currentScope, currentScope);
	if (token("else", true)) {
		const BranchPoint endPoint = emitForwardBranch(Processor::JMP_OP);
		completeForwardBranch(falsePoint);
		statement(currentScope, currentScope);
		completeForwardBranch(endPoint);
	} else {
		completeForwardBranch(falsePoint);
	}				
}

void Compiler::whileStatement(SemanticScope* currentScope, SemanticScope* scopeLabelsEnd) {
	const BranchPoint loopPoint = markBackwardBranch();
	SemanticScope emptyLabelScope(heap, EMPTY_STRING, currentSection->stackDepth, currentScope);
	emptyLabelScope.makeIteratorScopes(scopeLabelsEnd);
	rvalueGroup();
	const BranchPoint exitLoopPoint = emitForwardBranch(Processor::JF_OP);
	statement(&emptyLabelScope, &emptyLabelScope);
	completeIteratorContinues(&emptyLabelScope, scopeLabelsEnd);
	emitBackwardBranch(Processor::JMP_OP, loopPoint);
	completeForwardBranch(exitLoopPoint);
	completeIteratorBreaks(&emptyLabelScope, scopeLabelsEnd);
}

void Compiler::doWhileStatement(SemanticScope* currentScope, SemanticScope* scopeLabelsEnd) {
	white();
	const BranchPoint loopPoint = markBackwardBranch();
	SemanticScope emptyLabelScope(heap, EMPTY_STRING, currentSection->stackDepth, currentScope);
	emptyLabelScope.makeIteratorScopes(scopeLabelsEnd);
	statement(&emptyLabelScope, &emptyLabelScope);
	completeIteratorContinues(&emptyLabelScope, scopeLabelsEnd);
	expectToken("while", true);
	rvalueGroup();
	emitBackwardBranch(Processor::JT_OP, loopPoint);
	semicolon();
	completeIteratorBreaks(&emptyLabelScope, scopeLabelsEnd);
}

void Compiler::returnStatement(SemanticScope* currentScope) {
	if (compilingFor != FOR_FUNCTION) {
		error(SYNTAX_ERROR, "Illegal return");
	}
	ExpressionResult returnXR;
	if (whiteNoLF()) {
		optionalExpression(returnXR, LOWEST_PREC);
	}
	returnXR = makeRValue(returnXR);

	Int32 rollbackStackDepth = currentSection->stackDepth;
	for (SemanticScope* s = currentScope; s != 0; s = s->next) {
		rollbackStackDepth = s->rollback(*this, rollbackStackDepth, Processor::PUSH_BACK_OP);
	}
	emit(Processor::RETURN_OP);
	semicolon();
}

void Compiler::throwStatement() {
	if (!whiteNoLF()) {
		error(SYNTAX_ERROR, "Illegal newline after throw");
	}
	rvalueExpression();
	emit(Processor::THROW_OP);
	semicolon();
}

void Compiler::block(SemanticScope* scope) {
	expectToken("{", true);
	statementList(scope);
	expectToken("}", false);
}

/*
	Return statement needs a stack entry. All calls to finally() must have identical stack pointer. In combination this
	means we must preallocate an entry on the stack for any potential return statements ("space holder"). Also in eval
	mode we need to always flush eval results to stack before each call to finally() and when setting up try scopes.
	There is also a rule that eval result from try scope should be rolled back in case of a throw.
	
							// NO EVAL						// EVAL
							// ========						// ========
 
							// spaceHolder					// (eval)
	try {
							// spaceHolder					// rollback (eval)
		<statements>
							// spaceHolder					// (eval)
		finally()
	}
							// spaceHolder thrownA			// (eval) thrownA
	catch (x) {
		*try {
							// spaceHolder					// (eval)
			<statements>
			finally()
		*}
							// spaceHolder thrownA thrownB	// (eval) thrownA thrownB
		*catch {
							// thrownB						// thrownB
			*finally()
							// thrownB						// thrownB
			*throw
		*}
	}
							// spaceHolder					// (eval)
	finally {
							// spaceHolder					// rollback (eval)
		<statements>
							// spaceHolder					// rollback
	}
							//								// (eval)



							// NO EVAL						// EVAL
							// ========						// ========
 
							// spaceHolder					// (eval)
	try {
							// spaceHolder					// rollback (eval)
		<statements>
							// spaceHolder					// (eval)
		finally()
	}
							// spaceHolder thrownA			// (eval) thrownA
	*catch {
							// thrownA						// thrownA
		*finally()
							// thrownA						// thrownA
		*throw
	*}
							// spaceHolder					// (eval)
	finally {
							// spaceHolder					// rollback (eval)
		<statements>
							// spaceHolder					// rollback
	}
							//								// (eval)
*/
void Compiler::tryStatement(SemanticScope* currentScope) {
	white();

/* FIX : drop if we keep the reserve-one-element-for-return-on-stack-solution
	if (compilingFor != FOR_EVAL) { // space holder for return value so that we can break out of finallys (with the same stack pop count) regardless if try scope returns a value or not
		emit(Processor::VOID_OP, 0);
	}
*/

	QuickVector<BranchPoint> allFinallys(&heap);
	BranchPoint catchPoint;
	const Int32 trySectionEntryStackDepth = currentSection->stackDepth;
	{
		SemanticScope tryScope(heap, SemanticScope::TRY_TYPE, trySectionEntryStackDepth, currentScope);
		catchPoint = emitForwardBranch(Processor::TRY_OP);
		if (compilingFor == FOR_EVAL) { // Eval result from try scope should be rolled back in case of a throw
			emit(Processor::REPUSH_OP, 0);
		}
		block(&tryScope);
		if (compilingFor == FOR_EVAL) {
			emit(Processor::PUSH_BACK_OP, 1);
		}
		tryScope.finallys.push(emitForwardBranch(Processor::JSR_OP));
		allFinallys.insert(allFinallys.end(), tryScope.finallys.begin(), tryScope.finallys.end());
	}
	const BranchPoint tryEndPoint = emitForwardBranch(Processor::JMP_OP);

	// Catcher
	
	BranchPoint catchEndPoint;
	bool gotCatch = false;
	if (token("catch", true)) {
		gotCatch = true;
		expectToken("(", true);
		white();
		const Char* b = identifier(true, false);
		const String* exceptionVarName = String::newHashedString(heap, b, p);
		Int32 previousLocalIndex = 0;
		Table& nameIndexes = code->nameIndexes;
		bool didExist = false; // FIX : can use NON_EXISTENT type if we choose to keep it
		Table::Bucket* indexBucket = nameIndexes.lookup(exceptionVarName);
		if (indexBucket != 0) {
			didExist = true;
			previousLocalIndex = indexBucket->getIndexValue();
		} else {
			indexBucket = nameIndexes.insert(exceptionVarName);
		}
		nameIndexes.update(indexBucket, CATCH_PARAMETER); // prohibit further var accesses or declarations to the capture parameter via fast index binding.
		completeForwardBranch(catchPoint);
		catchPoint = emitForwardBranch(Processor::TRY_OP);
		emitWithConstant(Processor::CATCH_SCOPE_OP, Value(exceptionVarName));
		expectToken(")", true);
		{
			SemanticScope catchScope(heap, SemanticScope::CATCH_TYPE, currentSection->stackDepth, currentScope);
			block(&catchScope);
			emit(Processor::POP_FRAME_OP);
			catchScope.finallys.push(emitForwardBranch(Processor::JSR_OP));
			catchEndPoint = emitForwardBranch(Processor::JMP_OP);
			allFinallys.insert(allFinallys.end(), catchScope.finallys.begin(), catchScope.finallys.end());
		}
		if (didExist) {
			nameIndexes.update(nameIndexes.insert(exceptionVarName), previousLocalIndex);
		} else {
			nameIndexes.erase(nameIndexes.lookup(exceptionVarName));
		}
	}
	
	// Re-thrower (could actually be eliminated if we only have catch and no finally, but we don't know yet)
	
	completeForwardBranch(catchPoint);
	emit(Processor::PUSH_BACK_OP, currentSection->stackDepth - trySectionEntryStackDepth);
	BranchPoint finallyRethrowPoint = emitForwardBranch(Processor::JSR_OP);
	emit(Processor::THROW_OP);

	// Finally
	
	completeForwardBranches(allFinallys.begin(), allFinallys.end());
	emit(Processor::TRIED_OP);
	completeForwardBranch(finallyRethrowPoint);
	if (token("finally", true)) {
		SemanticScope finallyScope(heap, SemanticScope::FINALLY_TYPE, currentSection->stackDepth, currentScope);
		if (compilingFor == FOR_EVAL) {	// Ecmascript dictates that finally block should never change completion value.
			emit(Processor::VOID_OP);
		}
		block(&finallyScope);
		if (compilingFor == FOR_EVAL) {
			emit(Processor::POP_OP, 1);
		}
	} else {
		if (!gotCatch) {
			error(SYNTAX_ERROR, "Expected 'catch' or 'finally'");
		}
	}
	emit(Processor::RETURN_OP);
	
	completeForwardBranch(tryEndPoint);
	completeForwardBranch(catchEndPoint);

/* FIX : drop if we keep the reserve-one-element-for-return-on-stack-solution
	if (compilingFor != FOR_EVAL) { // pop return space holder
		emit(Processor::POP_OP, 1);
	}
*/
}

/*
	Switch statement places the test block *after* the cases block. This is to simplify forward branches (breaks etc) as
	they would be marked at incorrect addresses otherwise.
*/
void Compiler::switchStatement(SemanticScope* currentScope) {
	rvalueGroup();
	ExpressionResult controlXR(safeKeep());
	expectToken("{", true);
	const Int32 testsSectionEntryStackDepth = currentSection->stackDepth;
	const BranchPoint testsPoint = emitForwardBranch(Processor::JMP_OP);
	CodeSection testsSection(heap, testsSectionEntryStackDepth);
	QuickVector<Int32> jumpOffsets(&heap);
	BranchPoint defaultPoint;
	SemanticScope emptyLabelScope(heap, EMPTY_STRING, testsSectionEntryStackDepth, currentScope);
	bool firstStatement = true;
	white();
	while (!token("}", false)) {
		if (eof()) {
			error(SYNTAX_ERROR, "Expected '}'");
		}
		else if (token("case", false)) {
			assert(currentSection == &mainSection);
			currentSection->stackDepth = testsSection.stackDepth;
			const BranchPoint casePoint = markBackwardBranch();

			CodeSection* previousSection = changeSection(&testsSection);
			makeRValue(controlXR);
			rvalueExpression();
			expectToken(":", true);
			if (!currentSection->inDeadCode()) {
				emit(Processor::X_EQ_OP);
				jumpOffsets.push(testsSection.code.size());
				emit(Processor::JT_OP, casePoint.codeOffset - 1 - currentSection->code.size());
			}
			changeSection(previousSection);
		} else if (token("default", false)) {
			expectToken(":", true);
			if (defaultPoint.isValid()) {
				error(SYNTAX_ERROR, "More than one 'default:'");
			}
			assert(currentSection == &mainSection);
			currentSection->stackDepth = testsSectionEntryStackDepth;
			defaultPoint = markBackwardBranch();
		} else if (firstStatement) {
			error(SYNTAX_ERROR, "Expected 'case', 'default' or '}'");
		} else {
			const Char* b = p;
			statement(&emptyLabelScope, &emptyLabelScope);
			if (b == p) {
				error(SYNTAX_ERROR, "Expected '}'");
			}
		}
		firstStatement = false;
		white();
	}
	
	assert(currentSection == &mainSection);
	const BranchPoint donePoint = emitForwardBranch(Processor::JMP_OP);
	completeForwardBranch(testsPoint);
	for (const Int32* it = jumpOffsets.begin(); it != jumpOffsets.end(); ++it) {
		const std::pair<Processor::Opcode, Int32> instruction = Processor::unpackInstruction(testsSection.code[*it]);
		testsSection.code[*it] = Processor::packInstruction(instruction.first, instruction.second - mainSection.code.size());
	}
	currentSection->insertSection(testsSection);
	assert(currentSection->stackDepth == testsSectionEntryStackDepth); // since we assume this in the default case code
	if (defaultPoint.isValid()) {
		emitBackwardBranch(Processor::JMP_OP, defaultPoint);
	}
	completeForwardBranch(donePoint);
	completeBreaks(&emptyLabelScope);
	returnSafeKept(controlXR);
}

/*
	'scopeLabelsEnd' points to the end of a range of scopes that are all labelled scopes. This is required
	because it is legal to give while loops etc mulitple alternative labels to be used with 'continue'.
	(These should all be converted to 'ITERATOR_LABEL_TYPE' upon entry.)
*/
void Compiler::statement(SemanticScope* currentScope, SemanticScope* scopeLabelsEnd) {
	assert(currentSection == &mainSection); // statements must produce into main-section because of breaks etc...
	
	white();
	
	if (token("{", false)) {
		statementList(currentScope);
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
			for (SemanticScope* s = currentScope; s != 0; s = s->next) {
				if (!s->label.empty() && s->label.isEqualTo(label)) {
					error(SYNTAX_ERROR, "Duplicate label");
				}
			}
			SemanticScope newLabelScope(heap, label, currentSection->stackDepth, currentScope);
			statement(&newLabelScope, scopeLabelsEnd);
			if (newLabelScope.type != SemanticScope::ITERATOR_LABEL_TYPE) {
				completeBreaks(&newLabelScope);
			}
		} else {
			p = b;
			ExpressionResult evalXR = (compilingFor == FOR_EVAL ? ExpressionResult::PUSHED : ExpressionResult::NONE);
			optionalExpression(evalXR, LOWEST_PREC);
			if (compilingFor == FOR_EVAL) {
				evalXR = makeRValue(evalXR);
			} else {
				evalXR = discard(evalXR);
			}
			semicolon();
		}
	} else {
		switch (static_cast<Statement>(statementTokenIndex)) {
			case BREAK_STATEMENT: breakStatement(currentScope); break;
			case CONTINUE_STATEMENT: continueStatement(currentScope); break;
			case VAR_STATEMENT: varStatement(); break;
			case IF_STATEMENT: ifStatement(currentScope); break;
			case WHILE_STATEMENT: whileStatement(currentScope, scopeLabelsEnd); break;
			case DO_WHILE_STATEMENT: doWhileStatement(currentScope, scopeLabelsEnd); break;
			case FOR_STATEMENT: forOrForInStatement(currentScope, scopeLabelsEnd); break;
			case RETURN_STATEMENT: returnStatement(currentScope); break;
			case THROW_STATEMENT: throwStatement(); break;
			case TRY_STATEMENT: tryStatement(currentScope); break;
			case SWITCH_STATEMENT: switchStatement(currentScope); break;
			case FUNCTION_STATEMENT: functionStatement(); break;
			case WITH_STATEMENT: withStatement(currentScope); break;

			default: assert(0);
		}
	}
}

void Compiler::statementList(SemanticScope* currentScope) {
	const Char* b;
	do {
		b = p;
		statement(currentScope, currentScope);
	} while (!eof() && b != p);
}

const Char* Compiler::compile(const Char* b, const Char* e) {
	changeSection(&mainSection);
	p = b;
	this->e = e;
	acceptInOperator = true;
	
	// FIX : not 100% necessary now because we should always start with undefined on top of stack
	if (compilingFor == FOR_EVAL) {
		emit(Processor::POP_OP, 1);	// FIX : only if we reserve one element for return like we do now
		emit(Processor::VOID_OP);
	}
	SemanticScope rootScope(heap, SemanticScope::ROOT_TYPE, 1, 0);
	statementList(&rootScope);
	// FIX : sometimes necessary even if we start with undefined on top of stack, because try/catch rethrower might need to safe-keep its exception there
	if (compilingFor != FOR_EVAL) {
		// FIX : if RETURN_OP took a push back count we could just do void_op here, or even have another RETURN_VOID_OP
		emit(Processor::POP_OP, 1);	// FIX : only if we reserve one element for return like we do now
		emit(Processor::VOID_OP);
	}
	emit(Processor::RETURN_OP);

	assert(currentSection == &mainSection);
	code->code.resize(setupSection.code.size() + mainSection.code.size());
	std::copy(setupSection.code.begin(), setupSection.code.end(), code->code.begin());
	std::copy(mainSection.code.begin(), mainSection.code.end(), code->code.begin() + setupSection.code.size());
#if (VERIFY_STACK_LOGIC)
	assert(setupSection.stackDepths.size() == setupSection.code.size());
	assert(mainSection.stackDepths.size() == mainSection.code.size());
	code->stackDepths.resize(setupSection.stackDepths.size() + mainSection.stackDepths.size());
	std::copy(setupSection.stackDepths.begin(), setupSection.stackDepths.end(), code->stackDepths.begin());
	std::copy(mainSection.stackDepths.begin(), mainSection.stackDepths.end(), code->stackDepths.begin() + setupSection.stackDepths.size());
#endif
	code->constants->shrink();
	code->maxStackDepth = std::max(mainSection.maxStackDepth, setupSection.maxStackDepth);
	
if (!suppressStdErr) {
	disassemble(heap, *code);
	Int32 stackDepth = mainSection.stackDepth;
	assert(mainSection.inDeadCode() || stackDepth == 0);
	std::cerr << "\tEnd stack depth: " << stackDepth << std::endl;
	std::cerr << "\tMax stack depth: " << code->maxStackDepth << std::endl;
	std::cerr << "\tCode: " << code->code.size() << std::endl;
	std::cerr << "\tConstants: " << code->constants->size() << std::endl;
	std::cerr << "\tBloom set: " << std::hex << code->bloomSet << std::dec << std::endl;
/* FIX :	for (const Value* p = code->constants.begin(); p != code->constants.end(); ++p) {
		std::wcerr << "\t\t" << p->toString(heap)->toStdString() << std::endl;
	}*/
	std::cerr << "\tVars: " << code->getVarsCount() << std::endl;
	std::cerr << "\tArguments: " << code->getArgumentsCount() << std::endl;
} else {
	QuickArray<Int32> stackDepths(code->getCodeSize(), &heap);
	std::fill(stackDepths.begin(), stackDepths.end(), DEAD_CODE_STACK_DEPTH);
	
	Int32 maxStackDepth = 0;
	int errors = recursiveStackCheck(*code, stackDepths, 0, 1, maxStackDepth);
	assert(errors == 0);
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
	Table& nameIndexes = code->nameIndexes;
	QuickVector<const String*>& argumentNames = code->argumentNames;
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
		nameIndexes.update(nameIndexes.insert(name), static_cast<Int32>(argumentNames.size()));
		argumentNames.push(name);
		code->bloomSet |= name->createBloomCode();
		white();
	}
	expectToken("{", true);
	compile(p, e); // FIX: ugly as it sets p and e again, although it doesn't hurt
	expectToken("}", false);
	code->name = functionName;
	code->selfName = selfName;
	code->source = String::concat(heap, String(FUNCTION_SPACE, *functionName, &heap), String(b, p, &heap));
	code->bloomSet |= (selfName != 0 ? selfName->createBloomCode() : 0) | ARGUMENTS_STRING.createBloomCode();
	return p;
}

void Compiler::compile(const String& source) {
	compile(source.begin(), source.end());
	white();
	if (!eof()) {
		error(SYNTAX_ERROR, "Syntax error"); // FIX : better error?
	}
}

Value EvalFunction::call(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	if (argc < 1) return Value::UNDEFINED;
	if (!argv[0].typeOfString()->isEqualTo(STRING_STRING)) return argv[0]; // FIX : better way to check type without involving typeOfString?

	Heap& heap = rt.getHeap();
	const String* sourceCode = argv[0].toString(heap);
	Code* code;
	
	const Table::Bucket* bucket = functionDefCache.lookup(sourceCode);
	if (bucket != 0) {
		Object* o = bucket->getValue().getObject();
		assert(dynamic_cast<Code*>(o) != 0);
		code = reinterpret_cast<Code*>(o);
	} else {
		code = heap.govern(new(heap) Code(heap, heap.govern(new(heap) Constants(heap)))); // Notice: doesn't share constants with anyone else, so they will be nicely erased when this temporary function gets purged.
		Compiler compiler(heap, code, Compiler::FOR_EVAL);
		compiler.compile(*sourceCode);
		functionDefCache.update(functionDefCache.insert(sourceCode), Value(code));
	}
	processor.enterEval(code, direct);
	return Value::UNDEFINED;
}

/* FIX : funny C++11 thing
#include <functional>
template<class F> Value newNativeFunction(Heap& heap, const F& f) { return Value(heap.govern(new FunctorAdapter<F>(f))); }
mathObject->setOwnProperty(*this, &SQRT_STRING, newNativeFunction(heap, [](Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
	return (argc >= 1 ? Value(sqrt(argv[0].toDouble())) : Value::NOT_A_NUMBER);
}));
*/

typedef Value (*NativeFunction)(Runtime&, Processor&, Int32, const Value*, Object*);

template<class F> struct FunctorAdapter : public Function {
	FunctorAdapter(const F& f) : f(f) { }
	virtual Value call(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		return f(rt, processor, argc, argv, thisObject);
	}
	F* f;
};

template<class F> struct UnaryMathFunction : public Function {
	UnaryMathFunction(const F& f) : f(f) { }
	virtual Value call(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		return (argc >= 1 ? Value(f(argv[0].toDouble())) : Value::NOT_A_NUMBER);
	}
	const F* f;
};

struct Support {
	static Value getInternalProperty(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 2) {
			Object* o = argv[0].asObject();
			if (o != 0) {
				const String* s = argv[1].toString(rt.getHeap());
				if (s->isEqualTo(PROTOTYPE_STRING)) {
					Object* p = o->getPrototype(rt);
					return (p == 0 ? Value::NUL : Value(p));
				} else if (s->isEqualTo(CLASS_STRING)) {
					return Value(o->getClassName());
				} else if (s->isEqualTo(VALUE_STRING)) {
					return o->getInternalValue(rt.getHeap());
				}
			}
		}
		return Value::UNDEFINED;
	}

	static Value createWrapper(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 2) {
			Heap& heap = rt.getHeap();
			const String* const className = argv[0].toString(heap);
			const Value internalValue = argv[1];
			Object* prototype = (argc >= 3 ? argv[2].toObject(rt, false) : 0);
			if (prototype == 0) {
				prototype = &rt.objectPrototype;
			}
			Object* o = 0;
			if (className->isEqualTo(S_TRING_STRING)) {
				o = new(heap) StringWrapper(heap, internalValue.toString(heap), prototype);
			} else if (className->isEqualTo(E_RROR_STRING)) {
				for (int i = 0; i < ERROR_TYPE_COUNT; ++i) {
					if (prototype == rt.errorPrototypes[i]) {
						o = new(heap) Error(heap, static_cast<ErrorType>(i));
						break;
					}
				}
			}
			if (o == 0) {
				o = new(heap) WrapperObject(heap, className, internalValue, prototype);
			}
			return Value(heap.govern(o));
		}
		return Value::UNDEFINED;
	}

	static Value specialConstructor(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 1) {
			Function* regularFunction = argv[0].asFunction();
			if (regularFunction != 0) {
				Function* constructorFunction = (argc >= 2 ? argv[1].asFunction() : 0);
				Heap& heap = rt.getHeap();
				return Value(heap.govern(new(heap) SpecialConstructorFunction(heap, regularFunction, constructorFunction)));
			}
		}
		return Value::UNDEFINED;
	}

	static Value defineProperty(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		bool success = false;
		if (argc >= 2) {
			Object* o = argv[0].asObject();
			if (o != 0) {
				success = o->setOwnProperty(rt, argv[1], (argc >= 3 ? argv[2] : Value::UNDEFINED)
						, (argc >= 4 && argv[3].toBool() ? READ_ONLY_FLAG : 0)
						| (argc >= 5 && argv[4].toBool() ? DONT_ENUM_FLAG : 0)
						| (argc >= 6 && argv[5].toBool() ? DONT_DELETE_FLAG : 0));
			}
		}
		return Value(success);
	}

	static Value compileFunction(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 1) {
			Heap& heap = rt.getHeap();
			const String* source = argv[0].toString(heap);
			Code* code = heap.govern(new(heap) Code(heap, heap.govern(new(heap) Constants(heap)))); // Notice: doesn't share constants with anyone else, so they will be nicely erased if this function gets deleted.
			Compiler compiler(heap, code, Compiler::FOR_FUNCTION);
			compiler.compileFunction(source->begin(), source->end(), argc >= 2 ? argv[1].toString(heap) : &ANONYMOUS_STRING);
			return Value(heap.govern(new(heap) ScriptFunction(heap, code, rt.getGlobalScope())));
		}
		return Value::UNDEFINED;
	}

	static Value callWithArgs(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Heap& heap = rt.getHeap();
		Function* callFunction = (argc > 0 ? argv[0].asFunction() : 0);
		if (callFunction == 0) {
			Exception::throwError(heap, TYPE_ERROR, "apply / call used on non-function");
			return Value();
		} else {
			Object* newThis = (argc > 1 ? argv[1].toObject(rt, true) : 0);
			QuickArray<Value> args(0, &heap);
			Object* arrayObject = (argc > 2 ? argv[2].toObject(rt, false) : 0);
			if (arrayObject != 0) {
				Value v;
				if (arrayObject->getPropertyDeep(rt, Value(&LENGTH_STRING), &v) != NONEXISTENT) { // FIX : in the future I think we should have a getLength
					Int32 offset = (argc > 3 ? argv[3].toInt() : 0);
					UInt32 length = static_cast<UInt32>(std::max(v.toInt() - offset, 0));
					args.resize(length);
					for (UInt32 i = 0; i < length; ++i) {
						args[i] = Value::UNDEFINED;
						arrayObject->getPropertyDeep(rt, Value(i + offset), &args[i]);
					}
				}
			}
			// FIX : we copy all arguments once to args, and then chain will copy them again to a scope object, couldn't we short-cut that somehow?
			// FIX : since only arrays and arguments are really valid here, perhaps even have a new virtual in object for implementing efficient apply with these?
			return callFunction->call(rt, processor, args.size(), args.begin(), (newThis == 0 ? rt.getGlobals() : newThis));
		}
	}
	
	static Value hasOwnProperty(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Object* object = (argc >= 2 ? argv[0].toObject(rt, false) : 0);
		return (object != 0 ? Value(object->hasOwnProperty(rt, argv[1])) : Value(false));
	}

	static Value isPropertyEnumerable(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Object* object = (argc >= 2 ? argv[0].toObject(rt, false) : 0);
		return (object != 0 ? Value(object->isOwnPropertyEnumerable(rt, argv[1])) : Value(false));
	}

	static Value fromCharCode(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Char c = static_cast<Char>(argc > 0 ? argv[0].toInt() : 0);
		Heap& heap = rt.getHeap();
		return ((0 <= c && c < 127) ? QUICK_CONSTANTS.ascii[c] : Value(heap.govern(new(heap) String(&c, &c + 1, &heap))));
	}

	static Value atan2(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		return (argc >= 2 ? Value(std::atan2(argv[0].toDouble(), argv[1].toDouble())) : Value::NOT_A_NUMBER);
	}

	static Value pow(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		return (argc >= 2 ? Value(std::pow(argv[0].toDouble(), argv[1].toDouble())) : Value::NOT_A_NUMBER);
	}

	static Value parseFloat(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Value result = Value::NOT_A_NUMBER;
		if (argc >= 1) {
			const String* s = argv[0].toString(rt.getHeap());
			const Char* e = s->end();
			const Char* b = eatStringWhite(s->begin(), e);
			double d = 0.0;
			if (parseDouble(b, e, d) != b) {
				result = Value(d);
			}
		}
		return result;
	}

	static Value charCodeAt(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 2) {
			const String* s = argv[0].toString(rt.getHeap());
			const double d = nanToZero(argv[1].toDouble());
			if (-1.0 < d && d < s->size()) {
				return Value((*s)[static_cast<UInt32>(d)]);
			}
		}
		return Value::NOT_A_NUMBER;
	}

	static Value substring(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Value result(&EMPTY_STRING);
		if (argc >= 3) {
			const String* s = argv[0].toString(rt.getHeap());
			const double l = s->size();
			const double from = std::max(nanToZero(argv[1].toDouble()), 0.0);
			const double to = std::min(nanToZero(argv[2].toDouble()), l);
			if (from < to && from < l && to >= 1.0) {
				const Char* b = s->begin();
				Heap& heap = rt.getHeap();
				return Value(from < 1.0 && to >= l ? s
						: heap.govern(new(heap) String(b + static_cast<UInt32>(from), b + static_cast<UInt32>(to), &heap)));
			}
		}
		return Value(&EMPTY_STRING);
	}

	static Value submatch(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 3) {
			const String* const text = argv[0].toString(rt.getHeap());
			const Int32 offset = argv[1].toInt();
			const String* const match = argv[2].toString(rt.getHeap());
			const UInt32 matchSize = match->size();
			if (offset + matchSize <= text->size()
					&& std::equal(text->begin() + offset, text->begin() + offset + matchSize, match->begin())) {
				return Value(true);
			}
		}
		return Value(false);
	}

	static Value getCurrentTime(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		return Value(time(0) * 1000.0 + rt.unixEpochTimeDiff);
	}

	static Value localTimeDifference(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		const time_t t = (argc > 0 ? static_cast<time_t>((argv[0].toDouble() - rt.unixEpochTimeDiff) / 1000.0 + 0.5) : time(0));
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
	UnaryMathFunction<double (double)> func;
} NATIVE_MATH_FUNCTIONS[] = {
	{ String("acos"), acos }, { String("asin"), asin }, { String("atan"), atan }, { String("cos"), cos },
	{ String("exp"), exp }, { String("floor"), floor }, { String("log"), log }, { String("sin"), sin },
	{ String("sqrt"), sqrt }, { String("tan"), tan }
};
// TODO : min, max, random

static struct {
	String name;
	FunctorAdapter<Value (Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) > func;
} SUPPORT_FUNCTIONS[] = {
	{ String("getInternalProperty"), Support::getInternalProperty }, { String("createWrapper"), Support::createWrapper },
	{ String("defineProperty"), Support::defineProperty }, { String("compileFunction"), Support::compileFunction },
	{ String("specialConstructor"), Support::specialConstructor }, { String("callWithArgs"), Support::callWithArgs },
	{ String("hasOwnProperty"), Support::hasOwnProperty }, { String("fromCharCode"), Support::fromCharCode },
	{ String("isPropertyEnumerable"), Support::isPropertyEnumerable }, { String("atan2"), Support::atan2 },
	{ String("pow"), Support::pow }, { String("parseFloat"), Support::parseFloat },
	{ String("charCodeAt"), Support::charCodeAt }, { String("substring"), Support::substring },
	{ String("submatch"), Support::submatch }, { String("getCurrentTime"), Support::getCurrentTime },
	{ String("localTimeDifference"), Support::localTimeDifference }
};

static UnaryMathFunction<bool (double)> IS_NAN_FUNCTION(isNaN);
static UnaryMathFunction<bool (double)> IS_FINITE_FUNCTION(isFinite);

Runtime::Runtime(Heap& heap) : super(heap), heap(heap), globalScope(heap), globals(0), stackSize(DEFAULT_STACK_SIZE)
		, objectPrototype(heap, 0), functionPrototype(heap), evalFunction(heap, false)
		, evalDirectFunction(heap, true), toNumberFunction(0), toStringFunction(0), createRegExpFunction(0)
		, booleanPrototype(heap, &B_OOLEAN_STRING, Value(false), &objectPrototype)
		, numberPrototype(heap, &N_UMBER_STRING, Value(0.0), &objectPrototype)
		, stringPrototype(heap, &S_TRING_STRING, Value(&EMPTY_STRING), &objectPrototype)
		, arrayPrototype(heap), unixEpochTimeDiff(0.0) {
	std::fill(errorPrototypes, errorPrototypes + ERROR_TYPE_COUNT, (Error*)(0));
	for (int i = 0; i < ERROR_TYPE_COUNT; ++i) {
		errorPrototypes[i] = heap.govern(new(heap) ErrorPrototype(heap, static_cast<ErrorType>(i)));
	}
}

// FIX : make this optional by including default toNumberFunction etc that throws (or should they return something?)
void Runtime::setupStandardLibrary(const String& librarySourceCode) {
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
	
	globals->setOwnProperty(*this, Value(&EVAL_STRING), Value(&evalFunction), DONT_ENUM_FLAG);
	ScriptObject* supportObject = heap.govern(new(heap) ScriptObject(heap, &objectPrototype));
	ScriptObject* prototypesObject = heap.govern(new(heap) ScriptObject(heap, &objectPrototype));
	// FIX : static strings?
	supportObject->setOwnProperty(*this, Value(String::newString(heap, "prototypes")), Value(prototypesObject), STANDARD_FLAGS);
	for (int i = 0; i < sizeof (SUPPORT_FUNCTIONS) / sizeof (*SUPPORT_FUNCTIONS); ++i) {
		supportObject->setOwnProperty(*this, Value(&SUPPORT_FUNCTIONS[i].name), Value(&SUPPORT_FUNCTIONS[i].func), STANDARD_FLAGS);
	}
	supportObject->setOwnProperty(*this, Value(String::newString(heap, "isNaN")), Value(&IS_NAN_FUNCTION), STANDARD_FLAGS);
	supportObject->setOwnProperty(*this, Value(String::newString(heap, "isFinite")), Value(&IS_FINITE_FUNCTION), STANDARD_FLAGS);
	supportObject->setOwnProperty(*this, Value(&UNDEFINED_STRING), Value::UNDEFINED, STANDARD_FLAGS);
	supportObject->setOwnProperty(*this, Value(&NAN_STRING), Value::NOT_A_NUMBER, STANDARD_FLAGS);
	supportObject->setOwnProperty(*this, Value(&INFINITY_STRING), Value::INFINITE_NUMBER, STANDARD_FLAGS);
	supportObject->setOwnProperty(*this, Value(String::newString(heap, "maxNumber")), Value(std::numeric_limits<double>::max()), STANDARD_FLAGS);
	supportObject->setOwnProperty(*this, Value(String::newString(heap, "minNumber")), Value(std::numeric_limits<double>::denorm_min()), STANDARD_FLAGS);
	
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

	Code* code = heap.govern(new(heap) Code(heap, heap.govern(new(heap) Constants(heap))));
	{
		Compiler compiler(heap, code, Compiler::FOR_EVAL);
		compiler.compile(librarySourceCode);
	}
	{
		Processor processor(*this);
		processor.enterEval(code, false);
		bool needsMoreTime = processor.run();
		assert(!needsMoreTime);
		Value v = processor.getResult();
		Function* f = v.asFunction();
		if (f != 0) {
			Value argv[1] = { Value(supportObject) };
			f->call(*this, processor, 1, argv, getGlobals());
			needsMoreTime = processor.run();
			assert(!needsMoreTime);
		}
	}
	heap.gc();

	// FIX : sub v
	toNumberFunction = supportObject->getPropertyDeep(*this, "toPrimitiveNumber").asFunction();
	if (toNumberFunction == 0) {
		Exception::throwError(heap, GENERIC_ERROR, "Standard library is missing support.toPrimitiveNumber");
	}
	
	toStringFunction = supportObject->getPropertyDeep(*this, "toPrimitiveString").asFunction();
	if (toStringFunction == 0) {
		Exception::throwError(heap, GENERIC_ERROR, "Standard library is missing support.toPrimitiveString");
	}

	createRegExpFunction = supportObject->getPropertyDeep(*this, "createRegExp").asFunction();
	if (toStringFunction == 0) {
		Exception::throwError(heap, GENERIC_ERROR, "Standard library is missing support.createRegExp");
	}
}

void Runtime::setGlobalObject(Object* globals) {
	assert(heap.belongs(globals));
	this->globals = globals;
}

void Runtime::setStackSize(UInt32 stackSize) { this->stackSize = stackSize; }
UInt32 Runtime::getStackSize() const { return stackSize; }

#include <fstream>
#include <memory>
#include <vector>

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
		public:		virtual Value call(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
						const String* s = (argc >= 1 ? argv[0].toString(rt.getHeap()) : &EMPTY_STRING);
						std::wcout << s->toStdString().c_str() << std::endl;
						if (interactive) {
							pushIOLines('<', *s);
						}
						return Value::UNDEFINED;
					}
	};
	
	struct GCFunction : public Function {
		virtual Value call(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
			UInt32 sweptCount = rt.getHeap().gc();
			return Value(static_cast<Int32>(sweptCount));
		}
	};

	class NativeTestObject : public Object {
		protected:	struct AddFunction : public Function {
						typedef Function super;
						virtual Value call(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
							Heap& heap = rt.getHeap();
							if (thisObject->getClassName() != &NativeTestObject::className) {
								Exception::throwError(heap, TYPE_ERROR, "Invalid class");
							}
							const NativeTestObject* me = reinterpret_cast<NativeTestObject*>(thisObject);
							const double addX = (argc >= 1 ? argv[0].toDouble() : 0.0);
							const double addY = (argc >= 2 ? argv[1].toDouble() : 0.0);
							return Value(heap.govern(new(heap) NativeTestObject(heap, addX + me->x, addY + me->y)));
						}
					};

		public:		typedef Object super;
					NativeTestObject(Heap& heap, double x, double y) : super(heap), x(x), y(y) { }
					virtual const String* getClassName() const { return &className; };
					virtual Flags getOwnProperty(Runtime& rt, const Value& key, Value* v) const {
						if (key.isEqualTo(String("x"))) { *v = Value(x); return EXISTS_FLAG | READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG; };
						if (key.isEqualTo(String("y"))) { *v = Value(y); return EXISTS_FLAG | READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG; };
						if (key.isEqualTo(String("add"))) { *v = Value(&addFunction); return EXISTS_FLAG | READ_ONLY_FLAG | DONT_ENUM_FLAG | DONT_DELETE_FLAG; };
						return super::getOwnProperty(rt, key, v);
					}
		
					virtual const String* toString(Heap& heap) const {
						std::stringstream ss;
						ss << "x: " << x << ", y: " << y;
						return String::newString(heap, ss.str().c_str());
					}

		
		protected:	double x;
					double y;
					static String className;
					static AddFunction addFunction;
	};
	String NativeTestObject::className("NativeTestObject");
	NativeTestObject::AddFunction NativeTestObject::addFunction;

	class CallbackTest : public Function {
		public:		virtual Value call(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
						PrintFunction printFunction;
						for (int i = 0; i < argc; ++i) {
							Function* f = argv[i].asFunction();
							if (f == 0) {
								f = &printFunction;
							}
							f->call(rt, processor, 1, &argv[i], thisObject);
						}
						return Value::UNDEFINED;
					}
	};

int testMain(int argc, const char* argv[]) {
    String source(EMPTY_STRING);
    std::auto_ptr<std::wifstream> inputFileStream;
    std::wistream* inStream = &std::wcin;
    bool doTime = false;
    int gcRate = 256;
    bool autoGCRate = true;
    bool pauseBeforeQuit = false;
	bool doSuppressStdErr = false;
	bool loadStdLib = true;
    for (int argi = 1; argi < argc; ++argi) {
        if (strcmp(argv[argi], "-t") == 0) doTime = true;
        else if (strcmp(argv[argi], "-s") == 0) doSuppressStdErr = true;
        else if (argi + 1 < argc && strcmp(argv[argi], "-gc") == 0) {
            gcRate = atoi(argv[++argi]);
            autoGCRate = false;
        }
        else if (strcmp(argv[argi], "-p") == 0) pauseBeforeQuit = true;
        else if (strcmp(argv[argi], "-n") == 0) loadStdLib = false;
        else if (inputFileStream.get() == 0) {
            inputFileStream.reset(new std::wifstream(argv[argi]));
            inStream = inputFileStream.get();
            interactive = false;
        } else {
            std::cout << "Too many arguments" << std::endl;
            return 1;
        }
    }

    /*std::cout << sizeof(Value) << std::endl;
     std::cout << sizeof(String) << std::endl;
     std::cout << sizeof(ScriptObject) << std::endl;
     std::cout << sizeof(Array) << std::endl;
     std::cout << sizeof(ScriptFunction) << std::endl;*/
suppressStdErr = true;
    const String LF_STRING("\n");
    const String PRINT_STRING("print");
    /*std::cerr << "Value size: " << sizeof (Value) << std::endl;
     std::cerr << "Bucket size: " << sizeof (Table::Bucket) << std::endl;
     std::cerr << "Table size: " << sizeof (Table) << std::endl;*/
    Heap heap;
    Runtime rt(heap);
    Constants globalConstants(heap);
    Code globalCode(heap, &globalConstants);

    ScriptObject globals(heap, &rt.objectPrototype);
	rt.setGlobalObject(&globals);
	
    PrintFunction printFunction;
    globals.setOwnProperty(rt, Value(&PRINT_STRING), Value(&printFunction), DONT_ENUM_FLAG);
    GCFunction gcFunction;
    const String GC_STRING("gc");
    globals.setOwnProperty(rt, Value(&GC_STRING), Value(&gcFunction), DONT_ENUM_FLAG);
CallbackTest callbackTest;
globals.setOwnProperty(rt, Value(String::newString(heap, "callbackTest")), Value(&callbackTest), DONT_ENUM_FLAG);
//globals.setOwnProperty(rt, Value(String::newString(heap, "nativeTestObject")), Value(heap.govern(new(heap) NativeTestObject(heap, 1.4, 2.9))), STANDARD_FLAGS);

	if (loadStdLib) {
		try {
			std::wifstream stdlibStream("stdlib.js");
			String source = String(std::wstring(std::istreambuf_iterator<wchar_t>(stdlibStream), std::istreambuf_iterator<wchar_t>()), &heap);
			stdlibStream.close();
			rt.setupStandardLibrary(source);
		}
		catch (const std::exception& x) {
			std::cerr << "exception setting up standard lib: " << x.what() << std::endl;
			return 1;
		}
		catch (...) {
			std::cerr << "exception setting up standard lib" << std::endl;
			return 1;
		}
	}
    suppressStdErr = doSuppressStdErr;

	Processor processor(rt);
	processor.run();	// just testing some weird behaviour
//	const Value TEST_ARGV[1] = { Value(String::newString(heap, "Hello")) };
//	printFunction.call(rt, processor, 1, TEST_ARGV, &globals);
//	processor.run();	// just testing some weird behaviour
	
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
                        source = String(String(String("print("), String(source.begin() + 1, source.end(), &heap), &heap), String(")"), &heap); // FIX : operator +
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
                Compiler compiler(heap, &globalCode, (interactive ? Compiler::FOR_EVAL : Compiler::FOR_GLOBAL));
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
// FIX :						std::wcout << std::wstring(std::max<const Char*>(stopPoint - 8, source.begin()), std::min<const Char*>(stopPoint + 8, source.end())) << std::endl;
                    throw;
                }
                if (!suppressStdErr) {
                    for (const Value* p = globalConstants.begin(); p != globalConstants.end(); ++p) {
                        std::wcerr << "\t\t" << p->toString(heap)->toStdString() << std::endl;
                    }
                }
               	processor.run();	// just testing some weird behaviour
                processor.chain(&globalCode, rt.getGlobalScope(), &globals);

                bool done = false;
                clock_t start = ::clock();
                do {
                    processor.resetTimeOut(gcRate);
                    done = !processor.run();
                    {
                        long beforeSize = static_cast<long>(heap.size());
                        UInt32 sweptCount = heap.gc();
                        long nowSize = static_cast<long>(heap.size());
                        if (!suppressStdErr) {
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
                    	processor.run();	// just testing some weird behaviour
                        if (!suppressStdErr) {
                            Value v = processor.getResult();
                            std::wcerr << L"\t=" << (v.toString(heap))->toStdString().c_str() << std::endl;
                            assert(processor.firstCatcher == 0);
                        }
                        if (doTime) {
                            clock_t end = ::clock();
                            const double divisor = (CLOCKS_PER_SEC / 1000.0);
                            std::cout << "\ttime: " << (static_cast<double>(end - start) / divisor) << "ms" << std::endl;
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

int main(int argc, const char* argv[]) {
    testMain(argc, argv);
}
