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

static StringIt parseHex(StringIt p, const StringIt& e, UInt32& i) {
	for (i = 0; p != e && ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F') || (*p >= 'a' && *p <= 'f')); ++p)
		i = (i << 4) + (*p <= '9' ? *p - '0' : (*p & ~0x20) - ('A' - 10));
	return p;
}

static StringIt parseUnsignedInt(StringIt p, const StringIt& e, UInt32& i) {
	for (i = 0; p != e && *p >= '0' && *p <= '9'; ++p) i = i * 10 + (*p - '0');
	return p;
}

static StringIt parseInt(StringIt p, const StringIt& e, Int32& i) {
	bool negative = (e - p >= 2 && ((*p == '+' || *p == '-') && p[1] >= '0' && p[1] <= '9') ? (*p++ == '-') : false);
	UInt32 ui;
	p = parseUnsignedInt(p, e, ui);
	i = (negative ? -static_cast<Int32>(ui) : ui);
	return p;
}

static StringIt parseDouble(StringIt p, const StringIt& e, double& v) {
	assert(p <= e);
	double d = 0;
	StringIt q = p;
	double sign = (e - q > 1 && (*q == '+' || *q == '-') ? (*q++ == '-' ? -1.0 : 1.0) : 1.0);
	if (q == e || (*q != '.' && (*q < '0' || *q > '9'))) return p;
	StringIt b = q;
	while (q != e && *q >= '0' && *q <= '9') d = d * 10.0 + (*q++ - '0');
	if (q != e && *q == '.') {
		double f = 1.0;
		while (++q != e && *q >= '0' && *q <= '9') d += (*q - '0') * (f *= 0.1);
		if (q == b + 1) return p;
	}
	if (q != e && (*q == 'E' || *q == 'e')) {
		Int32 i;
		StringIt t = parseInt(q + 1, e, i);
		if (t != q + 1) { d *= pow(10, static_cast<double>(i)); q = t; }
	}
	v = d * sign;
	return q;
}

// Based on code from PikaScript
static String intToString(int i, int radix = 10, int minLength = 1) {
	assert(2 <= radix && radix <= 16);
	assert(0 <= minLength && minLength <= sizeof (int) * 8);
	Char buffer[sizeof (int) * 8 + 1], * p = buffer + sizeof (int) * 8 + 1, * e = p - minLength;
	for (int x = i; p > e || x != 0; x /= radix) {
		assert(p >= buffer + 2);
		*--p = "fedcba9876543210123456789abcdef"[15 + x % radix];														// Mirrored hex string to handle negative x.
	}
	if (i < 0) *--p = '-';
	return String(p, buffer + sizeof (i) * 8 + 1 - p);
}

// Based on code from PikaScript
static String doubleToString(double d, int precision = 15) {
	assert(1 <= precision && precision <= 24);

	const double EPSILON = 1.0e-300;
	const double SMALL = 1.0e-6;
	const double LARGE = 1.0e+21;

	if (std::isnan(d)) return "NaN";
	
	double x = fabs(d);
	if (x <= EPSILON) return "0";
	else if (std::numeric_limits<double>::has_infinity && x == std::numeric_limits<double>::infinity())
		return d < 0 ? "-Infinity" : "+Infinity";

	Char buffer[32];
	Char* bp = buffer + 2;
	Char* dp = bp;
	Char* pp = dp + 1;
	Char* ep = pp + precision;
	
	double y = x;	
	for (; x >= 10.0 && pp < ep; x *= 0.1) ++pp;																		// Normalize values > 10 and move period position.

	if (pp >= ep || y <= SMALL || y >= LARGE) {																			// Exponential treatment of very small or large values.
		double e = floor(log10(y) + 1.0e-10);
		String exps(e >= 0 ? "e+" : "e");
		exps += intToString(static_cast<int>(e));
		int maxp = 15;																									// Limit precision because of rounding errors in log10 etc
		for (double f = fabs(e); f >= 8; f /= 10) --maxp;
		return (doubleToString(d * pow(0.1, e), std::min(maxp, precision)) += exps);
	}
	for (; x < 1.0 && dp < buffer + 32; ++ep, x *= 10.0) {																// For values < 1, spit out leading 0's and increase precision.
		*dp++ = '0';
		if (dp == pp) *dp++ = '9';																						// Hop over period position (set to 9 to avoid when eliminating 9's).
	}
	for (; dp < ep; ) {																									// Exhaust all remaining digits of mantissa into buffer.
		unsigned int ix = static_cast<unsigned int>(x);
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
	return String(bp, ep - bp);
}

const int ESCAPE_CODE_COUNT = 10;

static String unescape(StringIt& p, const StringIt& e) {
	assert(p <= e);
	static const Char ESCAPE_CHARS[ESCAPE_CODE_COUNT] = { '\\', '\"', '\'',  'a',  'b',  'f',  'n',  'r',  't',  'v' };
	static const Char ESCAPE_CODES[ESCAPE_CODE_COUNT] = { '\\', '\"', '\'', '\a', '\b', '\f', '\n', '\r', '\t', '\v' };
	if (p >= e || *p != '"' && *p != '\'') throw std::runtime_error("Invalid string literal");
	Char endChar = *p;
	String d;
	StringIt b = ++p;
	while (p < e && *p != endChar) {
	/* FIX :	if (*p == '\\' && p + 1 < e) {
			d += String(b, p);
			const Char* f = find(ESCAPE_CHARS, ESCAPE_CHARS + ESCAPE_CODE_COUNT, *++p);
			long l;
			if (f != ESCAPE_CHARS + ESCAPE_CODE_COUNT) { ++p; l = ESCAPE_CODES[f - ESCAPE_CHARS]; }
			else if (*p == 'x') { b = ++p; l = hexToLong<String>(p, (e - p > 2 ? p + 2 : e)); }
			else if (*p == 'u') { b = ++p; l = hexToLong<String>(p, (e - p > 4 ? p + 4 : e)); }
			else { b = p; l = stringToLong<String>(p, e); }
			if (p == b) throw std::runtime_error("Invalid escape character");
			b = p;
			d += Char(l);
		} else*/ ++p;
	}
	if (p >= e) throw std::runtime_error("Unterminated string");
	return (d += String(b, p++));
}

class StringObject : public Object {
	public:		static StringObject* newString(GCHeap* heap, const String& s);
	public:		StringObject(const String& s) : s(s) { }
	public:		virtual bool getProperty(StringObject* name, Value& v);
	public:		virtual bool setProperty(StringObject* name, const Value& v) { return false; }
	public:		virtual ~StringObject() { std::cout << "Deleting " << s.c_str() << std::endl; }
	public:		String s;
};

StringObject* StringObject::newString(GCHeap* heap, const String& s) {
	return heap->manage(new StringObject(s));
}

bool StringObject::getProperty(StringObject* name, Value& v) {
	UInt32 i;
	StringIt p = name->s.begin();
	StringIt e = name->s.end();
	StringIt q = parseUnsignedInt(p, e, i);
	if (q == e && 0 <= i && i < s.size()) {
		v = Value(newString(gcHeap(), String(1, s[i])));
		return true;
	}
	return false;
}

int GCHeap::sweep() {
	int count = 0;
	for (std::vector<GCItem*>::reverse_iterator it = objects.rbegin(), e = objects.rend(); it != e; ++it) {
		if (*it != 0 && (*it)->_gcHeap == this) {
			delete *it;
			*it = 0;
			count++;
		}
	}
	objects.clear();
	return count;
}

void GCHeap::add(GCItem* object) {
	assert(object->_gcHeap == 0);
	objects.push_back(object);
	object->_gcHeap = this;
}

void GCHeap::mark(GCItem* object) {
	if (object != 0 && object->_gcHeap != this && object->_gcHeap != 0) {
		objects.push_back(object);
		object->_gcHeap = this;
		object->garbageCollectAndMark(this);
	}
}

double NaN() { return std::numeric_limits<double>::quiet_NaN(); }

struct StringObjectCompare {
	bool operator() (StringObject* a, StringObject* b) const {
		return (a != b && a->s < b->s);
	}
};

class UserObject : public Object {
	protected:	typedef std::map<StringObject*, Value, StringObjectCompare> PropertyMap;
	public:		virtual bool getProperty(StringObject* name, Value& v) {
					PropertyMap::const_iterator it = properties.find(name);
					if (it == properties.end()) return false;
					v = it->second;
					return true;
				}
	public:		virtual bool setProperty(StringObject* name, const Value& v) { properties[name] = v; return true; }
	public:		virtual void garbageCollectAndMark(GCHeap* heap) {
					for (PropertyMap::iterator it = properties.begin(), e = properties.end(); it != e; ++it) {
						heap->mark(it->first);
						it->second.mark(heap);
					}
				}
	protected:	PropertyMap properties;
};

StringObject UNDEFINED_STRING("undefined");
StringObject NULL_STRING("null");
StringObject FALSE_STRING("false");
StringObject TRUE_STRING("true");

Value::Value(Type type) : type(type) { }
Value::Value(bool b) : type(BOOLEAN_TYPE), b(b) { }
Value::Value(Int32 i) : type(NUMBER_TYPE), d(i) { }
Value::Value(double d) : type(NUMBER_TYPE), d(d) { }
Value::Value(StringObject* s) : type(STRING_TYPE), s(s) { }
Value::Value(Object* o) : type(OBJECT_TYPE), o(o) { }

FunctionDef* Value::getFunctionDef() const {
	assert(type == OBJECT_TYPE);
	assert(o->asFunctionDef() != 0);
	return reinterpret_cast<FunctionDef*>(o);
}

StringObject* Value::getString() const {
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

Int32 Value::toInt() const { return static_cast<int>(toDouble()); } // FIX : should do modulo, see http://www.ecma-international.org/ecma-262/5.1/#sec-9.6

double Value::toDouble() const {
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return NaN();
		case NULL_TYPE: return 0.0;
		case BOOLEAN_TYPE: return b ? 1.0 : 0.0;
		case NUMBER_TYPE: return d;
		case STRING_TYPE: {
			double v;
			StringIt p = s->s.begin();
			StringIt e = s->s.end();
			StringIt q = parseDouble(p, e, v);
			if (q == p) return NaN();
			p = q;
			return v;
		}
	}
}
			
			// FIX : drop
			String Value::toString() const {
				switch (type) {
					default: assert(0);
					case UNDEFINED_TYPE: return "undefined";
					case NULL_TYPE: return "null";
					case BOOLEAN_TYPE: return b ? "true" : "false";
					case NUMBER_TYPE: return doubleToString(d);
					case STRING_TYPE: return s->s;
case OBJECT_TYPE: return "<object>";
				}
			}

StringObject* Value::toString(GCHeap* heap) const {
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return &UNDEFINED_STRING;
		case NULL_TYPE: return &NULL_STRING;
		case BOOLEAN_TYPE: return b ? &TRUE_STRING : &FALSE_STRING;
		case NUMBER_TYPE: return StringObject::newString(heap, doubleToString(d));
		case STRING_TYPE: return s;
case OBJECT_TYPE: return StringObject::newString(heap, "<object>");
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
		case UNDEFINED_TYPE: return true;
		case NULL_TYPE: return true;
		case BOOLEAN_TYPE: return b == y.b;
		case NUMBER_TYPE: return d == y.d;
		case STRING_TYPE: return s->s == y.s->s;
		case OBJECT_TYPE: return o == y.o;
	}
}

bool Value::operator<(const Value& y) const {
	if (type < y.type) return true;
	if (type > y.type) return false;
	switch (type) {
		default: assert(0);
		case UNDEFINED_TYPE: return false;
		case NULL_TYPE: return false;
		case BOOLEAN_TYPE: return b < y.b;
		case NUMBER_TYPE: return d < y.d;
		case STRING_TYPE: return s->s < y.s->s;
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
		case STRING_TYPE: return (s == y.s || s->s == y.s->s);
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
		// If Type(x) is Number and Type(y) is String,
		// return the result of the comparison x == ToNumber(y).
		// If Type(x) is String and Type(y) is Number,
		// return the result of the comparison ToNumber(x) == y.
		if (type == BOOLEAN_TYPE || y.type == BOOLEAN_TYPE)
			return toDouble() == y.toDouble();
		// If Type(x) is either String or Number and Type(y) is Object,
		// return the result of the comparison x == ToPrimitive(y).
		// If Type(x) is Object and Type(y) is either String or Number,
		// return the result of the comparison ToPrimitive(x) == y.
		return false;
	}
}

bool Value::isLessThan(const Value& y) const {
	// if (type == STRING_TYPE && y.type == STRING_TYPE)
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
	// if (type == STRING_TYPE && y.type == STRING_TYPE)
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
		String rs = y.toString(heap)->s;
		if (rs.empty()) return *this;
		String ls = toString(heap)->s;
		if (ls.empty()) return y;
		return Value(StringObject::newString(heap, ls + rs));
	}
	return Value(toDouble() + y.toDouble());
}
				
void Value::mark(GCHeap* heap) const {
	switch (type) {
		case STRING_TYPE: heap->mark(s); break;
		case OBJECT_TYPE: heap->mark(o); break;
	}
}

typedef std::map<StringObject*, Int32, StringObjectCompare> LocalNameMap;

class FunctionDef : public Object {
	public:		FunctionDef() : source(0), argumentsCount(0), localsCount(0) { }
	public:		virtual FunctionDef* asFunctionDef() { return this; }
	public:		bool lookupLocalName(StringObject* name, Int32& index) const {
					LocalNameMap::const_iterator it = locals.find(name);
					if (it == locals.end()) return false;
					index = it->second;
					return true;
				}
	public:		virtual void garbageCollectAndMark(GCHeap* heap) {
					heap->mark(source);
					// FIX : subs
					for (std::vector<Value>::iterator it = constants.begin(), e = constants.end(); it != e; ++it) {
						it->mark(heap);
					}
					for (LocalNameMap::iterator it = locals.begin(), e = locals.end(); it != e; ++it) {
						heap->mark(it->first);
					}
				}
	public:		const Value* getConstantsPointer() const { return (constants.empty() ? 0 : &constants[0]); }
	public:		StringObject* source;
	public:		std::vector<Byte> code;
	public:		std::vector<Value> constants;
	public:		Int32 argumentsCount;
	public:		Int32 localsCount; // including arguments
	public:		LocalNameMap locals;
};

class Function : public UserObject {
	friend class Frame;
	public:		Function(FunctionDef* definition, Object* closure) : definition(definition), closure(closure) { }
	public:		virtual Function* asFunction() { return this; }
	public:		virtual void garbageCollectAndMark(GCHeap* heap) {
					heap->mark(definition);
					heap->mark(closure);
				}
	public:		FunctionDef* getDefinition() const { return definition; }
	public:		Object*  getClosure() const { return closure; }
	protected:	FunctionDef* const definition;
	protected:	Object* const closure;
};

class Frame : public Object {
	public:		Frame(Function* function) : functionDef(function->getDefinition()), closure(function->getClosure())
						, locals(std::max(function->definition->localsCount, 1)) {
				}
	protected:	typedef std::map<StringObject*, Value, StringObjectCompare> DynamicMap;
	public:		virtual bool getProperty(StringObject* name, Value& v) {
					Int32 index;
					if (functionDef->lookupLocalName(name, index)) {
						assert(0 <= index && index < locals.size());
						v = locals[index];
						return true;
					} else {
						if (dynamicLocals.get() != 0) {
							DynamicMap::const_iterator it = dynamicLocals->find(name);
							if (it != dynamicLocals->end()) {
								v = it->second;
								return true;
							}
						}
						return closure->getProperty(name, v);
					}
				}
	public:		virtual bool setProperty(StringObject* name, const Value& v) {
					Int32 index;
					if (functionDef->lookupLocalName(name, index)) {
						assert(0 <= index && index < locals.size());
						locals[index] = v;
						return true;
					} else {
						if (dynamicLocals.get() != 0) {
							DynamicMap::iterator it = dynamicLocals->find(name);
							if (it != dynamicLocals->end()) {
								it->second = v;
								return true;
							}
						}
						return closure->setProperty(name, v);
					}
				}
	public:		virtual void garbageCollectAndMark(GCHeap* heap) {
					heap->mark(functionDef);
					heap->mark(closure);
					// FIX : subs
					for (std::vector<Value>::iterator it = locals.begin(), e = locals.end(); it != e; ++it) {
						it->mark(heap);
					}
					if (dynamicLocals.get() != 0) {
						for (DynamicMap::iterator it = dynamicLocals->begin(), e = dynamicLocals->end(); it != e; ++it) {
							heap->mark(it->first);
							it->second.mark(heap);
						}
					}
				}
	public:		FunctionDef* getFunctionDef() { return functionDef; }
	public:		Value* getLocalsPointer() { return &locals[0]; }
	protected:	FunctionDef* const functionDef;
	protected:	Object* const closure;
	protected:	std::vector<Value> locals;
	protected:	std::auto_ptr<DynamicMap> dynamicLocals;
};

VM::VM(Context* cstackStart, Context* cstackEnd, Value* vstackStart, Value* vstackEnd)
		: cstackStart(cstackStart), cstackEnd(cstackEnd), vstackStart(vstackStart), vstackEnd(vstackEnd)
		, cstackPointer(cstackEnd - 1), vstackPointer(vstackEnd) {
}

void VM::garbageCollectAndMark(GCHeap* heap) {
	if (cstackPointer == cstackEnd) return;
	for (Value* vsp = vstackPointer; vsp != vstackEnd; ++vsp) vsp->mark(heap);
	for (Context* csp = cstackPointer; csp != cstackEnd; ++csp) heap->mark(csp->frame);
}

void VM::push(Value*& vsp, const Value& v) { assert(vsp > vstackStart); *--vsp = v; }
void VM::pop(Value*& vsp, int count) { assert(vsp + count <= vstackEnd); vsp += count; }
Int32 VM::readInt32(const Byte*& ip) { Int32 v = *reinterpret_cast<const Int32*>(ip); ip += 4; return v; }

void VM::error() {
	throw std::runtime_error("error"); // FIX
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
			case POP_OP: pop(vsp, 1); break;
			case REPUSH_OP: push(vsp, vsp[0]); break;
			case CONST_OP: push(vsp, cp[readInt32(ip)]); break;
			case READ_INDEXED_OP: push(vsp, lp[readInt32(ip)]); break;
			case WRITE_INDEXED_OP: lp[readInt32(ip)] = vsp[0]; break;
			case READ_NAMED_OP: {
				Value v;
				bool success = frame->getProperty(cp[readInt32(ip)].getString(), v);
				if (!success) error();
				push(vsp, v);
				break;
			}
			case WRITE_NAMED_OP:  {
				bool success = frame->setProperty(cp[readInt32(ip)].getString(), vsp[0]);
				if (!success) error();
				break;
			}
			case GET_PROPERTY_OP: {
				Value v;
				Object* o = vsp[1].toObject(h);
				if (o == 0) error();
				o->getProperty(vsp[0].toString(h), v);
				vsp[1] = v;
				pop(vsp, 1);
				break;
			}
			case SET_PROPERTY_OP: {
				Object* o = vsp[2].toObject(h);
				if (o == 0) error();
				o->setProperty(vsp[1].toString(h), vsp[0]);
				vsp[2] = vsp[0];
				pop(vsp, 2);
				break;
			}
			case ADD_OP: vsp[1] = vsp[1].add(h, vsp[0]); pop(vsp, 1); break;
			case SUB_OP: vsp[1] = Value(vsp[1].toDouble() - vsp[0].toDouble()); pop(vsp, 1); break;
			case MUL_OP: vsp[1] = Value(vsp[1].toDouble() * vsp[0].toDouble()); pop(vsp, 1); break;
			case DIV_OP: vsp[1] = Value(vsp[1].toDouble() / vsp[0].toDouble()); pop(vsp, 1); break;
			case MOD_OP: vsp[1] = Value(fmod(vsp[1].toDouble(), vsp[0].toDouble())); pop(vsp, 1); break;
			case OR_OP: vsp[1] = Value(vsp[1].toInt() | vsp[0].toInt()); pop(vsp, 1); break;
			case XOR_OP: vsp[1] = Value(vsp[1].toInt() ^ vsp[0].toInt()); pop(vsp, 1); break;
			case AND_OP: vsp[1] = Value(vsp[1].toInt() & vsp[0].toInt()); pop(vsp, 1); break;
			case SHL_OP: vsp[1] = Value(vsp[1].toInt() << vsp[0].toInt()); pop(vsp, 1); break;
			case SHR_OP: vsp[1] = Value(vsp[1].toInt() >> vsp[0].toInt()); pop(vsp, 1); break;
			case UNSIGNED_SHR_OP: vsp[1] = Value((Int32)((UInt32)(vsp[1].toInt()) >> vsp[0].toInt())); pop(vsp, 1); break;
			case PLUS_OP: vsp[0] = Value(vsp[0].toDouble()); break;
			case MINUS_OP: vsp[0] = Value(-vsp[0].toDouble()); break;
			case BIT_NOT_OP: vsp[0] = Value(~vsp[0].toInt()); break;
			case NOT_OP: vsp[0] = Value(!vsp[0].toBool()); break;
			case STRICTLY_EQ_OP: vsp[1] = Value(vsp[1].isStrictlyEqualTo(vsp[0])); pop(vsp, 1); break;
			case STRICTLY_NEQ_OP: vsp[1] = Value(!vsp[1].isStrictlyEqualTo(vsp[0])); pop(vsp, 1); break;
			case EQ_OP: vsp[1] = Value(vsp[1].isEqualTo(vsp[0])); pop(vsp, 1); break;
			case NEQ_OP: vsp[1] = Value(!vsp[1].isEqualTo(vsp[0])); pop(vsp, 1); break;
			case LT_OP: vsp[1] = Value(vsp[1].isLessThan(vsp[0])); pop(vsp, 1); break; // FIX : do toPrimitive here, isLessThan is order-less
			case LEQ_OP: vsp[1] = Value(vsp[1].isLessThanOrEqualTo(vsp[0])); pop(vsp, 1); break; // FIX : do toPrimitive here, isLessThan is order-less
			case GT_OP: vsp[1] = Value(vsp[0].isLessThan(vsp[1])); pop(vsp, 1); break; // FIX : do toPrimitive here, isLessThan is order-less
			case GEQ_OP: vsp[1] = Value(vsp[0].isLessThanOrEqualTo(vsp[1])); pop(vsp, 1); break; // FIX : do toPrimitive here, isLessThan is order-less
			case JMP_OP: ip += readInt32(ip); break;
			case JT_OP: ip += (vsp[0].toBool() ? readInt32(ip) : 4); pop(vsp, 1); break;
			case JF_OP: ip += (!vsp[0].toBool() ? readInt32(ip) : 4); pop(vsp, 1); break;
			case RETURN_OP: {
				if (++cstackPointer == cstackEnd) {
					cstackPointer->instructionPointer = ip;
					vstackPointer = vsp;
					return true;
				}
				ip = cstackPointer->instructionPointer;
				frame = cstackPointer->frame;
				cp = frame->getFunctionDef()->getConstantsPointer();
				lp = frame->getLocalsPointer();
				break;
			}
			case CALL_OP: {
				int n = readInt32(ip);
				Object* o = vsp[n].toObject(h);
				if (o == 0) error();
				Function* f = o->asFunction();
				if (f == 0) error();
				frame = h->manage(new Frame(f));
				cstackPointer->instructionPointer = ip;
				--cstackPointer;
				cstackPointer->frame = frame;
				cstackPointer->thisValue = Value(); // FIX
				FunctionDef* def = frame->getFunctionDef();
				ip = &def->code[0];
				cp = def->getConstantsPointer();
				lp = frame->getLocalsPointer();
				for (int i = std::min(n, def->argumentsCount) - 1; i >= 0; --i) lp[i] = vsp[i];
				pop(vsp, n + 1);
				break;
			}
			case THIS_OP: push(vsp, cstackPointer->thisValue); break;
			case VOID_OP: push(vsp, Value()); break;
			case GEN_FUNC_OP: {
				push(vsp, Value(h->manage(new Function(cp[readInt32(ip)].getFunctionDef(), frame))));
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

static void disassemble(const std::vector<Byte>& code, const std::vector<Value>& constants) {
	const char* OP_STRINGS[] = {
		"POP", "REPUSH", "CONST", "READ_INDEXED", "WRITE_INDEXED", "READ_NAMED", "WRITE_NAMED", "GET_PROPERTY", "SET_PROPERTY"
		, "ADD", "SUB", "MUL", "DIV", "MOD"
		, "OR", "XOR", "AND"
		, "SHL", "SHR", "UNSIGNED_SHR"
		, "PLUS", "MINUS"
		, "BIT_NOT", "NOT"
		, "STRICTLY_EQ", "STRICTLY_NEQ", "EQ", "NEQ", "LT", "LEQ", "GT", "GEQ"
		, "JMP", "JT", "JF"
		, "CALL", "RETURN", "THIS", "VOID"
		, "GEN_FUNC"
	};
	std::vector<Byte>::const_iterator b = code.begin();
	std::vector<Byte>::const_iterator it = b;
	std::vector<Byte>::const_iterator e = code.end();
	while (it != e) {
		std::cout << (it - b) << ':' << OP_STRINGS[*it];
		switch (*it++) {
			case VM::JT_OP:
			case VM::JF_OP:
			case VM::JMP_OP: std::cout << " @" << ((it - b) + getInt32(it, e) + 4); break;
			case VM::READ_INDEXED_OP:
			case VM::WRITE_INDEXED_OP: std::cout << " $" << getInt32(it, e); break;
			case VM::CALL_OP: std::cout << " *" << getInt32(it, e); break;
			case VM::CONST_OP:
			case VM::GEN_FUNC_OP: std::cout << " #" << constants[getInt32(it, e)].toString(); break;
			case VM::READ_NAMED_OP:
			case VM::WRITE_NAMED_OP: std::cout << " " << constants[getInt32(it, e)].getString()->s; break;
		}
		std::cout << std::endl;
	}
}

/* --- Compiler --- */

enum OperatorType { GROUP, UNARY, BINARY, ASSIGNMENT, POST_UNARY, LOGICAL_AND_OR, PROPERTY_BRACKETS, PROPERTY_DOT, CONDITIONAL, FUNCTION_CALL };
enum Associativity { LEFT_TO_RIGHT, RIGHT_TO_LEFT };

struct OperatorInfo {
	const char* token;
	const char* closeToken;
	Precedence prec;
	Associativity assoc;
	OperatorType type;
	VM::Op vmOp;
};

const OperatorInfo PRE_OPS[] = {
	{ "(", ")", GROUP_PREC, RIGHT_TO_LEFT, GROUP, VM::INVALID_OP },
	{ "++", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::ADD_OP },
	{ "--", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::SUB_OP },
	{ "+", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::PLUS_OP },
	{ "+", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::PLUS_OP },
	{ "-", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::MINUS_OP },
	{ "~", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::BIT_NOT_OP },
	{ "!", 0, PREFIX_PREC, RIGHT_TO_LEFT, UNARY, VM::NOT_OP }
};

const OperatorInfo POST_OPS[] = {
	{ "===", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, VM::STRICTLY_EQ_OP },
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
	{ "!==", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, VM::STRICTLY_NEQ_OP },
	{ "!=", 0, EQUALITY_PREC, LEFT_TO_RIGHT, BINARY, VM::NEQ_OP },
	{ "<=", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, VM::LEQ_OP }, 
	{ "<", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, VM::LT_OP }, 
	{ ">=", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, VM::GEQ_OP }, 
	{ ">", 0, RELATIONAL_PREC, LEFT_TO_RIGHT, BINARY, VM::GT_OP },
	{ ".", 0, PROPERTY_PREC, LEFT_TO_RIGHT, PROPERTY_DOT, VM::INVALID_OP },
	{ "[", "]", PROPERTY_PREC, LEFT_TO_RIGHT, PROPERTY_BRACKETS, VM::INVALID_OP },
	{ "?", ":", CONDITIONAL_PREC, RIGHT_TO_LEFT, CONDITIONAL, VM::INVALID_OP },
	{ "(", ")", FUNCTION_CALL_PREC, LEFT_TO_RIGHT, FUNCTION_CALL, VM::INVALID_OP }
};

enum Statement { VAR_STATEMENT, IF_STATEMENT, WHILE_STATEMENT, DO_WHILE_STATEMENT, RETURN_STATEMENT };
/* Built with QuickHashMaker.pika */
static int findStatementToken(int n /* string length */, const char* s /* zero-terminated string */) {
	static const char* STRINGS[5] = {
		"var", "if", "while", "do", "return"
	};
	static const int QUICK_HASH_TABLE[8] = {
		2, 0, -1, -1, -1, 4, 1, 3
	};
	if (n < 2 || n > 6) return -1;
	int stringIndex = QUICK_HASH_TABLE[(s[1]) & 7];
	return (stringIndex >= 0 && strncmp(s, STRINGS[stringIndex], n) == 0 && STRINGS[stringIndex][n] == 0) ? stringIndex : -1;
};

struct Compiler::XValue {
	enum Type { NONE, PUSHED, CONSTANT, INDEXED, NAMED, PROPERTY };
	XValue() : t(NONE) { }
	XValue(Type type, const Value& value = Value()) : t(type), v(value) { }
	Type t;
	Value v;
};

Compiler::Compiler(FunctionDef* function) : function(function) { }

void Compiler::garbageCollectAndMark(GCHeap* heap) {
	heap->mark(function);
	for (std::map<Value, Int32>::iterator it = definedConstants.begin(), e = definedConstants.end(); it != e; ++it) {
		it->first.mark(heap);
	}
}

bool Compiler::isComment(const StringIt& p, const StringIt& e) {
	return (p != e && p + 1 != e && *p == '/' && (p[1] == '/' || p[1] == '*'));
}

void Compiler::eatComment(StringIt& p, const StringIt& e) {
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

void Compiler::eatWhite(StringIt& p, const StringIt& e) {
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

double Compiler::parseNumber(StringIt& p, const StringIt& e) {
	double v;
	StringIt q = parseDouble(p, e, v);
	if (q == p) throw std::runtime_error("Invalid number");
	p = q;
	return v;
}

bool Compiler::isLeadingIdentifierChar(Char c) {
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '$' || c == '_');
}

bool Compiler::eatToken(StringIt& p, const StringIt& e, const Char* t) {
	assert(*t != 0);
	StringIt q = p;
	while (*t != 0 && q != e && *q == *t) { ++q; ++t; }
	if (*t == 0 && (!isLeadingIdentifierChar(t[-1]) || (q == e || !isLeadingIdentifierChar(*q)))) {
		p = q;
		return true;
	}
	return false;
}

void Compiler::eatToken(StringIt& p, const StringIt& e) {
	while (p != e && *p >= 'a' && *p <= 'z') ++p;
}

void Compiler::makeRValue(XValue& xv) {
	switch (xv.t) {
		default: assert(0);
		case XValue::PUSHED: return;
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

Compiler::XValue Compiler::parseOperand(StringIt& p, const StringIt& e, const OperatorInfo& op) {
	XValue rv;
	parseExpression(p, e, rv, (op.closeToken != 0 ? LOWEST_PREC : op.prec));
	if (op.closeToken != 0) {
		eatWhite(p, e);
		expectToken(p, e, op.closeToken);
	}
	return rv;
}

void Compiler::parseRValueOperand(StringIt& p, const StringIt& e, const OperatorInfo& op) {
	XValue xv(parseOperand(p, e, op));
	makeRValue(xv);
}

bool Compiler::parseOperator(StringIt& p, const StringIt& e, XValue& v, Precedence precedence
		, int opCount, const OperatorInfo* opList) {
	StringIt b = p;
	int i = 0;
	while (i < opCount && !eatToken(p, e, opList[i].token)) ++i;
	if (i >= opCount) return false;
	const OperatorInfo& op = opList[i];
	if (precedence < op.prec + (op.assoc == RIGHT_TO_LEFT ? 0 : 1)) {
		p = b;
		return false;
	}
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
			std::vector<Byte>& code = function->code;
			makeRValue(v);
			emit(VM::REPUSH_OP);
			emitWithInt32(op.vmOp, 0);
			int offset = code.size() - 4;
			emit(VM::POP_OP);
			parseRValueOperand(p, e, op);
			*reinterpret_cast<Int32*>(&code[offset]) = code.size() - offset - 4;
			break;
		}
		
		case CONDITIONAL: {
			std::vector<Byte>& code = function->code;
			makeRValue(v);
			emitWithInt32(VM::JF_OP, 0);	// FIX : sub
			int offsetA = code.size() - 4;
			parseRValueOperand(p, e, op);
			emitWithInt32(VM::JMP_OP, 0);
			int offsetB = code.size() - 4;
			*reinterpret_cast<Int32*>(&code[offsetA]) = code.size() - offsetA - 4;
			parseExpression(p, e, v, op.prec);
			makeRValue(v);
			*reinterpret_cast<Int32*>(&code[offsetB]) = code.size() - offsetB - 4;
			break;			
		}
		
		case PROPERTY_DOT: {
			makeRValue(v);
			eatWhite(p, e);
			StringIt b = p;
			parseIdentifier(p, e);
			if (p == b) throw std::runtime_error("Expected identifier");
			emitWithConstant(VM::CONST_OP, Value(StringObject::newString(gcHeap(), String(b, p))));
			v = XValue(XValue::PROPERTY);
			break;
		}
		
		case FUNCTION_CALL: {
			makeRValue(v);
			eatWhite(p, e);
			int count = 0;
			while (p != e && *p != ')') {
				if (count != 0) {
					expectToken(p, e, ",");
					eatWhite(p, e);
				}
				parseExpression(p, e, v, COMMA_PREC);
				makeRValue(v);
				eatWhite(p, e);
				++count;
			}
			expectToken(p, e, ")");
			emitWithInt32(VM::CALL_OP, count);
			break;
		}
		
		default: {
			switch (op.type) {
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
					parseRValueOperand(p, e, op);
					v = XValue(XValue::PROPERTY);
					break;
				}
				
				default: assert(0);
			}
		}
	}
	return true;
}

void Compiler::parseIdentifier(StringIt& p, const StringIt& e) {
	if (p != e && isLeadingIdentifierChar(*p)) {
		while (++p != e && isLeadingIdentifierChar(*p) || (*p >= '0' && *p <= '9')) { }
	}
}

enum Literal { NULL_LITERAL, FALSE_LITERAL, TRUE_LITERAL, FUNCTION_LITERAL, THIS_LITERAL };

/* Built with QuickHashMaker.pika */
static int findLiteralToken(int n /* string length */, const char* s /* zero-terminated string */) {
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

void Compiler::expectToken(StringIt& p, const StringIt& e, const Char* token) {
	if (!eatToken(p, e, token)) throw std::runtime_error(String("Expected '") + token + "'");
}

bool Compiler::parseOptionalExpression(StringIt& p, const StringIt& e, XValue& v, Precedence precedence) {
	eatWhite(p, e);
	if (p == e) return false;
	if (!parseOperator(p, e, v, precedence, sizeof (PRE_OPS) / sizeof (*PRE_OPS), PRE_OPS)) {
		StringIt q = p;
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
					FunctionDef* func = gcHeap()->manage(new FunctionDef());
					eatWhite(p, e);
					// FIX : the optional function identifier
					expectToken(p, e, "(");
					eatWhite(p, e);
					while (p != e && *p != ')') {
						if (func->argumentsCount != 0) {
							expectToken(p, e, ",");
							eatWhite(p, e);
						}
						StringIt b = p;
						parseIdentifier(p, e);
						if (p == b) throw std::runtime_error("Expected identifier");
						func->locals[StringObject::newString(gcHeap(), String(b, p))] = func->localsCount;
						func->localsCount++;
						func->argumentsCount++;
						eatWhite(p, e);
					}
					expectToken(p, e, ")");
					eatWhite(p, e);
					expectToken(p, e, "{");
					{
						Compiler* funcCompiler = gcHeap()->manage(new Compiler(func));
						funcCompiler->parseStatementList(p, e);
						funcCompiler->finish();
					}
					expectToken(p, e, "}");
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
						StringIt q = parseHex(p + 2, e, i);
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
					v = XValue(XValue::CONSTANT, Value(StringObject::newString(gcHeap(), unescape(p, e))));
					break;
				}
				default: {
					StringIt b = p;
					parseIdentifier(p, e);
					if (p == b) return false; // fix : nicer way?
					StringObject* identifier = StringObject::newString(gcHeap(), String(b, p));
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
	StringIt q = p;
    eatWhite(q, e);
    while (parseOperator(q, e, v, precedence, sizeof (POST_OPS) / sizeof (*POST_OPS), POST_OPS)) {
		p = q;
		eatWhite(q, e);
	}
	return true;
}

void Compiler::parseExpression(StringIt& p, const StringIt& e, XValue& v, Precedence precedence) {
	if (!parseOptionalExpression(p, e, v, precedence)) throw std::runtime_error("Expected expression");
}

void Compiler::parseStatement(StringIt& p, const StringIt& e) {
	eatWhite(p, e);	
	if (eatToken(p, e, "{")) {
		parseStatementList(p, e);
		expectToken(p, e, "}");
		return;
	}
	
	StringIt q = p;
	eatToken(q, e);
	int i = findStatementToken(q - p, &*p);
	if (i < 0) {
		XValue xv;
		if (parseOptionalExpression(p, e, xv)) {
			switch (xv.t) {
				case XValue::PUSHED: emit(VM::POP_OP); break;
				case XValue::PROPERTY: emit(VM::POP_OP); emit(VM::POP_OP); break;
			}
		}
	} else {
		p = q;
		eatWhite(p, e);
		switch (static_cast<Statement>(i)) {
			case VAR_STATEMENT: {
				while (true) {
					StringIt b = p; // FIX : sub
					parseIdentifier(p, e);
					if (p == b) throw std::runtime_error("Expected identifier");
					StringObject* identifier = StringObject::newString(gcHeap(), String(b, p));
					Int32 index;
					LocalNameMap& nameMap = function->locals;
					LocalNameMap::const_iterator it = nameMap.find(identifier);
					if (it != nameMap.end()) index = it->second;
					else {
						index = function->localsCount++;
						nameMap[identifier] = index;
					}
					StringIt q = p;
					eatWhite(q, e);
					if (eatToken(q, e, "=")) {
						p = q;
						XValue v;
						parseExpression(p, e, v); // FIX : SUB
						makeRValue(v);
						emitWithInt32(VM::WRITE_INDEXED_OP, index);
						emit(VM::POP_OP);
						q = p;
						eatWhite(q, e);
					}
					if (!eatToken(q, e, ",")) break;
					p = q;
					eatWhite(p, e);
				}
				break;
			}
			
			case IF_STATEMENT: {
				std::vector<Byte>& code = function->code;
				expectToken(p, e, "(");
				XValue v; // FIX : sub
				parseExpression(p, e, v); // FIX : SUB
				makeRValue(v);
				eatWhite(p, e);
				expectToken(p, e, ")");
				emitWithInt32(VM::JF_OP, 0);	// FIX : sub
				int offsetA = code.size() - 4;
				parseStatement(p, e);
				StringIt q = p;
				eatWhite(q, e);
				if (eatToken(q, e, "else")) {
					p = q;
					emitWithInt32(VM::JMP_OP, 0);
					int offsetB = code.size() - 4;
					*reinterpret_cast<Int32*>(&code[offsetA]) = code.size() - offsetA - 4;
					parseStatement(p, e);
					*reinterpret_cast<Int32*>(&code[offsetB]) = code.size() - offsetB - 4;
				} else {
					*reinterpret_cast<Int32*>(&code[offsetA]) = code.size() - offsetA - 4;
				}				
				break;
			}

			case WHILE_STATEMENT: {
				std::vector<Byte>& code = function->code;
				int offsetB = code.size();
				expectToken(p, e, "(");
				XValue v; // FIX : sub
				parseExpression(p, e, v); // FIX : SUB
				makeRValue(v);
				eatWhite(p, e);
				expectToken(p, e, ")");
				emitWithInt32(VM::JF_OP, 0);	// FIX : sub
				int offsetA = code.size() - 4;
				parseStatement(p, e);
				emitWithInt32(VM::JMP_OP, offsetB - (code.size() + 1) - 4);
				*reinterpret_cast<Int32*>(&code[offsetA]) = code.size() - offsetA - 4;
				break;
			}

			case DO_WHILE_STATEMENT: {
				std::vector<Byte>& code = function->code;
				int offset = code.size();
				parseStatement(p, e);
				eatWhite(p, e);
				expectToken(p, e, "while");
				eatWhite(p, e);
				expectToken(p, e, "(");
				XValue v; // FIX : sub
				parseExpression(p, e, v); // FIX : SUB
				makeRValue(v);
				eatWhite(p, e);
				expectToken(p, e, ")");
				emitWithInt32(VM::JT_OP, offset - (code.size() + 1) - 4);
				break;
			}
			
			case RETURN_STATEMENT: {
				XValue v;
				if (parseOptionalExpression(p, e, v)) makeRValue(v);
				else emit(VM::VOID_OP);
				emit(VM::RETURN_OP);
				break;
			}
			
			default: assert(0);
		}
	}
	
	q = p;
	eatWhite(q, e);
	if (q != e && *q != '}' && !eatToken(q, e, ";") && std::find(p, q, '\n') == q)
		throw std::runtime_error("Syntax error");
	p = q;
}

void Compiler::parseStatementList(StringIt& p, const StringIt& e) {
	StringIt b;
	do {
		b = p;
		parseStatement(p, e);
	} while (p != e && b != p);
}

void Compiler::finish() {
	emit(VM::VOID_OP);
	emit(VM::RETURN_OP);
disassemble(function->code, function->constants);
}

int main(int argc, const char* argv[]) {
	std::vector<Value> locals;
	GCHeap* newHeap = 0;
	GCHeap* heap = new GCHeap();
	Object* globals = heap->manage(new UserObject());
	FunctionDef* functionDef = heap->manage(new FunctionDef());
	Compiler* compiler = heap->manage(new Compiler(functionDef));
	String source;
	do {
		try {
			functionDef->code.clear();
			
			String s;
			std::getline(std::cin, s);
			if (!s.empty()) {
				source += s + "\n";
			} else {
				StringIt p = source.begin();
				StringIt e = source.end();
		/*
				Compiler::XValue v;
				compiler->parseExpression(p, e, v);
				compiler->eatWhite(p, e);
				if (p != e) throw std::runtime_error("Syntax error");
				compiler->makeRValue(v);
		*/
				compiler->parseStatementList(p, e);
				compiler->finish();
				locals.resize(functionDef->localsCount);
		static bool firstTime = true;
		if (firstTime) {
			Object* o = heap->manage(new UserObject());
			locals[0] = Value(o);
			o->setProperty(StringObject::newString(heap, "sune"), Value(StringObject::newString(heap, "banan")));
			firstTime = false;
		}
				
				std::cout << "Code: " << functionDef->code.size() << std::endl;
				std::cout << "Constants: " << functionDef->constants.size() << std::endl;
				std::cout << "Locals: " << locals.size() << std::endl;
				
				Function* function = heap->manage(new Function(functionDef, globals));
				Frame* frame = heap->manage(new Frame(function));

				Value vstack[1024];
				Context cstack[1024];
				cstack[1023].instructionPointer = functionDef->code.data();
				cstack[1023].frame = frame;
				VM* vm = heap->manage(new VM(cstack, cstack + 1024, vstack, vstack + 1024));
				bool done = false;
				
				do {
					done = vm->run(10000000);
					Value v = vstack[1023];
					
					newHeap = new GCHeap();
					newHeap->mark(compiler);
					newHeap->mark(vm);
					newHeap->mark(globals);
					v.mark(newHeap);
					for (std::vector<Value>::const_iterator it = locals.begin(); it != locals.end(); ++it) it->mark(newHeap);
					for (std::vector<Value>::const_iterator it = functionDef->constants.begin(); it != functionDef->constants.end(); ++it) it->mark(newHeap);
					int sweptCount = heap->sweep();
					std::swap(heap, newHeap);
					delete newHeap;
					newHeap = 0;

					std::cout << "Kept: " << heap->size() << std::endl;
					std::cout << "Swept: " << sweptCount << std::endl;
					if (done) {
						std::cout << "On stack: " << (1024 - (vm->vstackPointer - vstack)) << std::endl;
						std::cout << "=" << (v.toString(heap))->s.c_str() << std::endl;
					}
				} while (!done);
				source = String();
			}
		}
		catch (const std::exception& x) {
			std::cout << "Exception: " << x.what() << std::endl;
			source = String();
		}
		catch (...) {
			std::cout << "Unknown exception" << std::endl;
			source = String();
		}
	} while (true);
	return 0;
}
