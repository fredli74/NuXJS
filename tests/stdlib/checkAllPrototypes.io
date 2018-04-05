> function analyzeFlags(object, property, testValue) {
>     var origObject = Object; // In case it is Object we are testing
>     property = '' + property;
>     testValue = (testValue != null ? testValue : { });
>     if (!object.hasOwnProperty(property)) {
>         print(property + " : doesn't exist");
>		  return null;
>     } else {
>         var dontEnum = true;
>         for (var p in object) {
>             if (p === property) {
>                 dontEnum = false;
>                 break;
>             }
>         }
>         var doEnum = object.propertyIsEnumerable(property);
>         if (doEnum === dontEnum) {
>             print("Warning! object.propertyIsEnumerable() returned " + doEnum + " for property '" + property
>                     + "' although it " + (dontEnum ? "was not" : "was") + " listed by for-in");
>         }
>         var valueWas = object[property];
>         object[property] = testValue;
>         var valueIs = object[property];
>         var readOnly = (valueIs !== testValue);
>         var deleted = delete object[property];
>         var dontDelete = object.hasOwnProperty(property);
>         print(property + " : " + (dontEnum ? "dontEnum," : "") + (readOnly ? "readOnly," : "")
>                 + (dontDelete ? "dontDelete," : "") + " (deleted:" + deleted + ')');
>         if (origObject.defineProperty) {
>             origObject.defineProperty(object, property, { value: valueWas, enumerable: !dontEnum, configurable: !dontDelete, writable: !readOnly });
>         } else {
>             object[property] = valueWas;
>         }
>		  return { dontEnum: dontEnum, readOnly: readOnly, dontDelete: dontDelete };
>     }
> }   
-
> function checkFunc(className, funcName) {
> 	print(className + "." + funcName);
> 	print("--------");
> 	var o = this[className];
> 	var f = o[funcName];
> 	print(".name: " + f.name);
>	var ok = (f.name === funcName);
> 	print(".length: " + f.length);
> 	print("'" + funcName + "' in " + className + ": " + (funcName in o))
> 	var flags = analyzeFlags(o, funcName);
>	ok = ok && flags.dontEnum && !flags.readOnly && !flags.dontDelete;
> 	flags = analyzeFlags(f, 'name');
>	ok = ok && flags.dontEnum && !flags.readOnly && !flags.dontDelete;
> 	flags = analyzeFlags(f, 'length');
>	ok = ok && flags.dontEnum && flags.readOnly && flags.dontDelete;
>	try { var o = new f; print("constructable"); ok = false; } catch (x) { print("unconstructable: " + x); }
> 	print("OK: " + ok);
> 	print("--------");
> }
-
> function checkProtoFunc(className, funcName) {
> 	print(className + ".prototype." + funcName);
> 	print("--------");
> 	var p = this[className].prototype;
> 	var f = p[funcName];
> 	print(".name: " + f.name);
>	var ok = (f.name === funcName);
> 	print(".length: " + f.length);
> 	print("'" + funcName + "' in " + className + ".prototype: " + (funcName in p))
> 	var flags = analyzeFlags(p, funcName);
>	ok = ok && flags.dontEnum && !flags.readOnly && !flags.dontDelete;
> 	flags = analyzeFlags(f, 'name');
>	ok = ok && flags.dontEnum && !flags.readOnly && !flags.dontDelete;
> 	flags = analyzeFlags(f, 'length');
>	ok = ok && flags.dontEnum && flags.readOnly && flags.dontDelete;
>	try { var o = new f; print("constructable"); ok = false; } catch (x) { print("unconstructable: " + x); }
> 	print("OK: " + ok);
> 	print("--------");
> }
-
> function checkConstant(className, constantName) {
> print(className + "." + constantName);
> print("--------");
> var o = this[className];
> print("'" + constantName + "' in " + className + ": " + (constantName in o))
> print(className + "." + constantName + " = " + o[constantName])
> var flags = analyzeFlags(o, constantName);
> var ok = flags.dontEnum && flags.readOnly && flags.dontDelete;
> print("OK: " + ok);
> print("--------");
> }
-
> checkFunc('Math', 'abs', 1);
< Math.abs
< --------
< .name: abs
< .length: 1
< 'abs' in Math: true
< abs : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: abs is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'acos', 1);
< Math.acos
< --------
< .name: acos
< .length: 1
< 'acos' in Math: true
< acos : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: acos is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'asin', 1);
< Math.asin
< --------
< .name: asin
< .length: 1
< 'asin' in Math: true
< asin : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: asin is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'atan', 1);
< Math.atan
< --------
< .name: atan
< .length: 1
< 'atan' in Math: true
< atan : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: atan is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'atan2', 2);
< Math.atan2
< --------
< .name: atan2
< .length: 2
< 'atan2' in Math: true
< atan2 : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: atan2 is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'ceil', 1);
< Math.ceil
< --------
< .name: ceil
< .length: 1
< 'ceil' in Math: true
< ceil : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: ceil is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'cos', 1);
< Math.cos
< --------
< .name: cos
< .length: 1
< 'cos' in Math: true
< cos : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: cos is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'exp', 1);
< Math.exp
< --------
< .name: exp
< .length: 1
< 'exp' in Math: true
< exp : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: exp is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'floor', 1);
< Math.floor
< --------
< .name: floor
< .length: 1
< 'floor' in Math: true
< floor : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: floor is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'log', 1);
< Math.log
< --------
< .name: log
< .length: 1
< 'log' in Math: true
< log : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: log is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'max', 2);
< Math.max
< --------
< .name: max
< .length: 2
< 'max' in Math: true
< max : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: max is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'min', 2);
< Math.min
< --------
< .name: min
< .length: 2
< 'min' in Math: true
< min : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: min is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'pow', 2);
< Math.pow
< --------
< .name: pow
< .length: 2
< 'pow' in Math: true
< pow : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: pow is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'random', 1);
< Math.random
< --------
< .length: 0
< 'random' in Math: true
< random : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: random is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'round', 1);
< Math.round
< --------
< .name: round
< .length: 1
< 'round' in Math: true
< round : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: round is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'sin', 1);
< Math.sin
< --------
< .name: sin
< .length: 1
< 'sin' in Math: true
< sin : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: sin is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'sqrt', 1);
< Math.sqrt
< --------
< .name: sqrt
< .length: 1
< 'sqrt' in Math: true
< sqrt : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: sqrt is not a constructor
< OK: true
< --------
-
> checkFunc('Math', 'tan', 1);
< Math.tan
< --------
< .name: tan
< .length: 1
< 'tan' in Math: true
< tan : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: tan is not a constructor
< OK: true
< --------
-
> checkConstant('Math', 'E');
< Math.E
< --------
< 'E' in Math: true
< Math.E = 2.718281828459045
< E : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkConstant('Math', 'LN10');
< Math.LN10
< --------
< 'LN10' in Math: true
< Math.LN10 = 2.302585092994046
< LN10 : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkConstant('Math', 'LN2');
< Math.LN2
< --------
< 'LN2' in Math: true
< Math.LN2 = 0.6931471805599453
< LN2 : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkConstant('Math', 'LOG2E');
< Math.LOG2E
< --------
< 'LOG2E' in Math: true
< Math.LOG2E = 1.4426950408889633
< LOG2E : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkConstant('Math', 'LOG10E');
< Math.LOG10E
< --------
< 'LOG10E' in Math: true
< Math.LOG10E = 0.43429448190325185
< LOG10E : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkConstant('Math', 'PI');
< Math.PI
< --------
< 'PI' in Math: true
< Math.PI = 3.141592653589793
< PI : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkConstant('Math', 'SQRT1_2');
< Math.SQRT1_2
< --------
< 'SQRT1_2' in Math: true
< Math.SQRT1_2 = 0.7071067811865475
< SQRT1_2 : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkConstant('Math', 'SQRT2');
< Math.SQRT2
< --------
< 'SQRT2' in Math: true
< Math.SQRT2 = 1.4142135623730951
< SQRT2 : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkProtoFunc('Number', 'toString', 1);
< Number.prototype.toString
< --------
< .name: toString
< .length: 1
< 'toString' in Number.prototype: true
< toString : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toString is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Number', 'toLocaleString', 0);
< Number.prototype.toLocaleString
< --------
< .name: toLocaleString
< .length: 0
< 'toLocaleString' in Number.prototype: true
< toLocaleString : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toLocaleString is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Number', 'valueOf', 0);
< Number.prototype.valueOf
< --------
< .name: valueOf
< .length: 0
< 'valueOf' in Number.prototype: true
< valueOf : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: valueOf is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Number', 'toFixed', 1);
< Number.prototype.toFixed
< --------
< .name: toFixed
< .length: 1
< 'toFixed' in Number.prototype: true
< toFixed : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toFixed is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Number', 'toExponential', 1);
< Number.prototype.toExponential
< --------
< .name: toExponential
< .length: 1
< 'toExponential' in Number.prototype: true
< toExponential : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toExponential is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Number', 'toPrecision', 1);
< Number.prototype.toPrecision
< --------
< .name: toPrecision
< .length: 1
< 'toPrecision' in Number.prototype: true
< toPrecision : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toPrecision is not a constructor
< OK: true
< --------
-
> checkConstant('Number', 'MAX_VALUE')
< Number.MAX_VALUE
< --------
< 'MAX_VALUE' in Number: true
< Number.MAX_VALUE = 1.7976931348623156e+308
< MAX_VALUE : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkConstant('Number', 'MIN_VALUE')
< Number.MIN_VALUE
< --------
< 'MIN_VALUE' in Number: true
< Number.MIN_VALUE = 5e-324
< MIN_VALUE : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkConstant('Number', 'NaN')
< Number.NaN
< --------
< 'NaN' in Number: true
< Number.NaN = NaN
< NaN : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkConstant('Number', 'NEGATIVE_INFINITY')
< Number.NEGATIVE_INFINITY
< --------
< 'NEGATIVE_INFINITY' in Number: true
< Number.NEGATIVE_INFINITY = -Infinity
< NEGATIVE_INFINITY : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkConstant('Number', 'POSITIVE_INFINITY')
< Number.POSITIVE_INFINITY
< --------
< 'POSITIVE_INFINITY' in Number: true
< Number.POSITIVE_INFINITY = Infinity
< POSITIVE_INFINITY : dontEnum,readOnly,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkProtoFunc('String', 'toString', 0);
< String.prototype.toString
< --------
< .name: toString
< .length: 0
< 'toString' in String.prototype: true
< toString : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toString is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'valueOf', 0);
< String.prototype.valueOf
< --------
< .name: valueOf
< .length: 0
< 'valueOf' in String.prototype: true
< valueOf : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: valueOf is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'charAt', 1);
< String.prototype.charAt
< --------
< .name: charAt
< .length: 1
< 'charAt' in String.prototype: true
< charAt : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: charAt is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'concat', 1);
< String.prototype.concat
< --------
< .name: concat
< .length: 1
< 'concat' in String.prototype: true
< concat : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: concat is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'indexOf', 1);
< String.prototype.indexOf
< --------
< .name: indexOf
< .length: 1
< 'indexOf' in String.prototype: true
< indexOf : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: indexOf is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'lastIndexOf', 1);
< String.prototype.lastIndexOf
< --------
< .name: lastIndexOf
< .length: 1
< 'lastIndexOf' in String.prototype: true
< lastIndexOf : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: lastIndexOf is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'localeCompare', 1);
< String.prototype.localeCompare
< --------
< .name: localeCompare
< .length: 1
< 'localeCompare' in String.prototype: true
< localeCompare : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: localeCompare is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'match', 1);
< String.prototype.match
< --------
< .name: match
< .length: 1
< 'match' in String.prototype: true
< match : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: match is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'replace', 2);
< String.prototype.replace
< --------
< .name: replace
< .length: 2
< 'replace' in String.prototype: true
< replace : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: replace is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'search', 1);
< String.prototype.search
< --------
< .name: search
< .length: 1
< 'search' in String.prototype: true
< search : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: search is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'slice', 2);
< String.prototype.slice
< --------
< .name: slice
< .length: 2
< 'slice' in String.prototype: true
< slice : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: slice is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'split', 2);
< String.prototype.split
< --------
< .name: split
< .length: 2
< 'split' in String.prototype: true
< split : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: split is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'substring', 2);
< String.prototype.substring
< --------
< .name: substring
< .length: 2
< 'substring' in String.prototype: true
< substring : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: substring is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'toLowerCase', 0);
< String.prototype.toLowerCase
< --------
< .name: toLowerCase
< .length: 0
< 'toLowerCase' in String.prototype: true
< toLowerCase : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toLowerCase is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'toLocaleLowerCase', 0);
< String.prototype.toLocaleLowerCase
< --------
< .name: toLocaleLowerCase
< .length: 0
< 'toLocaleLowerCase' in String.prototype: true
< toLocaleLowerCase : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toLocaleLowerCase is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'toUpperCase', 0);
< String.prototype.toUpperCase
< --------
< .name: toUpperCase
< .length: 0
< 'toUpperCase' in String.prototype: true
< toUpperCase : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toUpperCase is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'toLocaleUpperCase', 0);
< String.prototype.toLocaleUpperCase
< --------
< .name: toLocaleUpperCase
< .length: 0
< 'toLocaleUpperCase' in String.prototype: true
< toLocaleUpperCase : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toLocaleUpperCase is not a constructor
< OK: true
< --------
-
> try { x = Math(); print("BAD"); } catch (e) { print(e) }
< TypeError: [object Math] is not a function
-
> try { x = new Math(); print("BAD"); } catch (e) { print(e) }
< TypeError: [object Math] is not a function
-
> try { x = new Math; print("BAD"); } catch (e) { print(e) }
< TypeError: [object Math] is not a function
-
