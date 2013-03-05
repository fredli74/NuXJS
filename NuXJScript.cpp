// --- TODO ---
//
//	Run-time
//
//		eval support
//		count (and run-time check) necessary value stack depth for functions
//		special write with pop instructions (very common)?
//		divide by 0 checks? NaN checks?
//			is currently not compatible with -ffast_math, which is bad I guess
//			also isnan is not standard C++ :(, on Windows it is called _isnan
//			do a special Value variation that tracks NAN and Infinities?
//		native calls (and returns)
//			need easier API, shouldn't crash if missing push etc
//			no synchronized return to native code right now
//			arguments are reversed on native calls, and will be gc'd as stack has already popped 
//				arguments no longer reversed since we push forwards now, but will still be gc'd
//		prototype support
//		arguments container
//		array object (length property support, optimized for numerical indexes?)
//			take value as key for getproperty etc and we could optimize array and string accesses by not having to go via strings
//			only downside to that would be read and write named, which assumes the constant is a string pointer
//			also, how to efficiently convert between continuous array and sparse array / associative array... have QuickArray *and* Table in the object?
//		change byte code to int code and check performance, or maybe the opposite, add byte, short etc immediates?
//			not too fond of the int cast in the byte array
//		basic library (math, string, array ops)
//		assignable globals this in processor
//		only initialize Value to some BAD_TYPE in debug target, don't touch anything in release
//		quicker calls by using a frame stack for functions that do not create closures
//		could we gain something on representing bools as doubles (0 and 1) in the union? e.g. converting to/from double would be quicker
//		if we can guarantee that vsp is always at function entry level for each catch scope we should try a table-based exception handler, e.g.
//			struct TryRange { Int32 offset; Int32 length; Int32 catchIndex; String* catchName; } // catchName != 0 then create temporary "catch frame"
//			TRY_OP, TRIED_OP and CATCH_FRAME_OP would go away then...
//		if we supplied a local variable index to CatchFrame (for the exception variable) we could access it by index instead of by name in the catch scope (and in the CatchFrame property getters/setters), but I am not convinced it would make an enormous performance improvement.
//
//	Compiler
//
//		don't accept lf's after ++ etc (automatic ; insertion logic)
//		for (in)
//		switch
//		global environment compiler (don't accept return etc)
//		better errors with line numbers and stuff (should we go back to the global "pickup" idea i had in old NuXScript?)
//		function name() { } syntax
//		literal function names, e.g. myGlobalName = function myLocalName() { ... }
//		exceptions (and catching both virtual and native)
//			only thing left is integration with C++ exceptions
//		typeof operator
//		instanceof operator
//		in operator
//		new operator
//			done, but need to implement prototype
//		array literals
//		object literals
//		jump optimizations
//		pop void optimization (almost always occurs with a return;)?
//		eliminate re-thrower in try / catch / finally if no finally block (by replacing the first try in catch with jmp +0)
//		share constant pointers (strings etc) between each compiled unit, e.g. function in function will not share the same string pointers
//		try to move POP_FRAME to first in FINALLY block, need different JSR and a new RSR / JMP_INDIRECT then that pushes and pops instruction offset on value stack instead (ought to work)
//			we could go back to switches for these when we don't need to POP_FRAME_OP on CATCH_TYPE:
//			if (s->type == Scope::CATCH_TYPE || s->type == Scope::FINALLY_TYPE) emit(Processor::POP_FRAME_OP);
//		we could reuse free variable indexes in safeKeep, but it kind of feels like sub-optimization
//		don't like the outputCode solution, setting / resetting stuff manually isn't good programming style
//
//	GC
//
//		take a shot at generational gc
//			couldn't it work if we just make sure that a reference from old -> new gen protects the item in new gen until we sweep the old gen?
//		no need for recursive marking, just add to one end of the list and continue looping to the end of list
//		fast allocation pools
//		automatic gc pace
//		count actual memory allocated as in old NuXScript?
//
// --- DONE ---
//
//		for (;;)
//		comma operator
//		void operator
//		delete operator
//		proper string comparisons (< etc)
//		also probe gcitems with gcheap == 0, so that we can have those safely on stack etc
//			thinking of dropping mark etc from gcheap and have in engine instead, we could then have a gcheap list for protected items (probe these before general mark phase)
//		break, continue etc (with labels!)
//		virtual machine should include gc etc
//		check that identifier is never one of the reserved words
//		drop the backwards jump for continue in while(), can jump down to the loop instead, for code simplicity (some day we'll do jump optimizations anyhow)
//		maybe drop the scope vector and just put it all on the program stack

#include "assert.h"
#include <cmath>
#include <ctime>
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
static bool isNaN(double d) { return d != d; }

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
	Char endChar = *p;
	const Char* b = ++p;
	Char* d = buffer;
	while (p < e && *p != endChar && *p != '\r' && *p != '\n') { // FIX : use std::find_any
		if (*p == '\\' && p + 1 < e) {
			d = std::copy(b, p, d);
			const Char* f = std::find(ESCAPE_CHARS, ESCAPE_CHARS + ESCAPE_CODE_COUNT, *++p);
			b = p;
			UInt32 l;
			if (f != ESCAPE_CHARS + ESCAPE_CODE_COUNT) { ++p; l = ESCAPE_CODES[f - ESCAPE_CHARS]; }
			else if (*p == 'x') { b = ++p; p = parseHex(p, (e - p > 2 ? p + 2 : e), l); } // FIX : in ecma, you *need* two (and four) hex chars, nothing else, throw here!
			else if (*p == 'u') { b = ++p; p = parseHex(p, (e - p > 4 ? p + 4 : e), l); }
			if (p == b) throw std::runtime_error("Illegal escape character");
			b = p;
			*d++ = Char(l);
		} else ++p;
	}
	if (p >= e || *p != endChar) throw std::runtime_error("Unterminated string");
	return std::copy(b, p++, d);
}

static bool isComment(const Char* p, const Char* e) {
	return (p != e && p + 1 != e && *p == '/' && (p[1] == '/' || p[1] == '*'));
}

/* --- String --- */

String::String(const char* sz) : QArray(strlen(sz)), hash(0) { std::copy(sz, sz + size(), begin()); calcHash(); }

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
			ascii[i - 32] = Value(Value::NONEXISTENT_TYPE);
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

String::String() : QArray(0), hash(2166136261U) { assert(isEqualTo(EMPTY_STRING)); }
String::String(const Char* b, const Char* e) : QArray(e - b), hash(0) { std::copy(b, e, begin()); }
String::String(const String& o) : QArray(o), hash(o.hash) { }

String::String(const String& l, const String& r) : QArray(l.size() + r.size()), hash(0) {
	Char* e = std::copy(l.begin(), l.end(), begin());
	std::copy(r.begin(), r.end(), e);
}

String* String::newHashedString(GCHeap& heap, const Char* b, const Char* e) {
	String* o = heap.manage(new String(b, e));
	o->calcHash();
	return o;
}

bool String::getProperty(Engine& engine, const String* name, Value& v) {
	if (name->isEqualTo(LENGTH_STRING)) {
		v = Value(static_cast<Int32>(size()));
		return true;
	}
	UInt32 i;
	const Char* q = parseUnsignedInt(name->begin(), name->end(), i);
	if (q == name->end() && i < size()) {
		const Char* p = begin() + i;
		if (32 <= *p && *p < 127) v = QUICK_CONSTANTS.ascii[*p - 32];
		else v = Value(engine.heap().manage(new String(p, p + 1)));
		return true;
	}
	v = Value(Value::UNDEFINED_TYPE);
	return false;
}

// FIX : speed up by skipping when string is really long
// FNV Hash
Table::HashType String::calcHash() const {
	if (hash == 0) {
		size_t l = size();
		UInt32 x = 2166136261U;
		const Char* p = begin();
		for (size_t i = 0; i < l; ++i) {
			x *= 16777619;
			x += p[i];
		}
		hash = (x == 0 ? x + 1 : x);
	}
	return hash;
}

bool String::isEqualTo(const String& o) const {
	if (this == &o) return true;
	if (hash != o.hash && hash != 0 && o.hash != 0) return false;
	size_t l = size();
	if (l != o.size()) return false;
	return std::equal(begin(), end(), o.begin());
}

bool String::isLessThan(const String& o) const {
	if (this == &o) return false;
	size_t l = std::min(size(), o.size());
	const Char* a = begin();
	const Char* b = o.begin();
	for (int i = 0; i < l; ++i) if (a[i] != b[i]) return (a[i] < b[i]);
	return size() < o.size();
}

std::wstring String::toStdString() const { return std::wstring(begin(), end()); }

/* --- GCItem --- */

GCItem::GCItem() : _gcList(0), _gcPrev(this), _gcNext(this) { }
void GCItem::gcMark(GCHeap& heap) { }
GCItem::~GCItem() { if (_gcList != 0) _gcList->relinquish(this); }

/* --- GCList --- */

GCList::GCList() : count(0) { }

size_t GCList::size() const { return count; }

void GCList::relinquish(GCItem* item) {
	assert(item->_gcList == this);
	assert(count > 0);
	count--;
	item->_gcPrev->_gcNext = item->_gcNext;
	item->_gcNext->_gcPrev = item->_gcPrev;
}

void GCList::claim(GCItem* item) {
	if (item->_gcList != 0) item->_gcList->relinquish(item);
	item->_gcPrev = this;
	item->_gcNext = _gcNext;
	_gcNext->_gcPrev = item;
	_gcNext = item;
	item->_gcList = this;
	++count;
}

/* --- Table --- */

Table::Bucket::Bucket() : key(0), value(Value::NONEXISTENT_TYPE) { }

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
	while (++b != e && !b->value.exists()) { }
	return (b == e ? 0 : b);
}

const Table::Bucket* Table::lookup(const String* key) {
	const Bucket* b = find(key, key->calcHash());
	return (b->key != 0 ? b : 0);
}

void Table::assign(const String* key, const Value& value) { assert(value.exists()); update(key, value); }
void Table::erase(const String* key) { update(key, Value(Value::NONEXISTENT_TYPE)); }

void Table::mark(GCHeap& heap) {
	UInt32 rebuildLoadCount = 0;
	for (UInt32 i = 0; i < buckets.size(); ++i) {
		heap.mark(buckets[i].key);
		buckets[i].value.mark(heap);
		if (buckets[i].value.exists()) rebuildLoadCount++;
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
	while (buckets[i].key != 0 && (buckets[i].hash != hash || !buckets[i].key->isEqualTo(*key))) i = (i + 1) & mask;
	return &buckets[i];
}

UInt32 Table::rebuild(UInt32 newN) {
	QuickArray<Bucket, 1U << TABLE_BUILT_IN_N> newBuckets(1U << newN);
	UInt32 newLoadCount = 0;
	UInt32 mask = (1U << newN) - 1;
	for (UInt32 i = 0; i < buckets.size(); ++i) {
		if (buckets[i].value.exists()) {
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

bool UserObject::setProperty(Engine& engine, const String* name, const Value& v) {
	properties.assign(name, v);
	return true;
}

bool UserObject::getProperty(Engine& engine, const String* name, Value& v) {
	const Table::Bucket* b = properties.lookup(name);
	if (b != 0) {
		v = b->value;
		return true;
	}
	v = Value(Value::UNDEFINED_TYPE);
	return false;
}

bool UserObject::deleteProperty(Engine& engine, const String* name) {
	properties.erase(name);
	return true;
}

void UserObject::gcMark(GCHeap& heap) {
	properties.mark(heap);
	super::gcMark(heap);
}

Value::Value(Type type) : type(type) { }
Value::Value(bool b) : type(BOOLEAN_TYPE), b(b) { }
Value::Value(Int32 i) : type(NUMBER_TYPE), d(i) { }
Value::Value(double d) : type(NUMBER_TYPE), d(d) { }
Value::Value(String* s) : type(STRING_TYPE), s(s) { }
Value::Value(Object* o) : type(OBJECT_TYPE), o(o) { }

bool Value::exists() const { return type != NONEXISTENT_TYPE; }
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
			const Char* p = s->begin();
			const Char* q = parseDouble(p, s->end(), v);
			if (q == p) return NaN();
			return v;
		}
	}
}

String* Value::toString(GCHeap& heap) const {
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
				else return heap.manage(new String(intToString(buffer, i), buffer + 32));
			}
			if (isNaN(d)) return &NAN_STRING;
			double x = fabs(d);
			if (x <= EPSILON) return &ZERO_STRING;
			if (x == std::numeric_limits<double>::infinity()) return d < 0 ? &MINUS_INFINITY_STRING : &INFINITY_STRING;
			std::pair<Char*, Char*> r = doubleToString(buffer, d);
			return heap.manage(new String(r.first, r.second));
		}
		case STRING_TYPE: return s;
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
		case NONEXISTENT_TYPE: return true;
		case BOOLEAN_TYPE: return b == y.b;
		case NUMBER_TYPE: return d == y.d;
		case STRING_TYPE: return s->isEqualTo(*y.s);
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
		case NONEXISTENT_TYPE: return false;
		case BOOLEAN_TYPE: return b < y.b;
		case NUMBER_TYPE: return d < y.d;
		case STRING_TYPE: return s->isLessThan(*y.s);
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
			return thisNumber == otherNumber;
		}
		case STRING_TYPE: return s->isEqualTo(*y.s);
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
	if (type == STRING_TYPE && y.type == STRING_TYPE) return s->isLessThan(*y.s);
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
	if (type == STRING_TYPE && y.type == STRING_TYPE) return !y.s->isLessThan(*s);
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
		String* ls = toString(heap);
		String* rs = y.toString(heap);
		if (rs->empty()) return Value(ls);
		if (ls->empty()) return Value(rs);
		return Value(heap.manage(new String(*ls, *rs)));
	}
	return Value(toDouble() + y.toDouble());
}
				
void Value::mark(GCHeap& heap) const {
	switch (type) {
		case STRING_TYPE: heap.mark(s); break;
		case OBJECT_TYPE: heap.mark(o); break;
		default: break;
	}
}

/* --- ScriptFunctionDef --- */

ScriptFunctionDef::ScriptFunctionDef() : source(0), argumentsCount(0), localsCount(0) { }

bool ScriptFunctionDef::lookupLocalName(const String* name, Int32& index) const {
	LocalNameMap::const_iterator it = localNames.find(name);
	if (it == localNames.end()) return false;
	index = it->second;
	return true;
}

void ScriptFunctionDef::gcMark(GCHeap& heap) {
	heap.mark(source);
	// FIX : subs
	for (std::vector<Value>::iterator it = constants.begin(), e = constants.end(); it != e; ++it) {
		it->mark(heap);
	}
	for (LocalNameMap::iterator it = localNames.begin(), e = localNames.end(); it != e; ++it) {
		heap.mark(it->first);
	}
	super::gcMark(heap);
}

const Value* ScriptFunctionDef::getConstantsPointer() const { return (constants.empty() ? 0 : &constants[0]); }

/* --- Function --- */

Function* Function::asFunction() { return this; }

void Function::construct(GCHeap& heap, Processor* processor, Int32 argc, const Value* argv) {
	Object* newObject = heap.manage(new UserObject());
	Value thisValue(newObject);
	processor->push(thisValue);
	call(heap, processor, argc, argv, thisValue);
}

/* --- ScriptFunction --- */

ScriptFunction::ScriptFunction(ScriptFunctionDef* definition, Object* environment) : definition(definition), environment(environment) { }
ScriptFunctionDef* ScriptFunction::getDefinition() const { return definition; }
Object* ScriptFunction::getEnvironment() const { return environment; }

void ScriptFunction::call(GCHeap& heap, Processor* processor, Int32 argc, const Value* argv, const Value& thisValue) {
	processor->enterCall(this, argc, argv, thisValue);
}

void ScriptFunction::gcMark(GCHeap& heap) {
	heap.mark(definition);
	heap.mark(environment);
	super::gcMark(heap);
}

#if 0
/* --- ArgumentsObject --- */

ArgumentsObject(FunctionFrame* frame) : frame(frame) { }

bool ArgumentsObject::setProperty(Engine& engine, const String* name, Value& v) {
	if (name->s == "length") return 
}

void ArgumentsObject::gcMark(GCHeap& heap) {
	heap.mark(frame);
	super::gcMark(heap);
}
#endif

/* --- FunctionFrame --- */

FunctionFrame::FunctionFrame(const ScriptFunction* function) : environment(function->getEnvironment())
		, functionDef(function->getDefinition()), locals(std::max(functionDef->localsCount, 1)) { }

// TODO: solution for injecting dynamic bound variables, e.g. from running eval(), could secretely add a environment above maybe?

bool FunctionFrame::getProperty(Engine& engine, const String* name, Value& v) {
	Int32 index;
	if (functionDef->lookupLocalName(name, index)) {
		assert(0 <= index && index < locals.size());
		v = locals[index];
		return true;
	} else {
		return environment->getProperty(engine, name, v); // FIX: can we do a looping non-recursive solution? faster
	}
}

bool FunctionFrame::setProperty(Engine& engine, const String* name, const Value& v) {
	Int32 index;
	if (functionDef->lookupLocalName(name, index)) {
		assert(0 <= index && index < locals.size());
		locals[index] = v;
		return true;
	} else {
		return environment->setProperty(engine, name, v);
	}
}

bool FunctionFrame::deleteProperty(Engine& engine, const String* name) {
	Int32 index;
	if (functionDef->lookupLocalName(name, index)) return false;
	else return environment->deleteProperty(engine, name);
}

void FunctionFrame::gcMark(GCHeap& heap) {
	heap.mark(functionDef);
	heap.mark(environment);
	// FIX : subs
	for (Int32 i = 0; i < locals.size(); ++i) locals[i].mark(heap);
	super::gcMark(heap);
}

/* --- CatchFrame --- */

CatchFrame::CatchFrame(Frame* parentFrame, const String* exceptionName, const Value& exceptionValue)
		: parentFrame(parentFrame), exceptionName(exceptionName), exceptionValue(exceptionValue) { }

void CatchFrame::gcMark(GCHeap& heap) {
	heap.mark(parentFrame);
	heap.mark(exceptionName);
	exceptionValue.mark(heap);
	super::gcMark(heap);
}

bool CatchFrame::getProperty(Engine& engine, const String* name, Value& v) {
	if (name->isEqualTo(*exceptionName)) { v = exceptionValue; return true; }
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

/* --- Processor --- */

const int OPERAND_SIZE = (sizeof (Int32) / sizeof (Byte));

Processor::Processor(Engine& engine, Context* cstackStart, Context* cstackEnd, Value* vstackStart, Value* vstackEnd)
		: engine(engine), cstackStart(cstackStart), cstackEnd(cstackEnd), vstackStart(vstackStart), vstackEnd(vstackEnd)
		, cstackPointer(cstackStart - 1), vstackPointer(vstackStart - 1) {
}

void Processor::gcMark(GCHeap& heap) {
	for (Value* vsp = vstackStart; vsp != vstackPointer + 1; ++vsp) vsp->mark(heap);
	for (Context* csp = cstackStart; csp != cstackPointer + 1; ++csp) heap.mark(csp->frame);
	super::gcMark(heap);
}

void Processor::push(Value*& vsp, const Value& v) { assert(vsp + 1 < vstackEnd); *++vsp = v; }
void Processor::push(const Value& v) { push(vstackPointer, v); }
void Processor::pop(Value*& vsp, int count) { assert(vsp - count + 1 >= vstackStart); vsp -= count; }
void Processor::poppush(Value*& vsp, const Value& v) { assert(vsp >= vstackStart); *--vsp = v; }
Int32 Processor::readInt32(const Byte* ip) { return *reinterpret_cast<const Int32*>(ip); }
Int32 Processor::nextInt32(const Byte*& ip) { Int32 v = *reinterpret_cast<const Int32*>(ip); ip += OPERAND_SIZE; return v; }

void Processor::error() {
	throw std::runtime_error("error"); // FIX
}

void Processor::enterCall(ScriptFunction* func, Int32 argc, const Value* argv, const Value& thisValue) {
	ScriptFunctionDef* def = func->getDefinition();
	FunctionFrame* frame = engine.heap().manage(new FunctionFrame(func));
	++cstackPointer;
	cstackPointer->frame = frame;
	cstackPointer->thisValue = thisValue;
	cstackPointer->instructionPointer = &def->code[0];
	cstackPointer->vstackPointer = vstackPointer;  // FIX : i think ->vstackPointer here will go, at least if we keep the "catch stack" solution
	Value* lp = frame->getLocalsPointer();
	std::fill(std::copy(argv, argv + std::min(argc, def->argumentsCount), lp), lp + def->argumentsCount, Value(Value::UNDEFINED_TYPE));
}

bool Processor::run(Int32 count) {
	Value* vsp = vstackPointer;
	const Byte* ip = cstackPointer->instructionPointer;
	Frame* frame = cstackPointer->frame;
	const Value* cp = frame->getFunctionDef()->getConstantsPointer();
	Value* lp = frame->getLocalsPointer();
	GCHeap& heap = engine.heap();
	while (--count >= 0) {
		switch (*ip++) {
			default: assert(0);
			case POP_OP: pop(vsp, 1); break;
			case REPUSH_OP: push(vsp, vsp[0]); break;
			case CONST_OP: push(vsp, cp[nextInt32(ip)]); break;
			case READ_INDEXED_OP: push(vsp, lp[nextInt32(ip)]); break;
			case WRITE_INDEXED_OP: lp[nextInt32(ip)] = vsp[0]; break;
			case READ_NAMED_OP: {
				Value v(Value::NONEXISTENT_TYPE);
				bool success = frame->getProperty(engine, cp[nextInt32(ip)].getString(), v);
				if (!success) error();
				push(vsp, v);
				break;
			}
			case WRITE_NAMED_OP:  {
				bool success = frame->setProperty(engine, cp[nextInt32(ip)].getString(), vsp[0]);
				if (!success) error();
				break;
			}
			case GET_PROPERTY_OP: {
				Object* o = vsp[-1].toObject(heap);
				if (o == 0) error();
				o->getProperty(engine, vsp[0].toString(heap), vsp[-1]);
				pop(vsp, 1);
				break;
			}
			case SET_PROPERTY_OP: {
				Object* o = vsp[-2].toObject(heap);
				if (o == 0) error();
				o->setProperty(engine, vsp[-1].toString(heap), vsp[0]);
				vsp[-2] = vsp[0];
				pop(vsp, 2);
				break;
			}
			case ADD_OP: poppush(vsp, vsp[-1].add(heap, vsp[0])); break;
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
			case JMP_OP: ip += readInt32(ip) + OPERAND_SIZE; break;
			case JSR_OP: {
				Int32 offset = nextInt32(ip);
				cstackPointer->instructionPointer = ip;
				++cstackPointer;
				cstackPointer->frame = frame;
				cstackPointer->thisValue = (cstackPointer - 1)->thisValue;
				cstackPointer->vstackPointer = vstackPointer;  // FIX : i think ->vstackPointer here will go, at least if we keep the "catch stack" solution
				ip += offset;
				break;
			}
			case JT_OP: ip += (vsp[0].toBool() ? readInt32(ip) + OPERAND_SIZE : OPERAND_SIZE); pop(vsp, 1); break;
			case JF_OP: ip += (!vsp[0].toBool() ? readInt32(ip) + OPERAND_SIZE : OPERAND_SIZE); pop(vsp, 1); break;
			case RETURN_OP: {
				if (cstackPointer == cstackStart) {
					// FIX : yeah, we can't you know :) we need a new way to return properly to native caller
					// cstackPointer->instructionPointer = ip;
					vstackPointer = vsp;
					return true;
				}
				assert(frame == (cstackPointer - 1)->frame || vsp == cstackPointer->vstackPointer + 1); // Fail here and we have an unbalanced stack.
				--cstackPointer;
				ip = cstackPointer->instructionPointer;
				frame = cstackPointer->frame;
				cp = frame->getFunctionDef()->getConstantsPointer();
				lp = frame->getLocalsPointer();
				break;
			}
			case CALL_METHOD_OP: { // FIX : share more with the others...
				Int32 n = nextInt32(ip);
				Object* thisObject = vsp[-n - 1].toObject(heap);
				if (thisObject == 0) error();
				Value v(Value::NONEXISTENT_TYPE);
				thisObject->getProperty(engine, vsp[-n].toString(heap), v);
				Object* o = v.toObject(heap);
				if (o == 0) error();
				Function* f = o->asFunction();
				if (f == 0) error();
				cstackPointer->instructionPointer = ip;
				vstackPointer = vsp - n - 2;
				f->call(heap, this, n, vsp - n + 1, Value(thisObject));
				vsp = vstackPointer;
				ip = cstackPointer->instructionPointer;
				frame = cstackPointer->frame;
				cp = frame->getFunctionDef()->getConstantsPointer();
				lp = frame->getLocalsPointer();
				break;
			}
			case CALL_GLOBAL_OP: {
				Int32 n = nextInt32(ip);
				Object* o = vsp[-n].toObject(heap);
				if (o == 0) error();
				Function* f = o->asFunction();
				if (f == 0) error();
				cstackPointer->instructionPointer = ip;
				vstackPointer = vsp - n - 1;
				f->call(heap, this, n, vsp - n + 1, Value(Value::UNDEFINED_TYPE));
				vsp = vstackPointer;
				ip = cstackPointer->instructionPointer;
				frame = cstackPointer->frame;
				cp = frame->getFunctionDef()->getConstantsPointer();
				lp = frame->getLocalsPointer();
				break;
			}
			case NEW_OP: {
				Int32 n = nextInt32(ip);
				Object* o = vsp[-n].toObject(heap);
				if (o == 0) error();
				Function* f = o->asFunction();
				if (f == 0) error();
				cstackPointer->instructionPointer = ip;
				vstackPointer = vsp - n - 1;
				f->construct(heap, this, n, vsp - n + 1);
				vsp = vstackPointer;
				ip = cstackPointer->instructionPointer;
				frame = cstackPointer->frame;
				cp = frame->getFunctionDef()->getConstantsPointer();
				lp = frame->getLocalsPointer();
				break;
			}
			case NEW_RESULT_OP: poppush(vsp, (vsp[0].isObject() ? vsp[0] : vsp[-1])); break;
			case THIS_OP: push(vsp, cstackPointer->thisValue); break;
			case VOID_OP: push(vsp, Value(Value::UNDEFINED_TYPE)); break;
			case GEN_FUNC_OP: {
				push(vsp, Value(heap.manage(new ScriptFunction(cp[nextInt32(ip)].getFunctionDef(), frame))));
				break;
			}
			case FRAME_OP: push(vsp, Value(frame)); break;
			case DELETE_OP: {
				Object* o = vsp[-1].toObject(heap);
				if (o == 0) error();
				poppush(vsp, Value(o->deleteProperty(engine, vsp[0].toString(heap))));
				break;
			}
			case POP_FRAME_OP: {
				assert(cstackPointer != cstackStart);
				--cstackPointer;
				frame = cstackPointer->frame;
				assert(cp == frame->getFunctionDef()->getConstantsPointer());
				assert(lp == frame->getLocalsPointer());
				break;
			}
			case TRY_OP: {
				ip += OPERAND_SIZE;
				catchers.push_back(Catcher(cstackPointer, vsp, ip + readInt32(ip - OPERAND_SIZE)));
				break;
			}
			case TRIED_OP: {
				catchers.pop_back();
				break;
			}
			case CATCH_FRAME_OP: {
				++cstackPointer;
				frame = cstackPointer->frame = heap.manage(new CatchFrame(frame, cp[nextInt32(ip)].getString(), vsp[0]));
				pop(vsp, 1);
				cstackPointer->thisValue = (cstackPointer - 1)->thisValue;
				cstackPointer->vstackPointer = vstackPointer; // FIX : i think ->vstackPointer here will go, at least if we keep the "catch stack" solution
				break;
			}
			case THROW_OP: {
				const Value& exception = vsp[0];
				if (catchers.empty()) {
					std::wcout << "Unhandled exception: " << (exception.toString(heap))->toStdString().c_str() << std::endl;
					error();
				}
				const Catcher& catcher = catchers.back();
				cstackPointer = catcher.cstackPointer;
				vsp = catcher.vstackPointer;
				push(vsp, exception);
				ip = catcher.instructionPointer;
				frame = cstackPointer->frame;
				cp = frame->getFunctionDef()->getConstantsPointer();
				lp = frame->getLocalsPointer();
				catchers.pop_back();
				break;
			}
		}
	}
	cstackPointer->instructionPointer = ip;
	vstackPointer = vsp;
	return false;
}

static Int32 getInt32(std::vector<Byte>::const_iterator& it, const std::vector<Byte>::const_iterator& e) {
	assert(e - it >= OPERAND_SIZE);
	Int32 i = *reinterpret_cast<const Int32*>(&*it);
	it += OPERAND_SIZE;
	return i;
}

static void disassemble(GCHeap& heap, const std::vector<Byte>& code, const std::vector<Value>& constants) {
	const char* OP_STRINGS[] = {
		"!!! INVALID !!!"
		, "POP", "REPUSH", "CONST", "READ_INDEXED", "WRITE_INDEXED", "READ_NAMED", "WRITE_NAMED", "GET_PROPERTY", "SET_PROPERTY"
		, "ADD", "SUB", "MUL", "DIV", "MOD"
		, "OR", "XOR", "AND"
		, "SHL", "SHR", "UNSIGNED_SHR"
		, "PLUS", "MINUS"
		, "BIT_NOT", "NOT"
		, "STRICTLY_EQ", "STRICTLY_NEQ", "EQ", "NEQ", "LT", "LEQ", "GT", "GEQ"
		, "JMP", "JSR", "JT", "JF"
		, "CALL_GLOBAL", "CALL_METHOD", "NEW", "NEW_RESULT"
		, "RETURN", "THIS", "VOID", "FRAME", "DELETE"
		, "GEN_FUNC"
		, "THROW", "TRY", "TRIED", "CATCH_FRAME", "POP_FRAME"
	};
	std::vector<Byte>::const_iterator b = code.begin();
	std::vector<Byte>::const_iterator it = b;
	std::vector<Byte>::const_iterator e = code.end();
	while (it != e) {
		Processor::Opcode opcode = static_cast<Processor::Opcode>(*it);
		if (opcode < 0 || opcode >= sizeof (OP_STRINGS) / sizeof (*OP_STRINGS)) opcode = Processor::INVALID_OP;
		std::cerr << "\t" << (it - b) << ':' << OP_STRINGS[opcode];
		++it;
		switch (opcode) {
			case Processor::JT_OP:
			case Processor::JF_OP:
			case Processor::JMP_OP:
			case Processor::JSR_OP:
			case Processor::TRY_OP: std::cerr << " @" << ((it - b) + getInt32(it, e) + OPERAND_SIZE); break;
			case Processor::READ_INDEXED_OP:
			case Processor::WRITE_INDEXED_OP: std::cerr << " $" << getInt32(it, e); break;
			case Processor::CALL_GLOBAL_OP:
			case Processor::CALL_METHOD_OP:
			case Processor::NEW_OP: std::cerr << " *" << getInt32(it, e); break;
			case Processor::CONST_OP:
			case Processor::GEN_FUNC_OP:
			case Processor::CATCH_FRAME_OP: std::wcerr << L" #" << constants[getInt32(it, e)].toString(heap)->toStdString(); break;
			case Processor::READ_NAMED_OP:
			case Processor::WRITE_NAMED_OP: std::wcerr << L" " << constants[getInt32(it, e)].getString()->toStdString(); break;
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
	Processor::Opcode vmOp;
};

// FIX : sort these two
const OperatorInfo PRE_OPS[] = {
	{ "(", ")", GROUP_PREC, RIGHT_TO_LEFT, GROUP, Processor::INVALID_OP },
	{ "++", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::ADD_OP },
	{ "--", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::SUB_OP },
	{ "+", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::PLUS_OP },
	{ "+", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::PLUS_OP },
	{ "-", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::MINUS_OP },
	{ "~", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::BIT_NOT_OP },
	{ "!", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, Processor::NOT_OP },
	{ "void", 0, PREFIX_PREC, RIGHT_TO_LEFT, VOID_OPERATOR, Processor::VOID_OP },
	{ "delete", 0, PREFIX_PREC, RIGHT_TO_LEFT, DELETE_OPERATOR, Processor::VOID_OP },
	{ "new", 0, NEW_PREC, RIGHT_TO_LEFT, NEW_OPERATOR, Processor::VOID_OP }
};

const OperatorInfo POST_OPS[] = {
	{ "===", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, Processor::STRICT_EQ_OP },
	{ "==", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, Processor::EQ_OP },
	{ "=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::INVALID_OP },
	{ ">>>=", 0, SHIFT_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::UNSIGNED_SHR_OP },
	{ ">>=", 0, SHIFT_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::SHR_OP },
	{ "<<=", 0, SHIFT_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::SHL_OP },
	{ "+=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::ADD_OP },
	{ "-=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::SUB_OP },
	{ "*=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::MUL_OP },
	{ "/=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::DIV_OP },
	{ "%=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::MOD_OP },
	{ "&=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::AND_OP },
	{ "^=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::XOR_OP },
	{ "|=", 0, ASSIGN_PREC, RIGHT_TO_LEFT, ASSIGNMENT, Processor::OR_OP },
	{ ">>>", 0, SHIFT_PREC, LEFT_TO_RIGHT, BINARY, Processor::UNSIGNED_SHR_OP },
	{ ">>", 0, SHIFT_PREC, LEFT_TO_RIGHT, BINARY, Processor::SHR_OP },
	{ "<<", 0, SHIFT_PREC, LEFT_TO_RIGHT, BINARY, Processor::SHL_OP },
	{ "++", 0, POST_INC_DEC, LEFT_TO_RIGHT, POST_UNARY, Processor::ADD_OP },
	{ "--", 0, POST_INC_DEC, LEFT_TO_RIGHT, POST_UNARY, Processor::SUB_OP },
	{ "&&", 0, LOGICAL_AND_PREC, LEFT_TO_RIGHT, LOGICAL_AND_OR, Processor::JF_OP },
	{ "||", 0, LOGICAL_OR_PREC, LEFT_TO_RIGHT, LOGICAL_AND_OR, Processor::JT_OP },
	{ "+", 0, ADD_SUB_PREC, LEFT_TO_RIGHT, BINARY, Processor::ADD_OP },
	{ "-", 0, ADD_SUB_PREC, LEFT_TO_RIGHT, BINARY, Processor::SUB_OP },
	{ "*", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, Processor::MUL_OP },
	{ "/", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, Processor::DIV_OP },
	{ "%", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, Processor::MOD_OP },
	{ "&", 0, BIT_AND_PREC, LEFT_TO_RIGHT, BINARY, Processor::AND_OP },
	{ "^", 0, BIT_XOR_PREC, LEFT_TO_RIGHT, BINARY, Processor::XOR_OP },
	{ "|", 0, BIT_OR_PREC, LEFT_TO_RIGHT, BINARY, Processor::OR_OP },
	{ "!==", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, Processor::STRICT_NEQ_OP },
	{ "!=", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, Processor::NEQ_OP },
	{ "<=", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::LEQ_OP }, 
	{ "<", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::LT_OP }, 
	{ ">=", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::GEQ_OP }, 
	{ ">", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, Processor::GT_OP },
	{ ".", 0, PROPERTY_PREC, LEFT_TO_RIGHT, PROPERTY_DOT, Processor::INVALID_OP },
	{ "[", "]", PROPERTY_PREC, LEFT_TO_RIGHT, PROPERTY_BRACKETS, Processor::INVALID_OP },
	{ "?", ":", CONDITIONAL_PREC, RIGHT_TO_LEFT, CONDITIONAL, Processor::INVALID_OP },
	{ "(", ")", FUNCTION_CALL_PREC, LEFT_TO_RIGHT, FUNCTION_CALL, Processor::INVALID_OP },
	{ ",", 0, COMMA_PREC, LEFT_TO_RIGHT, COMMA, Processor::INVALID_OP }
};

// FIX : sort
enum Statement {
	VAR_STATEMENT, IF_STATEMENT, WHILE_STATEMENT, DO_WHILE_STATEMENT, FOR_STATEMENT, RETURN_STATEMENT
	, CONTINUE_STATEMENT, BREAK_STATEMENT, TRY_STATEMENT, THROW_STATEMENT, SWITCH_STATEMENT
};
/* Built with QuickHashMaker.pika */
static int findStatementToken(int n /* string length */, const Char* s) {
	static const char* STRINGS[11] = {
		"var", "if", "while", "do", "for", "return", "continue", "break", "try", 
		"throw", "switch"
	};
	static const int QUICK_HASH_TABLE[32] = {
		-1, -1, -1, 7, -1, 6, -1, -1, -1, -1, -1, -1, -1, 0, 2, 8, 
		-1, 9, -1, 10, 5, -1, -1, -1, -1, 1, -1, -1, -1, 4, 3, -1
	};
	if (n < 2 || n > 8) return -1;
	int stringIndex = QUICK_HASH_TABLE[(n - s[0]) & 31];
	return (stringIndex >= 0 && strncmp(s, STRINGS[stringIndex], n) == 0 && STRINGS[stringIndex][n] == 0) ? stringIndex : -1;
};

struct Compiler::XValue {
	enum Type { NONE, PUSHED, CONSTANT, INDEXED, NAMED, PROPERTY };
	XValue() : t(NONE), v(Value::UNDEFINED_TYPE) { }
	XValue(Type type, const Value& value = Value()) : t(type), v(value) { }
	Type t;
	Value v;
};

Compiler::Compiler(Engine& engine, ScriptFunctionDef* function) : engine(engine), function(function), outputCode(&function->code) { }

void Compiler::gcMark(GCHeap& heap) {
	heap.mark(function);
	for (std::map<Value, Int32>::iterator it = definedConstants.begin(); it != definedConstants.end(); ++it) {
		it->first.mark(heap);
	}
	super::gcMark(heap);
}

void Compiler::eatComment() {
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

void Compiler::eatWhite() {
	while (p != e) {
		switch (*p) {
			case ' ': case '\t': ++p; break;
			case '\r': if (p + 1 != e && p[1] == '\n') ++p;
			case '\n': {																								// Eat leading .. on a new line
				while (++p != e && (*p == ' ' || *p == '\t')) { }
				if (e - p >= 2 && p[0] == '.' && p[1] == '.') p += 2;
				break;
			}
			case '/': if (isComment(p, e)) { eatComment(); break; }
			/* else continue */
			default: return;
		}
	}
}

void Compiler::emit(Processor::Opcode op) {
	outputCode->push_back(op);
}

void Compiler::emitWithInt32(Processor::Opcode op, Int32 i) {
	outputCode->push_back(op);
	outputCode->resize(outputCode->size() + OPERAND_SIZE);
	*reinterpret_cast<Int32*>(&(*outputCode)[outputCode->size() - OPERAND_SIZE]) = i;
}

Int32 Compiler::emitForwardBranch(Processor::Opcode op) {
	outputCode->push_back(op);
	Int32 point = outputCode->size();
	outputCode->resize(outputCode->size() + OPERAND_SIZE);
	return point;
}

void Compiler::completeForwardBranch(Int32 point) {
	*reinterpret_cast<Int32*>(&(*outputCode)[point]) = outputCode->size() - point - OPERAND_SIZE;
}

Int32 Compiler::markBackwardBranch() { return outputCode->size(); }

void Compiler::emitBackwardBranch(Processor::Opcode op, Int32 point) {
	emitWithInt32(op, point - (outputCode->size() + 5));
}

void Compiler::emitWithConstant(Processor::Opcode op, const Value& constant) {
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

bool Compiler::isLeadingIdentifierChar(Char c) {
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '$' || c == '_');
}

bool Compiler::eatToken(const char* t, bool eatLeadingWhite) {
	assert(*t != 0);
	const Char* b = p;
	if (eatLeadingWhite) eatWhite();
	while (*t != 0 && p != e && *p == *t) { ++p; ++t; }
	if (*t == 0 && (!isLeadingIdentifierChar(t[-1]) || (p == e || !isLeadingIdentifierChar(*p)))) {
		return true;
	} else {
		p = b;
		return false;
	}
}

void Compiler::eatToken() {
	while (p != e && *p >= 'a' && *p <= 'z') ++p;
}

void Compiler::safeKeep(XValue& xv) {
	switch (xv.t) {
		case XValue::PUSHED:
		case XValue::INDEXED:
		case XValue::NAMED:
		case XValue::PROPERTY: {
			makeRValue(xv);
			Int32 tempLocalIndex = function->localsCount++;
			emitWithInt32(Processor::WRITE_INDEXED_OP, tempLocalIndex);
			emit(Processor::POP_OP);
			xv = XValue(XValue::INDEXED, Value(tempLocalIndex));
			break;
		}
		default: break;
	}
}

void Compiler::makeRValue(XValue& xv) {
	switch (xv.t) {
		default: assert(0);
		case XValue::PUSHED: return;
		case XValue::NONE: emit(Processor::VOID_OP); break;
		case XValue::CONSTANT: emitWithConstant(Processor::CONST_OP, xv.v); break;
		case XValue::INDEXED: emitWithInt32(Processor::READ_INDEXED_OP, xv.v.toInt()); break;
		case XValue::NAMED: emitWithConstant(Processor::READ_NAMED_OP, xv.v); break;
		case XValue::PROPERTY: emit(Processor::GET_PROPERTY_OP); break;
	}
	xv = XValue(XValue::PUSHED);
}

void Compiler::makeAssignment(XValue& xv) {
	switch (xv.t) {
		default: throw std::runtime_error("Illegal l-value"); // FIX : ECMA like the word "reference"
		case XValue::INDEXED: emitWithInt32(Processor::WRITE_INDEXED_OP, xv.v.toInt()); break;
		case XValue::NAMED: emitWithConstant(Processor::WRITE_NAMED_OP, xv.v); break;
		case XValue::PROPERTY: emit(Processor::SET_PROPERTY_OP); break;
	}
	xv = XValue(XValue::PUSHED);
}

void Compiler::discard(XValue& xv) {
	switch (xv.t) {
		case XValue::PUSHED: emit(Processor::POP_OP); break;
		case XValue::NAMED: emitWithConstant(Processor::READ_NAMED_OP, xv.v); emit(Processor::POP_OP); break;
		case XValue::PROPERTY: emit(Processor::GET_PROPERTY_OP); emit(Processor::POP_OP); break;
		default: break;
	}
	xv = XValue(XValue::NONE);
}

void Compiler::expectToken(const char* token, bool eatLeadingWhite) {
	if (!eatToken(token, eatLeadingWhite)) throw std::runtime_error(std::string("Expected '") + token + "'");
}

Compiler::XValue Compiler::parseOperand(const OperatorInfo& op) {
	XValue rv;
	parseExpression(rv, (op.closeToken != 0 ? LOWEST_PREC : op.prec));
	if (op.closeToken != 0) expectToken(op.closeToken, true);
	return rv;
}

void Compiler::parseRValueOperand(const OperatorInfo& op) {
	XValue xv(parseOperand(op));
	makeRValue(xv);
}

void Compiler::parseFunctionCall(XValue& v, Processor::Opcode op) {
	eatWhite();
	Int32 count = 0;
	while (p != e && !eatToken(")", false)) {
		if (count != 0) {
			if (!eatToken(",", false)) throw std::runtime_error("Expected ',' or ')'");
			eatWhite();
		}
		parseExpression(v, COMMA_PREC);
		makeRValue(v);
		eatWhite();
		++count;
	}
	if (p == e) throw std::runtime_error("Expected ',' or ')'");
	emitWithInt32(op, count);
	v = XValue(XValue::PUSHED);
}

bool Compiler::parseOperator(XValue& v, Precedence precedence, int opCount, const OperatorInfo* opList) {
	const Char* b = p;
	// FIX : use a quick hash solution
	int i = 0;
	while (i < opCount && !eatToken(opList[i].token, false)) ++i;
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
			emit(Processor::REPUSH_OP);
			emitWithConstant(Processor::CONST_OP, Value(1.0)); 
			emit(op.vmOp);
			makeAssignment(v);
			emit(Processor::POP_OP);
			break;
		}
		
		case LOGICAL_AND_OR: {
			makeRValue(v);
			emit(Processor::REPUSH_OP);
			Int32 point = emitForwardBranch(op.vmOp);
			emit(Processor::POP_OP);
			parseRValueOperand(op);
			completeForwardBranch(point);
			break;
		}
		
		case CONDITIONAL: {
			makeRValue(v);
			Int32 falsePoint = emitForwardBranch(Processor::JF_OP);
			parseRValueOperand(op);
			Int32 endPoint = emitForwardBranch(Processor::JMP_OP);
			completeForwardBranch(falsePoint);
			parseExpression(v, op.prec);
			makeRValue(v);
			completeForwardBranch(endPoint);
			break;			
		}
		
		case PROPERTY_DOT: {
			makeRValue(v);
			eatWhite();
			const Char* b = p;
			parseIdentifier(true);
			if (p == b) throw std::runtime_error("Expected identifier");
			emitWithConstant(Processor::CONST_OP, Value(String::newHashedString(engine.heap(), b, p)));
			v = XValue(XValue::PROPERTY);
			break;
		}
		
		case FUNCTION_CALL: {
			Processor::Opcode op = Processor::CALL_GLOBAL_OP;
			if (v.t == XValue::PROPERTY) op = Processor::CALL_METHOD_OP;
			else makeRValue(v);
			parseFunctionCall(v, op);
			break;
		}
		
		case NEW_OPERATOR: {
			parseRValueOperand(op);
			if (!eatToken("(", true)) {
				emitWithInt32(Processor::NEW_OP, 0);
				v = XValue(XValue::PUSHED);
			} else {
				eatWhite();
				parseFunctionCall(v, Processor::NEW_OP);
			}
			emit(Processor::NEW_RESULT_OP);
			break;
		}
		
		case COMMA: {
			discard(v);
			v = parseOperand(op);
			makeRValue(v);
			break;
		}
		
		case DELETE_OPERATOR: {
			v = parseOperand(op);
			switch (v.t) {
				default: assert(0);
				case XValue::PUSHED: emit(Processor::POP_OP); /* continue */
				case XValue::NONE:
				case XValue::CONSTANT: v = XValue(XValue::CONSTANT, Value(true)); break;
				case XValue::INDEXED: v = XValue(XValue::CONSTANT, Value(false)); break;
				case XValue::NAMED: emit(Processor::FRAME_OP); emitWithConstant(Processor::CONST_OP, v.v);  /* continue */
				case XValue::PROPERTY: emit(Processor::DELETE_OP); v = XValue(XValue::PUSHED); break;
			}
			break;
		}
		
		case VOID_OPERATOR: {
			v = parseOperand(op);
			discard(v);
			break;
		}
		
		case GROUP: {
			v = parseOperand(op);
			break;
		}
		
		case UNARY: {
			v = parseOperand(op);
			if (op.vmOp == Processor::ADD_OP || op.vmOp == Processor::SUB_OP) {
				XValue lv(v);
				makeRValue(lv);
				emitWithConstant(Processor::CONST_OP, Value(1.0)); 
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
			parseRValueOperand(op);
			emit(op.vmOp);
			break;
		}
		
		case ASSIGNMENT: {
			XValue lv(v);
			if (op.vmOp != Processor::INVALID_OP) {
				makeRValue(lv);
				parseRValueOperand(op);
				emit(op.vmOp);
			} else {
				parseRValueOperand(op);
			}
			makeAssignment(v);
			break;
		}
		
		case PROPERTY_BRACKETS: {
			makeRValue(v);
			parseRValueOperand(op); // FIX : convert to string const if const, or actually if we allow Value properties and optimize array[double] we shouldn't
			v = XValue(XValue::PROPERTY);
			break;
		}
		
		default: assert(0);
	}
	return true;
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
	int stringIndex = QUICK_HASH_TABLE[((s[1] + s[0] << 3) - n) & 255];
	return (stringIndex >= 0 && strncmp(s, STRINGS[stringIndex], n) == 0 && STRINGS[stringIndex][n] == 0) ? stringIndex : -1;
};

void Compiler::parseIdentifier(bool acceptReservedKeywords) {
	if (p != e && isLeadingIdentifierChar(*p)) {
		const Char* b = p;
		while (++p != e && (isLeadingIdentifierChar(*p) || (*p >= '0' && *p <= '9'))) { }
		if (!acceptReservedKeywords && findReservedKeyword(p - b, b) >= 0)
			throw std::runtime_error("Illegal use of reserved keyword");
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

bool Compiler::parseOptionalExpression(XValue& v, Precedence precedence) {
	eatWhite();
	if (p == e) return false;
	if (!parseOperator(v, precedence, sizeof (PRE_OPS) / sizeof (*PRE_OPS), PRE_OPS)) {
		const Char* b = p;
		eatToken();
		int i = findLiteralToken(p - b, &*b);
		if (i >= 0) {
			switch (static_cast<Literal>(i)) {
				case NULL_LITERAL: v = XValue(XValue::CONSTANT, Value(Value::NULL_TYPE)); break;
				case FALSE_LITERAL: v = XValue(XValue::CONSTANT, Value(false)); break;
				case TRUE_LITERAL: v = XValue(XValue::CONSTANT, Value(true)); break;
				case THIS_LITERAL: {
					emit(Processor::THIS_OP);
					v = XValue(XValue::PUSHED); break;
					break;
				}
				case FUNCTION_LITERAL: {
					ScriptFunctionDef* func = engine.heap().manage(new ScriptFunctionDef());
					// FIX : the optional function identifier
					expectToken("(", true);
					eatWhite();
					while (p != e && !eatToken(")", false)) {
						if (func->argumentsCount != 0) {
							if (!eatToken(",", false)) throw std::runtime_error("Expected ',' or ')'");
							eatWhite();
						}
						const Char* b = p;
						parseIdentifier(false);
						if (p == b) throw std::runtime_error("Expected identifier");
						func->localNames[String::newHashedString(engine.heap(), b, p)] = func->localsCount;
						func->localsCount++;
						func->argumentsCount++;
						eatWhite();
					}
					if (p == e) throw std::runtime_error("Expected ',' or ')'");
					expectToken("{", true);
					{
						Compiler funcCompiler(engine, func);
						engine.heap().protect(&funcCompiler);
						p = funcCompiler.compile(p, e);
						funcCompiler.finish();
					}
					expectToken("}", false);
					emitWithConstant(Processor::GEN_FUNC_OP, Value(func)); 
					v = XValue(XValue::PUSHED);
					break;
				}
				default: assert(0);
			}
		} else {
			p = b;
			assert(p != e);
			switch (*p) {
				case '0': {
					if (p + 1 != e && p[1] == 'x') {
						UInt32 i;
						const Char* b = p += 2;
						p = parseHex(b, e, i);
						if (p == b) throw std::runtime_error("Invalid number");
						v = XValue(XValue::CONSTANT, Value((Int32)(i)));
						break;
					}
					/* else continue */
				}
				case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '.': {
					double d;
					const Char* b = p;
					p = parseDouble(p, e, d);
					if (p == b) throw std::runtime_error("Invalid number");
					v = XValue(XValue::CONSTANT, Value(d));
					break;
				}
				case '\'': case '"': {
					size_t maxLength = unescapedMaxLength(p, e);
					QuickArray<Char, 64> buffer(maxLength);
					Char* unescapeEnd = unescape(buffer, p, e);
					assert(unescapeEnd <= buffer + maxLength);
					v = XValue(XValue::CONSTANT, Value(String::newHashedString(engine.heap(), buffer, unescapeEnd)));
					break;
				}
				default: {
					const Char* b = p;
					parseIdentifier(false);
					if (p == b) return false; // fix : nicer way?
					String* identifier = String::newHashedString(engine.heap(), b, p);
					Int32 index;
					LocalNameMap& nameMap = function->localNames;
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
	const Char* b = p;
    eatWhite();
    while (parseOperator(v, precedence, sizeof (POST_OPS) / sizeof (*POST_OPS), POST_OPS)) {
		b = p;
		eatWhite();
	}
	p = b;
	return true;
}

void Compiler::parseExpression(XValue& v, Precedence precedence) {
	if (!parseOptionalExpression(v, precedence)) throw std::runtime_error("Expected expression");
}

void Compiler::parseAndPopOptionalExpression() {
	XValue xv;
	if (parseOptionalExpression(xv, LOWEST_PREC)) discard(xv);
}

void Compiler::parseVarDeclarations() {
	while (true) {
		const Char* b = p; // FIX : sub
		parseIdentifier(false);
		if (p == b) throw std::runtime_error("Expected identifier"); // FIX : add flag to parseIdentifier
		String* identifier = String::newHashedString(engine.heap(), b, p);
		Int32 index;
		LocalNameMap& nameMap = function->localNames;
		LocalNameMap::const_iterator it = nameMap.find(identifier);
		if (it != nameMap.end()) index = it->second;
		else {
			index = function->localsCount++;
			nameMap[identifier] = index;
		}
		if (eatToken("=", true)) {
			XValue xv;
			parseExpression(xv, COMMA_PREC); // FIX : SUB
			makeRValue(xv);
			emitWithInt32(Processor::WRITE_INDEXED_OP, index);
			emit(Processor::POP_OP);
		}
		if (!eatToken(",", true)) break;
		eatWhite();
	}
}

void Compiler::expectSemicolon() {
	const Char* b = p;
	eatWhite();
	if (p != e && *p != '}' && !eatToken(";", false) && std::find(b, p, '\n') == p) {
		p = b;
		throw std::runtime_error("Syntax error");
	}
}

void Compiler::completeContinues(const Scope* fromScope, const Scope* untilScope) {
	for (const Scope* s = fromScope; s != untilScope; s = s->next) {
		for (std::vector<Int32>::const_iterator it = s->continues.begin(); it != s->continues.end(); ++it)
			completeForwardBranch(*it);
	}
}

void Compiler::completeBreaks(const Scope* scope) {
	for (std::vector<Int32>::const_iterator it = scope->breaks.begin(); it != scope->breaks.end(); ++it)
		completeForwardBranch(*it);
}

// FIX : make member functions of all the different statements

void Compiler::parseStatement(Scope* firstScope, Scope* scopeLabelsEnd) {
	eatWhite();	
		
	if (eatToken("{", false)) {
		parseStatementList(firstScope);
		expectToken("}", false);
		return;
	}
	
	const Char* b = p;
	eatToken();
	int statementTokenIndex = findStatementToken(p - b, &*b);
	if (statementTokenIndex < 0) {
		p = b;
		parseIdentifier(true);
		const Char* q = p;
		if (p != b && eatToken(":", true) && findReservedKeyword(q - b, b) < 0) {
			String identifier(b, q);
			for (Scope* s = firstScope; s != 0; s = s->next) {
				if (!s->label.empty() && s->label.isEqualTo(identifier)) throw std::runtime_error("Duplicate label");
			}
			Scope newLabelScope(identifier, firstScope);
			parseStatement(&newLabelScope, scopeLabelsEnd);
			completeBreaks(&newLabelScope);
		} else {
			p = b;
			parseAndPopOptionalExpression();
			expectSemicolon();
		}
	} else {
		eatWhite();
		switch (static_cast<Statement>(statementTokenIndex)) {
			case BREAK_STATEMENT:
			case CONTINUE_STATEMENT: {
				bool isBreak = (statementTokenIndex == BREAK_STATEMENT);
				const Char* b = p;
				Scope* s = firstScope;
				parseIdentifier(false);
				String identifier(b, p);
				while (s != 0 && !((s->type == Scope::ITERATOR_LABEL_TYPE || s->type == Scope::LABEL_TYPE)
						&& s->label.isEqualTo(identifier))) {
					if (s->type == Scope::CATCH_TYPE || s->type == Scope::TRY_TYPE) // FIX : share with return
						s->finallys.push_back(emitForwardBranch(Processor::JSR_OP));
					if (s->type == Scope::CATCH_TYPE || s->type == Scope::FINALLY_TYPE) emit(Processor::POP_FRAME_OP);
					s = s->next;
				}
				if (s == 0) {
					throw std::runtime_error(!identifier.empty() ? "Undefined label"
							: (isBreak ? "Illegal break" : "Illegal continue"));
				}
				if (!isBreak && s->type != Scope::ITERATOR_LABEL_TYPE)
					throw std::runtime_error("Illegal label for continue");
				(isBreak ? s->breaks : s->continues).push_back(emitForwardBranch(Processor::JMP_OP));
				expectSemicolon();
				break;
			}
						
			case VAR_STATEMENT: {
				parseVarDeclarations();
				expectSemicolon();
				break;
			}
			
			case IF_STATEMENT: {
				expectToken("(", false);
				XValue xv; // FIX : sub
				parseExpression(xv, LOWEST_PREC); // FIX : SUB
				makeRValue(xv);
				expectToken(")", true);
				Int32 falsePoint = emitForwardBranch(Processor::JF_OP);
				parseStatement(firstScope, firstScope);
				if (eatToken("else", true)) {
					Int32 endPoint = emitForwardBranch(Processor::JMP_OP);
					completeForwardBranch(falsePoint);
					parseStatement(firstScope, firstScope);
					completeForwardBranch(endPoint);
				} else {
					completeForwardBranch(falsePoint);
				}				
				break;
			}

			case WHILE_STATEMENT: {
				Int32 loopPoint = markBackwardBranch();
				Scope emptyLabelScope(EMPTY_STRING, firstScope);
				emptyLabelScope.makeIteratorScopes(scopeLabelsEnd);
				expectToken("(", false);
				XValue xv; // FIX : sub
				parseExpression(xv, LOWEST_PREC); // FIX : SUB
				makeRValue(xv);
				expectToken(")", true);
				Int32 falsePoint = emitForwardBranch(Processor::JF_OP);
				parseStatement(&emptyLabelScope, &emptyLabelScope);
				completeContinues(&emptyLabelScope, scopeLabelsEnd);
				emitBackwardBranch(Processor::JMP_OP, loopPoint);
				completeForwardBranch(falsePoint);
				completeBreaks(&emptyLabelScope);
				break;
			}

			case DO_WHILE_STATEMENT: {
				Int32 loppPoint = markBackwardBranch();
				Scope emptyLabelScope(EMPTY_STRING, firstScope);
				emptyLabelScope.makeIteratorScopes(scopeLabelsEnd);
				parseStatement(&emptyLabelScope, &emptyLabelScope);
				expectToken("while", true);
				expectToken("(", true);
				completeContinues(&emptyLabelScope, scopeLabelsEnd);
				XValue xv; // FIX : sub
				parseExpression(xv, LOWEST_PREC); // FIX : SUB
				makeRValue(xv);
				expectToken(")", true);
				emitBackwardBranch(Processor::JT_OP, loppPoint);
				expectSemicolon();
				completeBreaks(&emptyLabelScope);
				break;
			}
			
			case FOR_STATEMENT: {
				expectToken("(", false);
				if (eatToken("var", true)) {
					eatWhite();
					parseVarDeclarations();
				} else {
					parseAndPopOptionalExpression();
				}
				expectToken(";", true);
				Int32 loopPoint = markBackwardBranch();
				Int32 falsePoint = -1;
				XValue xv;
				if (parseOptionalExpression(xv, LOWEST_PREC)) {
					makeRValue(xv);
					falsePoint = emitForwardBranch(Processor::JF_OP);
				}
				expectToken(";", true);
				std::vector<Byte> incCode;
				outputCode = &incCode;
				parseAndPopOptionalExpression();
				expectToken(")", true);
				outputCode = &function->code;
				Scope emptyLabelScope(EMPTY_STRING, firstScope);
				emptyLabelScope.makeIteratorScopes(scopeLabelsEnd);
				parseStatement(&emptyLabelScope, &emptyLabelScope);
				completeContinues(&emptyLabelScope, scopeLabelsEnd);
				outputCode->insert(outputCode->end(), incCode.begin(), incCode.end());
				emitBackwardBranch(Processor::JMP_OP, loopPoint);
				if (falsePoint >= 0) completeForwardBranch(falsePoint);
				completeBreaks(&emptyLabelScope);
				break;
			}
			
			case RETURN_STATEMENT: {
				XValue returnXValue;
				parseOptionalExpression(returnXValue, LOWEST_PREC);
				Scope* s = firstScope;
				bool safeKept = false;
				while (s != 0) { // FIX : sub
					if (s->type == Scope::CATCH_TYPE || s->type == Scope::TRY_TYPE) {
						if (!safeKept) {
							safeKeep(returnXValue);
							safeKept = true;
						}
						s->finallys.push_back(emitForwardBranch(Processor::JSR_OP));
					}
					if (s->type == Scope::CATCH_TYPE || s->type == Scope::FINALLY_TYPE) emit(Processor::POP_FRAME_OP);
					s = s->next;
				}
				makeRValue(returnXValue);
				emit(Processor::RETURN_OP);
				expectSemicolon();
				break;
			}
			
			case TRY_STATEMENT: {
				Scope tryScope(Scope::TRY_TYPE, firstScope);
				Int32 catchPoint = emitForwardBranch(Processor::TRY_OP);
				expectToken("{", true); // FIX : sub for block
				parseStatementList(&tryScope);
				expectToken("}", false);
				tryScope.finallys.push_back(emitForwardBranch(Processor::JSR_OP));
				Int32 tryEndPoint = emitForwardBranch(Processor::JMP_OP);
				Int32 catchEndPoint = -1;
				
				// Catcher
				
				bool gotCatch = false;
				if (eatToken("catch", true)) {
					gotCatch = true;
					expectToken("(", true);
					const Char* b = p;
					parseIdentifier(false);
					if (p == b) throw std::runtime_error("Expected identifier"); // FIX : add flag to parseIdentifier
					String* exceptionVarName = String::newHashedString(engine.heap(), b, p);
					LocalNameMap& nameMap = function->localNames;
					Int32 hadLocalIndex = -1;
					LocalNameMap::iterator it = nameMap.find(exceptionVarName);
					if (it != nameMap.end()) {
						hadLocalIndex = it->second;
						nameMap.erase(it);
					}
					completeForwardBranch(catchPoint);
					catchPoint = emitForwardBranch(Processor::TRY_OP);
					emitWithConstant(Processor::CATCH_FRAME_OP, Value(exceptionVarName));
					expectToken(")", true);
					Scope catchScope(Scope::CATCH_TYPE, firstScope);
					expectToken("{", true); // FIX : sub
					parseStatementList(&catchScope);
					expectToken("}", false);
					emit(Processor::POP_FRAME_OP); // FIX : try to move POP_FRAME to first in FINALLY block, need different JSR then
					catchScope.finallys.push_back(emitForwardBranch(Processor::JSR_OP));
					catchEndPoint = emitForwardBranch(Processor::JMP_OP);
					for (std::vector<Int32>::const_iterator it = catchScope.finallys.begin(); it != catchScope.finallys.end(); ++it) // FIX : sub
						completeForwardBranch(*it);
					if (hadLocalIndex >= 0) nameMap[exceptionVarName] = hadLocalIndex;
				}
				
				// Finally
				
				for (std::vector<Int32>::const_iterator it = tryScope.finallys.begin(); it != tryScope.finallys.end(); ++it) // FIX : sub
					completeForwardBranch(*it);
				emit(Processor::TRIED_OP);
				Int32 rethrowPoint = markBackwardBranch();
				if (eatToken("finally", true)) {
					Scope finallyScope(Scope::FINALLY_TYPE, firstScope);
					expectToken("{", true); // FIX : sub
					parseStatementList(&finallyScope);
					expectToken("}", false);
				} else {
					if (!gotCatch) throw std::runtime_error("Expected 'catch' or 'finally'");
				}
				emit(Processor::RETURN_OP);
				
				// Re-thrower (could actually be eliminated if we only have catch and no finally)
				
				completeForwardBranch(catchPoint);
				XValue thrownXValue(XValue::PUSHED);
				safeKeep(thrownXValue);
				emitBackwardBranch(Processor::JSR_OP, rethrowPoint);
				makeRValue(thrownXValue);
				emit(Processor::THROW_OP);
				completeForwardBranch(tryEndPoint);
				if (catchEndPoint >= 0) completeForwardBranch(catchEndPoint);
				break;
			}
			
			case THROW_STATEMENT: {
				XValue xv; // FIX : sub
				parseExpression(xv, LOWEST_PREC); // FIX : SUB
				makeRValue(xv);
				emit(Processor::THROW_OP);
				expectSemicolon();
				break;
			}
			
			case SWITCH_STATEMENT: {
				expectToken("(", true);
				XValue controlXValue; // FIX : sub
				parseExpression(controlXValue, LOWEST_PREC); // FIX : SUB
				safeKeep(controlXValue);
				expectToken(")", true);
				expectToken("{", true);
				Int32 testsPoint = emitForwardBranch(Processor::JMP_OP);
				std::vector<Byte> testsCode;
				std::vector<Int32> jumpOffsets;
				Int32 startOffset = outputCode->size();
				bool firstStatement = true;
				eatWhite();
				while (p != e && !eatToken("}", false)) {
					if (eatToken("case", false)) {
						outputCode = &testsCode;
						XValue xv(controlXValue); // copy so that makeRValue doesn't change it to pushed
						makeRValue(xv);
						parseExpression(xv, LOWEST_PREC);
						expectToken(":", true);
						makeRValue(xv);
						emit(Processor::STRICT_EQ_OP);
						emitWithInt32(Processor::JT_OP, function->code.size() - startOffset - testsCode.size() - OPERAND_SIZE - 1);
						jumpOffsets.push_back(testsCode.size() - OPERAND_SIZE);
						outputCode = &function->code;
					} else if (eatToken("default", false)) {
						expectToken(":", true);
					} else if (firstStatement) {
						throw std::runtime_error("Expected 'case', 'default' or '}'");
					} else {
						const Char* b = p;
						parseStatement(firstScope, firstScope);
						if (b == p) throw std::runtime_error("Expected '}'");
					}
					firstStatement = false;
					eatWhite();
				}
				if (p == e) throw std::runtime_error("Expected '}'");
				
				Int32 donePoint = emitForwardBranch(Processor::JMP_OP);
				completeForwardBranch(testsPoint);
				for (std::vector<Int32>::const_iterator it = jumpOffsets.begin(), e = jumpOffsets.end(); it != e; ++it) {
					testsCode[*it] -= outputCode->size() - startOffset;
				}
				outputCode->insert(outputCode->end(), testsCode.begin(), testsCode.end());
				completeForwardBranch(donePoint);
				break;
			}
			
			default: assert(0);
		}
	}
}

void Compiler::parseStatementList(Scope* firstScope) {
	const Char* b;
	do {
		b = p;
		parseStatement(firstScope, firstScope);
	} while (p != e && b != p);
}

const Char* Compiler::compile(const Char* b, const Char* e) { outputCode = &function->code; p = b; this->e = e; parseStatementList(0); return p; }
void Compiler::compile(const String& source) { compile(source.begin(), source.end()); }

void Compiler::finish() {
	emit(Processor::VOID_OP);
	emit(Processor::RETURN_OP);
	// FIX : shrink code vector (copy to quickarray instead?)
disassemble(engine.heap(), *outputCode, function->constants);
}

GCHeap::GCHeap() : currentList(&managedLists[0]), newList(&managedLists[1]) { }

size_t GCHeap::size() const { return currentList->size(); }
size_t GCHeap::gc() {
	size_t sweptCount = 0;

	for (const GCItem* item = rootList._gcNext; item != &rootList; item = item->_gcNext)
		const_cast<GCItem*>(item)->gcMark(*this);
	
	{
		size_t n = 0;
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


#include <fstream>

	class PrintFunction : public NativeFunction {
		public:		typedef NativeFunction super;
		public:		virtual void call(GCHeap& heap, Processor* processor, Int32 argc, const Value* argv, const Value& thisValue) {
						String* s = (argc >= 1 ? argv[0].toString(heap) : &EMPTY_STRING);
						std::wcout << s->toStdString().c_str() << std::endl;
						processor->push(Value());
					}
	};


int main(int argc, const char* argv[]) {
	static const String LF_STRING("\n");
	GCHeap heap;
	Engine engine(heap);
	UserObject globals;
	heap.protect(&globals);
	static String PRINT_STRING("print");
	PrintFunction printFunction;
	globals.setProperty(engine, &PRINT_STRING, Value(heap.protect(&printFunction))); // FIX : should be able to not have to new this, just like strings, but then we need the GC to be able to probe objects without assigned heap
	ScriptFunctionDef functionDef;
	heap.protect(&functionDef);
	Compiler compiler(engine, &functionDef);
	heap.protect(&compiler);
	String source(EMPTY_STRING);
	std::auto_ptr<std::ifstream> inputFileStream;
	std::istream* inStream = &std::cin;
	if (argc >= 2) {
		inputFileStream.reset(new std::ifstream(argv[1]));
		inStream = inputFileStream.get();
	}
	while (inStream->good()) {
		try {
			functionDef.code.clear();
			
			std::string s;
			std::getline(*inStream, s);
			if (!s.empty()) {
				String line(s.c_str());
				source = String(source, line);
				source = String(source, LF_STRING);
			} else {
				compiler.compile(source);
				compiler.finish();
				
				std::cerr << "\tCode: " << functionDef.code.size() << std::endl;
				std::cerr << "\tConstants: " << functionDef.constants.size() << std::endl;
				std::cerr << "\tLocals: " << functionDef.localsCount << std::endl;
				
				Value vstack[1024];
				Context cstack[1024];
				Processor processor(engine, cstack, cstack + 1024, vstack, vstack + 1024);
				heap.protect(&processor);
				processor.enterCall(heap.manage(new ScriptFunction(&functionDef, &globals)), 0, 0, Value(&globals));
				bool done = false;
				
				clock_t start = ::clock();	
				do {
					done = processor.run(10000000);
					Value v = vstack[0];
					
					size_t sweptCount = heap.gc();

					std::cerr << "\tKept: " << heap.size() << std::endl;
					std::cerr << "\tSwept: " << sweptCount << std::endl;
					
					if (done) {
						std::cerr << "\tOn stack: " << (processor.vstackPointer - vstack + 1) << std::endl;
						assert(processor.vstackPointer - vstack + 1 == 1);
						std::wcerr << L"\t=" << (v.toString(heap))->toStdString().c_str() << std::endl;
						clock_t end = ::clock();
						const double divisor = (CLOCKS_PER_SEC / 1000.0);
						std::cerr << "\ttime: " << (static_cast<double>(end - start) / divisor) << std::endl;
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
	return 0;
}
