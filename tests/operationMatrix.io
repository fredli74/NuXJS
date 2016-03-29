> (function(){
>     print("*** operation matrix ***");
>     
>     var valueStrings = [
>         'undefined', 'null', 'false', 'true', '0', '1', '-128', '1234.5678', '-1.0e+7', 'NaN', 'Infinity', '""', '"str"'
>         , '"-128"', '"1.0"', '"  +1234  "', '"string"', '"5tring"'
>         , '({valueOf:function(){return "-128"}})','({valueOf:function(){return null}})'
>         , 'new Boolean(true)', 'new Number(1234.5678)', 'new String("str")'
>     ];
>     
>     var values = [ ];
>     for (var i = 0; i < valueStrings.length; ++i) {
>         values[i] = eval(valueStrings[i]);
>     }
>     
>     var tests = [
>         "==", "!=", "===", "!==", "<", "<=", ">", ">=", "+", "-", "*", "/", "%", "&", "^", "|", "<<", ">>", ">>>"
>     ];
>     
>     for (var t = 0; t < tests.length; ++t) {
>         var test = tests[t];
>         print(test);
>         for (var i = 0; i < values.length; ++i) {
>             var a = values[i];
>             var results = "";
>             for (var j = 0; j < values.length; ++j) {
>                 var b = values[j];
>                 var r;
>                 try {
>                     r = eval('a' + test + 'b');
>                     if (typeof r === 'number') {
>                         r = Math.floor(r * 100000 + 0.5) / 100000;
>                     }
>                 }
>                 catch (e) {
>                     r = e;
>                 }
>                 if (results !== "") {
>                     results += " ";
>                 }
>                 results += r;
>             }
>             results = valueStrings[i] + ": " + results;
>             print(results);
>         }
>         print("");
>     }
> })();
< *** operation matrix ***
< ==
< undefined: true true false false false false false false false false false false false false false false false false false false false false false
< null: true true false false false false false false false false false false false false false false false false false false false false false
< false: false false true false true false false false false false false true false false false false false false false false false false false
< true: false false false true false true false false false false false false false false true false false false false false true false false
< 0: false false true false true false false false false false false true false false false false false false false false false false false
< 1: false false false true false true false false false false false false false false true false false false false false true false false
< -128: false false false false false false true false false false false false false true false false false false true false false false false
< 1234.5678: false false false false false false false true false false false false false false false false false false false false false true false
< -1.0e+7: false false false false false false false false true false false false false false false false false false false false false false false
< NaN: false false false false false false false false false false false false false false false false false false false false false false false
< Infinity: false false false false false false false false false false true false false false false false false false false false false false false
< "": false false true false true false false false false false false true false false false false false false false false false false false
< "str": false false false false false false false false false false false false true false false false false false false false false false true
< "-128": false false false false false false true false false false false false false true false false false false true false false false false
< "1.0": false false false true false true false false false false false false false false true false false false false false true false false
< "  +1234  ": false false false false false false false false false false false false false false false true false false false false false false false
< "string": false false false false false false false false false false false false false false false false true false false false false false false
< "5tring": false false false false false false false false false false false false false false false false false true false false false false false
< ({valueOf:function(){return "-128"}}): false false false false false false true false false false false false false true false false false false true false false false false
< ({valueOf:function(){return null}}): false false false false false false false false false false false false false false false false false false false true false false false
< new Boolean(true): false false false true false true false false false false false false false false true false false false false false true false false
< new Number(1234.5678): false false false false false false false true false false false false false false false false false false false false false true false
< new String("str"): false false false false false false false false false false false false true false false false false false false false false false true
< 
< !=
< undefined: false false true true true true true true true true true true true true true true true true true true true true true
< null: false false true true true true true true true true true true true true true true true true true true true true true
< false: true true false true false true true true true true true false true true true true true true true true true true true
< true: true true true false true false true true true true true true true true false true true true true true false true true
< 0: true true false true false true true true true true true false true true true true true true true true true true true
< 1: true true true false true false true true true true true true true true false true true true true true false true true
< -128: true true true true true true false true true true true true true false true true true true false true true true true
< 1234.5678: true true true true true true true false true true true true true true true true true true true true true false true
< -1.0e+7: true true true true true true true true false true true true true true true true true true true true true true true
< NaN: true true true true true true true true true true true true true true true true true true true true true true true
< Infinity: true true true true true true true true true true false true true true true true true true true true true true true
< "": true true false true false true true true true true true false true true true true true true true true true true true
< "str": true true true true true true true true true true true true false true true true true true true true true true false
< "-128": true true true true true true false true true true true true true false true true true true false true true true true
< "1.0": true true true false true false true true true true true true true true false true true true true true false true true
< "  +1234  ": true true true true true true true true true true true true true true true false true true true true true true true
< "string": true true true true true true true true true true true true true true true true false true true true true true true
< "5tring": true true true true true true true true true true true true true true true true true false true true true true true
< ({valueOf:function(){return "-128"}}): true true true true true true false true true true true true true false true true true true false true true true true
< ({valueOf:function(){return null}}): true true true true true true true true true true true true true true true true true true true false true true true
< new Boolean(true): true true true false true false true true true true true true true true false true true true true true false true true
< new Number(1234.5678): true true true true true true true false true true true true true true true true true true true true true false true
< new String("str"): true true true true true true true true true true true true false true true true true true true true true true false
< 
< ===
< undefined: true false false false false false false false false false false false false false false false false false false false false false false
< null: false true false false false false false false false false false false false false false false false false false false false false false
< false: false false true false false false false false false false false false false false false false false false false false false false false
< true: false false false true false false false false false false false false false false false false false false false false false false false
< 0: false false false false true false false false false false false false false false false false false false false false false false false
< 1: false false false false false true false false false false false false false false false false false false false false false false false
< -128: false false false false false false true false false false false false false false false false false false false false false false false
< 1234.5678: false false false false false false false true false false false false false false false false false false false false false false false
< -1.0e+7: false false false false false false false false true false false false false false false false false false false false false false false
< NaN: false false false false false false false false false false false false false false false false false false false false false false false
< Infinity: false false false false false false false false false false true false false false false false false false false false false false false
< "": false false false false false false false false false false false true false false false false false false false false false false false
< "str": false false false false false false false false false false false false true false false false false false false false false false false
< "-128": false false false false false false false false false false false false false true false false false false false false false false false
< "1.0": false false false false false false false false false false false false false false true false false false false false false false false
< "  +1234  ": false false false false false false false false false false false false false false false true false false false false false false false
< "string": false false false false false false false false false false false false false false false false true false false false false false false
< "5tring": false false false false false false false false false false false false false false false false false true false false false false false
< ({valueOf:function(){return "-128"}}): false false false false false false false false false false false false false false false false false false true false false false false
< ({valueOf:function(){return null}}): false false false false false false false false false false false false false false false false false false false true false false false
< new Boolean(true): false false false false false false false false false false false false false false false false false false false false true false false
< new Number(1234.5678): false false false false false false false false false false false false false false false false false false false false false true false
< new String("str"): false false false false false false false false false false false false false false false false false false false false false false true
< 
< !==
< undefined: false true true true true true true true true true true true true true true true true true true true true true true
< null: true false true true true true true true true true true true true true true true true true true true true true true
< false: true true false true true true true true true true true true true true true true true true true true true true true
< true: true true true false true true true true true true true true true true true true true true true true true true true
< 0: true true true true false true true true true true true true true true true true true true true true true true true
< 1: true true true true true false true true true true true true true true true true true true true true true true true
< -128: true true true true true true false true true true true true true true true true true true true true true true true
< 1234.5678: true true true true true true true false true true true true true true true true true true true true true true true
< -1.0e+7: true true true true true true true true false true true true true true true true true true true true true true true
< NaN: true true true true true true true true true true true true true true true true true true true true true true true
< Infinity: true true true true true true true true true true false true true true true true true true true true true true true
< "": true true true true true true true true true true true false true true true true true true true true true true true
< "str": true true true true true true true true true true true true false true true true true true true true true true true
< "-128": true true true true true true true true true true true true true false true true true true true true true true true
< "1.0": true true true true true true true true true true true true true true false true true true true true true true true
< "  +1234  ": true true true true true true true true true true true true true true true false true true true true true true true
< "string": true true true true true true true true true true true true true true true true false true true true true true true
< "5tring": true true true true true true true true true true true true true true true true true false true true true true true
< ({valueOf:function(){return "-128"}}): true true true true true true true true true true true true true true true true true true false true true true true
< ({valueOf:function(){return null}}): true true true true true true true true true true true true true true true true true true true false true true true
< new Boolean(true): true true true true true true true true true true true true true true true true true true true true false true true
< new Number(1234.5678): true true true true true true true true true true true true true true true true true true true true true false true
< new String("str"): true true true true true true true true true true true true true true true true true true true true true true false
< 
< <
< undefined: false false false false false false false false false false false false false false false false false false false false false false false
< null: false false false true false true false true false false true false false false true true false false false false true true false
< false: false false false true false true false true false false true false false false true true false false false false true true false
< true: false false false false false false false true false false true false false false false true false false false false false true false
< 0: false false false true false true false true false false true false false false true true false false false false true true false
< 1: false false false false false false false true false false true false false false false true false false false false false true false
< -128: false true true true true true false true false false true true false false true true false false false true true true false
< 1234.5678: false false false false false false false false false false true false false false false false false false false false false false false
< -1.0e+7: false true true true true true true true false false true true false true true true false false true true true true false
< NaN: false false false false false false false false false false false false false false false false false false false false false false false
< Infinity: false false false false false false false false false false false false false false false false false false false false false false false
< "": false false false true false true false true false false true false true true true true true true true false true true true
< "str": false false false false false false false false false false false false false false false false true false false false false false false
< "-128": false true true true true true false true false false true false true false true false true true false true true true true
< "1.0": false false false false false false false true false false true false true false false false true true false false false true true
< "  +1234  ": false false false false false false false true false false true false true true true false true true true false false true true
< "string": false false false false false false false false false false false false false false false false false false false false false false false
< "5tring": false false false false false false false false false false false false true false false false true false false false false false true
< ({valueOf:function(){return "-128"}}): false true true true true true false true false false true false true false true false true true false true true true true
< ({valueOf:function(){return null}}): false false false true false true false true false false true false false false true true false false false false true true false
< new Boolean(true): false false false false false false false true false false true false false false false true false false false false false true false
< new Number(1234.5678): false false false false false false false false false false true false false false false false false false false false false false false
< new String("str"): false false false false false false false false false false false false false false false false true false false false false false false
< 
< <=
< undefined: false false false false false false false false false false false false false false false false false false false false false false false
< null: false true true true true true false true false false true true false false true true false false false true true true false
< false: false true true true true true false true false false true true false false true true false false false true true true false
< true: false false false true false true false true false false true false false false true true false false false false true true false
< 0: false true true true true true false true false false true true false false true true false false false true true true false
< 1: false false false true false true false true false false true false false false true true false false false false true true false
< -128: false true true true true true true true false false true true false true true true false false true true true true false
< 1234.5678: false false false false false false false true false false true false false false false false false false false false false true false
< -1.0e+7: false true true true true true true true true false true true false true true true false false true true true true false
< NaN: false false false false false false false false false false false false false false false false false false false false false false false
< Infinity: false false false false false false false false false false true false false false false false false false false false false false false
< "": false true true true true true false true false false true true true true true true true true true true true true true
< "str": false false false false false false false false false false false false true false false false true false false false false false true
< "-128": false true true true true true true true false false true false true true true false true true true true true true true
< "1.0": false false false true false true false true false false true false true false true false true true false false true true true
< "  +1234  ": false false false false false false false true false false true false true true true true true true true false false true true
< "string": false false false false false false false false false false false false false false false false true false false false false false false
< "5tring": false false false false false false false false false false false false true false false false true true false false false false true
< ({valueOf:function(){return "-128"}}): false true true true true true true true false false true false true true true false true true true true true true true
< ({valueOf:function(){return null}}): false true true true true true false true false false true true false false true true false false false true true true false
< new Boolean(true): false false false true false true false true false false true false false false true true false false false false true true false
< new Number(1234.5678): false false false false false false false true false false true false false false false false false false false false false true false
< new String("str"): false false false false false false false false false false false false true false false false true false false false false false true
< 
< >
< undefined: false false false false false false false false false false false false false false false false false false false false false false false
< null: false false false false false false true false true false false false false true false false false false true false false false false
< false: false false false false false false true false true false false false false true false false false false true false false false false
< true: false true true false true false true false true false false true false true false false false false true true false false false
< 0: false false false false false false true false true false false false false true false false false false true false false false false
< 1: false true true false true false true false true false false true false true false false false false true true false false false
< -128: false false false false false false false false true false false false false false false false false false false false false false false
< 1234.5678: false true true true true true true false true false false true false true true true false false true true true false false
< -1.0e+7: false false false false false false false false false false false false false false false false false false false false false false false
< NaN: false false false false false false false false false false false false false false false false false false false false false false false
< Infinity: false true true true true true true true true false false true false true true true false false true true true true false
< "": false false false false false false true false true false false false false false false false false false false false false false false
< "str": false false false false false false false false false false false true false true true true false true true false false false false
< "-128": false false false false false false false false true false false true false false false true false false false false false false false
< "1.0": false true true false true false true false true false false true false true false true false false true true false false false
< "  +1234  ": false true true true true true true false true false false true false false false false false false false true true false false
< "string": false false false false false false false false false false false true true true true true false true true false false false true
< "5tring": false false false false false false false false false false false true false true true true false false true false false false false
< ({valueOf:function(){return "-128"}}): false false false false false false false false true false false true false false false true false false false false false false false
< ({valueOf:function(){return null}}): false false false false false false true false true false false false false true false false false false true false false false false
< new Boolean(true): false true true false true false true false true false false true false true false false false false true true false false false
< new Number(1234.5678): false true true true true true true false true false false true false true true true false false true true true false false
< new String("str"): false false false false false false false false false false false true false true true true false true true false false false false
< 
< >=
< undefined: false false false false false false false false false false false false false false false false false false false false false false false
< null: false true true false true false true false true false false true false true false false false false true true false false false
< false: false true true false true false true false true false false true false true false false false false true true false false false
< true: false true true true true true true false true false false true false true true false false false true true true false false
< 0: false true true false true false true false true false false true false true false false false false true true false false false
< 1: false true true true true true true false true false false true false true true false false false true true true false false
< -128: false false false false false false true false true false false false false true false false false false true false false false false
< 1234.5678: false true true true true true true true true false false true false true true true false false true true true true false
< -1.0e+7: false false false false false false false false true false false false false false false false false false false false false false false
< NaN: false false false false false false false false false false false false false false false false false false false false false false false
< Infinity: false true true true true true true true true false true true false true true true false false true true true true false
< "": false true true false true false true false true false false true false false false false false false false true false false false
< "str": false false false false false false false false false false false true true true true true false true true false false false true
< "-128": false false false false false false true false true false false true false true false true false false true false false false false
< "1.0": false true true true true true true false true false false true false true true true false false true true true false false
< "  +1234  ": false true true true true true true false true false false true false false false true false false false true true false false
< "string": false false false false false false false false false false false true true true true true true true true false false false true
< "5tring": false false false false false false false false false false false true false true true true false true true false false false false
< ({valueOf:function(){return "-128"}}): false false false false false false true false true false false true false true false true false false true false false false false
< ({valueOf:function(){return null}}): false true true false true false true false true false false true false true false false false false true true false false false
< new Boolean(true): false true true true true true true false true false false true false true true false false false true true true false false
< new Number(1234.5678): false true true true true true true true true false false true false true true true false false true true true true false
< new String("str"): false false false false false false false false false false false true true true true true false true true false false false true
< 
< +
< undefined: NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN undefined undefinedstr undefined-128 undefined1.0 undefined  +1234   undefinedstring undefined5tring undefined-128 NaN NaN NaN undefinedstr
< null: NaN 0 0 1 0 1 -128 1234.5678 -10000000 NaN Infinity null nullstr null-128 null1.0 null  +1234   nullstring null5tring null-128 0 1 1234.5678 nullstr
< false: NaN 0 0 1 0 1 -128 1234.5678 -10000000 NaN Infinity false falsestr false-128 false1.0 false  +1234   falsestring false5tring false-128 0 1 1234.5678 falsestr
< true: NaN 1 1 2 1 2 -127 1235.5678 -9999999 NaN Infinity true truestr true-128 true1.0 true  +1234   truestring true5tring true-128 1 2 1235.5678 truestr
< 0: NaN 0 0 1 0 1 -128 1234.5678 -10000000 NaN Infinity 0 0str 0-128 01.0 0  +1234   0string 05tring 0-128 0 1 1234.5678 0str
< 1: NaN 1 1 2 1 2 -127 1235.5678 -9999999 NaN Infinity 1 1str 1-128 11.0 1  +1234   1string 15tring 1-128 1 2 1235.5678 1str
< -128: NaN -128 -128 -127 -128 -127 -256 1106.5678 -10000128 NaN Infinity -128 -128str -128-128 -1281.0 -128  +1234   -128string -1285tring -128-128 -128 -127 1106.5678 -128str
< 1234.5678: NaN 1234.5678 1234.5678 1235.5678 1234.5678 1235.5678 1106.5678 2469.1356 -9998765.4322 NaN Infinity 1234.5678 1234.5678str 1234.5678-128 1234.56781.0 1234.5678  +1234   1234.5678string 1234.56785tring 1234.5678-128 1234.5678 1235.5678 2469.1356 1234.5678str
< -1.0e+7: NaN -10000000 -10000000 -9999999 -10000000 -9999999 -10000128 -9998765.4322 -20000000 NaN Infinity -10000000 -10000000str -10000000-128 -100000001.0 -10000000  +1234   -10000000string -100000005tring -10000000-128 -10000000 -9999999 -9998765.4322 -10000000str
< NaN: NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaNstr NaN-128 NaN1.0 NaN  +1234   NaNstring NaN5tring NaN-128 NaN NaN NaN NaNstr
< Infinity: NaN Infinity Infinity Infinity Infinity Infinity Infinity Infinity Infinity NaN Infinity Infinity Infinitystr Infinity-128 Infinity1.0 Infinity  +1234   Infinitystring Infinity5tring Infinity-128 Infinity Infinity Infinity Infinitystr
< "": undefined null false true 0 1 -128 1234.5678 -10000000 NaN Infinity  str -128 1.0   +1234   string 5tring -128 null true 1234.5678 str
< "str": strundefined strnull strfalse strtrue str0 str1 str-128 str1234.5678 str-10000000 strNaN strInfinity str strstr str-128 str1.0 str  +1234   strstring str5tring str-128 strnull strtrue str1234.5678 strstr
< "-128": -128undefined -128null -128false -128true -1280 -1281 -128-128 -1281234.5678 -128-10000000 -128NaN -128Infinity -128 -128str -128-128 -1281.0 -128  +1234   -128string -1285tring -128-128 -128null -128true -1281234.5678 -128str
< "1.0": 1.0undefined 1.0null 1.0false 1.0true 1.00 1.01 1.0-128 1.01234.5678 1.0-10000000 1.0NaN 1.0Infinity 1.0 1.0str 1.0-128 1.01.0 1.0  +1234   1.0string 1.05tring 1.0-128 1.0null 1.0true 1.01234.5678 1.0str
< "  +1234  ":   +1234  undefined   +1234  null   +1234  false   +1234  true   +1234  0   +1234  1   +1234  -128   +1234  1234.5678   +1234  -10000000   +1234  NaN   +1234  Infinity   +1234     +1234  str   +1234  -128   +1234  1.0   +1234    +1234     +1234  string   +1234  5tring   +1234  -128   +1234  null   +1234  true   +1234  1234.5678   +1234  str
< "string": stringundefined stringnull stringfalse stringtrue string0 string1 string-128 string1234.5678 string-10000000 stringNaN stringInfinity string stringstr string-128 string1.0 string  +1234   stringstring string5tring string-128 stringnull stringtrue string1234.5678 stringstr
< "5tring": 5tringundefined 5tringnull 5tringfalse 5tringtrue 5tring0 5tring1 5tring-128 5tring1234.5678 5tring-10000000 5tringNaN 5tringInfinity 5tring 5tringstr 5tring-128 5tring1.0 5tring  +1234   5tringstring 5tring5tring 5tring-128 5tringnull 5tringtrue 5tring1234.5678 5tringstr
< ({valueOf:function(){return "-128"}}): -128undefined -128null -128false -128true -1280 -1281 -128-128 -1281234.5678 -128-10000000 -128NaN -128Infinity -128 -128str -128-128 -1281.0 -128  +1234   -128string -1285tring -128-128 -128null -128true -1281234.5678 -128str
< ({valueOf:function(){return null}}): NaN 0 0 1 0 1 -128 1234.5678 -10000000 NaN Infinity null nullstr null-128 null1.0 null  +1234   nullstring null5tring null-128 0 1 1234.5678 nullstr
< new Boolean(true): NaN 1 1 2 1 2 -127 1235.5678 -9999999 NaN Infinity true truestr true-128 true1.0 true  +1234   truestring true5tring true-128 1 2 1235.5678 truestr
< new Number(1234.5678): NaN 1234.5678 1234.5678 1235.5678 1234.5678 1235.5678 1106.5678 2469.1356 -9998765.4322 NaN Infinity 1234.5678 1234.5678str 1234.5678-128 1234.56781.0 1234.5678  +1234   1234.5678string 1234.56785tring 1234.5678-128 1234.5678 1235.5678 2469.1356 1234.5678str
< new String("str"): strundefined strnull strfalse strtrue str0 str1 str-128 str1234.5678 str-10000000 strNaN strInfinity str strstr str-128 str1.0 str  +1234   strstring str5tring str-128 strnull strtrue str1234.5678 strstr
< 
< -
< undefined: NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< null: NaN 0 0 -1 0 -1 128 -1234.5678 10000000 NaN -Infinity 0 NaN 128 -1 -1234 NaN NaN 128 0 -1 -1234.5678 NaN
< false: NaN 0 0 -1 0 -1 128 -1234.5678 10000000 NaN -Infinity 0 NaN 128 -1 -1234 NaN NaN 128 0 -1 -1234.5678 NaN
< true: NaN 1 1 0 1 0 129 -1233.5678 10000001 NaN -Infinity 1 NaN 129 0 -1233 NaN NaN 129 1 0 -1233.5678 NaN
< 0: NaN 0 0 -1 0 -1 128 -1234.5678 10000000 NaN -Infinity 0 NaN 128 -1 -1234 NaN NaN 128 0 -1 -1234.5678 NaN
< 1: NaN 1 1 0 1 0 129 -1233.5678 10000001 NaN -Infinity 1 NaN 129 0 -1233 NaN NaN 129 1 0 -1233.5678 NaN
< -128: NaN -128 -128 -129 -128 -129 0 -1362.5678 9999872 NaN -Infinity -128 NaN 0 -129 -1362 NaN NaN 0 -128 -129 -1362.5678 NaN
< 1234.5678: NaN 1234.5678 1234.5678 1233.5678 1234.5678 1233.5678 1362.5678 0 10001234.5678 NaN -Infinity 1234.5678 NaN 1362.5678 1233.5678 0.5678 NaN NaN 1362.5678 1234.5678 1233.5678 0 NaN
< -1.0e+7: NaN -10000000 -10000000 -10000001 -10000000 -10000001 -9999872 -10001234.5678 0 NaN -Infinity -10000000 NaN -9999872 -10000001 -10001234 NaN NaN -9999872 -10000000 -10000001 -10001234.5678 NaN
< NaN: NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< Infinity: NaN Infinity Infinity Infinity Infinity Infinity Infinity Infinity Infinity NaN NaN Infinity NaN Infinity Infinity Infinity NaN NaN Infinity Infinity Infinity Infinity NaN
< "": NaN 0 0 -1 0 -1 128 -1234.5678 10000000 NaN -Infinity 0 NaN 128 -1 -1234 NaN NaN 128 0 -1 -1234.5678 NaN
< "str": NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< "-128": NaN -128 -128 -129 -128 -129 0 -1362.5678 9999872 NaN -Infinity -128 NaN 0 -129 -1362 NaN NaN 0 -128 -129 -1362.5678 NaN
< "1.0": NaN 1 1 0 1 0 129 -1233.5678 10000001 NaN -Infinity 1 NaN 129 0 -1233 NaN NaN 129 1 0 -1233.5678 NaN
< "  +1234  ": NaN 1234 1234 1233 1234 1233 1362 -0.5678 10001234 NaN -Infinity 1234 NaN 1362 1233 0 NaN NaN 1362 1234 1233 -0.5678 NaN
< "string": NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< "5tring": NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< ({valueOf:function(){return "-128"}}): NaN -128 -128 -129 -128 -129 0 -1362.5678 9999872 NaN -Infinity -128 NaN 0 -129 -1362 NaN NaN 0 -128 -129 -1362.5678 NaN
< ({valueOf:function(){return null}}): NaN 0 0 -1 0 -1 128 -1234.5678 10000000 NaN -Infinity 0 NaN 128 -1 -1234 NaN NaN 128 0 -1 -1234.5678 NaN
< new Boolean(true): NaN 1 1 0 1 0 129 -1233.5678 10000001 NaN -Infinity 1 NaN 129 0 -1233 NaN NaN 129 1 0 -1233.5678 NaN
< new Number(1234.5678): NaN 1234.5678 1234.5678 1233.5678 1234.5678 1233.5678 1362.5678 0 10001234.5678 NaN -Infinity 1234.5678 NaN 1362.5678 1233.5678 0.5678 NaN NaN 1362.5678 1234.5678 1233.5678 0 NaN
< new String("str"): NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< 
< *
< undefined: NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< null: NaN 0 0 0 0 0 0 0 0 NaN NaN 0 NaN 0 0 0 NaN NaN 0 0 0 0 NaN
< false: NaN 0 0 0 0 0 0 0 0 NaN NaN 0 NaN 0 0 0 NaN NaN 0 0 0 0 NaN
< true: NaN 0 0 1 0 1 -128 1234.5678 -10000000 NaN Infinity 0 NaN -128 1 1234 NaN NaN -128 0 1 1234.5678 NaN
< 0: NaN 0 0 0 0 0 0 0 0 NaN NaN 0 NaN 0 0 0 NaN NaN 0 0 0 0 NaN
< 1: NaN 0 0 1 0 1 -128 1234.5678 -10000000 NaN Infinity 0 NaN -128 1 1234 NaN NaN -128 0 1 1234.5678 NaN
< -128: NaN 0 0 -128 0 -128 16384 -158024.6784 1280000000 NaN -Infinity 0 NaN 16384 -128 -157952 NaN NaN 16384 0 -128 -158024.6784 NaN
< 1234.5678: NaN 0 0 1234.5678 0 1234.5678 -158024.6784 1524157.6528 -12345678000 NaN Infinity 0 NaN -158024.6784 1234.5678 1523456.6652 NaN NaN -158024.6784 0 1234.5678 1524157.6528 NaN
< -1.0e+7: NaN 0 0 -10000000 0 -10000000 1280000000 -12345678000 100000000000000 NaN -Infinity 0 NaN 1280000000 -10000000 -12340000000 NaN NaN 1280000000 0 -10000000 -12345678000 NaN
< NaN: NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< Infinity: NaN NaN NaN Infinity NaN Infinity -Infinity Infinity -Infinity NaN Infinity NaN NaN -Infinity Infinity Infinity NaN NaN -Infinity NaN Infinity Infinity NaN
< "": NaN 0 0 0 0 0 0 0 0 NaN NaN 0 NaN 0 0 0 NaN NaN 0 0 0 0 NaN
< "str": NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< "-128": NaN 0 0 -128 0 -128 16384 -158024.6784 1280000000 NaN -Infinity 0 NaN 16384 -128 -157952 NaN NaN 16384 0 -128 -158024.6784 NaN
< "1.0": NaN 0 0 1 0 1 -128 1234.5678 -10000000 NaN Infinity 0 NaN -128 1 1234 NaN NaN -128 0 1 1234.5678 NaN
< "  +1234  ": NaN 0 0 1234 0 1234 -157952 1523456.6652 -12340000000 NaN Infinity 0 NaN -157952 1234 1522756 NaN NaN -157952 0 1234 1523456.6652 NaN
< "string": NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< "5tring": NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< ({valueOf:function(){return "-128"}}): NaN 0 0 -128 0 -128 16384 -158024.6784 1280000000 NaN -Infinity 0 NaN 16384 -128 -157952 NaN NaN 16384 0 -128 -158024.6784 NaN
< ({valueOf:function(){return null}}): NaN 0 0 0 0 0 0 0 0 NaN NaN 0 NaN 0 0 0 NaN NaN 0 0 0 0 NaN
< new Boolean(true): NaN 0 0 1 0 1 -128 1234.5678 -10000000 NaN Infinity 0 NaN -128 1 1234 NaN NaN -128 0 1 1234.5678 NaN
< new Number(1234.5678): NaN 0 0 1234.5678 0 1234.5678 -158024.6784 1524157.6528 -12345678000 NaN Infinity 0 NaN -158024.6784 1234.5678 1523456.6652 NaN NaN -158024.6784 0 1234.5678 1524157.6528 NaN
< new String("str"): NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< 
< /
< undefined: NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< null: NaN NaN NaN 0 NaN 0 0 0 0 NaN 0 NaN NaN 0 0 0 NaN NaN 0 NaN 0 0 NaN
< false: NaN NaN NaN 0 NaN 0 0 0 0 NaN 0 NaN NaN 0 0 0 NaN NaN 0 NaN 0 0 NaN
< true: NaN Infinity Infinity 1 Infinity 1 -0.00781 0.00081 0 NaN 0 Infinity NaN -0.00781 1 0.00081 NaN NaN -0.00781 Infinity 1 0.00081 NaN
< 0: NaN NaN NaN 0 NaN 0 0 0 0 NaN 0 NaN NaN 0 0 0 NaN NaN 0 NaN 0 0 NaN
< 1: NaN Infinity Infinity 1 Infinity 1 -0.00781 0.00081 0 NaN 0 Infinity NaN -0.00781 1 0.00081 NaN NaN -0.00781 Infinity 1 0.00081 NaN
< -128: NaN -Infinity -Infinity -128 -Infinity -128 1 -0.10368 0.00001 NaN 0 -Infinity NaN 1 -128 -0.10373 NaN NaN 1 -Infinity -128 -0.10368 NaN
< 1234.5678: NaN Infinity Infinity 1234.5678 Infinity 1234.5678 -9.64506 1 -0.00012 NaN 0 Infinity NaN -9.64506 1234.5678 1.00046 NaN NaN -9.64506 Infinity 1234.5678 1 NaN
< -1.0e+7: NaN -Infinity -Infinity -10000000 -Infinity -10000000 78125 -8100.00066 1 NaN 0 -Infinity NaN 78125 -10000000 -8103.72771 NaN NaN 78125 -Infinity -10000000 -8100.00066 NaN
< NaN: NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< Infinity: NaN Infinity Infinity Infinity Infinity Infinity -Infinity Infinity -Infinity NaN NaN Infinity NaN -Infinity Infinity Infinity NaN NaN -Infinity Infinity Infinity Infinity NaN
< "": NaN NaN NaN 0 NaN 0 0 0 0 NaN 0 NaN NaN 0 0 0 NaN NaN 0 NaN 0 0 NaN
< "str": NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< "-128": NaN -Infinity -Infinity -128 -Infinity -128 1 -0.10368 0.00001 NaN 0 -Infinity NaN 1 -128 -0.10373 NaN NaN 1 -Infinity -128 -0.10368 NaN
< "1.0": NaN Infinity Infinity 1 Infinity 1 -0.00781 0.00081 0 NaN 0 Infinity NaN -0.00781 1 0.00081 NaN NaN -0.00781 Infinity 1 0.00081 NaN
< "  +1234  ": NaN Infinity Infinity 1234 Infinity 1234 -9.64062 0.99954 -0.00012 NaN 0 Infinity NaN -9.64062 1234 1 NaN NaN -9.64062 Infinity 1234 0.99954 NaN
< "string": NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< "5tring": NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< ({valueOf:function(){return "-128"}}): NaN -Infinity -Infinity -128 -Infinity -128 1 -0.10368 0.00001 NaN 0 -Infinity NaN 1 -128 -0.10373 NaN NaN 1 -Infinity -128 -0.10368 NaN
< ({valueOf:function(){return null}}): NaN NaN NaN 0 NaN 0 0 0 0 NaN 0 NaN NaN 0 0 0 NaN NaN 0 NaN 0 0 NaN
< new Boolean(true): NaN Infinity Infinity 1 Infinity 1 -0.00781 0.00081 0 NaN 0 Infinity NaN -0.00781 1 0.00081 NaN NaN -0.00781 Infinity 1 0.00081 NaN
< new Number(1234.5678): NaN Infinity Infinity 1234.5678 Infinity 1234.5678 -9.64506 1 -0.00012 NaN 0 Infinity NaN -9.64506 1234.5678 1.00046 NaN NaN -9.64506 Infinity 1234.5678 1 NaN
< new String("str"): NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< 
< %
< undefined: NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< null: NaN NaN NaN 0 NaN 0 0 0 0 NaN 0 NaN NaN 0 0 0 NaN NaN 0 NaN 0 0 NaN
< false: NaN NaN NaN 0 NaN 0 0 0 0 NaN 0 NaN NaN 0 0 0 NaN NaN 0 NaN 0 0 NaN
< true: NaN NaN NaN 0 NaN 0 1 1 1 NaN 1 NaN NaN 1 0 1 NaN NaN 1 NaN 0 1 NaN
< 0: NaN NaN NaN 0 NaN 0 0 0 0 NaN 0 NaN NaN 0 0 0 NaN NaN 0 NaN 0 0 NaN
< 1: NaN NaN NaN 0 NaN 0 1 1 1 NaN 1 NaN NaN 1 0 1 NaN NaN 1 NaN 0 1 NaN
< -128: NaN NaN NaN 0 NaN 0 0 -128 -128 NaN -128 NaN NaN 0 0 -128 NaN NaN 0 NaN 0 -128 NaN
< 1234.5678: NaN NaN NaN 0.5678 NaN 0.5678 82.5678 0 1234.5678 NaN 1234.5678 NaN NaN 82.5678 0.5678 0.5678 NaN NaN 82.5678 NaN 0.5678 0 NaN
< -1.0e+7: NaN NaN NaN 0 NaN 0 0 -0.82 0 NaN -10000000 NaN NaN 0 0 -898 NaN NaN 0 NaN 0 -0.82 NaN
< NaN: NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< Infinity: NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< "": NaN NaN NaN 0 NaN 0 0 0 0 NaN 0 NaN NaN 0 0 0 NaN NaN 0 NaN 0 0 NaN
< "str": NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< "-128": NaN NaN NaN 0 NaN 0 0 -128 -128 NaN -128 NaN NaN 0 0 -128 NaN NaN 0 NaN 0 -128 NaN
< "1.0": NaN NaN NaN 0 NaN 0 1 1 1 NaN 1 NaN NaN 1 0 1 NaN NaN 1 NaN 0 1 NaN
< "  +1234  ": NaN NaN NaN 0 NaN 0 82 1234 1234 NaN 1234 NaN NaN 82 0 0 NaN NaN 82 NaN 0 1234 NaN
< "string": NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< "5tring": NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< ({valueOf:function(){return "-128"}}): NaN NaN NaN 0 NaN 0 0 -128 -128 NaN -128 NaN NaN 0 0 -128 NaN NaN 0 NaN 0 -128 NaN
< ({valueOf:function(){return null}}): NaN NaN NaN 0 NaN 0 0 0 0 NaN 0 NaN NaN 0 0 0 NaN NaN 0 NaN 0 0 NaN
< new Boolean(true): NaN NaN NaN 0 NaN 0 1 1 1 NaN 1 NaN NaN 1 0 1 NaN NaN 1 NaN 0 1 NaN
< new Number(1234.5678): NaN NaN NaN 0.5678 NaN 0.5678 82.5678 0 1234.5678 NaN 1234.5678 NaN NaN 82.5678 0.5678 0.5678 NaN NaN 82.5678 NaN 0.5678 0 NaN
< new String("str"): NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN
< 
< &
< undefined: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< null: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< false: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< true: 0 0 0 1 0 1 0 0 0 0 0 0 0 0 1 0 0 0 0 0 1 0 0
< 0: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< 1: 0 0 0 1 0 1 0 0 0 0 0 0 0 0 1 0 0 0 0 0 1 0 0
< -128: 0 0 0 0 0 0 -128 1152 -10000000 0 0 0 0 -128 0 1152 0 0 -128 0 0 1152 0
< 1234.5678: 0 0 0 0 0 0 1152 1234 128 0 0 0 0 1152 0 1234 0 0 1152 0 0 1234 0
< -1.0e+7: 0 0 0 0 0 0 -10000000 128 -10000000 0 0 0 0 -10000000 0 128 0 0 -10000000 0 0 128 0
< NaN: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< Infinity: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "str": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "-128": 0 0 0 0 0 0 -128 1152 -10000000 0 0 0 0 -128 0 1152 0 0 -128 0 0 1152 0
< "1.0": 0 0 0 1 0 1 0 0 0 0 0 0 0 0 1 0 0 0 0 0 1 0 0
< "  +1234  ": 0 0 0 0 0 0 1152 1234 128 0 0 0 0 1152 0 1234 0 0 1152 0 0 1234 0
< "string": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "5tring": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< ({valueOf:function(){return "-128"}}): 0 0 0 0 0 0 -128 1152 -10000000 0 0 0 0 -128 0 1152 0 0 -128 0 0 1152 0
< ({valueOf:function(){return null}}): 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< new Boolean(true): 0 0 0 1 0 1 0 0 0 0 0 0 0 0 1 0 0 0 0 0 1 0 0
< new Number(1234.5678): 0 0 0 0 0 0 1152 1234 128 0 0 0 0 1152 0 1234 0 0 1152 0 0 1234 0
< new String("str"): 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< 
< ^
< undefined: 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< null: 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< false: 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< true: 1 1 1 0 1 0 -127 1235 -9999999 1 1 1 1 -127 0 1235 1 1 -127 1 0 1235 1
< 0: 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< 1: 1 1 1 0 1 0 -127 1235 -9999999 1 1 1 1 -127 0 1235 1 1 -127 1 0 1235 1
< -128: -128 -128 -128 -127 -128 -127 0 -1198 9999872 -128 -128 -128 -128 0 -127 -1198 -128 -128 0 -128 -127 -1198 -128
< 1234.5678: 1234 1234 1234 1235 1234 1235 -1198 0 -9999022 1234 1234 1234 1234 -1198 1235 0 1234 1234 -1198 1234 1235 0 1234
< -1.0e+7: -10000000 -10000000 -10000000 -9999999 -10000000 -9999999 9999872 -9999022 0 -10000000 -10000000 -10000000 -10000000 9999872 -9999999 -9999022 -10000000 -10000000 9999872 -10000000 -9999999 -9999022 -10000000
< NaN: 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< Infinity: 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< "": 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< "str": 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< "-128": -128 -128 -128 -127 -128 -127 0 -1198 9999872 -128 -128 -128 -128 0 -127 -1198 -128 -128 0 -128 -127 -1198 -128
< "1.0": 1 1 1 0 1 0 -127 1235 -9999999 1 1 1 1 -127 0 1235 1 1 -127 1 0 1235 1
< "  +1234  ": 1234 1234 1234 1235 1234 1235 -1198 0 -9999022 1234 1234 1234 1234 -1198 1235 0 1234 1234 -1198 1234 1235 0 1234
< "string": 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< "5tring": 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< ({valueOf:function(){return "-128"}}): -128 -128 -128 -127 -128 -127 0 -1198 9999872 -128 -128 -128 -128 0 -127 -1198 -128 -128 0 -128 -127 -1198 -128
< ({valueOf:function(){return null}}): 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< new Boolean(true): 1 1 1 0 1 0 -127 1235 -9999999 1 1 1 1 -127 0 1235 1 1 -127 1 0 1235 1
< new Number(1234.5678): 1234 1234 1234 1235 1234 1235 -1198 0 -9999022 1234 1234 1234 1234 -1198 1235 0 1234 1234 -1198 1234 1235 0 1234
< new String("str"): 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< 
< |
< undefined: 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< null: 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< false: 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< true: 1 1 1 1 1 1 -127 1235 -9999999 1 1 1 1 -127 1 1235 1 1 -127 1 1 1235 1
< 0: 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< 1: 1 1 1 1 1 1 -127 1235 -9999999 1 1 1 1 -127 1 1235 1 1 -127 1 1 1235 1
< -128: -128 -128 -128 -127 -128 -127 -128 -46 -128 -128 -128 -128 -128 -128 -127 -46 -128 -128 -128 -128 -127 -46 -128
< 1234.5678: 1234 1234 1234 1235 1234 1235 -46 1234 -9998894 1234 1234 1234 1234 -46 1235 1234 1234 1234 -46 1234 1235 1234 1234
< -1.0e+7: -10000000 -10000000 -10000000 -9999999 -10000000 -9999999 -128 -9998894 -10000000 -10000000 -10000000 -10000000 -10000000 -128 -9999999 -9998894 -10000000 -10000000 -128 -10000000 -9999999 -9998894 -10000000
< NaN: 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< Infinity: 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< "": 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< "str": 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< "-128": -128 -128 -128 -127 -128 -127 -128 -46 -128 -128 -128 -128 -128 -128 -127 -46 -128 -128 -128 -128 -127 -46 -128
< "1.0": 1 1 1 1 1 1 -127 1235 -9999999 1 1 1 1 -127 1 1235 1 1 -127 1 1 1235 1
< "  +1234  ": 1234 1234 1234 1235 1234 1235 -46 1234 -9998894 1234 1234 1234 1234 -46 1235 1234 1234 1234 -46 1234 1235 1234 1234
< "string": 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< "5tring": 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< ({valueOf:function(){return "-128"}}): -128 -128 -128 -127 -128 -127 -128 -46 -128 -128 -128 -128 -128 -128 -127 -46 -128 -128 -128 -128 -127 -46 -128
< ({valueOf:function(){return null}}): 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< new Boolean(true): 1 1 1 1 1 1 -127 1235 -9999999 1 1 1 1 -127 1 1235 1 1 -127 1 1 1235 1
< new Number(1234.5678): 1234 1234 1234 1235 1234 1235 -46 1234 -9998894 1234 1234 1234 1234 -46 1235 1234 1234 1234 -46 1234 1235 1234 1234
< new String("str"): 0 0 0 1 0 1 -128 1234 -10000000 0 0 0 0 -128 1 1234 0 0 -128 0 1 1234 0
< 
< <<
< undefined: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< null: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< false: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< true: 1 1 1 2 1 2 1 262144 1 1 1 1 1 1 2 262144 1 1 1 1 2 262144 1
< 0: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< 1: 1 1 1 2 1 2 1 262144 1 1 1 1 1 1 2 262144 1 1 1 1 2 262144 1
< -128: -128 -128 -128 -256 -128 -256 -128 -33554432 -128 -128 -128 -128 -128 -128 -256 -33554432 -128 -128 -128 -128 -256 -33554432 -128
< 1234.5678: 1234 1234 1234 2468 1234 2468 1234 323485696 1234 1234 1234 1234 1234 1234 2468 323485696 1234 1234 1234 1234 2468 323485696 1234
< -1.0e+7: -10000000 -10000000 -10000000 -20000000 -10000000 -20000000 -10000000 -1509949440 -10000000 -10000000 -10000000 -10000000 -10000000 -10000000 -20000000 -1509949440 -10000000 -10000000 -10000000 -10000000 -20000000 -1509949440 -10000000
< NaN: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< Infinity: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "str": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "-128": -128 -128 -128 -256 -128 -256 -128 -33554432 -128 -128 -128 -128 -128 -128 -256 -33554432 -128 -128 -128 -128 -256 -33554432 -128
< "1.0": 1 1 1 2 1 2 1 262144 1 1 1 1 1 1 2 262144 1 1 1 1 2 262144 1
< "  +1234  ": 1234 1234 1234 2468 1234 2468 1234 323485696 1234 1234 1234 1234 1234 1234 2468 323485696 1234 1234 1234 1234 2468 323485696 1234
< "string": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "5tring": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< ({valueOf:function(){return "-128"}}): -128 -128 -128 -256 -128 -256 -128 -33554432 -128 -128 -128 -128 -128 -128 -256 -33554432 -128 -128 -128 -128 -256 -33554432 -128
< ({valueOf:function(){return null}}): 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< new Boolean(true): 1 1 1 2 1 2 1 262144 1 1 1 1 1 1 2 262144 1 1 1 1 2 262144 1
< new Number(1234.5678): 1234 1234 1234 2468 1234 2468 1234 323485696 1234 1234 1234 1234 1234 1234 2468 323485696 1234 1234 1234 1234 2468 323485696 1234
< new String("str"): 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< 
< >>
< undefined: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< null: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< false: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< true: 1 1 1 0 1 0 1 0 1 1 1 1 1 1 0 0 1 1 1 1 0 0 1
< 0: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< 1: 1 1 1 0 1 0 1 0 1 1 1 1 1 1 0 0 1 1 1 1 0 0 1
< -128: -128 -128 -128 -64 -128 -64 -128 -1 -128 -128 -128 -128 -128 -128 -64 -1 -128 -128 -128 -128 -64 -1 -128
< 1234.5678: 1234 1234 1234 617 1234 617 1234 0 1234 1234 1234 1234 1234 1234 617 0 1234 1234 1234 1234 617 0 1234
< -1.0e+7: -10000000 -10000000 -10000000 -5000000 -10000000 -5000000 -10000000 -39 -10000000 -10000000 -10000000 -10000000 -10000000 -10000000 -5000000 -39 -10000000 -10000000 -10000000 -10000000 -5000000 -39 -10000000
< NaN: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< Infinity: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "str": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "-128": -128 -128 -128 -64 -128 -64 -128 -1 -128 -128 -128 -128 -128 -128 -64 -1 -128 -128 -128 -128 -64 -1 -128
< "1.0": 1 1 1 0 1 0 1 0 1 1 1 1 1 1 0 0 1 1 1 1 0 0 1
< "  +1234  ": 1234 1234 1234 617 1234 617 1234 0 1234 1234 1234 1234 1234 1234 617 0 1234 1234 1234 1234 617 0 1234
< "string": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "5tring": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< ({valueOf:function(){return "-128"}}): -128 -128 -128 -64 -128 -64 -128 -1 -128 -128 -128 -128 -128 -128 -64 -1 -128 -128 -128 -128 -64 -1 -128
< ({valueOf:function(){return null}}): 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< new Boolean(true): 1 1 1 0 1 0 1 0 1 1 1 1 1 1 0 0 1 1 1 1 0 0 1
< new Number(1234.5678): 1234 1234 1234 617 1234 617 1234 0 1234 1234 1234 1234 1234 1234 617 0 1234 1234 1234 1234 617 0 1234
< new String("str"): 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< 
< >>>
< undefined: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< null: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< false: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< true: 1 1 1 0 1 0 1 0 1 1 1 1 1 1 0 0 1 1 1 1 0 0 1
< 0: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< 1: 1 1 1 0 1 0 1 0 1 1 1 1 1 1 0 0 1 1 1 1 0 0 1
< -128: 4294967168 4294967168 4294967168 2147483584 4294967168 2147483584 4294967168 16383 4294967168 4294967168 4294967168 4294967168 4294967168 4294967168 2147483584 16383 4294967168 4294967168 4294967168 4294967168 2147483584 16383 4294967168
< 1234.5678: 1234 1234 1234 617 1234 617 1234 0 1234 1234 1234 1234 1234 1234 617 0 1234 1234 1234 1234 617 0 1234
< -1.0e+7: 4284967296 4284967296 4284967296 2142483648 4284967296 2142483648 4284967296 16345 4284967296 4284967296 4284967296 4284967296 4284967296 4284967296 2142483648 16345 4284967296 4284967296 4284967296 4284967296 2142483648 16345 4284967296
< NaN: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< Infinity: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "str": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "-128": 4294967168 4294967168 4294967168 2147483584 4294967168 2147483584 4294967168 16383 4294967168 4294967168 4294967168 4294967168 4294967168 4294967168 2147483584 16383 4294967168 4294967168 4294967168 4294967168 2147483584 16383 4294967168
< "1.0": 1 1 1 0 1 0 1 0 1 1 1 1 1 1 0 0 1 1 1 1 0 0 1
< "  +1234  ": 1234 1234 1234 617 1234 617 1234 0 1234 1234 1234 1234 1234 1234 617 0 1234 1234 1234 1234 617 0 1234
< "string": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< "5tring": 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< ({valueOf:function(){return "-128"}}): 4294967168 4294967168 4294967168 2147483584 4294967168 2147483584 4294967168 16383 4294967168 4294967168 4294967168 4294967168 4294967168 4294967168 2147483584 16383 4294967168 4294967168 4294967168 4294967168 2147483584 16383 4294967168
< ({valueOf:function(){return null}}): 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< new Boolean(true): 1 1 1 0 1 0 1 0 1 1 1 1 1 1 0 0 1 1 1 1 0 0 1
< new Number(1234.5678): 1234 1234 1234 617 1234 617 1234 0 1234 1234 1234 1234 1234 1234 617 0 1234 1234 1234 1234 617 0 1234
< new String("str"): 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
< 
-
