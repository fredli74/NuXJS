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
static bool isNaN(double d) { return _isnan(d); }
static bool isFinite(double d) { return _finite(d); }

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

// FIX : shouldn't need this, used in quick-hash-finders and they should be optimized somehow i think
static int strncmp(const Char* a, const char* b, UInt32 n) {
	UInt32 i = 0;
	while (i < n && a[i] == b[i] && a[i] != 0 && b[i] != 0) {
		++i;
	}
	return (i == n ? 0 : a[i] - b[i]);
}
/* Built with QuickHashMaker.pika */
static int findStatementToken(int n /* string length */, const Char* s) {
	static const char* STRINGS[12] = {
		"var", "if", "while", "do", "for", "return", "continue", "break", "try", 
		"throw", "switch", "function"
	};
	static const int QUICK_HASH_TABLE[32] = {
		-1, -1, 11, 7, -1, 6, -1, -1, -1, -1, -1, -1, -1, 0, 2, 8, 
		-1, 9, -1, 10, 5, -1, -1, -1, -1, 1, -1, -1, -1, 4, 3, -1
	};
	if (n < 2 || n > 8) return -1;
	int stringIndex = QUICK_HASH_TABLE[(n - s[0]) & 31];
	return (stringIndex >= 0 && strncmp(s, STRINGS[stringIndex], n) == 0 && STRINGS[stringIndex][n] == 0) ? stringIndex : -1;
};
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

static const Char* parseInt(const Char* p, const Char* e, Int32& i) {
	bool negative = (e - p >= 2 && ((*p == '+' || *p == '-') && p[1] >= '0' && p[1] <= '9') ? (*p++ == '-') : false);
	UInt32 ui;
	p = parseUnsignedInt(p, e, ui);
	i = (negative ? -static_cast<Int32>(ui) : ui);
	return p;
}

static const Char* parseDouble(const Char* p, const Char* e, double& v) {
	assert(p <= e);
	double d = 0;
	const Char* q = p;
	double sign = (e - q > 1 && (*q == '+' || *q == '-') ? (*q++ == '-' ? -1.0 : 1.0) : 1.0);
	if (e - q >= 8 && strncmp(q, "Infinity", 8) == 0) {
		q += 8;
		d = std::numeric_limits<double>::infinity();
	} else {
		if (q == e || (*q != '.' && (*q < '0' || *q > '9'))) {
			v = 0.0;
			return p;
		}
		const Char* b = q;
		while (q != e && *q >= '0' && *q <= '9') {
			d = d * 10.0 + (*q++ - '0');
		}
		if (q != e && *q == '.') {
			double f = 1.0;
			while (++q != e && *q >= '0' && *q <= '9') {
				d += (*q - '0') * (f *= 0.1);
			}
			if (q == b + 1) {
				v = 0.0;
				return p;
			}
		}
		if (q != e && (*q == 'E' || *q == 'e')) {
			Int32 i;
			const Char* t = parseInt(q + 1, e, i);
			if (t != q + 1) { d *= pow(10, static_cast<double>(i)); q = t; }
		}
	}
	v = d * sign;
	return q;
}

// Returns pointer to beginning of string
// End of string is always buffer + 32 (*not* zero-terminated by this routine)
static Char* intToString(Char buffer[32], Int32 i) {
	Char* p = buffer + 32;
	*--p = "9876543210123456789"[9 + i % 10];																			// Mirrored string to handle negative x.
	Int32 x = i;
	while ((x /= 10) != 0) *--p = "9876543210123456789"[9 + x % 10];													// Mirrored string to handle negative x.
	if (i < 0) *--p = '-';
	assert(p >= buffer);
	return p;
}

// Based on code from PikaScript
// Returns pair of pointers to beginning and end of string
static std::pair<Char*, Char*> doubleToString(Char buffer[32], double d, int precision = 15) {
	assert(1 <= precision && precision <= 24);

	const double SMALL = 1.0e-6;
	const double LARGE = 1.0e+21;

	double x = fabs(d);

	Char* bp = buffer + 2;
	Char* dp = bp;
	Char* pp = dp + 1;
	Char* ep = pp + precision;
	
	double y = x;	
	for (; x >= 10.0 && pp < ep; x *= 0.1) ++pp;																		// Normalize values > 10 and move period position.

	if (pp >= ep || y <= SMALL || y >= LARGE) {																			// Exponential treatment of very small or large values.
		double e = floor(log10(y) + 1.0e-10);
		int maxp = 15;																									// Limit precision because of rounding errors in log10 etc
		for (double f = fabs(e); f >= 8; f /= 10) --maxp;
		std::pair<Char*, Char*> r = doubleToString(buffer, d * pow(0.1, e), std::min(maxp, precision));
		*r.second++ = 'e';
		if (e >= 0) *r.second++ = '+';
		bp = intToString(buffer, static_cast<Int32>(e));
		assert(bp >= r.second);
		r.second = std::copy(bp, buffer + 32, r.second);
		return r;
	}
	for (; x < 1.0 && dp < buffer + 32; ++ep, x *= 10.0) {																// For values < 1, spit out leading 0's and increase precision.
		*dp++ = '0';
		if (dp == pp) *dp++ = '9';																						// Hop over period position (set to 9 to avoid when eliminating 9's).
	}
	for (; dp < ep; ) {																									// Exhaust all remaining digits of mantissa into buffer.
		UInt32 ix = static_cast<UInt32>(x);
		*dp++ = ix + '0';
		if (dp == pp) *dp++ = '9';																						// Hop over period position (set to 9 to avoid when eliminating 9's).
		x = (x - ix) * 10.0;
	}
	if (x >= 5) {																										// If remainder is >= 5, increment trailing 9's...
		while (dp[-1] == '9') *--dp = '0';
		if (dp == bp) *--bp = '1';																						// If we are at spare position, set to '1' and include, otherwise, increment last non-9.
		else dp[-1]++;
	}
	*pp = '.';
	if (ep > pp) while (ep[-1] == '0') --ep;
	if (ep - 1 == pp) --ep;
	if (d < 0) *--bp = '-';	
	return std::pair<Char*, Char*>(bp, ep);
}

static const Char* eatStringWhite(const Char* p, const Char* e) {
	while (p < e) {
		switch (*p) {
			case '\t': case '\n': case '\v': case '\f': case '\r': case ' ': break; // FIX : support U+00A0	NO-BREAK SPACE	<NBSP>, U+2028	LINE SEPARATOR	<LS>, U+2029	PARAGRAPH SEPARATOR, U+FEFF	ZERO WIDTH NO-BREAK SPACE	 ?
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
	if (p != e && p[0] == '0' && p + 1 != e && p[1] == 'x') {
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
const String ZERO_STRING("0");
const String LENGTH_STRING("length");
const String ARGUMENTS_STRING("arguments");
const String A_RGUMENTS_STRING("Arguments");
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
const String ERROR_NAMES[ERROR_TYPE_COUNT] = {
	String("Error"), String("RangeError"), String("ReferenceError"), String("SyntaxError"), String("TypeError")
};

/* --- Value --- */

const String* Value::TYPE_STRINGS[7] = { 0, &UNDEFINED_STRING, &OBJECT_STRING, &BOOLEAN_STRING, &NUMBER_STRING, &STRING_STRING, 0 };

const String* Value::typeOfString() const {
	return (type == OBJECT_TYPE ? var.object->typeOfString() : TYPE_STRINGS[static_cast<int>(type)]);
}

bool Value::toBool() const {
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE:
		case NULL_TYPE: return false;
		case BOOLEAN_TYPE: return var.boolean;
		case NUMBER_TYPE: return (var.number != 0.0 && !isNaN(var.number));
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

Int32 Value::toInt() const {
	double v = toDouble();
	int i = static_cast<Int32>(v);
	if (i == v) { // FIX : <2^32 but with decimals will now go via slow fmod, right? unneccessary
		return i;
	} else if (!isFinite(v)) {
		return 0;
	} else {
		return (v >= 0) ? static_cast<UInt32>(fmod(v, 4294967296.0)) : -static_cast<UInt32>(fmod(-v, 4294967296.0));
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
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return 0;
		case NULL_TYPE: return 0;
		case BOOLEAN_TYPE: return new WrapperObject(&B_OOLEAN_STRING, *this, &engine.booleanPrototype);
		case NUMBER_TYPE: return new WrapperObject(&N_UMBER_STRING, *this, &engine.numberPrototype);
		case STRING_TYPE: return (requestExtensible ? new WrapperObject(&S_TRING_STRING, *this, &engine.stringPrototype) : static_cast<Object*>(const_cast<String*>(var.string))); // I think const_cast can be accepted here, the string is always immutable when accessed as an Object
		case OBJECT_TYPE: return var.object;
	}
}

bool Value::operator==(const Value& r) const {
	if (type != r.type) return false;
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE:
		case NULL_TYPE:
		case NONEXISTENT_TYPE: return true;
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
		case NULL_TYPE:
		case NONEXISTENT_TYPE: return false;
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

bool Value::isLessThan(const Value& y) const {
	if (type == STRING_TYPE && y.type == STRING_TYPE) return var.string->isLessThan(*y.var.string);
	double thisNumber = toDouble();
	double otherNumber = y.toDouble();
	return thisNumber < otherNumber;
//	FIX : need to check?
//	If nx is +0 and ny is −0, return false.
//	If nx is −0 and ny is +0, return false.
//	If nx is +∞, return false.
//	If ny is +∞, return true.
//	If ny is −∞, return false.
//	If nx is −∞, return true.
}

bool Value::isLessThanOrEqualTo(const Value& y) const {
	if (type == STRING_TYPE && y.type == STRING_TYPE) return !y.var.string->isLessThan(*var.string);
	double thisNumber = toDouble();
	double otherNumber = y.toDouble();
	return thisNumber <= otherNumber;
//	FIX : need to check?
//	If nx is +0 and ny is −0, return false.
//	If nx is −0 and ny is +0, return false.
//	If nx is +∞, return false.
//	If ny is +∞, return true.
//	If ny is −∞, return false.
//	If nx is −∞, return true.
}

Value Value::add(GCHeap& heap, const Value& y) const {
	if (type == STRING_TYPE || y.type == STRING_TYPE) {
		const String* ls = toString(heap);
		const String* rs = y.toString(heap);
		if (rs->empty()) return Value(ls);
		if (ls->empty()) return Value(rs);
		return Value(heap.manage(new String(*ls, *rs)));
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

const Value Value::NONEXISTENT(Value::NONEXISTENT_TYPE);
const Value Value::UNDEFINED(Value::UNDEFINED_TYPE);
const Value Value::NUL(Value::NULL_TYPE);
const Value Value::NOT_A_NUMBER(NaN());
const Value Value::INFINITE_NUMBER(std::numeric_limits<double>::infinity());

/* --- String --- */

const int QUICK_CONSTANTS_INTEGERS_RANGE = 1000;
struct QuickConstants {
	QuickConstants() {
		try {
			std::fill_n(integers, QUICK_CONSTANTS_INTEGERS_RANGE * 2 + 1, (String*)(0));
			for (int i = 0; i < 127; ++i) {
				Char buf[1] = { static_cast<Char>(i) };
				ascii[i] = Value(new String(buf, buf + 1));
			}
			for (int i = -QUICK_CONSTANTS_INTEGERS_RANGE; i <= QUICK_CONSTANTS_INTEGERS_RANGE; ++i) {
				Char buffer[32];
				integers[i + QUICK_CONSTANTS_INTEGERS_RANGE] = new String(intToString(buffer, i), buffer + 32); // FIX : newHashedString
				integers[i + QUICK_CONSTANTS_INTEGERS_RANGE]->calcHash();
			}
		}
		catch (...) {
			cleanup();
			throw;
		}
	}
	void cleanup() {
		for (int i = 0; i < 127; ++i) {
			delete ascii[i].getString();
			ascii[i] = Value::NONEXISTENT;
		}
		for (int i = -QUICK_CONSTANTS_INTEGERS_RANGE; i <= QUICK_CONSTANTS_INTEGERS_RANGE; ++i) {
			delete integers[i + QUICK_CONSTANTS_INTEGERS_RANGE];
			integers[i + QUICK_CONSTANTS_INTEGERS_RANGE] = 0;
		}
	}
	~QuickConstants() { cleanup(); }
	Value ascii[127];
	String* integers[QUICK_CONSTANTS_INTEGERS_RANGE * 2 + 1];
};

QuickConstants QUICK_CONSTANTS;

String::String() : QArray(0), hash(2166136261U) { assert(isEqualTo(EMPTY_STRING)); }
String::String(const Char* b, const Char* e) : QArray(e - b), hash(0) { std::copy(b, e, begin()); }
// FIX : needed ? String::String(const String& o) : QArray(o), hash(o.hash) { }
String::String(const std::wstring& s) : QArray(s.size()), hash(0) { std::copy(s.begin(), s.end(), begin()); }

String::String(const String& l, const String& r) : QArray(l.size() + r.size()), hash(0) {
	Char* e = std::copy(l.begin(), l.end(), begin());
	std::copy(r.begin(), r.end(), e);
}

const String* String::typeOfString() const { return &STRING_STRING; }
const String* String::getClassName() const { return &S_TRING_STRING; }
const String* String::toString(GCHeap& heap) const { return this; }
Object* String::getPrototype(Engine& engine) const { return &engine.stringPrototype; }

bool String::getProperty(Engine& engine, const Value& key, Value* v) const {
	if (key.isString() && key.getString()->isEqualTo(LENGTH_STRING)) {
		*v = Value(static_cast<Int32>(size()));
		return true;
	}
	const double d = key.toDouble(); // FIX : this is not right, it shouldn't accept "  123  " as a number...
	const Int32 i = static_cast<Int32>(d);
	if (i == d && i >= 0 && i < size()) {
		const Char* p = begin() + i;
		if (*p < 127) *v = QUICK_CONSTANTS.ascii[*p];
		else *v = Value(engine.getHeap().manage(new String(p, p + 1)));
		return true;
	}
	return false;
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
	return engine.getHeap().manage(new RangeEnumerator(engine.getHeap(), 0, size()));
}

const String* String::newHashedString(GCHeap& heap, const Char* b, const Char* e) {
	String* o = heap.manage(new String(b, e));
	o->calcHash();
	return o;
}

const String* String::fromInt(GCHeap& heap, Int32 i) {
	Char buffer[32];
	return (i >= -QUICK_CONSTANTS_INTEGERS_RANGE && i <= QUICK_CONSTANTS_INTEGERS_RANGE)
		 ? QUICK_CONSTANTS.integers[i + QUICK_CONSTANTS_INTEGERS_RANGE]
		 : heap.manage(new String(intToString(buffer, i), buffer + 32));
}

const String* String::fromDouble(GCHeap& heap, double d) {
	Int32 i = static_cast<Int32>(d);
	if (i == d) return fromInt(heap, i);
	if (isNaN(d)) return &NAN_STRING;
	double x = fabs(d);
	if (x <= EPSILON) return &ZERO_STRING;
	if (x == std::numeric_limits<double>::infinity()) return d < 0 ? &MINUS_INFINITY_STRING : &INFINITY_STRING; // FIX : put in doubleToString for consistency
	Char buffer[32];
	std::pair<Char*, Char*> r = doubleToString(buffer, d);
	return heap.manage(new String(r.first, r.second));
}

/* --- GCItem --- */

GCItem::GCItem() throw() : _gcList(0), _gcPrev(this), _gcNext(this) { }
GCItem::GCItem(const GCItem& copy) throw() : _gcList(0), _gcPrev(0), _gcNext(0) { }
GCItem& GCItem::operator=(const GCItem& copy) throw() { _gcList = 0; _gcPrev = 0; _gcNext = 0; return *this; }
void GCItem::gcMark(GCHeap& heap) const { }
GCItem::~GCItem() { if (_gcList != 0) _gcList->relinquish(this); }

/* --- GCList --- */

GCList::GCList() throw() : count(0) { }

UInt32 GCList::size() const { return count; }

void GCList::relinquish(GCItem* item) throw() {
	assert(item->_gcList == this);
	assert(count > 0);
	count--;
	item->_gcPrev->_gcNext = item->_gcNext;
	item->_gcNext->_gcPrev = item->_gcPrev;
}

void GCList::claim(GCItem* item) throw() {
	if (item->_gcList != 0) item->_gcList->relinquish(item);
	item->_gcPrev = this;
	item->_gcNext = _gcNext;
	_gcNext->_gcPrev = item;
	_gcNext = item;
	item->_gcList = this;
	++count;
}

/* --- Object --- */

static struct EmptyEnumerator : public Enumerator { const String* nextPropertyName() { return 0; } } EMPTY_ENUMERATOR;

const String* Object::typeOfString() const { return &OBJECT_STRING; }
const String* Object::getClassName() const { return &O_BJECT_STRING; }
Object* Object::getPrototype(Engine& engine) const { return &engine.objectPrototype; }
Value Object::getInternalValue(GCHeap& heap) const { return Value::UNDEFINED; }
bool Object::getProperty(Engine& engine, const Value& key, Value* v) const { return false; }
bool Object::setProperty(Engine& engine, const Value& key, const Value& v) { return false; }
bool Object::deleteProperty(Engine& engine, const Value& key) { return false; }
Enumerator* Object::getPropertyEnumerator(Engine& engine) const { return &EMPTY_ENUMERATOR; }
bool Object::isPropertyEnumerable(Engine& engine, const Value& key) const { return hasProperty(engine, key); }

const String* Object::toString(GCHeap& heap) const {
	return heap.manage(new String(String(BRACKET_OBJECT_STRING, *getClassName()), END_BRACKET_STRING));
}

bool Object::hasInstance(Engine& engine, Processor& processor, Object* object) const {
	processor.error(TYPE_ERROR);
	return false;
}
Value Object::call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
	processor.error(TYPE_ERROR);
	return Value();
}
Value Object::construct(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
	return call(engine, processor, argc, argv, thisObject);
}
bool Object::defineProperty(Engine& engine, const Value& key, const Value& v, const Attributes& attributes) {
	return setProperty(engine, key, v);
}
bool Object::hasProperty(Engine& engine, const Value& key) const {
	Value dummy;
	return getProperty(engine, key, &dummy);
}
bool Object::hasPropertyDeep(Engine& engine, const Value& key) const {
	Value dummy;
	return getPropertyDeep(engine, key, &dummy);
}
bool Object::getPropertyDeep(Engine& engine, const Value& key, Value* v) const {
	const Object* o = this;
	do {
		if (o->getProperty(engine, key, v)) {
			return true;
		}
		o = o->getPrototype(engine);
	} while (o != 0);
	return false;
}
Value Object::getPropertyDeep(Engine& engine, const char* name) const { // FIX : keep?
	Value v;
	String s(name);
	getPropertyDeep(engine, Value(&s), &v);
	return v;
}

/* --- Table --- */

UInt32 Table::calcMaxLoad(UInt32 bucketCount) { return (bucketCount - (bucketCount >> 4) - 1); }

Table::Table(UInt32 n) : n(TABLE_BUILT_IN_N), buckets(1U << TABLE_BUILT_IN_N), loadCount(0) { }

UInt32 Table::getLoadCount() const { return loadCount; }
const Table::Bucket* Table::getFirst() const { return getNext(buckets.begin() - 1); }

const Table::Bucket* Table::getNext(const Bucket* bucket) const {
	assert(bucket != 0);
	const Bucket* e = buckets.end();
	while (++bucket != e && !bucket->valueExists()) { }
	return (bucket == e ? 0 : bucket);
}

const Table::Bucket* Table::lookup(const String* key) const {
	const Bucket* bucket = find(key, key->calcHash());
	return (bucket->valueExists() ? bucket : 0);
}

bool Table::assign(const String* key, const Value& value, Byte flags) {
	assert(value.exists());
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
		bucket->flags = KEY_EXISTS | VALUE_EXISTS | flags;
		bucket->hash16 = static_cast<UInt16>(hash);
		bucket->key = key;
		bucket->type = static_cast<Byte>(value.type);
		bucket->var = value.var;
	} else if ((bucket->flags & READ_ONLY) != 0) {
		return false;
	} else {
		bucket->flags |= KEY_EXISTS | VALUE_EXISTS | flags;
		bucket->type = static_cast<Byte>(value.type);
		bucket->var = value.var;
	}
	return true;
}

bool Table::erase(const String* key) {
	Bucket* bucket = find(key, key->calcHash());
	if (bucket->valueExists()) {
		if ((bucket->flags & DONT_DELETE) != 0) {
			return false;
		}
		bucket->flags = KEY_EXISTS;
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
			if (i != (hash & mask)) {
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
	QuickArray<Bucket, 1U << TABLE_BUILT_IN_N> newBuckets(1U << newN);
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

/* --- UserObject --- */

UserObject::UserObject(Object* prototype) : prototype(prototype) { }

Object* UserObject::getPrototype(Engine& engine) const { return prototype; }

bool UserObject::setProperty(Engine& engine, const Value& key, const Value& v) {
	properties.assign(key.toString(engine.getHeap()), v);
	return true;
}

bool UserObject::getProperty(Engine& engine, const Value& key, Value* v) const {
	const Table::Bucket* bucket = properties.lookup(key.toString(engine.getHeap()));
	if (bucket != 0) {
		*v = bucket->getValue();
		return true;
	}
	return false;
}

bool UserObject::deleteProperty(Engine& engine, const Value& key) {
	return properties.erase(key.toString(engine.getHeap()));
}

Enumerator* UserObject::getPropertyEnumerator(Engine& engine) const {
	StringListEnumerator* list = engine.getHeap().manage(new StringListEnumerator(properties.getLoadCount()));
	for (const Table::Bucket* bucket = properties.getFirst(); bucket != 0; bucket = properties.getNext(bucket)) {
		if (bucket->enumerate()) {
			list->add(bucket->getKey());
		}
	}
	return list;
}

bool UserObject::isPropertyEnumerable(Engine& engine, const Value& key) const {
	const Table::Bucket* bucket = properties.lookup(key.toString(engine.getHeap()));
	return (bucket != 0 && bucket->enumerate());
}

bool UserObject::defineProperty(Engine& engine, const Value& key, const Value& v, const Attributes& attributes) {
	return properties.assign(key.toString(engine.getHeap()), v, (attributes.readOnly ? Table::READ_ONLY : 0)
			| (attributes.dontEnum ? Table::DONT_ENUM : 0)
			| (attributes.dontDelete ? Table::DONT_DELETE : 0));
}

/* --- WrapperObject --- */

WrapperObject::WrapperObject(const String* className, const Value& value, Object* prototype)
	: className(className), wrapped(value), super(prototype) {
}

const String* WrapperObject::getClassName() const { return className; }
const String* WrapperObject::toString(GCHeap& heap) const { return wrapped.toString(heap); }
Value WrapperObject::getInternalValue(GCHeap& heap) const { return wrapped; }

/* --- StringWrapper --- */

StringWrapper::StringWrapper(const String* string, Object* prototype)
		: wrapped(string), super(prototype) {
}

const String* StringWrapper::getClassName() const { return &S_TRING_STRING; }
const String* StringWrapper::toString(GCHeap& heap) const { return wrapped; }
Value StringWrapper::getInternalValue(GCHeap& heap) const { return Value(wrapped); }

bool StringWrapper::defineProperty(Engine& engine, const Value& key, const Value& v, const Attributes& attributes) {
	if (wrapped->hasProperty(engine, key)) {
		return false;
	}
	return super::defineProperty(engine, key, v, attributes);
}

bool StringWrapper::getProperty(Engine& engine, const Value& key, Value* v) const {
	if (wrapped->getProperty(engine, key, v)) {
		return true;
	}
	return super::getProperty(engine, key, v);
}

bool StringWrapper::setProperty(Engine& engine, const Value& key, const Value& v) {
	if (wrapped->hasProperty(engine, key)) {
		return false;
	}
	return super::setProperty(engine, key, v);
}

bool StringWrapper::deleteProperty(Engine& engine, const Value& key) {
	if (wrapped->hasProperty(engine, key)) {
		return false;
	} else {
		return super::deleteProperty(engine, key);
	}
}

Enumerator* StringWrapper::getPropertyEnumerator(Engine& engine) const {
	return engine.getHeap().manage(new MergingEnumerator(engine, super::getPropertyEnumerator(engine), wrapped));
}

bool StringWrapper::isPropertyEnumerable(Engine& engine, const Value& key) const {
	return wrapped->isPropertyEnumerable(engine, key) || super::isPropertyEnumerable(engine, key);
}

/* --- RangeEnumerator --- */

RangeEnumerator::RangeEnumerator(GCHeap& heap, Int32 from, Int32 count) : heap(heap), index(from), to(from + count) { }
const String* RangeEnumerator::nextPropertyName() { return (index >= to ? 0 : String::fromInt(heap, index++)); }

/* --- StringListEnumerator --- */

StringListEnumerator::StringListEnumerator(UInt32 initialCapacity) : list(initialCapacity), nextIndex(0) { }
void StringListEnumerator::add(const String* s) { list.append(s); }
const String* StringListEnumerator::nextPropertyName() { return (nextIndex < list.size() ? list[nextIndex++] : 0); }

/* --- MergingEnumerator --- */

MergingEnumerator::MergingEnumerator(Engine& engine, Enumerator* leftEnumerator, const Object* rightObject)
		: engine(engine)
		, leftEnumerator(leftEnumerator)
		, rightObject(rightObject)
		, rightEnumerator(rightObject->getPropertyEnumerator(engine)) {
}

const String* MergingEnumerator::nextPropertyName() {
	const String* name = rightEnumerator->nextPropertyName();
	if (name == 0) {
		do {
			name = leftEnumerator->nextPropertyName();
			if (name != 0 && !rightObject->isPropertyEnumerable(engine, Value(name))) {
				return name;
			}
		} while (name != 0);
	}
	return name;
}

/* --- CodeDef --- */

CodeDefinition::CodeDefinition() : name(0), selfName(0), source(0) { }

bool CodeDefinition::lookupNameIndex(const String* name, Int32& index) const {
	const Table::Bucket* bucket = nameIndexes.lookup(name);
	if (bucket == 0) return false;
	index = bucket->getValue().toInt();
	return true;
}

/* --- Function --- */

const String* Function::typeOfString() const { return &FUNCTION_STRING; }
const String* Function::getClassName() const { return &F_UNCTION_STRING; }
const String* Function::toString(GCHeap& heap) const { return &NATIVE_FUNCTION_STRING; }
Value Function::getInternalValue(GCHeap& heap) const { return Value(&NATIVE_FUNCTION_STRING); }
Object* Function::getPrototype(Engine& engine) const { return &engine.functionPrototype; };

bool Function::hasInstance(Engine& engine, Processor& processor, Object* object) const {
	if (object == 0) {
		return false;
	}
	Value v;
	Object* prototypeObject;
	if (!getProperty(engine, Value(&PROTOTYPE_STRING), &v) || (prototypeObject = v.asObject()) == 0) {
		processor.error(TYPE_ERROR);
		return false;
	}
	while ((object = object->getPrototype(engine)) != 0) {
		if (object == prototypeObject) {
			return true;
		}
	}
	return false;
}

/* --- Object support --- */

class ObjectConstructor : public Function {
// FIX : if we allow a object prototype to change after construction we could override construct here as well and just change the prototype of thisObject, this would save an extra new UserObject when running "new Object()"
	public:		virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
					return Value(engine.getHeap().manage(new UserObject(&engine.objectPrototype)));
				}
};

static ObjectConstructor OBJECT_OBJECT; // for object literals

const String* Array::getClassName() const { return &A_RRAY_STRING; }
Object* Array::getPrototype(Engine& engine) const { return &engine.arrayPrototype; }

Int32 Array::keyToIndex(const Value& key) {
	const double d = key.toDouble(); // FIX : util method in value // FIX : shouldn't accept "  123  "
	const Int32 i = static_cast<Int32>(d);
	return (i == d && 0 <= i ? i : -1);
}

bool Array::getProperty(Engine& engine, const Value& key, Value* v) const {
	const Int32 i = keyToIndex(key);
	if (i >= 0 && i < elements.size() && elements[i].exists()) {
		*v = elements[i];
		return true;
	}
	if (key.isString() && key.getString()->isEqualTo(LENGTH_STRING)) {
		*v = Value(elements.size());
		return true;
	}
	return super::getProperty(engine, key, v);
}

bool Array::setProperty(Engine& engine, const Value& key, const Value& v) {
	const Int32 i = keyToIndex(key);
	if (i >= 0) {
		if (i >= elements.size()) {
			const Int32 oldSize = elements.size();
			elements.resize(i + 1);
			std::fill(elements.begin() + oldSize, elements.end(), Value::NONEXISTENT); // FIX : I feel like this should be part of vector actually. resize(x, fillValue) or make that SafeValue that initialises to NONEXISTENT?
		}
		elements[i] = v;
		return true;
	}
	if (key.isString() && key.getString()->isEqualTo(LENGTH_STRING)) {
		// FIX : error if not UInt32! but how if we are to go to exception-lessness?
		const Int32 i = keyToIndex(v);
		if (i < 0) {
			throw Exception(engine.getHeap(), Value(engine.getHeap().manage(new Error(RANGE_ERROR)))); // FIX : how could we do this without exceptions?
		}
		const Int32 oldSize = elements.size();
		if (i != oldSize) {
			elements.resize(i);
			if (oldSize < i) {
				std::fill(elements.begin() + oldSize, elements.end(), Value::NONEXISTENT);
			}
		}
		return true;
	}
	return super::setProperty(engine, key, v);
}

bool Array::deleteProperty(Engine& engine, const Value& key) {
	const Int32 i = keyToIndex(key);
	if (i >= 0 && i < elements.size()) {
		elements[i] = Value::NONEXISTENT;
		return true;
	}
	return super::deleteProperty(engine, key);
}

void Array::constructCompleteObject(Engine& engine) const { }

// FIX : we should have GEN_ARRAY, GEN_OBJECT I think...

class ArrayConstructor : public Function {
// FIX : if we allow a object prototype to change after construction we could override construct here as well and just change the prototype of thisObject, this would save an extra new UserObject when running "new Object()"
	public:		virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
					Array* array = engine.getHeap().manage(new Array());
					if (argc == 1 && argv[0].typeOfString()->isEqualTo(NUMBER_STRING)
							&& argv[0].toDouble() == static_cast<UInt32>(argv[0].toInt())) { // FIX : more efficient way?
						array->setProperty(engine, Value(&LENGTH_STRING), argv[0]);
					} else {
						for (Int32 i = 0; i < argc; ++i) {
							array->setProperty(engine, Value(i), argv[i]);
						}
					}
					return Value(array);
				}
	public:		virtual bool getProperty(Engine& engine, const Value& key, Value* v) const {
					if (key.isString() && key.getString()->isEqualTo(PROTOTYPE_STRING)) {
						*v = Value(&engine.arrayPrototype);
						return true;
					}
					return false;
				}
};

static ArrayConstructor ARRAY_OBJECT; // for array literals

#if 0
class FunctionConstructor : public Function {
// FIX : if we allow a object prototype to change after construction we could override construct here as well and just change the prototype of thisObject, this would save an extra new UserObject when running "new Object()"
	public:		virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
					Array* array = engine.getHeap().manage(new Function(&engine.functionPrototype));
					if (argc == 1 && argv[0].typeOfString()->isEqualTo(NUMBER_STRING) && argv[0].toDouble() == static_cast<UInt32>(argv[0].toInt())) { // FIX : more efficient way?
						array->setProperty(engine, &LENGTH_STRING, argv[0]);
					} else {
						GCHeap& heap = engine.getHeap();
						for (Int32 i = 0; i < argc; ++i) {
							array->setProperty(engine, String::fromInt(heap, i), argv[i]);
						}
					}
					return Value(array);
				}
};
#endif

/* --- LazyObject --- */

template<class SUPER> LazyObject<SUPER>::LazyObject() : completeObject(0) { }

template<class SUPER> bool LazyObject<SUPER>::getProperty(Engine& engine, const Value& key, Value* v) const {
	return getCompleteObject(engine)->getProperty(engine, key, v);
}

template<class SUPER> bool LazyObject<SUPER>::setProperty(Engine& engine, const Value& key, const Value& v) {
	return getCompleteObject(engine)->setProperty(engine, key, v);
}

template<class SUPER> bool LazyObject<SUPER>::deleteProperty(Engine& engine, const Value& key) {
	return getCompleteObject(engine)->deleteProperty(engine, key);
}

template<class SUPER> Enumerator* LazyObject<SUPER>::getPropertyEnumerator(Engine& engine) const {
	return getCompleteObject(engine)->getPropertyEnumerator(engine);
}

template<class SUPER> bool LazyObject<SUPER>::defineProperty(Engine& engine, const Value& key, const Value& v, const Attributes& attributes) {
	return getCompleteObject(engine)->defineProperty(engine, key, v, attributes);
}

template<class SUPER> bool LazyObject<SUPER>::isPropertyEnumerable(Engine& engine, const Value& key) const {
	return getCompleteObject(engine)->isPropertyEnumerable(engine, key);
}

template<class SUPER> UserObject* LazyObject<SUPER>::getCompleteObject(Engine& engine) const {
	if (completeObject == 0) {
		completeObject = engine.getHeap().manage(new UserObject(SUPER::getPrototype(engine)));
		constructCompleteObject(engine);
	}
	return completeObject;
}

/* --- ExtensibleFunction --- */

void ExtensibleFunction::createPrototypeObject(Engine& engine, Object* forObject) const {
	UserObject* prototype = engine.getHeap().manage(new UserObject(&engine.objectPrototype));
	Attributes attributes;
	attributes.dontEnum = true;
	prototype->defineProperty(engine, Value(&CONSTRUCTOR_STRING), Value(const_cast<ExtensibleFunction*>(this)), attributes);
	attributes.dontDelete = true;
	forObject->defineProperty(engine, Value(&PROTOTYPE_STRING), Value(prototype), attributes);
}

/* --- UserFunction --- */

UserFunction::UserFunction(CodeDefinition* codeDef, Object* environment) : codeDef(codeDef), environment(environment) { }
Value UserFunction::getInternalValue(GCHeap& heap) const { return Value(toString(heap)); }
CodeDefinition* UserFunction::getDefinition() const { return codeDef; }
Object* UserFunction::getEnvironment() const { return environment; }

const String* UserFunction::toString(GCHeap& heap) const {
	const String* s = codeDef->getSource();
	return (s != 0 ? s : Object::toString(heap));
}

Value UserFunction::call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
	processor.chain(this, argc, argv, thisObject);
	return Value();
}

void UserFunction::constructCompleteObject(Engine& engine) const {
	createPrototypeObject(engine, completeObject);
	Attributes attributes;
	attributes.dontEnum = true;
	attributes.readOnly = true;
	completeObject->defineProperty(engine, Value(&NAME_STRING), Value(codeDef->getName()), attributes);
	attributes.dontDelete = true;
	completeObject->defineProperty(engine, Value(&LENGTH_STRING), Value(codeDef->getArgumentsCount()), attributes);
}

/* --- SpecialConstructorFunction --- */

SpecialConstructorFunction::SpecialConstructorFunction(Object* regularFunction, Object* constructorFunction)
		: regularFunction(regularFunction), constructorFunction(constructorFunction) {
	assert(regularFunction != 0);
}

Value SpecialConstructorFunction::call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
	return regularFunction->call(engine, processor, argc, argv, thisObject);
}

Value SpecialConstructorFunction::construct(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
	if (constructorFunction == 0) {
		processor.error(TYPE_ERROR);
		return Value();
	} else {
		return constructorFunction->call(engine, processor, argc, argv, thisObject);
	}
}

void SpecialConstructorFunction::constructCompleteObject(Engine& engine) const {
	Object* templateFunction = regularFunction;
	if (constructorFunction != 0) {
		createPrototypeObject(engine, completeObject);
		templateFunction = constructorFunction;
	}
	Value v;
	Attributes attributes;
	attributes.dontEnum = true;
	attributes.readOnly = true;
	if (templateFunction->getProperty(engine, Value(&NAME_STRING), &v)) { // FIX : getName?
		completeObject->defineProperty(engine, Value(&NAME_STRING), v, attributes);
	}
	attributes.dontDelete = true;
	if (templateFunction->getProperty(engine, Value(&LENGTH_STRING), &v)) { // FIX : in the future I think we should have a getLength
		completeObject->defineProperty(engine, Value(&LENGTH_STRING), v, attributes);
	}
}

/* --- FunctionPrototypeFunction --- */

Value FunctionPrototypeFunction::call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
	return Value::UNDEFINED;
}

Object* FunctionPrototypeFunction::getPrototype(Engine& engine) const { return &engine.objectPrototype; }

void FunctionPrototypeFunction::constructCompleteObject(Engine& engine) const {
	Attributes attributes; // FIX : constant for this common attribute?
	attributes.dontEnum = true;
	attributes.readOnly = true;
	completeObject->defineProperty(engine, Value(&NAME_STRING), Value(&EMPTY_STRING), attributes);
	attributes.dontDelete = true;
	completeObject->defineProperty(engine, Value(&LENGTH_STRING), Value(0.0), attributes);
}

/* --- Error --- */

Error::Error(ErrorType type, const String* message)	: errorType(type), name(&ERROR_NAMES[errorType]), message(message) {
	assert(0 <= errorType && errorType < ERROR_TYPE_COUNT);
}

const String* Error::getClassName() const { return &E_RROR_STRING; }
Value Error::getInternalValue(GCHeap& heap) const { return Value(&ERROR_NAMES[errorType]); }
Object* Error::getPrototype(Engine& engine) const { return engine.errorPrototypes[errorType]; }

const String* Error::toString(GCHeap& heap) const {
	return (message == 0 ? name : heap.manage(new String(String(*name, COLON_SPACE), *message))); // FIX : we really need operator+ :)
}

void Error::updateReflection(Engine& engine) {
	Value v;
	name = (getProperty(engine, Value(&NAME_STRING), &v) ? v.toString(engine.getHeap()) : &ERROR_NAMES[errorType]);
	message = (getProperty(engine, Value(&MESSAGE_STRING), &v) ? v.toString(engine.getHeap()) : 0);
}

bool Error::defineProperty(Engine& engine, const Value& key, const Value& v, const Attributes& attributes) {
	const bool result = super::defineProperty(engine, key, v, attributes);
	updateReflection(engine);
	return result;
}

bool Error::setProperty(Engine& engine, const Value& key, const Value& v) {
	const bool result = super::setProperty(engine, key, v);
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
		Attributes attributes;
		attributes.dontEnum = true;
		completeObject->defineProperty(engine, Value(&MESSAGE_STRING), Value(message), attributes);
	}
}

/* --- ErrorPrototype --- */

ErrorPrototype::ErrorPrototype(ErrorType type) : super(type) { }

Object* ErrorPrototype::getPrototype(Engine& engine) const {
	return (errorType == GENERIC_ERROR ? static_cast<Object*>(&engine.objectPrototype)
			: static_cast<Object*>(engine.errorPrototypes[GENERIC_ERROR]));
}

/* --- Arguments --- */

Arguments::Arguments(FunctionFrame* frame, UInt32 argumentsCount)
		: frame(frame)
		, argumentsCount(argumentsCount)
		, lengthProperty(argumentsCount)
		, deletedArguments(argumentsCount + 1) {
	std::fill(deletedArguments.begin(), deletedArguments.end(), false);
}

const String* Arguments::getClassName() const { return &A_RGUMENTS_STRING; }

Value* Arguments::findProperty(const Value& key) const {
	if (key.isString() && key.getString()->isEqualTo(LENGTH_STRING)) {
		return (deletedArguments[0] ? 0 : const_cast<Value*>(&lengthProperty));
	}
	const double d = key.toDouble();
	const int i = static_cast<Int32>(d);
	return (i >= 0 && i < argumentsCount && !deletedArguments[i + 1] ? frame->getIndexedPointer() + i : 0);
}

bool Arguments::getProperty(Engine& engine, const Value& key, Value* v) const {
	const Value* p = findProperty(key);
	return (p == 0 ? super::getProperty(engine, key, v) : (*v = *p, true));
}

bool Arguments::setProperty(Engine& engine, const Value& key, const Value& v) {
	Value* p = findProperty(key);
	return (p == 0 ? super::setProperty(engine, key, v) : (*p = v, true));
}

bool Arguments::deleteProperty(Engine& engine, const Value& key) {
	const Value* p = findProperty(key);
	if (p == 0) return super::deleteProperty(engine, key);
	else if (p == &lengthProperty) return (deletedArguments[0] = true);
	else return (deletedArguments[p - frame->getIndexedPointer() + 1] = true);
}

Enumerator* Arguments::getPropertyEnumerator(Engine& engine) const {
	StringListEnumerator* list = engine.getHeap().manage(new StringListEnumerator(properties.getLoadCount() + argumentsCount + 1));
	for (const Table::Bucket* b = properties.getFirst(); b != 0; b = properties.getNext(b)) {
		list->add(b->getKey());
	}
	for (UInt32 i = 0; i < argumentsCount; ++i)
		if (!deletedArguments[i + 1]) list->add(String::fromInt(engine.getHeap(), i));
	return list;
}

/* --- FunctionFrame --- */

FunctionFrame::FunctionFrame(UserFunction* function, Int32 argc, const Value* argv)
		: super(function->getDefinition()), selfFunction(function), passedArgumentsCount(argc)
		, indexed(codeDef->getVarsCount() + std::max(codeDef->getArgumentsCount(), argc))
		, argumentsValue(Value::NONEXISTENT), environment(function->getEnvironment()), evalVars(0) {
	indexedPointer = indexed.begin() + codeDef->getVarsCount();
	std::fill(indexed.begin(), indexedPointer, Value::UNDEFINED); // FIX : optimize
	Value* e = std::copy(argv, argv + argc, indexedPointer);
	if (codeDef->getArgumentsCount() > argc) {
		std::fill(e, indexed.end(), Value::UNDEFINED);
	}
}

// FIX : this one must GO! Q&D crap
void FunctionFrame::updatedCode() {
	const Int32 oldVarsCount = indexedPointer - indexed.begin();
	const Int32 oldArgsCount = indexed.end() - indexedPointer;
	const Int32 newVarsCount = codeDef->getVarsCount();
	const Int32 newArgsCount = std::max(codeDef->getArgumentsCount(), passedArgumentsCount);
	assert(newVarsCount >= oldVarsCount);
	assert(newArgsCount >= oldArgsCount);
	QuickArray<Value> newIndexed(newVarsCount + newArgsCount);
	std::fill(newIndexed.begin(), newIndexed.begin() + (newVarsCount - oldVarsCount), Value::UNDEFINED);
	std::copy(indexed.begin(), indexed.end(), newIndexed.begin() + (newVarsCount - oldVarsCount));
	std::fill(newIndexed.end() - (newArgsCount - oldArgsCount), newIndexed.end(), Value::UNDEFINED);
	indexed.swap(newIndexed);
	indexedPointer = indexed.begin() + newVarsCount;
}

bool FunctionFrame::defineProperty(Engine& engine, const Value& key, const Value& v, const Attributes& attributes) {
	Int32 index;
	const String* name = key.toString(engine.getHeap());
	if (codeDef->lookupNameIndex(name, index)) return false;
	else if ((codeDef->selfName != 0 && name->isEqualTo(*codeDef->selfName)) || name->isEqualTo(ARGUMENTS_STRING)) return false;
	else return environment->defineProperty(engine, key, v, attributes);
}

bool FunctionFrame::getProperty(Engine& engine, const Value& key, Value* v) const {
	Int32 index;
	const String* name = key.toString(engine.getHeap());
	if (codeDef->lookupNameIndex(name, index)) {
		assert(indexed.begin() <= indexedPointer + index && indexedPointer + index < indexed.end());
		*v = indexedPointer[index];
		return true;
	} else if (name->isEqualTo(ARGUMENTS_STRING)) {
		if (!argumentsValue.exists()) {
			Object* o = engine.getHeap().manage(new Arguments(const_cast<FunctionFrame*>(this), passedArgumentsCount));
			argumentsValue = Value(o);
		}
		*v = argumentsValue;
		return true;
	} else if (codeDef->selfName != 0 && name->isEqualTo(*codeDef->selfName)) {
		*v = Value(selfFunction);
		return true;
	} else {
		return environment->getProperty(engine, key, v); // FIX: can we do a looping non-recursive solution? faster  AND (stack overflow) SAFER!
	}
}

bool FunctionFrame::setProperty(Engine& engine, const Value& key, const Value& v) {
	Int32 index;
	const String* name = key.toString(engine.getHeap());
	if (codeDef->lookupNameIndex(name, index)) {
		assert(indexed.begin() <= indexedPointer + index && indexedPointer + index < indexed.end());
		indexedPointer[index] = v;
		return true;
	} else if (name->isEqualTo(ARGUMENTS_STRING)) {
		argumentsValue = v;
		return true;
	} else if (codeDef->selfName != 0 && name->isEqualTo(*codeDef->selfName)) {
		return false;
	} else {
		return environment->setProperty(engine, key, v);
	}
}

bool FunctionFrame::deleteProperty(Engine& engine, const Value& key) {
	Int32 index;
	const String* name = key.toString(engine.getHeap());
	if (codeDef->lookupNameIndex(name, index)) return false;
	else if ((codeDef->selfName != 0 && name->isEqualTo(*codeDef->selfName)) || name->isEqualTo(ARGUMENTS_STRING)) return false;
	else return environment->deleteProperty(engine, key);
}

void FunctionFrame::mergeEvalVars(GCHeap& heap, CodeDefinition* def) {
	if (evalVars == 0) {
		evalVars = heap.manage(new EvalVarsObject(environment));
		environment = evalVars;
	}
	for (const String** it = def->varNames.begin(); it != def->varNames.end(); ++it) {
		if (*it != 0 && evalVars->vars.lookup(*it) == 0) {
			evalVars->vars.assign(*it, Value::UNDEFINED); 		// FIX : we need a better way to lookup & update in one
		}
	}
}

/* --- EvalFrame --- */

EvalFrame::EvalFrame(const CodeDefinition* evalFunctionDef, Frame* parentFrame) : super(evalFunctionDef)
		, parentFrame(parentFrame), indexed(evalFunctionDef->getVarsCount()) {
	indexedPointer = indexed.end();
}
bool EvalFrame::getProperty(Engine& engine, const Value& key, Value* v) const { return parentFrame->getProperty(engine, key, v); }
bool EvalFrame::setProperty(Engine& engine, const Value& key, const Value& v) { return parentFrame->setProperty(engine, key, v); }
bool EvalFrame::deleteProperty(Engine& engine, const Value& key) { return parentFrame->deleteProperty(engine, key); }
void EvalFrame::mergeEvalVars(GCHeap& heap, CodeDefinition* evalFunctionDef) { parentFrame->mergeEvalVars(heap, evalFunctionDef); }

/* --- CatchFrame --- */

CatchFrame::CatchFrame(Frame* parentFrame, const String* exceptionName, const Value& exceptionValue)
		: super(parentFrame->getFunctionDef()), parentFrame(parentFrame), exceptionName(exceptionName)
		, exceptionValue(exceptionValue) {
	indexedPointer = parentFrame->getIndexedPointer();
}

bool CatchFrame::getProperty(Engine& engine, const Value& key, Value* v) const {
	// FIX : always isString here right?
	if (key.isString() && key.getString()->isEqualTo(*exceptionName)) { *v = exceptionValue; return true; }
	else return parentFrame->getProperty(engine, key, v);
}

bool CatchFrame::setProperty(Engine& engine, const Value& key, const Value& v) {
	if (key.isString() && key.getString()->isEqualTo(*exceptionName)) { exceptionValue = v; return true; }
	else return parentFrame->setProperty(engine, key, v);
}

bool CatchFrame::deleteProperty(Engine& engine, const Value& key) {
	if (key.isString() && key.getString()->isEqualTo(*exceptionName)) return false;
	else return parentFrame->deleteProperty(engine, key);
}

void CatchFrame::mergeEvalVars(GCHeap& heap, CodeDefinition* evalFunctionDef) {
	parentFrame->mergeEvalVars(heap, evalFunctionDef);
}

/* --- Processor --- */

Processor::Processor(Engine& engine, CStackEntry* cstackStart, CStackEntry* cstackEnd, Value* vstackStart, Value* vstackEnd)
	: engine(engine), cstackStart(cstackStart), cstackEnd(cstackEnd), vstackStart(vstackStart), vstackEnd(vstackEnd)
	, clockCyclesLeft(0x7FFFFFFF), ip(0), vsp(vstackStart - 1), csp(cstackStart - 1), frame(0), constants(0), indexed(0) { }

void Processor::push(const Value& v) { assert(vsp + 1 < vstackEnd); *++vsp = v; }
void Processor::poppush(const Value& v) { assert(vsp >= vstackStart); *--vsp = v; }
void Processor::pop(Int32 count) { assert(vsp - count + 1 >= vstackStart); vsp -= count; }
Int32 Processor::operand() { Int32 v = *reinterpret_cast<const Int32*>(ip); ip += OPERAND_SIZE; return v; }
void Processor::resetTimeOut(Int32 clockCycles) { clockCyclesLeft = clockCycles; }

void Processor::error(ErrorType errorType) {
// FIX : which? both works!
//	throwVirtualException(Value(errorObject)
	throw Exception(engine.getHeap(), Value(engine.getHeap().manage(new Error(errorType))));
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

void Processor::chain(UserFunction* func, Int32 argc, const Value* argv, Object* thisObject) {
	frame = engine.getHeap().manage(new FunctionFrame(func, argc, argv));
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
	updateInternals();
}

void Processor::enterEval(CodeDefinition* codeDef, bool directCall) {
--vsp;
	Frame* f = (directCall ? frame : engine.getGlobals());
	f->mergeEvalVars(engine.getHeap(), codeDef);
	frame = engine.getHeap().manage(new EvalFrame(codeDef, f));
	++csp; // FIX : overflow check
	assert(csp < cstackEnd);
	csp->returnIP = ip;
	csp->frame = frame;
	csp->thisObject = csp[-1].thisObject;
	ip = codeDef->getCodePointer();
	updateInternals();
}

void Processor::throwVirtualException(const Value& exception) {
	if (catchers.empty()) { // FIX: what exception to throw here?
		throw Exception(engine.getHeap(), exception);
	}
	const Catcher& catcher = catchers[catchers.size() - 1];
	ip = catcher.ip;
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
			case READ_NAMED_OP:		if (!frame->getProperty(engine, constants[operand()], ++vsp)) { // FIX : assert always string?
										error(REFERENCE_ERROR);
									}
									break;
									
			case WRITE_NAMED_OP:	frame->setProperty(engine, constants[operand()], vsp[0]);
									break;
									
			case GET_PROPERTY_OP:	if ((o = vsp[-1].toObject(engine, false)) == 0) {
										error(TYPE_ERROR);
										break;
									}
									if (!o->getPropertyDeep(engine, vsp[0], vsp - 1)) {
										vsp[-1] = Value::UNDEFINED;
									}
									pop(1);
									break;
			
			case SET_PROPERTY_OP:	if ((o = vsp[-2].toObject(engine, false)) == 0) {
										error(TYPE_ERROR);
										break;
									}
									o->setProperty(engine, vsp[-1], vsp[0]);
									vsp[-2] = vsp[0];
									pop(2);
									break;

			case TO_NUMBER_OP:
				if ((o = vsp[0].asObject()) != 0) {
					engine.toNumberFunction->call(engine, *this, 1, vsp, engine.getGlobals());
				}
				break;

			case TO_STRING_OP:
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
					error(TYPE_ERROR);
					break;
				}
				Value v(Value::NONEXISTENT);
				thisObject->getPropertyDeep(engine, vsp[-n], &v);
				Value* returnPointer = vsp - n - 1; // Notice that call() will change vsp if it is a script function, so we save this first.
				Object* f = v.asObject();
				if (f == 0) {
					error(TYPE_ERROR);
					break;
				}
				*returnPointer = f->call(engine, *this, n, returnPointer + 2, thisObject); // Yeah, this *returnPointer assignment goes to waste if we called a script function (since it returns directly), but so what?
				vsp -= n + 1;
				break;
			}
			case CALL_EVAL_OP: { // FIX : all these are too similar!
				Int32 n = operand();
				Value* returnPointer = vsp - n; // Notice that call() will change vsp if it is a script function, so we save this first.
				Object* f = vsp[-n].asObject();
				if (f == 0) {
					error(TYPE_ERROR);
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
				Object* f = vsp[-n].asObject();
				if (f == 0) {
					error(TYPE_ERROR);
					break;
				}
				*returnPointer = f->call(engine, *this, n, returnPointer + 1, engine.getGlobals()); // Yeah, this *returnPointer assignment goes to waste if we called a script function (since it returns directly), but so what?
				vsp -= n;
				break;
			}
			case NEW_OP: {
				Int32 n = operand(); // fix : top declare all variables in this switch, I seem to recall from GAZL that it is better
				Object* f = vsp[-n].asObject(); // fix: move this logic into f->construct, which means the NEW_RESULT_OP should be a chained function instead
				if (f == 0) {
					error(TYPE_ERROR);
					break;
				}
				Value v(Value::NONEXISTENT);
				f->getProperty(engine, Value(&PROTOTYPE_STRING), &v);
				Object* prototype = v.asObject();
				Object* newObject = heap.manage(new UserObject(prototype != 0 ? prototype : &engine.objectPrototype));
				Value* returnPointer = vsp - n; // Notice that call() will change vsp if it is a script function, so we save this first.
				returnPointer[1] = f->construct(engine, *this, n, returnPointer + 1, newObject);							// Yeah, this *returnPointer assignment goes to waste if we called a script function (since it returns directly), but so what?
				returnPointer[0] = Value(newObject);
				vsp -= n - 1;
				break;
			}
			case NEW_RESULT_OP: poppush(vsp[0].isObject() ? vsp[0] : vsp[-1]); break;
			case THIS_OP: push(Value(csp->thisObject)); break;
			case VOID_OP: push(Value::UNDEFINED); break;
			case GEN_FUNC_OP: {
				o = constants[operand()].asObject(); // FIX : .getObject?
				assert(o != 0);
				push(Value(heap.manage(new UserFunction(reinterpret_cast<CodeDefinition*>(o), frame)))); // FIX : lose reinterpret_cast ?
				break;
			}
			case ADD_PROPERTY_OP: {
				o = vsp[-1].asObject();
				assert(o != 0);
				o->setProperty(engine, constants[operand()], vsp[0]);
				pop(1);
				break;
			}
			
			case FRAME_OP: push(Value(frame)); break;
			case DELETE_OP: {
				o = vsp[-1].toObject(engine, true);
				if (o == 0) {
					error(TYPE_ERROR);
					break;
				}
				poppush(Value(o->deleteProperty(engine, vsp[0])));
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
				frame = csp->frame = heap.manage(new CatchFrame(frame, constants[operand()].getString(), vsp[0]));
				pop(1);
				csp->thisObject = csp[-1].thisObject;
				break;
			}
			case THROW_OP: throwVirtualException(vsp[0]); break;
			case IN_OP: {
				o = vsp[0].asObject();
				if (o == 0) {
					error(TYPE_ERROR);
					break;
				}
				vsp[-1] = Value(o->hasPropertyDeep(engine, vsp[-1]));
				pop(1);
				break;
			}
			case INSTANCE_OF_OP: {
				o = vsp[0].asObject();
				if (o == 0) {
					error(TYPE_ERROR);
					break;
				}
				vsp[-1] = Value(o->hasInstance(engine, *this, vsp[-1].asObject()));
				pop(1);
				break;
			}
			case TYPEOF_OP: vsp[0] = Value(vsp[0].typeOfString()); break;
			case TYPEOF_NAMED_OP: {
				Value v(Value::UNDEFINED);
				frame->getProperty(engine, constants[operand()], &v);
				push(Value(v.typeOfString()));
				break;
			}
			case GET_ENUMERATOR_OP: {
				o = vsp[0].toObject(engine, false);
				// FIX : this is awesome and all, but it *will* become recursive nextPropertyName() calls, and recursive C++ is baaad for sandboxing
				// FIX : also, I don't like the order: prototypes are first (not that it matters much though)
				Enumerator* enumerator = &EMPTY_ENUMERATOR;
				while (o != 0) {
					enumerator = heap.manage(new MergingEnumerator(engine, enumerator, o));
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
		, "TO_PRIMITIVE", "TO_STRING", "PRE_EQ"
		, "INC", "DEC", "ADD", "SUB", "MUL", "DIV", "MOD"
		, "OR", "XOR", "AND"
		, "SHL", "SHR", "USHR"
		, "PLUS", "MINUS"
		, "BIT_NOT", "NOT"
		, "STRICTLY_EQ", "STRICTLY_NEQ", "EQ", "NEQ", "LT", "LEQ", "GT", "GEQ"
		, "JMP", "JSR", "JT", "JF"
		, "CALL", "CALL_METHOD", "CALL_EVAL", "NEW", "NEW_RESULT"
		, "RETURN", "RETURN_TO_HOST"
		, "THIS", "VOID", "FRAME", "DELETE"
		, "GEN_FUNC"
		, "THROW", "TRY", "TRIED", "CATCH_FRAME", "POP_FRAME"
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
			case Processor::READ_NAMED_OP:
			case Processor::WRITE_NAMED_OP:
			case Processor::ADD_PROPERTY_OP:
			case Processor::TYPEOF_NAMED_OP: std::wcerr << L" " << constants[getInt32(code, offset)].getString()->toStdString(); break;
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
};
enum Literal { NULL_LITERAL, FALSE_LITERAL, TRUE_LITERAL, FUNCTION_LITERAL, THIS_LITERAL };

struct Compiler::Scope {
	enum Type { LABEL_TYPE, ITERATOR_LABEL_TYPE, TRY_TYPE, CATCH_TYPE, FINALLY_TYPE };
	Scope(Type type, Scope* next) : type(type), next(next) { }
	Scope(const String& label, Scope* next) : type(LABEL_TYPE), label(label), next(next) { }
	void makeIteratorScopes(Scope* untilScope) {
		for (Scope* s = this; s != untilScope; s = s->next) s->type = ITERATOR_LABEL_TYPE;
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
	ExpressionResult(Type type, const Value& value = Value::NONEXISTENT) : t(type), v(value) { }
	Type t;
	Value v;
};

Compiler::Compiler(GCHeap& heap, CodeDefinition* codeDef, Target compileFor) : heap(heap), codeDef(codeDef)
		, compilingFor(compileFor), outputCode(&codeDef->code), acceptInOperator(true) {
}

void Compiler::error(ErrorType errorType, const char* message) {
	throw Exception(heap, Value(heap.manage(new Error(errorType, heap.manage(new String(message))))));
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
		Int32 index = codeDef->constants.size();
		codeDef->constants.append(constant);
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
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '$' || c == '_');
}

static const Char LINE_TERMINATORS[] = { '\n', 0x2028, 0x2029, '\r' }; // FIX : '\r' must be last for line count to work, but I think we should change line counting algo in the future, a single '\r' without '\n' should count too really

void Compiler::white() {
	while (!eof()) {
		switch (*p) {
			case ' ': case '\v': case '\f': case '\t':	case '\r': case '\n': case 0xA0: case 0x2028: case 0x2029: ++p; break;
			case '/':	if (p + 1 != e && p[1] == '/') {
							p = std::find_first_of(p += 2, e, LINE_TERMINATORS, LINE_TERMINATORS + 4);
							break;
						} else if (p + 1 != e && p[1] == '*') {
							static const Char END_CHARS[] = { '*', '/' };
							p = std::search(p += 2, e, END_CHARS, END_CHARS + 2);
							if (eof()) error(SYNTAX_ERROR, "Missing */");
							p += 2;
							break;
						}
			default:	return;
		}
	}
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
	while (++p < e && *p != endChar && std::find(LINE_TERMINATORS, LINE_TERMINATORS + 4, *p) == LINE_TERMINATORS + 4) {
		if (*p == '\\' && p + 1 < e) ++p;
		++l;
	}
	return l;
}

Char* Compiler::unescape(Char* buffer, const Char* e) {	
	assert(p <= e && (*p == '"' || *p == '\''));
	static const Char ESCAPE_CHARS[] = { '\\', '\"', '\'', 'b',  'f',  'n',  'r',  't',  'v', '0' };
	static const Char ESCAPE_CODES[] = { '\\', '\"', '\'', '\b', '\f', '\n', '\r', '\t', '\v', '\0' };
	const int ESCAPE_CODE_COUNT = sizeof (ESCAPE_CHARS) / sizeof (*ESCAPE_CHARS);
	Char endChar = *p;
	const Char* b = ++p;
	Char* d = buffer;
	while (!eof() && *p != endChar && std::find(LINE_TERMINATORS, LINE_TERMINATORS + 4, *p) == LINE_TERMINATORS + 4) {
		if (*p == '\\' && p + 1 < e) {
			d = std::copy(b, p, d);
			const Char* f = std::find(ESCAPE_CHARS, ESCAPE_CHARS + ESCAPE_CODE_COUNT, *++p);
			b = p;
			UInt32 l = 0;
			if (f != ESCAPE_CHARS + ESCAPE_CODE_COUNT) { ++p; l = ESCAPE_CODES[f - ESCAPE_CHARS]; }
			else if (*p == 'x') { b = ++p; p = parseHex(p, (e - p > 2 ? p + 2 : e), l); } // FIX : in ecma, you *need* two (and four) hex chars, nothing else, throw here!
			else if (*p == 'u') { b = ++p; p = parseHex(p, (e - p > 4 ? p + 4 : e), l); }
			if (p == b) error(SYNTAX_ERROR, "Illegal escape character");
			b = p;
			*d++ = Char(l);
		} else ++p;
	}
	if (eof() || *p != endChar) error(SYNTAX_ERROR, "Unterminated string");
	return std::copy(b, p++, d);
}

Compiler::ExpressionResult Compiler::makeRValue(const ExpressionResult& xr, bool toPrimitive) {
	switch (xr.t) {
		case ExpressionResult::PUSHED: if (toPrimitive) emit(Processor::TO_NUMBER_OP); return xr;
		case ExpressionResult::NONE: emit(Processor::VOID_OP); break;
		case ExpressionResult::CONSTANT: emitWithConstant(Processor::CONST_OP, xr.v); break;
		case ExpressionResult::INDEXED: emitWithInt32(Processor::READ_INDEXED_OP, xr.v.toInt()); if (toPrimitive) emit(Processor::TO_NUMBER_OP); break;
		case ExpressionResult::NAMED: emitWithConstant(Processor::READ_NAMED_OP, xr.v); if (toPrimitive) emit(Processor::TO_NUMBER_OP); break;
		case ExpressionResult::PROPERTY: emit(Processor::GET_PROPERTY_OP); if (toPrimitive) emit(Processor::TO_NUMBER_OP); break;
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
			if (p + 1 != e && (p[1] == 'x' || p[1] == 'X')) {
				UInt32 u;
				const Char* b = p += 2;
				p = parseHex(b, e, u);
				if (p == b) {
					error(SYNTAX_ERROR, "Invalid number");
				}
				return Value(u);
			}
			/* else continue */
		}
		case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '.': {
			double d;
			const Char* b = p;
			p = parseDouble(p, e, d);
			if (p == b) {
				error(SYNTAX_ERROR, "Invalid number");
			}
			return Value(d);
		}
		case '\'': case '"': {
			UInt32 maxLength = unescapedMaxLength(p, e);
			QuickArray<Char, 64> buffer(maxLength);
			Char* unescapeEnd = unescape(buffer, e);
			assert(unescapeEnd <= buffer + maxLength);
			return Value(String::newHashedString(heap, buffer, unescapeEnd));
		}
	}
	return Value();
}

Compiler::ExpressionResult Compiler::arrayInitialiser() { // FIX : share stuff with functionCall and arrayInitialiser
	// FIX : we could add an operator for these three, I guess they will be common
	emitWithConstant(Processor::CONST_OP, Value(&ARRAY_OBJECT));
	emitWithInt32(Processor::NEW_OP, 0);
	emit(Processor::NEW_RESULT_OP); // FIX : make a chain for script constructors and require returning object for native constructors instead?
	
	Int32 index = 0;
	Int32 lastAddedIndex = 0;
	while (!token("]", true)) {
		Compiler::ExpressionResult xr;
		if (optionalExpression(xr, COMMA_PREC)) {
			makeRValue(xr);
			emitWithConstant(Processor::ADD_PROPERTY_OP, Value(String::fromInt(heap, index)));
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
	// FIX : we could add an operator for these three, I guess they will be common
	emitWithConstant(Processor::CONST_OP, Value(&OBJECT_OBJECT));
	emitWithInt32(Processor::NEW_OP, 0);
	emit(Processor::NEW_RESULT_OP); // FIX : make a chain for script constructors and require returning object for native constructors instead?
	
	white();
	while (!token("}", false)) {
		const Char* b = p;
		Value name = stringOrNumberConstant();
		if (p == b) {
			b = identifier(false, true);
			if (p == b) {
				error(SYNTAX_ERROR, "Expected property name");
			} else {
				name = Value(String::newHashedString(heap, b, p));
			}
		}
		expectToken(":", true);
		rvalueExpression(COMMA_PREC);
		emitWithConstant(Processor::ADD_PROPERTY_OP, Value(name.toString(heap)));
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
			makeRValue(xr);
			emit(Processor::TO_STRING_OP);
			xr = makeRValue(operand(op));
			emit(op.vmOp);
			break;
		}

		case POST_INC_DEC: {
			if (xr.t == ExpressionResult::PROPERTY) {
				emit(Processor::REPUSH_2_OP);
			}
			makeRValue(xr, true);
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
			if (xr.t == ExpressionResult::NAMED && xr.v.getString()->isEqualTo(EVAL_STRING)) op = Processor::CALL_EVAL_OP;
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
			makeRValue(operand(op)); // FIX : convert to string const if const, or actually if we allow Value properties and optimize array[double] we shouldn't
			emit(Processor::TO_STRING_OP);
			xr = ExpressionResult(ExpressionResult::PROPERTY);
			break;
		}
		
		default: assert(0);
	}
	return true;
}

void Compiler::functionDefinition(const String* functionName, const String* selfName) {
	assert(functionName != 0);
	CodeDefinition* func = heap.manage(new CodeDefinition());
	Compiler funcCompiler(heap, func, Compiler::FOR_FUNCTION);
	heap.protect(&funcCompiler);
	try {
		p = funcCompiler.compileFunction(p, e, functionName, selfName);
	}
	catch (const Exception& x) { // FIX : a bit Q & D ish
		p = funcCompiler.getStopPoint();
		throw;
	}
	emitWithConstant(Processor::GEN_FUNC_OP, Value(func));
}

bool Compiler::optionalExpression(ExpressionResult& xr, Precedence precedence) {
	white();
	if (eof()) {
		return false;
	}
	if (!operate(xr, precedence, sizeof (PRE_OPS) / sizeof (*PRE_OPS), PRE_OPS)) {
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
					
					default: {
						const Char* b = identifier(false, false);
						if (p == b) {
							return false;
						}
						discard(xr);
						const String* name = String::newHashedString(heap, b, p);
						xr = ExpressionResult(ExpressionResult::NAMED, Value(name));
						if (compilingFor != FOR_EVAL) { // Notice: compilingFor != FOR_EVAL is only for non-strict mode (if we ever support strict-mode in the future)
							const Table::Bucket* bucket = codeDef->nameIndexes.lookup(name);
							if (bucket != 0) {
								xr = ExpressionResult(ExpressionResult::INDEXED, bucket->getValue());
							}
						}
						break;
					}
				}
			}
		}
    }
	const Char* b = p;
    white();
    while (operate(xr, precedence, sizeof (POST_OPS) / sizeof (*POST_OPS) - (acceptInOperator ? 0 : 1), POST_OPS)) {
		b = p;
		white();
	}
	p = b;
	return true;
}

Compiler::ExpressionResult Compiler::expression(Precedence precedence) {
	ExpressionResult xr;
	if (!optionalExpression(xr, precedence)) {
		error(SYNTAX_ERROR, "Expected expression");
	}
	return xr;
}

Compiler::ExpressionResult Compiler::rvalueExpression(Precedence precedence) {
	return makeRValue(expression(precedence));
}

/*
	Normally statements discard expression results, but when compiling for eval, statements should return results.
	flushXR is required to before any conditional branching and before branch targets to produce the result on the
	vstack in case it has not already been pushed (in other words if it is not an rvalue, e.g. a constant).
*/
void Compiler::flushXR(ExpressionResult& evalXR) {
	if (compilingFor == FOR_EVAL) {
		evalXR = makeRValue(evalXR);
	}
}

Compiler::ExpressionResult Compiler::declareIdentifier(const String* name) {
	ExpressionResult lxr(ExpressionResult::NAMED, Value(name));
	if (!name->isEqualTo(ARGUMENTS_STRING)) {
		Value index;
		const Table::Bucket* bucket = codeDef->nameIndexes.lookup(name);
		if (bucket != 0) {
			index = bucket->getValue();
		} else {
			codeDef->varNames.append(name);
			index = Value(-static_cast<Int32>(codeDef->varNames.size()));
			codeDef->nameIndexes.assign(name, index);
		}
		 // Notice: compilingFor != FOR_EVAL is only for non-strict mode (if we ever support strict-mode in the future)
		if (compilingFor != FOR_EVAL) lxr = ExpressionResult(ExpressionResult::INDEXED, index);
	}
	return lxr;
}

Compiler::ExpressionResult Compiler::varDeclaration() {
	const Char* b = identifier(true, false);
	ExpressionResult lxr(declareIdentifier(String::newHashedString(heap, b, p)));
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
	QuickVector<CodeWord> incCode;
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

void Compiler::semicolon() {
	const Char* b = p;
	white();
	if (!eof() && *p != '}' && !token(";", false) && std::find_first_of(b, p, LINE_TERMINATORS, LINE_TERMINATORS + 4) == p) {
		p = b;
		error(SYNTAX_ERROR, "Syntax error");
	}
}

// FIX : make member functions of all the different statements

void Compiler::functionStatement() {
	outputCode = &funcInitCode;
	const Char* b = identifier(true, false);
	const String* name = String::newHashedString(heap, b, p);
	ExpressionResult lxr(declareIdentifier(name));
	functionDefinition(name);
	makeAssignment(lxr);
	emit(Processor::POP_OP);
	outputCode = &codeDef->code;
}

void Compiler::statement(Scope* firstScope, Scope* scopeLabelsEnd, ExpressionResult& evalXR) {
	white();
	
	if (token("{", false)) {
		statementList(firstScope, evalXR);
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
			String label(b, q);
			for (Scope* s = firstScope; s != 0; s = s->next) {
				if (!s->label.empty() && s->label.isEqualTo(label)) {
					error(SYNTAX_ERROR, "Duplicate label");
				}
			}
			Scope newLabelScope(label, firstScope);
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
		white();
		switch (static_cast<Statement>(statementTokenIndex)) {
			case BREAK_STATEMENT:
			case CONTINUE_STATEMENT: {
				flushXR(evalXR);
				bool isBreak = (statementTokenIndex == BREAK_STATEMENT);
				Scope* s = firstScope;
				const Char* b = identifier(false, false);
				// FIX : too complicated!! bone out...
				String label(b, p);
				while (s != 0 && !((s->type == Scope::ITERATOR_LABEL_TYPE
						|| ((isBreak || !label.empty()) && s->type == Scope::LABEL_TYPE))
						&& s->label.isEqualTo(label))) {
					if (s->type == Scope::CATCH_TYPE || s->type == Scope::TRY_TYPE) // FIX : share with return
						s->finallys.append(emitForwardBranch(Processor::JSR_OP));
					if (s->type == Scope::CATCH_TYPE || s->type == Scope::FINALLY_TYPE) emit(Processor::POP_FRAME_OP);
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
				varDeclarations();
				semicolon();
				break;
			}
			
			case IF_STATEMENT: {
				flushXR(evalXR);
				expectToken("(", false);
				rvalueExpression();
				expectToken(")", true);
				Int32 falsePoint = emitForwardBranch(Processor::JF_OP);
				statement(firstScope, firstScope, evalXR);
				if (token("else", true)) {
					Int32 endPoint = emitForwardBranch(Processor::JMP_OP);
					completeForwardBranch(falsePoint);
					statement(firstScope, firstScope, evalXR);
					completeForwardBranch(endPoint);
				} else {
					flushXR(evalXR);
					completeForwardBranch(falsePoint);
				}				
				break;
			}

			case WHILE_STATEMENT: {
				flushXR(evalXR);
				Int32 loopPoint = markBackwardBranch();
				Scope emptyLabelScope(EMPTY_STRING, firstScope);
				emptyLabelScope.makeIteratorScopes(scopeLabelsEnd);
				expectToken("(", false);
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
				flushXR(evalXR);
				Int32 loopPoint = markBackwardBranch();
				Scope emptyLabelScope(EMPTY_STRING, firstScope);
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
				flushXR(evalXR);
				Scope emptyLabelScope(EMPTY_STRING, firstScope);
				emptyLabelScope.makeIteratorScopes(scopeLabelsEnd);
				expectToken("(", false);
				QuickVector<CodeWord> initCode;
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
					outputCode = &initCode;
					acceptInOperator = false;
					ExpressionResult iterXR;
					bool gotExpression = optionalExpression(iterXR, LOWEST_PREC);
					acceptInOperator = true;
					outputCode = &codeDef->code;
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
				optionalExpression(returnXR, LOWEST_PREC);
				Scope* s = firstScope;
				bool safeKept = false;
				while (s != 0) { // FIX : sub
					if (s->type == Scope::CATCH_TYPE || s->type == Scope::TRY_TYPE) {
						if (!safeKept) {
							returnXR = safeKeep(returnXR);
							safeKept = true;
						}
						s->finallys.append(emitForwardBranch(Processor::JSR_OP));
					}
					if (s->type == Scope::CATCH_TYPE || s->type == Scope::FINALLY_TYPE) emit(Processor::POP_FRAME_OP);
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
				flushXR(evalXR);
				Scope tryScope(Scope::TRY_TYPE, firstScope);
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
					Value localIndex = Value::NONEXISTENT;
					const Table::Bucket* bucket = codeDef->nameIndexes.lookup(exceptionVarName);
					if (bucket != 0) {
						localIndex = bucket->getValue();
						codeDef->nameIndexes.erase(exceptionVarName);
					}
					completeForwardBranch(catchPoint);
					catchPoint = emitForwardBranch(Processor::TRY_OP);
					emitWithConstant(Processor::CATCH_FRAME_OP, Value(exceptionVarName));
					expectToken(")", true);
					Scope catchScope(Scope::CATCH_TYPE, firstScope);
					expectToken("{", true); // FIX : sub
					statementList(&catchScope, evalXR);
					expectToken("}", false);
					flushXR(evalXR);
					emit(Processor::POP_FRAME_OP); // FIX : try to move POP_FRAME to first in FINALLY block, need different JSR then
					catchScope.finallys.append(emitForwardBranch(Processor::JSR_OP));
					catchEndPoint = emitForwardBranch(Processor::JMP_OP);
					for (const Int32* it = catchScope.finallys.begin(); it != catchScope.finallys.end(); ++it) // FIX : sub
						completeForwardBranch(*it);
					if (localIndex.exists()) codeDef->nameIndexes.assign(exceptionVarName, localIndex);
				}
				
				// Finally
				
				for (const Int32* it = tryScope.finallys.begin(); it != tryScope.finallys.end(); ++it) // FIX : sub
					completeForwardBranch(*it);
				emit(Processor::TRIED_OP);
				Int32 rethrowPoint = markBackwardBranch();
				if (token("finally", true)) {
					Scope finallyScope(Scope::FINALLY_TYPE, firstScope);
					expectToken("{", true); // FIX : sub
					statementList(&finallyScope, evalXR);
					expectToken("}", false);
					flushXR(evalXR);
				} else {
					if (!gotCatch) error(SYNTAX_ERROR, "Expected 'catch' or 'finally'");
				}
				emit(Processor::RETURN_OP);
				
				// Re-thrower (could actually be eliminated if we only have catch and no finally)
				
				completeForwardBranch(catchPoint);
				ExpressionResult thrownXR(ExpressionResult::PUSHED);
				thrownXR = safeKeep(thrownXR);
				emitBackwardBranch(Processor::JSR_OP, rethrowPoint);
				makeRValue(thrownXR);
				returnSafeKept(thrownXR, true);
				emit(Processor::THROW_OP);
				completeForwardBranch(tryEndPoint);
				if (catchEndPoint != INVALID_CODE_POINT) completeForwardBranch(catchEndPoint);
				break;
			}
			
			case THROW_STATEMENT: {
				rvalueExpression();
				emit(Processor::THROW_OP);
				semicolon();
				break;
			}
			
			case SWITCH_STATEMENT: {
				flushXR(evalXR);
				expectToken("(", true);
				ExpressionResult controlXR(safeKeep(expression(LOWEST_PREC)));
				expectToken(")", true);
				expectToken("{", true);
				Int32 testsPoint = emitForwardBranch(Processor::JMP_OP);
				QuickVector<CodeWord> testsCode;
				QuickVector<Int32> jumpOffsets;
				Int32 startOffset = outputCode->size();
				Int32 defaultPoint = INVALID_CODE_POINT;
				Scope emptyLabelScope(EMPTY_STRING, firstScope);
				bool firstStatement = true;
				white();
				while (!token("}", false)) {
					if (eof()) {
						error(SYNTAX_ERROR, "Expected '}'");
					}
					else if (token("case", false)) {
						flushXR(evalXR);
						outputCode = &testsCode;
						makeRValue(controlXR);
						rvalueExpression();
						expectToken(":", true);
						emit(Processor::X_EQ_OP);
						emitWithInt32(Processor::JT_OP, codeDef->code.size() - startOffset - testsCode.size() - OPERAND_SIZE - 1);
						jumpOffsets.append(testsCode.size() - OPERAND_SIZE);
						outputCode = &codeDef->code;
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
				for (const Int32* it = jumpOffsets.begin(); it != jumpOffsets.end(); ++it)
					accessCodeInt32(testsCode, *it) -= outputCode->size() - startOffset;
				outputCode->insert(outputCode->end(), testsCode.begin(), testsCode.end());
				if (defaultPoint != INVALID_CODE_POINT) emitBackwardBranch(Processor::JMP_OP, defaultPoint);
				completeForwardBranch(donePoint);
				completeBreaks(&emptyLabelScope);
				returnSafeKept(controlXR, true);
				break;
			}
			
			case FUNCTION_STATEMENT: functionStatement(); break;
			
			default: assert(0);
		}
	}
}

void Compiler::statementList(Scope* firstScope, ExpressionResult& evalXR) {
	const Char* b;
	do {
		b = p;
		statement(firstScope, firstScope, evalXR);
	} while (!eof() && b != p);
}

const Char* Compiler::compile(const Char* b, const Char* e) {
	// FIX : only accept one compile? or opposite? create many different functions?
	codeDef->code.resize(0);
	codeDef->constants.resize(0);
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
	if (!funcInitCode.empty()) {
		outputCode->insert(outputCode->begin(), funcInitCode.begin(), funcInitCode.end());
	}
	// FIX : shrink code vector (copy to quickarray instead?)
	assert(outputCode == &codeDef->code);
	codeDef->code.shrink();
	codeDef->constants.shrink();
if (!supressStdErr) {
	disassemble(heap, *codeDef);
	std::cerr << "\tCode: " << codeDef->code.size() << std::endl;
	std::cerr << "\tConstants: " << codeDef->constants.size() << std::endl;
	for (const Value* p = codeDef->constants.begin(); p != codeDef->constants.end(); ++p) {
		std::wcerr << "\t\t" << p->toString(heap)->toStdString() << std::endl;
	}
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
	while (!token(")", false)) {
		if (eof()) {
			error(SYNTAX_ERROR, codeDef->argumentNames.size() == 0 ? "Expected ')'" : "Expected ',' or ')'");
		}
		if (codeDef->argumentNames.size() != 0) {
			if (!token(",", false)) {
				error(SYNTAX_ERROR, "Expected ',' or ')'");
			}
			white();
		}
		const Char* b = identifier(true, false);
		const String* name = String::newHashedString(heap, b, p);
		codeDef->nameIndexes.assign(name, Value(static_cast<Int32>(codeDef->argumentNames.size())));
		codeDef->argumentNames.append(name);
		white();
	}
	expectToken("{", true);
	compile(p, e); // FIX: ugly as it sets p and e again, although it doesn't hurt
	expectToken("}", false);
	codeDef->name = functionName;
	codeDef->selfName = selfName;
	codeDef->source = heap.manage(new String(String(FUNCTION_SPACE, *functionName), String(b, p))); // FIX : operator+ would make things clearer :)
	return p;
}

void Compiler::compile(const String& source) {
	compile(source.begin(), source.end());
	white();
	if (!eof()) {
		error(SYNTAX_ERROR, "Syntax error"); // FIX : better error?
	}
}

GCHeap::GCHeap() : currentList(&managedLists[0]), newList(&managedLists[1]) { }

UInt32 GCHeap::size() const { return currentList->size(); }
UInt32 GCHeap::gc() {
	UInt32 sweptCount = 0;

	for (const GCItem* item = rootList._gcNext; item != &rootList; item = item->_gcNext)
		item->gcMark(*this);
	
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

Value EvalFunction::call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
	if (argc < 1) return Value::UNDEFINED;
	if (!argv[0].typeOfString()->isEqualTo(STRING_STRING)) return argv[0]; // FIX : better way to check type without involving typeOfString?

	const String* sourceCode = argv[0].toString(engine.getHeap());
	CodeDefinition* codeDef;
	
	const Table::Bucket* bucket = functionDefCache.lookup(sourceCode);
	 // FIX : .getObject?
	if (bucket != 0) {
		codeDef = reinterpret_cast<CodeDefinition*>(bucket->getValue().asObject());
	} else {
		codeDef = engine.getHeap().manage(new CodeDefinition());
		Compiler compiler(engine.getHeap(), codeDef, Compiler::FOR_EVAL);
		engine.getHeap().protect(&compiler);
		compiler.compile(*sourceCode);
		functionDefCache.assign(sourceCode, Value(codeDef));
	}
	processor.enterEval(codeDef, direct);
	return Value();
}

/* FIX : funny C++11 thing
#include <functional>


typedef std::function<Value (Engine&, Processor&, Int32, const Value*, Object*)> NativeFunction;

		mathObject->setProperty(*this, &SQRT_STRING, newNativeFunction(gcHeap, [](Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
			return (argc >= 1 ? Value(sqrt(argv[0].toDouble())) : Value::NOT_A_NUMBER);
		}));
		
*/

typedef Value (*NativeFunction)(Engine&, Processor&, Int32, const Value*, Object*);

template<class F> struct FunctorAdapter : public Function {
	FunctorAdapter(const F& f) : f(f) { }
	virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
		return f(engine, processor, argc, argv, thisObject);
	}
	F f;
};

// FIX : only toDouble?
template<class F> struct UnaryNativeFunction : public Function {
	UnaryNativeFunction(const F& f) : f(f) { }
	virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
		return (argc >= 1 ? Value(f(argv[0].toDouble())) : Value::NOT_A_NUMBER);
	}
	const F f;
};

template<class F> Value newNativeFunction(GCHeap& heap, const F& f) { return Value(heap.manage(new FunctorAdapter<F>(f))); }

struct Support {
	static Value getInternalProperty(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
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

	static Value createWrapper(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 2) {
			const String* const className = argv[0].toString(engine.getHeap());
			const Value internalValue = argv[1];
			Object* prototype = (argc >= 3 ? argv[2].toObject(engine, false) : 0);
			if (prototype == 0) {
				prototype = &engine.objectPrototype;
			}
			Object* o = 0;
			if (className->isEqualTo(S_TRING_STRING)) {
				o = new StringWrapper(internalValue.toString(engine.getHeap()), prototype);
			} else if (className->isEqualTo(E_RROR_STRING)) {
				for (int i = 0; i < ERROR_TYPE_COUNT; ++i) {
					if (prototype == engine.errorPrototypes[i]) {
						o = new Error(static_cast<ErrorType>(i));
						break;
					}
				}
			}
			if (o == 0) {
				o = new WrapperObject(className, internalValue, prototype);
			}
			return Value(engine.getHeap().manage(o));
		}
		return Value::UNDEFINED;
	}

	static Value specialConstructor(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 1) {
			Object* regularFunction = argv[0].toObject(engine, false);
			if (regularFunction != 0) {
				Object* constructorFunction = (argc >= 2 ? argv[1].toObject(engine, false) : 0);
				return Value(engine.getHeap().manage(new SpecialConstructorFunction(regularFunction, constructorFunction)));
			}
		}
		return Value::UNDEFINED;
	}

	static Value defineProperty(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
		bool success = false;
		if (argc >= 2) {
			Object* o = argv[0].asObject();
			if (o != 0) {
				Attributes attributes;
				attributes.readOnly = (argc >= 4 && argv[3].toBool());
				attributes.dontEnum = (argc >= 5 && argv[4].toBool());
				attributes.dontDelete = (argc >= 6 && argv[5].toBool());
				success = o->defineProperty(engine, argv[1], (argc >= 3 ? argv[2] : Value::UNDEFINED), attributes);
			}
		}
		return Value(success);
	}

	static Value compileFunction(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
		if (argc >= 1) {
			GCHeap& heap = engine.getHeap();
			const String* source = argv[0].toString(heap);
			CodeDefinition* codeDef = heap.manage(new CodeDefinition());
			Compiler compiler(heap, codeDef, Compiler::FOR_FUNCTION);
			heap.protect(&compiler);
			compiler.compileFunction(source->begin(), source->end(), argc >= 2 ? argv[1].toString(heap) : &ANONYMOUS_STRING);
			return Value(heap.manage(new UserFunction(codeDef, engine.getGlobals())));
		}
		return Value::UNDEFINED;
	}

	/* FIX : drop
	Value callFunction(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
		Object* newThis = (argc > 0 ? argv[0].toObject(engine, false) : 0);
		return thisObject->call(engine, processor, std::max(argc - 1, 0), argv + std::min(1, argc)
				, (newThis == 0 ? engine.getGlobals() : newThis));
	}
	*/

	static Value callWithArgs(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
		Object* callObject = (argc > 0 ? argv[0].toObject(engine, false) : 0);
		if (callObject == 0) {
			processor.error(TYPE_ERROR); // FIX : ?
			return Value();
		} else {
			Object* newThis = (argc > 1 ? argv[1].toObject(engine, false) : 0);
			QuickArray<Value> args;
			Object* arrayObject = (argc > 2 ? argv[2].toObject(engine, false) : 0);
			if (arrayObject != 0) {
				Value v;
				if (arrayObject->getProperty(engine, Value(&LENGTH_STRING), &v)) { // FIX : in the future I think we should have a getLength
					Int32 length = v.toInt();
					Int32 offset = (argc > 3 ? argv[3].toInt() : 0);
					args.resize(length);
					for (Int32 i = 0; i < length; ++i) {
						args[i] = Value::UNDEFINED;
						arrayObject->getProperty(engine, Value(i + offset), &args[i]);
					}
				}
			}
			// FIX : we copy all arguments once to args, and then chain will copy them again to a frame object, couldn't we short-cut that somehow?
			// FIX : since only arrays and arguments are really valid here, perhaps even have a new virtual in object for implementing efficient apply with these?
			return callObject->call(engine, processor, args.size(), args.begin(), (newThis == 0 ? engine.getGlobals() : newThis));
		}
	}
	
	static Value hasOwnProperty(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
		Object* object = (argc >= 2 ? argv[0].toObject(engine, false) : 0);
		return (object != 0 ? Value(object->hasProperty(engine, argv[1])) : Value(false));
	}

	static Value isPropertyEnumerable(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
		Object* object = (argc >= 2 ? argv[0].toObject(engine, false) : 0);
		return (object != 0 ? Value(object->isPropertyEnumerable(engine, argv[1])) : Value(false));
	}

	/*
	Value fromCharCode(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
		Char c;
		if (argc == 1 && (c = argv[0].toInt()) < 127) return QUICK_CONSTANTS.ascii[c];
		String* s = engine.getHeap().manage(new String(argc));
		for (Int32 i = 0; i < argc; ++i) (*s)[i] = static_cast<Char>(argv[i].toInt());
		return Value(s);
	}*/
	// UnaryNativeFunction< std::function<double (double)> > NATIVE_SQRT_FUNCTION([](double d) { return sqrt(d); });

	static Value fromCharCode(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
		Char c = static_cast<Char>(argc > 0 ? argv[0].toInt() : 0);
		return ((0 <= c && c < 127) ? QUICK_CONSTANTS.ascii[c] : Value(engine.getHeap().manage(new String(&c, &c + 1))));
	}
};

static struct {
	String name;
	UnaryNativeFunction<double (*)(double)> func;
} NATIVE_MATH_FUNCTIONS[] = {
	{ String("acos"), acos }, { String("asin"), asin }, { String("atan"), atan }, { String("cos"), cos },
	{ String("exp"), exp }, { String("floor"), floor }, { String("log"), log }, { String("sin"), sin },
	{ String("sqrt"), sqrt }, { String("tan"), tan }
};
// TODO : atan2, pow, min, max, random

static struct {
	String name;
	FunctorAdapter<Value (*)(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) > func;
} SUPPORT_FUNCTIONS[] = {
	{ String("getInternalProperty"), Support::getInternalProperty }, { String("createWrapper"), Support::createWrapper },
	{ String("defineProperty"), Support::defineProperty }, { String("compileFunction"), Support::compileFunction },
	{ String("specialConstructor"), Support::specialConstructor }, { String("callWithArgs"), Support::callWithArgs },
	{ String("hasOwnProperty"), Support::hasOwnProperty }, { String("fromCharCode"), Support::fromCharCode },
	{ String("isPropertyEnumerable"), Support::isPropertyEnumerable }
};

static UnaryNativeFunction<bool (*)(double)> IS_NAN_FUNCTION(isNaN);
static UnaryNativeFunction<bool (*)(double)> IS_FINITE_FUNCTION(isFinite);

Engine::Engine(GCHeap& heap, Frame* globals) : gcHeap(heap), globals(globals), evalFunction(false)
		, evalDirectFunction(true), toNumberFunction(0), toStringFunction(0)
		, booleanPrototype(&B_OOLEAN_STRING, Value(false), &objectPrototype)
		, numberPrototype(&N_UMBER_STRING, Value(0.0), &objectPrototype)
		, stringPrototype(&S_TRING_STRING, Value(&EMPTY_STRING), &objectPrototype)
		, arrayPrototype(&objectPrototype) {
	assert(heap.belongs(globals));

	std::fill(errorPrototypes, errorPrototypes + ERROR_TYPE_COUNT, (Error*)(0));
	
	gcHeap.protect(&objectPrototype);
	gcHeap.protect(&functionPrototype);
	gcHeap.protect(&booleanPrototype);
	gcHeap.protect(&numberPrototype);
	gcHeap.protect(&stringPrototype);
	gcHeap.protect(&arrayPrototype);
	
	for (int i = 0; i < ERROR_TYPE_COUNT; ++i) {
		errorPrototypes[i] = gcHeap.protect(new ErrorPrototype(static_cast<ErrorType>(i)));
	}
	
	ObjectConstructor* objectObject = gcHeap.manage(new ObjectConstructor());
	ArrayConstructor* arrayObject = gcHeap.manage(new ArrayConstructor());
//	FunctionConstructor* functionObject = gcHeap.manage(new FunctionConstructor());
	objectObject->setProperty(*this, Value(&PROTOTYPE_STRING), Value(&objectPrototype));
	arrayObject->setProperty(*this, Value(&PROTOTYPE_STRING), Value(&arrayPrototype));
//	functionObject->setProperty(*this, &PROTOTYPE_STRING, Value(&functionPrototype));
	globals->setProperty(*this, Value(&A_RRAY_STRING), Value(arrayObject));
//	globals->setProperty(*this, &F_UNCTION_STRING, Value(functionObject));
	globals->setProperty(*this, Value(&EVAL_STRING), Value(gcHeap.protect(&evalFunction)));
	gcHeap.protect(&evalDirectFunction);
}

void Engine::setupStandardLibrary(const String& librarySourceCode) {
	UserObject* supportObject = gcHeap.manage(new UserObject(&objectPrototype));
	// FIX : no new String?
	globals->setProperty(*this, Value(gcHeap.manage(new String("support"))), Value(supportObject));
	UserObject* prototypesObject = gcHeap.manage(new UserObject(&objectPrototype));
	supportObject->setProperty(*this, Value(gcHeap.manage(new String("prototypes"))), Value(prototypesObject));
	for (int i = 0; i < sizeof (SUPPORT_FUNCTIONS) / sizeof (*SUPPORT_FUNCTIONS); ++i) {
		supportObject->setProperty(*this, Value(&SUPPORT_FUNCTIONS[i].name), Value(&SUPPORT_FUNCTIONS[i].func));
	}
	supportObject->setProperty(*this, Value(gcHeap.manage(new String("isNaN"))), Value(&IS_NAN_FUNCTION));
	supportObject->setProperty(*this, Value(gcHeap.manage(new String("isFinite"))), Value(&IS_FINITE_FUNCTION));
	supportObject->setProperty(*this, Value(&UNDEFINED_STRING), Value::UNDEFINED);
	supportObject->setProperty(*this, Value(&NAN_STRING), Value::NOT_A_NUMBER);
	supportObject->setProperty(*this, Value(&INFINITY_STRING), Value::INFINITE_NUMBER);
	supportObject->setProperty(*this, Value(gcHeap.manage(new String("maxNumber"))), Value(std::numeric_limits<double>::max()));
	supportObject->setProperty(*this, Value(gcHeap.manage(new String("minNumber"))), Value(std::numeric_limits<double>::denorm_min()));
	
	for (int i = 0; i < sizeof (NATIVE_MATH_FUNCTIONS) / sizeof (*NATIVE_MATH_FUNCTIONS); ++i) {
		supportObject->setProperty(*this, Value(&NATIVE_MATH_FUNCTIONS[i].name), Value(&NATIVE_MATH_FUNCTIONS[i].func));
	}
	
	struct { const String* name; Object* object; } PROTOTYPES[] = {
		{ &O_BJECT_STRING, &objectPrototype }, { &F_UNCTION_STRING, &functionPrototype },
		{ &S_TRING_STRING, &stringPrototype }, { &B_OOLEAN_STRING, &booleanPrototype },
		{ &N_UMBER_STRING, &numberPrototype }, { &A_RRAY_STRING, &arrayPrototype }
	};
	for (int i = 0; i < sizeof (PROTOTYPES) / sizeof (*PROTOTYPES); ++i) {
		prototypesObject->setProperty(*this, Value(PROTOTYPES[i].name), Value(PROTOTYPES[i].object));
	}
	for (int i = 0; i < ERROR_TYPE_COUNT; ++i) {
		prototypesObject->setProperty(*this, Value(&ERROR_NAMES[i]), Value(errorPrototypes[i]));
	}
	// FIX : all this boilerplating... build a function object or something instead?

	CodeDefinition* code = gcHeap.manage(new CodeDefinition());
	gcHeap.protect(code); // FIX : necessary? can we unprotect?
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
		UserFunction* localFunction = gcHeap.manage(new UserFunction(code, globals));
		FunctionFrame* localFrame = gcHeap.manage(new FunctionFrame(localFunction, 0, 0));
		processor.chain(code->getCodePointer(), localFrame, globals);
		bool needsMoreTime = processor.run();
		assert(!needsMoreTime);
	}
	gcHeap.gc();

	// FIX: UserFunction cast sucks! call() instead of chain() in Processor so that we can have arbitrary object
	toNumberFunction = reinterpret_cast<UserFunction*>(supportObject->getPropertyDeep(*this, "toPrimitiveNumber").asObject());
	if (toNumberFunction == 0) {
	// FIX : all these gcHeap.manage(new String()) ... could be a utility function!
		throw Exception(gcHeap, Value(gcHeap.manage(new Error(GENERIC_ERROR, gcHeap.manage(new String("Standard library is missing support.toPrimitiveNumber"))))));
	}
		
	toStringFunction = reinterpret_cast<UserFunction*>(supportObject->getPropertyDeep(*this, "toPrimitiveString").asObject());
	if (toStringFunction == 0) {
		throw Exception(gcHeap, Value(gcHeap.manage(new Error(GENERIC_ERROR, gcHeap.manage(new String("Standard library is missing support.toPrimitiveString"))))));
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

	class PrintFunction : public Function {
		public:		typedef Function super;
					virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
						const String* s = (argc >= 1 ? argv[0].toString(engine.getHeap()) : &EMPTY_STRING);
						std::wcout << s->toStdString().c_str() << std::endl;
						if (interactive) {
							pushIOLines('<', *s);
						}
						return Value::UNDEFINED;
					}
	};
	
	struct GCFunction : public Function {
		virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) {
			UInt32 sweptCount = engine.getHeap().gc();
			return Value(static_cast<Int32>(sweptCount));
		}
	};

int main(int argc, const char* argv[]) {
supressStdErr = true;
	const String LF_STRING("\n");
	const String PRINT_STRING("print");
/*std::cerr << "Value size: " << sizeof (Value) << std::endl;
std::cerr << "Bucket size: " << sizeof (Table::Bucket) << std::endl;
std::cerr << "Table size: " << sizeof (Table) << std::endl;*/
	GCHeap heap;
	UserObject globals;
	heap.protect(&globals);
	CodeDefinition globalCode;
	heap.protect(&globalCode);
	UserFunction globalClosure(&globalCode, &globals); // FIX : yuck!
	heap.protect(&globalClosure);
	FunctionFrame globalsFrame(&globalClosure, 0, 0);
	heap.protect(&globalsFrame);
	
	Engine engine(heap, &globalsFrame);
	
	try {
		std::wifstream stdlibStream("stdlib.js");
		String source = String(std::wstring(std::istreambuf_iterator<wchar_t>(stdlibStream), std::istreambuf_iterator<wchar_t>()));
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
	
	PrintFunction printFunction;
	globals.setProperty(engine, Value(&PRINT_STRING), Value(heap.protect(&printFunction)));
	GCFunction gcFunction;
	const String GC_STRING("gc");
	globals.setProperty(engine, Value(&GC_STRING), Value(heap.protect(&gcFunction)));
	String source(EMPTY_STRING);
	std::auto_ptr<std::wifstream> inputFileStream;
	std::wistream* inStream = &std::wcin;
	bool doTime = false;
	long gcRate = 20000000;
	bool pauseBeforeQuit = false;
	for (int argi = 1; argi < argc; ++argi) {
		if (strcmp(argv[argi], "-t") == 0) doTime = true;
		else if (strcmp(argv[argi], "-s") == 0) supressStdErr = true;
		else if (argi + 1 < argc && strcmp(argv[argi], "-gc") == 0) gcRate = atol(argv[++argi]);
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
		try {
			bool execute = false;
			std::wstring s;
			if (interactive) {
				std::getline(*inStream, s);
				// FIX : add #undo that drops just the last one
				if (s == L"#save") { // FIX : add optional file name, can also be used to update results from testio.pika
					const time_t t = time(0);
					char fn[257];
					strftime(fn, 256, "tests/%Y%m%d_%H%M%S.io", localtime(&t));
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
					String line(s);
					if (!source.empty()) source = String(source, LF_STRING);
					source = String(source, line);
				} else {
					if (source.size() > 0 && source[0] == '?') {
						source = String(String(String("print("), String(source.begin() + 1, source.end())), String(")")); // FIX : operator+
					}
					if (interactive) {
						pushIOLines('>', source);
					}
					execute = true;
				}
			} else {
				source = String(std::wstring(std::istreambuf_iterator<wchar_t>(*inStream), std::istreambuf_iterator<wchar_t>()));
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

				CodeDefinition codeDef;
				heap.protect(&codeDef);
				Compiler compiler(heap, &globalCode, (interactive ? Compiler::FOR_EVAL : Compiler::FOR_GLOBAL));
				heap.protect(&compiler);
				try {
					compiler.compile(source);
				}
				catch (const Exception& x) {
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
						pushIOLines('!', String(ws));
					}
					throw;
				}
				globalsFrame.updatedCode();
				processor.chain(globalCode.getCodePointer(), &globalsFrame, &globalsFrame);
				
				bool done = false;
				clock_t start = ::clock();	
				do {
					processor.resetTimeOut(gcRate);
					done = !processor.run();
				
					UInt32 sweptCount = heap.gc();
					if (!supressStdErr) {
						std::cerr << "\tGC swept: " << sweptCount << std::endl;
						std::cerr << "\tGC kept: " << heap.size() << std::endl;
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
				pushIOLines('!', String(ws));
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
	}
	if (pauseBeforeQuit) std::wcin.get();
	return 0;
}
