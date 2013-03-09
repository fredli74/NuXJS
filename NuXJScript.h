// --- TODO ---
//
//	Run-time
//
//		eval support
//		count (and run-time check) necessary value stack depth for functions
//		array object (length property support, optimized for numerical indexes?)
//			take value as key for getproperty etc and we could optimize array and string accesses by not having to go via strings
//			only downside to that would be read and write named, which assumes the constant is a string pointer
//			also, how to efficiently convert between continuous array and sparse array / associative array... have QuickArray *and* Table in the object?
//		basic library (math, string, array ops)
//		assignable globals this in processor
//		true primitive <-> object conversions
//		true toString, toNumber, which is problematic because we might have to execute JS-code then and I want to avoid synchronous callbacks to JS-code (=blocking!)
//		string -> number doesn't conform to the http://www.ecma-international.org/ecma-262/5.1/#sec-9.3.1 , e.g. empty string should be 0 , spaces should be accepted etc...
//		for in should also include properties in prototypes, I believe
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
//		I don't know but the "emulation" in arguments object feels a bit over the top (registering deleted items etc)
//		Why don't we make Engine a GCItem too and gcMark whatever it has instead of "rooting" it?
//
// Compiler
//
//		better errors with line numbers and stuff (should we go back to the global "pickup" idea i had in old NuXScript?)
//		safe-kept variables aren't resetted on early exits, e.g.
//			b: switch (x) { case 1: switch(x) { case 2: break b } }
//			won't reset the safe kept x's, so there is a risk this kind of code holds on to more garbage than necessary (but freed at function exit of course!)
//
// Other
//
//		included in tests should be to config the gc to sweep after every instruction
//
// --- OPTIMIZE ---
//
//	Run-time
//
//		special write with pop instructions (very common)?
//		change byte code to int code and check performance, or maybe the opposite, add byte, short etc immediates?
//			not too fond of the int cast in the byte array
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
//		Make writes terminal and REPUSH in a = b = c type of expression...
//		Don't lookup names in frame when not necessary. (And use hash table!)
//
//	Compiler
//
//		jump optimizations
//			another way to deal with it would be to think in the lines of the break scopes... complete forward jumps first at definite end-point, I think this was the way I solved it in ACL
//			we should at least have two alternative JF and JT, JF_NO_POP, JT_NO_POP that doesn't pop on jump
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
//		prototype support
//		new operator
//			done, but need to implement prototype
//		only initialize Value to some BAD_TYPE in debug target, don't touch anything in release
//		native calls (and returns)
//			no synchronized return to native code right now
//			need easier API, shouldn't crash if missing push etc
//			arguments are reversed on native calls, and will be gc'd as stack has already popped 
//				arguments no longer reversed since we push forwards now, but will still be gc'd

#if !defined(NuXJScript_h)
#define NuXJScript_h 1

#include "assert.h"
#include <string>

typedef unsigned short Char;
typedef int Int32;
typedef unsigned int UInt32;
typedef unsigned char Byte;
typedef Int32 CodeWord;
const int OPERAND_SIZE = (sizeof (Int32) / sizeof (CodeWord));

const UInt32 TABLE_BUILT_IN_N = 3; // 1 << 3 == 8
const UInt32 FRAME_BUILT_IN_LOCALS_COUNT = 8;
const size_t STRING_BUILT_IN_LENGTH = 16;
const double EPSILON = 1.0e-300;

// FIX : name
template<typename T, size_t BUILT_IN_COUNT> class QuickArray {
	public:		explicit QuickArray(size_t n = BUILT_IN_COUNT) : n(n), p(n <= BUILT_IN_COUNT ? a : new T[n]) { }
				explicit QuickArray(const QuickArray& copy) : n(0), p(a) { (*this) = copy; }

				void swap(QuickArray& other) {
					if (this != &other) {
						std::swap_ranges(a + 0, a + BUILT_IN_COUNT, other.a); // Note: if this one throws, we have messed up this object, but it is not completely broken (i.e. shouldn't crash destruction).
						std::swap(n, other.n);
						T* x = p;
						p = ((other.p == other.a) ? a : other.p);
						other.p = ((x == a) ? other.a : x);
					}
				}
				
				QuickArray& operator=(const QuickArray& copy) {
					if (this != &copy) {
						QuickArray<T, BUILT_IN_COUNT> na(copy.n);
						std::copy(copy.p, copy.p + copy.n, na.p);
						swap(na);
					}
					return *this;
				}
				
				bool empty() const { return n == 0; }
				size_t size() const { return n; }
				operator T*() const { return p; }
				T* begin() const { return p; }
				T* end() const { return p + n; }
				~QuickArray() { if (p != a) delete [] p; }
				
	protected:	size_t n;
				T* p;
				T a[BUILT_IN_COUNT];
};

class GCList;
class GCHeap;
class GCItem {
	friend class GCList;
	friend class GCHeap; // FIX : move stuff to GCList so GCHeap doesn't have to be friend

	public:		GCItem();
				GCItem(const GCItem& copy);
				GCItem& operator=(const GCItem& copy);
				virtual void gcMark(GCHeap& heap);
				virtual ~GCItem();

	protected:	GCList* _gcList;
				GCItem* _gcPrev;
				GCItem* _gcNext;
};

class GCList : public GCItem {
	public:		GCList();
				size_t size() const;
				void claim(GCItem* item);
				void relinquish(GCItem* item);

	protected:	size_t count;
};

class GCHeap {
	public:		GCHeap();
				template<class T> T* manage(T* item) { currentList->claim(item); return item; }
				template<class T> T* protect(T* item) { rootList.claim(item); return item; } // FIX : don't like this name, it's really also "manage", but don't auto-free
				size_t size() const;
				size_t gc();
				void mark(const GCItem* item);

	protected:	GCList* currentList;
				GCList* newList;
				GCList managedLists[2];
				GCList rootList;

	private:	GCHeap(const GCHeap& other); // N/A
				GCHeap& operator=(const GCHeap& other); // N/A
};

class String;
class Object;
class Function;
class Value {
	public:		enum Type {
					NONEXISTENT_TYPE = 0		// Used in tables for deleted entries. Never seen by script-code.
					, UNDEFINED_TYPE
					, NULL_TYPE
					, BOOLEAN_TYPE
					, NUMBER_TYPE
					, STRING_TYPE
					, OBJECT_TYPE
					, BAD_TYPE = 0xBAADBAAD
				};
			
			#if !defined(NDEBUG)
				Value()	: type(BAD_TYPE) { }
			#else
				Value() { }
			#endif
				explicit Value(Type type) : type(type) { }
				explicit Value(bool b) : type(BOOLEAN_TYPE), b(b) { }
				explicit Value(Int32 i) : type(NUMBER_TYPE), d(i) { }
				explicit Value(UInt32 u) : type(NUMBER_TYPE), d(u) { }
				explicit Value(double d) : type(NUMBER_TYPE), d(d) { }
				explicit Value(const String* s) : type(STRING_TYPE), s(s) { }
				explicit Value(Object* o) : type(OBJECT_TYPE), o(o) { }
	
				bool exists() const { return type != NONEXISTENT_TYPE; }
				Object* asObject() const { return (type == OBJECT_TYPE ? o : 0); }
				Function* getFunction() const; // returns a stub function that throws if value is not Function object
				const String* getString() const { assert(type == STRING_TYPE); return s; }
				const String* typeString() const;
				bool toBool() const;
				Int32 toInt() const; // FIX : ?
				double toDouble() const;
				const String* toString(GCHeap& heap) const;
				Object* toObject(GCHeap& heap) const;
				bool operator==(const Value& y) const;
				bool operator<(const Value& y) const;
				bool isStrictlyEqualTo(const Value& y) const;
				bool isEqualTo(const Value& y) const;
				bool isLessThan(const Value& y) const;
				bool isLessThanOrEqualTo(const Value& y) const;
				Value add(GCHeap& heap, const Value& y) const;
				void markIn(GCHeap& heap) const;

	protected:	bool compareStrictly(const Value& y) const;
				Type type;
				union {
					bool b;
					double d;
					const String* s;
					Object* o;
				};

	private:	explicit Value(void* v); // N/A. Just to avoid unwanted casting of a pointer to bool.
};

class Engine;
class Enumerator;
class Function;
class Processor;
class Object : public GCItem {
	public:		typedef GCItem super;
				
				virtual Function* asFunction(); // FIX : needed?
				virtual Function* getFunction(); // returns a stub function that throws if Object is not function
				virtual const String* typeString() const;
				virtual const String* className() const;
				virtual bool hasProperty(Engine& engine, const String* name);
				virtual bool getProperty(Engine& engine, const String* name, Value* v); // don't touch v if you return false
				virtual bool setProperty(Engine& engine, const String* name, const Value& v); // setProperty can store a reference to the name pointer, therefore the name string must live as long as this object lives, naturally, the string can be managed by the gc-heap
				virtual bool deleteProperty(Engine& engine, const String* name);
				virtual Enumerator* getEnumerator(Engine& engine);

				void toPrimitiveStringOrThrow(Engine& engine, Processor& processor);
				void toPrimitiveNumber(Engine& engine, Processor& processor);
};

class Enumerator : public Object {
	public:		typedef Object super;
				
				virtual Enumerator* asEnumerator() { return this; }
				virtual const String* nextPropertyName() = 0;
};

class Table {
	public:		typedef UInt32 HashType;
				
				struct Bucket {
					Bucket();
					HashType hash;
					const String* key;
					Value value;
				};
				
				Table(UInt32 n = TABLE_BUILT_IN_N);
				UInt32 getLoadCount() const;
				const Bucket* getFirst() const;
				const Bucket* getNext(const Bucket* b) const;
				const Bucket* lookup(const String* key);
				void assign(const String* key, const Value& value);
				void erase(const String* key);
				void mark(GCHeap& heap);

	protected:	static UInt32 calcMaxLoad(UInt32 bucketCount);
				static UInt32 suggestN(UInt32 count); // FIX : keep?
				void update(const String* key, const Value& value);
				Bucket* find(const String* key, HashType hash) const;
				UInt32 rebuild(UInt32 newN);
				
				UInt32 n; ///< Size of member table is two to the power of n.
				QuickArray<Bucket, 1U << TABLE_BUILT_IN_N> buckets;
				UInt32 loadCount; ///< Count of buckets with defined keys.
};

class String : public Object, public QuickArray<Char, STRING_BUILT_IN_LENGTH> {
	public:		typedef Object super;
				typedef QuickArray<Char, STRING_BUILT_IN_LENGTH> QArray;

				static const String* newHashedString(GCHeap& heap, const Char* b, const Char* e);
				static const String* fromInt(GCHeap& heap, Int32 i);
				static const String* fromDouble(GCHeap& heap, double d);

				String();
				String(const String& o);
				String(const Char* b, const Char* e);
				String(const String& l, const String& r);
				explicit String(const std::wstring& s);
				explicit String(size_t length) : QArray(length), hash(0) { } // FIX : dangerous, right, make protected?
				template<size_t N> explicit String(const char (&s)[N]) : QArray(N - 1), hash(0) { std::copy(s, s + N - 1, begin()); calcHash(); }
				virtual const String* typeString() const;
				virtual const String* className() const;
				virtual bool getProperty(Engine& engine, const String* name, Value* v);
				virtual Enumerator* getEnumerator(Engine& engine);
				Table::HashType calcHash() const;
				bool isEqualTo(const String& o) const;
				bool isLessThan(const String& o) const;
				std::wstring toStdString() const;

	protected:	mutable size_t hash;
};

struct StringObjectCompare {
	bool operator() (const String* a, const String* b) const { return (a != b && a->isLessThan(*b)); }
};

typedef std::map<const String*, Int32, StringObjectCompare> LocalNameMap; // FIX : use table
typedef std::vector<CodeWord> CodeVector; // FIX : use QArray in all these places

// This is an Object so that we can store it as a constant
class ScriptFunctionDef : public Object {	
	public:		typedef Object super;
				
				ScriptFunctionDef();
				virtual void gcMark(GCHeap& heap);
				bool lookupLocalName(const String* name, Int32& index) const;
				const Value* getConstantsPointer() const;

// FIX : protectize
	public:		const String* source;
				CodeVector code;
				std::vector<Value> constants;
				Int32 argumentsCount;
				Int32 varsCount; // not including arguments
				LocalNameMap localNames;
};

class RangeEnumerator : public Enumerator {
	public:		typedef Enumerator super;
				RangeEnumerator(GCHeap& heap, Int32 from, Int32 count);
				virtual const String* nextPropertyName();
				
	protected:	GCHeap& heap;
				const Int32 to;
				Int32 index;
};

class StringListEnumerator : public Enumerator {
	public:		typedef Enumerator super;
				StringListEnumerator(UInt32 maxCount);
				void add(const String* s);
				void sort(); // FIX : bad idea?
				virtual void gcMark(GCHeap& heap);
				virtual const String* nextPropertyName();

	protected:	QuickArray<const String*, 8> list;
				UInt32 actualCount;
				UInt32 nextIndex;
};

class UserObject : public Object {
	public:		typedef Object super;
				
				UserObject(Object* prototype = 0);
				virtual void gcMark(GCHeap& heap);
				virtual bool getProperty(Engine& engine, const String* name, Value* v);
				virtual bool setProperty(Engine& engine, const String* name, const Value& v);
				virtual bool deleteProperty(Engine& engine, const String* name);
				virtual Enumerator* getEnumerator(Engine& engine);
				
	protected:	void populateStringListEnumerator(StringListEnumerator* list);
				
				Table properties;
				Object* prototype;
};

class Engine {
	friend class String;
	
	public:		Engine(GCHeap& heap, Object* globalThis);
				GCHeap& getHeap() const { return gcHeap; }
				Object* getGlobalThis() const { return globalThis; }

	protected:	GCHeap& gcHeap;
				UserObject stringPrototype;
				Object* globalThis;
};

class Function : public UserObject {
	public:		typedef UserObject super;

				virtual Function* asFunction();
				virtual Function* getFunction();
				virtual const String* typeString() const;
				virtual const String* className() const;
				virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) = 0;
				virtual Value construct(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue);
};

// This is actually a closure, but ECMAScript calls them Function objects.
class ScriptFunction : public Function {
	public:		typedef Function super;
				
				ScriptFunction(ScriptFunctionDef* definition, Object* environment);
				virtual void gcMark(GCHeap& heap);
				virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue);
				ScriptFunctionDef* getDefinition() const;
				Object* getEnvironment() const;

	protected:	ScriptFunctionDef* const definition;
				Object* const environment; // Usually the global object or the frame of the context that created this function.
};

class Frame : public Object {
	public:		typedef Object super;
				Frame(const ScriptFunctionDef* functionDef) : functionDef(functionDef) { }
				const ScriptFunctionDef* getFunctionDef() const { return functionDef; }
				Value* getLocalsPointer() const { return localsPointer; }
				
	protected:	const ScriptFunctionDef* const functionDef;
				Value* localsPointer;
};

class Arguments;
class FunctionFrame : public Frame {
	public:		typedef Frame super;

				FunctionFrame(const ScriptFunction* function, Int32 argc, const Value* argv);
				virtual void gcMark(GCHeap& heap);
				virtual bool getProperty(Engine& engine, const String* name, Value* v);
				virtual bool setProperty(Engine& engine, const String* name, const Value& v);
				virtual bool deleteProperty(Engine& engine, const String* name);

	protected:	Object* const environment;
				const Int32 passedArgumentsCount;
				QuickArray<Value, FRAME_BUILT_IN_LOCALS_COUNT> locals;
				Value argumentsValue;
};

class CatchFrame : public Frame {
	public:		typedef Frame super;

				CatchFrame(Frame* parentFrame, const String* exceptionName, const Value& exceptionValue);
				virtual void gcMark(GCHeap& heap);
				virtual bool getProperty(Engine& engine, const String* name, Value* v);
				virtual bool setProperty(Engine& engine, const String* name, const Value& v);
				virtual bool deleteProperty(Engine& engine, const String* name);

	protected:	Frame* parentFrame;
				const String* exceptionName;
				Value exceptionValue;
};

class Arguments : public UserObject {
	public:		typedef UserObject super;

				Arguments(FunctionFrame* frame, Int32 argumentsCount);
				virtual const String* className() const;
				virtual void gcMark(GCHeap& heap);
				virtual bool getProperty(Engine& engine, const String* name, Value* v);
				virtual bool setProperty(Engine& engine, const String* name, const Value& v);
				virtual bool deleteProperty(Engine& engine, const String* name);
				virtual Enumerator* getEnumerator(Engine& engine);

	protected:	Value* findProperty(const String* name);
				FunctionFrame* frame;
				Int32 argumentsCount;
				Value lengthProperty;
				QuickArray<Byte, 8> deletedArguments;
};

class Processor : public GCItem {
	public:		typedef GCItem super;

	public:		struct Continuation {
					public:		virtual bool execute(Engine& engine, Processor& processor, Object* thisValue) = 0;
				};

				struct Context { // FIX : make Processor local
					const CodeWord* ip;
					Frame* frame; // doesn't change during function execution
					Object* thisValue; // FIX : pointer instead? // doesn't change during function execution
				};

	protected:	struct Catcher {
					Catcher(const CodeWord* ip, Value* vsp, Context* csp) : ip(ip), csp(csp), vsp(vsp) { }
					const CodeWord* ip;
					Value* vsp;
					Context* csp;
				};
				
				std::vector<Catcher> catchers; // FIX : std::vector slow? linked list?
				std::vector<Continuation*> continuations; // FIX : std::vector slow? linked list? in callstack directly? I have a feeling there is a more elegant solution to all this

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
					, SHL_OP, SHR_OP, USHR_OP						// stack: left value, right value -> result value
					, PLUS_OP, MINUS_OP, INV_OP, NOT_OP				// stack: value -> value
					, X_EQ_OP, X_NEQ_OP, EQ_OP, NEQ_OP				// stack: value -> boolean
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
				virtual void gcMark(GCHeap& heap);
				void chain(Continuation* continuation, Object* thisValue);
				void chain(ScriptFunction* func, Int32 argc, const Value* argv, Object* thisValue);
	// FIX :			void call(ScriptFunction* func, Int32 argc, const Value* argv, Object* thisValue);
				void popCStack();
				void push(const Value& v); // FIX : wow, this feels low level, public really?
				Value& peek(Int32 offset);
				void pop(Int32 count = 1);
				void resetTimeOut(Int32 clockCycles);
				bool run();
void error(); // FIX
				
	protected:	void poppush(const Value& v);
				Int32 operand();
				void callStub(const CodeWord*& ip, const CodeWord* stub);
				bool toPrimitiveTrap(const CodeWord* rollbackIp);

				Engine& engine;
				Context* const cstackStart;
				Context* const cstackEnd;
				Value* const vstackStart;
				Value* const vstackEnd;

public: // FIX : not public, just for some asserts now I think
				Int32 clockCyclesLeft;
				const CodeWord* ip;
				Value* vsp; // FIX : not public, just for an assert now
				Context* csp;
				Frame* frame;
				const Value* constants;
				Value* locals;
};

enum Precedence {
	GROUP_PREC, PROPERTY_PREC, NEW_PREC, FUNCTION_CALL_PREC, POST_INC_DEC_PREC, PREFIX_PREC, MUL_DIV_PREC, ADD_SUB_PREC, SHIFT_PREC
	, RELATIONAL_PREC, EQUALITY_PREC, BIT_AND_PREC, BIT_XOR_PREC, BIT_OR_PREC, LOGICAL_AND_PREC, LOGICAL_OR_PREC
	, CONDITIONAL_PREC, ASSIGN_PREC, COMMA_PREC, LOWEST_PREC
};

struct OperatorInfo;
class Compiler : public GCItem {
	public:		typedef GCItem super;
				struct XValue; // FIX : rename to ExpressionResult
				
				Compiler(Engine& engine, ScriptFunctionDef* function);
				virtual void gcMark(GCHeap& heap);
				const Char* compile(const Char* b, const Char* e);
				void compile(const String& source);
				void finish();

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

				void emit(Processor::Opcode op);
				void emitWithInt32(Processor::Opcode op, Int32 i);
				void emitWithConstant(Processor::Opcode op, const Value& constant);
				Int32 emitForwardBranch(Processor::Opcode op);
				void completeForwardBranch(Int32 point);
				Int32 markBackwardBranch();
				void emitBackwardBranch(Processor::Opcode op, Int32 point);
				void completeContinues(const Scope* fromScope, const Scope* untilScope);
				void completeBreaks(const Scope* scope);
				XValue parseOperand(const OperatorInfo& op); // FIX : drop parse from all these
				void parseRValueOperand(const OperatorInfo& op);
				void parseFunctionCall(XValue& v, Processor::Opcode op);
				bool parseOperator(XValue& v, Precedence precedence, int opCount, const OperatorInfo* opList);
				void parseExpression(XValue& v, Precedence precedence);
				bool parseOptionalExpression(XValue& v, Precedence precedence);
				void parseAndPopOptionalExpression();
				Compiler::XValue parseVarDeclaration();
				void parseVarDeclarations();
				void parseForInStatement(Scope* emptyLabelScope, Scope* scopeLabelsEnd, const CodeVector& iterCode, XValue& iterXValue);
				void parseForStatement(Scope* emptyLabelScope, Scope* scopeLabelsEnd);
				void parseStatement(Scope* firstScope, Scope* scopeLabelsEnd); // FIX : return false if b == p?
				void parseStatementList(Scope* firstScope);
				bool eatToken(const char* t, bool eatLeadingWhite); // FIX : not all of these, and name!
				void expectSemicolon();
				void expectToken(const char* t, bool eatLeadingWhite);
				void eatWhite();
				void eatToken();
				void safeKeep(XValue& xv); // Used to store result into a new temporary local (used by return statement if there are finally blocks to call first)
				void returnSafeKept(XValue& xv, bool doReset, bool makeRValue); // if doReset == true then reset the local variable used for safe-keeping (to UNDEFINED), if makeRValue == true, place the safe-kept value on stack
				void makeRValue(XValue& xv); // Creates an r-value (i.e. pushed on value stack) out of a result.
				void makeAssignment(XValue& xv); // Creates an assignment out of an l-value (throws if xv is not a valid l-value)
				void discard(XValue& xv); // Discards result (e.g. popping it if it is on stack)
				bool isLeadingIdentifierChar(Char c);
				void parseIdentifier(bool acceptReservedKeywords); // FIX : return begin pointer as this is what we always do

				Engine& engine; // FIX : necessary?
				ScriptFunctionDef* const function;
				std::map<Value, Int32> definedConstants;
				const Char* p;
				const Char* e;
				CodeVector* outputCode;
				std::vector<Int32> freeSafeKeepingIndexes;
				Int32 safeKeptIndexCount; // Just used for assert. FIX : remove or what?
				bool acceptInOperator;
};

#endif
