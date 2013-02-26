// TODO
//
// for (in)
// switch
// break, continue etc (with labels!)
// native calls (and returns)
//	need easier API, shouldn't crash if missing push etc
//	no synchronized return to native code right now
//	arguments are reversed on native calls, and will be gc'd as stack has already popped 
//		arguments no longer reversed since we push forwards now, but will still be gc'd
// exceptions (and catching both virtual and native)
// typeof, instanceof, in operators
// new operator
//  done, but need to implement prototype
// prototype support
// eval support
// arguments container
// array literals
// object literals
// array object (length property support, optimized for numerical indexes?)
// count (and run-time check) necessary value stack depth for functions
// jump optimizations
// pop void optimization (almost always occurs with a return;)?
// special write with pop instructions (very common)?
// change byte code to int code and check performance, or maybe the opposite, add byte, short etc immediates?
//	not too fond of the int cast in the byte array
// don't accept lf's after ++ etc (automatic ; insertion logic)
// divide by 0 checks? NaN checks?
//	is currently not compatible with -ffast_math, which is bad I guess
// basic library (math, string, array ops)
// virtual machine should include gc etc
// also probe gcitems with gcheap == 0, so that we can have those safely on stack etc
//	thinking of dropping mark etc from gcheap and have in environment instead, we could then have a gcheap list for protected items (probe these before general mark phase)
// take value as key for getproperty etc and we could optimize array and string accesses by not having to go via strings
// assignable globals this in vm
// function name() { } syntax
// literal function names, e.g. myGlobalName = function myLocalName() { ... }
// better errors with line numbers and stuff (should we go back to the global "pickup" idea i had in old NuXScript?)
// global scope compiler (don't accept return etc)
//
// DONE
//
// for (;;)
// comma operator
// void operator
// delete operator
// proper string comparisons (< etc)

#include "assert.h"
#include <cmath>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <limits>
#include <map>
#include "NuXJScript.h"

// FIX : shouldn't need this, used in quick-hash-finders and they should be optimized somehow i think
static int strncmp(const Char* a, const char* b, size_t n) {
	int i = 0;
	while (i < n && a[i] == b[i] && a[i] != 0 && b[i] != 0) ++i;
	return (i == n ? 0 : a[i] - b[i]);
}

static double NaN() { return std::numeric_limits<double>::quiet_NaN(); }

// FIX : should first argument be a changable reference or not? we use that convention a lot in the compiler, but not here

static const Char* parseHex(const Char* p, const Char* e, UInt32& i) {
	for (i = 0; p != e && ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F') || (*p >= 'a' && *p <= 'f')); ++p)
		i = (i << 4) + (*p <= '9' ? *p - '0' : (*p & ~0x20) - ('A' - 10));
	return p;
}

static const Char* parseUnsignedInt(const Char* p, const Char* e, UInt32& i) {
	for (i = 0; p != e && *p >= '0' && *p <= '9'; ++p) i = i * 10 + (*p - '0');
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
	if (q == e || (*q != '.' && (*q < '0' || *q > '9'))) {
		v = 0.0;
		return p;
	}
	const Char* b = q;
	while (q != e && *q >= '0' && *q <= '9') d = d * 10.0 + (*q++ - '0');
	if (q != e && *q == '.') {
		double f = 1.0;
		while (++q != e && *q >= '0' && *q <= '9') d += (*q - '0') * (f *= 0.1);
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
		if (dp[-1] == '1') --ep;																						// We incremented a 0, cut precision (afterwards, a bit Q&D).
	}
	*pp = '.';
	if (ep > pp) while (ep[-1] == '0') --ep;
	if (ep - 1 == pp) --ep;
	if (d < 0) *--bp = '-';	
	return std::pair<Char*, Char*>(bp, ep);
}

const int ESCAPE_CODE_COUNT = 10;

static size_t unescapedMaxLength(const Char* p, const Char* e) {
	assert(p <= e && (*p == '"' || *p == '\''));
	Char endChar = *p;
	size_t l = 0;
	while (++p < e && *p != endChar && *p != '\r' && *p != '\n') { // FIX : use std::find_any
		if (*p == '\\' && p + 1 < e) ++p;
		++l;
	}
	return l;
}
	
static Char* unescape(Char* buffer, const Char*& p, const Char* e) {	
	assert(p <= e && (*p == '"' || *p == '\''));
	static const Char ESCAPE_CHARS[ESCAPE_CODE_COUNT] = { '\\', '\"', '\'', 'b',  'f',  'n',  'r',  't',  'v', '0' };
	static const Char ESCAPE_CODES[ESCAPE_CODE_COUNT] = { '\\', '\"', '\'', '\b', '\f', '\n', '\r', '\t', '\v', '\0' };
	if (p >= e || (*p != '"' && *p != '\'')) throw std::runtime_error("Invalid string literal");
	Char endChar = *p;
	const Char* b = ++p;
	Char* d = buffer;
	while (p < e && *p != endChar && *p != '\r' && *p != '\n') { // FIX : use std::find_any
		if (*p == '\\' && p + 1 < e) {
			d = std::copy(b, p, d);
			const Char* f = std::find(ESCAPE_CHARS, ESCAPE_CHARS + ESCAPE_CODE_COUNT, *++p);
			UInt32 l;
			if (f != ESCAPE_CHARS + ESCAPE_CODE_COUNT) { ++p; l = ESCAPE_CODES[f - ESCAPE_CHARS]; }
			else if (*p == 'x') { b = ++p; p = parseHex(p, (e - p > 2 ? p + 2 : e), l); } // FIX : in ecma, you *need* two (and four) hex chars, nothing else, throw here!
			else if (*p == 'u') { b = ++p; p = parseHex(p, (e - p > 4 ? p + 4 : e), l); }
			if (p == b) throw std::runtime_error("Invalid escape character");
			b = p;
			*d++ = Char(l);
		} else ++p;
	}
	if (p >= e || *p != endChar) throw std::runtime_error("Unterminated string");
	return std::copy(b, p++, d);
}

/* --- String --- */

String::String(const char* sz) : chars(strlen(sz)), hash(0) { std::copy(sz, sz + chars.size(), chars.data()); calcHash(); }

const int QUICK_CONSTANTS_INTEGERS_RANGE = 1000;
struct QuickConstants {
	QuickConstants() {
		try {
			std::fill_n(integers, QUICK_CONSTANTS_INTEGERS_RANGE * 2 + 1, (String*)(0));
			for (int i = 32; i < 127; ++i) {
				char buf[2] = { i, 0 };
				ascii[i - 32] = Value(new String(buf));
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
		for (int i = 32; i < 127; ++i) {
			delete ascii[i - 32].getString();
			ascii[i - 32] = Value();
		}
		for (int i = -QUICK_CONSTANTS_INTEGERS_RANGE; i <= QUICK_CONSTANTS_INTEGERS_RANGE; ++i) {
			delete integers[i + QUICK_CONSTANTS_INTEGERS_RANGE];
			integers[i + QUICK_CONSTANTS_INTEGERS_RANGE] = 0;
		}
	}
	~QuickConstants() { cleanup(); }
	Value ascii[127 - 32];
	String* integers[QUICK_CONSTANTS_INTEGERS_RANGE * 2 + 1];
};

QuickConstants QUICK_CONSTANTS;

String EMPTY_STRING("");
String UNDEFINED_STRING("undefined");
String NULL_STRING("null");
String FALSE_STRING("false");
String TRUE_STRING("true");
String NAN_STRING("NaN");
String INFINITY_STRING("Infinity");
String MINUS_INFINITY_STRING("-Infinity");
String ZERO_STRING("0");
String LENGTH_STRING("length");

String::String(const Char* b, const Char* e) : chars(e - b), hash(0) { std::copy(b, e, chars.data()); }

String::String(const String* l, const String* r) : chars(l->chars.size() + r->chars.size()), hash(0) {
	Char* e = std::copy(l->chars.data(), l->chars.data() + l->chars.size(), chars.data());
	std::copy(r->chars.data(), r->chars.data() + r->chars.size(), e);
}

String* String::newHashedString(GCHeap* heap, const Char* b, const Char* e) {
	String* o = heap->manage(new String(b, e));
	o->calcHash();
	return o;
}

bool String::getProperty(const String* name, Value& v) {
	if (name->isEqualTo(&LENGTH_STRING)) {
		v = Value(static_cast<Int32>(chars.size()));
		return true;
	}
	UInt32 i;
	const Char* p = name->chars.data();
	const Char* e = p + name->chars.size();
	const Char* q = parseUnsignedInt(p, e, i);
	if (q == e && i < chars.size()) {
		Char c = chars[i];
		if (32 <= c && c < 127) v = QUICK_CONSTANTS.ascii[c - 32];
		else v = Value(gcHeap()->manage(new String(chars.data() + i, chars.data() + i + 1)));
		return true;
	}
	v = Value();
	return false;
}

// FIX : speed up by skipping when string is really long
// FNV Hash
Table::HashType String::calcHash() const {
	if (hash == 0) {
		size_t l = chars.size();
		UInt32 x = 2166136261U;
		for (size_t i = 0; i < l; ++i) {
			x *= 16777619;
			x += chars[i];
		}
		hash = (x == 0 ? x + 1 : x);
	}
	return hash;
}

bool String::isEqualTo(const String* o) const {
	if (this == o) return true;
	if (hash != o->hash && hash != 0 && o->hash != 0) return false;
	size_t l = chars.size();
	if (l != o->chars.size()) return false;
	for (int i = 0; i < l; ++i) if (chars[i] != o->chars[i]) return false;
	return true;
}

bool String::isLessThan(const String* o) const {
	if (this == o) return false;
	size_t l = std::min(chars.size(), o->chars.size());
	for (int i = 0; i < l; ++i) if (chars[i] != o->chars[i]) return (chars[i] < o->chars[i]);
	return chars.size() < o->chars.size();
}

size_t String::getLength() const { return chars.size(); }
const Char* String::accessChars() const { return chars.data(); }
std::wstring String::toStdString() const { return std::wstring(chars.data(), chars.data() + chars.size()); }

/* --- GCHeap --- */

GCHeap::GCHeap() : count(0), items(new GCItem()) { }

size_t GCHeap::sweep() {
	size_t n = 0;
	for (const GCItem* i = items->_gcNext->_gcNext; i != items->_gcNext; i = i->_gcNext) {
		assert(i->_gcPrev->_gcHeap == this);
		delete i->_gcPrev;
		++n;
	}

	items->_gcPrev = items->_gcNext = items;
	assert(n == count);
	count = 0;
	return n;
}

size_t GCHeap::size() const { return count; }

void GCHeap::claim(const GCItem* item) {
	if (item->_gcHeap != 0) {
		item->_gcHeap->count--;
		item->_gcPrev->_gcNext = item->_gcNext;
		item->_gcNext->_gcPrev = item->_gcPrev;
	}
	item->_gcPrev = items;
	item->_gcNext = items->_gcNext;
	items->_gcNext->_gcPrev = item;
	items->_gcNext = item;
	item->_gcHeap = this;
	++count;
}

void GCHeap::mark(const GCItem* item) {
	if (item != 0 && item->_gcHeap != this && item->_gcHeap != 0) {
		claim(item);
		const_cast<GCItem*>(item)->gcMark(this);
	}
}

GCHeap::~GCHeap() {
	sweep();
	delete items;
}

/* --- Table --- */

Table::Bucket::Bucket() : key(0), value(Value::DELETED_TYPE) { }

UInt32 Table::calcMaxLoad(UInt32 bucketCount) {
	return (bucketCount - (bucketCount >> 3) - 1);
}

UInt32 Table::suggestN(UInt32 count) { // FIX : keep?
	UInt32 n = 0;
	while (count >= calcMaxLoad(1U << n)) ++n;
	return n;
}

Table::Table(UInt32 n) : n(TABLE_BUILT_IN_N), buckets(1U << TABLE_BUILT_IN_N), loadCount(0) { }

const Table::Bucket* Table::getFirst() const { return getNext(&buckets[0]); }

const Table::Bucket* Table::getNext(const Bucket* b) const {
	assert(b != 0);
	const Bucket* e = &buckets[buckets.size()];
	while (++b != e && b->value.isDeleted()) { }
	return (b == e ? 0 : b);
}

const Table::Bucket* Table::lookup(const String* key) {
	const Bucket* b = find(key, key->calcHash());
	return (b->key != 0 ? b : 0);
}

void Table::assign(const String* key, const Value& value) { assert(!value.isDeleted()); update(key, value); }
void Table::erase(const String* key) { update(key, Value(Value::DELETED_TYPE)); }

void Table::mark(GCHeap* heap) {
	UInt32 rebuildLoadCount = 0;
	for (UInt32 i = 0; i < buckets.size(); ++i) {
		heap->mark(buckets[i].key);
		buckets[i].value.mark(heap);
		if (!buckets[i].value.isDeleted()) rebuildLoadCount++;
	}
	if (rebuildLoadCount != loadCount) {
		UInt32 check = rebuild((loadCount < calcMaxLoad(1U << (n - 1))) ? n - 1 : n);
		(void)check;
		assert(check == rebuildLoadCount);
	}
}

void Table::update(const String* key, const Value& value) {
	HashType hash = key->calcHash();
	Bucket* b = find(key, hash);
	if (b->key == 0) {
		if (loadCount >= calcMaxLoad(buckets.size())) {
			rebuild(n + 2);
			assert(loadCount < calcMaxLoad(buckets.size()));
			b = find(key, hash);
			assert(b->key == 0);
		}
		++loadCount;
		b->hash = hash;
		b->key = key;
	}
	b->value = value;
}

Table::Bucket* Table::find(const String* key, HashType hash) const {
	UInt32 mask = (buckets.size() - 1);
	UInt32 i = hash & mask;
	while (buckets[i].key != 0 && (buckets[i].hash != hash || !buckets[i].key->isEqualTo(key))) i = (i + 1) & mask;
	return &buckets[i];
}

UInt32 Table::rebuild(UInt32 newN) {
	QuickArray<Bucket, 1U << TABLE_BUILT_IN_N> newBuckets(1U << newN);
	UInt32 newLoadCount = 0;
	UInt32 mask = (1U << newN) - 1;
	for (UInt32 i = 0; i < buckets.size(); ++i) {
		if (!buckets[i].value.isDeleted()) {
			assert(buckets[i].key != 0);
			UInt32 j = buckets[i].key->calcHash() & mask;
			while (newBuckets[j].key != 0) j = (j + 1) & mask;
			newBuckets[j] = buckets[i];
			++newLoadCount;
		}
	}
	buckets.swap(newBuckets);
	n = newN;
	loadCount = newLoadCount;
	return newLoadCount;
}

/* --- UserObject --- */

bool UserObject::setProperty(const String* name, const Value& v) {
	properties.assign(name, v);
	return true;
}

bool UserObject::getProperty(const String* name, Value& v) {
	const Table::Bucket* b = properties.lookup(name);
	if (b != 0) {
		v = b->value;
		return true;
	}
	v = Value();
	return false;
}

bool UserObject::deleteProperty(const String* name) {
	properties.erase(name);
	return true;
}

void UserObject::gcMark(GCHeap* heap) {
	properties.mark(heap);
	super::gcMark(heap);
}

Value::Value(Type type) : type(type) { }
Value::Value(bool b) : type(BOOLEAN_TYPE), b(b) { }
Value::Value(Int32 i) : type(NUMBER_TYPE), d(i) { }
Value::Value(double d) : type(NUMBER_TYPE), d(d) { }
Value::Value(String* s) : type(STRING_TYPE), s(s) { }
Value::Value(Object* o) : type(OBJECT_TYPE), o(o) { }

bool Value::isDeleted() const { return type == DELETED_TYPE; }
bool Value::isObject() const { return type == OBJECT_TYPE; }

ScriptFunctionDef* Value::getFunctionDef() const {
	assert(type == OBJECT_TYPE);
	return reinterpret_cast<ScriptFunctionDef*>(o);
}

String* Value::getString() const {
	assert(type == STRING_TYPE);
	return s;
}

bool Value::toBool() const {
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return false;
		case NULL_TYPE: return false;
		case BOOLEAN_TYPE: return b;
		case NUMBER_TYPE: return (d != 0.0);
	}
}

Int32 Value::toInt() const { return static_cast<Int32>(toDouble()); } // FIX : should do modulo, see http://www.ecma-international.org/ecma-262/5.1/#sec-9.6

double Value::toDouble() const {
	switch (type) {
		default: assert(0); // FIX : object type
		case UNDEFINED_TYPE: return NaN();
		case NULL_TYPE: return 0.0;
		case BOOLEAN_TYPE: return b ? 1.0 : 0.0;
		case NUMBER_TYPE: return d;
		case STRING_TYPE: {
			double v;
			const Char* p = s->accessChars();
			const Char* q = parseDouble(p, p + s->getLength(), v);
			if (q == p) return NaN();
			return v;
		}
	}
}

String* Value::toString(GCHeap* heap) const {
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return &UNDEFINED_STRING;
		case NULL_TYPE: return &NULL_STRING;
		case BOOLEAN_TYPE: return b ? &TRUE_STRING : &FALSE_STRING;
		case NUMBER_TYPE: {
			Char buffer[32];
			int i = static_cast<int>(d);
			if (i == d) {
				if (i >= -QUICK_CONSTANTS_INTEGERS_RANGE && i <= QUICK_CONSTANTS_INTEGERS_RANGE)
					return QUICK_CONSTANTS.integers[i + QUICK_CONSTANTS_INTEGERS_RANGE];
				else return heap->manage(new String(intToString(buffer, i), buffer + 32));
			}
			if (std::isnan(d)) return &NAN_STRING;
			double x = fabs(d);
			if (x <= EPSILON) return &ZERO_STRING;
			if (x == std::numeric_limits<double>::infinity()) return d < 0 ? &MINUS_INFINITY_STRING : &INFINITY_STRING;
			std::pair<Char*, Char*> r = doubleToString(buffer, d);
			return heap->manage(new String(r.first, r.second));
		}
		case STRING_TYPE: return s;
case OBJECT_TYPE: return heap->manage(new String("<object>"));
	}
}

Object* Value::toObject(GCHeap* heap) const {
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return 0;
		case NULL_TYPE: return 0;
		case BOOLEAN_TYPE: /* FIX */ return 0;
		case NUMBER_TYPE: /* FIX */ return 0;
		case STRING_TYPE: return s;
		case OBJECT_TYPE: return o;
	}
}

bool Value::operator==(const Value& y) const {
	if (type != y.type) return false;
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE:
		case NULL_TYPE:
		case DELETED_TYPE:
		case BOOLEAN_TYPE: return b == y.b;
		case NUMBER_TYPE: return d == y.d;
		case STRING_TYPE: return s->isEqualTo(y.s);
		case OBJECT_TYPE: return o == y.o;
	}
}

bool Value::operator<(const Value& y) const {
	if (type < y.type) return true;
	if (type > y.type) return false;
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE:
		case NULL_TYPE:
		case DELETED_TYPE: return false;
		case BOOLEAN_TYPE: return b < y.b;
		case NUMBER_TYPE: return d < y.d;
		case STRING_TYPE: return s->isLessThan(y.s);
		case OBJECT_TYPE: return o < y.o;
	}
}

bool Value::compareStrictly(const Value& y) const {
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return true;
		case NULL_TYPE: return true;
		case BOOLEAN_TYPE: return b == y.b;
		case NUMBER_TYPE: {
			double thisNumber = toDouble();
			double otherNumber = y.toDouble();
		// FIX : necessary to avoid optimizing away?
		//	if (std::isnan(thisNumber) || std::isnan(otherNumber)) return false;
			return thisNumber == otherNumber;
		}
		case STRING_TYPE: return s->isEqualTo(y.s);
		case OBJECT_TYPE: return o == y.o;
	}
}

bool Value::isStrictlyEqualTo(const Value& y) const {
	if (type != y.type) return false;
	else return compareStrictly(y);
}

bool Value::isEqualTo(const Value& y) const {
	if (type == y.type) return compareStrictly(y);
	else {
		if (type == NULL_TYPE && y.type == UNDEFINED_TYPE) return true;
		if (type == UNDEFINED_TYPE && y.type == NULL_TYPE) return true;
		if (type == NUMBER_TYPE && y.type == STRING_TYPE) return d == y.toDouble();
		if (type == STRING_TYPE && y.type == NUMBER_TYPE) return toDouble() == y.d;
		if (type == BOOLEAN_TYPE || y.type == BOOLEAN_TYPE) return toDouble() == y.toDouble();
		// If Type(x) is either String or Number and Type(y) is Object,
		// return the result of the comparison x == ToPrimitive(y).
		// If Type(x) is Object and Type(y) is either String or Number,
		// return the result of the comparison ToPrimitive(x) == y.
		return false;
	}
}

bool Value::isLessThan(const Value& y) const {
	if (type == STRING_TYPE && y.type == STRING_TYPE) return s->isLessThan(y.s);
	double thisNumber = toDouble();
	double otherNumber = y.toDouble();
	// FIX : necessary? if (std::isnan(thisNumber) || std::isnan(otherNumber)) return Value();
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
	if (type == STRING_TYPE && y.type == STRING_TYPE) return !y.s->isLessThan(s);
	double thisNumber = toDouble();
	double otherNumber = y.toDouble();
	// FIX : necessary? if (std::isnan(thisNumber) || std::isnan(otherNumber)) return Value();
	return thisNumber <= otherNumber;
//	FIX : need to check?
//	If nx is +0 and ny is −0, return false.
//	If nx is −0 and ny is +0, return false.
//	If nx is +∞, return false.
//	If ny is +∞, return true.
//	If ny is −∞, return false.
//	If nx is −∞, return true.
}

Value Value::add(GCHeap* heap, const Value& y) const {
	if (type == STRING_TYPE || y.type == STRING_TYPE) {
		String* ls = toString(heap);
		String* rs = y.toString(heap);
		if (rs->getLength() == 0) return Value(ls);
		if (ls->getLength() == 0) return Value(rs);
		return Value(heap->manage(new String(ls, rs)));
	}
	return Value(toDouble() + y.toDouble());
}
				
void Value::mark(GCHeap* heap) const {
	switch (type) {
		case STRING_TYPE: heap->mark(s); break;
		case OBJECT_TYPE: heap->mark(o); break;
		default: break;
	}
}

/* --- ScriptFunctionDef --- */

ScriptFunctionDef::ScriptFunctionDef() : source(0), argumentsCount(0), localsCount(0) { }

bool ScriptFunctionDef::lookupLocalName(const String* name, Int32& index) const {
	LocalNameMap::const_iterator it = locals.find(name);
	if (it == locals.end()) return false;
	index = it->second;
	return true;
}

void ScriptFunctionDef::gcMark(GCHeap* heap) {
	heap->mark(source);
	// FIX : subs
	for (std::vector<Value>::iterator it = constants.begin(), e = constants.end(); it != e; ++it) {
		it->mark(heap);
	}
	for (LocalNameMap::iterator it = locals.begin(), e = locals.end(); it != e; ++it) {
		heap->mark(it->first);
	}
	super::gcMark(heap);
}

const Value* ScriptFunctionDef::getConstantsPointer() const { return (constants.empty() ? 0 : &constants[0]); }

/* --- Function --- */

Function* Function::asFunction() { return this; }

void Function::construct(VM* vm, Int32 argc, const Value* argv) {
	Object* newObject = gcHeap()->manage(new UserObject());
	Value thisValue(newObject);
	vm->push(thisValue);
	call(vm, argc, argv, thisValue);
}

/* --- ScriptFunction --- */

ScriptFunction::ScriptFunction(ScriptFunctionDef* definition, Object* scope) : definition(definition), scope(scope) { }
ScriptFunctionDef* ScriptFunction::getDefinition() const { return definition; }
Object* ScriptFunction::getScope() const { return scope; }

void ScriptFunction::call(VM* vm, Int32 argc, const Value* argv, const Value& thisValue) {
	vm->enterCall(this, argc, argv, thisValue);
}

void ScriptFunction::gcMark(GCHeap* heap) {
	heap->mark(definition);
	heap->mark(scope);
	super::gcMark(heap);
}

#if 0
/* --- ArgumentsObject --- */

ArgumentsObject(Frame* frame) : frame(frame) { }

bool ArgumentsObject::getProperty(const String* name, Value& v) {
	if (name->s == "length") return 
}

void ArgumentsObject::gcMark(GCHeap* heap) {
	heap->mark(frame);
	super::gcMark(heap);
}
#endif

/* --- Frame --- */

Frame::Frame(ScriptFunction* function) : functionDef(function->getDefinition()), scope(function->getScope())
		, locals(std::max(functionDef->localsCount, 1)) {
}

ScriptFunctionDef* Frame::getFunctionDef() { return functionDef; }
Value* Frame::getLocalsPointer() { return &locals[0]; }

// TODO: solution for injecting dynamic bound variables, e.g. from running eval(), could secretely add a scope above maybe?

bool Frame::getProperty(const String* name, Value& v) {
	Int32 index;
	if (functionDef->lookupLocalName(name, index)) {
		assert(0 <= index && index < locals.size());
		v = locals[index];
		return true;
	} else {
		return scope->getProperty(name, v); // FIX: can we do a looping non-recursive solution? faster
	}
}

bool Frame::setProperty(const String* name, const Value& v) {
	Int32 index;
	if (functionDef->lookupLocalName(name, index)) {
		assert(0 <= index && index < locals.size());
		locals[index] = v;
		return true;
	} else {
		return scope->setProperty(name, v);
	}
}

bool Frame::deleteProperty(const String* name) {
	Int32 index;
	if (functionDef->lookupLocalName(name, index)) return false;
	else return scope->deleteProperty(name);
}

void Frame::gcMark(GCHeap* heap) {
	heap->mark(functionDef);
	heap->mark(scope);
	// FIX : subs
	for (Int32 i = 0; i < locals.size(); ++i) locals[i].mark(heap);
	super::gcMark(heap);
}

/* --- VM --- */

VM::VM(Context* cstackStart, Context* cstackEnd, Value* vstackStart, Value* vstackEnd)
		: cstackStart(cstackStart), cstackEnd(cstackEnd), vstackStart(vstackStart), vstackEnd(vstackEnd)
		, cstackPointer(cstackStart - 1), vstackPointer(vstackStart - 1) {
}

void VM::gcMark(GCHeap* heap) {
	for (Value* vsp = vstackStart; vsp != vstackPointer + 1; ++vsp) vsp->mark(heap);
	for (Context* csp = cstackStart; csp != cstackPointer + 1; ++csp) heap->mark(csp->frame);
	super::gcMark(heap);
}

void VM::push(Value*& vsp, const Value& v) { assert(vsp + 1 < vstackEnd); *++vsp = v; }
void VM::push(const Value& v) { push(vstackPointer, v); }
void VM::pop(Value*& vsp, int count) { assert(vsp - count + 1 >= vstackStart); vsp -= count; }
void VM::poppush(Value*& vsp, const Value& v) { assert(vsp >= vstackStart); *--vsp = v; }
Int32 VM::readInt32(const Byte* ip) { return *reinterpret_cast<const Int32*>(ip); }
Int32 VM::nextInt32(const Byte*& ip) { Int32 v = *reinterpret_cast<const Int32*>(ip); ip += 4; return v; }

void VM::error() {
	throw std::runtime_error("error"); // FIX
}

void VM::enterCall(ScriptFunction* func, Int32 argc, const Value* argv, const Value& thisValue) {
	ScriptFunctionDef* def = func->getDefinition();
	Frame* frame = gcHeap()->manage(new Frame(func));
	++cstackPointer;
	cstackPointer->frame = frame;
	cstackPointer->thisValue = thisValue;
	cstackPointer->instructionPointer = &def->code[0];
	std::copy(argv, argv + std::min(argc, def->argumentsCount), frame->getLocalsPointer());
}

bool VM::run(Int32 count) {
	Value* vsp = vstackPointer;
	const Byte* ip = cstackPointer->instructionPointer;
	Frame* frame = cstackPointer->frame;
	const Value* cp = frame->getFunctionDef()->getConstantsPointer();
	Value* lp = frame->getLocalsPointer();
	GCHeap* h = gcHeap();
	while (--count >= 0) {
		switch (*ip++) {
			default: assert(0);
			case POP_OP: pop(vsp, 1); break;
			case REPUSH_OP: push(vsp, vsp[0]); break;
			case CONST_OP: push(vsp, cp[nextInt32(ip)]); break;
			case READ_INDEXED_OP: push(vsp, lp[nextInt32(ip)]); break;
			case WRITE_INDEXED_OP: lp[nextInt32(ip)] = vsp[0]; break;
			case READ_NAMED_OP: {
				Value v;
				bool success = frame->getProperty(cp[nextInt32(ip)].getString(), v);
				if (!success) error();
				push(vsp, v);
				break;
			}
			case WRITE_NAMED_OP:  {
				bool success = frame->setProperty(cp[nextInt32(ip)].getString(), vsp[0]);
				if (!success) error();
				break;
			}
			case GET_PROPERTY_OP: {
				Object* o = vsp[-1].toObject(h);
				if (o == 0) error();
				o->getProperty(vsp[0].toString(h), vsp[-1]);
				pop(vsp, 1);
				break;
			}
			case SET_PROPERTY_OP: {
				Object* o = vsp[-2].toObject(h);
				if (o == 0) error();
				o->setProperty(vsp[-1].toString(h), vsp[0]);
				vsp[-2] = vsp[0];
				pop(vsp, 2);
				break;
			}
			case ADD_OP: poppush(vsp, vsp[-1].add(h, vsp[0])); break;
			case SUB_OP: poppush(vsp, Value(vsp[-1].toDouble() - vsp[0].toDouble())); break;
			case MUL_OP: poppush(vsp, Value(vsp[-1].toDouble() * vsp[0].toDouble())); break;
			case DIV_OP: poppush(vsp, Value(vsp[-1].toDouble() / vsp[0].toDouble())); break;
			case MOD_OP: poppush(vsp, Value(fmod(vsp[-1].toDouble(), vsp[0].toDouble()))); break;
			case OR_OP: poppush(vsp, Value(vsp[-1].toInt() | vsp[0].toInt())); break;
			case XOR_OP: poppush(vsp, Value(vsp[-1].toInt() ^ vsp[0].toInt())); break;
			case AND_OP: poppush(vsp, Value(vsp[-1].toInt() & vsp[0].toInt())); break;
			case SHL_OP: poppush(vsp, Value(vsp[-1].toInt() << vsp[0].toInt())); break;
			case SHR_OP: poppush(vsp, Value(vsp[-1].toInt() >> vsp[0].toInt())); break;
			case UNSIGNED_SHR_OP: poppush(vsp, Value((Int32)((UInt32)(vsp[-1].toInt()) >> vsp[0].toInt()))); break;
			case PLUS_OP: vsp[0] = Value(vsp[0].toDouble()); break;
			case MINUS_OP: vsp[0] = Value(-vsp[0].toDouble()); break;
			case BIT_NOT_OP: vsp[0] = Value(~vsp[0].toInt()); break;
			case NOT_OP: vsp[0] = Value(!vsp[0].toBool()); break;
			case STRICT_EQ_OP: poppush(vsp, Value(vsp[-1].isStrictlyEqualTo(vsp[0]))); break;
			case STRICT_NEQ_OP: poppush(vsp, Value(!vsp[-1].isStrictlyEqualTo(vsp[0]))); break;
			case EQ_OP: poppush(vsp, Value(vsp[-1].isEqualTo(vsp[0]))); break;
			case NEQ_OP: poppush(vsp, Value(!vsp[-1].isEqualTo(vsp[0]))); break;
			case LT_OP: poppush(vsp, Value(vsp[-1].isLessThan(vsp[0]))); break; // FIX : do toPrimitive here, isLessThan is order-less
			case LEQ_OP: poppush(vsp, Value(vsp[-1].isLessThanOrEqualTo(vsp[0]))); break; // FIX : do toPrimitive here, isLessThan is order-less
			case GT_OP: poppush(vsp, Value(vsp[0].isLessThan(vsp[-1]))); break; // FIX : do toPrimitive here, isLessThan is order-less
			case GEQ_OP: poppush(vsp, Value(vsp[0].isLessThanOrEqualTo(vsp[-1]))); break; // FIX : do toPrimitive here, isLessThan is order-less
			case JMP_OP: ip += readInt32(ip) + 4; break;
			case JT_OP: ip += (vsp[0].toBool() ? readInt32(ip) + 4 : 4); pop(vsp, 1); break;
			case JF_OP: ip += (!vsp[0].toBool() ? readInt32(ip) + 4 : 4); pop(vsp, 1); break;
			case RETURN_OP: {
				if (cstackPointer == cstackStart) {
					// FIX : yeah, we can't you know :) we need a new way to return properly to native caller
					// cstackPointer->instructionPointer = ip;
					vstackPointer = vsp;
					return true;
				}
				--cstackPointer;
				ip = cstackPointer->instructionPointer;
				frame = cstackPointer->frame;
				cp = frame->getFunctionDef()->getConstantsPointer();
				lp = frame->getLocalsPointer();
				break;
			}
			case CALL_METHOD_OP: { // FIX : share more with the others...
				Int32 n = nextInt32(ip);
				Object* thisObject = vsp[-n - 1].toObject(h);
				if (thisObject == 0) error();
				Value v;
				thisObject->getProperty(vsp[-n].toString(h), v);
				Object* o = v.toObject(h);
				if (o == 0) error();
				Function* f = o->asFunction();
				if (f == 0) error();
				cstackPointer->instructionPointer = ip;
				vstackPointer = vsp - n - 2;
				f->call(this, n, vsp - n + 1, Value(thisObject));
				vsp = vstackPointer;
				ip = cstackPointer->instructionPointer;
				frame = cstackPointer->frame;
				cp = frame->getFunctionDef()->getConstantsPointer();
				lp = frame->getLocalsPointer();
				break;
			}
			case CALL_GLOBAL_OP: {
				Int32 n = nextInt32(ip);
				Object* o = vsp[-n].toObject(h);
				if (o == 0) error();
				Function* f = o->asFunction();
				if (f == 0) error();
				cstackPointer->instructionPointer = ip;
				vstackPointer = vsp - n - 1;
				f->call(this, n, vsp - n + 1, Value());
				vsp = vstackPointer;
				ip = cstackPointer->instructionPointer;
				frame = cstackPointer->frame;
				cp = frame->getFunctionDef()->getConstantsPointer();
				lp = frame->getLocalsPointer();
				break;
			}
			case NEW_OP: {
				Int32 n = nextInt32(ip);
				Object* o = vsp[-n].toObject(h);
				if (o == 0) error();
				Function* f = o->asFunction();
				if (f == 0) error();
				cstackPointer->instructionPointer = ip;
				vstackPointer = vsp - n - 1;
				f->construct(this, n, vsp - n + 1);
				vsp = vstackPointer;
				ip = cstackPointer->instructionPointer;
				frame = cstackPointer->frame;
				cp = frame->getFunctionDef()->getConstantsPointer();
				lp = frame->getLocalsPointer();
				break;
			}
			case NEW_RESULT_OP: poppush(vsp, (vsp[0].isObject() ? vsp[0] : vsp[-1])); break;
			case THIS_OP: push(vsp, cstackPointer->thisValue); break;
			case VOID_OP: push(vsp, Value()); break;
			case GEN_FUNC_OP: {
				push(vsp, Value(h->manage(new ScriptFunction(cp[nextInt32(ip)].getFunctionDef(), frame))));
				break;
			}
			case FRAME_OP: push(vsp, Value(frame)); break;
			case DELETE_OP: {
				Object* o = vsp[-1].toObject(h);
				if (o == 0) error();
				poppush(vsp, Value(o->deleteProperty(vsp[0].toString(h))));
				break;
			}
		}
	}
	cstackPointer->instructionPointer = ip;
	vstackPointer = vsp;
	return false;
}

static Int32 getInt32(std::vector<Byte>::const_iterator& it, const std::vector<Byte>::const_iterator& e) {
	assert(e - it >= 4);
	Int32 i = *reinterpret_cast<const Int32*>(&*it);
	it += 4;
	return i;
}

static void disassemble(GCHeap* heap, const std::vector<Byte>& code, const std::vector<Value>& constants) {
	const char* OP_STRINGS[] = {
		"!!! INVALID !!!"
		, "POP", "REPUSH", "CONST", "READ_INDEXED", "WRITE_INDEXED", "READ_NAMED", "WRITE_NAMED", "GET_PROPERTY", "SET_PROPERTY"
		, "ADD", "SUB", "MUL", "DIV", "MOD"
		, "OR", "XOR", "AND"
		, "SHL", "SHR", "UNSIGNED_SHR"
		, "PLUS", "MINUS"
		, "BIT_NOT", "NOT"
		, "STRICTLY_EQ", "STRICTLY_NEQ", "EQ", "NEQ", "LT", "LEQ", "GT", "GEQ"
		, "JMP", "JT", "JF"
		, "CALL_GLOBAL", "CALL_METHOD", "NEW", "NEW_RESULT"
		, "RETURN", "THIS", "VOID", "FRAME", "DELETE"
		, "GEN_FUNC"
	};
	std::vector<Byte>::const_iterator b = code.begin();
	std::vector<Byte>::const_iterator it = b;
	std::vector<Byte>::const_iterator e = code.end();
	while (it != e) {
		VM::Op opcode = static_cast<VM::Op>(*it);
		if (opcode < 0 || opcode >= sizeof (OP_STRINGS) / sizeof (*OP_STRINGS)) opcode = VM::INVALID_OP;
		std::cerr << "\t" << (it - b) << ':' << OP_STRINGS[opcode];
		++it;
		switch (opcode) {
			case VM::JT_OP:
			case VM::JF_OP:
			case VM::JMP_OP: std::cerr << " @" << ((it - b) + getInt32(it, e) + 4); break;
			case VM::READ_INDEXED_OP:
			case VM::WRITE_INDEXED_OP: std::cerr << " $" << getInt32(it, e); break;
			case VM::CALL_GLOBAL_OP:
			case VM::CALL_METHOD_OP:
			case VM::NEW_OP: std::cerr << " *" << getInt32(it, e); break;
			case VM::CONST_OP:
			case VM::GEN_FUNC_OP: std::wcerr << L" #" << constants[getInt32(it, e)].toString(heap)->toStdString(); break;
			case VM::READ_NAMED_OP:
			case VM::WRITE_NAMED_OP: std::wcerr << L" " << constants[getInt32(it, e)].getString()->toStdString(); break;
			default: break;
		}
		std::cerr << std::endl;
	}
}

/* --- Compiler --- */

// FIX : sort
enum OperatorType {
	GROUP, UNARY, BINARY, ASSIGNMENT, POST_UNARY, LOGICAL_AND_OR, PROPERTY_BRACKETS, PROPERTY_DOT, CONDITIONAL
	, FUNCTION_CALL, COMMA, VOID_OPERATOR, DELETE_OPERATOR, NEW_OPERATOR
};
enum Associativity { LEFT_TO_RIGHT, RIGHT_TO_LEFT };

struct OperatorInfo {
	const char* token;
	const char* closeToken;
	Precedence prec;
	Associativity assoc;
	OperatorType type;
	VM::Op vmOp;
};

// FIX : sort these two
const OperatorInfo PRE_OPS[] = {
	{ "(", ")", GROUP_PREC, RIGHT_TO_LEFT, GROUP, VM::INVALID_OP },
	{ "++", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::ADD_OP },
	{ "--", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::SUB_OP },
	{ "+", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::PLUS_OP },
	{ "+", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::PLUS_OP },
	{ "-", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::MINUS_OP },
	{ "~", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::BIT_NOT_OP },
	{ "!", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::NOT_OP },
	{ "void", 0, PREFIX_PREC, RIGHT_TO_LEFT, VOID_OPERATOR, VM::VOID_OP },
	{ "delete", 0, PREFIX_PREC, RIGHT_TO_LEFT, DELETE_OPERATOR, VM::VOID_OP },
	{ "new", 0, NEW_PREC, RIGHT_TO_LEFT, NEW_OPERATOR, VM::VOID_OP }
};

const OperatorInfo POST_OPS[] = {
	{ "===", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, VM::STRICT_EQ_OP },
	{ "==", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, VM::EQ_OP },
	{ "=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, VM::INVALID_OP },
	{ ">>>=", 0, SHIFT_PREC, RIGHT_TO_LEFT, ASSIGNMENT, VM::UNSIGNED_SHR_OP },
	{ ">>=", 0, SHIFT_PREC, RIGHT_TO_LEFT, ASSIGNMENT, VM::SHR_OP },
	{ "<<=", 0, SHIFT_PREC, RIGHT_TO_LEFT, ASSIGNMENT, VM::SHL_OP },
	{ "+=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, VM::ADD_OP },
	{ "-=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, VM::SUB_OP },
	{ "*=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, VM::MUL_OP },
	{ "/=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, VM::DIV_OP },
	{ "%=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, VM::MOD_OP },
	{ "&=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, VM::AND_OP },
	{ "^=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, VM::XOR_OP },
	{ "|=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, VM::OR_OP },
	{ ">>>", 0, SHIFT_PREC, LEFT_TO_RIGHT, BINARY, VM::UNSIGNED_SHR_OP },
	{ ">>", 0, SHIFT_PREC, LEFT_TO_RIGHT, BINARY, VM::SHR_OP },
	{ "<<", 0, SHIFT_PREC, LEFT_TO_RIGHT, BINARY, VM::SHL_OP },
	{ "++", 0, POST_INC_DEC, LEFT_TO_RIGHT, POST_UNARY, VM::ADD_OP },
	{ "--", 0, POST_INC_DEC, LEFT_TO_RIGHT, POST_UNARY, VM::SUB_OP },
	{ "&&", 0, LOGICAL_AND_PREC, LEFT_TO_RIGHT, LOGICAL_AND_OR, VM::JF_OP },
	{ "||", 0, LOGICAL_OR_PREC, LEFT_TO_RIGHT, LOGICAL_AND_OR, VM::JT_OP },
	{ "+", 0, ADD_SUB_PREC, LEFT_TO_RIGHT, BINARY, VM::ADD_OP },
	{ "-", 0, ADD_SUB_PREC, LEFT_TO_RIGHT, BINARY, VM::SUB_OP },
	{ "*", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, VM::MUL_OP },
	{ "/", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, VM::DIV_OP },
	{ "%", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, VM::MOD_OP },
	{ "&", 0, BIT_AND_PREC, LEFT_TO_RIGHT, BINARY, VM::AND_OP },
	{ "^", 0, BIT_XOR_PREC, LEFT_TO_RIGHT, BINARY, VM::XOR_OP },
	{ "|", 0, BIT_OR_PREC, LEFT_TO_RIGHT, BINARY, VM::OR_OP },
	{ "!==", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, VM::STRICT_NEQ_OP },
	{ "!=", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, VM::NEQ_OP },
	{ "<=", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, VM::LEQ_OP }, 
	{ "<", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, VM::LT_OP }, 
	{ ">=", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, VM::GEQ_OP }, 
	{ ">", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, VM::GT_OP },
	{ ".", 0, PROPERTY_PREC, LEFT_TO_RIGHT, PROPERTY_DOT, VM::INVALID_OP },
	{ "[", "]", PROPERTY_PREC, LEFT_TO_RIGHT, PROPERTY_BRACKETS, VM::INVALID_OP },
	{ "?", ":", CONDITIONAL_PREC, RIGHT_TO_LEFT, CONDITIONAL, VM::INVALID_OP },
	{ "(", ")", FUNCTION_CALL_PREC, LEFT_TO_RIGHT, FUNCTION_CALL, VM::INVALID_OP },
	{ ",", 0, COMMA_PREC, LEFT_TO_RIGHT, COMMA, VM::INVALID_OP }
};

enum Statement { VAR_STATEMENT, IF_STATEMENT, WHILE_STATEMENT, DO_WHILE_STATEMENT, FOR_STATEMENT, RETURN_STATEMENT };
/* Built with QuickHashMaker.pika */
static int findStatementToken(int n /* string length */, const Char* s) {
	static const char* STRINGS[6] = {
		"var", "if", "while", "do", "for", "return"
	};
	static const int QUICK_HASH_TABLE[8] = {
		1, 3, 4, 5, 0, 2, -1, -1
	};
	if (n < 2 || n > 6) return -1;
	int stringIndex = QUICK_HASH_TABLE[(s[1] + n) & 7];
	return (stringIndex >= 0 && strncmp(s, STRINGS[stringIndex], n) == 0 && STRINGS[stringIndex][n] == 0) ? stringIndex : -1;
};

struct Compiler::XValue {
	enum Type { NONE, PUSHED, CONSTANT, INDEXED, NAMED, PROPERTY };
	XValue() : t(NONE) { }
	XValue(Type type, const Value& value = Value()) : t(type), v(value) { }
	Type t;
	Value v;
};

Compiler::Compiler(ScriptFunctionDef* function) : function(function) { }

void Compiler::gcMark(GCHeap* heap) {
	heap->mark(function);
	for (std::map<Value, Int32>::iterator it = definedConstants.begin(), e = definedConstants.end(); it != e; ++it) {
		it->first.mark(heap);
	}
	super::gcMark(heap);
}

bool Compiler::isComment(const Char* p, const Char* e) {
	return (p != e && p + 1 != e && *p == '/' && (p[1] == '/' || p[1] == '*'));
}

void Compiler::eatComment(const Char*& p, const Char* e) {
	assert(isComment(p, e));
	if (p[1] == '/') {
		static const Char END_CHARS[] = { '\r', '\n' };
		p = std::find_first_of(p += 2, e, END_CHARS, END_CHARS + 2);
	} else {
		static const Char END_CHARS[] = { '*', '/' };
		p = std::search(p += 2, e, END_CHARS, END_CHARS + 2);
		if (p == e) throw std::runtime_error("Missing */");
		p += 2;
	}
}

void Compiler::eatWhite(const Char*& p, const Char* e) {
	while (p != e) {
		switch (*p) {
			case ' ': case '\t': ++p; break;
			case '\r': if (p + 1 != e && p[1] == '\n') ++p;
			case '\n': {																								// Eat leading .. on a new line
				while (++p != e && (*p == ' ' || *p == '\t')) { }
				if (e - p >= 2 && p[0] == '.' && p[1] == '.') p += 2;
				break;
			}
			case '/': if (isComment(p, e)) { eatComment(p, e); break; }
			/* else continue */
			default: return;
		}
	}
}

void Compiler::emit(VM::Op op) {
	std::vector<Byte>& code = function->code;
	code.push_back(op);
}

void Compiler::emitWithInt32(VM::Op op, Int32 i) {
	std::vector<Byte>& code = function->code;
	code.push_back(op);
	code.resize(code.size() + 4);
	*reinterpret_cast<Int32*>(&code[code.size() - 4]) = i;
}

Int32 Compiler::emitForwardBranch(VM::Op op) {
	std::vector<Byte>& code = function->code;
	code.push_back(op);
	Int32 point = code.size();
	code.resize(code.size() + 4);
	return point;
}

void Compiler::completeForwardBranch(Int32 point) {
	std::vector<Byte>& code = function->code;
	*reinterpret_cast<Int32*>(&code[point]) = code.size() - point - 4;
}

Int32 Compiler::markBackwardBranch() { return function->code.size(); }
void Compiler::emitBackwardBranch(VM::Op op, Int32 point) { emitWithInt32(op, point - (function->code.size() + 5)); }

void Compiler::emitWithConstant(VM::Op op, const Value& constant) {
	std::map<Value, Int32>::const_iterator it = definedConstants.find(constant);
	if (it == definedConstants.end()) {
		std::vector<Value>& constants = function->constants;
		constants.push_back(constant);
		definedConstants[constant] = constants.size() - 1;
		emitWithInt32(op, constants.size() - 1);
	} else {
		emitWithInt32(op, it->second);
	}
}

double Compiler::parseNumber(const Char*& p, const Char* e) {
	double v;
	const Char* q = parseDouble(p, e, v);
	if (q == p) throw std::runtime_error("Invalid number");
	p = q;
	return v;
}

bool Compiler::isLeadingIdentifierChar(Char c) {
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '$' || c == '_');
}

bool Compiler::eatToken(const Char*& p, const Char* e, const char* t, bool eatLeadingWhite) {
	assert(*t != 0);
	const Char* q = p;
	if (eatLeadingWhite) eatWhite(q, e);
	while (*t != 0 && q != e && *q == *t) { ++q; ++t; }
	if (*t == 0 && (!isLeadingIdentifierChar(t[-1]) || (q == e || !isLeadingIdentifierChar(*q)))) {
		p = q;
		return true;
	}
	return false;
}

void Compiler::eatToken(const Char*& p, const Char* e) {
	while (p != e && *p >= 'a' && *p <= 'z') ++p;
}

void Compiler::makeRValue(XValue& xv) {
	switch (xv.t) {
		default: assert(0);
		case XValue::PUSHED: return;
		case XValue::NONE: emit(VM::VOID_OP); break;
		case XValue::CONSTANT: emitWithConstant(VM::CONST_OP, xv.v); break;
		case XValue::INDEXED: emitWithInt32(VM::READ_INDEXED_OP, xv.v.toInt()); break;
		case XValue::NAMED: emitWithConstant(VM::READ_NAMED_OP, xv.v); break;
		case XValue::PROPERTY: emit(VM::GET_PROPERTY_OP); break;
	}
	xv = XValue(XValue::PUSHED);
}

void Compiler::makeAssignment(XValue& xv) {
	switch (xv.t) {
		default: throw std::runtime_error("Invalid lvalue");
		case XValue::INDEXED: emitWithInt32(VM::WRITE_INDEXED_OP, xv.v.toInt()); break;
		case XValue::NAMED: emitWithConstant(VM::WRITE_NAMED_OP, xv.v); break;
		case XValue::PROPERTY: emit(VM::SET_PROPERTY_OP); break;
	}
	xv = XValue(XValue::PUSHED);
}

void Compiler::throwAway(XValue& xv) {
	switch (xv.t) {
		case XValue::PUSHED: emit(VM::POP_OP); break;
		case XValue::NAMED: emitWithConstant(VM::READ_NAMED_OP, xv.v); emit(VM::POP_OP); break;
		case XValue::PROPERTY: emit(VM::GET_PROPERTY_OP); emit(VM::POP_OP); break;
		default: break;
	}
	xv = XValue(XValue::NONE);
}

Compiler::XValue Compiler::parseOperand(const Char*& p, const Char* e, const OperatorInfo& op) {
	XValue rv;
	parseExpression(p, e, rv, (op.closeToken != 0 ? LOWEST_PREC : op.prec));
	if (op.closeToken != 0) expectToken(p, e, op.closeToken, true);
	return rv;
}

void Compiler::parseRValueOperand(const Char*& p, const Char* e, const OperatorInfo& op) {
	XValue xv(parseOperand(p, e, op));
	makeRValue(xv);
}

void Compiler::parseFunctionCall(const Char*& p, const Char* e, XValue& v, VM::Op op) {
	eatWhite(p, e);
	Int32 count = 0;
	while (p != e && *p != ')') {
		if (count != 0) {
			expectToken(p, e, ",", false);
			eatWhite(p, e);
		}
		parseExpression(p, e, v, COMMA_PREC);
		makeRValue(v);
		eatWhite(p, e);
		++count;
	}
	expectToken(p, e, ")", false);
	emitWithInt32(op, count);
	v = XValue(XValue::PUSHED);
}

bool Compiler::parseOperator(const Char*& p, const Char* e, XValue& v, Precedence precedence
		, int opCount, const OperatorInfo* opList) {
	const Char* b = p;
	// FIX : use a quick hash solution
	int i = 0;
	while (i < opCount && !eatToken(p, e, opList[i].token, false)) ++i;
	if (i >= opCount) return false;
	const OperatorInfo& op = opList[i];
	if (precedence < op.prec + (op.assoc == RIGHT_TO_LEFT ? 0 : 1)) {
		p = b;
		return false;
	}
	// FIX : sort
	switch (op.type) {
		case POST_UNARY: {
			XValue lv(v);
			makeRValue(lv);
			emit(VM::REPUSH_OP);
			emitWithConstant(VM::CONST_OP, Value(1.0)); 
			emit(op.vmOp);
			makeAssignment(v);
			emit(VM::POP_OP);
			break;
		}
		
		case LOGICAL_AND_OR: {
			makeRValue(v);
			emit(VM::REPUSH_OP);
			Int32 point = emitForwardBranch(op.vmOp);
			emit(VM::POP_OP);
			parseRValueOperand(p, e, op);
			completeForwardBranch(point);
			break;
		}
		
		case CONDITIONAL: {
			makeRValue(v);
			Int32 pointA = emitForwardBranch(VM::JF_OP);
			parseRValueOperand(p, e, op);
			Int32 pointB = emitForwardBranch(VM::JMP_OP);
			completeForwardBranch(pointA);
			parseExpression(p, e, v, op.prec);
			makeRValue(v);
			completeForwardBranch(pointB);
			break;			
		}
		
		case PROPERTY_DOT: {
			makeRValue(v);
			eatWhite(p, e);
			const Char* b = p;
			parseIdentifier(p, e);
			if (p == b) throw std::runtime_error("Expected identifier");
			emitWithConstant(VM::CONST_OP, Value(String::newHashedString(gcHeap(), b, p)));
			v = XValue(XValue::PROPERTY);
			break;
		}
		
		case FUNCTION_CALL: {
			VM::Op op = VM::CALL_GLOBAL_OP;
			if (v.t == XValue::PROPERTY) op = VM::CALL_METHOD_OP;
			else makeRValue(v);
			parseFunctionCall(p, e, v, op);
			break;
		}
		
		case NEW_OPERATOR: {
			parseRValueOperand(p, e, op);
			if (!eatToken(p, e, "(", true)) {
				emitWithInt32(VM::NEW_OP, 0);
				v = XValue(XValue::PUSHED);
			} else {
				eatWhite(p, e);
				parseFunctionCall(p, e, v, VM::NEW_OP);
			}
			emit(VM::NEW_RESULT_OP);
			break;
		}
		
		case COMMA: {
			throwAway(v);
			v = parseOperand(p, e, op);
			makeRValue(v);
			break;
		}
		
		case DELETE_OPERATOR: {
			v = parseOperand(p, e, op);
			switch (v.t) {
				default: assert(0);
				case XValue::PUSHED: emit(VM::POP_OP); /* continue */
				case XValue::NONE:
				case XValue::CONSTANT: v = XValue(XValue::CONSTANT, Value(true)); break;
				case XValue::INDEXED: v = XValue(XValue::CONSTANT, Value(false)); break;
				case XValue::NAMED: emit(VM::FRAME_OP); emitWithConstant(VM::CONST_OP, v.v);  /* continue */
				case XValue::PROPERTY: emit(VM::DELETE_OP); v = XValue(XValue::PUSHED); break;
			}
			break;
		}
		
		case VOID_OPERATOR: {
			v = parseOperand(p, e, op);
			throwAway(v);
			break;
		}
		
		case GROUP: {
			v = parseOperand(p, e, op);
			break;
		}
		
		case UNARY: {
			v = parseOperand(p, e, op);
			if (op.vmOp == VM::ADD_OP || op.vmOp == VM::SUB_OP) {
				XValue lv(v);
				makeRValue(lv);
				emitWithConstant(VM::CONST_OP, Value(1.0)); 
				emit(op.vmOp);
				makeAssignment(v);
			} else {
				makeRValue(v);
				emit(op.vmOp);
			}
			break;
		}
		
		case BINARY: {
			makeRValue(v);
			parseRValueOperand(p, e, op);
			emit(op.vmOp);
			break;
		}
		
		case ASSIGNMENT: {
			XValue lv(v);
			if (op.vmOp != VM::INVALID_OP) {
				makeRValue(lv);
				parseRValueOperand(p, e, op);
				emit(op.vmOp);
			} else {
				parseRValueOperand(p, e, op);
			}
			makeAssignment(v);
			break;
		}
		
		case PROPERTY_BRACKETS: {
			makeRValue(v);
			parseRValueOperand(p, e, op); // FIX : convert to string const if const, or actually if we allow Value properties and optimize array[double] we shouldn't
			v = XValue(XValue::PROPERTY);
			break;
		}
		
		default: assert(0);
	}
	return true;
}

void Compiler::parseIdentifier(const Char*& p, const Char* e) {
	if (p != e && isLeadingIdentifierChar(*p)) {
		while (++p != e && (isLeadingIdentifierChar(*p) || (*p >= '0' && *p <= '9'))) { }
	}
}

enum Literal { NULL_LITERAL, FALSE_LITERAL, TRUE_LITERAL, FUNCTION_LITERAL, THIS_LITERAL };

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

void Compiler::expectToken(const Char*& p, const Char* e, const char* token, bool eatLeadingWhite) {
	if (!eatToken(p, e, token, eatLeadingWhite)) throw std::runtime_error(std::string("Expected '") + token + "'");
}

bool Compiler::parseOptionalExpression(const Char*& p, const Char* e, XValue& v, Precedence precedence) {
	eatWhite(p, e);
	if (p == e) return false;
	if (!parseOperator(p, e, v, precedence, sizeof (PRE_OPS) / sizeof (*PRE_OPS), PRE_OPS)) {
		const Char* q = p;
		eatToken(q, e);
		int i = findLiteralToken(q - p, &*p);
		if (i >= 0) {
			p = q;
			switch (static_cast<Literal>(i)) {
				case NULL_LITERAL: v = XValue(XValue::CONSTANT, Value(Value::NULL_TYPE)); break;
				case FALSE_LITERAL: v = XValue(XValue::CONSTANT, Value(false)); break;
				case TRUE_LITERAL: v = XValue(XValue::CONSTANT, Value(true)); break;
				case THIS_LITERAL: {
					emit(VM::THIS_OP);
					v = XValue(XValue::PUSHED); break;
					break;
				}
				case FUNCTION_LITERAL: {
					ScriptFunctionDef* func = gcHeap()->manage(new ScriptFunctionDef());
					// FIX : the optional function identifier
					expectToken(p, e, "(", true);
					eatWhite(p, e);
					while (p != e && *p != ')') {
						if (func->argumentsCount != 0) {
							expectToken(p, e, ",", false);
							eatWhite(p, e);
						}
						const Char* b = p;
						parseIdentifier(p, e);
						if (p == b) throw std::runtime_error("Expected identifier");
						func->locals[String::newHashedString(gcHeap(), b, p)] = func->localsCount;
						func->localsCount++;
						func->argumentsCount++;
						eatWhite(p, e);
					}
					expectToken(p, e, ")", false);
					expectToken(p, e, "{", true);
					{
						Compiler* funcCompiler = gcHeap()->manage(new Compiler(func));
						funcCompiler->parseStatementList(p, e);
						funcCompiler->finish();
					}
					expectToken(p, e, "}", false);
					emitWithConstant(VM::GEN_FUNC_OP, Value(func)); 
					v = XValue(XValue::PUSHED);
					break;
				}
				default: assert(0);
			}
		} else {
			assert(p != e);
			switch (*p) {
				case '0': {
					if (p + 1 != e && p[1] == 'x') {
						UInt32 i;
						const Char* q = parseHex(p + 2, e, i);
						if (q == p) throw std::runtime_error("Invalid number");
						p = q;
						v = XValue(XValue::CONSTANT, Value((Int32)(i)));
						break;
					}
					/* else continue */
				}
				case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '.': {
					double d = parseNumber(p, e);
					v = XValue(XValue::CONSTANT, Value(d));
					break;
				}
				case '\'': case '"': {
					size_t maxLength = unescapedMaxLength(p, e);
					QuickArray<Char, 64> buffer(maxLength);
					Char* unescapeEnd = unescape(buffer, p, e);
					assert(unescapeEnd <= buffer + maxLength);
					v = XValue(XValue::CONSTANT, Value(String::newHashedString(gcHeap(), buffer, unescapeEnd)));
					break;
				}
				default: {
					const Char* b = p;
					parseIdentifier(p, e);
					if (p == b) return false; // fix : nicer way?
					String* identifier = String::newHashedString(gcHeap(), b, p);
					Int32 index;
					LocalNameMap& nameMap = function->locals;
					LocalNameMap::const_iterator it = nameMap.find(identifier);
					if (it != nameMap.end()) {
						index = it->second;
						v = XValue(XValue::INDEXED, Value(index));
					} else {
						v = XValue(XValue::NAMED, Value(identifier));
					}
					break;
				}
			}
		}
    }
	const Char* q = p;
    eatWhite(q, e);
    while (parseOperator(q, e, v, precedence, sizeof (POST_OPS) / sizeof (*POST_OPS), POST_OPS)) {
		p = q;
		eatWhite(q, e);
	}
	return true;
}

void Compiler::parseExpression(const Char*& p, const Char* e, XValue& v, Precedence precedence) {
	if (!parseOptionalExpression(p, e, v, precedence)) throw std::runtime_error("Expected expression");
}

void Compiler::parseAndPopOptionalExpression(const Char*& p, const Char* e) {
	XValue xv;
	if (parseOptionalExpression(p, e, xv, LOWEST_PREC)) throwAway(xv);
}

void Compiler::parseVarDeclarations(const Char*& p, const Char* e) {
	while (true) {
		const Char* b = p; // FIX : sub
		parseIdentifier(p, e);
		if (p == b) throw std::runtime_error("Expected identifier");
		String* identifier = String::newHashedString(gcHeap(), b, p);
		Int32 index;
		LocalNameMap& nameMap = function->locals;
		LocalNameMap::const_iterator it = nameMap.find(identifier);
		if (it != nameMap.end()) index = it->second;
		else {
			index = function->localsCount++;
			nameMap[identifier] = index;
		}
		const Char* q = p;
		if (eatToken(q, e, "=", true)) {
			p = q;
			XValue xv;
			parseExpression(p, e, xv, COMMA_PREC); // FIX : SUB
			makeRValue(xv);
			emitWithInt32(VM::WRITE_INDEXED_OP, index);
			emit(VM::POP_OP);
			q = p;
		}
		if (!eatToken(q, e, ",", true)) break;
		p = q;
		eatWhite(p, e);
	}
}

void Compiler::expectSemicolon(const Char*& p, const Char* e) {
	const Char* q = p;
	eatWhite(q, e);
	if (q != e && *q != '}' && !eatToken(q, e, ";", false) && std::find(p, q, '\n') == q)
		throw std::runtime_error("Syntax error");
	p = q;
}

void Compiler::parseStatement(const Char*& p, const Char* e) {
	eatWhite(p, e);	
	if (eatToken(p, e, "{", false)) {
		parseStatementList(p, e);
		expectToken(p, e, "}", false);
		return;
	}
	
	const Char* q = p;
	eatToken(q, e);
	int i = findStatementToken(q - p, &*p);
	if (i < 0) {
		parseAndPopOptionalExpression(p, e);
		expectSemicolon(p, e);
	} else {
		p = q;
		eatWhite(p, e);
		switch (static_cast<Statement>(i)) {
			case VAR_STATEMENT: {
				parseVarDeclarations(p, e);
				expectSemicolon(p, e);
				break;
			}
			
			case IF_STATEMENT: {
				expectToken(p, e, "(", false);
				XValue xv; // FIX : sub
				parseExpression(p, e, xv, LOWEST_PREC); // FIX : SUB
				makeRValue(xv);
				expectToken(p, e, ")", true);
				Int32 pointA = emitForwardBranch(VM::JF_OP);
				parseStatement(p, e);
				if (eatToken(p, e, "else", true)) {
					Int32 pointB = emitForwardBranch(VM::JMP_OP);
					completeForwardBranch(pointA);
					parseStatement(p, e);
					completeForwardBranch(pointB);
				} else {
					completeForwardBranch(pointA);
				}				
				break;
			}

			case WHILE_STATEMENT: {
				Int32 pointA = markBackwardBranch();
				expectToken(p, e, "(", false);
				XValue xv; // FIX : sub
				parseExpression(p, e, xv, LOWEST_PREC); // FIX : SUB
				makeRValue(xv);
				expectToken(p, e, ")", true);
				Int32 pointB = emitForwardBranch(VM::JF_OP);
				parseStatement(p, e);
				emitBackwardBranch(VM::JMP_OP, pointA);
				completeForwardBranch(pointB);
				break;
			}

			case DO_WHILE_STATEMENT: {
				Int32 point = markBackwardBranch();
				parseStatement(p, e);
				expectToken(p, e, "while", true);
				expectToken(p, e, "(", true);
				XValue xv; // FIX : sub
				parseExpression(p, e, xv, LOWEST_PREC); // FIX : SUB
				makeRValue(xv);
				expectToken(p, e, ")", true);
				emitBackwardBranch(VM::JT_OP, point);
				expectSemicolon(p, e);
				break;
			}
			
			case FOR_STATEMENT: {
				expectToken(p, e, "(", false);
				if (eatToken(p, e, "var", true)) {
					eatWhite(p, e);
					parseVarDeclarations(p, e);
				} else {
					parseAndPopOptionalExpression(p, e);
				}
				expectToken(p, e, ";", true);
				Int32 pointA = markBackwardBranch();
				Int32 pointB = -1;
				XValue xv;
				if (parseOptionalExpression(p, e, xv, LOWEST_PREC)) {
					makeRValue(xv);
					pointB = emitForwardBranch(VM::JF_OP);
				}
				expectToken(p, e, ";", true);
				std::vector<Byte>& code = function->code;
				Int32 pointC = code.size();
				parseAndPopOptionalExpression(p, e);
				expectToken(p, e, ")", true);
				Int32 pointD = code.size();
				parseStatement(p, e);
				std::rotate(code.begin() + pointC, code.begin() + pointD, code.end());
				emitBackwardBranch(VM::JMP_OP, pointA);
				if (pointB >= 0) completeForwardBranch(pointB);
				break;
			}
			
			case RETURN_STATEMENT: {
				XValue xv;
				parseOptionalExpression(p, e, xv, LOWEST_PREC);
				makeRValue(xv);
				emit(VM::RETURN_OP);
				expectSemicolon(p, e);
				break;
			}
			
			default: assert(0);
		}
	}
}

void Compiler::parseStatementList(const Char*& p, const Char* e) {
	const Char* b;
	do {
		b = p;
		parseStatement(p, e);
	} while (p != e && b != p);
}

void Compiler::compile(const String* source) {
	const Char* p = source->accessChars();
	const Char* e = p + source->getLength();
	parseStatementList(p, e);
}

void Compiler::finish() {
	emit(VM::VOID_OP);
	emit(VM::RETURN_OP);
	// FIX : shrink code vector (copy to quickarray instead?)
disassemble(gcHeap(), function->code, function->constants);
}

Environment::Environment() : currentHeap(0), stringPrototype(0) {
	try {
		currentHeap = new GCHeap();
		stringPrototype = new UserObject();
	}
	catch (...) {
		cleanup();
		throw;
	}
}

size_t Environment::gc() {
	size_t sweptCount = 0;
	GCHeap* newHeap = 0;
	try {
		newHeap = new GCHeap();
		sweptCount = currentHeap->sweep();
		std::swap(currentHeap, newHeap);
		delete newHeap;
	}
	catch (...) {
		delete newHeap;
		throw;
	}
	return sweptCount;
}

void Environment::cleanup() {
	delete currentHeap;
	currentHeap = 0;
	delete stringPrototype;
	stringPrototype = 0;
}

Environment::~Environment() { cleanup(); }

#include <fstream>

	class PrintFunction : public NativeFunction {
		public:		typedef NativeFunction super;
		public:		virtual void call(VM* vm, Int32 argc, const Value* argv, const Value& thisValue) {
						String* s = (argc >= 1 ? argv[0].toString(gcHeap()) : &EMPTY_STRING);
						std::wcout << s->toStdString().c_str() << std::endl;
						vm->push(Value());
					}
	};


int main(int argc, const char* argv[]) {
	static const String LF_STRING("\n");
	GCHeap* newHeap = 0;
	GCHeap* heap = new GCHeap();
	Object* globals = heap->manage(new UserObject());
	static String PRINT_STRING("print");
	globals->setProperty(&PRINT_STRING, Value(heap->manage(new PrintFunction()))); // FIX : should be able to not have to new this, just like strings, but then we need the GC to be able to probe objects without assigned heap
	ScriptFunctionDef* functionDef = heap->manage(new ScriptFunctionDef());
	Compiler* compiler = heap->manage(new Compiler(functionDef));
	String* source = &EMPTY_STRING;
	std::auto_ptr<std::ifstream> inputFileStream;
	std::istream* inStream = &std::cin;
	if (argc >= 2) {
		inputFileStream.reset(new std::ifstream(argv[1]));
		inStream = inputFileStream.get();
	}
	while (inStream->good()) {
		try {
			functionDef->code.clear();
			
			std::string s;
			std::getline(*inStream, s);
			if (!s.empty()) {
				String line(s.c_str());
				source = heap->manage(new String(source, &line));
				source = heap->manage(new String(source, &LF_STRING));
			} else {
				compiler->compile(source);
				compiler->finish();
				
				std::cerr << "\tCode: " << functionDef->code.size() << std::endl;
				std::cerr << "\tConstants: " << functionDef->constants.size() << std::endl;
				std::cerr << "\tLocals: " << functionDef->localsCount << std::endl;
				
				Value vstack[1024];
				Context cstack[1024];
				VM* vm = heap->manage(new VM(cstack, cstack + 1024, vstack, vstack + 1024));
				vm->enterCall(heap->manage(new ScriptFunction(functionDef, globals)), 0, 0, Value(globals));
				bool done = false;
				
				clock_t start = ::clock();
				do {
					done = vm->run(10000000);
					Value v = vstack[0];
					
					newHeap = new GCHeap();
					newHeap->mark(compiler);
					newHeap->mark(vm);
					newHeap->mark(globals);
					newHeap->mark(functionDef);
					newHeap->mark(source);
					v.mark(newHeap);
					int sweptCount = heap->sweep();
					std::swap(heap, newHeap);
					delete newHeap;
					newHeap = 0;

					std::cerr << "\tKept: " << heap->size() << std::endl;
					std::cerr << "\tSwept: " << sweptCount << std::endl;
					
					if (done) {
						std::cerr << "\tOn stack: " << (vm->vstackPointer - vstack + 1) << std::endl;
						assert(vm->vstackPointer - vstack + 1 == 1);
						std::wcerr << L"\t=" << (v.toString(heap))->toStdString().c_str() << std::endl;
						clock_t end = ::clock();
						const double divisor = (CLOCKS_PER_SEC / 1000.0);
						std::cerr << "\ttime: " << (static_cast<double>(end - start) / divisor) << std::endl;
					}
				} while (!done);
				source = &EMPTY_STRING;
			}
		}
		catch (const std::exception& x) {
			std::cerr << "Exception: " << x.what() << std::endl;
			source = &EMPTY_STRING;
		}
		catch (...) {
			std::cerr << "Unknown exception" << std::endl;
			source = &EMPTY_STRING;
		}
	}
	return 0;
}
