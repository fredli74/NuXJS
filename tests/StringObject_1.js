/*
According to 11.2.1 (and 8.7.1), any property access may result in a conversion of the target base (i.e. the left-hand
side of '.' or '[]') to an object (if it is a "primitive type"). A string is a "primitive type". This means that, in
theory, any subscript access ('[]') on a string requires the creation of a temporary full-blown String object (which is
then just thrown away).

Workaround in NuXJScript: strings are "shallow" objects meaning that they can't be extended but they have properties
that can be accessed without any object conversion / creation. Only member calls (on a member in String.prototype) will
actually convert "this" to a full-blown "deep" String object. Any such member call *might* indirectly use "this" for
extending the String so this is necessary to obtain 100% EcmaScript compatibility.
*/

breakOuts = [ ];
String.prototype.test = function(i, s) { breakOuts[i] = this; breakOuts[i].s = s; };
var s = "zxcv";
s.test(0, 'asdf');
s.test(1, 'qwerty');
print('typeof breakOuts[0]: ' + typeof breakOuts[0]);
print('typeof breakOuts[0].s: ' + typeof breakOuts[0].s);
print('breakOuts[0]: ' + breakOuts[0]);
print('breakOuts[0].s: ' + breakOuts[0].s);
print('breakOuts[1]: ' + breakOuts[1]);
print('breakOuts[1].s: ' + breakOuts[1].s);
