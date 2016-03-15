#include "assert.h"
#include <cmath>
#include <ctime>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <limits>
#include <map>
#include "NuXJScript.h"

	bool supressStdErr = false; // FIX : drop

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

static double NaN() { return std::numeric_limits<double>::quiet_NaN(); }
static bool isNaN(double d) { return d != d; }

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

/* --- String constants --- */

// FIX : sort, merge to few lines, or maybe even do some smart array based construction?
const String EMPTY_STRING("");
const String UNDEFINED_STRING("undefined");
const String OBJECT_STRING("object");
const String O_BJECT_STRING("Object");
const String A_RRAY_STRING("Array");
const String BOOLEAN_STRING("boolean");
const String NUMBER_STRING("number");
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
const String CONSTRUCTOR_STRING("constructor");
const String VALUE_OF_STRING("valueOf");
const String TO_STRING_STRING("toString");
const String EVAL_STRING("eval");

/* --- Value --- */

static struct BadFunction : public Function { // FIX : .length?! and all the others?!
	virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) {
		processor.error(); // FIX : error string, type should be TypeError
		return Value();
	}
} BAD_FUNCTION;

const String* Value::TYPE_STRINGS[7] = { 0, &UNDEFINED_STRING, &OBJECT_STRING, &BOOLEAN_STRING, &NUMBER_STRING, &STRING_STRING, 0 };

const String* Value::typeOfString() const { return (type == OBJECT_TYPE ? var.object->typeOfString() : TYPE_STRINGS[static_cast<int>(type)]); }
Function* Value::getFunction() const { return (type != OBJECT_TYPE ? &BAD_FUNCTION : var.object->getFunction()); }

bool Value::toBool() const {
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE:
		case NULL_TYPE: return false;
		case BOOLEAN_TYPE: return var.boolean;
		case NUMBER_TYPE: return (var.number != 0.0);
		case STRING_TYPE: return (!var.string->empty());
		case OBJECT_TYPE: return true;
	}
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
	if (i == v) {
		return i;
	} else if (isNaN(v) || fabs(v) == std::numeric_limits<double>::infinity()) {
		return 0; // FIX : faster test for weird numbers?
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
case OBJECT_TYPE: return heap.manage(new String("<object>"));
	}
}

Object* Value::toObject(GCHeap& heap) const {
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return 0;
		case NULL_TYPE: return 0;
		case BOOLEAN_TYPE: /* FIX */ return 0;
		case NUMBER_TYPE: /* FIX */ return 0;
		case STRING_TYPE: return const_cast<String*>(var.string); // I think const_cast can be accepted here, the string is always immutable when accessed as an Object
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
	boolean		FALSE		FALSE	l === r				num(l) === r	num(l) === num(r)	FALSE
	number		FALSE		FALSE	l === num(r)		l === r			l === num(r)		l == prim(r)
	string		FALSE		FALSE	num(l) === num(r)	num(l) === r	l === r				l == prim(r)
	object		FALSE		FALSE	FALSE				prim(l) == r	prim(l) == r		l === r
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

void Value::markIn(GCHeap& heap) const {
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
				Char buf[1] = { i };
				ascii[i] = Value(new String(buf, buf + 1));
			}
			for (int i = -QUICK_CONSTANTS_INTEGERS_RANGE; i <= QUICK_CONSTANTS_INTEGERS_RANGE; ++i) {
				Char buffer[32];
				integers[i + QUICK_CONSTANTS_INTEGERS_RANGE] = new String(intToString(buffer, i), buffer + 32);
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
String::String(const String& o) : QArray(o), hash(o.hash) { }
String::String(const std::wstring& s) : QArray(s.size()), hash(0) { std::copy(s.begin(), s.end(), begin()); }

String::String(const String& l, const String& r) : QArray(l.size() + r.size()), hash(0) {
	Char* e = std::copy(l.begin(), l.end(), begin());
	std::copy(r.begin(), r.end(), e);
}

const String* String::typeOfString() const { return &STRING_STRING; }
const String* String::className() const { return &S_TRING_STRING; }

bool String::getProperty(Engine& engine, const String* name, Value* v) {
	if (name->isEqualTo(LENGTH_STRING)) {
		*v = Value(static_cast<Int32>(size()));
		return true;
	}
	UInt32 i;
	const Char* q = parseUnsignedInt(name->begin(), name->end(), i);
	if (q == name->end() && i < size()) {
		const Char* p = begin() + i;
		if (*p < 127) *v = QUICK_CONSTANTS.ascii[*p];
		else *v = Value(engine.getHeap().manage(new String(p, p + 1)));
		return true;
	} else {
		return engine.stringPrototype.getProperty(engine, name, v);
	}
}

// FIX : speed up by skipping when string is really long
// FNV Hash
UInt32 String::calcHash() const {
	if (hash == 0) {
		UInt32 l = size();
		UInt32 x = 2166136261U;
		const Char* p = begin();
		for (UInt32 i = 0; i < l; ++i) {
			x *= 16777619;
			x += p[i]; // FIX : FNV uses xor, not add, why did I do this? Also FNV is really made for byte processing. Also, http://www.isthe.com/chongo/tech/comp/fnv/ recommends xor'ing before multiplying. Better? 
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

Enumerator* String::getEnumerator(Engine& engine) { return engine.getHeap().manage(new RangeEnumerator(engine.getHeap(), 0, size())); }

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
	if (x == std::numeric_limits<double>::infinity()) return d < 0 ? &MINUS_INFINITY_STRING : &INFINITY_STRING;
	Char buffer[32];
	std::pair<Char*, Char*> r = doubleToString(buffer, d);
	return heap.manage(new String(r.first, r.second));
}

/* --- GCItem --- */

GCItem::GCItem() throw() : _gcList(0), _gcPrev(this), _gcNext(this) { }
GCItem::GCItem(const GCItem& copy) throw() : _gcList(0), _gcPrev(0), _gcNext(0) { }
GCItem& GCItem::operator=(const GCItem& copy) throw() { _gcList = 0; _gcPrev = 0; _gcNext = 0; return *this; }
void GCItem::gcMark(GCHeap& heap) { }
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

Function* Object::asFunction() { return 0; }
Function* Object::getFunction() { return &BAD_FUNCTION; }
const String* Object::typeOfString() const { return &OBJECT_STRING; }
const String* Object::className() const { return &O_BJECT_STRING; }
bool Object::hasProperty(Engine& engine, const String* name) { Value v; return getProperty(engine, name, &v); }
bool Object::getProperty(Engine& engine, const String* name, Value* v) { return false; }
bool Object::setProperty(Engine& engine, const String* name, const Value& v) { return false; }
bool Object::deleteProperty(Engine& engine, const String* name) { return false; }
Enumerator* Object::getEnumerator(Engine& engine) { return &EMPTY_ENUMERATOR; }

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

void Table::markIn(GCHeap& heap) {
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
		UInt32 check = rebuild((loadCount < calcMaxLoad(1U << (n - 1))) ? n - 1 : n);
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

bool UserObject::setProperty(Engine& engine, const String* name, const Value& v) {
	properties.assign(name, v);
	return true;
}

bool UserObject::getProperty(Engine& engine, const String* name, Value* v) {
	const Table::Bucket* bucket = properties.lookup(name);
	if (bucket != 0) {
		*v = bucket->getValue();
		return true;
	} else {
		return (prototype != 0 ? prototype->getProperty(engine, name, v) : false);
	}
}

bool UserObject::deleteProperty(Engine& engine, const String* name) {
	return properties.erase(name);
}

Enumerator* UserObject::getEnumerator(Engine& engine) {
	StringListEnumerator* list = engine.getHeap().manage(new StringListEnumerator(properties.getLoadCount()));
	for (const Table::Bucket* b = properties.getFirst(); b != 0; b = properties.getNext(b)) {
		if (b->enumerate()) {
			list->add(b->getKey());
		}
	}
	if (prototype != 0) {
		Enumerator* protoEnum = prototype->getEnumerator(engine);
		const String* s;
		while ((s = protoEnum->nextPropertyName()) != 0) {
			if (properties.lookup(s) == 0) {
				list->add(s);
			}
		}
	}
	return list;
}

/* --- RangeEnumerator --- */

RangeEnumerator::RangeEnumerator(GCHeap& heap, Int32 from, Int32 count) : heap(heap), index(from), to(from + count) { }
const String* RangeEnumerator::nextPropertyName() { return (index >= to ? 0 : String::fromInt(heap, index++)); }

/* --- StringListEnumerator --- */

StringListEnumerator::StringListEnumerator(UInt32 initialCapacity) : list(initialCapacity), nextIndex(0) { }
void StringListEnumerator::add(const String* s) { list.append(s); }
const String* StringListEnumerator::nextPropertyName() { return (nextIndex < list.size() ? list[nextIndex++] : 0); }

/* --- CodeDef --- */

CodeDefinition::CodeDefinition() : source(0), selfName(0) { }

bool CodeDefinition::lookupNameIndex(const String* name, Int32& index) const {
	const Table::Bucket* bucket = nameIndexes.lookup(name);
	if (bucket == 0) return false;
	index = bucket->getValue().toInt();
	return true;
}

/* --- Function --- */

Function* Function::asFunction() { return this; }
Function* Function::getFunction() { return this; }
const String* Function::typeOfString() const { return &FUNCTION_STRING; }
const String* Function::className() const { return &F_UNCTION_STRING; }
Value Function::construct(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) {
	return call(engine, processor, argc, argv, thisValue);
}

/* --- Object support --- */

class ObjectConstructor : public Function {
// FIX : if we allow a object prototype to change after construction we could override construct here as well and just change the prototype of thisValue, this would save an extra new UserObject when running "new Object()"
	public:		virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) {
					return Value(engine.getHeap().manage(new UserObject(&engine.objectPrototype)));
				}
};

static ObjectConstructor OBJECT_OBJECT; // for object literals

Array::Array(Object* prototype) : super(prototype), lengthProperty(0) { }

bool Array::getProperty(Engine& engine, const String* name, Value* v) {
	if (name->isEqualTo(LENGTH_STRING)) {
		*v = Value(lengthProperty);
		return true;
	} else {
		return super::getProperty(engine, name, v);
	}
}

bool Array::setProperty(Engine& engine, const String* name, const Value& v) {
	if (name->isEqualTo(LENGTH_STRING)) {
		// FIX : error if not int!
		UInt32 newLength = v.toInt(); // fix : to unsigned int
		if (newLength < lengthProperty) { // FIX : optimize: if onyl removing a few items, loop through them and remove instead of looping through all
			for (const Table::Bucket* b = properties.getFirst(); b != 0; b = properties.getNext(b)) {
				UInt32 i;
				const String* key = b->getKey();
				const Char* const e = key->end();
				const Char* p = parseUnsignedInt(key->begin(), e, i);
				if (p == e && i >= newLength) {
					properties.erase(key);
				}
			}
		}
		lengthProperty = newLength;
		return true;
	} else {
		UInt32 i;
		const Char* const e = name->end();
		const Char* p = parseUnsignedInt(name->begin(), e, i);
		if (p == e && i >= lengthProperty) {
			lengthProperty = i + 1;
		}
		return super::setProperty(engine, name, v);
	}
}

bool Array::deleteProperty(Engine& engine, const String* name) {
	if (name->isEqualTo(LENGTH_STRING)) {
		return false;
	} else {
		return super::deleteProperty(engine, name);
	}
}

class ArrayConstructor : public Function {
// FIX : if we allow a object prototype to change after construction we could override construct here as well and just change the prototype of thisValue, this would save an extra new UserObject when running "new Object()"
	public:		virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) {
					Array* array = engine.getHeap().manage(new Array(&engine.arrayPrototype));
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
	public:		virtual bool getProperty(Engine& engine, const String* name, Value* v) {
					if (name->isEqualTo(PROTOTYPE_STRING)) {
						*v = Value(&engine.arrayPrototype);
						return true;
					} else {
						return false;
					}
				}
};

static ArrayConstructor ARRAY_OBJECT; // for array literals

#if 0
class FunctionConstructor : public Function {
// FIX : if we allow a object prototype to change after construction we could override construct here as well and just change the prototype of thisValue, this would save an extra new UserObject when running "new Object()"
	public:		virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) {
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

/* --- UserFunction --- */

UserFunction::UserFunction(CodeDefinition* codeDef, Object* environment) : codeDef(codeDef), environment(environment), userObject(0) { }
CodeDefinition* UserFunction::getDefinition() const { return codeDef; }
Object* UserFunction::getEnvironment() const { return environment; }

UserObject* UserFunction::createUserObject(Engine& engine) {
	if (userObject == 0) {
		userObject = engine.getHeap().manage(new UserObject(&engine.objectPrototype)); // FIX : functionPrototype
		userObject->defineProperty(&LENGTH_STRING, Value(codeDef->getArgumentsCount()), Table::READ_ONLY | Table::DONT_ENUM | Table::DONT_DELETE);
		UserObject* prototype = engine.getHeap().manage(new UserObject(&engine.objectPrototype));
		prototype->defineProperty(&CONSTRUCTOR_STRING, Value(this), Table::DONT_ENUM);
		userObject->defineProperty(&PROTOTYPE_STRING, Value(prototype), Table::DONT_ENUM | Table::DONT_DELETE);
	}
	return userObject;
}

Value UserFunction::call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) {
	processor.chain(this, argc, argv, thisValue);
	--processor.vsp; // That is right, we pre-compensate for the fact that in comparison to native calls we will end up pushing vsp once for the return value
	return Value();
}

/* --- Arguments --- */

Arguments::Arguments(FunctionFrame* frame, UInt32 argumentsCount) : frame(frame)
		, argumentsCount(argumentsCount), lengthProperty(argumentsCount), deletedArguments(argumentsCount + 1) {
	std::fill(deletedArguments.begin(), deletedArguments.end(), false);
}

const String* Arguments::className() const { return &A_RGUMENTS_STRING; }

Value* Arguments::findProperty(const String* name) {
	if (name->isEqualTo(LENGTH_STRING)) {
		return (deletedArguments[0] ? 0 : &lengthProperty);
	}
	UInt32 i;
	const Char* q = parseUnsignedInt(name->begin(), name->end(), i);
	return (q == name->end() && i < argumentsCount && !deletedArguments[i + 1] ? frame->getIndexedPointer() + i : 0);
}

bool Arguments::getProperty(Engine& engine, const String* name, Value* v) {
	Value* p = findProperty(name);
	return (p == 0 ? super::getProperty(engine, name, v) : (*v = *p, true));
}

bool Arguments::setProperty(Engine& engine, const String* name, const Value& v) {
	Value* p = findProperty(name);
	return (p == 0 ? super::setProperty(engine, name, v) : (*p = v, true));
}

bool Arguments::deleteProperty(Engine& engine, const String* name) {
	Value* p = findProperty(name);
	if (p == 0) return super::deleteProperty(engine, name);
	else if (p == &lengthProperty) return (deletedArguments[0] = true);
	else return (deletedArguments[p - frame->getIndexedPointer() + 1] = true);
}

Enumerator* Arguments::getEnumerator(Engine& engine) {
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

bool FunctionFrame::getProperty(Engine& engine, const String* name, Value* v) {
	Int32 index;
	if (codeDef->lookupNameIndex(name, index)) {
		assert(indexed.begin() <= indexedPointer + index && indexedPointer + index < indexed.end());
		*v = indexedPointer[index];
		return true;
	} else if (name->isEqualTo(ARGUMENTS_STRING)) {
		if (!argumentsValue.exists()) {
			Object* o = engine.getHeap().manage(new Arguments(this, passedArgumentsCount));
			argumentsValue = Value(o);
		}
		*v = argumentsValue;
		return true;
	} else if (codeDef->selfName != 0 && name->isEqualTo(*codeDef->selfName)) {
		*v = Value(selfFunction);
		return true;
	} else {
		return environment->getProperty(engine, name, v); // FIX: can we do a looping non-recursive solution? faster
	}
}

bool FunctionFrame::setProperty(Engine& engine, const String* name, const Value& v) {
	Int32 index;
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
		return environment->setProperty(engine, name, v);
	}
}

bool FunctionFrame::deleteProperty(Engine& engine, const String* name) {
	Int32 index;
	if (codeDef->lookupNameIndex(name, index)) return false;
	else if (codeDef->selfName != 0 && name->isEqualTo(*codeDef->selfName)) return false;
	else return environment->deleteProperty(engine, name);
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
bool EvalFrame::getProperty(Engine& engine, const String* name, Value* v) { return parentFrame->getProperty(engine, name, v); }
bool EvalFrame::setProperty(Engine& engine, const String* name, const Value& v) { return parentFrame->setProperty(engine, name, v); }
bool EvalFrame::deleteProperty(Engine& engine, const String* name) { return parentFrame->deleteProperty(engine, name); }
void EvalFrame::mergeEvalVars(GCHeap& heap, CodeDefinition* evalFunctionDef) { parentFrame->mergeEvalVars(heap, evalFunctionDef); }

/* --- CatchFrame --- */

CatchFrame::CatchFrame(Frame* parentFrame, const String* exceptionName, const Value& exceptionValue)
		: super(parentFrame->getFunctionDef()), parentFrame(parentFrame), exceptionName(exceptionName)
		, exceptionValue(exceptionValue) {
	indexedPointer = parentFrame->getIndexedPointer();
}

bool CatchFrame::getProperty(Engine& engine, const String* name, Value* v) {
	if (name->isEqualTo(*exceptionName)) { *v = exceptionValue; return true; }
	else return parentFrame->getProperty(engine, name, v);
}

bool CatchFrame::setProperty(Engine& engine, const String* name, const Value& v) {
	if (name->isEqualTo(*exceptionName)) { exceptionValue = v; return true; }
	else return parentFrame->setProperty(engine, name, v);
}

bool CatchFrame::deleteProperty(Engine& engine, const String* name) {
	if (name->isEqualTo(*exceptionName)) return false;
	else return parentFrame->deleteProperty(engine, name);
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

void Processor::error() {
	static const String ERROR_STRING("runtime error");
	throw Exception(engine.getHeap(), Value(&ERROR_STRING));
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
	csp->returnIP = ip;
	csp->frame = 0;
	csp->thisValue = 0;
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

void Processor::chain(UserFunction* func, Int32 argc, const Value* argv, Object* thisValue) {
	frame = engine.getHeap().manage(new FunctionFrame(func, argc, argv));
	++csp; // FIX : overflow check
	csp->returnIP = ip;
	csp->frame = frame;
	csp->thisValue = thisValue;
	ip = func->getDefinition()->getCodePointer();
	updateInternals();
}

void Processor::chain(const CodeWord* ip, Frame* frame, Object* thisValue) {
	this->frame = frame;
	++csp; // FIX : overflow check
	csp->returnIP = this->ip;
	csp->frame = this->frame;
	csp->thisValue = thisValue;
	this->ip = ip;
	updateInternals();
}

void Processor::enterEval(CodeDefinition* codeDef, bool directCall) {
--vsp;
	Frame* f = (directCall ? frame : engine.getGlobals());
	f->mergeEvalVars(engine.getHeap(), codeDef);
	frame = engine.getHeap().manage(new EvalFrame(codeDef, f));
	++csp; // FIX : overflow check
	csp->returnIP = ip;
	csp->frame = frame;
	csp->thisValue = csp[-1].thisValue;
	ip = codeDef->getCodePointer();
	updateInternals();
}

void Processor::throwVirtualException(const Value& exception) {
	if (catchers.empty()) { // FIX
		std::string s1("Unhandled exception: ");
		std::wstring s2 = exception.toString(engine.getHeap())->toStdString();
		s1.append(s2.begin(), s2.end());
		throw std::runtime_error(s1);
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
			case READ_NAMED_OP:		if (!frame->getProperty(engine, constants[operand()].getString(), ++vsp))
										error(); // FIX : do we need to balance vsp on errors?
									break;
									
			case WRITE_NAMED_OP:	frame->setProperty(engine, constants[operand()].getString(), vsp[0]);
									break;
									
			case GET_PROPERTY_OP:	if ((o = vsp[-1].toObject(heap)) == 0) error();
									if (!o->getProperty(engine, vsp[0].getString(), vsp - 1))
										vsp[-1] = Value::UNDEFINED; // FIX : is this right?
									pop(1);
									break;
			
			case SET_PROPERTY_OP:	if ((o = vsp[-2].toObject(heap)) == 0) error();
									o->setProperty(engine, vsp[-1].getString(), vsp[0]);
									vsp[-2] = vsp[0];
									pop(2);
									break;

			case TO_PRIMITIVE_OP:
				if (vsp[0].isObject()) {
					engine.toPrimitiveValueFunction->call(engine, *this, 1, vsp, engine.getGlobals());
				}
				break;

			case TO_STRING_OP:
				if (!vsp[0].isString()) {
					if (vsp[0].isObject()) {
						engine.toStringFunction->call(engine, *this, 1, vsp, engine.getGlobals());
					} else {
						vsp[0] = Value(vsp[0].toString(heap));
					}
				}
				break;
				
			case PRE_EQ_OP:
				if (vsp[-1].isObject() != vsp[0].isObject()) {
					if (vsp[-1].isObject()) {
						std::swap(vsp[-1], vsp[0]);
					}
					if (vsp[-1].isString() || vsp[-1].isNumber()) {
						engine.toPrimitiveValueFunction->call(engine, *this, 1, vsp, engine.getGlobals());
					}
				}
				break;

			case INC_OP:	vsp[0] = Value(vsp[0].toDouble() + 1.0); break;
			case DEC_OP:	vsp[0] = Value(vsp[0].toDouble() - 1.0); break;
			case ADD_OP:	poppush(vsp[-1].add(heap, vsp[0])); break;
			case SUB_OP:	poppush(Value(vsp[-1].toDouble() - vsp[0].toDouble())); break;
			case MUL_OP:	poppush(Value(vsp[-1].toDouble() * vsp[0].toDouble())); break;
			case DIV_OP:	poppush(Value(vsp[-1].toDouble() / vsp[0].toDouble())); break;
			case MOD_OP:	poppush(Value(fmod(vsp[-1].toDouble(), vsp[0].toDouble()))); break;
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
							csp->returnIP = ip;
							csp->frame = frame;
							csp->thisValue = csp[-1].thisValue;
							ip += offset;
							break;

			case RETURN_OP:	ip = (csp--)->returnIP;
							assert(csp >= cstackStart);							
							updateInternals();
							break;

			case RETURN_TO_HOST_OP:
							ip = (csp--)->returnIP;
							assert(csp >= cstackStart - 1);
							result = vsp[0];
							pop(1);
							// FIX : not necessary now cause we updateInternals at top, should we always? cause updateInterals here will fail if csp == cstackStart - 1
							// updateInternals();
							return false;

			case CALL_METHOD_OP: { // FIX : share more with the others...
				Int32 n = operand();
				Object* thisObject = vsp[-n - 1].toObject(heap);
				if (thisObject == 0) error();
				Value v(Value::NONEXISTENT);
				thisObject->getProperty(engine, vsp[-n].toString(heap), &v);
				Value* returnPointer = vsp - n - 1; // Notice that call() will change vsp if it is a script function, so we save this first.
				*returnPointer = v.getFunction()->call(engine, *this, n, returnPointer + 2, thisObject); // Yeah, this *returnPointer assignment goes to waste if we called a script function (since it returns directly), but so what?
				vsp -= n + 1;
				break;
			}
			case CALL_EVAL_OP: { // FIX : all these are too similar!
				Int32 n = operand();
				Value* returnPointer = vsp - n; // Notice that call() will change vsp if it is a script function, so we save this first.
				Function* f = vsp[-n].getFunction();
				if (f == &engine.evalFunction) f = &engine.evalDirectFunction;
				*returnPointer = f->call(engine, *this, n, returnPointer + 1, engine.getGlobals()); // Yeah, this *returnPointer assignment goes to waste if we called a script function (since it returns directly), but so what?
				vsp -= n;
				break;
			}
			case CALL_OP: {
				Int32 n = operand();
				Value* returnPointer = vsp - n; // Notice that call() will change vsp if it is a script function, so we save this first.
				*returnPointer = vsp[-n].getFunction()->call(engine, *this, n, returnPointer + 1, engine.getGlobals()); // Yeah, this *returnPointer assignment goes to waste if we called a script function (since it returns directly), but so what?
				vsp -= n;
				break;
			}
			case NEW_OP: {
				Int32 n = operand(); // fix : top declare all variables in this switch, I seem to recall from GAZL that it is better
				Function* f = vsp[-n].getFunction();
				Value v(Value::NONEXISTENT);
				f->getProperty(engine, &PROTOTYPE_STRING, &v);
				Object* prototype = v.asObject();
				Object* newObject = heap.manage(new UserObject(prototype != 0 ? prototype : &engine.objectPrototype));
				Value* returnPointer = vsp - n; // Notice that call() will change vsp if it is a script function, so we save this first.
				returnPointer[1] = f->construct(engine, *this, n, returnPointer + 1, newObject);							// Yeah, this *returnPointer assignment goes to waste if we called a script function (since it returns directly), but so what?
				returnPointer[0] = Value(newObject);
				vsp -= n - 1;
				break;
			}
			case NEW_RESULT_OP: poppush(vsp[0].isObject() ? vsp[0] : vsp[-1]); break;
			case THIS_OP: push(Value(csp->thisValue)); break;
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
				o->setProperty(engine, constants[operand()].getString(), vsp[0]);
				pop(1);
				break;
			}
			
			case FRAME_OP: push(Value(frame)); break;
			case DELETE_OP: {
				o = vsp[-1].toObject(heap);
				if (o == 0) error();
				poppush(Value(o->deleteProperty(engine, vsp[0].getString())));
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
				csp->returnIP = 0;
				frame = csp->frame = heap.manage(new CatchFrame(frame, constants[operand()].getString(), vsp[0]));
				pop(1);
				csp->thisValue = csp[-1].thisValue;
				break;
			}
			case THROW_OP: throwVirtualException(vsp[0]); break;
			case IN_OP: {
				o = vsp[0].asObject();
				if (o == 0) error(); // FIX : TypeError
				vsp[-1] = Value(o->hasProperty(engine, vsp[-1].getString()));
				pop(1);
				break;				
			}
			case TYPEOF_OP: vsp[0] = Value(vsp[0].typeOfString()); break;
			case TYPEOF_NAMED_OP: {
				Value v(Value::UNDEFINED);
				frame->getProperty(engine, constants[operand()].getString(), &v);
				push(Value(v.typeOfString()));
				break;
			}
			case GET_ENUMERATOR_OP: {
				o = vsp[0].toObject(heap);
				vsp[0] = Value((o == 0 ? &EMPTY_ENUMERATOR : o->getEnumerator(engine)));
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
		, "IN", "TYPEOF", "TYPEOF_NAMED"
		, "GET_ENUMERATOR", "NEXT_PROPERTY"
	};
	const CodeWord* code = codeDef.getCodePointer();
	const Value* constants = codeDef.getConstantsPointer();
	for (UInt32 offset = 0; offset < codeDef.getCodeSize();) {
		Processor::Opcode opcode = static_cast<Processor::Opcode>(code[offset]);
		if (opcode < 0 || opcode >= sizeof (OP_STRINGS) / sizeof (*OP_STRINGS)) opcode = Processor::INVALID_OP;
		std::cerr << "\t" << offset << ':' << OP_STRINGS[opcode];
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

void Compiler::syntaxError(const String& what) {
	throw Exception(heap, Value(heap.manage(new String(what))));
}

void Compiler::syntaxError(const std::string& what) {
	std::wstring ws(what.begin(), what.end());
	syntaxError(String(ws));
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

void Compiler::white() {
	while (!eof()) {
		switch (*p) {
			case ' ':
			case '\v':
			case '\f':
			case '\t':	++p; break;
			case '\r':	if (p + 1 != e && p[1] == '\n') ++p;
			case '\n':	++p;
						while (!eof() && (*p == ' ' || *p == '\t')) ++p; // Eat leading .. on a new line
						if (e - p >= 2 && p[0] == '.' && p[1] == '.') p += 2;
						break;
			case '/':	if (p + 1 != e && p[1] == '/') {
							static const Char END_CHARS[] = { '\r', '\n' };
							p = std::find_first_of(p += 2, e, END_CHARS, END_CHARS + 2);
							break;
						} else if (p + 1 != e && p[1] == '*') {
							static const Char END_CHARS[] = { '*', '/' };
							p = std::search(p += 2, e, END_CHARS, END_CHARS + 2);
							if (eof()) syntaxError("Missing */");
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
			syntaxError("Illegal use of reserved keyword");
		}
	}
	if (required && p == b) {
		syntaxError("Expected identifier");
	}
	return b;
}

static UInt32 unescapedMaxLength(const Char* p, const Char* e) {
	assert(p <= e && (*p == '"' || *p == '\''));
	Char endChar = *p;
	UInt32 l = 0;
	while (++p < e && *p != endChar && *p != '\r' && *p != '\n') { // FIX : use std::find_any
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
	while (!eof() && *p != endChar && *p != '\r' && *p != '\n') { // FIX : use std::find_any
		if (*p == '\\' && p + 1 < e) {
			d = std::copy(b, p, d);
			const Char* f = std::find(ESCAPE_CHARS, ESCAPE_CHARS + ESCAPE_CODE_COUNT, *++p);
			b = p;
			UInt32 l = 0;
			if (f != ESCAPE_CHARS + ESCAPE_CODE_COUNT) { ++p; l = ESCAPE_CODES[f - ESCAPE_CHARS]; }
			else if (*p == 'x') { b = ++p; p = parseHex(p, (e - p > 2 ? p + 2 : e), l); } // FIX : in ecma, you *need* two (and four) hex chars, nothing else, throw here!
			else if (*p == 'u') { b = ++p; p = parseHex(p, (e - p > 4 ? p + 4 : e), l); }
			if (p == b) syntaxError("Illegal escape character");
			b = p;
			*d++ = Char(l);
		} else ++p;
	}
	if (eof() || *p != endChar) syntaxError("Unterminated string");
	return std::copy(b, p++, d);
}

Compiler::ExpressionResult Compiler::makeRValue(const ExpressionResult& xr, bool toPrimitive) {
	switch (xr.t) {
		case ExpressionResult::PUSHED: if (toPrimitive) emit(Processor::TO_PRIMITIVE_OP); return xr;
		case ExpressionResult::NONE: emit(Processor::VOID_OP); break;
		case ExpressionResult::CONSTANT: emitWithConstant(Processor::CONST_OP, xr.v); break;
		case ExpressionResult::INDEXED: emitWithInt32(Processor::READ_INDEXED_OP, xr.v.toInt()); if (toPrimitive) emit(Processor::TO_PRIMITIVE_OP); break;
		case ExpressionResult::NAMED: emitWithConstant(Processor::READ_NAMED_OP, xr.v); if (toPrimitive) emit(Processor::TO_PRIMITIVE_OP); break;
		case ExpressionResult::PROPERTY: emit(Processor::GET_PROPERTY_OP); if (toPrimitive) emit(Processor::TO_PRIMITIVE_OP); break;
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
		default: syntaxError("Illegal l-value"); // FIX : ECMA like the word "reference"
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
		syntaxError(std::string("Expected '") + token + "'");
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
			syntaxError(count == 0 ? "Expected ')'" : "Expected ',' or ')'");
		}
		if (count != 0) {
			if (!token(",", false)) {
				syntaxError("Expected ',' or ')'");
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
			if (p + 1 != e && p[1] == 'x') {
				UInt32 u;
				const Char* b = p += 2;
				p = parseHex(b, e, u);
				if (p == b) {
					syntaxError("Invalid number");
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
				syntaxError("Invalid number");
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
	while (!token("]", true)) {
		rvalueExpression(COMMA_PREC);
		emitWithConstant(Processor::ADD_PROPERTY_OP, Value(String::fromInt(heap, index)));
		if (token("]", true)) {
			break;
		}
		if (!token(",", true)) {
			syntaxError("Expected ',' or ']'");
		}
		++index;
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
				syntaxError("Expected property name");
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
			syntaxError("Expected ',' or '}'");
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

CodeDefinition* Compiler::functionDefinition(const Char* selfNameBegin, const Char* selfNameEnd) {
	CodeDefinition* func = heap.manage(new CodeDefinition());
	if (selfNameBegin != selfNameEnd) {
		func->selfName = String::newHashedString(heap, selfNameBegin, selfNameEnd);
	}
	Compiler funcCompiler(heap, func, Compiler::FOR_FUNCTION);
	heap.protect(&funcCompiler);
	p = funcCompiler.compile(p, e);
	return func;
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
					emitWithConstant(Processor::GEN_FUNC_OP, Value(functionDefinition(b, p)));
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
		syntaxError("Expected expression");
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

Compiler::ExpressionResult Compiler::declareIdentifier() {
	const Char* b = identifier(true, false);
	const String* name = String::newHashedString(heap, b, p);
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
	ExpressionResult lxr(declareIdentifier());
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
	if (!eof() && *p != '}' && !token(";", false) && std::find(b, p, '\n') == p) {
		p = b;
		syntaxError("Syntax error");
	}
}

// FIX : make member functions of all the different statements

void Compiler::functionStatement() {
	outputCode = &funcInitCode;
	ExpressionResult lxr(declareIdentifier());
	emitWithConstant(Processor::GEN_FUNC_OP, Value(functionDefinition(p, p)));
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
					syntaxError("Duplicate label");
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
					syntaxError(!label.empty() ? "Undefined label"
							: (isBreak ? "Illegal break" : "Illegal continue"));
				}
				if (!isBreak && s->type != Scope::ITERATOR_LABEL_TYPE)
					syntaxError("Illegal label for continue");
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
					syntaxError("Illegal return");
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
					if (!gotCatch) syntaxError("Expected 'catch' or 'finally'");
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
						syntaxError("Expected '}'");
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
						if (defaultPoint != INVALID_CODE_POINT) syntaxError("More than one 'default:'");
						flushXR(evalXR);
						defaultPoint = markBackwardBranch();
					} else if (firstStatement) {
						syntaxError("Expected 'case', 'default' or '}'");
					} else {
						const Char* b = p;
						statement(&emptyLabelScope, &emptyLabelScope, evalXR);
						if (b == p) syntaxError("Expected '}'");
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
	if (compilingFor == FOR_FUNCTION) {
		expectToken("(", true);
		white();
		while (!token(")", false)) {
			if (eof()) {
				syntaxError(codeDef->argumentNames.size() == 0 ? "Expected ')'" : "Expected ',' or ')'");
			}
			if (codeDef->argumentNames.size() != 0) {
				if (!token(",", false)) {
					syntaxError("Expected ',' or ')'");
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
	}
	ExpressionResult evalXR;
	statementList(0, evalXR);
	if (compilingFor == FOR_FUNCTION) {
		expectToken("}", false);
	}
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

void Compiler::compile(const String& source) {
	compile(source.begin(), source.end());
	white();
	if (!eof()) {
		syntaxError("Syntax error"); // FIX : better error?
	}
}

GCHeap::GCHeap() : currentList(&managedLists[0]), newList(&managedLists[1]) { }

UInt32 GCHeap::size() const { return currentList->size(); }
UInt32 GCHeap::gc() {
	UInt32 sweptCount = 0;

	for (const GCItem* item = rootList._gcNext; item != &rootList; item = item->_gcNext)
		const_cast<GCItem*>(item)->gcMark(*this);
	
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
		const_cast<GCItem*>(item)->gcMark(*this);
	}
}

class FromCharCodeFunction : public Function { // FIX : .length?! and all the others?!
	public:		typedef Function super;
				virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) {
					Char c;
					if (argc == 1 && (c = argv[0].toInt()) < 127) return QUICK_CONSTANTS.ascii[c];
					String* s = engine.getHeap().manage(new String(argc));
					for (Int32 i = 0; i < argc; ++i) (*s)[i] = static_cast<Char>(argv[i].toInt());
					return Value(s);
				}
};

Value EvalFunction::call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) {
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

const String FROM_CHAR_CODE_STRING("fromCharCode");

Engine::Engine(GCHeap& heap, FunctionFrame* globals) : gcHeap(heap), globals(globals), evalFunction(false)
		, evalDirectFunction(true), functionPrototype(0), toPrimitiveValueFunction(0), toStringFunction(0) {
	assert(heap.belongs(globals));
	gcHeap.protect(&stringPrototype);
	stringPrototype.setProperty(*this, &FROM_CHAR_CODE_STRING, Value(gcHeap.manage(new FromCharCodeFunction())));
	gcHeap.protect(&objectPrototype);
	gcHeap.protect(&arrayPrototype);
//	functionPrototype = gcHeap.protect(new UserFunction);
	ObjectConstructor* objectObject = gcHeap.manage(new ObjectConstructor());
	ArrayConstructor* arrayObject = gcHeap.manage(new ArrayConstructor());
//	FunctionConstructor* functionObject = gcHeap.manage(new FunctionConstructor());
	objectObject->setProperty(*this, &PROTOTYPE_STRING, Value(&objectPrototype));
	arrayObject->setProperty(*this, &PROTOTYPE_STRING, Value(&arrayPrototype));
//	functionObject->setProperty(*this, &PROTOTYPE_STRING, Value(&functionPrototype));
	globals->setProperty(*this, &O_BJECT_STRING, Value(objectObject));
	globals->setProperty(*this, &A_RRAY_STRING, Value(arrayObject));
//	globals->setProperty(*this, &F_UNCTION_STRING, Value(functionObject));
	globals->setProperty(*this, &EVAL_STRING, Value(gcHeap.protect(&evalFunction)));
	gcHeap.protect(&evalDirectFunction);
	
	globals->setProperty(*this, &UNDEFINED_STRING, Value::UNDEFINED);
	globals->setProperty(*this, &NAN_STRING, Value::NOT_A_NUMBER);
	globals->setProperty(*this, &INFINITY_STRING, Value::INFINITE_NUMBER);
	
	{
		UserObject* mathObject = gcHeap.manage(new UserObject(&objectPrototype));
		static const String M_ATH_STRING("Math");
		globals->setProperty(*this, &M_ATH_STRING, Value(mathObject));

		static struct SqrtFunction : public Function { // FIX : .length?! and all the others?!
			virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) {
				return (argc >= 1 ? Value(sqrt(argv[0].toDouble())) : Value::NOT_A_NUMBER);
			}
		} SQRT_FUNCTION;
		static const String SQRT_STRING("sqrt");
		mathObject->setProperty(*this, &SQRT_STRING, Value(&SQRT_FUNCTION));

		static struct FloorFunction : public Function { // FIX : .length?! and all the others?!
			virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) {
				return (argc >= 1 ? Value(floor(argv[0].toDouble())) : Value::NOT_A_NUMBER);
			}
		} FLOOR_FUNCTION;
		static const String FLOOR_STRING("floor");
		mathObject->setProperty(*this, &FLOOR_STRING, Value(&FLOOR_FUNCTION));
	}

	{
		CodeDefinition* codeDef = gcHeap.manage(new CodeDefinition());
		Compiler compiler(gcHeap, codeDef, Compiler::FOR_FUNCTION);
		gcHeap.protect(&compiler);
		// FIX : throw what?
		// full source:
		/*
			function toPrimitiveValue(o) {
				var v;
				function convertWith(m) {
					var t;
					return (typeof (o[m]) === "function" && ((v = o[m]()) === null || ((t = typeof(v)) !== 'object' && t !== 'function')));
				}
				if (!convertWith("valueOf") && !convertWith("toString")) {
					throw("error");
				}
				return v;
			}
		*/
supressStdErr = true;
		compiler.compile(String("(c){function a(a){var d;return'function'===typeof c[a]&&(null===(b=c[a]())||'object'!==(d=typeof b)&&'function'!==d)}var b;if(!a('valueOf')&&!a('toString'))throw'error';return b}"));
		toPrimitiveValueFunction = gcHeap.protect(new UserFunction(codeDef, globals));
	}
	{
		CodeDefinition* codeDef = gcHeap.manage(new CodeDefinition());
		Compiler compiler(gcHeap, codeDef, Compiler::FOR_FUNCTION);
		gcHeap.protect(&compiler);
		// FIX : isb+'' right at the end here? + converts with toPrimitiveValue
		compiler.compile(String("(c){function a(a){var d;return'function'===typeof c[a]&&(null===(b=c[a]())||'object'!==(d=typeof b)&&'function'!==d)}var b;if(!a('toString')&&!a('valueOf'))throw'error';return b+''}"));
		toStringFunction = gcHeap.protect(new UserFunction(codeDef, globals));
supressStdErr = false;
	}
}

#include <fstream>
#include <memory>

	class PrintFunction : public Function {
		public:		typedef Function super;
					virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) {
						const String* s = (argc >= 1 ? argv[0].toString(engine.getHeap()) : &EMPTY_STRING);
						std::wcout << s->toStdString().c_str() << std::endl;
						return Value::UNDEFINED;
					}
	};
	
	struct GCFunction : public Function {
		virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) {
			UInt32 sweptCount = engine.getHeap().gc();
			return Value(static_cast<Int32>(sweptCount));
		}
	};

int main(int argc, const char* argv[]) {
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
	UserFunction globalClosure(&globalCode, &globals);
	heap.protect(&globalClosure);
	FunctionFrame globalFrame(&globalClosure, 0, 0);
	heap.protect(&globalFrame);
	
	Engine engine(heap, &globalFrame);
	
	globals.setProperty(engine, &S_TRING_STRING, Value(&EMPTY_STRING));
	PrintFunction printFunction;
	globals.setProperty(engine, &PRINT_STRING, Value(heap.protect(&printFunction)));
	GCFunction gcFunction;
	const String GC_STRING("gc");
	globals.setProperty(engine, &GC_STRING, Value(heap.protect(&gcFunction)));
	String source(EMPTY_STRING);
	std::auto_ptr<std::wifstream> inputFileStream;
	std::wistream* inStream = &std::wcin;
	bool doTime = false;
	long gcRate = 20000000;
	bool pauseBeforeQuit = false;
	bool interactive = true;
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
				if (!s.empty()) {
					String line(s);
					if (!source.empty()) source = String(source, LF_STRING);
					source = String(source, line);
				} else {
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
				catch (Exception& x) {
					std::cout << "Compilation error: " << x.what() << std::endl;
					int lineNumber = 1;
					const Char* stopPoint = compiler.getStopPoint();
					const Char* p = source.begin();
					while (p < stopPoint) {
						assert(p != source.end());
						if (*p == '\n') {
							++lineNumber;
						}
						++p;
					}
					std::cout << "Line: " << lineNumber << std::endl;
					throw;
				}
				globalFrame.updatedCode();
				processor.chain(globalCode.getCodePointer(), &globalFrame, &globalFrame);
				
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
		catch (const std::exception& x) {
			std::cout << "Exception: " << x.what() << std::endl;
			source = EMPTY_STRING;
		}
		catch (...) {
			std::cout << "Unknown exception" << std::endl;
			source = EMPTY_STRING;
		}
	}
	if (pauseBeforeQuit) std::wcin.get();
	return 0;
}
