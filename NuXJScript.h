// --- TODO ---
//
//	Run-time
//
//		eval support
//		count (and run-time check) necessary value stack depth for functions
//		native calls (and returns)
//			need easier API, shouldn't crash if missing push etc
//			no synchronized return to native code right now
//			arguments are reversed on native calls, and will be gc'd as stack has already popped 
//				arguments no longer reversed since we push forwards now, but will still be gc'd
//		prototype support
//		array object (length property support, optimized for numerical indexes?)
//			take value as key for getproperty etc and we could optimize array and string accesses by not having to go via strings
//			only downside to that would be read and write named, which assumes the constant is a string pointer
//			also, how to efficiently convert between continuous array and sparse array / associative array... have QuickArray *and* Table in the object?
//		basic library (math, string, array ops)
//		assignable globals this in processor
//
//	Compiler
//
//		don't accept lf's after ++ etc (automatic ; insertion logic)
//		global environment compiler (don't accept return etc)
//		function name() { } syntax
//		literal function names, e.g. myGlobalName = function myLocalName() { ... }
//		exceptions (and catching both virtual and native)
//			only thing left is integration with C++ exceptions
//		instanceof operator
//		new operator
//			done, but need to implement prototype
//		array literals
//		object literals
//
//	GC
//
// --- LOW PRIO ---
//
// Run-time
//
//		divide by 0 checks? NaN checks?
//			is currently not compatible with -ffast_math, which is bad I guess
//			also isnan is not standard C++ :(, on Windows it is called _isnan
//			do a special Value variation that tracks NAN and Infinities?
//		(1.0E-3 == 0.001) is not true (due to accuracy problems I guess), I think it should be
//			same problem with (0.003e3 == 3.0)
//
// Compiler
//
//		better errors with line numbers and stuff (should we go back to the global "pickup" idea i had in old NuXScript?)
//		safe-kept variables aren't resetted on early exits, e.g.
//			b: switch (x) { case 1: switch(x) { case 2: break b } }
//			won't reset the safe kept x's, so there is a risk this kind of code holds on to more garbage than necessary (but freed at function exit of course!)
//
// GC
//
// --- OPTIMIZE ---
//
//	Run-time
//
//		special write with pop instructions (very common)?
//		change byte code to int code and check performance, or maybe the opposite, add byte, short etc immediates?
//			not too fond of the int cast in the byte array
//		only initialize Value to some BAD_TYPE in debug target, don't touch anything in release
//		quicker calls by using a frame stack for functions that do not create closures
//			problem is when we create closures with eval though, e.g. return(eval("(function(a,b) { return a+b+c })"));
//			two options: 1) copy frame (byte by byte) to heap when necessary, 2) make super-quick allocation possible and some flag to indicate direct destruction on exit (poor-man generational gc)
//		could we gain something on representing bools as doubles (0 and 1) in the union? e.g. converting to/from double would be quicker
//		if we can guarantee that vsp is always at function entry level for each catch scope we should try a table-based exception handler, e.g.
//			struct TryRange { Int32 offset; Int32 length; Int32 catchIndex; String* catchName; } // catchName != 0 then create temporary "catch frame"
//			TRY_OP, TRIED_OP and CATCH_FRAME_OP would go away then...
//		if we supplied a local variable index to CatchFrame (for the exception variable) we could access it by index instead of by name in the catch scope (and in the CatchFrame property getters/setters), but I am not convinced it would make an enormous performance improvement.
//		ERASE_INDEXED_OP for quickly resetting safe kept variables, worth it? (three instructions now, void, write, pop)
//		NEXT_PROPERTY_OP is always preceeded by READ_INDEXED (since it is always a non-constant safe-kept variable), combine into one instruction (with two operands)
//			if we don't do the REPUSH idea instead of safe-keeping so much
//
//	Compiler
//
//		jump optimizations
//		pop void optimization (almost always occurs with a return;)?
//		eliminate re-thrower in try / catch / finally if no finally block (by replacing the first try in catch with jmp +0)
//		share constant pointers (strings etc) between each compiled unit, e.g. function in function will not share the same string pointers
//		try to move POP_FRAME to first in FINALLY block, need different JSR and a new RSR / JMP_INDIRECT then that pushes and pops instruction offset on value stack instead (ought to work)
//			we could go back to switches for these when we don't need to POP_FRAME_OP on CATCH_TYPE:
//			if (s->type == Scope::CATCH_TYPE || s->type == Scope::FINALLY_TYPE) emit(Processor::POP_FRAME_OP);
//		don't like the outputCode solution, setting / resetting stuff manually isn't good programming style
//		if it wasn't for the problem with the value stack order in the return statement (i.e. the return expression has to be emitted before any "early exits pop's" etc are emitted) we wouldn't have to use safeKeep so much, but could just keep stuff on the stack and repush (e.g. the enumerator object for "for in")
//			actually, this isn't the case, case return expressions "safeKeep" before finally calls, not sure what the problem is to be honest, maybe we can keep stuff on the stack, could turn into many pops on early exits though
//		switch statements could benefit from a JUMP_AND_POP_IF_TRUE so that we could REPUSH the control value instead of safekeeping it
//		fast lookup of operators (some clever quick hashing)
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
//		switch
//		in operator
//		typeof operator
//		we could reuse free variable indexes in safeKeep (by having a way to "return" them and a queue with free ones), but it kind of feels like sub-optimization
//		for (in)
//		arguments container
//			we need to decide what should happen when deleting members still...

#if !defined(NuXJScript_h)
#define NuXJScript_h 1

#include "assert.h"
#include <string>

typedef unsigned short Char;
typedef int Int32;
typedef unsigned int UInt32;
typedef unsigned char Byte;

const UInt32 TABLE_BUILT_IN_N = 3; // 1 << 3 == 8
const UInt32 FRAME_BUILT_IN_LOCALS_COUNT = 8;
const size_t STRING_BUILT_IN_LENGTH = 16;
const double EPSILON = 1.0e-300;

// FIX : name
template<typename T, size_t BUILT_IN_COUNT> class QuickArray {
	public:		explicit QuickArray(size_t n = BUILT_IN_COUNT) : n(n), p(n <= BUILT_IN_COUNT ? a : new T[n]) { }
	public:		explicit QuickArray(const QuickArray& copy) : n(0), p(a) { (*this) = copy; }

	public:		void swap(QuickArray& other) {
					if (this != &other) {
						std::swap_ranges(a + 0, a + BUILT_IN_COUNT, other.a); // Note: if this one throws, we have messed up this object, but it is not completely broken (i.e. shouldn't crash destruction).
						std::swap(n, other.n);
						T* x = p;
						p = ((other.p == other.a) ? a : other.p);
						other.p = ((x == a) ? other.a : x);
					}
				}
				
	public:		QuickArray& operator=(const QuickArray& copy) {
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
	public:		explicit Value(const String* s);
	public:		explicit Value(Object* o);
	
	public:		bool exists() const;
	public:		bool isObject() const;
	public:		const String* getAsString() const;
	public:		Object* getAsObject() const;
	public:		const String* getTypeString() const;
	public:		bool toBool() const;
	public:		Int32 toInt() const;
	public:		double toDouble() const;
	public:		const String* toString(GCHeap& heap) const;
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
					const String* s;
					Object* o;
				};
};

class Enumerator;
class Object : public GCItem {
	public:		typedef GCItem super;
	public:		virtual Function* asFunction();
	public:		virtual Enumerator* asEnumerator();
	public:		virtual const String* getTypeString() const;
	public:		virtual bool hasProperty(Engine& engine, const String* name);
	public:		virtual bool getProperty(Engine& engine, const String* name, Value& v);
	public:		virtual bool setProperty(Engine& engine, const String* name, const Value& v);
	public:		virtual bool deleteProperty(Engine& engine, const String* name);
	public:		virtual Enumerator* getEnumerator(Engine& engine);
};

class Enumerator : public Object {
	public:		typedef Object super;
	public:		virtual Enumerator* asEnumerator() { return this; }
	public:		virtual const String* nextPropertyName() = 0;
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
	public:		UInt32 getLoadCount() const;
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
	public:		std::wstring toStdString() const;
	public:		virtual bool getProperty(Engine& engine, const String* name, Value& v);
	public:		virtual Enumerator* getEnumerator(Engine& engine);
	protected:	mutable size_t hash;
	protected:	class CharEnumerator;
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
	public:		Int32 varsCount; // not including arguments
	public:		LocalNameMap localNames;
};

class StringListEnumerator : public Enumerator {
	public:		typedef Enumerator super;
	public:		StringListEnumerator(UInt32 maxCount) : list(maxCount), actualCount(0), nextIndex(0) { }
	public:		void add(const String* s) { assert(actualCount < list.size()); list[actualCount++] = s; }
	public:		void sort() { std::sort(list.begin(), list.begin() + actualCount, StringObjectCompare()); }
	public:		virtual void gcMark(GCHeap& heap) { for (const String** p = list.begin(); p != list.end(); ++p) heap.mark(*p); super::gcMark(heap); }
	public:		virtual const String* nextPropertyName() { return (nextIndex < actualCount ? list[nextIndex++] : 0); }
	protected:	QuickArray<const String*, 8> list;
	protected:	UInt32 actualCount;
	protected:	UInt32 nextIndex;
};

class UserObject : public Object {
	public:		typedef Object super;
	public:		UserObject(Object* prototype = 0);
	public:		virtual void gcMark(GCHeap& heap);
	public:		virtual bool getProperty(Engine& engine, const String* name, Value& v);
	public:		virtual bool setProperty(Engine& engine, const String* name, const Value& v);
	public:		virtual bool deleteProperty(Engine& engine, const String* name);
	public:		virtual Enumerator* getEnumerator(Engine& engine);
	protected:	Table properties;
	protected:	Object* prototype;
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
	public:		virtual const String* getTypeString() const;
	public:		virtual void call(Engine& engine, Processor* processor, Int32 argc, const Value* argv, const Value& thisValue) = 0;
	public:		virtual void construct(Engine& engine, Processor* processor, Int32 argc, const Value* argv);
};

class NativeFunction : public Function {
	public:		typedef Function super;
};

// This is actually a closure, but ECMAScript calls them Function objects.
class ScriptFunction : public Function {
	public:		typedef Function super;
	public:		ScriptFunction(ScriptFunctionDef* definition, Object* environment);
	public:		virtual void gcMark(GCHeap& heap);
	public:		virtual void call(Engine& engine, Processor* processor, Int32 argc, const Value* argv, const Value& thisValue);
	public:		ScriptFunctionDef* getDefinition() const;
	public:		Object* getEnvironment() const;
	protected:	ScriptFunctionDef* const definition;
	protected:	Object* const environment; // Usually the global object or the frame of the context that created this function.
};

class Frame : public Object {
	public:		typedef Object super;
	public:		virtual const ScriptFunctionDef* getFunctionDef() const = 0;
	public:		Value* getLocalsPointer() const { return localsPointer; }
	protected:	Value* localsPointer;
};

class ArgumentsObject;
class FunctionFrame : public Frame {
	public:		typedef Frame super;
	public:		FunctionFrame(const ScriptFunction* function, Int32 argc, const Value* argv);
	public:		virtual void gcMark(GCHeap& heap);
	public:		virtual bool getProperty(Engine& engine, const String* name, Value& v);
	public:		virtual bool setProperty(Engine& engine, const String* name, const Value& v);
	public:		virtual bool deleteProperty(Engine& engine, const String* name);
	public:		virtual const ScriptFunctionDef* getFunctionDef() const { return functionDef; } // FIX : not inline
	protected:	const ScriptFunctionDef* const functionDef;
	protected:	Object* const environment;
	protected:	const Int32 passedArgumentsCount;
	protected:	QuickArray<Value, FRAME_BUILT_IN_LOCALS_COUNT> locals;
	protected:	Value argumentsValue;
};

class CatchFrame : public Frame {
	public:		typedef Frame super;
	public:		CatchFrame(Frame* parentFrame, const String* exceptionName, const Value& exceptionValue);
	public:		virtual void gcMark(GCHeap& heap);
	public:		virtual bool getProperty(Engine& engine, const String* name, Value& v);
	public:		virtual bool setProperty(Engine& engine, const String* name, const Value& v);
	public:		virtual bool deleteProperty(Engine& engine, const String* name);
	public:		virtual const ScriptFunctionDef* getFunctionDef() const { return parentFrame->getFunctionDef(); }
	protected:	Frame* parentFrame;
	protected:	const String* exceptionName;
	protected:	Value exceptionValue;
};

class ArgumentsObject : public UserObject {
	public:		typedef UserObject super;
	public:		ArgumentsObject(FunctionFrame* frame, Int32 argumentsCount);
	public:		virtual void gcMark(GCHeap& heap);
	public:		virtual bool getProperty(Engine& engine, const String* name, Value& v);
	public:		virtual bool setProperty(Engine& engine, const String* name, const Value& v);
	public:		virtual bool deleteProperty(Engine& engine, const String* name);
	protected:	Value* findProperty(const String* name);
	protected:	FunctionFrame* frame;
	protected:	Int32 argumentsCount;
	protected:	QuickArray<Byte, 32> deletedArguments;
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
					, GET_PROPERTY_OP								// stack: object, name -> value
					, SET_PROPERTY_OP								// stack: object, name, value -> value
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
					, TYPEOF_OP										// stack: value -> string
					, TYPEOF_NAMED_OP								// operand: const index (name), stack: -> string
					, GET_ENUMERATOR_OP								// stack: object -> enumerator
					, NEXT_PROPERTY_OP								// immediate: exit loop offset, stack: enumerator -> string
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
	GROUP_PREC, PROPERTY_PREC, NEW_PREC, FUNCTION_CALL_PREC, POST_INC_DEC_PREC, PREFIX_PREC, MUL_DIV_PREC, ADD_SUB_PREC, SHIFT_PREC
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
	protected:	XValue parseOperand(const OperatorInfo& op); // FIX : drop parse from all these
	protected:	void parseRValueOperand(const OperatorInfo& op);
	protected:	void parseFunctionCall(XValue& v, Processor::Opcode op);
	protected:	bool parseOperator(XValue& v, Precedence precedence, int opCount, const OperatorInfo* opList);
	protected:	void parseExpression(XValue& v, Precedence precedence);
	protected:	bool parseOptionalExpression(XValue& v, Precedence precedence);
	protected:	void parseAndPopOptionalExpression();
	protected:	Compiler::XValue parseVarDeclaration();
	protected:	void parseVarDeclarations();
	protected:	void parseForInStatement(Scope* emptyLabelScope, Scope* scopeLabelsEnd, const CodeVector& iterCode, XValue& iterXValue);
	protected:	void parseStatement(Scope* firstScope, Scope* scopeLabelsEnd); // FIX : return false if b == p?
	protected:	void parseStatementList(Scope* firstScope);
	protected:	bool eatToken(const char* t, bool eatLeadingWhite); // FIX : not all of these, and name!
	protected:	void expectSemicolon();
	protected:	void expectToken(const char* t, bool eatLeadingWhite);
	protected:	void eatToken();
	protected:	void safeKeep(XValue& xv); // Used to store result into a new temporary local (used by return statement if there are finally blocks to call first)
	protected:	void returnSafeKept(XValue& xv, bool doReset, bool makeRValue); // if doReset == true then reset the local variable used for safe-keeping (to UNDEFINED), if makeRValue == true, place the safe-kept value on stack
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
	protected:	std::vector<Int32> freeSafeKeepingIndexes;
	protected:	Int32 safeKeptIndexCount; // Just used for assert. FIX : remove or what?
	protected:	bool acceptInOperator;
};

#endif
