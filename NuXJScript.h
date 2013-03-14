// --- TODO ---
//
//	Run-time
//
//		eval support
//			I think the if in the "if (compilingForEval) evalXR = makeRValue(evalXR);" lines are actually unnecessary? 
//			implement rest of eval r-value logic for do while, for, switch and try / catch (any more?)
//			global eval support (sets all var and function declarations in global this)
//		count (and run-time check) necessary value stack depth for functions
//		array object (length property support, optimized for numerical indexes?)
//			take value as key for getproperty etc and we could optimize array and string accesses by not having to go via strings
//			only downside to that would be read and write named, which assumes the constant is a string pointer
//			also, how to efficiently convert between continuous array and sparse array / associative array... have QuickArray *and* Table in the object?
//		basic library (math, string, array ops)
//		true primitive <-> object conversions
//		true toString, toNumber, which is problematic because we might have to execute JS-code then and I want to avoid synchronous callbacks to JS-code (=blocking!)
//		string -> number doesn't conform to the http://www.ecma-international.org/ecma-262/5.1/#sec-9.3.1 , e.g. empty string should be 0 , spaces should be accepted etc...
//		for in should also include properties in prototypes, I believe
//		it would be nice if it wasn't so easy to forget that Value() didn't create an undefined type, have a SafeValue()? That also maybe even protects from garbage-collect, that would be nice :)
//		lots of problems with gc safety and native calls / constructors: newed objects, methods, this values are not protected
//			push them temporarily last on stack
//			 or hold on, I think the entire gc approach needs rethinking, we should have something like gc(beyond) like in NuXScript...
//		would be cleaner if we could pass a retValue pointer instead of taking a return value (but not nicer syntax for native calls returning tho, hmm, need to think)
//			we want nice, quick, safe and clean syntax for:
//				* calling synchronously <-- not prio
//					script: running vm code until return with option to set max count before error)
//					native: call directly
//					syntax: either { ret = function->call } or { processor->call(function }
//				* calling asynchronously  <-- prio
//					script: enter call ("chain"), user runs { while (processor->run()) { } }, return value is either popped { processor->getReturnValue() } or stored into a predesignated pointer
//					native: chain a native call, syntax should be the same as for script
//				* calling script async or native sync
//					script: enter call ("chain"), user is notified and have the option to "while (run)" or alt. wait for returned value later (via pointer?? nah, probably not)
//					native: call directly, return value pushed?
//				* implementing native functions
//					prios: little overhead (quick), error safe (args and maybe even return values are automatically gc protected)
//						so maybe argc is a bad idea, maybe we should have minimum fixed (which is also .length), like native functions
//			alt 1: like today, native calls are direct and faster call sequence than script code (they access arguments directly on stack)
//					pros:	fast
//					cons:	wacky code to accomodate both synchronized native and asynchronized script function calls with little overhead
//							harder to make safe
//							can't form closures right?
//			alt 2: native calls are "embedded" in script functions, they also get a frame, only difference is that they execute natively, probably via a dummy stub that calls the native code
//					pros:	probably cleaner and easier to make safe (all args are protected by a frame object)
//							an eval call could create a closure (but what would it access, I guess that would be up to the native function tho)
//					cons:	slower, more garbage (frames)
//			alt 1 is regarding functions in themselves to be either script or native
//			alt 2 is regarding code to be either script or native
//			IMPORTANT! There must be a call procedure that doesn't need to differentiate between script and native functions.
//		arguments container
//			missing callee or whatever
//		at any point where we call a c++ virtual I think we should let the interpreter have a way to update-internals and possibly repeat the instruction. E.g. for implementing getters and setters.
//
//	Compiler
//
//		don't accept lf's after ++ etc (automatic ; insertion logic)
//		global environment compiler (don't accept return etc)
//			global is really a function frame too, with vars that you can't delete etc...
//		exceptions (and catching both virtual and native)
//			only thing left is integration with C++ exceptions
//		instanceof operator
//		array literals
//		object literals
//
//	GC
//
//		we ought to have a min and max-roof, min before gc'ing at all, over max = error
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
//		tedious to have to create string globals for constants strings (e.g. for setting properties), is the only alternative to new them?
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
//				might be a third option: eval code that isn't executed via "eval" direct-call is treated differently according to http://www.ecma-international.org/ecma-262/5.1/#sec-10.4.2
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
//		assignable globals this in processor
//		function name() { } syntax
//		literal function names, e.g. myGlobalName = function myLocalName() { ... }

#if !defined(NuXJScript_h)
#define NuXJScript_h 1

#include "assert.h"
#include <string>

typedef unsigned short Char;
typedef int Int32;
typedef unsigned int UInt32;
typedef unsigned char Byte;
typedef Int32 CodeWord; // FIX : int or byte?
const int OPERAND_SIZE = (sizeof (Int32) / sizeof (CodeWord));

const UInt32 TABLE_BUILT_IN_N = 3; // 1 << 3 == 8
const size_t STANDARD_BUILT_IN_COUNT = 16;
const double EPSILON = 1.0e-300;

// FIX : name
template<typename T, size_t BUILT_IN_COUNT = STANDARD_BUILT_IN_COUNT> class QuickArray {
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
				
				void resize(size_t newSize) {
					QuickArray<T, BUILT_IN_COUNT> na(newSize);
					std::copy(p, p + std::min(n, newSize), na.p);
					swap(na);
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

template<typename T, size_t BUILT_IN_COUNT = STANDARD_BUILT_IN_COUNT> class QuickVector {
	public:		explicit QuickVector(size_t capacity = BUILT_IN_COUNT) : qArray(capacity), count(0) { }
				bool empty() const { return count == 0; }
				size_t size() const { return count; }
				operator T*() const { return qArray.begin(); }
				size_t capacity() const { return qArray.size(); }
				T* begin() const { return qArray.begin(); }
				T* end() const { return qArray.begin() + count; }
				void reserve(size_t newSize) {
					if (newSize > capacity()) qArray.resize(std::max(newSize, capacity() * 2));
				}
				void resize(size_t newSize) { reserve(newSize); count = newSize; }
				void insert(T* p, const T* b, const T* e) {
					assert(0 <= p && p <= begin() + count);
					assert(b <= e);
					size_t o = p - begin();
					reserve(count + (e - b));
					p = begin() + o;
					std::copy(p, end(), p + (e - b));
					count += (e - b);
					std::copy(b, e, p);
				}
				void append(const T& t) { insert(end(), &t, &t + 1); }
				void shrink() { qArray.resize(count); }
	protected:	QuickArray<T, BUILT_IN_COUNT> qArray;
				size_t count;
};				

class GCList;
class GCHeap;

/*
	GCItems normally belong to heaps*. You put them there by calling GCHeap::manage() or GCHeap::protect(). Every GCItem
	class is responsible for "marking" all GCItems it has references to. If it fails to do this, items that are still in
	use may get garbage collected (= programming error).
	
	GCItems should never refer to items in a different heap than its own, unless the items referred to in turn do not
	refer to items in the first items heap. In other words, circular references between different heaps are not allowed.
	
	In rare circumstances it is ok to not place a GCItem into a heap at all. The item will in this case never be
	a candidate for garbage collection, but it will also never mark any of its references. In other words, this item
	must be a terminal leaf that has no further references.
	
	When a GCItem is destructed (regardless if it is from automatic garbage collection or not) it is removed from the
	heap it belongs to. This enables heaps to contain a mixture of automatically garbage collected and manually
	memory managed items.
	
	* They actually belong to particular lists inside heaps.
*/
class GCItem {
	friend class GCList;
	friend class GCHeap; // FIX : move stuff to GCList so GCHeap doesn't have to be friend

	public:		GCItem() throw();
				GCItem(const GCItem& copy) throw();
				GCItem& operator=(const GCItem& copy) throw();
				virtual void gcMark(GCHeap& heap);
				virtual ~GCItem();

	protected:	GCList* _gcList;
				GCItem* _gcPrev;
				GCItem* _gcNext;
};

// FIX : what should we do with items that are left in a GCList that destructs? I guess at least "release" them from the
// list (_gcList = 0). But in GCHeap, managed items ought to be deleted, right?
// These are double-linked circular lists with a dummy node first to minimize code complexity.
class GCList : public GCItem {
	public:		GCList() throw();
				size_t size() const;
				void claim(GCItem* item) throw();
				void relinquish(GCItem* item) throw();

	protected:	size_t count;
};

class GCHeap {
	public:		GCHeap();
				template<class T> T* manage(T* item) throw() { currentList->claim(item); return item; }
				template<class T> T* protect(T* item) throw() { rootList.claim(item); return item; } // FIX : don't like this name, it's really also "manage", but don't auto-free
				size_t size() const;
				size_t gc(); // FIX : gc(beyondItem) as in NuXScript was perhaps a very good idea, not sure we should have "protect" at all?
				void mark(const GCItem* item);
				bool belongs(const GCItem* item) { return (item->_gcList == currentList || item->_gcList == newList || item->_gcList == &rootList); }

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
	public:		Value()
				#if !defined(NDEBUG)
						: type(BAD_TYPE)
				#endif
						{ }
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
				const String* typeOfString() const;
				bool toBool() const;
				Int32 toInt() const;
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

				static const Value NONEXISTENT;		// Used in tables for deleted entries. Never seen by script-code.
				static const Value UNDEFINED;
				static const Value NUL; // Sorry, NULL is a sensitive word, e.g. macro in Windows

	protected:	enum InternalType {
					NONEXISTENT_TYPE, UNDEFINED_TYPE, NULL_TYPE, BOOLEAN_TYPE
					, NUMBER_TYPE, STRING_TYPE, OBJECT_TYPE
				#if !defined(NDEBUG)
					, BAD_TYPE = 0xBAADBAAD		// Only used when NDEBUG is not defined to detect use of uninitialized values.
				#endif
				};
				explicit Value(InternalType type) throw()  : type(type) { }
				bool compareStrictly(const Value& y) const;
				InternalType type;
				union {
					bool b;
					double d;
					const String* s;
					Object* o;
				};
				static const String* TYPE_STRINGS[7];
	
	private:	explicit Value(const void* v); // N/A. Just to avoid unwanted casting of a pointer to bool.
};

class Engine;
class Enumerator;
class Function;
class Processor;
class Object : public GCItem {
	public:		typedef GCItem super;
				
				virtual Function* asFunction(); // FIX : needed?
				virtual Function* getFunction(); // returns a stub function that throws if Object is not function
				virtual const String* typeOfString() const;
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
	public:		typedef UInt32 KeyHash;
				
				struct Bucket {
					Bucket();
					KeyHash hash;
					const String* key;
					Value value;
				};
				
				Table(UInt32 n = TABLE_BUILT_IN_N);
				UInt32 getLoadCount() const;
				const Bucket* getFirst() const;
				const Bucket* getNext(const Bucket* b) const;
				// FIX : we need a better way to lookup & update in one, right now it needs separate lookup and assign (see EvalVarsObject)
				const Bucket* lookup(const String* key) const; // Important: In a future implementation, Table *might* store a reference to 'key', so 'key' has to live as long as Table does (e.g. have it managed by the GCHeap).
				void assign(const String* key, const Value& value); // Important: Table will store a reference to 'key', so 'key' has to live as long as Table does (e.g. have it managed by the GCHeap).
				void erase(const String* key); // Important: In a future implementation, Table *might* store a reference to 'key', so 'key' has to live as long as Table does (e.g. have it managed by the GCHeap).
				void markIn(GCHeap& heap);

	protected:	static UInt32 calcMaxLoad(UInt32 bucketCount);
				static UInt32 suggestN(UInt32 count); // FIX : keep?
				void update(const String* key, const Value& value);
				Bucket* find(const String* key, KeyHash hash) const;
				UInt32 rebuild(UInt32 newN);
				
				UInt32 n; ///< Size of member table is two to the power of n.
				QuickArray<Bucket, 1U << TABLE_BUILT_IN_N> buckets;
				UInt32 loadCount; ///< Count of buckets with defined keys.
};

/*
	Note that although the ECMAScript specification clearly separates string values from String objects, this
	implementation regards both as Objects for efficiency (e.g. for quickly retrieving properties like the character
	elements and "length").
	
	String is immutable, meaning that it will never contain any references to other objects. Therefore it is ok to not
	place them in a heap. (See GCItem for more info.)
	
	The ECMAScript String object on the other hand is represented by a mutable UserObject.
*/
class String : public Object, public QuickArray<Char> {
	public:		typedef Object super;
				typedef QuickArray<Char> QArray;

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
				virtual const String* typeOfString() const;
				virtual const String* className() const;
				virtual bool getProperty(Engine& engine, const String* name, Value* v);
				virtual Enumerator* getEnumerator(Engine& engine);
				Table::KeyHash calcHash() const;
				bool isEqualTo(const String& o) const;
				bool isLessThan(const String& o) const;
				std::wstring toStdString() const;

	protected:	mutable size_t hash;
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

class Function : public UserObject {
	public:		typedef UserObject super;

				virtual Function* asFunction();
				virtual Function* getFunction();
				virtual const String* typeOfString() const;
				virtual const String* className() const;
				virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue) = 0;
				virtual Value construct(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue);
};

class EvalFunction : public Function {
	public:		virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue);
				virtual void gcMark(GCHeap& heap);
	protected:	Table functionDefCache; // eval() has a cache of source code strings -> functions (as they are neutral to which closure they run in). It is emptied on each big gc sweep.
};

class Engine {
	friend class String; // FIX : to access stringPrototype only
	
	public:		Engine(GCHeap& heap, Object* globals);
				GCHeap& getHeap() const { return gcHeap; }
				Object* getGlobals() const { return globals; }

	protected:	GCHeap& gcHeap;
				UserObject stringPrototype;
				Object* globals;
				EvalFunction evalDirectFunction;
};

// This is an Object so that we can store it as a constant and also in Table
class CodeDefinition : public Object {
	friend class FunctionFrame;
	friend class Compiler; // FIX : maybe not one day?
	
	public:		typedef Object super;
				
				CodeDefinition();
				virtual void gcMark(GCHeap& heap);
				bool lookupNameIndex(const String* name, Int32& index) const;
				Int32 getArgumentsCount() const { return argumentsCount; }
				const Value* getConstantsPointer() const { return (constants.empty() ? 0 : &constants[0]); }
				const CodeWord* getCodePointer() const { return (code.empty() ? 0 : &code[0]); }

	protected:	QuickVector<CodeWord> code;
				QuickVector<Value> constants;
				Int32 argumentsCount;
				Int32 varsCount; // not including arguments
				Table nameIndexes;
				const String* source;
				const String* selfName;				
};

class Closure : public Function {
	public:		typedef Function super;
				
				Closure(CodeDefinition* codeDef, Object* environment);
				virtual void gcMark(GCHeap& heap);
				virtual bool getProperty(Engine& engine, const String* name, Value* v); // for .length
				virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisValue);
				CodeDefinition* getDefinition() const;
				Object* getEnvironment() const;

	protected:	CodeDefinition* const codeDef;
				Object* const environment; // Usually the global object or the frame of the context that created this function.
};

class Frame : public Object {
	public:		typedef Object super;
				Frame(const CodeDefinition* codeDef) : codeDef(codeDef) { }
				const CodeDefinition* getFunctionDef() const { return codeDef; }
				Value* getLocalsPointer() const { return localsPointer; }
				virtual void mergeEvalVars(GCHeap& heap, CodeDefinition* def) = 0;
				
	protected:	const CodeDefinition* const codeDef;
				Value* localsPointer; // Pointer is offset so that negative indexes addresses local variables and positive indexes addresses arguments.
};

class Arguments;
class EvalVarsObject;
class FunctionFrame : public Frame {
	public:		typedef Frame super;

				FunctionFrame(Closure* function, Int32 argc, const Value* argv);
				virtual void gcMark(GCHeap& heap);
				virtual bool getProperty(Engine& engine, const String* name, Value* v);
				virtual bool setProperty(Engine& engine, const String* name, const Value& v);
				virtual bool deleteProperty(Engine& engine, const String* name);
				virtual void mergeEvalVars(GCHeap& heap, CodeDefinition* evalFunctionDef);

	protected:	Closure* const selfFunction; // FIX : we have codeDef too in Frame, not *really* necessary, we should also contemplate adding codeDef pointer to CStackEntry directly (see below)
				const Int32 passedArgumentsCount;
				QuickArray<Value> locals; // Includes variables and arguments.
				Value argumentsValue; // FIX : I don't appreciate this solution 100%, would be nicer if "arguments" could be part of evalVars (see below), or maybe even a true local?
				Object* environment;
				// FIX : rename below to "DynamicVars" or something and use it also for Arguments and selfName, although selfName is not writable... 
				EvalVarsObject* evalVars; // Created at first eval() call under this frame. Will be chained as an environment *above* this frame and contain variable slots for each var declared by the eval string.				
};

// FIX : if CStackEntry included a codeDef pointer, we wouldn't need this class at all, really worth contemplating. It is really a STUPID class, passing all virtuals on to parent, just setting a different codeDef really.
/*
	This is the direct frame of an eval(), it only bridges all virtuals to the parent FunctionFrame, but has a different
	codeDef pointer (for constants etc).
*/
class EvalFrame : public Frame {
	public:		typedef Frame super;

				EvalFrame(const CodeDefinition* evalFunctionDef, Frame* parentFrame);
				virtual void gcMark(GCHeap& heap);
				virtual bool getProperty(Engine& engine, const String* name, Value* v);
				virtual bool setProperty(Engine& engine, const String* name, const Value& v);
				virtual bool deleteProperty(Engine& engine, const String* name);
				virtual void mergeEvalVars(GCHeap& heap, CodeDefinition* evalFunctionDef);

	protected:	Frame* parentFrame;
};

/*
	The CatchFrame is necessary because within a catch block there is a single extra variable that is local to the
	catch block. It is created dynamically and pushed on the call stack when entering a catch block and removed at any
	exit point.
*/
class CatchFrame : public Frame {
	public:		typedef Frame super;

				CatchFrame(Frame* parentFrame, const String* exceptionName, const Value& exceptionValue);
				virtual void gcMark(GCHeap& heap);
				virtual bool getProperty(Engine& engine, const String* name, Value* v);
				virtual bool setProperty(Engine& engine, const String* name, const Value& v);
				virtual bool deleteProperty(Engine& engine, const String* name);
				virtual void mergeEvalVars(GCHeap& heap, CodeDefinition* evalFunctionDef);

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

// FIX : test with very short max sz length
class Exception : public std::exception, public GCItem {
	public:		Exception(GCHeap& heap, const Value& value) throw() : value(value) { // FIX : not inline
					strcpy(sz, "Unknown exception (failed conversion to string)");
					heap.protect(this);
					try {
						const String* src = value.toString(heap);
						const size_t l = (sizeof (sz) / sizeof (*sz) - 1);
						*std::copy(src->begin(), (src->size() < l ? src->end() : src->begin() + l), sz) = 0;
					}
					catch (...) {
						assert(0);
					}
				}
				const Value& getValue() const { return value; }
				virtual const char* what() const throw() { return sz; }													// what() is for compatibility only and has a length limitation. Use getValue() and convert to string to get the full length unicode exception string.
				virtual ~Exception() throw() { }																		// (GCC requires explicit destructor with one that has throw().)

	protected:	Value value;
	protected:	char sz[512]; // We use a fixed length buffer to store a char string version of the exception value. We do not use std::string or String to avoid potential throws in the constructor.
};

class Processor : public GCItem {
	public:		typedef GCItem super;

	public:		struct Continuation {
					public:		virtual bool execute(Engine& engine, Processor& processor, Object* thisValue) = 0;
				};
	
				// FIX : this could be a struct used by a member Processor too instead of separate ip, frame etc...
				struct CStackEntry {
					const CodeWord* returnIP;
					Frame* frame; // doesn't change during function execution
					Object* thisValue; // doesn't change during function execution
				};

				// FIX : {ip, vsp, csp} could be a struct used by Processor too, or.. CStackEntry?
	protected:	struct Catcher {
					Catcher() { }
					Catcher(const CodeWord* ip, Value* vsp, CStackEntry* csp) : ip(ip), csp(csp), vsp(vsp) { }
					const CodeWord* ip;
					Value* vsp;
					CStackEntry* csp;
				};
				
				QuickVector<Catcher> catchers; // FIX : QuickVector slow? linked list?
				QuickVector<Continuation*> continuations; // FIX : QuickVector slow? linked list? in callstack directly? I have a feeling there is a more elegant solution to all this

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
					, CALL_METHOD_OP								// operand: n, stack: object, name, n * args -> return value
					, CALL_EVAL_OP									// operand: n, stack: object, name, n * args -> return value, special eval call is required because of need to differentiate direct or indirect call to eval
					, NEW_OP										// operand: n, stack: constructor object, n * args -> return value, new object
					, NEW_RESULT_OP									// stack: new object, return value -> new this value
					, RETURN_OP
					, RETURN_TO_HOST_OP
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

	public:		Processor(Engine& engine, CStackEntry* cstackStart, CStackEntry* cstackEnd, Value* vstackStart, Value* vstackEnd);
				virtual void gcMark(GCHeap& heap);
				void chain(const CodeWord* stub);
				void chain(Continuation* continuation, Object* thisValue);
				void chain(Function* completion);
				void chain(Closure* func, Int32 argc, const Value* argv, Object* thisValue);
				void enterEval(CodeDefinition* codeDef);
	// FIX :			void call(Closure* func, Int32 argc, const Value* argv, Object* thisValue);
				void push(const Value& v); // FIX : wow, this feels low level, public really?
				Value& peek(Int32 offset);
				void pop(Int32 count = 1);
				void resetTimeOut(Int32 clockCycles);
				bool run();
void error(); // FIX
				
	protected:	void poppush(const Value& v);
				Int32 operand();
				bool toPrimitiveTrap(const CodeWord* rollbackIp);
				void updateInternals();
				bool privateRun();
				void throwVirtualException(const Value& exception);

				Engine& engine;
				CStackEntry* const cstackStart;
				CStackEntry* const cstackEnd;
				Value* const vstackStart;
				Value* const vstackEnd;

public: // FIX : not public, just for some asserts now I think
				Int32 clockCyclesLeft;
				const CodeWord* ip;
				Value* vsp; // FIX : not public, just for an assert now
				CStackEntry* csp;
				Frame* frame;
				const Value* constants;
				Value* locals;
};

enum Precedence {
	GROUP_PREC, PROPERTY_PREC, NEW_PREC, FUNCTION_CALL_PREC, POST_INC_DEC_PREC, PREFIX_PREC, MUL_DIV_PREC, ADD_SUB_PREC
	, SHIFT_PREC, RELATIONAL_PREC, EQUALITY_PREC, BIT_AND_PREC, BIT_XOR_PREC, BIT_OR_PREC, LOGICAL_AND_PREC
	, LOGICAL_OR_PREC, CONDITIONAL_PREC, ASSIGN_PREC, COMMA_PREC, LOWEST_PREC
};

struct OperatorInfo;
class Compiler : public GCItem {
	public:		typedef GCItem super;
				
				enum Target { FOR_GLOBAL, FOR_FUNCTION, FOR_EVAL };
				
				Compiler(GCHeap& heap, CodeDefinition* codeDef, Target compileFor);
				virtual void gcMark(GCHeap& heap);
				const Char* compile(const Char* b, const Char* e);
				void compile(const String& source);

	protected:	struct ExpressionResult;
				struct Scope;

				void syntaxError(const String& what);
				void syntaxError(const std::string& what); // FIX : try to get rid of?
				void emit(Processor::Opcode op);
				void emitWithInt32(Processor::Opcode op, Int32 i);
				void emitWithConstant(Processor::Opcode op, const Value& constant);
				Int32 emitForwardBranch(Processor::Opcode op);
				void completeForwardBranch(Int32 point);
				Int32 markBackwardBranch();
				void emitBackwardBranch(Processor::Opcode op, Int32 point);

				ExpressionResult safeKeep(const ExpressionResult& xr); // Used to store result into a new temporary local (used by return statement if there are finally blocks to call first)
				void returnSafeKept(const ExpressionResult& xr, bool doReset); // if doReset == true then reset the local variable used for safe-keeping (to UNDEFINED)
				ExpressionResult makeRValue(const ExpressionResult& xr); // Creates an r-value (i.e. pushed on value stack) out of a result.
				ExpressionResult makeAssignment(const ExpressionResult& xr); // Creates an assignment out of an l-value (throws if xr is not a valid l-value)
				ExpressionResult discard(const ExpressionResult& xr); // Discards result (e.g. popping it if it is on stack)

				void completeContinues(const Scope* fromScope, const Scope* untilScope);
				void completeBreaks(const Scope* scope);

				ExpressionResult operand(const OperatorInfo& op);
				ExpressionResult rvalueOperand(const OperatorInfo& op);
				ExpressionResult functionCall(Processor::Opcode op);
				bool operate(ExpressionResult& xr, Precedence precedence, int opCount, const OperatorInfo* opList);
				ExpressionResult expression(Precedence precedence);
				ExpressionResult rvalueExpression(Precedence precedence = LOWEST_PREC);
				CodeDefinition* functionDefinition(const Char* selfNameBegin, const Char* selfNameEnd);
				bool optionalExpression(ExpressionResult& xr, Precedence precedence);
				ExpressionResult declareIdentifier();
				ExpressionResult varDeclaration();
				void varDeclarations();
				void forInStatement(Scope* emptyLabelScope, Scope* scopeLabelsEnd, const QuickVector<CodeWord>& iterCode
						, const ExpressionResult& iterXR, ExpressionResult& evalXR);
				void forStatement(Scope* emptyLabelScope, Scope* scopeLabelsEnd, ExpressionResult& evalXR);
				void functionStatement();
				void statement(Scope* firstScope, Scope* scopeLabelsEnd, ExpressionResult& evalXR);
				void statementList(Scope* firstScope, ExpressionResult& evalXR);
				bool token(const char* t, bool eatLeadingWhite); // FIX : not all of these, and name!
				void semicolon();
				void expectToken(const char* t, bool eatLeadingWhite);
				void white();
				void token();
				Char* unescape(Char* buffer, const Char*& p, const Char* e);
				bool isLeadingIdentifierChar(Char c);
				const Char* identifier(bool required, bool acceptReservedKeywords);

				GCHeap& heap; // FIX : necessary?
				CodeDefinition* const codeDef;
				const Target compilingFor;
				std::map<Value, Int32> definedConstants;
				QuickVector<CodeWord> funcInitCode; // function declarations, inserted at top of function when finalizing
				const Char* p;
				const Char* e;
				QuickVector<Int32> freeSafeKeepingIndexes;
				Int32 safeKeptIndexCount; // Just used for assert. FIX : remove or what?
				QuickVector<CodeWord>* outputCode; // FIX : these two are bad, they are set and reset in blocks, which is what we normally use the C++ stack for really, e.g. create a new local compiler with the change
				bool acceptInOperator;
};

#endif
