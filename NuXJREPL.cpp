#include "NuXJScript.h"
#include <fstream>
#include <memory>
#include <vector>
#include <sstream>

using namespace NuXJScript;

static int recursiveStackCheck(const Code& code, QuickArray<Int32>& stackDepths
		, Int32 offset, Int32 currentStackDepth, Int32& maxStackDepth) {
	const CodeWord* codeWords = code.getCodeWords();
	const UInt32 codeSize = code.getCodeSize();
	
	int errors = 0;
	for (; offset < codeSize; ++offset) {
		if (stackDepths[offset] != DEAD_CODE_STACK_DEPTH) {
			if (stackDepths[offset] != currentStackDepth) {
				std::cerr << "Conflicting stack depth @" << offset << ": " << currentStackDepth << " & " << stackDepths[offset] << std::endl;
				++errors;
			}
			return errors;
		}

		const std::pair<Processor::Opcode, Int32> pair = Processor::unpackInstruction(codeWords[offset]);
		const Processor::Opcode opcode = pair.first;
		const Int32 operand = pair.second;
		if (opcode < 0 || opcode >= Processor::OP_COUNT) {
			std::cerr << "Invalid opcode " << static_cast<int>(opcode) << "@" << offset << std::endl;
			++errors;
		}
		stackDepths[offset] = currentStackDepth;
		if (currentStackDepth > code.getMaxStackDepth()) {
			std::cerr << "Stack overflow (code max is " << code.getMaxStackDepth() << ", stack depth @" << offset << " is " << currentStackDepth << ")" << std::endl;
			++errors;
		}
		maxStackDepth = std::max(maxStackDepth, currentStackDepth);
#if (VERIFY_STACK_LOGIC)
	if (stackDepths[offset] != code.stackDepths[offset]) {
		std::cerr << "Invalid compile-time stack depth @" << offset << " (" << code.stackDepths[offset] << " vs " << stackDepths[offset] << ")" << std::endl;
		++errors;
	}
#endif
		const Processor::OpcodeInfo& info = Processor::getOpcodeInfo(opcode);
		const Int32 thisStackUse = info.stackUse + (((info.flags & Processor::OpcodeInfo::POP_OPERAND) != 0) ? -operand : 0);
		currentStackDepth += thisStackUse;
		maxStackDepth = std::max(maxStackDepth, currentStackDepth);
		if (currentStackDepth < 0) {
			std::cerr << "Negative stack depth @" << offset << std::endl;
			++errors;
		}
		switch (opcode) {
			case Processor::JT_OP:
			case Processor::JF_OP:
			case Processor::JT_OR_POP_OP:
			case Processor::JF_OR_POP_OP:
			case Processor::JMP_OP:
			case Processor::JSR_OP:
			case Processor::TRY_OP:
			case Processor::NEXT_PROPERTY_OP: {
				const Int32 targetOffset = offset + 1 + operand;
				if (targetOffset < 0 || targetOffset >= codeSize) {
					std::cerr << "Invalid target @" << offset << ": @" << targetOffset << std::endl;
					++errors;
					return errors;
				}
				const Int32 targetStackDepth = currentStackDepth + (((info.flags & Processor::OpcodeInfo::NO_POP_ON_BRANCH) != 0) ? 1 : 0)
						+ (((info.flags & Processor::OpcodeInfo::POP_ON_BRANCH) != 0) ? -1 : 0);
				errors += recursiveStackCheck(code, stackDepths, targetOffset, targetStackDepth, maxStackDepth);
				if (opcode == Processor::JMP_OP) {
					return errors;
				}
				break;
			}
			case Processor::RETURN_OP: // fix: check all returns from jsr end with the same stack pointer as entry (keep track with extra parameter that only updates with JSR)
			case Processor::THROW_OP: {
				return errors;
			}
	
			default: break;
		}
	}

	return errors;
}

#if (_MSC_VER)
#include <Windows.h>
long getCPUSecs() {
	::FILETIME creationTime;
	::FILETIME exitTime;
	::FILETIME kernelTime;
	::FILETIME userTime;
	BOOL success = ::GetProcessTimes(::GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime);
	assert(success);
	return ((static_cast<__int64>(userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime) / 10000000;
}
#else
double getCPUSecs() {
	rusage rus;
	int res = getrusage(RUSAGE_SELF, &rus);
	assert(res == 0);
	return rus.ru_utime.tv_sec + rus.ru_utime.tv_usec / 1000000.0;
}
#endif

	bool interactive = true;

	std::vector<std::string> ioLines;

	void pushIOLines(char typeChar, const String& s) {
		const Char* p = s.begin();
		const Char* e = s.end();
		do {
			const Char* b = p;
			while (p != e && *p != '\n') {
				++p;
			}
			ioLines.push_back(typeChar + std::string(" ") + std::string(b, p));
			if (p != e) {
				++p;
			}
		} while (p != e);
	}

	void pushIOStop() {
		ioLines.push_back("-");
	}

	class PrintFunction : public Function {
		public:		virtual Value invoke(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
						const String* s = (argc >= 1 ? argv[0].toString(rt.getHeap()) : &String::EMPTY);
						std::wcout << s->toStdString().c_str() << std::endl;
						if (interactive) {
							pushIOLines('<', *s);
						}
						return Value::UNDEFINED;
					}
	};

	struct GCFunction : public Function {
		virtual Value invoke(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
			rt.getHeap().gc();
			return Value::UNDEFINED;
		}
	};

	class NativeTestObject : public Object {
		protected:	struct AddFunction : public Function {
						typedef Function super;
						virtual Value invoke(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
							Heap& heap = rt.getHeap();
							if (thisObject->getClassName() != &NativeTestObject::className) {
								ScriptException::throwError(heap, TYPE_ERROR, "Invalid class");
							}
							const NativeTestObject* me = reinterpret_cast<NativeTestObject*>(thisObject);
							const double addX = (argc >= 1 ? argv[0].toDouble() : 0.0);
							const double addY = (argc >= 2 ? argv[1].toDouble() : 0.0);
							return new(heap) NativeTestObject(heap.managed(), addX + me->x, addY + me->y);
						}
					};

		public:		typedef Object super;
					NativeTestObject(GCList& gcList, double x, double y) : super(gcList), x(x), y(y) { }
					virtual const String* getClassName() const { return &className; };
					virtual Flags getOwnProperty(Runtime& rt, const Value& key, Value* v) const {
						if (key.equalsString("x")) { *v = x; return HIDDEN_CONST_FLAGS; };
						if (key.equalsString("y")) { *v = y; return HIDDEN_CONST_FLAGS; };
						if (key.equalsString("add")) { *v = &addFunction; return HIDDEN_CONST_FLAGS; };
						return super::getOwnProperty(rt, key, v);
					}
		
					virtual const String* toString(Heap& heap) const {
						std::stringstream ss;
						ss << "x: " << x << ", y: " << y;
						return String::alloc(heap, ss.str().c_str());
					}

		
		protected:	double x;
					double y;
					static String className;
					static AddFunction addFunction;
	};
	String NativeTestObject::className("NativeTestObject");
	NativeTestObject::AddFunction NativeTestObject::addFunction;

	class CallbackTest : public Function {
		public:		virtual Value invoke(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
						PrintFunction printFunction;
						for (int i = 0; i < argc; ++i) {
							Function* f = argv[i].asFunction();
							if (f == 0) {
								f = &printFunction;
							}
							rt.call(f, 1, &argv[i]);
						}
						return Value::UNDEFINED;
					}
	};

	static void disassemble(Heap& heap, const Code& code) {
		const CodeWord* codeWords = code.getCodeWords();
		const Value* constants = code.getConstants()->begin();
		const UInt32 codeSize = code.getCodeSize();
		
		QuickArray<Int32> stackDepths(codeSize, &heap);
		std::fill(stackDepths.begin(), stackDepths.end(), DEAD_CODE_STACK_DEPTH);
		
		Int32 maxStackDepth = 0;
		int errors = recursiveStackCheck(code, stackDepths, 0, 1, maxStackDepth);
		if (maxStackDepth != code.getMaxStackDepth()) {
			std::cerr << "Notice: code definition max stack depth is " << code.getMaxStackDepth() << " but max stack depth actually used is " << maxStackDepth << std::endl;
		}
		
		for (UInt32 offset = 0; offset < codeSize; ++offset) {
			const std::pair<Processor::Opcode, Int32> pair = Processor::unpackInstruction(codeWords[offset]);
			const Processor::Opcode opcode = pair.first;
			const Int32 operand = pair.second;
			if (opcode < 0 || opcode >= Processor::OP_COUNT) {
				break;
			}

			if (stackDepths[offset] == DEAD_CODE_STACK_DEPTH) {
				std::cerr << "?";
			} else {
				std::cerr << stackDepths[offset];
			}
			std::cerr << "\t";
	#if (VERIFY_STACK_LOGIC)
			if (code.stackDepths[offset] == DEAD_CODE_STACK_DEPTH) {
				std::cerr << "?";
			} else {
				std::cerr << code.stackDepths[offset];
			}
			std::cerr << "\t";
	#endif
			std::cerr << "@" << offset << ":\t" << Processor::getOpcodeInfo(opcode).mnemonic;
			
			switch (opcode) {
				case Processor::JT_OP:
				case Processor::JF_OP:
				case Processor::JT_OR_POP_OP:
				case Processor::JF_OR_POP_OP:
				case Processor::JMP_OP:
				case Processor::JSR_OP:
				case Processor::TRY_OP:
				case Processor::NEXT_PROPERTY_OP: {
					const Int32 target = offset + 1 + operand;
					std::cerr << " @" << target;
					break;
				}
				case Processor::READ_LOCAL_OP:
				case Processor::WRITE_LOCAL_OP:
				case Processor::WRITE_LOCAL_POP_OP: {
					const Int32 index = operand;
					const String* name = code.getLocalName(index);
					if (name != 0) std::wcerr << L" $" << name->toStdString();
					std::wcerr << L" (" << index << L")";
					break;
				}
				case Processor::CALL_OP:
				case Processor::CALL_METHOD_OP:
				case Processor::CALL_EVAL_OP:
				case Processor::NEW_OP:
				case Processor::POP_OP:
				case Processor::PUSH_BACK_OP:
				case Processor::PUSH_ELEMENTS_OP: std::cerr << " *" << operand; break;
				case Processor::REPUSH_OP: std::cerr << " " << operand; break;
				case Processor::CONST_OP:
				case Processor::GEN_FUNC_OP:
				case Processor::CATCH_SCOPE_OP: std::wcerr << L" #" << constants[operand].toString(heap)->toStdString(); break;
				case Processor::DECLARE_OP:
				case Processor::READ_NAMED_OP:
				case Processor::WRITE_NAMED_OP:
				case Processor::WRITE_NAMED_POP_OP:
				case Processor::ADD_PROPERTY_OP:
				case Processor::DELETE_NAMED_OP:
				case Processor::TYPEOF_NAMED_OP: std::wcerr << L" " << constants[operand].toString(heap)->toStdString(); break;
				default: break;
			}
			std::cerr << std::endl;
		}
		
		std::cerr << "\tMax stack depth: " << code.getMaxStackDepth() << std::endl;
		std::cerr << "\tCode: " << code.getCodeSize() << std::endl;
	//	std::cerr << "\tConstants: " << code.getConstants().size() << std::endl;
		std::cerr << "\tVars: " << code.getVarsCount() << std::endl;
		std::cerr << "\tArguments: " << code.getArgumentsCount() << std::endl;
	/* FIX :	for (const Value* p = code.constants.begin(); p != code.constants.end(); ++p) {
			std::wcerr << "\t\t" << p.toString(heap).toStdString() << std::endl;
		}*/

		assert(errors == 0);
	}

	Value disassemble(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Heap& heap = rt.getHeap();
		Function* f = (argc >= 1 ? argv[0].asFunction() : 0);
		if (f == 0) { // FIX : sub to make is not a function error or something?
			ScriptException::throwError(heap, TYPE_ERROR, String::concat(heap, *argv[0].toString(heap), String(" is not a function")));
		}
		const Code* code = f->getScriptCode();
		if (code == 0) {
			ScriptException::throwError(heap, TYPE_ERROR, "Cannot disassemble native code");
		}
		disassemble(heap, *code);
		return Value::UNDEFINED;
	}

	Value gcTest(Runtime& rt, Processor& processor, UInt32 argc, const Value* argv, Object* thisObject) {
		Heap& heap = rt.getHeap();
		heap.gc();
		Value v = String::alloc(heap, "");
		for (int i = 0; i < argc; ++i) {
			v = String::concat(heap, *v.toString(heap), *argv[i].toString(heap));
		}
		return v;
	}

bool pauseBeforeQuit = false;
bool suppressStdErr = false; // FIX : drop

int testMain(int argc, const char* argv[]) {
    String source(String::EMPTY);
    std::auto_ptr<std::wifstream> inputFileStream;
    std::wistream* inStream = &std::wcin;
    bool doTime = false;
    int gcRate = 256; // FIX : drop or what?
    size_t peakMemory = 0;
    bool autoGCRate = true;
	bool doSuppressStdErr = false;
	bool loadStdLib = true;
    for (int argi = 1; argi < argc; ++argi) {
        if (strcmp(argv[argi], "-t") == 0) doTime = true;
        else if (strcmp(argv[argi], "-s") == 0) doSuppressStdErr = true;
        else if (argi + 1 < argc && strcmp(argv[argi], "-gc") == 0) {
            gcRate = atoi(argv[++argi]);
            autoGCRate = false;
        }
        else if (strcmp(argv[argi], "-p") == 0) pauseBeforeQuit = true;
        else if (strcmp(argv[argi], "-n") == 0) loadStdLib = false;
        else if (inputFileStream.get() == 0) {
            inputFileStream.reset(new std::wifstream(argv[argi]));
            inStream = inputFileStream.get();
            interactive = false;
        } else {
            std::cout << "Too many arguments" << std::endl;
            return 1;
        }
    }

    /*std::cout << sizeof(Value) << std::endl;
     std::cout << sizeof(String) << std::endl;
     std::cout << sizeof(ScriptObject) << std::endl;
     std::cout << sizeof(Array) << std::endl;
     std::cout << sizeof(ScriptFunction) << std::endl;*/
suppressStdErr = true;
    const String LF_STRING("\n");
    const String PRINT_STRING("print");
    /*std::cerr << "Value size: " << sizeof (Value) << std::endl;
     std::cerr << "Bucket size: " << sizeof (Table::Bucket) << std::endl;
     std::cerr << "Table size: " << sizeof (Table) << std::endl;*/

    Heap heap;
    Runtime rt(heap);

    rt.setMemoryCap(8192*10);
    Var anObject = rt.eval("({ a: 123.45, b: 'y' })");
    Var aValue = anObject["a"];
    std::wstring ws = anObject["a"];
    anObject["c"] = 99;
    anObject["z"] = rt.eval("(123)");
    std::wcout << aValue << std::endl;
    std::wcout << static_cast<Var>(anObject["z"]) << std::endl;
    std::wcout << ws << std::endl;

	
	Var v23(rt);
    bool b = v23;
    const Value args[1] = { v23 };
  //  rt.call(v23, 0, args);
    std::wcout << rt.eval("s = 0; for (i = 0; i < 100; ++i) s += i; s") << std::endl;
    double v = rt.eval("s = 0; for (i = 0; i < 100; ++i) s += i; s");
    std::wcout << v23 << std::endl;

    rt.setMemoryCap(512 * 1024 * 1024);
 /*   ScriptObject globals(heap.roots(), &rt.objectPrototype);
	rt.setGlobalObject(&globals);
	*/
	Object& globals = *rt.getGlobalObject();
	
rt.setupStandardLibrary();
	Var globs = rt.getGlobals();
	globs["tezt"] = 55;
	globs["tezt2"] = globs["tezt"];
	double ddd = globs["tezt"];
	globs["f"] = rt.eval("(function(a, b) { return a + b })");
	globs["arr"] = rt.eval("([1,3,5,2,4,6])");
	Var o = globs["o"] = new(heap) ScriptObject(heap.managed(), &rt.objectPrototype);
	o["gcTest"] = new(heap) FunctorAdapter<NativeFunction>(heap.managed(), gcTest);
	o["zub"] = "yo";
	std::wcout << globs["f"](29, 99) << std::endl;
	Var a = globs["arr"];
	std::wcout << o["gcTest"]("hej", 12959) << std::endl;
	std::wcout << o["gcTest"]("hej", 12959, "zzz") << std::endl;
	const Var list[4] = { Var(rt, "a"), Var(rt, "bcd"), Var(rt, 1391), Var(rt, "zz9z9") };
	std::wcout << o["gcTest"](4, list) << std::endl;
	std::wcout << globs["o"][o["gcTest"]("zub")] << std::endl;

	globs["o2"] = rt.newObject();
	Var arrr = globs["o2"]["a"] = rt.newArray();
	for (int i = 0; i < 22; ++i) arrr[i] = i * 3;
	std::wcout << globs["o2"]["a"]["length"] << std::endl;
	std::wcout << globs["o2"]["a"]["toString"]() << std::endl;
	std::wcout << globs["o2"]["a"][15]["toString"](16) << std::endl;

	Var regExp = rt.eval("(/x.*y/)");
	std::wcout << regExp["test"]("xyzzzy") << std::endl;
//	a = 55;
	for (int i = 0; i < a.length(); ++i) {
		std::wcout << a[i] << std::endl;
		a[i] = a[i] * 2;
	}

    PrintFunction printFunction;
    globals.setOwnProperty(rt, &PRINT_STRING, &printFunction, DONT_ENUM_FLAG);
    GCFunction gcFunction;
    const String GC_STRING("gc");
    globals.setOwnProperty(rt, &GC_STRING, &gcFunction, DONT_ENUM_FLAG);
    globals.setOwnProperty(rt, String::alloc(heap, "dasm"), new(heap) FunctorAdapter<NativeFunction>(heap.managed(), disassemble), DONT_ENUM_FLAG);
CallbackTest callbackTest;
globals.setOwnProperty(rt, String::alloc(heap, "callbackTest"), &callbackTest, DONT_ENUM_FLAG);
//globals.setOwnProperty(rt, String::alloc(heap, "nativeTestObject"), new(heap) NativeTestObject(heap.managed(), 1.4, 2.9));

	if (loadStdLib) {
		try {
/* FIX : drop			std::wifstream stdlibStream("stdlib.js");
			std::wstring sourceWString = std::wstring(std::istreambuf_iterator<wchar_t>(stdlibStream), std::istreambuf_iterator<wchar_t>());
			String source = String(heap.roots(), sourceWString.c_str());
			stdlibStream.close();*/
			rt.setupStandardLibrary();
		}
		catch (const std::exception& x) {
			std::cerr << "exception setting up standard lib: " << x.what() << std::endl;
			return 1;
		}
		catch (...) {
			std::cerr << "exception setting up standard lib" << std::endl;
			return 1;
		}
	}
    suppressStdErr = doSuppressStdErr;

	Processor processor(rt);
	processor.run(STANDARD_CYCLES_BETWEEN_AUTO_GC);	// just testing some weird behaviour
//	const Value TEST_ARGV[1] = { String::alloc(heap, "Hello") };
//	printFunction.invoke(processor, 1, TEST_ARGV, &globals);
//	processor.run();	// just testing some weird behaviour
	
    while (inStream->good()) {
        bool execute = false;
        try {
            std::wstring s;
            if (interactive) {
                std::getline(*inStream, s);
                // FIX : add #undo that drops just the last one
                if (s == L"#save" || s.substr(0, 6) == L"#save ") {
                    const time_t t = time(0);
                    char fn[257];
                    if (s.size() >= 6) {
                        std::wstring ws = L"tests/";
                        ws += s.substr(6);
                        ws += L".io";
                        strncpy(fn, std::string(ws.begin(), ws.end()).c_str(), 256);
                        fn[256] = 0;
                    } else {
                        strftime(fn, 256, "tests/%Y%m%d_%H%M%S.io", localtime(&t));
                    }
                    std::ofstream saveStream(fn);
                    for (std::vector<std::string>::const_iterator it = ioLines.begin(); it != ioLines.end(); ++it) {
                        saveStream << *it << std::endl;
                    }
                    saveStream.close();
                    ioLines.clear();
                    std::cout << "saved to " << fn << std::endl;
                } else if (s == L"#purge") {
                    ioLines.clear();
                    std::cout << "purged" << std::endl;
                } else if (!s.empty()) {
                    String line(heap.roots(), s.c_str());
                    if (!source.empty()) source = String(heap.roots(), source, LF_STRING);
                    source = String(heap.roots(), source, line);
                } else {
                    if (source.size() > 0 && source[0] == '?') {
                        source = String(heap.roots(), String(heap.roots(), String("print("), String(heap.roots(), source.begin() + 1, source.end())), String(")")); // FIX : operator +
                    }
                    if (interactive) {
                        pushIOLines('>', source);
                    }
                    execute = true;
                }
            } else {
				std::wstring sourceWString = std::wstring(std::istreambuf_iterator<wchar_t>(*inStream), std::istreambuf_iterator<wchar_t>());
				source = String(heap.roots(), sourceWString.c_str());
                inStream->get(); // FIX : weird, seems necessary to force !inStream->good
                execute = true;
            }
			
            if (execute) {
			    Code globalCode(heap.roots(), rt.getSharedConstants());
                Compiler compiler(heap.roots(), &globalCode, (interactive ? Compiler::FOR_EVAL : Compiler::FOR_GLOBAL));
                try {
                    compiler.compile(source);
                }
                catch (const Exception&) {
                	int lineNumber;
                	int columnNumber;
                	compiler.getStopLineAndColumnNumbers(lineNumber, columnNumber);
                    std::wstring ws;
                    {
                        std::wstringstream ss;
                        ss << L"!!!! Line: " << lineNumber;
						// ss << ", column: " << columnNumber; // FIX : keep or not? if so, need to update all tests:
                        ws = ss.str();
                    }
                    std::wcout << ws << std::endl;
                    if (interactive) {
                        pushIOLines('!', String(heap.roots(), ws.c_str()));
                    }
// FIX :						std::wcout << std::wstring(std::max<const Char*>(stopPoint - 8, source.begin()), std::min<const Char*>(stopPoint + 8, source.end())) << std::endl;
                    throw;
                }
                if (!suppressStdErr) {
/*                	const Constants* const consts = rt.getSharedConstants();
                    for (const Value* p = consts->begin(); p != consts->end(); ++p) {
                        std::wcerr << "\t\t" << p->toString(heap)->toStdString() << std::endl;
                    }*/
                }
               	processor.run(STANDARD_CYCLES_BETWEEN_AUTO_GC);	// just testing some weird behaviour
                processor.enterGlobalCode(&globalCode);

                bool done = false;
			    rt.resetTimeOut(60);
                double start = getCPUSecs();
                do {
                    done = !processor.run(STANDARD_CYCLES_BETWEEN_AUTO_GC);
					rt.autoGC();
					rt.checkTimeOut();
                   	peakMemory = std::max<long>(peakMemory, heap.size());
                } while (!done);

				processor.run(STANDARD_CYCLES_BETWEEN_AUTO_GC);	// just testing some weird behaviour
				if (!suppressStdErr) {
					Value v = processor.getResult();
					std::wcerr << L"\t=" << v.toString(heap)->toStdString() << std::endl;
				}
				if (doTime) {
					double end = getCPUSecs();
					std::cerr << (end - start) << "s" << std::endl;
					std::cerr << peakMemory / (1024.0 * 1024.0) << "MiB" << std::endl;
				}

                source = String::EMPTY;
            }
        }
        catch (const ScriptException& x) {
            source = String::EMPTY;
            std::wstring ws = x.getValue().toString(heap)->toStdString();
            ws = L"!!!! " + ws;
            std::wcout << ws << std::endl;
            if (!interactive) {
                return 1;
            } else {
                pushIOLines('!', String(heap.roots(), ws.c_str()));
            }
        }
        catch (const std::exception& x) {
            source = String::EMPTY;
            std::cout << "!!!! " << x.what() << std::endl;
            if (!interactive) {
                return 1;
            } else {
                pushIOLines('!', x.what());
            }
        }
        catch (...) {
            source = String::EMPTY;
            std::cout << "Unknown exception" << std::endl;
            if (!interactive) {
                return 1;
            } else {
                pushIOLines('!', "Unknown exception");
            }
        }
        if (execute && interactive) {
            pushIOStop();
        }
    }
    return 0;
}

int main(int argc, const char* argv[]) {
    int rc = testMain(argc, argv);
    if (pauseBeforeQuit) std::wcin.get();
    return rc;
}
