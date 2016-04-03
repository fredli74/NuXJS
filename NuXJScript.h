// --- TODO ---
//
//	Run-time
//
//		count (and run-time check) necessary value stack depth for functions
//			alternative have a low-rate check with a margin + make certain there is never a possibility to push more than x per instruction
//				we should build a VM test routine that checks each opcode, how much it changes the stack and stuff
//			also implement a max recursion on compiler to not overload stack (function in function in function etc)
//		array object (length property support, optimized for numerical indexes?)
//			take value as key for getproperty etc and we could optimize array and string accesses by not having to go via strings
//			only downside to that would be read and write named, which assumes the constant is a string pointer
//			also, how to efficiently convert between continuous array and sparse array / associative array... have QuickArray *and* Table in the object?
//			ok, one thought: normally an array only contains a cotninuous array (with start and end)... when too many items are (or are to become) undefined, or when non-integer elements are added, convert to a normal ScriptObject (accessed as through a proxy now)
//			more thoughts: use different compressed variations (templates) when type is homogenous over all the array, e.g. number array, object array, string array. One could even consider an int array.
//		basic library (math, string, array ops)
//		for in should also include properties in prototypes, except when they are shadowed... don't know if we should do this top-down or bottom-up
//			done, but not for strings and arguments objects
//		it would be nice if it wasn't so easy to forget that Value() didn't create an undefined type, have a SafeValue()? That also maybe even protects from garbage-collect, that would be nice :)
//		lots of problems with gc safety and native calls / constructors: newed objects, methods, this values are not protected
//			push them temporarily last on stack
//			 or hold on, I think the entire gc approach needs rethinking, we should have something like gc(beyond) like in NuXScript...
//			actually, the problem isn't huge as we have an asynchronous vm, we could restrict gc to happen in "bottom" main loop
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
//				check futures/promises in C++11, is that something useful for chaining?
//		arguments container
//			missing callee
//				that is, if we don't slaughter the whole stupid non-strict argument object and go for the simpler strict version
//		at any point where we call a c++ virtual I think we should let the interpreter have a way to update-internals and possibly repeat the instruction. E.g. for implementing getters and setters.
//		should "\/" be valid escape sequence? not according to ECMAScript spec, but JSON allows it right? (or wrong?)
//			actually, any \ that isn't escape is valid, \ is just swallowed see http://www.ecma-international.org/ecma-262/6.0/index.html#sec-literals-string-literals
//			also \<cr>|<crlf>|<lf>|<ls>|<ps> is valid and should continue string on next line
//		why is 0000nnn valid number in js, v8 etc? in ES 11.8.3 Numeric Literals it looks like a number need to start with a non-zero
//		perhaps we should separate native and script calls after all... I think one wants to use script stubs for a lot of stuff. native calls could be looked up during compile time and made into super-quick constants
//		exceptions over C++ callbacks (js->c++->js, should be able to throw from js, catch in c++, rethrow and capture in js)
//
//	Compiler
//
//		don't accept lf's after ++ etc (automatic ; insertion logic)
//		global environment compiler (don't accept return etc)
//			global is really a function frame too, with vars that you can't delete etc...
//		exceptions (and catching both virtual and native)
//			only thing left is integration with C++ exceptions
//		support -0.0 (use std::signbit? or whatever...) add to operation matrix test
//		with statement, it's actually not too bad... check http://stackoverflow.com/questions/61552/are-there-legitimate-uses-for-javascripts-with-statement
//		FIX : The SourceCharacter immediately following a NumericLiteral must not be an IdentifierStart or DecimalDigit.    For example:     3in     is an error and not the two input elements 3 and in.
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
//		function.name is actually not a bad idea, although standardized first in ES6 (in a very complex manner where x = function() also get a name)...
//			good to have the name accessible for tracing/debugging (future stack-trace support?)
//		perhaps it would have been better to split Frame into different pointers (more similar to ES-spec): one for running code (not changed by catch and with), one for variable object (not changed by catch, with or eval) and "current scope object" (changed by them all). This way we wouldn't have to declare so many dummy virtuals that just passes stuff upwards the Frame chain.
//
// Compiler
//
//		better errors with line numbers and stuff (should we go back to the global "pickup" idea i had in old NuXScript?)
//			should pass char* in exception probably, as it is now new Compilers are created for each function compiling and the Compiler owns the char*, which means in a throw we lose the correct char*
//		safe-kept variables aren't resetted on early exits, e.g.
//			b: switch (x) { case 1: switch(x) { case 2: break b } }
//			won't reset the safe kept x's, so there is a risk this kind of code holds on to more garbage than necessary (but freed at function exit of course!)
//		max recursion on compiler for sandboxed mode
//
// Other
//
//		included in tests should be to config the gc to sweep after every instruction
//		tedious to have to create string globals for constants strings (e.g. for setting properties), is the only alternative to new them?
//		optional version without exceptions?!
//			would work except for the compiler, how to abort compiler? longjump? I know it is ugly, but this woul be for embedded systems etc that disable exceptions. Would be nice to support them.
//			... or I suppose we could change all methods to return a bool :(
//		also, drop all STL dependencies, incl. std::string even, for simpler / older / stupid compiler compatibility and smaller footprint
//		this ES-262 test fails, why exactly, the number looks right:
//			if (+(Number.MAX_VALUE) !== 1.7976931348623157e308) {
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
//			another option would be to not require a frame *at all* for quick calls that doesn't have eval or sub-functions (is it possible?). Just allocate space on the value stack. But what to do with accepting too few arguments?
//			third option: simply focus on making a fast mem pool instead
//				oh snap, it's the gc that's the main performance problem, not the allocation/freeing... gotta look into that
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
//		big array literals goes nuts with ADD_PROPERTY constants, should have an PUSH_ELEMENT or something instead
//			or PUSH_ELEMENTS and PUSH_CONST_ELEMENTS and have a counter for both. PUSH_ELEMENTS allows us to push in batches of say max 32 elements on stack.
//		not a big fan of the POST_INC_DEC_SHUFFLE_OP solution, feels ugly, but can't for the world come up with any simpler solutions!?
//		PRE_EQ_OP isn't actually necessary, we could just backup ip once if isObject and retry the EQ after toPrimitive (same applies to TO_PRIMITIVE_OP actually, but that's a bigger story considering the virtually impossible task of "primitivizing" two stack elements at once for binary operators)
//			alternatively have two opcodes, swap on type < (like in isEqualTo today) and save CPU on isEqualTo as operands are already sorted
//		.length is very common, special opcode?
//		arguments and <self name>, couldn't they be indexed variables too instead of accessed through slow READ_NAMED? Either created only when present in code or allocated on demand?
//		"get parent index" opcode, it is very common to access variables one level up (two levels up not so much), and parent frame never changes in a closure so...
//		internal int type
//		so, it's definitely worth it to "consolidate" / "internalize" all strings so that all identifiers get the same pointer (proven by the chess benchmark). The question is: how do we do this thread agnostic?
//			there is also the "string sync" idea from old NuXScript... it could do something like { if (sameString && pointer < other.pointer && heap is 0 or mine) pointer = other.pointer } on all Values compared
//				the only problem is that it won't help stuff that is pushed on the stack only, e.g. a[x] = y <- both x and y are pushed data on stack and changing those pointers really won't help anyone :)
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
//			can't do normal quick-hashing since it requires string length... perhaps I can develop a quick hashing that matches beginning of strings only?
//				it would be required to use && to never look beyond the max length of all "current candidates"
//		couldn't we share identical strings in the same engine? any disadvantage to that?
//		simple constants pre-calculations
//		let ExpressionResult remember if a value is already primitive, no need to make primitive again then, e.g. (5+a)*8 <- two TO_PRIMITIVE today, could be one
//		also would be good if a["ko"] or a[3423] didn't create a TO_STRING_OP
//		instead of flushing/discarding results of statements directly, couldn't we keep them until the next statement? i.e. eliminating stuff like "WRITE_INDEXED #-1", "POP", "READ_INDEXED #-1" (except when branch targets of course)
//
//	GC
//
//		take a shot at generational gc
//			couldn't it work if we just make sure that a reference from old -> new gen protects the item in new gen until we sweep the old gen?
//			actually, I don't think we should think new and old gen... we should think standard heap and "sleepy" heap..
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
//		++ should add numbers *only*, now "3"++ is "31", which is wrong
//		eval support
//			I think the if in the "if (compilingForEval) evalXR = makeRValue(evalXR);" lines are actually unnecessary? 
//				nope, they weren't
//			implement rest of eval r-value logic for do while, for, switch and try / catch (any more?)
//				while, for done, switch and try / catch done, I think, any more?	
//			global eval support (sets all var and function declarations in global this)
//		object literals
//		array literals
//		true primitive <-> object conversions
//		true toString, toNumber, which is problematic because we might have to execute JS-code then and I want to avoid synchronous callbacks to JS-code (=blocking!)
//		string -> number doesn't conform to the http://www.ecma-international.org/ecma-262/5.1/#sec-9.3.1 , e.g. empty string should be 0 , spaces should be accepted etc...
//		are we comparing NaN correctly? e.g. NaN !== NaN
//			yes
//		I am beginning to give up on the idea of avoiding the property attributes... we need Writable, Enumerable & Configurable... perhaps place them in the hash key?
//		vars added by eval should be deletable: f=function() { eval("var x = 42"); delete x; print(x); }
//		proper error exceptions (TypeError etc)
//		instanceof operator
//		should we? *) Doesn't consider unicode Line separator <LS> (\u2028) or Paragraph separator <PS> (\u2029) to be linefeeds. Only LF or CR. Also <NBSP> (\u00A0) and <ZWNBSP> ZERO WIDTH NO-BREAK SPACE (\uFEFF) are not considered white space.
//		(new) Object(v) should convert v to number, string object etc
//		toPrimitive also for native function arguments like sqrt etc, how?
//			stubs

#if !defined(NuXJScript_h)
#define NuXJScript_h 1

#define STRICTLY_ES3 1

#include "assert.h"
#include <string>
#include <cstring>

typedef unsigned short UInt16;
typedef int Int32;
typedef unsigned int UInt32;
typedef unsigned char Byte;
typedef UInt16 Char;
typedef Int32 CodeWord; // FIX : int or byte?
typedef Byte Flags;
const int OPERAND_SIZE = (sizeof (Int32) / sizeof (CodeWord));

const UInt32 TABLE_BUILT_IN_N = 3; // 1 << 3 == 8
const UInt32 STANDARD_BUILT_IN_COUNT = 16;
const double EPSILON = 1.0e-300;

// FIX : name
template<typename T, UInt32 BUILT_IN_COUNT = STANDARD_BUILT_IN_COUNT> class QuickArray {
	public:		explicit QuickArray(UInt32 n = BUILT_IN_COUNT) : n(n), p(n <= BUILT_IN_COUNT ? a : new T[n]) { }
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
				
				void resize(UInt32 newSize) {
					QuickArray<T, BUILT_IN_COUNT> na(newSize);
					std::copy(p, p + std::min(n, newSize), na.p);
					swap(na);
				}
				
				bool empty() const { return n == 0; }
				UInt32 size() const { return n; }
				operator T*() const { return p; }
				T* begin() const { return p; }
				T* end() const { return p + n; }
			// FIX : ambigious according to c++ standard or something, didnt' work -v
			#if 0 && !defined(NDEBUG) // FIX : test performance in release
				T& operator[](Int32 index) const { assert(0 <= index && static_cast<UInt32>(index) < n); return p[index]; } // FIX : shouldn't this be const T&, but that breaks some code...
				T& operator[](Int32 index) { assert(0 <= index && static_cast<UInt32>(index) < n); return p[index]; }
			#endif
				~QuickArray() { if (p != a) delete [] p; }
				
	protected:	UInt32 n;
				T* p;
				T a[BUILT_IN_COUNT];
};

template<typename T, UInt32 BUILT_IN_COUNT = STANDARD_BUILT_IN_COUNT> class QuickVector {
	public:		explicit QuickVector(UInt32 capacity = BUILT_IN_COUNT) : qArray(capacity), count(0) { }
				bool empty() const { return count == 0; }
				UInt32 size() const { return count; }
				operator T*() const { return qArray.begin(); }
				UInt32 capacity() const { return qArray.size(); }
				T* begin() const { return qArray.begin(); }
				T* end() const { return qArray.begin() + count; }
				void reserve(UInt32 newSize) {
					if (newSize > capacity()) {
						qArray.resize(std::max(newSize, capacity() * 2));
					}
				}
				void resize(UInt32 newSize) {
					reserve(newSize);
					count = newSize;
				}
				void insert(T* p, const T* b, const T* e) {
					assert(0 <= p && p <= begin() + count);
					assert(b <= e);
					UInt32 o = p - begin();
					reserve(count + (e - b));
					p = begin() + o;
					std::copy(p, end(), p + (e - b));
					count += (e - b);
					std::copy(b, e, p);
				}
				void append(const T& t) { insert(end(), &t, &t + 1); }
				void shrink() { qArray.resize(count); }
			// FIX : ambigious according to c++ standard or something, didnt' work -v
			#if 0 && !defined(NDEBUG) // FIX : test performance in release
				T& operator[](UInt32 index) const { assert(index < count); return qArray[index]; } // FIX : shouldn't this be const T&, but that breaks some code...
				T& operator[](UInt32 index) { assert(index < count); return qArray[index]; }
			#endif
	protected:	QuickArray<T, BUILT_IN_COUNT> qArray;
				UInt32 count;
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
				bool gcIsOnHeap() const { return _gcList != 0; };
				virtual void gcMark(GCHeap& heap) const;
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
				UInt32 size() const;
				void claim(GCItem* item) throw();
				void relinquish(GCItem* item) throw();

	protected:	UInt32 count;
};

class GCHeap {
	public:		GCHeap();
				template<class T> T* manage(T* item) throw() { currentList->claim(item); return item; }
				template<class T> T* protect(T* item) throw() { rootList.claim(item); return item; } // FIX : don't like this name, it's really also "manage", but don't auto-free
				UInt32 size() const;
				UInt32 gc(); // FIX : gc(beyondItem) as in NuXScript was perhaps a very good idea, not sure we should have "protect" at all?
				void mark(const GCItem* item);
				template<class T> void markViaPointers(T begin, T end) { for (T it = begin; it != end; ++it) mark(*it); } // FIX : is this good with two of them? one is for Value etc (they used to have markIn(heap), but now have gcMark() like GCItem)
				template<class T> void markViaReferences(T begin, T end) { for (T it = begin; it != end; ++it) it->gcMark(*this); }
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
class Engine;
class Function;
class Value {
	friend class Table;
	
	public:		Value()
				#if !defined(NDEBUG)
						: type(BAD_TYPE)
				#endif
						{ }
				explicit Value(bool boolean) : type(BOOLEAN_TYPE) { var.boolean = boolean; }
				explicit Value(Int32 number) : type(NUMBER_TYPE) { var.number = number; }
				explicit Value(UInt32 number) : type(NUMBER_TYPE) { var.number = number; }
				explicit Value(double number) : type(NUMBER_TYPE) { var.number = number; }
				explicit Value(const String* string) : type(STRING_TYPE) { var.string = string; }
				explicit Value(Object* object) : type(OBJECT_TYPE) { var.object = object; }
				// FIX : Value from (heap and) const char* ?
	
				bool exists() const { return type != NONEXISTENT_TYPE; } // FIX : I think NONEXISTENT_TYPE *can* and *should* disappear very soon, only used in "legacy code" now (WELL actually, arrays *might* need them in the future if we don't have full flags instead like in tables)
				bool isUndefined() const { return type == UNDEFINED_TYPE; }
				bool isNull() const { return type == NULL_TYPE; }
				bool isBoolean() const { return type == BOOLEAN_TYPE; }
				bool isNumber() const { return type == NUMBER_TYPE; }
				bool isString() const { return type == STRING_TYPE; }
				bool isObject() const { return type == OBJECT_TYPE; } // FIX : should we expose the type enum instead?
				Object* asObject() const { return (type == OBJECT_TYPE ? var.object : 0); }
				Function* asFunction() const;
				const String* getString() const { assert(type == STRING_TYPE); return var.string; }
				const String* typeOfString() const;
				bool toBool() const; // or toBoolean?
				Int32 toInt() const; // FIX : name toInt32
				bool toArrayIndex(UInt32& index) const;
				double toDouble() const;
				const String* toString(GCHeap& heap) const; // this toString() method does not run any script code, so it doesn't honor any user toString or valueOf implementations.
				Object* toObject(Engine& engine, bool requestExtensible) const; // FIX : I don't like that we involved engine here, not the end of the world but feels inconsistent, it is needed to create wrapper objects for numbers and booleans (and strings if makeExtensible)
				bool operator==(const Value& r) const; // FIX : kill these two?
				bool operator<(const Value& r) const;
				bool isStrictlyEqualTo(const Value& r) const;
				bool isEqualTo(const Value& r) const;
				bool isEqualTo(const String& s) const;
				bool isLessThan(const Value& r) const;
				bool isLessThanOrEqualTo(const Value& r) const;
				Value add(GCHeap& heap, const Value& r) const;
				void gcMark(GCHeap& heap) const;

				static const Value NONEXISTENT;		// Used in tables for deleted entries. Never seen by script-code.
				static const Value UNDEFINED;
				static const Value NUL; // Sorry, NULL is a sensitive word, e.g. macro in Windows
				static const Value NOT_A_NUMBER; // Sorry, NAN is sensitive, defined by some math.h
				static const Value INFINITE_NUMBER; // Sorry, INFINITY is also sensitive

				// WARNING! Do NOT change order of elements in Type enum.
	protected:	enum Type {
					NONEXISTENT_TYPE, UNDEFINED_TYPE, NULL_TYPE, BOOLEAN_TYPE
					, NUMBER_TYPE, STRING_TYPE, OBJECT_TYPE
				#if !defined(NDEBUG)
					, BAD_TYPE = 0xBAADBAAD		// Only used when NDEBUG is not defined to detect use of uninitialized values.
				#endif
				} type;
				union Variable {
					bool boolean;
					double number;
					const String* string;
					Object* object;
				} var;

				explicit Value(Type type) throw() : type(type) { }
				explicit Value(Type type, const Variable& var) throw() : type(type), var(var) { }
				bool compareStrictly(const Value& y) const;
				static const String* TYPE_STRINGS[7];
	
	private:	explicit Value(const void* v); // N/A. Just to avoid unwanted casting of a pointer to bool.
};

struct Attributes {
	Attributes() : readOnly(false), dontEnum(false), dontDelete(false) { }
	bool readOnly;
	bool dontEnum;
	bool dontDelete;
};

class Engine;
class Enumerator;
class Function;
class Processor;
class ScriptObject;

const Flags EXISTS_FLAG = 1;
const Flags READ_ONLY_FLAG = 2;
const Flags DONT_ENUM_FLAG = 4;
const Flags DONT_DELETE_FLAG = 8;
const Flags STANDARD_FLAGS = 0;		// use with setProperty()
const Flags NONEXISTENT = 0;		// use with getProperty() to check for existence, e.g. getProperty(o, k, v) != NONEXISTENT

// FIX : rename Object to something else, like AbstractObject, but AbstractObject sounds a bit like pure virtual abstract which isn't what it is, it's more like internal primitive object (which isn't good either cause primitive is something else in JS)
// InternalObject ?
class Object : public GCItem {
	public:		typedef GCItem super;
				
				virtual Function* asFunction();
				virtual const String* typeOfString() const;
				virtual const String* getClassName() const;
				virtual const String* toString(GCHeap& heap) const; // this toString() method does not run any script code, so it doesn't honor any user toString or valueOf implementations.
				virtual Value getInternalValue(GCHeap& heap) const;
				virtual Object* getPrototype(Engine& engine) const;

				virtual Flags getProperty(Engine& engine, const Value& key, Value* v) const;  // don't touch v if you return NONEXISTENT
				virtual bool setProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Engine& engine) const;

				// FIX : rename non-deep to Own and drop Deep, for consistency with JS lingo
				// FIX : or maybe drop them, we now check with getProperty
				bool hasOwnProperty(Engine& engine, const Value& key) const; // checks via getProperty()
				bool isOwnPropertyEnumerable(Engine& engine, const Value& key) const;
				Flags getPropertyDeep(Engine& engine, const Value& key, Value* v) const; // searches prototype chain
				bool setPropertyDeep(Engine& engine, const Value& key, const Value& v); // checks prototype chain for read-only
				Value getPropertyDeep(Engine& engine, const char* s) const; // helper function
};

/*
	LazyObjects become complete extensible "script objects" first when their properties are accessed. There are a few reasons why this is desirable:
	
	1) Performance: not having to setup a fully customizable object directly is benficial when it is rare that the user accesses properties or extends the object (but you still need an object reference). Example are Functions which are most often not treated as objects (i.e. constructors) by the user.
	2) Memory: until the user accesses or adds properties, LazyObjects can be super tiny (vtable pointer + pointer to complete object + whatever internal fields are needed).
	3) Doesn't require an Engine or even a Heap to be constructed. Although they are required to be placed on the heap since they contain a reference.
		// TODO: this object could actually own an auto_ptr to the complete object since that object should *never* be referenced to from elsewhere. Saving GC time. But they would still not be thread-safe to make static.

	This class is a template so this concept can be used with different super classes.
*/
template<class SUPER> class LazyObject : public SUPER {
	public:		typedef SUPER super;
				LazyObject();
				virtual Flags getProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Engine& engine) const;

	protected:	virtual void constructCompleteObject(Engine& engine) const = 0;
				ScriptObject* getCompleteObject(Engine& engine) const;
				mutable ScriptObject* completeObject;

				virtual void gcMark(GCHeap& heap) const {
					heap.mark(completeObject);
					super::gcMark(heap);
				}
};

class Enumerator : public Object {
	public:		typedef Object super;
				
				virtual Enumerator* asEnumerator() { return this; }
				virtual const String* nextPropertyName() = 0;
};

class Table {
				/*
					The only reason why Bucket doesn't contain the Value class, but rather holds it's own Byte type and
					Variable union, is memory. This solution makes it possible to squeeze in key flags and a truncated
					16-bit hash (for even quicker value lookup) in the same space as a Value.
				*/
	public:		class Bucket {
					friend class Table;
					
					public:		Bucket() : key(0) { };
								Value getValue() const { assert(valueExists()); return Value(static_cast<Value::Type>(type), var); }
								Flags getFlags() const { return flags; }
								const String* getKey() const { assert(keyExists()); return key; }
								bool doEnumerate() const { return ((flags & DONT_ENUM_FLAG) == 0); }
								bool hasStandardFlags() const { return flags == EXISTS_FLAG; }
								bool valueExists() const { return (key != 0 && ((flags & EXISTS_FLAG) != 0)); }
								
					protected:	const String* key;
								Byte flags;
								Byte type;
								UInt16 hash16;
								Value::Variable var;
								bool keyExists() const { return key != 0; }
				};
				
				Table(UInt32 n = TABLE_BUILT_IN_N);
				UInt32 getLoadCount() const;
				Bucket* getFirst() const;
				Bucket* getNext(Bucket* bucket) const;
				Bucket* lookup(const String* key) const;
				Bucket* insert(const String* key);
				bool update(Bucket* bucket, const Value& value, Byte flags = 0);
				bool erase(Bucket* bucket);
				void gcMark(GCHeap& heap) const;
				
	protected:	static UInt32 calcMaxLoad(UInt32 bucketCount);
				void update(const String* key, const Value& value);
				Bucket* find(const String* key, UInt32 hash) const;
				UInt32 rebuild(UInt32 newN);
				
				UInt32 n; ///< Size of member table is two to the power of n.
				QuickArray<Bucket, 1U << TABLE_BUILT_IN_N> buckets;
				UInt32 loadCount; ///< Count of buckets with defined keys.
};

/*
	Note that although the ECMAScript specification clearly separates string values from String objects, this
	implementation regards both as Objects for efficiency (e.g. for quickly retrieving properties like the character
	elements and "length").
	
	String is immutable and it will never contain any references to other objects. Therefore it is ok to not place them
	in a heap. (See GCItem for more info.)
	
	The ECMAScript String object on the other hand is represented by an extensible StringWrapper.
*/
class String : public virtual Object, public QuickArray<Char> {
	public:		typedef Object super;
				typedef QuickArray<Char> QArray;

				static const String* newHashedString(GCHeap& heap, const Char* b, const Char* e);
				static const String* fromInt(GCHeap& heap, Int32 i);
				static const String* fromDouble(GCHeap& heap, double d);

				String();
// FIX : neeeded?				String(const String& o);
				String(const Char* b, const Char* e);
				String(const String& l, const String& r);
				explicit String(const std::wstring& s);
				explicit String(UInt32 length) : QArray(length), hash(0) { } // FIX : dangerous, right, make protected?
// FIX : wasn't compatible with Digital Mars, so I adde const char* constructor again, but do we care?
//				template<UInt32 N> explicit String(const char (&s)[N]) : QArray(N - 1), hash(0) { std::copy(s, s + N - 1, begin()); calcHash(); }
				explicit String(const char* s) : QArray(strlen(s)), hash(0) { std::copy(s, s + size(), begin()); calcHash(); }
				virtual const String* typeOfString() const;
				virtual const String* getClassName() const;
				virtual const String* toString(GCHeap& heap) const;
				virtual Object* getPrototype(Engine& engine) const;
				virtual Flags getProperty(Engine& engine, const Value& key, Value* v) const;
				virtual Enumerator* getPropertyEnumerator(Engine& engine) const;
				virtual Value getInternalValue(GCHeap& heap) const { return Value(this); }
				UInt32 calcHash() const;
				bool isEqualTo(const String& o) const;
				bool isLessThan(const String& o) const;
				std::wstring toStdString() const;

	protected:	mutable UInt32 hash;
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
				StringListEnumerator(UInt32 initialCapacity);
				void add(const String* s);
				virtual const String* nextPropertyName();

	protected:	QuickVector<const String*> list;
				UInt32 nextIndex;

				virtual void gcMark(GCHeap& heap) const {
					heap.markViaPointers(list.begin(), list.end());
					super::gcMark(heap);
				}
};

class JoinedEnumerator : public Enumerator {
	public:		typedef Enumerator super;
				JoinedEnumerator(Engine& engine, Enumerator* leftEnumerator, const Object* rightObject);
				virtual const String* nextPropertyName();

	protected:	Engine& engine;
				Enumerator* const leftEnumerator;
				const Object* const rightObject;
				Enumerator* const rightEnumerator;

				virtual void gcMark(GCHeap& heap) const {
					heap.mark(leftEnumerator);
					heap.mark(rightObject);
					heap.mark(rightEnumerator);
					super::gcMark(heap);
				}
};

class ScriptObject : public virtual Object {
	public:		typedef Object super;
				
				ScriptObject(Object* prototype);
				virtual Object* getPrototype(Engine& engine) const;
				virtual Flags getProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Engine& engine) const;
				Table& accessProperties();
				
	protected:	Table properties;
				Object* prototype;

				virtual void gcMark(GCHeap& heap) const {
					properties.gcMark(heap);
					heap.mark(prototype);
					super::gcMark(heap);
				}
};

// fix : good candidate for a lazy object
class WrapperObject : public ScriptObject {
	public:		typedef ScriptObject super;
				WrapperObject(const String* className, const Value& value, Object* prototype);
				virtual const String* getClassName() const;
				virtual const String* toString(GCHeap& heap) const;
				virtual Value getInternalValue(GCHeap& heap) const;
	protected:	const String* const className;
				const Value wrapped;
				virtual void gcMark(GCHeap& heap) const {
					heap.mark(className);
					wrapped.gcMark(heap);
					super::gcMark(heap);
				}
};

#if (!STRICTLY_ES3)
// FIX : a lot like a LazyObject, would be good to merge somehow... Or at least, I want to have a safe compiler mechanism to make sure I always override *all* property accessor when doing things like this?!
class StringWrapper : public ScriptObject {
	public:		typedef ScriptObject super;
				StringWrapper(const String* string, Object* prototype);
				virtual const String* getClassName() const;
				virtual const String* toString(GCHeap& heap) const;
				virtual Value getInternalValue(GCHeap& heap) const;

				virtual Flags getProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Engine& engine) const;

	protected:	const String* const wrapped;
				virtual void gcMark(GCHeap& heap) const {
					heap.mark(wrapped);
					super::gcMark(heap);
				}
};
#endif

class Array : public LazyObject<Object> {
	public:		typedef LazyObject<Object> super;
	
				Array();
				virtual const String* getClassName() const;
				virtual Object* getPrototype(Engine& engine) const;
				// FIX : toString too?
				virtual Flags getProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Engine& engine) const;

	protected:	virtual void constructCompleteObject(Engine& engine) const;
				void sliceDenseVector(Engine& engine, const Value& key);
				UInt32 length;
				QuickVector<Value> denseVector;
				virtual void gcMark(GCHeap& heap) const {
					const_cast<Array*>(this)->denseVector.shrink(); // FIX : split into two different gc-things? it really *is* different to just mark stuff and actively shrink / compress stuff
					heap.markViaReferences(denseVector.begin(), denseVector.end());
					super::gcMark(heap);
				}
};

/*
	The Function base class, which can be used to interface with native C++ functions, does not reference other objects.
	Therefore it is ok to not place instances in a heap. (See GCItem for more info.)
*/
class Function : public Object {
	public:		typedef Object super;
				
				virtual Function* asFunction();
				virtual const String* typeOfString() const;
				virtual const String* getClassName() const;
				virtual const String* toString(GCHeap& heap) const;
				virtual Value getInternalValue(GCHeap& heap) const;
				virtual Object* getPrototype(Engine& engine) const;
				// FIX : all Engine& engine, Processor& processor -> Processor& processor only... engine can be accessed anyhow
				// FIX : all argc -> UInt32 maybe?
				virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject) = 0;
				virtual Value construct(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject);
				virtual bool hasInstance(Engine& engine, Processor& processor, Object* object) const;
};

class EvalFunction : public Function {
	public:		typedef Function super;
				EvalFunction(bool direct) : direct(direct) { }
				virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject);
				// FIX : move down to Engine so that we share with eval direct, *but* Engine should be a GCItem then I think
	protected:	bool direct;
				// FIX : share cache (in Engine) between evalFunction and evalDirectFunction, oh, and share with new function as well
				mutable Table functionDefCache; // eval() has a cache of source code strings -> functions (as they are neutral to which closure they run in). It is emptied on each big gc sweep.

				virtual void gcMark(GCHeap& heap) const {
					functionDefCache = Table(); // Yeah we just empty everything on each gc sweep
					functionDefCache.gcMark(heap);
					super::gcMark(heap);
				}
};

class Frame;

class Constants : public GCItem, public QuickVector<Value> {
	public:		typedef GCItem super;
	protected:	virtual void gcMark(GCHeap& heap) const {
					heap.markViaReferences(begin(), end());
					super::gcMark(heap);
				}
};

// This is an Object so that we can store it as a constant and also in Table
class CodeDefinition : public Object {
	friend class ScriptFrame;
	friend class Compiler; // FIX : maybe not one day?
	
	public:		typedef Object super;
				
				CodeDefinition(Constants* constants);
				bool lookupNameIndex(const String* name, Int32& index) const;
				UInt32 getVarsCount() const { return varNames.size(); }
				UInt32 getArgumentsCount() const { return argumentNames.size(); }
				const String* getIndexedName(Int32 index) const { return (index < 0 ? varNames[static_cast<UInt32>(~index)] : argumentNames[static_cast<UInt32>(index)]); }
				const Value* getConstantsPointer() const { return constants->begin(); }
				const CodeWord* getCodePointer() const { return (code.empty() ? 0 : code.begin()); }
				UInt32 getCodeSize() const { return code.size(); }
				const String* getName() const { return name; }
				const String* getSource() const { return source; }

	protected:	QuickVector<CodeWord> code;
				Constants* const constants;
				Table nameIndexes; // < 0 : local variables, >= 0 : arguments
				// FIX : bloated with all built-in quickvector counts here? vvv
				QuickVector<const String*> varNames; // Notice that this list is reversed in relation to indexes in the "indexed" array in Frame. A zero means temporary "safe-kept" variable. "vars" and "arguments" are only used for disassembler and merging eval vars right now.
				QuickVector<const String*> argumentNames;
				QuickVector<Int32> temporaries; // available temporary variable indexes (pushed when returning) // FIX : why is this kept here in CodeDefinition, do we ever need it again after compiling?
				const String* name;
				const String* selfName;
				const String* source;

				virtual void gcMark(GCHeap& heap) const {
					heap.mark(constants);
					nameIndexes.gcMark(heap);
					heap.markViaPointers(varNames.begin(), varNames.end());
					heap.markViaPointers(argumentNames.begin(), argumentNames.end());
					heap.mark(name);
					heap.mark(selfName);
					heap.mark(source);
					super::gcMark(heap);
				}
};

class ExtensibleFunction : public LazyObject<Function> {
	public:		typedef LazyObject<Function> super;
	protected:	void createPrototypeObject(Engine& engine, Object* forObject) const;				
};

class FunctionPrototypeFunction : public ExtensibleFunction {
	public:		typedef ExtensibleFunction super;
				virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject);
				virtual Object* getPrototype(Engine& engine) const;
	protected:	virtual void constructCompleteObject(Engine& engine) const;
};

class ScriptFunction : public ExtensibleFunction {
	public:		typedef ExtensibleFunction super;
				
				ScriptFunction(CodeDefinition* codeDef, Object* environment);
				virtual const String* toString(GCHeap& heap) const;
				virtual Value getInternalValue(GCHeap& heap) const;
				virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject);
				CodeDefinition* getDefinition() const;
				Object* getEnvironment() const;
	
	protected:	virtual void constructCompleteObject(Engine& engine) const;
	
				CodeDefinition* const codeDef;
				Object* const environment; // Usually the global object or the frame of the context that created this function.
				
				virtual void gcMark(GCHeap& heap) const {
					heap.mark(codeDef);
					heap.mark(environment);
					super::gcMark(heap);
				}
};

class SpecialConstructorFunction : public ExtensibleFunction {
	public:		typedef ExtensibleFunction super;

				SpecialConstructorFunction(Function* regularFunction, Function* constructorFunction);
				virtual Value call(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject);
				virtual Value construct(Engine& engine, Processor& processor, Int32 argc, const Value* argv, Object* thisObject);
	
	protected:	virtual void constructCompleteObject(Engine& engine) const;
				Function* const regularFunction;
				Function* const constructorFunction;

				virtual void gcMark(GCHeap& heap) const {
					heap.mark(regularFunction);
					heap.mark(constructorFunction);
					super::gcMark(heap);
				}
};

enum ErrorType {
	GENERIC_ERROR, RANGE_ERROR, REFERENCE_ERROR, SYNTAX_ERROR, TYPE_ERROR, ERROR_TYPE_COUNT
};

class Error : public LazyObject<Object> {
	public:		typedef LazyObject<Object> super;
				Error(ErrorType type, const String* message = 0);
				virtual const String* getClassName() const;
				virtual const String* toString(GCHeap& heap) const;
				virtual Value getInternalValue(GCHeap& heap) const; // error type name
				virtual Object* getPrototype(Engine& engine) const;
				virtual bool setProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				
	protected:	virtual void constructCompleteObject(Engine& engine) const;
				void updateReflection(Engine& engine);
				
				const ErrorType errorType;
				const String* name; // may get updated by script code
				const String* message; // may get updated by script code
				virtual void gcMark(GCHeap& heap) const {
					heap.mark(name);
					heap.mark(message);
					super::gcMark(heap);
				}
};

class ErrorPrototype : public Error {
	public:		typedef Error super;
				ErrorPrototype(ErrorType type);
				virtual Object* getPrototype(Engine& engine) const;
};

class Engine {
	public:		Engine(GCHeap& heap, Frame* globals); // FIX : globals, in a Frame, this is confusing, but it is actually almost accurate... globally scoped code also declare undeletable 'vars', and although we could put them in a standard ScriptObject, there is some performance benefit to having them indexed just like a regular function.
				// FIX : the only problem with a standard ScriptFrame for globals is that it's got arguments. It shouldn't have arguments.
				GCHeap& getHeap() const { return gcHeap; }
				Frame* getGlobals() const { return globals; }
				void setupStandardLibrary(const String& librarySourceCode);

	protected:	GCHeap& gcHeap;
				Frame* const globals;
				
	// FIX : public? v
	public:		ScriptObject objectPrototype; // FIX : make private? put where really?
				FunctionPrototypeFunction functionPrototype;
				WrapperObject booleanPrototype; // FIX : maybe an array with these and loop to create etc?
				WrapperObject numberPrototype; // FIX : maybe an array with these and loop to create etc?
				WrapperObject stringPrototype;
				Error* errorPrototypes[ERROR_TYPE_COUNT];
				Array arrayPrototype;
				EvalFunction evalFunction;
				EvalFunction evalDirectFunction;
				ScriptFunction* toNumberFunction;
				ScriptFunction* toStringFunction;
};

class Frame : public Object {
	public:		typedef Object super;
				virtual const CodeDefinition* getFunctionDef() const = 0;
				virtual void declareVar(Engine& engine, const String* name, const Value& initValue) = 0;
				Value* getIndexedPointer() const { return indexedPointer; }
				
	protected:	Value* indexedPointer; // Pointer is offset so that negative indexes addresses local variables and positive indexes addresses arguments.
				Frame* parentFrame;
};

class ScriptFrame : public LazyObject<Frame> {
	friend class Arguments;
	
	public:		typedef LazyObject<Frame> super;

				ScriptFrame(ScriptFunction* function, Int32 argc, const Value* argv);
				virtual const CodeDefinition* getFunctionDef() const { return function->getDefinition(); }
				void updatedCode();
				virtual Flags getProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual void declareVar(Engine& engine, const String* name, const Value& initValue);

	protected:	virtual void constructCompleteObject(Engine& engine) const;
				ScriptFunction* const function;
				const UInt32 passedArgumentsCount;
				Object* const environment;
				QuickArray<Value> indexed; // Includes variables and arguments.
				
				virtual void gcMark(GCHeap& heap) const {
					heap.mark(function);
					heap.markViaReferences(indexed.begin(), indexed.end());
					heap.mark(environment);
					super::gcMark(heap);
				}

};

// FIX : if CStackEntry included a codeDef pointer, we wouldn't need this class at all, really worth contemplating. It is really a STUPID class, passing all virtuals on to parent, just setting a different codeDef really.
/*
	This is the direct frame of an eval(), it only bridges all virtuals to the parent ScriptFrame, but has a different
	codeDef pointer (for constants etc).
	
	FIX : I think we can use a ScriptFrame now for simplicity, if we just empty codeDef->nameIndexes first.
	Well, it shouldn't have arguments either. Like the global frame.
*/
class EvalFrame : public Frame {
	public:		typedef Frame super;

				EvalFrame(const CodeDefinition* evalCodeDef, Frame* parentFrame);
				virtual const CodeDefinition* getFunctionDef() const { return evalCodeDef; } // FIX : recursion
				virtual Flags getProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual void declareVar(Engine& engine, const String* name, const Value& initValue) { parentFrame->declareVar(engine, name, initValue); }

	protected:	const CodeDefinition* const evalCodeDef;
				Frame* const parentFrame;
				QuickArray<Value> indexed; // For temporaries (safe-kept) only. FIX : now this array is the size of varNames.size(), but I think the compiler should skip making nameIndexes when compiling for eval so this array would only be temporaries only. Then we could probably just use a ScriptFrame instead of EvalFrame. And enterEval would be more similar to calling a function also.

				virtual void gcMark(GCHeap& heap) const {
					heap.mark(evalCodeDef);
					heap.mark(parentFrame);
					heap.markViaReferences(indexed.begin(), indexed.end());
					super::gcMark(heap);
				}
};

/*
	The CatchFrame is necessary because within a catch block there is a single extra variable that is local to the
	catch block. It is created dynamically and pushed on the call stack when entering a catch block and removed at any
	exit point.
*/
class CatchFrame : public Frame {
	public:		typedef Frame super;

				CatchFrame(Frame* parentFrame, const String* exceptionName, const Value& exceptionValue);
				virtual const CodeDefinition* getFunctionDef() const { return parentFrame->getFunctionDef(); } // FIX : recursion
				virtual Flags getProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual void declareVar(Engine& engine, const String* name, const Value& initValue) { parentFrame->declareVar(engine, name, initValue); }

	protected:	Frame* parentFrame;
				const String* exceptionName;
				Value exceptionValue;

				virtual void gcMark(GCHeap& heap) const {
					heap.mark(parentFrame);
					heap.mark(exceptionName);
					exceptionValue.gcMark(heap);
					super::gcMark(heap);
				}
};

class WithFrame : public Frame {
	public:		typedef Frame super;
				WithFrame(Frame* parentFrame, Object* withObject);
				virtual const CodeDefinition* getFunctionDef() const;
				virtual Flags getProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual void declareVar(Engine& engine, const String* name, const Value& initValue) { parentFrame->declareVar(engine, name, initValue); }

	protected:	Frame* parentFrame;
				Object* withObject;
				virtual void gcMark(GCHeap& heap) const {
					heap.mark(parentFrame);
					heap.mark(withObject);
					super::gcMark(heap);
				}
};

class Arguments : public LazyObject<Object> {
	public:		typedef LazyObject<Object> super;

				Arguments(ScriptFrame* frame, UInt32 argumentsCount);
				virtual const String* getClassName() const;
				virtual const String* toString(GCHeap& heap) const;
				virtual Object* getPrototype(Engine& engine) const;
				virtual Flags getProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Engine& engine) const;

	protected:	virtual void constructCompleteObject(Engine& engine) const;
				Value* findProperty(const Value& key) const;
				ScriptFrame* const frame;
				UInt32 const argumentsCount;
				QuickArray<Byte> deletedArguments;

				virtual void gcMark(GCHeap& heap) const {
					heap.mark(frame);
					super::gcMark(heap);
				}
};

/* 
FIX: idea, have a simpler and therefore safer (no mem-alloc) Exception type and convert to full value string on demand instead?
class Exception : public std::exception {
	public:		virtual Value createValue(Engine& engine) = 0;
};
*/

// FIX : test with very short max sz length
class Exception : public std::exception { // FIX : runtime_error instead?
	public:		Exception(GCHeap& heap, const Value& value) throw() : value(value) { // FIX : not inline
					strcpy(sz, "Unknown exception (failed conversion to string)");
					const String* src = value.toString(heap);
					const UInt32 l = (sizeof (sz) / sizeof (*sz) - 1);
					*std::copy(src->begin(), (src->size() < l ? src->end() : src->begin() + l), sz) = 0;
				}
				const Value& getValue() const { return value; }
				virtual const char* what() const throw() { return sz; }													// what() is for compatibility only and has a length limitation. Use getValue() and convert to string to get the full length unicode exception string.
				virtual ~Exception() throw() { }																		// (GCC requires explicit destructor with one that has throw().)

	protected:	Value value;
	protected:	char sz[512]; // We use a fixed length buffer to store a char string version of the exception value. We do not use std::string or String to avoid potential throws in the constructor.
};

// FIX : Processor -> Interpreter
class Processor : public GCItem {
	public:		typedef GCItem super;

				// FIX : this could be a struct used by a member Processor too instead of separate ip, frame etc...
				struct CStackEntry {
					const CodeWord* returnIP;
					Frame* frame; // doesn't change during function execution
					Object* thisObject; // doesn't change during function execution
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

	public:		enum Opcode {	// FIX : sort
					INVALID_OP = -1 // FIX : why?!
					, POP_OP										// stack: value ->
					, REPUSH_OP										// stack: value -> value, value
					, REPUSH_2_OP									// stack: value_1, value_2 -> value_1, value_2, value_1, value_2	// used for duplicating property reference with assignment operators like += etc
					, POST_SHUFFLE_OP								// stack: object, name, value -> value, object, name, value			// used for special post inc/dec logic on properties (see code)
					, CONST_OP										// operand: const_index, stack: -> constant
					, READ_INDEXED_OP								// operand: local_index, stack: -> value
					, WRITE_INDEXED_OP								// operand: local_index, stack: value -> value
					, READ_NAMED_OP									// operand: const_index (name), stack: -> value
					, WRITE_NAMED_OP								// operand: const_index (name), stack: value -> value
					, GET_PROPERTY_OP								// stack: object, name -> value
					, SET_PROPERTY_OP								// stack: object, name, value -> value
					, ADD_PROPERTY_OP								// operand: const_index (name), stack: object, value -> object
					, TO_NUMBER_OP									// stack: value -> primitive_value (number preferred)
					, TO_STRING_OP									// stack: value -> primitive_value (string preferred)
					, PRE_EQ_OP										// stack: value, value -> value, value				// if first *or* second is object (and not both) and the other is string or number, convert object to primitive value
					, INC_OP, DEC_OP								// stack: value -> number
					, ADD_OP, SUB_OP, MUL_OP, DIV_OP, MOD_OP		// stack: left_value, right_value -> result_value
					, OR_OP, XOR_OP, AND_OP							// stack: left_value, right_value -> result_value
					, SHL_OP, SHR_OP, USHR_OP						// stack: left_value, right_value -> result_value
					, PLUS_OP, MINUS_OP, INV_OP, NOT_OP				// stack: value -> value
					, X_EQ_OP, X_NEQ_OP, EQ_OP, NEQ_OP				// stack: value -> boolean
					, LT_OP, LEQ_OP, GT_OP, GEQ_OP					// stack: value -> boolean
					, JMP_OP, JSR_OP								// operand: offset
					, JT_OP, JF_OP									// operand: offset, stack: boolean ->
					, CALL_OP										// operand: n, stack: function, n * args -> return_value
					, CALL_METHOD_OP								// operand: n, stack: object, name, n * args -> return_value
					, CALL_EVAL_OP									// operand: n, stack: object, name, n * args -> return_value // special eval call is required because of need to differentiate direct or indirect call to eval
					, NEW_OP										// operand: n, stack: constructor object, n * args -> new_object, return_value
					, NEW_RESULT_OP									// stack: new_object, return_value -> new_this value
					, NEW_OBJECT_OP									// stack: -> object
					, NEW_ARRAY_OP									// stack: -> array_object
					, RETURN_OP
					, RETURN_TO_HOST_OP
					, THIS_OP, VOID_OP, FRAME_OP					// stack: -> value
					, DELETE_OP										// stack: object, name -> boolean
					, GEN_FUNC_OP									// operand: const_index (function), stack: -> function
					, DECLARE_OP									// operand: const_index (name), stack: function|undefined ->		// only used by eval code to declare vars and functions (non-eval code sets all this up on entry and through WRITE_INDEXED_OP)
					, THROW_OP										// stack: value ->
					, TRY_OP										// operand: catch_offset
					, TRIED_OP
					, CATCH_FRAME_OP								// operand: const_index for exception variable name
					, PUSH_FRAME_OP									// stack: object ->
					, POP_FRAME_OP
					, IN_OP											// stack: string, object -> boolean
					, INSTANCE_OF_OP								// stack: object, function -> boolean
					, TYPEOF_OP										// stack: value -> string
					, TYPEOF_NAMED_OP								// operand: const_index (name), stack: -> string
					, GET_ENUMERATOR_OP								// stack: object -> enumerator
					, NEXT_PROPERTY_OP								// operand: exit_loop_offset, stack: enumerator -> string
					, OP_COUNT
				};

	public:		Processor(Engine& engine, CStackEntry* cstackStart, CStackEntry* cstackEnd, Value* vstackStart, Value* vstackEnd);
				void chain(const CodeWord* stub);
				void chain(Function* completion);
				void chain(ScriptFunction* func, Int32 argc, const Value* argv, Object* thisObject);
				void chain(const CodeWord* ip, Frame* frame, Object* thisObject); // FIX : turn into enter global code or what?
				void enterEval(CodeDefinition* codeDef, bool directCall);
	// FIX :			void call(ScriptFunction* func, Int32 argc, const Value* argv, Object* thisObject);
				void resetTimeOut(Int32 clockCycles);
				bool run();
				Value getResult() const { return result; }
				void error(ErrorType errorType, const String* message = 0);
				
	protected:	void push(const Value& v);
				void pop(Int32 count = 1);
				void poppush(const Value& v);
				Int32 operand();
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
				Value result;

				virtual void gcMark(GCHeap& heap) const {
					heap.markViaReferences(vstackStart, vsp + 1);
					for (CStackEntry* c = cstackStart; c != csp + 1; ++c) {
						heap.mark(c->frame);
						heap.mark(c->thisObject);
					}
					result.gcMark(heap);
					super::gcMark(heap);
				}

				// Copies from call stack. Will be gc marked from there.
				
				Frame* frame;
				const Value* constants;
				Value* indexed;
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
				const Char* compile(const Char* b, const Char* e);
				const Char* compileFunction(const Char* b, const Char* e, const String* functionName, const String* selfName = 0); // FIX : messy, why do we have compilerFor if we separate this anyhow? Maybe subclass Compiler instead?
				void compile(const String& source);
				const Char* getStopPoint() const { return p; }

	protected:	struct ExpressionResult;
				struct Scope;

				void error(ErrorType errorType, const char* message);
				void emit(Processor::Opcode op);
				void emitWithInt32(Processor::Opcode op, Int32 i);
				int addConstant(const Value& constant);
				void emitWithConstant(Processor::Opcode op, const Value& constant);
				Int32 emitForwardBranch(Processor::Opcode op);
				void completeForwardBranch(Int32 point);
				Int32 markBackwardBranch();
				void emitBackwardBranch(Processor::Opcode op, Int32 point);

				ExpressionResult safeKeep(const ExpressionResult& xr); // Used to store result into a new temporary local (e.g.used by return statement if there are finally blocks to call first)
				void returnSafeKept(const ExpressionResult& xr, bool doReset); // if doReset == true then reset the local variable used for safe-keeping (to UNDEFINED)
				ExpressionResult makeRValue(const ExpressionResult& xr, bool toPrimitive = false); // Creates an r-value (i.e. pushed on value stack) out of a result.
				ExpressionResult makeAssignment(const ExpressionResult& xr); // Creates an assignment out of an l-value (throws if xr is not a valid l-value)
				ExpressionResult discard(const ExpressionResult& xr); // Discards result (e.g. popping it if it is on stack)

				void completeContinues(const Scope* fromScope, const Scope* untilScope);
				void completeBreaks(const Scope* scope);

				ExpressionResult operand(const OperatorInfo& op);
				ExpressionResult functionCall(Processor::Opcode op);
				Value stringOrNumberConstant();
				ExpressionResult arrayInitialiser();
				ExpressionResult objectInitialiser();
				bool operate(ExpressionResult& xr, Precedence precedence, int opCount, const OperatorInfo* opList);
				ExpressionResult expression(Precedence precedence);
				ExpressionResult rvalueExpression(Precedence precedence = LOWEST_PREC);
				void functionDefinition(const String* functionName, const String* selfName = 0); // selfName is only for named function expressions
				bool optionalExpression(ExpressionResult& xr, Precedence precedence);
				ExpressionResult declareIdentifier(const String* name);
				ExpressionResult varDeclaration();
				void flushXR(ExpressionResult& evalXR);
				void varDeclarations();
				void forInStatement(Scope* emptyLabelScope, Scope* scopeLabelsEnd, const QuickVector<CodeWord>& iterCode
						, const ExpressionResult& iterXR, ExpressionResult& evalXR);
				void forStatement(Scope* emptyLabelScope, Scope* scopeLabelsEnd, ExpressionResult& evalXR);
				void functionStatement();
				void withStatement(Scope* currentScope, ExpressionResult& evalXR);
				void statement(Scope* firstScope, Scope* scopeLabelsEnd, ExpressionResult& evalXR);
				void statementList(Scope* firstScope, ExpressionResult& evalXR);
				bool token(const char* t, bool eatLeadingWhite); // FIX : not all of these, and name!
				bool eof() const { return p == e; }
				void semicolon();
				void expectToken(const char* t, bool eatLeadingWhite);
				void white();
				void token();
				Char* unescape(Char* buffer, const Char* e);
				bool isLeadingIdentifierChar(Char c);
				const Char* identifier(bool required, bool acceptReservedKeywords);

				GCHeap& heap; // FIX : necessary?
				CodeDefinition* const codeDef;
				const Target compilingFor;
				std::map<Value, Int32> definedConstants; // FIX : drop map dependency, increases code size
				QuickVector<CodeWord> funcInitCode; // function declarations, inserted at top of function when finalizing
				const Char* p;
				const Char* e;
				QuickVector<CodeWord>* outputCode; // FIX : these three are bad, they are set and reset in blocks, which is what we normally use the C++ stack for really, e.g. create a new local compiler with the change
				bool acceptInOperator;
				int withScopeDepth; // FIX : if we have a Context object instead as "this" we could create a new one with a simple flag for this instead of yucky counter

				virtual void gcMark(GCHeap& heap) const {
					heap.mark(codeDef);
					for (std::map<Value, Int32>::const_iterator it = definedConstants.begin(); it != definedConstants.end(); ++it) {
						it->first.gcMark(heap);
					}
					super::gcMark(heap);
				}
};

#endif
