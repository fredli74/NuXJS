#if !defined(NuXJScript_h)
#define NuXJScript_h 1

#include "assert.h"
#include <string>

typedef char Char;
typedef std::string String;
typedef std::string::const_iterator StringIt;
typedef int Int32;
typedef unsigned int UInt32;
typedef unsigned char Byte;

class GCItem;
class GCHeap {
	public:		static GCHeap* markAndSweep(GCHeap* original);
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
	public:		virtual void garbageCollectAndMark(GCHeap* heap) { }
	public:		virtual ~GCItem() { }
	protected:	GCHeap* _gcHeap;
};

class Object;
class StringObject;
class FunctionDef;
class Function;

class Value {
	public:		enum Type {
					UNDEFINED_TYPE = 0
					, NULL_TYPE = 1
					, BOOLEAN_TYPE = 2
					, NUMBER_TYPE = 3
					, STRING_TYPE = 4
					, OBJECT_TYPE = 5
					, FUNCTION_TYPE = 6
				};
	
	public:		explicit Value(Type type = UNDEFINED_TYPE);
	public:		explicit Value(bool b);
	public:		explicit Value(Int32 i);
	public:		explicit Value(double d);
	public:		explicit Value(StringObject* s);
	public:		explicit Value(Object* o);
	
	public:		StringObject* getString() const;
	public:		FunctionDef* getFunctionDef() const;
	public:		bool toBool() const;
	public:		Int32 toInt() const;
	public:		double toDouble() const;
	public:		String toString() const;
	public:		StringObject* toString(GCHeap* heap) const;
	public:		Object* toObject(GCHeap* heap) const;
	public:		bool operator==(const Value& y) const;
	public:		bool operator<(const Value& y) const;
	public:		bool isStrictlyEqualTo(const Value& y) const;
	public:		bool isEqualTo(const Value& y) const;
	public:		bool isLessThan(const Value& y) const;
	public:		bool isLessThanOrEqualTo(const Value& y) const;
	public:		Value add(GCHeap* heap, const Value& y) const;
	public:		void mark(GCHeap* heap) const;

	protected:	bool compareStrictly(const Value& y) const;
	protected:	Type type;	
	protected:	union {
					bool b;
					double d;
					StringObject* s;
					Object* o;
				};
};

class Object : public GCItem {
	public:		virtual Function* asFunction() { return 0; } // FIX : no
	public:		virtual FunctionDef* asFunctionDef() { return 0; }
	public:		virtual bool getProperty(StringObject* name, Value& v) { return false; }
	public:		virtual bool setProperty(StringObject* name, const Value& v) { return false; }
};

class StringObject : public Object {
	public:		static StringObject* newString(GCHeap* heap, const String& s);
	public:		StringObject(const String& s);
	public:		virtual bool getProperty(StringObject* name, Value& v);
	public:		virtual bool setProperty(StringObject* name, const Value& v);
	public:		virtual ~StringObject();
	public:		String s;
};

struct StringObjectCompare {
	bool operator() (StringObject* a, StringObject* b) const {
		return (a != b && a->s < b->s);
	}
};

typedef std::map<StringObject*, Int32, StringObjectCompare> LocalNameMap;

class FunctionDef : public Object {
	public:		FunctionDef();
	public:		virtual FunctionDef* asFunctionDef();
	public:		bool lookupLocalName(StringObject* name, Int32& index) const;
	public:		virtual void garbageCollectAndMark(GCHeap* heap);
	public:		const Value* getConstantsPointer() const;
	public:		StringObject* source;
	public:		std::vector<Byte> code;
	public:		std::vector<Value> constants;
	public:		Int32 argumentsCount;
	public:		Int32 localsCount; // including arguments
	public:		LocalNameMap locals;
};

class UserObject : public Object {
	protected:	typedef std::map<StringObject*, Value, StringObjectCompare> PropertyMap;
	public:		virtual bool getProperty(StringObject* name, Value& v);
	public:		virtual bool setProperty(StringObject* name, const Value& v);
	public:		virtual void garbageCollectAndMark(GCHeap* heap);
	protected:	PropertyMap properties;
};

class Function : public UserObject {
	friend class Frame;
	public:		Function(FunctionDef* definition, Object* closure);
	public:		virtual Function* asFunction();
	public:		virtual void garbageCollectAndMark(GCHeap* heap);
	public:		FunctionDef* getDefinition() const;
	public:		Object*  getClosure() const;
	protected:	FunctionDef* const definition;
	protected:	Object* const closure;
};


class Frame;

struct Context {
	const Byte* instructionPointer;
	Frame* frame;
	Value thisValue;
};

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
					, JMP_OP, JT_OP, JF_OP
					, CALL_OP, RETURN_OP, THIS_OP, VOID_OP
					, GEN_FUNC_OP
				};
	public:		VM(Context* cstackStart, Context* cstackEnd, Value* vstackStart, Value* vstackEnd);
	public:		virtual void garbageCollectAndMark(GCHeap* heap);
	public:		bool run(Int32 count);
	protected:	void push(Value*& vsp, const Value& v);
	protected:	void pop(Value*& vsp, int count);
	protected:	Int32 readInt32(const Byte*& ip);
	protected:	void error();
	protected:	Context* const cstackStart;
	protected:	Context* const cstackEnd;
	protected:	Value* const vstackStart;
	protected:	Value* const vstackEnd;
	protected:	Context* cstackPointer;
public:	Value* vstackPointer;
};

enum Precedence {
	GROUP_PREC, PROPERTY_PREC, FUNCTION_CALL_PREC, POST_INC_DEC, PREFIX_PREC, MUL_DIV_PREC, ADD_SUB_PREC, SHIFT_PREC, RELATIONAL_PREC, EQUALITY_PREC
	, BIT_AND_PREC, BIT_XOR_PREC, BIT_OR_PREC, LOGICAL_AND_PREC, LOGICAL_OR_PREC, CONDITIONAL_PREC, ASSIGN_PREC, COMMA_PREC, LOWEST_PREC
};

class OperatorInfo;
class Compiler : public GCItem {
	public:		struct XValue;
	public:		Compiler(FunctionDef* function);
	public:		virtual void garbageCollectAndMark(GCHeap* heap);
	public:		bool isComment(const StringIt& p, const StringIt& e);
	public:		void eatComment(StringIt& p, const StringIt& e);
	public:		void eatWhite(StringIt& p, const StringIt& e);
	public:		void emit(VM::Op op);
	public:		void emitWithInt32(VM::Op op, Int32 i);
	public:		void emitWithConstant(VM::Op op, const Value& constant);
	public:		double parseNumber(StringIt& p, const StringIt& e);
	public:		XValue parseOperand(StringIt& p, const StringIt& e, const OperatorInfo& op);
	public:		void parseRValueOperand(StringIt& p, const StringIt& e, const OperatorInfo& op);
	public:		bool parseOperator(StringIt& p, const StringIt& e, XValue& v, Precedence precedence, int opCount, const OperatorInfo* opList);
	public:		void parseExpression(StringIt& p, const StringIt& e, XValue& v, Precedence precedence = LOWEST_PREC);
	public:		bool parseOptionalExpression(StringIt& p, const StringIt& e, XValue& v, Precedence precedence = LOWEST_PREC);
	public:		void parseStatement(StringIt& p, const StringIt& e);
	public:		void parseStatementList(StringIt& p, const StringIt& e);
	public:		void finish();
	protected:	bool eatToken(StringIt& p, const StringIt& e, const Char* t);
	protected:	void expectToken(StringIt& p, const StringIt& e, const Char* t);
	protected:	void eatToken(StringIt& p, const StringIt& e);
	protected:	void makeRValue(XValue& v);
	protected:	void makeAssignment(XValue& v);
	protected:	bool isLeadingIdentifierChar(Char c);
	protected:	void parseIdentifier(StringIt& p, const StringIt& e);
	protected:	FunctionDef* function;
	protected:	std::map<Value, Int32> definedConstants;
};

#endif
