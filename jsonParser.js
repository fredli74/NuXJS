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

var JSON_ESCAPE_SEQUENCES = { "\\": "\\\\", "\"": "\\\"", "\b": "\\b", "\f": "\\f", "\n": "\\n", "\r": "\\r", "\t": "\\t" };

JSON.stringify = function(val) {
	var stack = [ ];

	function quote(str) {
		var s = '"', l = str.length;
		for (var i = 0; i < l; ++i) {
			var ch = str[i], seq;
			if (seq = JSON_ESCAPE_SEQUENCES[ch]) s += seq;
			else if (ch >= ' ' && ch <= '~') s += ch;
			else s += "\\u" + ("0000" + ch.charCodeAt(0).toString(16)).slice(-4);
		}
		return s + '"';
	}

	function str(key, holder) {
		var val = holder[key];
		switch (typeof val) {
			case "object": {
				if (!val) return "null";
				for (var i = stack.length - 1; --i >= 0; )
					if (stack[i] === val) throw TypeError("Cannot convert circular structure to JSON");
				stack[stack.length] = val;
				if (Array.isArray(val)) {
					var l = val.length, s = '[';
					for (var i = 0; i < l; ++i) {
						if (s !== '[') s += ',';
						s += str(i, val) || "null";
					}
					return s + ']';
				} else {
					var s = '{', v;
					for (var k in val) {
						if (val.hasOwnProperty(k)) {
							if (v = str(k, val)) {
								if (s !== '{') s += ',';
								s += quote(k) + ':' + v;
							}
						}
					}
					return s + '}';
				}
			}

			case "string": return quote(val);
			case "number": return (isFinite(val) ? String(val) : "null");
			case "boolean": return String(val);
		}
	}
	return str('', { '': val });
}

/*print(quote("abcd"));
print(quote("ab\ncd"));
print(quote("ab\bc\fd\n\r\t\u9999\"\u1234\\\/"));*/
print(JSON.stringify({}));
print(JSON.stringify([]));
print(JSON.stringify({ x: 1, y: "asdf", z: false, a: [ 1,2,3,undefined,5 ], o: { zzz: null, aaa: undefined, "\u9483": "\n\r\u1234\\/ab" } }));
var o = { n: null }; first = o; for (var i = 0; i < 20; ++i) { o.n = { i: i, n: null }; o = o.n; } o.n = first.n.n.n.n.n;
print(JSON.stringify(o));

