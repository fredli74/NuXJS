var JSON_ESCAPE_SEQUENCES = { "\\": "\\\\", "\"": "\\\"", "\b": "\\b", "\f": "\\f", "\n": "\\n", "\r": "\\r", "\t": "\\t" };
var MAX_JSON_DEPTH = 61;
	
if (typeof JSON === 'undefined') {
	JSON = {};

	JSON.stringify = function(val, replacer, space) {
		var stack = [ ], replacerFunction = (typeof replacer === "function" ? replacer : null), gap = '', includeProps;

		if (Array.isArray(replacer)) {
			includeProps = {};
			for (var i = replacer.length; --i >= 0;) includeProps[replacer[i]] = true;
		}

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
					for (var i = stack.length; --i >= 0; ) {
						if (stack[i] === val) throw TypeError("Cannot convert circular structure to JSON");
					}
					if (stack.length > MAX_JSON_DEPTH) throw TypeError("Structure too deeply nested for JSON conversion");
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
						for (var k in (includeProps ? includeProps : val)) {
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

/*
##################
#                #
#    JSON PEG    #
#                #
##################

root		<-	_ (value) !.
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

if (!JSON.parse) {
	JSON.parse = function(text, reviver) {
		var nest = 0;
		function space(t, p) {
			var ch;
			while ((ch = t[p]) === ' ' || ch === '\t' || ch === '\r' || ch === '\n') ++p;
			return p;
		}
		function number(t, p) {
			var ch;
			if (t[p] === '-') ++p;
			if ((ch = t[p]) === '0') ch = t[++p];
			else if (ch >= '1' && ch <= '9') do { ch = t[++p]; } while (ch >= '0' && ch <= '9');
			else return;
			if (ch === '.') {
				if ((ch = t[++p]) < '0' || ch > '9') return;
				while ((ch = t[++p]) >= '0' && ch <= '9') ;
			}
			if (ch === 'e' || ch === 'E') {
				if ((ch = t[++p]) === '-' || ch === '+') ch = t[++p];
				if ((ch = t[p]) < '0' || ch > '9') return;
				while ((ch = t[++p]) >= '0' && ch <= '9') ;
			}
			return p;
		}
		function string(t, p) {
			var ch;
			if (t[p] !== '"') return;
			++p;
			while ((ch = t[p]) !== '"' && ch >= ' ') {
				if (ch === '\\') {
					switch (t[++p]) {
						default: return;
						case '"': case '/': case '\\': case 'b': case 'f': case 'n': case 'r': case 't': break;
						case 'u':
							for (var i = 4; --i >= 0; ) {
								if (!(((ch = t[++p]) >= '0' && ch <= '9')
										|| (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))) return;
							};
							break;
					}
				}
				++p;
			}
			if (ch === '"') return ++p;
		}
		function literal(t, p) {
			if (t.substr(p, 4) === "true") return p + 4;
			else if (t.substr(p, 5) === "false") return p + 5;
			else if (t.substr(p, 4) === "null") return p + 4;
		}
		function objectOrArray(t, p, open, close) {
			if (nest > MAX_JSON_DEPTH) throw TypeError("Structure too deeply nested for JSON conversion");
			++nest;
			try {
				var ch, expectComma = false;
				if (t[p] !== open) return;
				p = space(t, ++p);
				while ((ch = t[p]) !== close && ch !== '') {
					if (expectComma) {
						if (ch !== ',') return;
						p = space(t, ++p);
					}
					if (open === '{') {
						if (!(p = string(t, p)) || t[p = space(t, p)] !== ':') return;
						p = space(t, ++p);
					}
					if (!(parser = PARSERS[t[p]]) || !(p = parser(t, p))) return;
					p = space(t, p);
					expectComma = true;
				}
				if (ch === close) return ++p;
			}
			finally {
				--nest;
			}
		}
		function object(t, p) { return objectOrArray(t, p, '{', '}'); }
		function array(t, p) { return objectOrArray(t, p, '[', ']'); }
		var PARSERS = {
			'{': object, '[': array, '"': string, 't': literal, 'f': literal, 'n': literal
			, '-': number, '0': number, '1': number, '2': number, '3': number, '4': number
			, '5': number, '6': number, '7': number, '8': number, '9': number
		};
		text = text.toString();
		var p, parser;
		if ((parser = PARSERS[text[p = space(text, 0)]]) && (p = parser(text, p))
				&& (p = space(text, p)) === text.length) {
			var val = eval('(' + text + ')');
            if (typeof reviver === "function") {
            	function walk(holder, key) {
	                var k, v, o;
	                if (typeof (o = holder[key]) === "object" && o) {
	                    for (k in o) {
	                        if (o.hasOwnProperty(k)) {
	                            if ((v = walk(o, k)) !== void 0) o[k] = v;
	                            else delete o[k];
	                        }
	                    }
	                }
	                return reviver.call(holder, key, o);
	            }
				val = walk({ "": val }, "");
			}
			return val;
		}
		throw SyntaxError("Error parsing JSON");
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
var o = { n: null }; first = o; for (var i = 0; i < 1000; ++i) { o.n = { i: i, n: null }; o = o.n; } o.n = first.n.n.n.n.n;
try { print(JSON.stringify(o)); } catch (x) { print(x) }
var o = { }; o.n = o;
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
print(JSON.stringify({a:"a",b:"b",3:"3"},["a","a",new Number(3)]));
print(JSON.stringify({a:"a",b:"b",3:"3",c:{a:"popo",d:"nada"}},["a","a",new Number(3)]));
print(JSON.stringify({a:"a",b:"b",3:"3",a:{a:"popo",d:"nada"}},["a","a",new Number(3)]));

print(JSON.parse("0"));
print(JSON.parse("0.1"));
print(JSON.parse("123"));
print(JSON.parse("123.456"));
print(JSON.parse("123.456e33"));
print(JSON.parse("123.456e+33"));
print(JSON.parse("-123.456e-33"));
print(JSON.parse("  \t\r\n   123.456  \n\r\t   "));
try { print(JSON.parse("")) } catch (x) { print(x) }
try { print(JSON.parse("    ")) } catch (x) { print(x) }
try { print(JSON.parse("x")) } catch (x) { print(x) }
try { print(JSON.parse("-")) } catch (x) { print(x) }
try { print(JSON.parse(".")) } catch (x) { print(x) }
try { print(JSON.parse("e-33")) } catch (x) { print(x) }
try { print(JSON.parse("0123")) } catch (x) { print(x) }
try { print(JSON.parse("0123")) } catch (x) { print(x) }
try { print(JSON.parse("-.e-33")) } catch (x) { print(x) }
try { print(JSON.parse("-.456e-33")) } catch (x) { print(x) }
try { print(JSON.parse("-23.e-33")) } catch (x) { print(x) }
try { print(JSON.parse("23+23")) } catch (x) { print(x) }
print(JSON.parse('""'));
print(JSON.parse('"abcd"'));
print(JSON.parse('"abcd\\n\\tefgh\\/\\\\\\\""'));
print(JSON.parse('"\\u0047 \\u004f \\u004C"'));
try { print(JSON.parse('"')) } catch (x) { print(x) }
try { print(JSON.parse('"\u0012"')) } catch (x) { print(x) }
try { print(JSON.parse('"\\u333g"')) } catch (x) { print(x) }
try { print(JSON.parse('"\\k"')) } catch (x) { print(x) }
print(JSON.stringify(JSON.parse('[]')));
print(JSON.stringify(JSON.parse('[ "bcd" ]')));
print(JSON.stringify(JSON.parse('[ "bcd"  , "fgh" ]')));
print(JSON.stringify(JSON.parse('[ "bcd", "fgh", [ 123.456e+11 ] ]')));
print(JSON.stringify(JSON.parse('["bcd","fgh",[123.456e+11]]')));
print(JSON.stringify(JSON.parse('["bcd","fgh",[123.456e+11], null, true, false]')));
try { print(JSON.stringify(JSON.parse('['))) } catch (x) { print(x) }
try { print(JSON.stringify(JSON.parse('[ "a":"b" ]'))) } catch (x) { print(x) }
try { print(JSON.stringify(JSON.parse('[ "a" "b" ]'))) } catch (x) { print(x) }
try { print(JSON.stringify(JSON.parse('[ "a","b", ]'))) } catch (x) { print(x) }
try { print(JSON.stringify(JSON.parse('[ "a",,"b" ]'))) } catch (x) { print(x) }
print(JSON.stringify(JSON.parse('{}')));
print(JSON.stringify(JSON.parse('{ "a": "bcd" }')));
print(JSON.stringify(JSON.parse('{ "a": "bcd"  , "e": "fgh" }')));
print(JSON.stringify(JSON.parse('{ "a": "bcd", "e": "fgh", "f": { "g\u004C": 123.456e+11 } }')));
print(JSON.stringify(JSON.parse('{"a":"bcd","e":"fgh","f":{"g\u004C":123.456e+11}}')));
try { print(JSON.stringify(JSON.parse('{'))) } catch (x) { print(x) }
try { print(JSON.stringify(JSON.parse('{ a:"b" }'))) } catch (x) { print(x) }
try { print(JSON.stringify(JSON.parse('{ "a" }'))) } catch (x) { print(x) }
try { print(JSON.stringify(JSON.parse('{ "a": }'))) } catch (x) { print(x) }
try { print(JSON.stringify(JSON.parse('{ "a":"b" "c":"d" }'))) } catch (x) { print(x) }
try { print(JSON.stringify(JSON.parse('{ "a":"b","c":"d", }'))) } catch (x) { print(x) }
try { print(JSON.stringify(JSON.parse('{ "a":"b",,"c":"d" }'))) } catch (x) { print(x) }
print(JSON.stringify(JSON.parse('{"a":"bcd","e":"fgh","f":{"g\u004C":123.456e+11}}', function(k, v) { return (k === "gL" ? "qwertyiop" : v) })));
try { print(JSON.stringify(JSON.parse("[1eE2]"))) } catch (x) { print(x) }
var s = ''; for (var i = 0; i < 63; ++i) s += '['; s += '234'; for (var i = 0; i < 63; ++i) s += ']'; try { print(JSON.stringify(JSON.parse(s))) } catch (x) { print(x) }
var s = ''; for (var i = 0; i < 62; ++i) s += '['; s += '234'; for (var i = 0; i < 62; ++i) s += ']'; try { print(JSON.stringify(JSON.parse(s))) } catch (x) { print(x) }
var s = ''; for (var i = 0; i < 62; ++i) s += '['; s += '234'; for (var i = 0; i < 62; ++i) s += ']'; try { print(JSON.stringify([JSON.parse(s)])) } catch (x) { print(x) }
