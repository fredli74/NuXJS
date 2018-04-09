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
typedef Int32 CodeWord;
typedef Byte Flags;

const UInt32 TABLE_BUILT_IN_N = 3; // 1 << 3 == 8
const UInt32 STANDARD_BUILT_IN_COUNT = 8;

class Heap {
	public:		virtual void* allocate(size_t size) = 0;
				virtual void* allocateArray(size_t size) = 0;
				virtual void deallocate(void *ptr) = 0;
				virtual void deallocateArray(void *ptr) = 0;
};

// Since these can be called during static initialization we allow heap == 0 to use standard new/delete.
inline void* operator new(size_t size, Heap* heap) { return (heap != 0 ? heap->allocate(size) : ::operator new(size)); }
inline void* operator new[](size_t size, Heap* heap) { return (heap != 0 ? heap->allocateArray(size) : ::operator new[](size)); }
inline void operator delete(void* ptr, Heap* heap) { return (heap != 0 ? heap->deallocate(ptr) : ::operator delete(ptr)); }
inline void operator delete[](void* ptr, Heap* heap) { return (heap != 0 ? heap->deallocateArray(ptr) : ::operator delete[](ptr)); }

// FIX : name
template<typename T, UInt32 BUILT_IN_COUNT = STANDARD_BUILT_IN_COUNT> class QuickArray {
	public:		explicit QuickArray(UInt32 n = BUILT_IN_COUNT, Heap* heap = 0)
						: heap(heap), n(n), p(n <= BUILT_IN_COUNT ? a : new (heap) T[n]) {	}
				explicit QuickArray(const QuickArray& copy) : n(0), heap(copy.heap), p(a) { (*this) = copy; }
				
				Heap* getHeap() const { return heap; }
				
				void swap(QuickArray& other) {
					if (this != &other) {
						std::swap(heap, other.heap);
						std::swap_ranges(a + 0, a + BUILT_IN_COUNT, other.a); // Note: if this one throws, we have messed up this object, but it is not completely broken (i.e. shouldn't crash destruction).
						std::swap(n, other.n);
						T* x = p;
						p = ((other.p == other.a) ? a : other.p);
						other.p = ((x == a) ? other.a : x);
					}
				}
				
				QuickArray& operator=(const QuickArray& copy) {
					if (this != &copy) {
						QuickArray<T, BUILT_IN_COUNT> na(copy.n, copy.heap);
						std::copy(copy.p, copy.p + copy.n, na.p);
						swap(na);
					}
					return *this;
				}
				
				void resize(UInt32 newSize) {
					QuickArray<T, BUILT_IN_COUNT> na(newSize, heap);
					std::copy(p, p + std::min(n, newSize), na.p);
					swap(na);
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
						::operator delete(p, heap);
					}
				}
				
	protected:	Heap* heap;
				T* p;
				T a[BUILT_IN_COUNT];
				UInt32 n;
};

template<typename T, UInt32 BUILT_IN_COUNT = STANDARD_BUILT_IN_COUNT> class QuickVector {
	public:		explicit QuickVector(Heap* heap = 0) : qArray(BUILT_IN_COUNT, heap), count(0) { }
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

	public:		static void* operator new(size_t n, Heap* heap);
				static void operator delete(void* ptr);
				static void operator delete(void* ptr, Heap* heap);
				
				GCItem() throw();
				GCItem(const GCItem& copy) throw();
				GCItem& operator=(const GCItem& copy) throw();
				GCHeap* gcGetHeap() const;
				virtual void gcMark(GCHeap& heap) const;
				virtual ~GCItem();
	
	protected:	GCList* _gcList;
				GCItem* _gcPrev;
				GCItem* _gcNext;

	private:	static void* operator new[](size_t, Heap&); // N/A
				static void operator delete[](void*); // N/A
};

// FIX : what should we do with items that are left in a GCList that destructs? I guess at least "release" them from the
// list (_gcList = 0). But in GCHeap, managed items ought to be deleted, right?
// These are double-linked circular lists with a dummy node first to minimize code complexity.
class GCList : public GCItem {
	public:		GCList(GCHeap& heap);
				GCHeap& getHeap() const;
				UInt32 size() const;
				void claim(GCItem* item) throw();
				void relinquish(GCItem* item) throw();

	protected:	GCHeap& heap;
				UInt32 count;
};

const int MAX_POOLED_SIZE = 1024;
const int POOL_SIZE_GRANULARITY = 16;
class GCHeap : public Heap {
	public:		typedef Heap super;
				GCHeap();
				template<class T> T* manage(T* item) throw() { currentList->claim(item); return item; }
				template<class T> T* protect(T* item) throw() { rootList.claim(item); return item; } // FIX : don't like this name, it's really also "manage", but don't auto-free
				virtual void* allocate(size_t size);
				virtual void deallocate(void *ptr);
				virtual void* allocateArray(size_t size);
				virtual void deallocateArray(void *ptr);
				UInt32 count() const;
				size_t size() const;
				UInt32 gc(); // FIX : gc(beyondItem) as in NuXScript was perhaps a very good idea, not sure we should have "protect" at all?
				void deletePooled(); // Release pooled blocks. Suggestion: call before each gc to only hold on to as much memory as we required since last gc.
				void mark(const GCItem* item);
				template<class T> void markViaPointers(T begin, T end) { for (T it = begin; it != end; ++it) mark(*it); } // FIX : is this good with two of them? one is for Value etc (they used to have markIn(heap), but now have gcMark() like GCItem)
				template<class T> void markViaReferences(T begin, T end) { for (T it = begin; it != end; ++it) it->gcMark(*this); }
				bool belongs(const GCItem* item) { return (item->_gcList == currentList || item->_gcList == newList || item->_gcList == &rootList); }
                virtual ~GCHeap();
    
	protected:	static int calcPoolIndex(size_t size);
				GCList managedListA;
				GCList managedListB;
				GCList rootList;
				GCList* currentList;
				GCList* newList;
				size_t allocated;
				void* pools[MAX_POOLED_SIZE / POOL_SIZE_GRANULARITY];
				
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
const Flags STANDARD_FLAGS = 0;		// use with setOwnProperty()
const Flags NONEXISTENT = 0;		// use with getOwnProperty() to check for existence, e.g. getOwnProperty(o, k, v) != NONEXISTENT

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

				virtual Flags getOwnProperty(Engine& engine, const Value& key, Value* v) const;  // don't touch v if you return NONEXISTENT
				virtual bool setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Engine& engine) const;

				// FIX : rename non-deep to Own and drop Deep, for consistency with JS lingo
				// FIX : or maybe drop them, we now check with getOwnProperty
				bool hasOwnProperty(Engine& engine, const Value& key) const; // checks via getOwnProperty()
				bool isOwnPropertyEnumerable(Engine& engine, const Value& key) const;
				Flags getPropertyDeep(Engine& engine, const Value& key, Value* v) const; // searches prototype chain
				bool setPropertyDeep(Engine& engine, const Value& key, const Value& v); // checks prototype chain for read-only
				Value getPropertyDeep(Engine& engine, const char* s) const; // helper function
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
				
				Table(Heap* heap = 0);
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
				String(const Char* b, const Char* e, Heap* heap = 0);
				String(const String& l, const String& r, Heap* heap = 0);
				explicit String(const std::wstring& s, Heap* heap = 0);
				explicit String(UInt32 length, Heap* heap = 0) : QArray(length, heap), hash(0) { } // FIX : dangerous, right, make protected?
// FIX : wasn't compatible with Digital Mars, so I added const char* constructor again, but do we care?
//				template<UInt32 N> explicit String(const char (&s)[N]) : QArray(N - 1), hash(0) { std::copy(s, s + N - 1, begin()); calcHash(); }
				explicit String(const char* s) : QArray(strlen(s)), hash(0) { std::copy(s, s + size(), begin()); calcHash(); }
				virtual const String* typeOfString() const;
				virtual const String* getClassName() const;
				virtual const String* toString(GCHeap& heap) const;
				virtual Object* getPrototype(Engine& engine) const;
				virtual Flags getOwnProperty(Engine& engine, const Value& key, Value* v) const;
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
				StringListEnumerator(Heap* heap, UInt32 initialCapacity);
				void add(const String* s);
				virtual const String* nextPropertyName();

	protected:	QuickVector<const String*> list;
				UInt32 nextIndex;

				virtual void gcMark(GCHeap& heap) const {
					heap.markViaPointers(list.begin(), list.end());
					super::gcMark(heap);
				}
};

class JoiningEnumerator : public Enumerator {
	public:		typedef Enumerator super;
				JoiningEnumerator(Engine& engine, Enumerator* leftEnumerator, const Object* rightObject);
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
				
				ScriptObject(Heap* heap, Object* prototype);
				virtual Object* getPrototype(Engine& engine) const;
				virtual Flags getOwnProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
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
				WrapperObject(Heap* heap, const String* className, const Value& value, Object* prototype);
				virtual const String* getClassName() const;
				virtual Value getInternalValue(GCHeap& heap) const;
	protected:	const String* const className;
				const Value wrapped;
				virtual void gcMark(GCHeap& heap) const {
					heap.mark(className);
					wrapped.gcMark(heap);
					super::gcMark(heap);
				}
};

// FIX : a lot like a LazyObject, would be good to merge somehow... Or at least, I want to have a safe compiler mechanism to make sure I always override *all* property accessor when doing things like this?!
class StringWrapper : public ScriptObject {
	public:		typedef ScriptObject super;
				StringWrapper(Heap* heap, const String* string, Object* prototype);
				virtual const String* getClassName() const;
				virtual const String* toString(GCHeap& heap) const;
				virtual Value getInternalValue(GCHeap& heap) const;

				virtual Flags getOwnProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Engine& engine) const;

	protected:	const String* const wrapped;
				virtual void gcMark(GCHeap& heap) const {
					heap.mark(wrapped);
					super::gcMark(heap);
				}
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
				virtual Flags getOwnProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
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

class Array : public LazyObject<Object> {
	public:		typedef LazyObject<Object> super;
	
				Array(Heap* heap);
				virtual const String* getClassName() const;
				virtual Object* getPrototype(Engine& engine) const;
				// FIX : toString too?
				virtual Flags getOwnProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
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
				virtual Value call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) = 0;
				virtual Value construct(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject);
				virtual bool hasInstance(Engine& engine, Processor& processor, Object* object) const;
};

class EvalFunction : public Function {
	public:		typedef Function super;
				EvalFunction(Heap* heap, bool direct) : direct(direct), functionDefCache(heap) { }
				virtual Value call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject);
				// FIX : move down to Engine so that we share with eval direct, *but* Engine should be a GCItem then I think
	protected:	bool direct;
				// FIX : share cache (in Engine) between evalFunction and evalDirectFunction, oh, and share with new function as well
				// FIX : or maybe just drop it? KISS you know!
				mutable Table functionDefCache; // eval() has a cache of source code strings -> functions (as they are neutral to which closure they run in). It is emptied on each big gc sweep.

				virtual void gcMark(GCHeap& heap) const {
					functionDefCache = Table(&heap); // Yeah we just empty everything on each gc sweep
					functionDefCache.gcMark(heap);
					super::gcMark(heap);
				}
};

class Frame;

class Constants : public GCItem, public QuickVector<Value> {
	public:		typedef GCItem super;
				Constants(Heap* heap) : QuickVector<Value>(heap) { }
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
				
				CodeDefinition(Heap* heap, Constants* constants);
				bool lookupNameIndex(const String* name, Int32& index) const;
				UInt32 getVarsCount() const { return varNames.size(); }
				UInt32 getArgumentsCount() const { return argumentNames.size(); }
				const String* getIndexedName(Int32 index) const { return (index < 0 ? varNames[static_cast<UInt32>(~index)] : argumentNames[static_cast<UInt32>(index)]); }
				const Value* getConstantsPointer() const { return constants->begin(); }
				const CodeWord* getCodePointer() const { return (code.empty() ? 0 : code.begin()); }
				UInt32 getCodeSize() const { return code.size(); }
				const String* getName() const { return name; }
				const String* getSource() const { return source; }

	protected:	QuickVector<CodeWord> code; // FIX : should be array really
				Constants* const constants;
				Table nameIndexes; // < 0 : local variables, >= 0 : arguments, type 'undefined' == current catch parameter during compile-time only (don't use fast index binding)
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
	protected:	void createPrototypeObject(Engine& engine, Object* forObject, bool dontEnumPrototype) const;
};

class FunctionPrototypeFunction : public ExtensibleFunction {
	public:		typedef ExtensibleFunction super;
				virtual Value call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject);
				virtual Object* getPrototype(Engine& engine) const;
	protected:	virtual void constructCompleteObject(Engine& engine) const;
};

// FIX : Frame -> Scope? More ES lingo
class Frame : public Object {
	public:		typedef Object super;
				Frame(Frame* parentFrame = 0) : parentFrame(parentFrame) { };
				virtual const CodeDefinition* getFunctionDef() const = 0;
				virtual void declareVar(Engine& engine, const String* name, const Value& initValue) = 0;
				Value* getIndexedPointer() const { return indexedPointer; }
				
	protected:	Value* indexedPointer; // Pointer is offset so that negative indexes addresses local variables and positive indexes addresses arguments.
				Frame* parentFrame;

				virtual void gcMark(GCHeap& heap) const {
					heap.mark(parentFrame);
					super::gcMark(heap);
				}
};

class ScriptFunction : public ExtensibleFunction {
	friend class ScriptFrame;
	
	public:		typedef ExtensibleFunction super;
				
				ScriptFunction(CodeDefinition* codeDef, Frame* environment);
				virtual const String* toString(GCHeap& heap) const;
				virtual Value getInternalValue(GCHeap& heap) const;
				virtual Value call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject);
				CodeDefinition* getDefinition() const;
				
	protected:	virtual void constructCompleteObject(Engine& engine) const;
				
				CodeDefinition* const codeDef;
				Frame* const environment;
				
				virtual void gcMark(GCHeap& heap) const {
					heap.mark(codeDef);
					heap.mark(environment);
					super::gcMark(heap);
				}
};

class SpecialConstructorFunction : public ExtensibleFunction {
	public:		typedef ExtensibleFunction super;

				SpecialConstructorFunction(Function* regularFunction, Function* constructorFunction);
				virtual Value call(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject);
				virtual Value construct(Engine& engine, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject);
	
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
	GENERIC_ERROR, EVAL_ERROR, RANGE_ERROR, REFERENCE_ERROR, SYNTAX_ERROR, TYPE_ERROR, URI_ERROR, ERROR_TYPE_COUNT
};

class Error : public LazyObject<Object> {
	public:		typedef LazyObject<Object> super;
				Error(ErrorType type, const String* message = 0);
				virtual const String* getClassName() const;
				virtual const String* toString(GCHeap& heap) const;
				virtual Value getInternalValue(GCHeap& heap) const; // error type name
				virtual Object* getPrototype(Engine& engine) const;
				virtual bool setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
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
				Function* toNumberFunction;
				Function* toStringFunction;
				Function* createRegExpFunction;
				double unixEpochTimeDiff;
};

class ScriptFrame : public LazyObject<Frame> {
	friend class Arguments;
	
	public:		typedef LazyObject<Frame> super;

				ScriptFrame(Heap* heap, ScriptFunction* function, UInt32 argc, const Value* argv);
				virtual const CodeDefinition* getFunctionDef() const { return function->getDefinition(); }
				void updatedCode();
				virtual Flags getOwnProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual Enumerator* getPropertyEnumerator(Engine& engine) const;
				virtual void declareVar(Engine& engine, const String* name, const Value& initValue);

	protected:	virtual void constructCompleteObject(Engine& engine) const;
				ScriptFunction* const function;
				const UInt32 passedArgumentsCount;
				QuickArray<Value> indexed; // Includes variables and arguments.
				virtual void gcMark(GCHeap& heap) const {
					heap.mark(function);
					heap.markViaReferences(indexed.begin(), indexed.end());
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

				EvalFrame(Heap* heap, Frame* parentFrame, const CodeDefinition* evalCodeDef);
				virtual const CodeDefinition* getFunctionDef() const { return evalCodeDef; } // FIX : recursion
				virtual Flags getOwnProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual void declareVar(Engine& engine, const String* name, const Value& initValue) { parentFrame->declareVar(engine, name, initValue); }

	protected:	const CodeDefinition* const evalCodeDef;
				QuickArray<Value> indexed; // For temporaries (safe-kept) only. FIX : now this array is the size of varNames.size(), but I think the compiler should skip making nameIndexes when compiling for eval so this array would only be temporaries only. Then we could probably just use a ScriptFrame instead of EvalFrame. And enterEval would be more similar to calling a function also.

				virtual void gcMark(GCHeap& heap) const {
					heap.mark(evalCodeDef);
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
				virtual Flags getOwnProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual void declareVar(Engine& engine, const String* name, const Value& initValue) { parentFrame->declareVar(engine, name, initValue); }

	protected:	const String* exceptionName;
				Value exceptionValue;

				virtual void gcMark(GCHeap& heap) const {
					heap.mark(exceptionName);
					exceptionValue.gcMark(heap);
					super::gcMark(heap);
				}
};

class WithFrame : public Frame {
	public:		typedef Frame super;
				WithFrame(Frame* parentFrame, Object* withObject);
				virtual const CodeDefinition* getFunctionDef() const;
				virtual Flags getOwnProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
				virtual bool deleteProperty(Engine& engine, const Value& key);
				virtual void declareVar(Engine& engine, const String* name, const Value& initValue) { parentFrame->declareVar(engine, name, initValue); }

	protected:	Object* withObject;
				virtual void gcMark(GCHeap& heap) const {
					heap.mark(withObject);
					super::gcMark(heap);
				}
};

class Arguments : public LazyObject<Object> {
	public:		typedef LazyObject<Object> super;

				Arguments(Heap* heap, ScriptFrame* frame, UInt32 argumentsCount);
				virtual const String* getClassName() const;
				virtual const String* toString(GCHeap& heap) const;
				virtual Object* getPrototype(Engine& engine) const;
				virtual Flags getOwnProperty(Engine& engine, const Value& key, Value* v) const;
				virtual bool setOwnProperty(Engine& engine, const Value& key, const Value& v, Flags flags);
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
					, OBJ_TO_NUMBER_OP								// stack: value -> primitive_value (number preferred)
					, OBJ_TO_STRING_OP								// stack: value -> primitive_value (string preferred)
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
					, JT_OR_POP_OP, JF_OR_POP_OP					// operand: offset, stack: boolean ->				// only pops when not jumping, for logical and/or instructions
					, CALL_OP										// operand: n, stack: function, n * args -> return_value
					, CALL_METHOD_OP								// operand: n, stack: object, name, n * args -> return_value
					, CALL_EVAL_OP									// operand: n, stack: object, name, n * args -> return_value // special eval call is required because of need to differentiate direct or indirect call to eval
					, NEW_OP										// operand: n, stack: constructor object, n * args -> new_object, return_value
					, NEW_RESULT_OP									// stack: new_object, return_value -> new_this value
					, NEW_OBJECT_OP									// stack: -> object
					, NEW_ARRAY_OP									// stack: -> array_object
					, NEW_REG_EXP_OP								// stack: pattern_string, flags_string -> regexp_object
					, DELETE_FRAME_AND_RETURN_OP
					, RETURN_OP
					, RETURN_TO_HOST_OP
					, THIS_OP, VOID_OP, FRAME_OP					// stack: -> value
					, DELETE_OP										// stack: object, name -> boolean
					, GEN_FUNC_OP									// operand: const_index (function), stack: -> function
					, DECLARE_OP									// operand: const_index (name), stack: function|undefined ->		// only used by eval code to declare vars and functions (non-eval code sets all this up on entry and using WRITE_INDEXED_OP).
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

				static CodeWord packInstruction(const Opcode opcode, const Int32 operand);
				static void unpackInstruction(const CodeWord codeWord, Opcode& opcode, Int32& operand);

	public:		Processor(Engine& engine, CStackEntry* cstackStart, CStackEntry* cstackEnd, Value* vstackStart, Value* vstackEnd);
				void chain(const CodeWord* stub);
				void chain(ScriptFunction* func, UInt32 argc, const Value* argv, Object* thisObject);
				void chain(const CodeWord* ip, Frame* frame, Object* thisObject); // FIX : turn into enter global code or what?
				void enterEval(CodeDefinition* codeDef, bool directCall);
	// FIX :			void call(ScriptFunction* func, UInt32 argc, const Value* argv, Object* thisObject);
				void resetTimeOut(Int32 clockCycles);
				bool run();
				Value getResult() const { return result; }
				void error(ErrorType errorType, const String* message = 0);
				
	protected:	void push(const Value& v);
				void pop(Int32 count = 1);
				void poppush(const Value& v);
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
				void emit(Processor::Opcode op, Int32 operand = 0);
				QuickVector<CodeWord>* changeOutput(QuickVector<CodeWord>* newOutputCode);
				void emitMultiple(const CodeWord* begin, const CodeWord* end);
				int addConstant(const Value& constant);
				void emitWithConstant(Processor::Opcode op, const Value& constant);
				Int32 emitForwardBranch(Processor::Opcode op);
				void completeForwardBranch(Int32 point);
				Int32 markBackwardBranch();
				void emitBackwardBranch(Processor::Opcode op, Int32 point);

				ExpressionResult safeKeep(const ExpressionResult& xr); // Used to store result into a new temporary local (e.g.used by return statement if there are finally blocks to call first)
				void returnSafeKept(const ExpressionResult& xr, bool doReset); // if doReset == true then reset the local variable used for safe-keeping (to UNDEFINED)
				ExpressionResult makeRValue(const ExpressionResult& xr, bool toPrimitive = false, bool preferString = false); // Creates an r-value (i.e. pushed on value stack) out of a result.
				ExpressionResult makeAssignment(const ExpressionResult& xr); // Creates an assignment out of an l-value (throws if xr is not a valid l-value)
				ExpressionResult discard(const ExpressionResult& xr); // Discards result (e.g. popping it if it is on stack)

				void completeContinues(const Scope* fromScope, const Scope* untilScope);
				void completeBreaks(const Scope* scope);
				void functionReturn();

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
				ExpressionResult declareIdentifier(const String* name, bool func);
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
				void semicolon(bool acceptLF = true);
				void expectToken(const char* t, bool eatLeadingWhite);
				void white();
				bool whiteNoLF();
				void token();
				Char* unescape(Char* buffer, const Char* e);
				bool isLeadingIdentifierChar(Char c);
				const Char* identifier(bool required, bool acceptReservedKeywords);

				GCHeap& heap; // FIX : necessary?
				CodeDefinition* const codeDef;
				const Target compilingFor;
				std::map<Value, Int32> definedConstants; // FIX : drop map dependency, increases code size
				QuickVector<CodeWord> setupCode; // function declarations (and vars in eval code), inserted at top of function when finalizing
				const Char* p;
				const Char* e;
				QuickVector<CodeWord>* outputCode; // FIX : these three are bad, they are set and reset in blocks, which is what we normally use the C++ stack for really, e.g. create a new local compiler with the change... or a separate Compiler Context object
				bool acceptInOperator;
				int withScopeDepth; // FIX : if we have a Context object instead as "this" we could create a new one with a simple flag for this instead of yucky counter
				bool canDeleteFrameOnExit; // three reasons we can't delete frame on exit: 1) function generated inside function == we might need the closure, 2) function calls eval directly == it might generate a function, 3) function uses arguments variable == arguments object share variable space with frame

				virtual void gcMark(GCHeap& heap) const {
					heap.mark(codeDef);
					for (std::map<Value, Int32>::const_iterator it = definedConstants.begin(); it != definedConstants.end(); ++it) {
						it->first.gcMark(heap);
					}
					super::gcMark(heap);
				}
};

#endif
