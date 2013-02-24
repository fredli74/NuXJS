#if !defined(NuXJScript_h)
#define NuXJScript_h 1

#include "assert.h"
#include <string>

typedef char Char;
typedef int Int32;
typedef unsigned int UInt32;
typedef unsigned char Byte;

const UInt32 TABLE_BUILT_IN_N = 3; // 1 << 3 == 8
const UInt32 BUILT_IN_LOCALS_COUNT = 8;
const size_t STRING_BUILT_IN_LENGTH = 16;
const double EPSILON = 1.0e-300;

// FIX : name
template<typename T, size_t BUILT_IN_COUNT> class QuickArray {
	public:		QuickArray(size_t n = BUILT_IN_COUNT) : n(n), p(n <= BUILT_IN_COUNT ? a : new T[n]) { }
	public:		QuickArray(const QuickArray& copy) : n(0), p(a) { (*this) = copy; }

	public:		void swap(QuickArray& other)
				{
					if (this != &other) {
						std::swap_ranges(a + 0, a + BUILT_IN_COUNT, other.a); // Note: if this one throws, we have messed up this object, but it is not completely broken (i.e. shouldn't crash destruction).
						std::swap(n, other.n);
						T* x = p;
						p = ((other.p == other.a) ? a : other.p);
						other.p = ((x == a) ? other.a : x);
					}
				}
				
	public:		QuickArray& operator=(const QuickArray& copy)
				{
					if (this != &copy) {
						QuickArray<T, BUILT_IN_COUNT> na(copy.n);
						std::copy(copy.p, copy.p + copy.n, na.p);
						swap(na);
					}
					return *this;
				}
				
	public:		size_t size() const { return n; }
	public:		T* data() const { return p; }
	public:		operator T*() const { return p; }
	public:		~QuickArray() { if (p != a) delete [] p; }
	protected:	size_t n;
	protected:	T* p;
	protected:	T a[BUILT_IN_COUNT];
};

class GCItem;
class GCHeap {
	public:		static GCHeap* markAndSweep(GCHeap* original);
	public:		template<class T> T* manage(T* object) { add(object); return object; }
	public:		void mark(GCItem* object);
	public:		size_t sweep();
	public:		size_t size() const;
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
class String;
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
					, DELETED_TYPE = -1
				};
	
	public:		explicit Value(Type type = UNDEFINED_TYPE);
	public:		explicit Value(bool b);
	public:		explicit Value(Int32 i);
	public:		explicit Value(double d);
	public:		explicit Value(String* s);
	public:		explicit Value(Object* o);
	
	public:		bool isDeleted() const;
	public:		String* getString() const;
	public:		FunctionDef* getFunctionDef() const;
	public:		bool toBool() const;
	public:		Int32 toInt() const;
	public:		double toDouble() const;
	public:		String* toString(GCHeap* heap) const;
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
					String* s;
					Object* o;
				};
};

class Object : public GCItem {
	public:		typedef GCItem super;
	public:		virtual Function* asFunction() { return 0; } // FIX : no
	public:		virtual FunctionDef* asFunctionDef() { return 0; }
	public:		virtual bool getProperty(String* name, Value& v) { return false; }
	public:		virtual bool setProperty(String* name, const Value& v) { return false; }
};

class Table {
	protected:	static UInt32 calcMaxLoad(UInt32 bucketCount);
	protected:	static UInt32 suggestN(UInt32 count); // FIX : keep?
	public:		typedef UInt32 HashType;
	public:		struct Bucket {
					Bucket();
					HashType hash;
					String* key;
					Value value;
				};
	public:		Table(UInt32 n = TABLE_BUILT_IN_N);
	public:		const Bucket* getFirst() const;
	public:		const Bucket* getNext(const Bucket* b) const;
	public:		const Bucket* lookup(String* key);
	public:		void assign(String* key, const Value& value);
	public:		void erase(String* key);
	public:		void mark(GCHeap* heap);
	protected:	void update(String* key, const Value& value);
	protected:	Bucket* find(String* key, HashType hash) const;
	protected:	UInt32 rebuild(UInt32 newN);
	protected:	UInt32 n; ///< Size of member table is two to the power of n.
	protected:	QuickArray<Bucket, 1U << TABLE_BUILT_IN_N> buckets;
	protected:	UInt32 loadCount; ///< Count of buckets with defined keys.
};

class String : public Object {
	public:		typedef Object super;
	public:		static String* newHashedString(GCHeap* heap, const Char* b, const Char* e);
	public:		String();
	public:		String(const Char* sz);
	public:		String(const Char* b, const Char* e);
	public:		String(const String* l, const String* r);
	public:		Table::HashType calcHash() const;
	public:		bool equals(const String* o) const;
	public:		bool isLessThan(const String* o) const;
	public:		size_t getLength() const;
	public:		const Char* accessChars() const;
	public:		std::wstring toStdString() const;
	public:		virtual bool getProperty(String* name, Value& v);
	public:		virtual bool setProperty(String* name, const Value& v);
	protected:	QuickArray<Char, STRING_BUILT_IN_LENGTH> chars;
	protected:	mutable size_t hash;
};

struct StringObjectCompare {
	bool operator() (String* a, String* b) const {
		return (a != b && a->isLessThan(b));
	}
};

typedef std::map<String*, Int32, StringObjectCompare> LocalNameMap;

class FunctionDef : public Object {
	public:		typedef Object super;
	public:		FunctionDef();
	public:		virtual FunctionDef* asFunctionDef();
	public:		bool lookupLocalName(String* name, Int32& index) const;
	public:		virtual void garbageCollectAndMark(GCHeap* heap);
	public:		const Value* getConstantsPointer() const;
	public:		String* source;
	public:		std::vector<Byte> code;
	public:		std::vector<Value> constants;
	public:		Int32 argumentsCount;
	public:		Int32 localsCount; // including arguments
	public:		LocalNameMap locals;
};

class UserObject : public Object {
	public:		typedef Object super;
	public:		virtual bool getProperty(String* name, Value& v);
	public:		virtual bool setProperty(String* name, const Value& v);
	public:		virtual void garbageCollectAndMark(GCHeap* heap);
	protected:	Table properties;
};

class Function : public UserObject {
	public:		typedef UserObject super;
	friend class Frame;
	public:		Function(FunctionDef* definition, Object* closure);
	public:		virtual Function* asFunction();
	public:		virtual void garbageCollectAndMark(GCHeap* heap);
	public:		FunctionDef* getDefinition() const;
	public:		Object*  getClosure() const;
	protected:	FunctionDef* const definition;
	protected:	Object* const closure;
};

#if 0
class ArgumentsObject : public UserObject {
	public:		typedef UserObject super;
	public:		ArgumentsObject(Frame* frame);
	public:		virtual bool getProperty(String* name, Value& v);
	public:		virtual void garbageCollectAndMark(GCHeap* heap);
	protected:	Frame* frame;
};
#endif

class Frame : public Object {
	public:		typedef Object super;
	public:		Frame(Function* function);
	public:		virtual bool getProperty(String* name, Value& v);
	public:		virtual bool setProperty(String* name, const Value& v);
	public:		virtual void garbageCollectAndMark(GCHeap* heap);
	public:		FunctionDef* getFunctionDef();
	public:		Value* getLocalsPointer();
	protected:	FunctionDef* const functionDef;
	protected:	Object* const closure;
	protected:	QuickArray<Value, BUILT_IN_LOCALS_COUNT> locals;
};

struct Context {
	const Byte* instructionPointer;
	Frame* frame;
	Value thisValue;
};

class VM : public GCItem {
	public:		typedef GCItem super;
	public:		enum Op {
					INVALID_OP = -1
					, POP_OP										// stack: value ->
					, REPUSH_OP										// stack: value -> value, value
					, CONST_OP										// immediate: const index, stack: -> constant
					, READ_INDEXED_OP								// immediate: local index, stack: -> value
					, WRITE_INDEXED_OP								// immediate: local index, stack: value -> value
					, READ_NAMED_OP									// immediate: const index (name), stack: -> value
					, WRITE_NAMED_OP								// immediate: const index (name), stack: value > value
					, GET_PROPERTY_OP								// stack: name, object -> value
					, SET_PROPERTY_OP								// stack: name, object -> value
					, ADD_OP, SUB_OP, MUL_OP, DIV_OP, MOD_OP		// stack: right value, left value -> result value
					, OR_OP, XOR_OP, AND_OP							// stack: right value, left value -> result value
					, SHL_OP, SHR_OP, UNSIGNED_SHR_OP				// stack: right value, left value -> result value
					, PLUS_OP, MINUS_OP, BIT_NOT_OP, NOT_OP			// stack: value -> value
					, STRICT_EQ_OP, STRICT_NEQ_OP, EQ_OP, NEQ_OP	// stack: value -> boolean
					, LT_OP, LEQ_OP, GT_OP, GEQ_OP					// stack: value -> boolean
					, JMP_OP										// immediate: offset
					, JT_OP, JF_OP									// immediate: offset, stack: boolean ->
					, CALL_GLOBAL_OP								// immediate: n, stack: n * args, function -> return value
					, CALL_METHOD_OP								// immediate: n, stack: n * args, name, object -> return value
					, RETURN_OP
					, THIS_OP, VOID_OP								// stack: -> constant value
					, GEN_FUNC_OP									// immediate: const index, stack: -> function
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
	GROUP_PREC, PROPERTY_PREC, FUNCTION_CALL_PREC, POST_INC_DEC, PREFIX_PREC, MUL_DIV_PREC, ADD_SUB_PREC, SHIFT_PREC
	, RELATIONAL_PREC, EQUALITY_PREC, BIT_AND_PREC, BIT_XOR_PREC, BIT_OR_PREC, LOGICAL_AND_PREC, LOGICAL_OR_PREC
	, CONDITIONAL_PREC, ASSIGN_PREC, COMMA_PREC, LOWEST_PREC
};

class OperatorInfo;
class Compiler : public GCItem {
	public:		typedef GCItem super;
	public:		struct XValue;
	public:		Compiler(FunctionDef* function);
	public:		virtual void garbageCollectAndMark(GCHeap* heap);
	public:		bool isComment(const Char* p, const Char* e);
	public:		void eatComment(const Char*& p, const Char* e);
	public:		void eatWhite(const Char*& p, const Char* e);
	public:		void emit(VM::Op op);
	public:		void emitWithInt32(VM::Op op, Int32 i);
	public:		void emitWithConstant(VM::Op op, const Value& constant);
	public:		double parseNumber(const Char*& p, const Char* e);
	public:		XValue parseOperand(const Char*& p, const Char* e, const OperatorInfo& op);
	public:		void parseRValueOperand(const Char*& p, const Char* e, const OperatorInfo& op);
	public:		bool parseOperator(const Char*& p, const Char* e, XValue& v, Precedence precedence, int opCount, const OperatorInfo* opList);
	public:		void parseExpression(const Char*& p, const Char* e, XValue& v, Precedence precedence = LOWEST_PREC);
	public:		bool parseOptionalExpression(const Char*& p, const Char* e, XValue& v, Precedence precedence = LOWEST_PREC);
	public:		void parseStatement(const Char*& p, const Char* e);
	public:		void parseStatementList(const Char*& p, const Char* e);
	public:		void finish();
	protected:	bool eatToken(const Char*& p, const Char* e, const Char* t);
	protected:	void expectToken(const Char*& p, const Char* e, const Char* t);
	protected:	void eatToken(const Char*& p, const Char* e);
	protected:	void makeRValue(XValue& v);
	protected:	void makeAssignment(XValue& v);
	protected:	bool isLeadingIdentifierChar(Char c);
	protected:	void parseIdentifier(const Char*& p, const Char* e);
	protected:	FunctionDef* function;
	protected:	std::map<Value, Int32> definedConstants;
};

#endif
