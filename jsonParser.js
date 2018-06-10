var JSON_ESCAPE_SEQUENCES = { "\\": "\\\\", "\"": "\\\"", "\b": "\\b", "\f": "\\f", "\n": "\\n", "\r": "\\r", "\t": "\\t" };

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

if (true) {
	JSON.parse = function(text, reviver) {
		function ws(t, p) {
			var ch;
			while ((ch = t[p]) === ' ' || ch === '\t' || ch === '\r' || ch === '\n') ++p;
			return p;
		}
		function num(t, p) {
			var b = p, ch;
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
				if ((ch = t[++p]) < '0' || ch > '9') return;
				while ((ch = t[++p]) >= '0' && ch <= '9') ;
			}
			return p;
		}
		function str(t, p) {
			var b = p, ch;
			if (t[p] !== '"') return;
			++p;
			while ((ch = t[p]) !== '"' && ch >= ' ') {
				if (ch === '\\') {
					switch (t[++p]) {
						default: return;
						case '"': case '/': case '\\': case 'b': case 'f': case 'n': case 'r': case 't': break;
						case 'u': {
							for (var i = 4; --i >= 0; ) {
								if (!(((ch = t[++p]) >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))) return;
							};
							break;
						}
					}
				}
				++p;
			}
			if (ch === '"') return ++p;
		}
		function lit(t, p) {
			if (t.substr(p, 4) === "true") return p + 4;
			else if (t.substr(p, 5) === "false") return p + 5;
			else if (t.substr(p, 4) === "null") return p + 4;
		}
		function obj(t, p) {
			var b = p, ch, expectComma = false;
			if (t[p] !== '{') return;
			p = ws(t, ++p);
			while ((ch = t[p]) !== '}' && ch !== '') {
				if (expectComma) {
					if (ch !== ',') return;
					p = ws(t, ++p);
				}
				if (!(p = str(t, p)) || t[p = ws(t, p)] !== ':') return;
				if (!(parser = PARSERS[t[p = ws(t, ++p)]]) || !(p = parser(t, p))) return;
				p = ws(t, p);
				expectComma = true;
			}
			if (ch === '}') return ++p;
		}
		function arr(t, p) {
			var b = p, ch, l = -1, expectComma = false;
			if (t[p] !== '[') return;
			p = ws(t, ++p);
			while ((ch = t[p]) !== ']' && ch !== '') {
				if (expectComma) {
					if (ch !== ',') return;
					p = ws(t, ++p);
				}
				if (!(parser = PARSERS[t[p]]) || !(p = parser(t, p))) return;
				p = ws(t, p);
				expectComma = true;
			}
			if (ch === ']') return ++p;
		}
		var PARSERS = {
			'{': obj, '[': arr, '"': str, 't': lit, 'f': lit, 'n': lit, '-': num
			, '0': num, '1': num, '2': num, '3': num, '4': num, '5': num, '6': num, '7': num, '8': num, '9': num
		};
		text = text.toString();
		var p, parser;
		if ((parser = PARSERS[text[p = ws(text, 0)]]) && (p = parser(text, p)) && (p = ws(text, p)) === text.length) {
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
