#if !defined(NuXJScript_h)
#define NuXJScript_h 1

#include "assert.h"
#include <string>
#include <cstring>

#ifndef NDEBUG
	#define VERIFY_STACK_LOGIC 1
#else
	#define VERIFY_STACK_LOGIC 0
#endif

typedef short Int16;
typedef unsigned short UInt16;
typedef int Int32;
typedef unsigned int UInt32;
typedef unsigned char Byte;
typedef UInt16 Char;
typedef Int32 CodeWord;

class GCList;
class Heap;

/*
FIX : not correct any more!

	GCItems normally belong to heaps*. You put them there by calling Heap::govern() or Heap::include(). Every GCItem
	class is responsible for "marking" all GCItems it has references to. If it fails to do this, items that are still in
	use may get garbage collected (= programming error).
 
	GCItems should never refer to items in a different heap than its own, unless the items referred to in turn do not
	refer to items in the first items heap. In other words, circular references between different heaps are not allowed.
 
	In rare circumstances it is ok to not place a GCItem into a heap at all. The item will in this case never be
	a candidate for garbage collection, but it will also never mark any of its references. In other words, this item
	must be a terminal leaf that has no further unique references.
 
	When a GCItem is destructed (regardless if it is from automatic garbage collection or not) it is removed from the
	heap it belongs to. This enables heaps to contain a mixture of automatically garbage collected and manually
	memory managed items.
 
	* They actually belong to particular lists inside heaps.
*/
class GCItem {
	friend class GCList;
	friend class Heap;

	public:		static void* operator new(size_t n, Heap& heap);
				static void operator delete(void* ptr);
				static void operator delete(void* ptr, Heap& heap);
	
	protected:	GCItem() throw();
				GCItem(GCList& gcList) throw();
				GCItem(const GCItem& copy) throw();
				GCItem& operator=(const GCItem& copy) throw();
				virtual void gcMark(Heap& heap) const;
				virtual ~GCItem();
	
				GCList* _gcList;
				GCItem* _gcPrev;
				GCItem* _gcNext;

	private:	static void* operator new[](size_t, Heap* heap); // N/A
				static void operator delete[](void*); // N/A
};

// These are double-linked circular lists with a dummy node first to minimize code complexity.
class GCList : public GCItem {
	friend class GCItem;
	friend class Heap;
	
	public:		GCList(Heap& heap);
				Heap& getHeap() const throw() { return heap; }
				bool owns(GCItem* item) throw() { return item->_gcList == this; } // FIX : drop?
				void claim(GCItem* item) throw();
				void relinquish(GCItem* item) throw();
				UInt32 deleteAll() throw();
				~GCList();

	protected:	Heap& heap;
				UInt32 count;
};

const int MAX_POOLED_SIZE = 1024;
const int POOL_SIZE_GRANULARITY = 16;
class Heap {
	public:		Heap();
	
				GCList& managed() throw() { return *currentList; }
				GCList& roots() throw() { return rootList; }
				
				void* allocate(size_t size);
				void* allocateArray(size_t size);
				void deallocate(void *ptr);
				void deallocateArray(void *ptr);
				void purgePooled(); // Release pooled blocks. Suggestion: call before each gc to only hold on to as much memory as we required since last gc.
				size_t size() const;
				UInt32 count() const;
				UInt32 gc();
				void mark(const GCItem* item) {
					if (item != 0 && item->_gcList == currentList) {
						newList->claim(const_cast<GCItem*>(item));
					}
				}
				template<class T> void markViaPointers(T begin, T end) { for (T it = begin; it != end; ++it) mark(*it); } // FIX : is this good with two of them? one is for Value etc (they used to have markIn(heap), but now have gcMark() like GCItem)
				template<class T> void markViaReferences(T begin, T end) { for (T it = begin; it != end; ++it) it->gcMark(*this); }
				// FIX : used extremely rarely, keep?
				bool belongs(const GCItem* item) { return (item != 0 && (item->_gcList == currentList || item->_gcList == newList || item->_gcList == &rootList)); }
				virtual ~Heap();
	
	protected:	static int calcPoolIndex(size_t size);
				void* pooled[MAX_POOLED_SIZE / POOL_SIZE_GRANULARITY];
				size_t allocated;
				GCList managedListA;
				GCList managedListB;
				GCList rootList;
				GCList* currentList;
				GCList* newList;

	private:	Heap(const Heap& that); // N/A
				Heap& operator=(const Heap& that); // N/A
};

// Since these can be called during static initialization we allow heap == 0 to use standard new/delete.
inline void* operator new(size_t size, Heap* optionalHeap) { return (optionalHeap != 0 ? optionalHeap->allocate(size) : ::operator new(size)); }
inline void* operator new[](size_t size, Heap* optionalHeap) { return (optionalHeap != 0 ? optionalHeap->allocateArray(size) : ::operator new[](size)); }
inline void operator delete(void* ptr, Heap* optionalHeap) { return (optionalHeap != 0 ? optionalHeap->deallocate(ptr) : ::operator delete(ptr)); }
inline void operator delete[](void* ptr, Heap* optionalHeap) { return (optionalHeap != 0 ? optionalHeap->deallocateArray(ptr) : ::operator delete[](ptr)); }

// FIX : name
const UInt32 STANDARD_BUILT_IN_COUNT = 8;
template<typename T, UInt32 BUILT_IN_COUNT = STANDARD_BUILT_IN_COUNT> class QuickArray {
	public:		explicit QuickArray(UInt32 n, Heap* optionalHeap)
						: associatedHeap(optionalHeap), n(n), p(n <= BUILT_IN_COUNT ? a : new(optionalHeap) T[n]) { }
				explicit QuickArray(const QuickArray& copy) : n(0), associatedHeap(copy.associatedHeap), p(a) {
					(*this) = copy;
				}
				
				Heap* getAssociatedHeap() const { return associatedHeap; }
				
				void swap(QuickArray& that) {
					if (this != &that) {
						std::swap(associatedHeap, that.associatedHeap);
						std::swap_ranges(a + 0, a + BUILT_IN_COUNT, that.a); // Note: if this one throws, we have messed up this object, but it is not completely broken (i.e. shouldn't crash destruction).
						std::swap(n, that.n);
						T* x = p;
						p = ((that.p == that.a) ? a : that.p);
						that.p = ((x == a) ? that.a : x);
					}
				}
				
				QuickArray& operator=(const QuickArray& copy) {
					if (this != &copy) {
						QuickArray<T, BUILT_IN_COUNT> na(copy.n, copy.associatedHeap);
						std::copy(copy.p, copy.p + copy.n, na.p);
						swap(na);
					}
					return *this;
				}
				
				void resize(UInt32 newSize) {
					if (newSize != n) {
						QuickArray<T, BUILT_IN_COUNT> na(newSize, associatedHeap);
						std::copy(p, p + std::min(n, newSize), na.p);
						swap(na);
					}
				}
				
				bool empty() const { return n == 0; }
				UInt32 size() const { return n; }
				operator T*() const { return p; }
				T* begin() const { return p; }
				T* end() const { return p + n; }

				virtual ~QuickArray() {
					if (p != a) {
						for (UInt32 i = n; i > 0; --i) {
							p[i - 1].~T();
						}
						::operator delete(p, associatedHeap);
					}
				}
				
	protected:	Heap* associatedHeap;
				T* p;
				T a[BUILT_IN_COUNT];
				UInt32 n;
};

template<typename T, UInt32 BUILT_IN_COUNT = STANDARD_BUILT_IN_COUNT> class QuickVector {
	public:		explicit QuickVector(Heap* optionalHeap) : qArray(BUILT_IN_COUNT, optionalHeap), count(0) { }
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
					assert(begin() <= p && p <= begin() + count);
					assert(b <= e);
					const UInt32 o = static_cast<UInt32>(p - begin());
					reserve(count + static_cast<UInt32>(e - b));
					p = begin() + o;
					std::copy(p, end(), p + (e - b)); // FIX : NOT RIGHT RIGHT? SHOULD COPY BACKWARDS (we don't use insert other than to append apparently)
					count += (e - b);
					std::copy(b, e, p);
				}
				void push(const T& t) { insert(end(), &t, &t + 1); }
                void pop(int n = 1) { resize(count - n); }
                T& back() { assert(count > 0); return qArray[count - 1]; }
                const T& back() const { assert(count > 0); return qArray[count - 1]; }
				void shrink() { qArray.resize(count); }

	protected:	QuickArray<T, BUILT_IN_COUNT> qArray;
				UInt32 count;
};

class String;
class Object;
class Function;
class Runtime;
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
	
				bool isUndefined() const { return type == UNDEFINED_TYPE; }
				bool isNull() const { return type == NULL_TYPE; }
				bool isBoolean() const { return type == BOOLEAN_TYPE; }
				bool isNumber() const { return type == NUMBER_TYPE; }
				bool isString() const { return type == STRING_TYPE; }
				bool isObject() const { return type == OBJECT_TYPE; } // FIX : should we expose the type enum instead?
				Object* asObject() const { return (type == OBJECT_TYPE ? var.object : 0); }
				Function* asFunction() const;
				const String* getString() const { assert(type == STRING_TYPE); return var.string; }
				Object* getObject() const { assert(type == OBJECT_TYPE); return var.object; }
				const String* typeOfString() const;
				bool toBool() const; // or toBoolean?
				Int32 toInt() const; // FIX : name toInt32
				bool toArrayIndex(UInt32& index) const;	// returns false if value is outside valid array index range (0..2^32-1)
				double toDouble() const;
				const String* toString(Heap& heap) const; // this toString() method does not run any script code, so it doesn't honor any user toString or valueOf implementations.
				Object* toObject(Runtime& rt, bool requestExtensible) const; // FIX : I don't like that we involved runtime here, not the end of the world but feels inconsistent, it is needed to create wrapper objects for numbers and booleans (and strings if makeExtensible)
				bool isStrictlyEqualTo(const Value& r) const;
				bool isEqualTo(const Value& r) const;
				bool isEqualTo(const String& s) const;
				bool isLessThan(const Value& r) const;
				bool isLessThanOrEqualTo(const Value& r) const;
				Value add(Heap& heap, const Value& r) const;
				void gcMark(Heap& heap) const;

				static const Value UNDEFINED;
				static const Value NUL;					// Sorry, NULL is a sensitive word, e.g. macro in Windows
				static const Value NOT_A_NUMBER;		// Sorry, NAN is sensitive, defined by some math.h
				static const Value INFINITE_NUMBER;		// Sorry, INFINITY is also sensitive

				// WARNING! Do NOT change order of elements in Type enum.
	protected:	enum Type {
					UNDEFINED_TYPE
					, NULL_TYPE
					, BOOLEAN_TYPE
					, NUMBER_TYPE
					, STRING_TYPE
					, OBJECT_TYPE
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
				static const String* TYPE_STRINGS[5];
	
	private:	explicit Value(const void* v); // N/A. Just to avoid unwanted casting of a pointer to bool.
};


typedef Byte Flags; // FIX : turn into enum
const Flags EXISTS_FLAG = 1; // FIX : turn these into enum, that's ok
const Flags READ_ONLY_FLAG = 2;
const Flags DONT_ENUM_FLAG = 4;
const Flags DONT_DELETE_FLAG = 8;
const Flags INDEX_TYPE_FLAG = 16;	// internal index type, only used as an optimization for faster name -> index lookup
const Flags STANDARD_FLAGS = 0;		// use with setOwnProperty()
const Flags NONEXISTENT = 0;		// use with getOwnProperty() to check for existence, e.g. getOwnProperty(o, k, v) != NONEXISTENT
const UInt32 TABLE_BUILT_IN_N = 3; // 1 << 3 == 8
class Table {
				/*
					The main reason why Bucket doesn't contain the Value class, but rather holds it's own Byte type and
					Variable union, is memory. This solution makes it possible to squeeze in key flags and a truncated
					16-bit hash (for even quicker value lookup) in the same space as a Value.
				*/
	public:		class Bucket {
					friend class Table;
		
					public:		Bucket() : key(0) { };
								Value getValue() const {
									assert(valueExists() && (flags & INDEX_TYPE_FLAG) == 0);
									return Value(static_cast<Value::Type>(type), var);
								}
								Int32 getIndexValue() const {
									assert(valueExists() && (flags & INDEX_TYPE_FLAG) != 0);
									return index;
								}
								Flags getFlags() const { return flags; }
								const String* getKey() const { assert(keyExists()); return key; }
								bool doEnumerate() const { return ((flags & DONT_ENUM_FLAG) == 0); }
								bool hasStandardFlags() const { return flags == EXISTS_FLAG; }
								bool valueExists() const { return (key != 0 && ((flags & EXISTS_FLAG) != 0)); }
		
					protected:	const String* key;
								Byte flags;
								Byte type;
								UInt16 hash16;
								union {
									Value::Variable var;
									Int32 index;
								};
								bool keyExists() const { return key != 0; }
				};
	
				Table(Heap* heap);
				Bucket* getFirst() const;											// Returns first bucket with an existing value or 0.
				Bucket* getNext(Bucket* bucket) const;								// Returns next bucket with an existing value or 0.
				Bucket* lookup(const String* key) const;							// Return bucket pointer for key or 0.
				Bucket* insert(const String* key);									// Insert key (if necessary) and return bucket pointer.
				bool update(Bucket* bucket, const Value& value, Flags flags = 0);	// Update value. Returns false if bucket is marked as read-only.
				bool erase(Bucket* bucket);											// Deletes bucket. Returns false if bucket is marked as dont-delete.
				UInt32 getLoadCount() const;										// Returns number of used hash table entries (not necessarily the same as the number of existing buckets!).
				void update(Bucket* bucket, const Int32 index);						// Update value as an index. Only used by name to index tables as an optimization.
				void gcMark(Heap& heap) const;
	
	protected:	static UInt32 calcMaxLoad(UInt32 bucketCount);
				Bucket* find(const String* key, UInt32 hash) const;
				UInt32 rebuild(UInt32 newN);
	
				QuickArray<Bucket, 1U << TABLE_BUILT_IN_N> buckets;
				UInt32 loadCount;													// Count of buckets with defined keys.
};

class Enumerator;
class Processor;
class ScriptObject;

// FIX : rename Object to something else, like AbstractObject, but AbstractObject sounds a bit like pure virtual abstract which isn't what it is, it's more like internal primitive object (which isn't good either cause primitive is something else in JS)
// InternalObject ?
class Object : public GCItem {
	public:		typedef GCItem super;
	
				virtual Function* asFunction();							// Default returns 0. (Functions returns this.)
				virtual const String* typeOfString() const;				// Default reutrns "object". (Strings and functions override.)
				virtual const String* getClassName() const;				// Default returns "Object".
				virtual const String* toString(Heap& heap) const; 		// this toString() method does not run any script code, so it doesn't honor any user toString or valueOf implementations.
				virtual Value getInternalValue(Heap& heap) const;		// Used by the standard library to retrieve internal value for wrappers (Number, String etc), source code for functions and parser function for RegExp. Default returns Value::UNDEFINED.
				virtual Object* getPrototype(Runtime& rt) const;		// Default returns the Object prototype.

				virtual Flags getOwnProperty(Runtime& rt, const Value& key, Value* v) const;					// Don't touch v if you return NONEXISTENT. Default returns NONEXISTENT.
				virtual bool setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags);		// Insert a new or update an existing property. Return false if not possible (e.g. read-only property already exists). Default returns false.
				virtual bool updateProperty(Runtime& rt, const Value& key, const Value& v);					// Update existing property. Return false if it doesn't exist or can't be updated (e.g. read-only property exists). Can be overriden for optimization. (Default implementation checks existence with hasOwnProperty() first.)
				virtual bool deleteProperty(Runtime& rt, const Value& key);									// Default returns false.
				virtual Enumerator* getPropertyEnumerator(Runtime& rt) const;								// Default returns an empty enumerator.

				// FIX : rename non-deep to Own and drop Deep, for consistency with JS lingo
				// FIX : or maybe drop them, we now check with getOwnProperty
				bool hasOwnProperty(Runtime& rt, const Value& key) const; // checks via getOwnProperty()
				bool isOwnPropertyEnumerable(Runtime& rt, const Value& key) const;
				Flags getPropertyDeep(Runtime& rt, const Value& key, Value* v) const; // searches prototype chain
				Value getPropertyDeep(Runtime& rt, const char* s) const; // helper function
				bool setPropertyDeep(Runtime& rt, const Value& key, const Value& v); // First tries updateProperty(). If that fails, checks prototype chain for read-only property with the same name and returns false if found. Otherwise attempts to insert a new property with setOwnProperty() and returns its outcome.

	protected:	Object() { }
				Object(GCList& gcList) : super(gcList) { }				// Automatically include on `list`.
};

class Enumerator : public Object {
	public:		typedef Object super;
				virtual Enumerator* asEnumerator() { return this; }
				virtual const String* nextPropertyName() = 0;

	protected:	Enumerator() { }
				Enumerator(GCList& gcList) : super(gcList) { }
};

/*
	Note that although the ECMAScript specification clearly separates string values from String objects, this
	implementation regards both as Objects for efficiency (e.g. for quickly retrieving properties like the character
	elements and "length").
	
	String is immutable and it will never contain any references to other objects. Therefore it is ok to not place them
	in a heap. (See GCItem for more info.)
	
	The ECMAScript String object on the other hand is represented by an extensible StringWrapper.
*/
class String : public Object, public QuickArray<Char> {
	public:		typedef Object super;
				typedef QuickArray<Char> QArray;

				static const String* newString(Heap& heap, const char* s);
				static const String* newHashedString(Heap& heap, const Char* b, const Char* e);
				static const String* concat(Heap& heap, const String& l, const String& r);
				static const String* fromInt(Heap& heap, Int32 i);
				static const String* fromDouble(Heap& heap, double d);

				String();													// Create empty string (allocated on standard C++ heap). Used for constant string generation on startup.
				explicit String(const char* s);								// Create string from constant 8-bit string (allocated on standard C++ heap). Will also generate hash and bloom code. Use for global string constants.
				String(const Char* b, const Char* e);						// Create string from begin to end pointers (allocated on standard C++ heap). Will also generate hash and bloom code. Used for constant string generation on startup.
				String(GCList& gcList);
				String(GCList& gcList, const Char* b, const Char* e);		// Create string from begin to end pointers.
				String(GCList& gcList, const String& l, const String& r);	// Create string by concatenating two strings.
				String(GCList& gcList, const char* s);						// Create string from constant 8-bit string. Will also generate hash and bloom code.
				String(GCList& gcList, const wchar_t* s);
				virtual const String* typeOfString() const;
				virtual const String* getClassName() const;
				virtual const String* toString(Heap& heap) const;
				virtual Object* getPrototype(Runtime& rt) const;
				virtual Flags getOwnProperty(Runtime& rt, const Value& key, Value* v) const;
				virtual Enumerator* getPropertyEnumerator(Runtime& rt) const;
				virtual Value getInternalValue(Heap& heap) const { return Value(this); }
				UInt32 calcHash() const;
				bool isEqualTo(const String& o) const;
				bool isLessThan(const String& o) const;
				std::wstring toStdString() const;
				UInt32 createBloomCode() const;

	protected:	mutable UInt32 bloom;
				mutable UInt32 hash;
};

class RangeEnumerator : public Enumerator {
	public:		typedef Enumerator super;
				RangeEnumerator(GCList& gcList, Int32 from, Int32 count);
				virtual const String* nextPropertyName();
				
	protected:	Heap& heap;
				const Int32 to;
				Int32 index;
};

class StringListEnumerator : public Enumerator {
	public:		typedef Enumerator super;
				StringListEnumerator(GCList& gcList, UInt32 initialCapacity);
				void add(const String* s);
				virtual const String* nextPropertyName();

	protected:	QuickVector<const String*> stringList;
				UInt32 nextIndex;

				virtual void gcMark(Heap& heap) const {
					heap.markViaPointers(stringList.begin(), stringList.end());
					super::gcMark(heap);
				}
};

class JoiningEnumerator : public Enumerator {
	public:		typedef Enumerator super;
				JoiningEnumerator(GCList& gcList, Runtime& rt, Enumerator* leftEnumerator, const Object* rightObject);
				virtual const String* nextPropertyName();

	protected:	Runtime& rt;
				Enumerator* const leftEnumerator;
				const Object* const rightObject;
				Enumerator* const rightEnumerator;

				virtual void gcMark(Heap& heap) const {
					heap.mark(leftEnumerator);
					heap.mark(rightObject);
					heap.mark(rightEnumerator);
					super::gcMark(heap);
				}
};

class ScriptObject : public Object {
	public:		typedef Object super;
				
				ScriptObject(GCList& gcList, Object* prototype);
				virtual Object* getPrototype(Runtime& rt) const;
				virtual Flags getOwnProperty(Runtime& rt, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags);
				virtual bool updateProperty(Runtime& rt, const Value& key, const Value& v);
				virtual bool deleteProperty(Runtime& rt, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Runtime& rt) const;
				Table& accessProperties();
				
	protected:	Table properties;
				Object* prototype;

				virtual void gcMark(Heap& heap) const {
					properties.gcMark(heap);
					heap.mark(prototype);
					super::gcMark(heap);
				}
};

// fix : good candidate for a lazy object
class WrapperObject : public ScriptObject {
	public:		typedef ScriptObject super;
				WrapperObject(GCList& gcList, const String* className, const Value& value, Object* prototype);
				virtual const String* getClassName() const;
				virtual Value getInternalValue(Heap& heap) const;
	protected:	const String* const className;
				const Value wrapped;
				virtual void gcMark(Heap& heap) const {
					heap.mark(className);
					wrapped.gcMark(heap);
					super::gcMark(heap);
				}
};

// FIX : a lot like a LazyScriptObject, would be good to merge somehow... Or at least, I want to have a safe compiler mechanism to make sure I always override *all* property accessor when doing things like this?!
class StringWrapper : public ScriptObject {
	public:		typedef ScriptObject super;
				StringWrapper(GCList& gcList, const String* string, Object* prototype);
				virtual const String* getClassName() const;
				virtual const String* toString(Heap& heap) const;
				virtual Value getInternalValue(Heap& heap) const;

				virtual Flags getOwnProperty(Runtime& rt, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Runtime& rt, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Runtime& rt) const;

	protected:	const String* const wrapped;
				virtual void gcMark(Heap& heap) const {
					heap.mark(wrapped);
					super::gcMark(heap);
				}
};

/*
	LazyScriptObjects become complete extensible "script objects" first when their properties are accessed. There are a few reasons why this is desirable:
	
	1) Performance: not having to setup a fully customizable object directly is benficial when it is rare that the user accesses properties or extends the object (but you still need an object reference). Example are Functions which are most often not treated as objects (i.e. constructors) by the user.
	2) Memory: until the user accesses or adds properties, LazyScriptObjects can be super tiny (vtable pointer + pointer to complete object + whatever internal fields are needed).
	3) Doesn't require an Runtime or even a Heap to be constructed. Although they are required to be placed on the heap since they contain a reference.
		// TODO: this object could actually own an auto_ptr to the complete object since that object should *never* be referenced to from elsewhere. Saving GC time. But they would still not be thread-safe to make static.

	This class is a template so this concept can be used with different super classes.
*/
template<class SUPER> class LazyScriptObject : public SUPER {
	public:		typedef SUPER super;
				LazyScriptObject(GCList& gcList) : super(gcList), completeObject(0) { }
				virtual Flags getOwnProperty(Runtime& rt, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Runtime& rt, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Runtime& rt) const;

	protected:	virtual void constructCompleteObject(Runtime& rt) const = 0;
				ScriptObject* getCompleteObject(Runtime& rt) const;
				mutable ScriptObject* completeObject;

				virtual void gcMark(Heap& heap) const {
					heap.mark(completeObject);
					super::gcMark(heap);
				}
};

class Array : public LazyScriptObject<Object> {
	public:		typedef LazyScriptObject<Object> super;
	
				Array(GCList& gcList);
				virtual const String* getClassName() const;
				virtual Object* getPrototype(Runtime& rt) const;
				// FIX : toString too?
				virtual Flags getOwnProperty(Runtime& rt, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags);
				virtual bool updateProperty(Runtime& rt, const Value& key, const Value& v);
				virtual bool deleteProperty(Runtime& rt, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Runtime& rt) const;

	protected:	virtual void constructCompleteObject(Runtime& rt) const;
				void sliceDenseVector(Runtime& rt, const Value& key);
				UInt32 length;
				QuickVector<Value> denseVector;
				virtual void gcMark(Heap& heap) const {
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
				virtual const String* toString(Heap& heap) const;
				virtual Value getInternalValue(Heap& heap) const;
				virtual Object* getPrototype(Runtime& rt) const;
				virtual Value call(Processor& processor, UInt32 argc, const Value* argv, Object* thisObject = 0) = 0;
				virtual Value construct(Processor& processor, UInt32 argc, const Value* argv, Object* thisObject);
				virtual bool hasInstance(Processor& processor, Object* object) const;

	protected:	Function() { }
				Function(GCList& gcList) : super(gcList) { }
};

class EvalFunction : public Function {
	public:		typedef Function super;
				EvalFunction(GCList& gcList, bool direct) : super(gcList), direct(direct), functionDefCache(&gcList.getHeap()) { }
				virtual Value call(Processor& processor, UInt32 argc, const Value* argv, Object* thisObject);
				// FIX : move down to Runtime so that we share with eval direct, *but* Runtime should be a GCItem then I think
	protected:	bool direct;
				// FIX : share cache (in Runtime) between evalFunction and evalDirectFunction, oh, and share with new function as well
				// FIX : or maybe just drop it? KISS you know!
				mutable Table functionDefCache; // eval() has a cache of source code strings -> functions (as they are neutral to which closure they run in). It is emptied on each big gc sweep.

				virtual void gcMark(Heap& heap) const {
					functionDefCache = Table(&heap); // Yeah we just empty everything on each gc sweep
					functionDefCache.gcMark(heap);
					super::gcMark(heap);
				}
};

// Separate from Code so that multiple functions can share the same constants (purely optimization).
class Constants : public GCItem, public QuickVector<Value> {
	public:		typedef GCItem super;
				Constants(GCList& gcList) : super(gcList), QuickVector<Value>(&gcList.getHeap()) { }
	protected:	virtual void gcMark(Heap& heap) const {
					heap.markViaReferences(begin(), end());
					super::gcMark(heap);
				}
};

// This is an Object so that we can store it as a constant and also in Table
class Code : public Object {
	friend class FunctionScope;
	friend class Compiler; // FIX : maybe not one day?
	
	public:		typedef Object super;
				
				Code(GCList& gcList, Constants* constants);
				bool lookupNameIndex(const String* name, Int32& index) const;
				UInt32 getVarsCount() const { return varNames.size(); }
				UInt32 getArgumentsCount() const { return argumentNames.size(); }
				const String* getLocalName(Int32 index) const { return (index < 0 ? varNames[static_cast<UInt32>(~index)] : argumentNames[static_cast<UInt32>(index)]); }
				const Value* getConstantsPointer() const { return constants->begin(); }
				const CodeWord* getCodeWords() const { return code.begin(); }
				UInt32 getCodeSize() const { return code.size(); }
				const String* getName() const { return name; }
				const String* getSource() const { return source; }
				UInt32 getMaxStackDepth() const { return maxStackDepth; }

	protected:	QuickArray<CodeWord> code;
				Constants* const constants;
				Table nameIndexes;							// < 0 : local variables, >= 0 : arguments, CATCH_PARAMETER == current catch parameter during compile-time only (don't use fast index binding)
				QuickVector<const String*> varNames;		// Notice that this list is reversed in relation to indexes in the "locals" array in Scope.
				QuickVector<const String*> argumentNames;
				const String* name;
				const String* selfName;
				const String* source;
				UInt32 bloomSet;							// Bloom bits of all local variables, arguments (+ self name and "arguments"). For faster scope resolution.
				UInt32 maxStackDepth;

				virtual void gcMark(Heap& heap) const {
					heap.mark(constants);
					nameIndexes.gcMark(heap);
					heap.markViaPointers(varNames.begin(), varNames.end());
					heap.markViaPointers(argumentNames.begin(), argumentNames.end());
					heap.mark(name);
					heap.mark(selfName);
					heap.mark(source);
					super::gcMark(heap);
				}

#if (VERIFY_STACK_LOGIC)
	public: QuickVector<int> stackDepths;
#endif
};

class Scope : public GCItem {
	public:		typedef GCItem super;
				Scope(GCList& gcList, Scope* parentScope);
				virtual Flags readVar(Runtime& rt, const String* name, Value* v) const;
				virtual void writeVar(Runtime& rt, const String* name, const Value& v);
				virtual bool deleteVar(Runtime& rt, const String* key);
				virtual void declareVar(Runtime& rt, const String* name, const Value& initValue, bool dontDelete);
				Value* getLocalsPointer() const { return localsPointer; }
				Scope* getParentScope() const { return parentScope; }
				void makeClosure();
				void leave() { if (deleteOnPop) { delete this; } }
	
	protected:	Scope* const parentScope;
				Value* localsPointer; // Pointer is offset so that negative indexes addresses local variables and positive indexes addresses arguments.
				bool deleteOnPop;

				virtual void gcMark(Heap& heap) const {
					heap.mark(parentScope);
					super::gcMark(heap);
				}
};

class ExtensibleFunction : public LazyScriptObject<Function> {
	public:		typedef LazyScriptObject<Function> super;
				ExtensibleFunction(GCList& gcList) : super(gcList) { }
	protected:	void createPrototypeObject(Runtime& rt, Object* forObject, bool dontEnumPrototype) const;
};

class FunctionPrototypeFunction : public ExtensibleFunction {
	public:		typedef ExtensibleFunction super;
				FunctionPrototypeFunction(GCList& gcList) : super(gcList) { }
				virtual Value call(Processor& processor, UInt32 argc, const Value* argv, Object* thisObject);
				virtual Object* getPrototype(Runtime& rt) const;
	protected:	virtual void constructCompleteObject(Runtime& rt) const;
};

class ScriptFunction : public ExtensibleFunction {
	friend class FunctionScope;
	
	public:		typedef ExtensibleFunction super;
				
				ScriptFunction(GCList& gcList, const Code* code, Scope* closure);
				virtual const String* toString(Heap& heap) const;
				virtual Value getInternalValue(Heap& heap) const;
				virtual Value call(Processor& processor, UInt32 argc, const Value* argv, Object* thisObject);
				const Code* getCode() const;
				
	protected:	virtual void constructCompleteObject(Runtime& rt) const;
				
				const Code* const code;
				Scope* const closure;
				
				virtual void gcMark(Heap& heap) const {
					heap.mark(code);
					heap.mark(closure);
					super::gcMark(heap);
				}
};

class SpecialConstructorFunction : public ExtensibleFunction {
	public:		typedef ExtensibleFunction super;
				SpecialConstructorFunction(GCList& gcList, Function* regularFunction, Function* constructorFunction);
				virtual Value call(Processor& processor, UInt32 argc, const Value* argv, Object* thisObject);
				virtual Value construct(Processor& processor, UInt32 argc, const Value* argv, Object* thisObject);
	
	protected:	virtual void constructCompleteObject(Runtime& rt) const;
				Function* const regularFunction;
				Function* const constructorFunction;

				virtual void gcMark(Heap& heap) const {
					heap.mark(regularFunction);
					heap.mark(constructorFunction);
					super::gcMark(heap);
				}
};

enum ErrorType {
	GENERIC_ERROR, EVAL_ERROR, RANGE_ERROR, REFERENCE_ERROR, SYNTAX_ERROR, TYPE_ERROR, URI_ERROR, ERROR_TYPE_COUNT
};

class Error : public LazyScriptObject<Object> {
	public:		typedef LazyScriptObject<Object> super;
				Error(GCList& heap, ErrorType type, const String* message = 0);
				virtual const String* getClassName() const;
				virtual const String* toString(Heap& heap) const;
				virtual Value getInternalValue(Heap& heap) const; // error type name
				virtual Object* getPrototype(Runtime& rt) const;
				virtual bool setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Runtime& rt, const Value& key);
				
	protected:	virtual void constructCompleteObject(Runtime& rt) const;
				void updateReflection(Runtime& rt);
				
				const ErrorType errorType;
				const String* name; // may get updated by script code
				const String* message; // may get updated by script code
				virtual void gcMark(Heap& heap) const {
					heap.mark(name);
					heap.mark(message);
					super::gcMark(heap);
				}
};

class ErrorPrototype : public Error {
	public:		typedef Error super;
				ErrorPrototype(GCList& gcList, ErrorType type);
				virtual Object* getPrototype(Runtime& rt) const;
};

// FIX : private class in runtime?
class GlobalScope : public Scope {
	public:		typedef Scope super;
				GlobalScope(GCList& gcList);
				virtual Flags readVar(Runtime& rt, const String* name, Value* v) const;
				virtual void writeVar(Runtime& rt, const String* name, const Value& v);
				virtual bool deleteVar(Runtime& rt, const String* key);
				virtual void declareVar(Runtime& rt, const String* name, const Value& initValue, bool dontDelete);
};

class WithScope : public Scope {
	public:		typedef Scope super;
				WithScope(GCList& gcList, Scope* parentScope, Object* withObject);
				virtual Flags readVar(Runtime& rt, const String* name, Value* v) const;
				virtual void writeVar(Runtime& rt, const String* name, const Value& v);
				virtual bool deleteVar(Runtime& rt, const String* key);

	protected:	Object* withObject;
				virtual void gcMark(Heap& heap) const {
					heap.mark(withObject);
					super::gcMark(heap);
				}
};

class FunctionScope : public Scope {
	friend class Arguments;
	
	public:		typedef Scope super;

				FunctionScope(GCList& gcList, ScriptFunction* function, UInt32 argc, const Value* argv);
				virtual Flags readVar(Runtime& rt, const String* name, Value* v) const;
				virtual void writeVar(Runtime& rt, const String* name, const Value& v);
				virtual bool deleteVar(Runtime& rt, const String* key);
				virtual void declareVar(Runtime& rt, const String* name, const Value& initValue, bool dontDelete);
				ScriptObject* getDynamicVars(Runtime& rt) const;

	protected:	ScriptFunction* const function;
				const UInt32 passedArgumentsCount;
				QuickArray<Value> locals; // Includes variables and arguments.
				mutable ScriptObject* dynamicVars;
				UInt32 bloomSet;
				virtual void gcMark(Heap& heap) const {
					heap.mark(function);
					heap.markViaReferences(locals.begin(), locals.end());
					heap.mark(dynamicVars);
					super::gcMark(heap);
				}
};

/*
	This is the direct scope of an eval(), it only bridges all virtuals to the parent FunctionScope, but has a different
	code pointer (for constants etc) and it declares deletable vars.
*/
class EvalScope : public Scope {
	public:		typedef Scope super;
				EvalScope(GCList& gcList, Scope* parentScope);
				virtual void declareVar(Runtime& rt, const String* name, const Value& initValue, bool dontDelete);
};

/*
	The CatchScope is necessary because within a catch block there is a single extra variable that is local to the
	catch block. It is created dynamically and pushed on the call stack when entering a catch block and removed at any
	exit point. Remember you can access the catch variable via evals, otherwise we could have just placed it among
	the locals variables.
*/
class CatchScope : public Scope {
	public:		typedef Scope super;

				CatchScope(GCList& gcList, Scope* parentScope, const String* exceptionName, const Value& exceptionValue);
				virtual Flags readVar(Runtime& rt, const String* name, Value* v) const;
				virtual void writeVar(Runtime& rt, const String* name, const Value& v);
				virtual bool deleteVar(Runtime& rt, const String* key);

	protected:	const String* exceptionName;
				Value exceptionValue;

				virtual void gcMark(Heap& heap) const {
					heap.mark(exceptionName);
					exceptionValue.gcMark(heap);
					super::gcMark(heap);
				}
};

const UInt32 DEFAULT_STACK_SIZE = 1024;
class Runtime : public GCItem {
	public:		typedef GCItem super;
				Runtime(GCList& gcList);
				void setGlobalObject(Object* globals);
				void setStackSize(UInt32 stackSize);
				UInt32 getStackSize() const;
				void setupStandardLibrary(const String& librarySourceCode);
				Heap& getHeap() const { return heap; }
				Object* getGlobals() const { assert(globals != 0); return globals; }
				GlobalScope* getGlobalScope() { return &globalScope; }
				Constants* getSharedConstants() { return &sharedConstants; }
				Value callSync(Function* function, UInt32 argc, const Value* argv, Object* thisObject = 0);

	protected:	Heap& heap;
				GlobalScope globalScope;
				Object* globals;
				Constants sharedConstants;
				UInt32 stackSize;
	
	// FIX : public? v
	public:		ScriptObject objectPrototype; // FIX : make private? put where really?
				FunctionPrototypeFunction functionPrototype;
				WrapperObject booleanPrototype; // FIX : maybe an array with these and loop to create etc?
				WrapperObject numberPrototype; // FIX : maybe an array with these and loop to create etc?
				WrapperObject stringPrototype;
				Array arrayPrototype;
				Error* errorPrototypes[ERROR_TYPE_COUNT];
				EvalFunction evalFunction;
				EvalFunction evalDirectFunction;
				Function* toNumberFunction;
				Function* toStringFunction;
				Function* createRegExpFunction;
				double unixEpochTimeDiff;
	
	public:		virtual void gcMark(Heap& heap) const {
					heap.mark(globals);
					heap.markViaPointers(errorPrototypes + 0, errorPrototypes + ERROR_TYPE_COUNT);
					heap.mark(toNumberFunction);
					heap.mark(toStringFunction);
					heap.mark(createRegExpFunction);
					super::gcMark(heap);
				}
};

class Arguments : public LazyScriptObject<Object> {
	public:		typedef LazyScriptObject<Object> super;

				Arguments(GCList& gcList, FunctionScope* scope, UInt32 argumentsCount);
				virtual const String* getClassName() const;
				virtual const String* toString(Heap& heap) const;
				virtual Object* getPrototype(Runtime& rt) const;
				virtual Flags getOwnProperty(Runtime& rt, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Runtime& rt, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Runtime& rt, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Runtime& rt) const;

	protected:	virtual void constructCompleteObject(Runtime& rt) const;
				Value* findProperty(const Value& key) const;
				FunctionScope* const scope;
				UInt32 const argumentsCount;
				QuickArray<Byte> deletedArguments;

				virtual void gcMark(Heap& heap) const {
					heap.mark(scope);
					super::gcMark(heap);
				}
};

/* 
FIX: idea, have a simpler and therefore safer (no mem-alloc) Exception type and convert to full value string on demand instead?
class Exception : public std::exception {
	public:		virtual Value createValue(Runtime& rt) = 0;
};
*/

// FIX : test with very short max sz length
class Exception : public std::exception { // FIX : runtime_error instead?
	public:		static void throwError(Heap& heap, ErrorType type, const String* message = 0) {
					throw Exception(heap, Value(new(heap) Error(heap.managed(), type, message)));
				}
	public:		static void throwError(Heap& heap, ErrorType type, const char* message) {
					throwError(heap, type, String::newString(heap, message));
				}
	public:		Exception(Heap& heap, const Value& value) throw() : value(value) { // FIX : not inline
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
	friend class ScriptFunction;
	
	public:		typedef GCItem super;
	
	public:		enum Opcode {	// FIX : sort
					INVALID_OP = -1 // FIX : why?!
					, CONST_OP										// operand: const_index, stack: -> constant
					, READ_LOCAL_OP									// operand: local_index, stack: -> value
					, WRITE_LOCAL_OP								// operand: local_index, stack: value -> value
					, WRITE_LOCAL_POP_OP							// operand: local_index, stack: value ->
					, READ_NAMED_OP									// operand: const_index (name), stack: -> value
					, WRITE_NAMED_OP								// operand: const_index (name), stack: value -> value
					, WRITE_NAMED_POP_OP							// operand: const_index (name), stack: value ->
					, GET_PROPERTY_OP								// stack: object, name -> value
					, SET_PROPERTY_OP								// stack: object, name, value -> value
					, SET_PROPERTY_POP_OP							// stack: object, name, value ->
					, ADD_PROPERTY_OP								// operand: const_index (name), stack: object, value -> object
					, OBJ_TO_NUMBER_OP								// stack: value -> primitive_value (number preferred)
					, OBJ_TO_STRING_OP								// stack: value -> primitive_value (string preferred)
					, PRE_EQ_OP										// stack: value, value -> value, value				// if first *or* second is object (and not both) and the other is string or number, convert object to primitive value
					, INC_OP, DEC_OP								// stack: value -> number
					, ADD_OP, SUB_OP, MUL_OP, DIV_OP, MOD_OP		// stack: left_value, right_value -> result_value
					, OR_OP, XOR_OP, AND_OP							// stack: left_value, right_value -> result_value
					, SHL_OP, SHR_OP, USHR_OP						// stack: left_value, right_value -> result_value
					, PLUS_OP, MINUS_OP, INV_OP, NOT_OP				// stack: value -> value
					, X_EQ_OP, X_NEQ_OP, EQ_OP, NEQ_OP				// stack: left_value, right_value -> boolean
					, LT_OP, LEQ_OP, GT_OP, GEQ_OP					// stack: left_value, right_value -> boolean
					, JMP_OP, JSR_OP								// operand: offset
					, JT_OP, JF_OP									// operand: offset, stack: boolean ->
					, JT_OR_POP_OP, JF_OR_POP_OP					// operand: offset, stack: boolean ->				// only pops when not jumping, for logical and/or instructions
					, POP_OP										// operand: count, stack: ... ->
					// FIX : push back is a really bad name, it's really pop back and then push sort of
					, PUSH_BACK_OP									// operand: count, stack: ..., back -> back
					, REPUSH_OP										// operand: offset, stack: ..., value, ... -> ..., value, ..., value
					, SWAP_OP
					, REPUSH_2_OP									// stack: value_1, value_2 -> value_1, value_2, value_1, value_2	// used for duplicating property reference with assignment operators like += etc
					, POST_SHUFFLE_OP								// stack: object, name, value -> value, object, name, value			// used for special post inc/dec logic on properties (see code)
					, CALL_OP										// operand: n, stack: function, n * args -> return_value
					, CALL_METHOD_OP								// operand: n, stack: object, name, n * args -> return_value
					, CALL_EVAL_OP									// operand: n, stack: function, n * args -> return_value			// special eval call is required because of need to differentiate direct or indirect call to eval
					, NEW_OP										// operand: n, stack: constructor object, n * args -> new_object, return_value
					, NEW_RESULT_OP									// stack: new_object, return_value -> new_this value
					, NEW_OBJECT_OP									// stack: -> object
					, NEW_ARRAY_OP									// stack: -> array_object
					, NEW_REG_EXP_OP								// stack: pattern_string, flags_string -> regexp_object
					// FIX : should we have an optional pop argument (push back) to RETURN_OP?
					, RETURN_OP
					// FIX : maybe we should also have a RETURN_VOID_OP?
					, THIS_OP, VOID_OP								// stack: -> value
					, DELETE_OP										// stack: object, name -> boolean
					, DELETE_NAMED_OP								// operand: const_index (name) -> boolean
					, GEN_FUNC_OP									// operand: const_index (function), stack: -> function
					, DECLARE_OP									// operand: const_index (name), stack: function|undefined ->		// only used by eval code to declare vars and functions (non-eval code sets all this up on entry and using WRITE_LOCAL_OP).
					, CATCH_SCOPE_OP								// operand: const_index for exception variable name, stack: value ->
					, WITH_SCOPE_OP									// stack: object ->
					, POP_FRAME_OP
					, TRY_OP										// operand: catch_offset
					, TRIED_OP
					, THROW_OP										// stack: value ->
					, IN_OP											// stack: string, object -> boolean
					, INSTANCE_OF_OP								// stack: object, function -> boolean
					, TYPEOF_OP										// stack: value -> string
					, TYPEOF_NAMED_OP								// operand: const_index (name), stack: -> string
					, GET_ENUMERATOR_OP								// stack: object -> enumerator
					, NEXT_PROPERTY_OP								// operand: exit_loop_offset, stack: enumerator -> string (unless end of loop)
					, OP_COUNT
				};
	
				struct OpcodeInfo {
					enum { TERMINAL = 1, POP_OPERAND = 2, POP_ON_BRANCH = 4, NO_POP_ON_BRANCH = 8 };
					Opcode opcode;
					const char* mnemonic;
					Int32 stackUse;
					Byte flags;
				};
	
				static CodeWord packInstruction(const Opcode opcode, const Int32 operand);
				static std::pair<Opcode, Int32> unpackInstruction(const CodeWord codeWord);
				static const OpcodeInfo& getOpcodeInfo(const Opcode opcode);

	public:		Processor(GCList& gcList, Runtime& rt);
				Runtime& getRuntime() const { return rt; }
				Heap& getHeap() const { return rt.getHeap(); }
				void execGlobal(const Code* code);
				void execEvalGlobal(const Code* code);
				void execEvalLocal(const Code* code);
				void execFunction(ScriptFunction* func, UInt32 argc, const Value* argv, Object* thisObject = 0);
				void resetTimeOut(Int32 clockCycles);
				bool run();
				Value getResult() const;

	protected:	void exec(const Code* code, Scope* scope, Object* thisObject);
				void error(ErrorType errorType, const String* message = 0);
				void push(const Value& v);
				void pop(Int32 count);
				void pop2push1(const Value& v);
				bool privateRun();
				void throwVirtualException(const Value& exception);
				Object* convertToObject(const Value& v, bool requestExtensible);
				void reset();
				void pushFrame(const Code* code, Scope* scope, Object* thisObject);
				void popFrame();
				void popCatcher();

				Runtime& rt;
	
				static const OpcodeInfo opcodeInfo[OP_COUNT];

public: // FIX : not public, just for some asserts now I think
				struct Frame : public GCItem {
					typedef GCItem super;
					Frame(GCList& gcList, const CodeWord* returnIP, const Code* code, Scope* scope, Object* thisObject, Frame* previousFrame)
							: super(gcList), returnIP(returnIP), code(code), scope(scope), thisObject(thisObject), previousFrame(previousFrame) { }
					const CodeWord* const returnIP;
					const Code* const code;
					Scope* const scope;
					Object* const thisObject;
					Frame* const previousFrame;
					virtual void gcMark(Heap& heap) const {
						heap.mark(code);
						heap.mark(scope);
						heap.mark(thisObject);
						heap.mark(previousFrame);
						super::gcMark(heap);
					}
				};
	
				struct Catcher : public GCItem {
					typedef GCItem super;
					Catcher(GCList& gcList, const CodeWord* ip, Value* sp, Frame* frame, Catcher* nextCatcher)
							: super(gcList), ip(ip), sp(sp), frame(frame), nextCatcher(nextCatcher) { }
					const CodeWord* const ip;
					Value* const sp;
					Frame* const frame;
					Catcher* const nextCatcher;
					virtual void gcMark(Heap& heap) const {
						heap.mark(frame);
						heap.mark(nextCatcher);
						super::gcMark(heap);
					}
				};

				Frame* currentFrame;
				Catcher* firstCatcher;
				Int32 cyclesLeft;
				const CodeWord* ip;
				Value* sp;
				QuickArray<Value> stack;

				virtual void gcMark(Heap& heap) const {
					heap.markViaReferences(stack.begin(), sp + 1);
					heap.mark(currentFrame);
					heap.mark(firstCatcher);
					super::gcMark(heap);
				}
};

enum Precedence {
	GROUP_PREC, PROPERTY_PREC, NEW_PREC, FUNCTION_CALL_PREC, POST_INC_DEC_PREC, PREFIX_PREC, MUL_DIV_PREC, ADD_SUB_PREC
	, SHIFT_PREC, RELATIONAL_PREC, EQUALITY_PREC, BIT_AND_PREC, BIT_XOR_PREC, BIT_OR_PREC, LOGICAL_AND_PREC
	, LOGICAL_OR_PREC, CONDITIONAL_PREC, ASSIGN_PREC, COMMA_PREC, LOWEST_PREC
};

const Int32 INVALID_CODE_OFFSET = -0x80000000;
const Int32 DEAD_CODE_STACK_DEPTH = -0x80000000;
struct OperatorInfo;
class Compiler : public GCItem {
	public:		typedef GCItem super;
				
				enum Target { FOR_GLOBAL, FOR_FUNCTION, FOR_EVAL };
				
				Compiler(GCList& gcList, Code* code, Target compileFor);
				const Char* compile(const Char* b, const Char* e);
				const Char* compileFunction(const Char* b, const Char* e, const String* functionName, const String* selfName = 0); // FIX : messy, why do we have compilerFor if we separate this anyhow? Maybe subclass Compiler instead?
				void compile(const String& source);
				const Char* getStopPoint() const { return p; }
				void getStopLineAndColumnNumbers(int& lineNumber, int& columnNumber) const;

	protected:	struct ExpressionResult {
					enum Type {
						NONE				// void
						, PUSHED			// arbitrary value on stack
						, PUSHED_PRIMITIVE	// primitive value on stack
						, CONSTANT			// constant (and primitive) value on stack
						, LOCAL				// arbitrary value in local variable
						, NAMED				// arbitrary value in named variable
						, PROPERTY			// arbitrary value in object property
						, SAFEKEPT			// further up the value stack (value = position relative to safe-kept-counter)
					};
					ExpressionResult() : t(NONE), v(Value::UNDEFINED) { }
					ExpressionResult(Type type, const Value& value = Value::UNDEFINED) : t(type), v(value) { }
					bool onStack() const { return t == PUSHED || t == PUSHED_PRIMITIVE; };
					Type t;
					Value v;
				};

				struct SemanticScope;
	
				struct CodeSection {
					CodeSection(Heap& heap, Int32 initialStackDepth)
							: code(&heap), lastEmitted(Processor::INVALID_OP), initialStackDepth(initialStackDepth)
							, stackDepth(initialStackDepth), maxStackDepth(initialStackDepth)
#if (VERIFY_STACK_LOGIC)
	, stackDepths(&heap)
#endif
							 { }
					void emit(Processor::Opcode opcode, Int32 operand);
					void insertSection(const CodeSection& section);
					bool inDeadCode() const { return stackDepth == DEAD_CODE_STACK_DEPTH; }
					QuickVector<CodeWord> code;
#if (VERIFY_STACK_LOGIC)
	QuickVector<Int32> stackDepths;
#endif
					Processor::Opcode lastEmitted;
					const Int32 initialStackDepth;
					Int32 stackDepth;
					Int32 maxStackDepth;
				};
	
				struct BranchPoint {
					BranchPoint(Int32 codeOffset = INVALID_CODE_OFFSET, Int32 stackDepth = DEAD_CODE_STACK_DEPTH)
							: codeOffset(codeOffset), stackDepth(stackDepth) { }
					bool isValid() const { return codeOffset != INVALID_CODE_OFFSET; };
					bool inDeadCode() const { return stackDepth == DEAD_CODE_STACK_DEPTH; };
					Int32 codeOffset;
					Int32 stackDepth;
				};

				void error(ErrorType errorType, const char* message);
				void emit(Processor::Opcode opcode, Int32 operand = 0);
				CodeSection* changeSection(CodeSection* newOutputSection);
				void insertSection(const CodeSection& section);
				UInt32 addConstant(const Value& constant);
				void emitWithConstant(Processor::Opcode opcode, const Value& constant);
				BranchPoint emitForwardBranch(Processor::Opcode opcode);
				void completeForwardBranch(const BranchPoint& point);
				void completeForwardBranches(const BranchPoint* begin, const BranchPoint* end);
				BranchPoint markBackwardBranch();
				void emitBackwardBranch(Processor::Opcode opcode, const BranchPoint& point);
	
				Processor::Opcode evalPopOpcode() const;
				ExpressionResult safeKeep(); // Used to store result into a new temporary local (e.g. used by return statement if there are finally blocks to call first). Good rule of thumb is that anything that needs to be preserved over statements need to be safe-kept (except the completion value of course).
				void returnSafeKept(const ExpressionResult& xr);
				ExpressionResult makeRValue(const ExpressionResult& xr, bool toPrimitive = false, bool preferString = false); // Creates an r-value (i.e. pushed on value stack) out of a result.
				ExpressionResult makeAssignment(const ExpressionResult& xr); // Creates an assignment out of an l-value (throws if xr is not a valid l-value)
				ExpressionResult discard(const ExpressionResult& xr); // Discards result (e.g. popping it if it is on stack)

				void completeIteratorContinues(const SemanticScope* fromScope, const SemanticScope* untilScope);
				void completeIteratorBreaks(const SemanticScope* fromScope, const SemanticScope* untilScope);
				void completeBreaks(const SemanticScope* ofScope);
				void functionReturn();

				ExpressionResult operand(const OperatorInfo& op);
				ExpressionResult functionCall(Processor::Opcode opcode);
				Value stringOrNumberConstant();
				ExpressionResult arrayInitialiser();
				ExpressionResult objectInitialiser();
				int parseOperator(Precedence precedence, int opCount, const OperatorInfo* opList);
				bool preOperate(ExpressionResult& xr, Precedence precedence);
				bool postOperate(ExpressionResult& xr, Precedence precedence);
				ExpressionResult expression(Precedence precedence);
				ExpressionResult rvalueExpression(Precedence precedence = LOWEST_PREC);
				void rvalueGroup();
				void functionDefinition(const String* functionName, const String* selfName = 0); // selfName is only for named function expressions
				bool optionalExpression(ExpressionResult& xr, Precedence precedence);
				ExpressionResult declareIdentifier(const String* name, bool func);
				ExpressionResult varDeclaration();
				void varDeclarations();
				void block(SemanticScope* scope);
				void forInStatement(SemanticScope* emptyLabelScope, SemanticScope* scopeLabelsEnd
						, const CodeSection& iterationSection, const ExpressionResult& iterationXR);
				void forStatement(SemanticScope* emptyLabelScope, SemanticScope* scopeLabelsEnd);
				void forOrForInStatement(SemanticScope* currentScope, SemanticScope* scopeLabelsEnd);
				void varStatement();
				void functionStatement();
				void breakStatement(SemanticScope* currentScope);
				void continueStatement(SemanticScope* currentScope);
				void ifStatement(SemanticScope* currentScope);
				void whileStatement(SemanticScope* currentScope, SemanticScope* scopeLabelsEnd);
				void doWhileStatement(SemanticScope* currentScope, SemanticScope* scopeLabelsEnd);
				void returnStatement(SemanticScope* currentScope);
				void withStatement(SemanticScope* currentScope);
				void throwStatement();
				void tryStatement(SemanticScope* currentScope);
				void switchStatement(SemanticScope* currentScope);
				void statement(SemanticScope* firstScope, SemanticScope* scopeLabelsEnd);
				void statementList(SemanticScope* firstScope);
				bool token(const char* t, bool eatLeadingWhite); // FIX : not all of these, and name!
				bool eof() const { return p == e; }
				void semicolon(bool acceptLF = true);
				void expectToken(const char* t, bool eatLeadingWhite);
				void white();
				bool whiteNoLF();
				void token();
				Char* unescape(Char* buffer, const Char* e);
				bool isLeadingIdentifierChar(Char c);
				const Char* identifier(bool required, bool acceptReservedKeywords);

				Heap& heap; // FIX : necessary?
				Code* const code;
				const Target compilingFor;
				CodeSection setupSection; // function declarations (and vars in eval code), inserted at top of function when finalizing
				CodeSection mainSection;
				const Char* b;
				const Char* p;
				const Char* e;
				CodeSection* currentSection;
				bool acceptInOperator;
				int withScopeCounter; // FIX : if we have a Context object instead as "this" we could create a new one with a simple flag for this instead of yucky counter

				virtual void gcMark(Heap& heap) const {
					heap.mark(code);
					super::gcMark(heap);
				}
};

#endif
