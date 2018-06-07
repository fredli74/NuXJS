/*
##################
#                #
#    JSON PEG    #
#                #
##################

root		<-	_ (object / array) !.
object		<-	"{"_ ( member (","_ member)* )? "}"_
member		<-	string ":"_ value
array		<-	"["_ ( value (","_ value)* )? "]"_
value		<-	string / number / object / array / "true"_ / "false"_ / "null"_
string		<-	'"' ( !["\\\0-\37] . / "\\" ( ["\\/bfnrt] / 'u' HEX HEX HEX HEX ) )* '"' _
number		<-	"-"? ("0" / [1-9] DIGIT*) ("." DIGIT+)? ( [eE][-+]? DIGIT+ )? _
HEX			<-	[0-9a-fA-F]
DIGIT		<-	[0-9]
_			<-	[ \t\r\n]*
*/

if (typeof JSON === 'undefined') {
	var JSON_ESCAPE_SEQUENCES = { "\\": "\\\\", "\"": "\\\"", "\b": "\\b", "\f": "\\f", "\n": "\\n", "\r": "\\r", "\t": "\\t" };

	JSON = {};

	JSON.stringify = function(val, replacer, space) {
		var stack = [ ], replacerFunction = (typeof replacer === "function" ? replacer : null), gap = '';

        if (typeof space === "number") {
            for (var i = (space > 10 ? 10 : space); --i >= 0;) gap += " ";
        } else if (typeof space === "string") {
            gap = space.slice(0, 10);
        }

		function quote(str) {
			var s = '"', l = str.length;
			for (var i = 0; i < l; ++i) {
				var ch = str[i], seq;
				s += ((seq = JSON_ESCAPE_SEQUENCES[ch])
						? seq : ((ch >= ' ' && ch <= '~')
						? ch : "\\u" + ("0000" + ch.charCodeAt(0).toString(16)).slice(-4)));
			}
			return s + '"';
		}

		function str(key, holder, indent) {
			var val;
	        if ((val = holder[key]) && typeof val === "object" && typeof val.toJSON === "function") val = val.toJSON(key);
	        if (replacerFunction) val = replacerFunction.call(holder, key, val);

	        var lineEnd = (gap ? '\n' + indent : '');
			switch (typeof val) {
				case "object": {
					if (!val) return "null";
					for (var i = stack.length - 1; --i >= 0; ) {
						if (stack[i] === val) throw TypeError("Cannot convert circular structure to JSON");
					}
					stack[stack.length] = val;
					if (Array.isArray(val)) {
						var s = '', l = val.length;
						for (var i = 0; i < l; ++i) {
							s += (s ? ',' : '[') + lineEnd + gap;
							s += str(i, val, indent + gap) || "null";
						}
						return (s ? s + lineEnd + ']' : "[]");
					} else {
						var s = '', v;
						for (var k in val) {
							if (val.hasOwnProperty(k)) {
								if (v = str(k, val, indent + gap)) {
									s += (s ? ',' : '{') + lineEnd + gap;
									s += quote(k) + (gap ? ": " : ':') + v;
								}
							}
						}
						return (s ? s + lineEnd + '}' : "{}");
					}
				}

				case "string": return quote(val);
				case "number": return (isFinite(val) ? String(val) : "null");
				case "boolean": return String(val);
			}
		}
		return str('', { '': val }, '');
	}
}

print(JSON.stringify("abcd"));
print(JSON.stringify("ab\ncd"));
print(JSON.stringify("ab\bc\fd\n\r\t\u9999\"\u1234\\\/"));
print(JSON.stringify({}));
print(JSON.stringify([]));
print(JSON.stringify({ x: 1, y: "asdf", z: false, a: [ 1,2,3,undefined,5 ], o: { zzz: null, aaa: undefined, "\u9483": "\n\r\u1234\\/ab" } }));
var o = { n: null }; first = o; for (var i = 0; i < 20; ++i) { o.n = { i: i, n: null }; o = o.n; } o.n = first.n.n.n.n.n;
try { print(JSON.stringify(o)); } catch (x) { print(x) }
var o = { toJSON: function(k) { return "smorfen " + k; } };
print(JSON.stringify(o));
var o = { "hellu": { a: "b", toJSON: function(k) { return "smorfen " + k; } } }
print(JSON.stringify(o));
print(JSON.stringify(o, function(k, v) { return this[k]; }));
var o = { "hellu": { a: "b", b: "c", d: [ 1, 2, 3, [ 4, 5 ] ] } }
print(JSON.stringify(o, function(k, v) { return (k === 'a' ? "yeah" : v); }));
try { print(JSON.stringify(o, function(k, v) { return this } )); } catch (x) { print(x) }
print(JSON.stringify(o, null, ' '));
print(JSON.stringify(o, null, 129831));
print(JSON.stringify(o, null, new Number(4)));
print(JSON.stringify(o, null, "\t\txx\t                      "));
print(JSON.stringify([], null, new String("\t\txx\t                      ")));
print(JSON.stringify([1], null, "\t\txx\t                      "));
print(JSON.stringify(o, null, ""));
print(JSON.stringify(new Number(234)));
print(JSON.stringify(new String(234)));
print(JSON.stringify(new Boolean(234)));
