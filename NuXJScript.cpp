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
typedef unsigned int UInt32;
typedef unsigned char Byte;

double NaN() { return std::numeric_limits<double>::quiet_NaN(); }

class Value {
	public:		Value() : format(UNDEFINED_FORMAT) { }
	public:		Value(const int i) : format(INT_FORMAT), i(i) { }
	public:		Value(const double d) : format(DOUBLE_FORMAT), d(d) { }
	public:		enum Format {
					UNDEFINED_FORMAT
					, NULL_FORMAT
					, BOOLEAN_FORMAT
					, INT_FORMAT
					, DOUBLE_FORMAT
					, STRING_FORMAT
				};
	public:		Format format;
	public:		union {
					bool b;
					int i;
					double d;
					String* s;
				};
	public:		operator int() const {
					switch (format) {
						default: assert(0);
						case UNDEFINED_FORMAT: return NaN();
						case NULL_FORMAT: return 0;
						case BOOLEAN_FORMAT: return b ? 1 : 0;
						case INT_FORMAT: return i;
						case DOUBLE_FORMAT: return static_cast<int>(d);
					}
				}
	public:		operator double() const {
					switch (format) {
						default: assert(0);
						case UNDEFINED_FORMAT: return NaN();
						case NULL_FORMAT: return 0.0;
						case BOOLEAN_FORMAT: return b ? 1.0 : 0.0;
						case INT_FORMAT: return i;
						case DOUBLE_FORMAT: return d;
					}
				}
};

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

class VM {
	public:		enum Op {
					NOP = -1
					, CONST_OP
					, READ_INDEXED_OP
					, WRITE_INDEXED_OP
					, READ_NAMED_OP
					, WRITE_NAMED_OP
					, ADD_OP
					, SUB_OP
					, MUL_OP
					, DIV_OP
					, MOD_OP
					, OR_OP
					, XOR_OP
					, AND_OP
					, PLUS_OP
					, NEG_OP
					, RETURN_OP
				};
	public:		VM(Context* cstackStart, Context* cstackEnd, Value* vstackStart, Value* vstackEnd)
						: cstackStart(cstackStart), cstackEnd(cstackEnd), vstackStart(vstackStart), vstackEnd(vstackEnd)
						, csp(cstackEnd - 1) {
				}
	protected:	void push(Value*& vsp, const Value& v) { assert(vsp > vstackStart); *--vsp = v; }
	protected:	const Value& pop(Value*& vsp) { assert(vsp < vstackEnd); return *vsp++; }
	protected:	void pop(Value*& vsp, int count) { assert(vsp + count <= vstackEnd); vsp += count; }
	protected:	UInt32 readUInt32(const Byte*& ip) { UInt32 v = *reinterpret_cast<const UInt32*>(ip); ip += 4; return v; }
	public:		void run(int count) {
					const Byte* ip = csp->ip;
					Value* vsp = csp->vsp;
					const Value* cp = csp->cp;
					Value* lp = csp->lp;
					while (--count >= 0) {
						switch (*ip++) {
							case CONST_OP: push(vsp, cp[readUInt32(ip)]); break;
							case READ_INDEXED_OP: push(vsp, lp[readUInt32(ip)]); break;
							case WRITE_INDEXED_OP: lp[readUInt32(ip)] = pop(vsp); break;
							case READ_NAMED_OP: break;
							case WRITE_NAMED_OP: break;
							case ADD_OP: vsp[1] = (double)vsp[1] + (double)vsp[0]; pop(vsp, 1); break;
							case SUB_OP: vsp[1] = (double)vsp[1] - (double)vsp[0]; pop(vsp, 1); break;
							case MUL_OP: vsp[1] = (double)vsp[1] * (double)vsp[0]; pop(vsp, 1); break;
							case DIV_OP: vsp[1] = (double)vsp[1] / (double)vsp[0]; pop(vsp, 1); break;
							case MOD_OP: vsp[1] = fmod(vsp[1], vsp[0]); pop(vsp, 1); break;
							case OR_OP: vsp[1] = vsp[1] | vsp[0]; pop(vsp, 1); break;
							case XOR_OP: vsp[1] = vsp[1] ^ vsp[0]; pop(vsp, 1); break;
							case AND_OP: vsp[1] = vsp[1] & vsp[0]; pop(vsp, 1); break;
							case PLUS_OP: vsp[0] = (double)vsp[0]; break;
							case NEG_OP: vsp[0] = -(double)vsp[0]; break;
							case RETURN_OP: {
								if (++csp == cstackEnd) return;
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
				}
	protected:	Context* const cstackStart;
	protected:	Context* const cstackEnd;
	protected:	Value* const vstackStart;
	protected:	Value* const vstackEnd;
	protected:	Context* csp;
};

enum Precedence { GROUP_PREC, PREFIX_PREC, MUL_DIV_PREC, ADD_SUB_PREC, BIT_AND_PREC, BIT_XOR_PREC, BIT_OR_PREC, LOWEST_PREC };
enum OperatorType { UNARY, BINARY };
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
	{ "(", ")", GROUP_PREC, RIGHT_TO_LEFT, UNARY, VM::NOP },
	{ "+", 0, PREFIX_PREC, LEFT_TO_RIGHT, UNARY, VM::PLUS_OP },
	{ "-", 0, PREFIX_PREC, LEFT_TO_RIGHT, UNARY, VM::NEG_OP }
};

const OperatorInfo POST_OPS[] = {
	{ "+", 0, ADD_SUB_PREC, LEFT_TO_RIGHT, BINARY, VM::ADD_OP },
	{ "-", 0, ADD_SUB_PREC, LEFT_TO_RIGHT, BINARY, VM::SUB_OP },
	{ "*", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, VM::MUL_OP },
	{ "/", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, VM::DIV_OP },
	{ "%", 0, MUL_DIV_PREC, LEFT_TO_RIGHT, BINARY, VM::MOD_OP },
	{ "&", 0, BIT_AND_PREC, LEFT_TO_RIGHT, BINARY, VM::AND_OP },
	{ "^", 0, BIT_XOR_PREC, LEFT_TO_RIGHT, BINARY, VM::XOR_OP },
	{ "|", 0, BIT_OR_PREC, LEFT_TO_RIGHT, BINARY, VM::OR_OP }
};

class Compiler {
	public:		struct ExpressionValue {
					enum Type {
						STACKED
						, CONSTANT
						, INDEXED
					};
					Type t;
					Value v;
				};

	public:		Compiler(std::vector<Byte>& code, std::vector<Value>& constants
						, std::vector<Value>& locals) : code(code), constants(constants), locals(locals) { }
	public:		StringIt parseHex(StringIt p, const StringIt& e, unsigned int& i);
	public:		StringIt parseUnsignedInt(StringIt p, const StringIt& e, unsigned int& i);
	public:		StringIt parseInt(StringIt p, const StringIt& e, int& i);
	public:		StringIt parseDouble(StringIt p, const StringIt& e, double& v);
	public:		bool isComment(const StringIt& p, const StringIt& e);
	public:		void eatComment(StringIt& p, const StringIt& e);
	public:		void eatWhite(StringIt& p, const StringIt& e);
	public:		void emit(VM::Op op);
	public:		void emitWithUInt32(VM::Op op, UInt32 i);
	public:		void emitWithConstant(VM::Op op, const Value& constant);
	public:		double parseNumber(StringIt& p, const StringIt& e);
	public:		bool token(StringIt& p, const StringIt& e, const Char* t);
	public:		bool parseOperator(StringIt& p, const StringIt& e, ExpressionValue& v, Precedence precedence, int opCount, const OperatorInfo* opList);
	public:		void parseExpression(StringIt& p, const StringIt& e, ExpressionValue& v, Precedence precedence = LOWEST_PREC);
	public:		std::vector<Byte>& code;
	public:		std::vector<Value>& constants;
	public:		std::vector<Value>& locals;
	public:		std::map<String, int> localIdentifiers;
};

StringIt Compiler::parseHex(StringIt p, const StringIt& e, unsigned int& i) {
	for (i = 0; p != e && ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F') || (*p >= 'a' && *p <= 'f')); ++p)
		i = (i << 4) + (*p <= '9' ? *p - '0' : (*p & ~0x20) - ('A' - 10));
	return p;
}

StringIt Compiler::parseUnsignedInt(StringIt p, const StringIt& e, unsigned int& i) {
	for (i = 0; p != e && *p >= '0' && *p <= '9'; ++p) i = i * 10 + (*p - '0');
	return p;
}

StringIt Compiler::parseInt(StringIt p, const StringIt& e, int& i) {
	bool negative = (e - p >= 2 && ((*p == '+' || *p == '-') && p[1] >= '0' && p[1] <= '9') ? (*p++ == '-') : false);
	unsigned int ui;
	p = parseUnsignedInt(p, e, ui);
	i = (negative ? -static_cast<int>(ui) : ui);
	return p;
}

StringIt Compiler::parseDouble(StringIt p, const StringIt& e, double& v) {
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
		int i;
		StringIt t = parseInt(q + 1, e, i);
		if (t != q + 1) { d *= pow(10, static_cast<double>(i)); q = t; }
	}
	v = d * sign;
	return q;
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
	code.push_back(op);
}

void Compiler::emitWithUInt32(VM::Op op, UInt32 i) {
	code.push_back(op);
	code.resize(code.size() + 4);
	*reinterpret_cast<UInt32*>(&code[code.size() - 4]) = i;
}

void Compiler::emitWithConstant(VM::Op op, const Value& constant) {
	constants.push_back(constant);
	emitWithUInt32(op, constants.size() - 1);
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

bool Compiler::parseOperator(StringIt& p, const StringIt& e, ExpressionValue& v, Precedence precedence
		, int opCount, const OperatorInfo* opList) {
	for (int i = 0; i < opCount; ++i) {
		if (precedence > opList[i].prec - (opList[i].assoc == RIGHT_TO_LEFT ? 1 : 0)
				&& token(p, e, opList[i].token)) {
			parseExpression(p, e, v, (opList[i].closeToken != 0 ? LOWEST_PREC : opList[i].prec));
			if (opList[i].closeToken != 0 && !token(p, e, opList[i].closeToken)) {
				throw std::runtime_error(std::string("Expected ") + opList[i].closeToken);
			}
			if (opList[i].vmOp != VM::NOP) emit(opList[i].vmOp);
			return true;
		}
	}
	return false;
}

void Compiler::parseExpression(StringIt& p, const StringIt& e, ExpressionValue& v, Precedence precedence) {
    eatWhite(p, e);
	if (p == e) throw std::runtime_error("Unexpected end of string");
	if (!parseOperator(p, e, v, precedence, sizeof (PRE_OPS) / sizeof (*PRE_OPS), PRE_OPS)) {
		assert(p != e);
		switch (*p) {
			case '0':	if (p + 1 != e && p[1] == 'x') {
							unsigned int i;
							StringIt q = parseHex(p + 2, e, i);
							if (q == p) throw std::runtime_error("Invalid number");
							p = q;
							emitWithConstant(VM::CONST_OP, static_cast<int>(i));
							break;
						}
						/* else continue */
			case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '.': {
				Value v;
				double d = parseNumber(p, e);
				emitWithConstant(VM::CONST_OP, (static_cast<int>(d) == d) ? Value(static_cast<int>(d)) : Value(d));
				break;
			}
			default: {
				StringIt q = p;
				while (q != e && ((*q >= 'a' && *q <= 'z') || (*q >= 'A' && *q <= 'Z') || (*q >= '0' && *q <= '9')
						|| *q == '$' || *q == '_')) ++q;
				if (q == p) throw std::runtime_error("Invalid identifier");
				String identifier(p, q);
				int index;
				std::map<String, int>::const_iterator it = localIdentifiers.find(identifier);
				if (it != localIdentifiers.end()) index = it->second;
				else {
					index = locals.size();
					locals.push_back(Value());
					localIdentifiers[identifier] = index;
				}
				emitWithUInt32(VM::READ_INDEXED_OP, index);
				p = q;
				break;
			}
		}
    }
    eatWhite(p, e);
    while (parseOperator(p, e, v, precedence, sizeof (POST_OPS) / sizeof (*POST_OPS), POST_OPS)) { }
}

int main(int argc, const char* argv[]) {
	do {
		try {
			std::vector<Byte> code;
			std::vector<Value> constants;
			std::vector<Value> locals;
			Compiler compiler(code, constants, locals);
			String s;
			std::getline(std::cin, s);
			if (s.empty()) return 0;
			
			StringIt p = s.begin();
			StringIt e = s.end();
			Compiler::ExpressionValue v;
			compiler.parseExpression(p, e, v);
			code.push_back(VM::RETURN_OP);

			Value vstack[1024];
			Context cstack[1024];
			cstack[1023].ip = code.data();
			cstack[1023].vsp = &vstack[1024];
			cstack[1023].cp = constants.data();
			cstack[1023].lp = locals.data();
			VM vm(cstack, cstack + 1024, vstack, vstack + 1024);
			for (int i = 0; i < locals.size(); ++i) locals[i] = i;
			vm.run(1000);
			std::cout << static_cast<double>(vstack[1023]) << std::endl;
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
