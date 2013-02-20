#include "assert.h"
#include <cmath>
#include <string>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <limits>
#include <map>

typedef char Char;
typedef std::string String;
typedef std::string::const_iterator StringIt;
typedef int Int32;
typedef unsigned int UInt32;
typedef unsigned char Byte;

class GCItem;
class GCHeap {
	public:		template<class T> T* manage(T* object) { add(object); return object; }
	public:		void mark(GCItem* object);
	public:		int sweep();
	public:		int size() const { return objects.size(); }
	protected:	void add(GCItem* object);
	protected:	std::vector<GCItem*> objects;
};

class GCItem {
	friend class GCHeap;
	public:		GCItem() : _gcHeap(0) { }
	public:		GCHeap* gcHeap() const { return _gcHeap; }
	public:		virtual void gcMark(GCHeap* heap) { }
	public:		virtual ~GCItem() { }
	protected:	GCHeap* _gcHeap;
};

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
		object->gcMark(this);
	}
}

class GCString : public GCItem {
	public:		GCString(const String& s) : s(s) { }
	public:		String s;
	public:		virtual ~GCString() { std::cout << "Deleting " << s.c_str() << std::endl; }
};

double NaN() { return std::numeric_limits<double>::quiet_NaN(); }

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
static String doubleToString(double d, int precision = 16) {
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

class GCString UNDEFINED_STRING("undefined");
class GCString NULL_STRING("null");
class GCString FALSE_STRING("false");
class GCString TRUE_STRING("true");

class Object;

class Value {
	public:		explicit Value() : type(UNDEFINED_TYPE) { }
	public:		explicit Value(bool b) : type(BOOLEAN_TYPE), b(b) { }
	public:		explicit Value(Int32 i) : type(NUMBER_TYPE), d(i) { }
	public:		explicit Value(double d) : type(NUMBER_TYPE), d(d) { }
	public:		explicit Value(GCString* s) : type(STRING_TYPE), s(s) { }
	public:		explicit Value(Object* o) : type(OBJECT_TYPE), o(o) { }
	
	public:		enum Type {
					UNDEFINED_TYPE = 0
					, NULL_TYPE = 1
					, BOOLEAN_TYPE = 2
					, NUMBER_TYPE = 3
					, STRING_TYPE = 4
					, OBJECT_TYPE = 5
				};
	public:		Type type;
	
	public:		union {
					bool b;
					double d;
					GCString* s;
					Object* o;
				};
				
	public:		bool toBool() const {
					switch (type) {
						default: assert(0);
						case UNDEFINED_TYPE: return false;
						case NULL_TYPE: return false;
						case BOOLEAN_TYPE: return b;
						case NUMBER_TYPE: return (d != 0.0);
					}
				}
				
	public:		Int32 toInt() const { return static_cast<int>(toDouble()); }
	
	public:		double toDouble() const {
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
				
	public:		String toString() const {
					switch (type) {
						default: assert(0);
						case UNDEFINED_TYPE: return "undefined";
						case NULL_TYPE: return "null";
						case BOOLEAN_TYPE: return b ? "true" : "false";
						case NUMBER_TYPE: return doubleToString(d);
						case STRING_TYPE: return s->s;
					}
				}
				
	public:		GCString* toString(GCHeap* heap) const {
					switch (type) {
						default: assert(0);
						case UNDEFINED_TYPE: return &UNDEFINED_STRING;
						case NULL_TYPE: return &NULL_STRING;
						case BOOLEAN_TYPE: return b ? &TRUE_STRING : &FALSE_STRING;
						case NUMBER_TYPE: return heap->manage(new GCString(doubleToString(d)));
						case STRING_TYPE: return s;
case OBJECT_TYPE: return heap->manage(new GCString("<object>"));
					}
				}
				
	public:		Object* toObject(GCHeap* heap) const {
					switch (type) {
						default: assert(0);
						case UNDEFINED_TYPE: return 0;
						case NULL_TYPE: return 0;
						case BOOLEAN_TYPE: /* FIX */ return 0;
						case NUMBER_TYPE: /* FIX */ return 0;
						case STRING_TYPE: /* FIX */ return 0;
						case OBJECT_TYPE: return o;
					}
				}
				
	public:		bool operator==(const Value& y) const {
					if (type != y.type) return false;
					switch (type) {
						default: assert(0);
						case UNDEFINED_TYPE: return true;
						case NULL_TYPE: return true;
						case BOOLEAN_TYPE: return b == y.b;
						case NUMBER_TYPE: return d == y.d;
						case STRING_TYPE: return s->s == y.s->s;
					}
				}
				
	public:		bool operator<(const Value& y) const {
					if (type < y.type) return true;
					if (type > y.type) return false;
					switch (type) {
						default: assert(0);
						case UNDEFINED_TYPE: return false;
						case NULL_TYPE: return false;
						case BOOLEAN_TYPE: return b < y.b;
						case NUMBER_TYPE: return d < y.d;
						case STRING_TYPE: return s->s < y.s->s;
					}
				}
				
	protected:	bool compareStrictly(const Value& y) const {
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
				
	public:		bool isStrictlyEqualTo(const Value& y) const {
					if (type != y.type) return false;
					else return compareStrictly(y);
				}
				
	public:		bool isEqualTo(const Value& y) const {
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
				
	public:		bool isLessThan(const Value& y) const {
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
				
	public:		bool isLessThanOrEqualTo(const Value& y) const {
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
				
	public:		Value add(GCHeap* heap, const Value& y) const {
					if (type == STRING_TYPE || y.type == STRING_TYPE) {
						String rs = y.toString(heap)->s;
						if (rs.empty()) return *this;
						String ls = toString(heap)->s;
						if (ls.empty()) return y;
						return Value(heap->manage(new GCString(ls + rs)));
					}
					return Value(toDouble() + y.toDouble());
				}
				
	public:		void mark(GCHeap* heap) const;
};

struct ComparePropertyName {
	bool operator() (GCString* a, GCString* b) const {
		return (a != b && a->s < b->s);
	}
};

class Object : public GCItem {
	protected:	typedef std::map<GCString*, Value, ComparePropertyName> PropertyMap;
	public:		Value getProperty(GCString* name) {
					PropertyMap::const_iterator it = properties.find(name);
					if (it == properties.end()) return Value();
					else return it->second;
				}
	public:		void setProperty(GCString* name, const Value& v) { properties[name] = v; }
	public:		virtual void gcMark(GCHeap* heap) {
					for (PropertyMap::iterator it = properties.begin(), e = properties.end(); it != e; ++it) {
						heap->mark(it->first);
						it->second.mark(heap);
					}
				}
	protected:	PropertyMap properties;
};

void Value::mark(GCHeap* heap) const {
	switch (type) {
		case STRING_TYPE: heap->mark(s); break;
		case OBJECT_TYPE: heap->mark(o); break;
	}
}

class Function {
	public:		std::vector<Byte> code;
	public:		std::vector<Value> constants;
};

struct Context {
	const Byte* ip;
	Value* vsp;
	const Value* cp;
	Value* lp;
};

// !(a || (b && (c || d) && e) || f)

/*
a
JT_OP -> l1
b
JF_OP -> l2
c
JT_OP -> l3
d
l3:
l2:
JF_OP -> l4
e
l4:
l1:
JT_OP -> l5
f
l5:
*/

class VM : public GCItem {
	public:		enum Op {
					INVALID_OP = -1
					, POP_OP, REPUSH_OP, CONST_OP
					, READ_INDEXED_OP, WRITE_INDEXED_OP, READ_NAMED_OP, WRITE_NAMED_OP, GET_PROPERTY_OP, SET_PROPERTY_OP
					, ADD_OP, SUB_OP, MUL_OP, DIV_OP, MOD_OP
					, OR_OP, XOR_OP, AND_OP
					, SHL_OP, SHR_OP, UNSIGNED_SHR_OP
					, PLUS_OP, MINUS_OP
					, BIT_NOT_OP, NOT_OP
					, STRICTLY_EQ_OP, STRICTLY_NEQ_OP, EQ_OP, NEQ_OP, LT_OP, LEQ_OP, GT_OP, GEQ_OP
					, JT_OP, JF_OP
					, RETURN_OP
				};
	public:		VM(Context* cstackStart, Context* cstackEnd, Value* vstackStart, Value* vstackEnd)
						: cstackStart(cstackStart), cstackEnd(cstackEnd), vstackStart(vstackStart), vstackEnd(vstackEnd)
						, csp(cstackEnd - 1) {
				}
	public:		virtual void gcMark(GCHeap* heap) {
					if (csp == cstackEnd) return;
					for (Value* vsp = csp->vsp; vsp != vstackEnd; ++vsp) vsp->mark(heap);
				}
	protected:	void push(Value*& vsp, const Value& v) { assert(vsp > vstackStart); *--vsp = v; }
	protected:	void pop(Value*& vsp, int count) { assert(vsp + count <= vstackEnd); vsp += count; }
	protected:	Int32 readInt32(const Byte*& ip) { Int32 v = *reinterpret_cast<const Int32*>(ip); ip += 4; return v; }
	public:		bool run(Int32 count) {
					const Byte* ip = csp->ip;
					Value* vsp = csp->vsp;
					const Value* cp = csp->cp;
					Value* lp = csp->lp;
					GCHeap* h = gcHeap();
					while (--count >= 0) {
						switch (*ip++) {
							case POP_OP: pop(vsp, 1); break;
							case REPUSH_OP: push(vsp, vsp[0]); break;
							case CONST_OP: push(vsp, cp[readInt32(ip)]); break;
							case READ_INDEXED_OP: push(vsp, lp[readInt32(ip)]); break;
							case WRITE_INDEXED_OP: lp[readInt32(ip)] = vsp[0]; break;
							case READ_NAMED_OP: break;
							case WRITE_NAMED_OP: break;
							case GET_PROPERTY_OP: vsp[1] = vsp[1].toObject(h)->getProperty(vsp[0].toString(h)); pop(vsp, 1); break;
							case SET_PROPERTY_OP: vsp[2].toObject(h)->setProperty(vsp[1].toString(h), vsp[0]); vsp[2] = vsp[0]; pop(vsp, 2); break;
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
							case JT_OP: if (vsp[0].toBool()) ip += readInt32(ip); else { ip += 4; pop(vsp, 1); } break;
							case JF_OP: if (!vsp[0].toBool()) ip += readInt32(ip); else { ip += 4; pop(vsp, 1); } break;
							case RETURN_OP: {
								csp->ip = ip;
								csp->vsp = vsp;
								if (++csp == cstackEnd) return true;
								ip = csp->ip;
								vsp = csp->vsp;
								cp = csp->cp;
								lp = csp->lp;
								break;
							}
						}
					}
					csp->ip = ip;
					csp->vsp = vsp;
					return false;
				}
	protected:	Context* const cstackStart;
	protected:	Context* const cstackEnd;
	protected:	Value* const vstackStart;
	protected:	Value* const vstackEnd;
	protected:	Context* csp;
};

enum Precedence {
	GROUP_PREC, PROPERTY_PREC, POST_INC_DEC, PREFIX_PREC, MUL_DIV_PREC, ADD_SUB_PREC, SHIFT_PREC, RELATIONAL_PREC, EQUALITY_PREC
	, BIT_AND_PREC, BIT_XOR_PREC, BIT_OR_PREC, LOGICAL_AND_PREC, LOGICAL_OR_PREC, ASSIGN_PREC, COMMA_PREC, LOWEST_PREC
};
enum OperatorType { GROUP, UNARY, BINARY, ASSIGNMENT, POST_UNARY, LOGICAL_AND_OR, PROPERTY_BRACKETS, PROPERTY_DOT };
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
	{ "[", "]", PROPERTY_PREC, LEFT_TO_RIGHT, PROPERTY_BRACKETS, VM::INVALID_OP }
};

class Compiler : public GCItem {
	public:		struct XValue {
					enum Type { STACKED, CONSTANT, INDEXED, PROPERTY };
					XValue() : t(STACKED) { }
					XValue(Type type, const Value& value = Value()) : t(type), v(value) { }
					Type t;
					Value v;
				};

	public:		Compiler(std::vector<Byte>& code, std::vector<Value>& constants
						, std::vector<Value>& locals) : code(code), constants(constants), locals(locals) { }
	public:		virtual void gcMark(GCHeap* heap) {
					for (std::map<Value, Int32>::iterator it = definedConstants.begin(), e = definedConstants.end(); it != e; ++it) {
						it->first.mark(heap);
					}
				}
	public:		bool isComment(const StringIt& p, const StringIt& e);
	public:		void eatComment(StringIt& p, const StringIt& e);
	public:		void eatWhite(StringIt& p, const StringIt& e);
	public:		void emit(VM::Op op);
	public:		void emitWithInt32(VM::Op op, Int32 i);
	public:		void emitWithConstant(VM::Op op, const Value& constant);
	public:		double parseNumber(StringIt& p, const StringIt& e);
	public:		bool token(StringIt& p, const StringIt& e, const Char* t);
	public:		void makeRValue(XValue& v);
	public:		void makeAssignment(XValue& v);
	public:		XValue parseOperand(StringIt& p, const StringIt& e, const OperatorInfo& op);
	public:		void parseRValueOperand(StringIt& p, const StringIt& e, const OperatorInfo& op);
	public:		bool parseOperator(StringIt& p, const StringIt& e, XValue& v, Precedence precedence, int opCount, const OperatorInfo* opList);
	public:		void parseExpression(StringIt& p, const StringIt& e, XValue& v, Precedence precedence = LOWEST_PREC);
	protected:	String parseIdentifier(StringIt& p, const StringIt& e);
	public:		std::vector<Byte>& code;
	public:		std::vector<Value>& constants;
	public:		std::vector<Value>& locals;
	public:		std::map<Value, Int32> definedConstants;
	public:		std::map<String, Int32> localIdentifiers;
};

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
	code.push_back(op);
}

void Compiler::emitWithInt32(VM::Op op, Int32 i) {
	code.push_back(op);
	code.resize(code.size() + 4);
	*reinterpret_cast<Int32*>(&code[code.size() - 4]) = i;
}

void Compiler::emitWithConstant(VM::Op op, const Value& constant) {
	std::map<Value, Int32>::const_iterator it = definedConstants.find(constant);
	if (it == definedConstants.end()) {
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

bool Compiler::token(StringIt& p, const StringIt& e, const Char* t) {
	StringIt q = p;
	while (*t != 0 && q != e && *q == *t) { ++q; ++t; }
	if (*t == 0) {
		p = q;
		return true;
	}
	return false;
}

void Compiler::makeRValue(XValue& xv) {
	switch (xv.t) {
		default: assert(0);
		case XValue::STACKED: return;
		case XValue::CONSTANT: emitWithConstant(VM::CONST_OP, xv.v); break;
		case XValue::INDEXED: emitWithInt32(VM::READ_INDEXED_OP, xv.v.toInt()); break;
		case XValue::PROPERTY: emit(VM::GET_PROPERTY_OP); break;
	}
	xv = XValue(XValue::STACKED);
}

void Compiler::makeAssignment(XValue& xv) {
	switch (xv.t) {
		default: throw std::runtime_error("Invalid lvalue");
		case XValue::INDEXED: emitWithInt32(VM::WRITE_INDEXED_OP, xv.v.toInt()); break;
		case XValue::PROPERTY: emit(VM::SET_PROPERTY_OP); break;
	}
	xv = XValue(XValue::STACKED);
}

Compiler::XValue Compiler::parseOperand(StringIt& p, const StringIt& e, const OperatorInfo& op) {
	XValue rv;
	parseExpression(p, e, rv, (op.closeToken != 0 ? LOWEST_PREC : op.prec));
	if (op.closeToken != 0 && !token(p, e, op.closeToken)) {
		throw std::runtime_error(std::string("Expected ") + op.closeToken);
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
	while (i < opCount && !token(p, e, opList[i].token)) ++i;
	if (i >= opCount) return false;
	const OperatorInfo& op = opList[i];
	if (precedence < op.prec + (op.assoc == RIGHT_TO_LEFT ? 0 : 1)) {
		p = b;
		return false;
	}
	switch (op.type) {
		case POST_UNARY: {
			switch (v.t) {
				default: throw std::runtime_error("Invalid lvalue");
				case XValue::INDEXED: {
					XValue lv(v);
					makeRValue(lv);
					emit(VM::REPUSH_OP);
					emitWithConstant(VM::CONST_OP, Value(1.0)); 
					emit(op.vmOp);
					makeAssignment(v);
					emit(VM::POP_OP);
				}
				break;
			}
			break;
		}
		case LOGICAL_AND_OR: {
			makeRValue(v);
			emitWithInt32(op.vmOp, 0);
			int offset = code.size() - 4;
			parseExpression(p, e, v, op.prec);
			makeRValue(v);
			*reinterpret_cast<Int32*>(&code[offset]) = code.size() - offset - 4;
			break;
		}
		case PROPERTY_DOT: {
			makeRValue(v);
			eatWhite(p, e);
			emitWithConstant(VM::CONST_OP, Value(gcHeap()->manage(new GCString(parseIdentifier(p, e)))));
			v = XValue(XValue::PROPERTY);
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

String Compiler::parseIdentifier(StringIt& p, const StringIt& e) {
	StringIt b = p;
	if (p != e && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '$' || *p == '_')) {
		while (++p != e && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9')
				|| *p == '$' || *p == '_')) { }
	}
	if (p == b) throw std::runtime_error("Invalid identifier");
	return String(b, p);
}

void Compiler::parseExpression(StringIt& p, const StringIt& e, XValue& v, Precedence precedence) {
    eatWhite(p, e);
	if (p == e) throw std::runtime_error("Unexpected end of string");
	if (!parseOperator(p, e, v, precedence, sizeof (PRE_OPS) / sizeof (*PRE_OPS), PRE_OPS)) {
		assert(p != e);
		switch (*p) {
			case '0':	if (p + 1 != e && p[1] == 'x') {
							UInt32 i;
							StringIt q = parseHex(p + 2, e, i);
							if (q == p) throw std::runtime_error("Invalid number");
							p = q;
							v = XValue(XValue::CONSTANT, Value((Int32)(i)));
							break;
						}
						/* else continue */
			case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '.': {
				double d = parseNumber(p, e);
				v = XValue(XValue::CONSTANT, Value(d));
				break;
			}
			case '\'': case '"': {
				v = XValue(XValue::CONSTANT, Value(gcHeap()->manage(new GCString(unescape(p, e)))));
				break;
			}
			default: {
				String identifier = parseIdentifier(p, e);
				Int32 index;
				std::map<String, Int32>::const_iterator it = localIdentifiers.find(identifier);
				if (it != localIdentifiers.end()) index = it->second;
				else {
					index = locals.size();
					locals.push_back(Value());
					localIdentifiers[identifier] = index;
				}
				v = XValue(XValue::INDEXED, Value(index));
				break;
			}
		}
    }
    eatWhite(p, e);
    while (parseOperator(p, e, v, precedence, sizeof (POST_OPS) / sizeof (*POST_OPS), POST_OPS)) { }
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
		, "JT", "JF"
		, "RETURN"
	};
	std::vector<Byte>::const_iterator b = code.begin();
	std::vector<Byte>::const_iterator it = b;
	std::vector<Byte>::const_iterator e = code.end();
	while (it != e) {
		std::cout << (it - b) << ':' << OP_STRINGS[*it];
		switch (*it++) {
			case VM::JT_OP:
			case VM::JF_OP: std::cout << " @" << ((it - b) + getInt32(it, e) + 4); break;
			case VM::READ_INDEXED_OP:
			case VM::WRITE_INDEXED_OP: std::cout << " $" << getInt32(it, e); break;
			case VM::CONST_OP: std::cout << " #" << getInt32(it, e); break;
		}
		std::cout << std::endl;
	}
}

int main(int argc, const char* argv[]) {
	std::vector<Byte> code;
	std::vector<Value> constants;
	std::vector<Value> locals;
	GCHeap* newHeap = 0;
	GCHeap* heap = new GCHeap();
	Compiler* compiler = heap->manage(new Compiler(code, constants, locals));
	do {
		try {
			code.clear();
			
			String s;
			std::getline(std::cin, s);
			if (s.empty()) return 0;
			
			StringIt p = s.begin();
			StringIt e = s.end();
			Compiler::XValue v;
			compiler->parseExpression(p, e, v);
			compiler->eatWhite(p, e);
			if (p != e) throw std::runtime_error("Syntax error");
			compiler->makeRValue(v);
			code.push_back(VM::RETURN_OP);
			disassemble(code, constants);
	static bool firstTime = true;
	if (firstTime) {
		Object* o = heap->manage(new Object());
		locals[0] = Value(o);
		o->setProperty(heap->manage(new GCString("sune")), Value(heap->manage(new GCString("banan"))));
		firstTime = false;
	}
			
			std::cout << "Code: " << code.size() << std::endl;
			std::cout << "Constants: " << constants.size() << std::endl;
			std::cout << "Locals: " << locals.size() << std::endl;

			Value vstack[1024];
			Context cstack[1024];
			cstack[1023].ip = code.data();
			cstack[1023].vsp = &vstack[1024];
			cstack[1023].cp = constants.data();
			cstack[1023].lp = locals.data();
			VM* vm = heap->manage(new VM(cstack, cstack + 1024, vstack, vstack + 1024));
			bool done = false;
			
			do {
				done = vm->run(1000);
				Value v = vstack[1023];
				
				newHeap = new GCHeap();
				newHeap->mark(compiler);
				newHeap->mark(vm);
				v.mark(newHeap);
				for (std::vector<Value>::const_iterator it = locals.begin(); it != locals.end(); ++it) it->mark(newHeap);
				for (std::vector<Value>::const_iterator it = constants.begin(); it != constants.end(); ++it) it->mark(newHeap);
				int sweptCount = heap->sweep();
				std::swap(heap, newHeap);
				delete newHeap;
				newHeap = 0;

				std::cout << "Kept: " << heap->size() << std::endl;
				std::cout << "Swept: " << sweptCount << std::endl;
				if (done) {
					std::cout << "On stack: " << (1023 - (cstack[1023].vsp - vstack)) << std::endl;
					std::cout << "=" << (v.toString(heap))->s.c_str() << std::endl;
				}
			} while (!done);
		}
		catch (const std::exception& x) {
			std::cout << "Exception: " << x.what() << std::endl;
		}
		catch (...) {
			std::cout << "Unknown exception" << std::endl;
		}
	} while (true);
	return 0;
}
