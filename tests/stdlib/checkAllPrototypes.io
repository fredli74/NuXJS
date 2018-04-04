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
> 	print(className + ".prototype." + funcName);
> 	print("--------");
> 	var p = this[className].prototype;
> 	var f = p[funcName];
> 	print(".name: " + f.name);
>	var ok = (f.name === funcName);
> 	print(".length: " + f.length);
> 	print("'" + funcName + "' in " + className + ".prototype: " + (funcName in p))
> 	flags = analyzeFlags(p, funcName);
>	ok = ok && flags.dontEnum && !flags.readOnly && !flags.dontDelete;
> 	flags = analyzeFlags(f, 'name');
>	ok = ok && flags.dontEnum && !flags.readOnly && !flags.dontDelete;
> 	flags = analyzeFlags(f, 'length');
>	ok = ok && flags.dontEnum && flags.readOnly && flags.dontDelete;
>	try { var o = new f; print("constructable"); ok = false; } catch (x) { print("unconstructable: " + x); }
> 	print("OK: " + ok + "\n");
> }
-
> checkFunc('String', 'toString', 0);
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
< 
-
> checkFunc('String', 'valueOf', 0);
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
< 
-
> checkFunc('String', 'charAt', 1);
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
< 
-
> checkFunc('String', 'concat', 1);
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
< 
-
> checkFunc('String', 'indexOf', 1);
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
< 
-
> checkFunc('String', 'lastIndexOf', 1);
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
< 
-
> checkFunc('String', 'localeCompare', 1);
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
< 
-
> checkFunc('String', 'match', 1);
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
< 
-
> checkFunc('String', 'replace', 2);
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
< 
-
> checkFunc('String', 'search', 1);
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
< 
-
> checkFunc('String', 'slice', 2);
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
< 
-
> checkFunc('String', 'split', 2);
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
< 
-
> checkFunc('String', 'substring', 2);
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
< 
-
> checkFunc('String', 'toLowerCase', 0);
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
< 
-
> checkFunc('String', 'toLocaleLowerCase', 0);
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
< 
-
> checkFunc('String', 'toUpperCase', 0);
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
< 
-
> checkFunc('String', 'toLocaleUpperCase', 0);
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
< 
-
