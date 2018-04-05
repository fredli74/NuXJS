> function analyzeFlags(object, property, testValue) {
>     var origObject = Object; // In case it is Object we are testing
>	  var defineProperty = Object.defineProperty; // and in case we are testing defineProperty
>     Object.prototype._hasOwnProperty = Object.prototype.hasOwnProperty; // we need another hasOwnProperty, can't simply save it away as the other because we would need to .call it, and .call can be tested too
>     Function.prototype._call = Function.prototype.call;
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
>         var dontDelete = object._hasOwnProperty(property);
>         print(property + " : " + (dontEnum ? "dontEnum," : "") + (readOnly ? "readOnly," : "")
>                 + (dontDelete ? "dontDelete," : "") + " (deleted:" + deleted + ')');
>         if (defineProperty) {
>             defineProperty(object, property, { value: valueWas, enumerable: !dontEnum, configurable: !dontDelete, writable: !readOnly });
>         } else {
>             object[property] = valueWas;
>         }
>         delete Object.prototype._hasOwnProperty;
>		  return { dontEnum: dontEnum, readOnly: readOnly, dontDelete: dontDelete };
>     }
> }   
-
> function checkFunc(className, funcName, expectedLength) {
> 	print(className + "." + funcName);
> 	print("--------");
> 	var o = this[className];
> 	var f = o[funcName];
> 	print(".name: " + f.name);
>	var ok = (f.name === funcName);
> 	print(".length: " + f.length);
>	ok = ok && (f.length === expectedLength);
> 	print("'" + funcName + "' in " + className + ": " + o.hasOwnProperty(funcName))
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
> function checkProtoFunc(className, funcName, expectedLength) {
> 	print(className + ".prototype." + funcName);
> 	print("--------");
>   var o = this[className]
> 	var flags = analyzeFlags(o, 'prototype');
>	var ok = flags.dontEnum && flags.readOnly && flags.dontDelete;
> 	var p = o.prototype;
>	print(className + ".prototype.constructor == " + className + " : " + (p.constructor === o))
>	ok = ok && p.constructor === o;
> 	var f = p[funcName];
> 	print(".name: " + f.name);
>	ok = ok && (f.name === funcName);
> 	print(".length: " + f.length);
>	ok = ok && (f.length === expectedLength);
> 	print("'" + funcName + "' in " + className + ".prototype: " + p.hasOwnProperty(funcName))
> 	flags = analyzeFlags(p, funcName);
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
> 	print(className + "." + constantName);
> 	print("--------");
> 	var o = this[className];
> 	print("'" + constantName + "' in " + className + ": " + o.hasOwnProperty(constantName))
> 	print(className + "." + constantName + " = " + o[constantName])
> 	var flags = analyzeFlags(o, constantName);
> 	var ok = flags.dontEnum && flags.readOnly && flags.dontDelete;
> 	print("OK: " + ok);
> 	print("--------");
> }
-
> function checkGlobalConstant(constantName) {
> 	print(constantName);
> 	print("--------");
> 	print("'" + constantName + "' in global: " + this.hasOwnProperty(constantName))
> 	print(constantName + " = " + this[constantName])
> 	var flags = analyzeFlags(this, constantName);
> 	var ok = flags.dontEnum && !flags.readOnly && flags.dontDelete;
> 	print("OK: " + ok);
> 	print("--------");
> }
-
> function checkGlobalFunction(funcName, expectedLength) {
> 	print(funcName);
> 	print("--------");
> 	var f = this[funcName];
> 	print(".name: " + f.name);
>	var ok = (f.name === funcName);
> 	print(".length: " + f.length);
>	ok = ok && (f.length === expectedLength);
> 	print("'" + funcName + "' in global: " + this.hasOwnProperty(funcName))
> 	var flags = analyzeFlags(this, funcName);
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
> checkProtoFunc('Array', 'toString', 0);
< Array.prototype.toString
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Array.prototype.constructor == Array : true
< .name: toString
< .length: 0
< 'toString' in Array.prototype: true
< toString : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toString is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Array', 'toLocaleString', 0);
< Array.prototype.toLocaleString
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Array.prototype.constructor == Array : true
< .name: toLocaleString
< .length: 0
< 'toLocaleString' in Array.prototype: true
< toLocaleString : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toLocaleString is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Array', 'concat', 1);
< Array.prototype.concat
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Array.prototype.constructor == Array : true
< .name: concat
< .length: 1
< 'concat' in Array.prototype: true
< concat : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: concat is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Array', 'join', 1);
< Array.prototype.join
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Array.prototype.constructor == Array : true
< .name: join
< .length: 1
< 'join' in Array.prototype: true
< join : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: join is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Array', 'pop', 0);
< Array.prototype.pop
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Array.prototype.constructor == Array : true
< .name: pop
< .length: 0
< 'pop' in Array.prototype: true
< pop : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: pop is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Array', 'push', 1);
< Array.prototype.push
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Array.prototype.constructor == Array : true
< .name: push
< .length: 1
< 'push' in Array.prototype: true
< push : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: push is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Array', 'reverse', 0);
< Array.prototype.reverse
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Array.prototype.constructor == Array : true
< .name: reverse
< .length: 0
< 'reverse' in Array.prototype: true
< reverse : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: reverse is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Array', 'shift', 0);
< Array.prototype.shift
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Array.prototype.constructor == Array : true
< .name: shift
< .length: 0
< 'shift' in Array.prototype: true
< shift : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: shift is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Array', 'slice', 2);
< Array.prototype.slice
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Array.prototype.constructor == Array : true
< .name: slice
< .length: 2
< 'slice' in Array.prototype: true
< slice : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: slice is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Array', 'sort', 1);
< Array.prototype.sort
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Array.prototype.constructor == Array : true
< .name: sort
< .length: 1
< 'sort' in Array.prototype: true
< sort : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: sort is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Array', 'splice', 2);
< Array.prototype.splice
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Array.prototype.constructor == Array : true
< .name: splice
< .length: 2
< 'splice' in Array.prototype: true
< splice : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: splice is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Array', 'unshift', 1);
< Array.prototype.unshift
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Array.prototype.constructor == Array : true
< .name: unshift
< .length: 1
< 'unshift' in Array.prototype: true
< unshift : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: unshift is not a constructor
< OK: true
< --------
-
> checkFunc('Array', 'isArray', 1);
< Array.isArray
< --------
< .name: isArray
< .length: 1
< 'isArray' in Array: true
< isArray : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: isArray is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Function', 'toString', 0);
< Function.prototype.toString
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Function.prototype.constructor == Function : true
< .name: toString
< .length: 0
< 'toString' in Function.prototype: true
< toString : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toString is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Function', 'apply', 2);
< Function.prototype.apply
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Function.prototype.constructor == Function : true
< .name: apply
< .length: 2
< 'apply' in Function.prototype: true
< apply : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: apply is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Function', 'call', 1);
< Function.prototype.call
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Function.prototype.constructor == Function : true
< .name: call
< .length: 1
< 'call' in Function.prototype: true
< call : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: call is not a constructor
< OK: true
< --------
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Number.prototype.constructor == Number : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Number.prototype.constructor == Number : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Number.prototype.constructor == Number : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Number.prototype.constructor == Number : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Number.prototype.constructor == Number : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Number.prototype.constructor == Number : true
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
> checkProtoFunc('Object', 'toString', 0);
< Object.prototype.toString
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Object.prototype.constructor == Object : true
< .name: toString
< .length: 0
< 'toString' in Object.prototype: true
< toString : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toString is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Object', 'toLocaleString', 0);
< Object.prototype.toLocaleString
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Object.prototype.constructor == Object : true
< .name: toLocaleString
< .length: 0
< 'toLocaleString' in Object.prototype: true
< toLocaleString : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toLocaleString is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Object', 'valueOf', 0);
< Object.prototype.valueOf
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Object.prototype.constructor == Object : true
< .name: valueOf
< .length: 0
< 'valueOf' in Object.prototype: true
< valueOf : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: valueOf is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Object', 'hasOwnProperty', 1);
< Object.prototype.hasOwnProperty
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Object.prototype.constructor == Object : true
< .name: hasOwnProperty
< .length: 1
< 'hasOwnProperty' in Object.prototype: true
< hasOwnProperty : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: hasOwnProperty is not a constructor
< OK: true
< --------
-
> checkProtoFunc('Object', 'isPrototypeOf', 1);
< Object.prototype.isPrototypeOf
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< Object.prototype.constructor == Object : true
< .name: isPrototypeOf
< .length: 1
< 'isPrototypeOf' in Object.prototype: true
< isPrototypeOf : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: isPrototypeOf is not a constructor
< OK: true
< --------
-
> checkFunc('Object', 'defineProperty', 3);
< Object.defineProperty
< --------
< .name: defineProperty
< .length: 3
< 'defineProperty' in Object: true
< defineProperty : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: defineProperty is not a constructor
< OK: true
< --------
-
> checkFunc('Object', 'getPrototypeOf', 1);
< Object.getPrototypeOf
< --------
< .name: getPrototypeOf
< .length: 1
< 'getPrototypeOf' in Object: true
< getPrototypeOf : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: getPrototypeOf is not a constructor
< OK: true
< --------
-
> checkProtoFunc('RegExp', 'exec', 1);
< RegExp.prototype.exec
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< RegExp.prototype.constructor == RegExp : true
< .name: exec
< .length: 1
< 'exec' in RegExp.prototype: true
< exec : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: exec is not a constructor
< OK: true
< --------
-
> checkProtoFunc('RegExp', 'test', 1);
< RegExp.prototype.test
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< RegExp.prototype.constructor == RegExp : true
< .name: test
< .length: 1
< 'test' in RegExp.prototype: true
< test : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: test is not a constructor
< OK: true
< --------
-
> checkProtoFunc('RegExp', 'toString', 0);
< RegExp.prototype.toString
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< RegExp.prototype.constructor == RegExp : true
< .name: toString
< .length: 0
< 'toString' in RegExp.prototype: true
< toString : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toString is not a constructor
< OK: true
< --------
-
> checkProtoFunc('String', 'toString', 0);
< String.prototype.toString
< --------
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
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
< prototype : dontEnum,readOnly,dontDelete, (deleted:false)
< String.prototype.constructor == String : true
< .name: toLocaleUpperCase
< .length: 0
< 'toLocaleUpperCase' in String.prototype: true
< toLocaleUpperCase : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: toLocaleUpperCase is not a constructor
< OK: true
< --------
> checkGlobalConstant('NaN');
< NaN
< --------
< 'NaN' in global: true
< NaN = NaN
< NaN : dontEnum,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkGlobalConstant('Infinity');
< Infinity
< --------
< 'Infinity' in global: true
< Infinity = Infinity
< Infinity : dontEnum,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkGlobalConstant('undefined');
< undefined
< --------
< 'undefined' in global: true
< undefined = undefined
< undefined : dontEnum,dontDelete, (deleted:false)
< OK: true
< --------
-
> checkGlobalFunction('eval', 1);
< eval
< --------
< .name: eval
< .length: 1
< 'eval' in global: true
< eval : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: eval is not a constructor
< OK: true
< --------
-
> checkGlobalFunction('parseInt', 2);
< parseInt
< --------
< .name: parseInt
< .length: 2
< 'parseInt' in global: true
< parseInt : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: parseInt is not a constructor
< OK: true
< --------
-
> checkGlobalFunction('parseFloat', 1);
< parseFloat
< --------
< .name: parseFloat
< .length: 1
< 'parseFloat' in global: true
< parseFloat : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: parseFloat is not a constructor
< OK: true
< --------
-
> checkGlobalFunction('isNaN', 1);
< isNaN
< --------
< .name: isNaN
< .length: 1
< 'isNaN' in global: true
< isNaN : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: isNaN is not a constructor
< OK: true
< --------
-
> checkGlobalFunction('isFinite', 1);
< isFinite
< --------
< .name: isFinite
< .length: 1
< 'isFinite' in global: true
< isFinite : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: isFinite is not a constructor
< OK: true
< --------
-
> checkGlobalFunction('decodeURI', 1);
< decodeURI
< --------
< .name: decodeURI
< .length: 1
< 'decodeURI' in global: true
< decodeURI : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: decodeURI is not a constructor
< OK: true
< --------
-
> checkGlobalFunction('decodeURIComponent', 1);
< decodeURIComponent
< --------
< .name: decodeURIComponent
< .length: 1
< 'decodeURIComponent' in global: true
< decodeURIComponent : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: decodeURIComponent is not a constructor
< OK: true
< --------
-
> checkGlobalFunction('encodeURI', 1);
< encodeURI
< --------
< .name: encodeURI
< .length: 1
< 'encodeURI' in global: true
< encodeURI : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: encodeURI is not a constructor
< OK: true
< --------
-
> checkGlobalFunction('encodeURIComponent', 1);
< encodeURIComponent
< --------
< .name: encodeURIComponent
< .length: 1
< 'encodeURIComponent' in global: true
< encodeURIComponent : dontEnum, (deleted:true)
< name : dontEnum, (deleted:true)
< length : dontEnum,readOnly,dontDelete, (deleted:false)
< unconstructable: TypeError: encodeURIComponent is not a constructor
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
