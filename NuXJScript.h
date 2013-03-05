#if !defined(NuXJScript_h)
#define NuXJScript_h 1

#include "assert.h"
#include <string>

typedef unsigned short Char;
typedef int Int32;
typedef unsigned int UInt32;
typedef unsigned char Byte;

const UInt32 TABLE_BUILT_IN_N = 3; // 1 << 3 == 8
const UInt32 BUILT_IN_LOCALS_COUNT = 8;
const size_t STRING_BUILT_IN_LENGTH = 16;
const double EPSILON = 1.0e-300;

// FIX : name
template<typename T, size_t BUILT_IN_COUNT> class QuickArray {
	public:		explicit QuickArray(size_t n = BUILT_IN_COUNT) : n(n), p(n <= BUILT_IN_COUNT ? a : new T[n]) { }
	public:		explicit QuickArray(const QuickArray& copy) : n(0), p(a) { (*this) = copy; }

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
				
	public:		bool empty() const { return n == 0; }
	public:		size_t size() const { return n; }
	public:		operator T*() const { return p; }
	public:		T* begin() const { return p; }
	public:		T* end() const { return p + n; }
	public:		~QuickArray() { if (p != a) delete [] p; }
	protected:	size_t n;
	protected:	T* p;
	protected:	T a[BUILT_IN_COUNT];
};

class GCList;
class GCHeap;
class GCItem {
	friend class GCList;
	friend class GCHeap; // FIX : move stuff to GCList so GCHeap doesn't have to be friend
	public:		GCItem();
	public:		virtual void gcMark(GCHeap& heap);
	public:		virtual ~GCItem();
	protected:	GCList* _gcList;
	protected:	GCItem* _gcPrev;
	protected:	GCItem* _gcNext;
};

class GCList : public GCItem {
	public:		GCList();
	public:		size_t size() const;
	public:		void claim(GCItem* item);
	public:		void relinquish(GCItem* item);
	protected:	size_t count;
};

class GCHeap {
	public:		template<class T> T* manage(T* item) { currentList->claim(item); return item; }
	public:		template<class T> T* protect(T* item) { rootList.claim(item); return item; }
	public:		GCHeap();
	public:		size_t size() const;
	public:		size_t gc();
	public:		void mark(const GCItem* item);
	protected:	GCList* currentList;
	protected:	GCList* newList;
	protected:	GCList managedLists[2];
	protected:	GCList rootList;
};

class Engine;
class Function;
class String;
class Object;
class ScriptFunctionDef;

class Value {
	public:		enum Type {
					NONEXISTENT_TYPE		// Used in tables for deleted entries. Never seen by script-code.
					, UNDEFINED_TYPE
					, NULL_TYPE
					, BOOLEAN_TYPE
					, NUMBER_TYPE
					, STRING_TYPE
					, OBJECT_TYPE
				};
	
	public:		explicit Value(Type type = NONEXISTENT_TYPE);
	private:	explicit Value(void* v); // N/A. Just to avoid unwanted casting of a pointer to bool.
	public:		explicit Value(bool b);
	public:		explicit Value(Int32 i);
	public:		explicit Value(double d);
	public:		explicit Value(String* s);
	public:		explicit Value(Object* o);
	
	public:		bool exists() const;
	public:		bool isObject() const;
	public:		String* getString() const;
	public:		ScriptFunctionDef* getFunctionDef() const;
	public:		bool toBool() const;
	public:		Int32 toInt() const;
	public:		double toDouble() const;
	public:		String* toString(GCHeap& heap) const;
	public:		Object* toObject(GCHeap& heap) const;
	public:		bool operator==(const Value& y) const;
	public:		bool operator<(const Value& y) const;
	public:		bool isStrictlyEqualTo(const Value& y) const;
	public:		bool isEqualTo(const Value& y) const;
	public:		bool isLessThan(const Value& y) const;
	public:		bool isLessThanOrEqualTo(const Value& y) const;
	public:		Value add(GCHeap& heap, const Value& y) const;
	public:		void mark(GCHeap& heap) const;

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
	public:		virtual bool hasProperty(Engine& engine, const String* name) { Value v; return getProperty(engine, name, v); }
	public:		virtual bool getProperty(Engine& engine, const String* name, Value& v) { return false; }
	public:		virtual bool setProperty(Engine& engine, const String* name, const Value& v) { return false; }
	public:		virtual bool deleteProperty(Engine& engine, const String* name) { return false; }
};

class Table {
	protected:	static UInt32 calcMaxLoad(UInt32 bucketCount);
	protected:	static UInt32 suggestN(UInt32 count); // FIX : keep?
	public:		typedef UInt32 HashType;
	public:		struct Bucket {
					Bucket();
					HashType hash;
					const String* key;
					Value value;
				};
	public:		Table(UInt32 n = TABLE_BUILT_IN_N);
	public:		const Bucket* getFirst() const;
	public:		const Bucket* getNext(const Bucket* b) const;
	public:		const Bucket* lookup(const String* key);
	public:		void assign(const String* key, const Value& value);
	public:		void erase(const String* key);
	public:		void mark(GCHeap& heap);
	protected:	void update(const String* key, const Value& value);
	protected:	Bucket* find(const String* key, HashType hash) const;
	protected:	UInt32 rebuild(UInt32 newN);
	protected:	UInt32 n; ///< Size of member table is two to the power of n.
	protected:	QuickArray<Bucket, 1U << TABLE_BUILT_IN_N> buckets;
	protected:	UInt32 loadCount; ///< Count of buckets with defined keys.
};

class String : public Object, public QuickArray<Char, STRING_BUILT_IN_LENGTH> {
	public:		typedef Object super;
	public:		typedef QuickArray<Char, STRING_BUILT_IN_LENGTH> QArray;
	public:		static String* newHashedString(GCHeap& heap, const Char* b, const Char* e);
	public:		String();
	public:		String(const String& o);
	public:		String(const Char* b, const Char* e);
	public:		String(const String& l, const String& r);
	public:		String(const char* sz);
	public:		Table::HashType calcHash() const;
	public:		bool isEqualTo(const String& o) const;
	public:		bool isLessThan(const String& o) const;
	public:		const Char* accessChars() const; // FIX : <- drop
	public:		std::wstring toStdString() const;
	public:		virtual bool getProperty(Engine& engine, const String* name, Value& v);
	protected:	mutable size_t hash;
};

struct StringObjectCompare {
	bool operator() (const String* a, const String* b) const {
		return (a != b && a->isLessThan(*b));
	}
};

typedef std::map<const String*, Int32, StringObjectCompare> LocalNameMap; // FIX : use table

typedef std::vector<Byte> CodeVector;

// This is an Object so that we can store it as a constant
class ScriptFunctionDef : public Object {
	public:		typedef Object super;
	public:		ScriptFunctionDef();
	public:		virtual void gcMark(GCHeap& heap);
	public:		bool lookupLocalName(const String* name, Int32& index) const;
	public:		const Value* getConstantsPointer() const;
// FIX : protectize
	public:		String* source;
	public:		CodeVector code;
	public:		std::vector<Value> constants;
	public:		Int32 argumentsCount;
	public:		Int32 localsCount; // including arguments
	public:		LocalNameMap localNames;
};

class UserObject : public Object {
	public:		typedef Object super;
	public:		virtual void gcMark(GCHeap& heap);
	public:		virtual bool getProperty(Engine& engine, const String* name, Value& v);
	public:		virtual bool setProperty(Engine& engine, const String* name, const Value& v);
	public:		virtual bool deleteProperty(Engine& engine, const String* name);
	protected:	Table properties;
};

class Engine {
	friend class String;
	public:		Engine(GCHeap& heap) : gcHeap(heap) { gcHeap.protect(&stringPrototype); };
	public:		GCHeap& heap() { return gcHeap; }
	protected:	GCHeap& gcHeap;
	protected:	UserObject stringPrototype;
};

class Processor;
class Function : public UserObject {
	public:		typedef UserObject super;
	public:		virtual Function* asFunction();
	public:		virtual void call(GCHeap& heap, Processor* processor, Int32 argc, const Value* argv, const Value& thisValue) = 0;
	public:		virtual void construct(GCHeap& heap, Processor* processor, Int32 argc, const Value* argv);
};

class NativeFunction : public Function {
	public:		typedef Function super;
};

// This is actually a closure, but ECMAScript calls them Function objects.
class ScriptFunction : public Function {
	public:		typedef Function super;
	public:		ScriptFunction(ScriptFunctionDef* definition, Object* environment);
	public:		virtual void gcMark(GCHeap& heap);
	public:		virtual void call(GCHeap& heap, Processor* processor, Int32 argc, const Value* argv, const Value& thisValue);
	public:		ScriptFunctionDef* getDefinition() const;
	public:		Object* getEnvironment() const;
	protected:	ScriptFunctionDef* const definition;
	protected:	Object* const environment; // Usually the global object or the frame of the context that created this function.
};

#if 0
class ArgumentsObject : public UserObject {
	public:		typedef UserObject super;
	public:		ArgumentsObject(FunctionFrame* frame);
	public:		virtual void gcMark(GCHeap& heap);
	public:		virtual bool getProperty(Engine& engine, String* name, Value& v);
	protected:	FunctionFrame* frame;
};
#endif

class Frame : public Object {
	public:		typedef Object super;
	public:		virtual const ScriptFunctionDef* getFunctionDef() const = 0;
	public:		virtual Value* getLocalsPointer() const = 0;
};

class FunctionFrame : public Frame {
	public:		typedef Frame super;
	public:		FunctionFrame(const ScriptFunction* function);
	public:		virtual void gcMark(GCHeap& heap);
	public:		virtual bool getProperty(Engine& engine, const String* name, Value& v);
	public:		virtual bool setProperty(Engine& engine, const String* name, const Value& v);
	public:		virtual bool deleteProperty(Engine& engine, const String* name);
	public:		virtual const ScriptFunctionDef* getFunctionDef() const { return functionDef; } // FIX : not inline
	public:		virtual Value* getLocalsPointer() const { return locals.begin(); }
	protected:	const ScriptFunctionDef* const functionDef;
	protected:	Object* const environment;
	protected:	QuickArray<Value, BUILT_IN_LOCALS_COUNT> locals;
};

class CatchFrame : public Frame {
	public:		typedef Frame super;
	public:		CatchFrame(Frame* parentFrame, const String* exceptionName, const Value& exceptionValue);
	public:		virtual void gcMark(GCHeap& heap);
	public:		virtual bool getProperty(Engine& engine, const String* name, Value& v);
	public:		virtual bool setProperty(Engine& engine, const String* name, const Value& v);
	public:		virtual bool deleteProperty(Engine& engine, const String* name);
	public:		virtual const ScriptFunctionDef* getFunctionDef() const { return parentFrame->getFunctionDef(); }
	public:		virtual Value* getLocalsPointer() const { return parentFrame->getLocalsPointer(); }
	protected:	Frame* parentFrame;
	protected:	const String* exceptionName;
	protected:	Value exceptionValue;
};

struct Context { // FIX : make Processor local
	const Byte* instructionPointer;
Value* vstackPointer;  // doesn't change during function execution, i.e. entry value stack pointer, FIX : keep, only in debug?
	Frame* frame; // doesn't change during function execution
	Value thisValue; // FIX : pointer instead? // doesn't change during function execution
};

class Processor : public GCItem {
	public:		typedef GCItem super;
	public:		enum Opcode {	// FIX : sort
					INVALID_OP
					, POP_OP										// stack: value ->
					, REPUSH_OP										// stack: value -> value, value
					, CONST_OP										// operand: const index, stack: -> constant
					, READ_INDEXED_OP								// operand: local index, stack: -> value
					, WRITE_INDEXED_OP								// operand: local index, stack: value -> value
					, READ_NAMED_OP									// operand: const index (name), stack: -> value
					, WRITE_NAMED_OP								// operand: const index (name), stack: value -> value
					, GET_PROPERTY_OP								// stack: name, object -> value
					, SET_PROPERTY_OP								// stack: name, object -> value
					, ADD_OP, SUB_OP, MUL_OP, DIV_OP, MOD_OP		// stack: left value, right value -> result value
					, OR_OP, XOR_OP, AND_OP							// stack: left value, right value -> result value
					, SHL_OP, SHR_OP, UNSIGNED_SHR_OP				// stack: left value, right value -> result value
					, PLUS_OP, MINUS_OP, BIT_NOT_OP, NOT_OP			// stack: value -> value
					, STRICT_EQ_OP, STRICT_NEQ_OP, EQ_OP, NEQ_OP	// stack: value -> boolean
					, LT_OP, LEQ_OP, GT_OP, GEQ_OP					// stack: value -> boolean
					, JMP_OP, JSR_OP								// operand: offset
					, JT_OP, JF_OP									// operand: offset, stack: boolean ->
					, CALL_GLOBAL_OP								// operand: n, stack: function, n * args -> return value
					, CALL_METHOD_OP								// operand: n, stack: object, name, n * args,  -> return value
					, NEW_OP										// operand: n, stack: constructor object, n * args -> return value, new object
					, NEW_RESULT_OP									// stack: new object, return value -> new this value
					, RETURN_OP
					, THIS_OP, VOID_OP, FRAME_OP					// stack: -> value
					, DELETE_OP										// stack: object, name -> boolean
					, GEN_FUNC_OP									// operand: const index, stack: -> function
					, THROW_OP										// stack: value ->
					, TRY_OP										// operand: offset for catch
					, TRIED_OP
					, CATCH_FRAME_OP								// operand: const index for exception variable name
					, POP_FRAME_OP
					, IN_OP											// stack: string, object -> boolean
				};
	public:		Processor(Engine& engine, Context* cstackStart, Context* cstackEnd, Value* vstackStart, Value* vstackEnd);
	public:		virtual void gcMark(GCHeap& heap);
	public:		void enterCall(ScriptFunction* func, Int32 argc, const Value* argv, const Value& thisValue = Value(Value::UNDEFINED_TYPE));
	public:		void push(const Value& v);
	public:		bool run(Int32 count);
	protected:	void push(Value*& vsp, const Value& v);
	protected:	void pop(Value*& vsp, int count);
	protected:	void poppush(Value*& vsp, const Value& v);
	protected:	Int32 readInt32(const Byte* ip);
	protected:	Int32 nextInt32(const Byte*& ip);
	protected:	void error();
	protected:	Engine& engine;
	protected:	Context* const cstackStart;
	protected:	Context* const cstackEnd;
	protected:	Value* const vstackStart;
	protected:	Value* const vstackEnd;
	protected:	Context* cstackPointer;
public:	Value* vstackPointer;
	protected:	struct Catcher {
					Catcher(Context* csp, Value* vsp, const Byte* ip) : cstackPointer(csp), vstackPointer(vsp), instructionPointer(ip) { }
					Context* cstackPointer;
					Value* vstackPointer;
					const Byte* instructionPointer;
				};
	protected:	std::vector<Catcher> catchers; // FIX : std::vector slow? linked list?
};

enum Precedence {
	GROUP_PREC, PROPERTY_PREC, NEW_PREC, FUNCTION_CALL_PREC, POST_INC_DEC, PREFIX_PREC, MUL_DIV_PREC, ADD_SUB_PREC, SHIFT_PREC
	, RELATIONAL_PREC, EQUALITY_PREC, BIT_AND_PREC, BIT_XOR_PREC, BIT_OR_PREC, LOGICAL_AND_PREC, LOGICAL_OR_PREC
	, CONDITIONAL_PREC, ASSIGN_PREC, COMMA_PREC, LOWEST_PREC
};

struct OperatorInfo;
class Compiler : public GCItem {
	public:		typedef GCItem super;
	public:		struct XValue; // FIX : rename to ExpressionResult
	public:		Compiler(Engine& engine, ScriptFunctionDef* function);
	public:		virtual void gcMark(GCHeap& heap);
	public:		const Char* compile(const Char* b, const Char* e);
	public:		void compile(const String& source);
	public:		void finish();
	protected:	struct Scope { // FIX : .cpp local
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
					std::vector<Int32> breaks; // source points for break jmp's
					std::vector<Int32> continues; // source points for continue jmp's
					std::vector<Int32> finallys; // source points for finally jsr's
				};
	protected:	void eatComment();
	protected:	void eatWhite();
	protected:	void emit(Processor::Opcode op);
	protected:	void emitWithInt32(Processor::Opcode op, Int32 i);
	protected:	void emitWithConstant(Processor::Opcode op, const Value& constant);
	protected:	Int32 emitForwardBranch(Processor::Opcode op);
	protected:	void completeForwardBranch(Int32 point);
	protected:	Int32 markBackwardBranch();
	protected:	void emitBackwardBranch(Processor::Opcode op, Int32 point);
	protected:	void completeContinues(const Scope* fromScope, const Scope* untilScope);
	protected:	void completeBreaks(const Scope* scope);
	protected:	XValue parseOperand(const OperatorInfo& op);
	protected:	void parseRValueOperand(const OperatorInfo& op);
	protected:	void parseFunctionCall(XValue& v, Processor::Opcode op);
	protected:	bool parseOperator(XValue& v, Precedence precedence, int opCount, const OperatorInfo* opList);
	protected:	void parseExpression(XValue& v, Precedence precedence);
	protected:	bool parseOptionalExpression(XValue& v, Precedence precedence);
	protected:	void parseAndPopOptionalExpression();
	protected:	void parseVarDeclarations();
	protected:	void parseStatement(Scope* firstScope, Scope* scopeLabelsEnd); // FIX : return false if b == p?
	protected:	void parseStatementList(Scope* firstScope);
	protected:	bool eatToken(const char* t, bool eatLeadingWhite); // FIX : not all of these, and name!
	protected:	void expectSemicolon();
	protected:	void expectToken(const char* t, bool eatLeadingWhite);
	protected:	void eatToken();
	protected:	void safeKeep(XValue& xv); // Used to store result into a new temporary local (used by return statement if there are finally blocks to call first)
	protected:	void makeRValue(XValue& xv); // Creates an r-value (i.e. pushed on value stack) out of a result.
	protected:	void makeAssignment(XValue& xv); // Creates an assignment out of an l-value (throws if xv is not a valid l-value)
	protected:	void discard(XValue& xv); // Discards result (e.g. popping it if it is on stack)
	protected:	bool isLeadingIdentifierChar(Char c);
	protected:	void parseIdentifier(bool acceptReservedKeywords); // FIX : return begin pointer as this is what we always do
	protected:	Engine& engine; // FIX : necessary?
	protected:	ScriptFunctionDef* const function;
	protected:	std::map<Value, Int32> definedConstants;
	protected:	const Char* p;
	protected:	const Char* e;
	protected:	CodeVector* outputCode;
};

#endif
