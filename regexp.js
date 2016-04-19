"use strict";

var v = 1;
var EMPTY_CHAR = v, NEWLINE_CHAR = (v <<= 1), SPACE_CHAR = (v <<= 1), WORD_CHAR = (v <<= 1), DECIMAL_CHAR = (v <<= 1);
var LETTER_CHAR = (v <<= 1), HEX_CHAR = (v <<= 1), ESCAPE_CHAR = (v <<= 1), SPECIAL_CHAR = (v <<= 1);
var OK_IN_STRING_LITERAL = (v <<= 1);
var CC = { };

function setupCharClass(mask, chars) { for (var i in chars) CC[chars[i]] |= mask; }

setupCharClass(SPECIAL_CHAR, "\\^$.*+?()[]{}|");
setupCharClass(DECIMAL_CHAR | HEX_CHAR | WORD_CHAR, "0123456789");
setupCharClass(HEX_CHAR | LETTER_CHAR | WORD_CHAR, "abcdefABCDEF");
setupCharClass(LETTER_CHAR | WORD_CHAR, "ghijklmnopqrstuvwxyzGHIJKLMNOPQRSTUVWXYZ");
setupCharClass(NEWLINE_CHAR | SPACE_CHAR, "\n\r\u2028\u2029");
setupCharClass(SPACE_CHAR, " \t\v\f\xA0");
CC['_'] |= WORD_CHAR;
CC['undefined'] |= EMPTY_CHAR;
CC[''] |= EMPTY_CHAR;
setupCharClass(ESCAPE_CHAR, "fnrtv");
for (var i = 32; i <= 126; ++i) {
	var c = String.fromCharCode(i);
	if (c !== '"') {
		CC[String.fromCharCode(i)] |= OK_IN_STRING_LITERAL;
	}
}

// Identity chars
// [\0-\60\72-\101\133-\137\140\173-\180]

function compileRegExp(s) {
	function isClass(char, mask) { return ((CC[char] & mask) !== 0); }
	function areClass(str, mask) {
		for (var i = str.length - 1; i >= 0; --i) {
			if ((CC[str[i]] & mask) === 0) {
				return false;
			}
		}
		return true;
	}
	var CHAR_CLASS_RULES = {
		'D': [ DECIMAL_CHAR, true ], 'd': [ DECIMAL_CHAR, false ],
		'S': [ SPACE_CHAR, true ], 's': [ SPACE_CHAR, false ],
		'W': [ WORD_CHAR, true ], 'w': [ WORD_CHAR, false ],
		'.': [ NEWLINE_CHAR, true ]
	};

	var p = 0, e = s.length;

	function parseNumber(defaultValue) {
		var n = defaultValue;
		if (isClass(s[p], DECIMAL_CHAR)) {
			n = 0;
			do {
				n = n * 10 + (s.charCodeAt(p) - 48);
				++p;
			} while (isClass(s[p], DECIMAL_CHAR))
		}
		return n;
	}

	function parseQuantifier() {
		var min = 0, max = Infinity, greedy = true;
		switch (s[p]) {
			case '*': ++p; break;
			case '+': ++p; min = 1; break;
			case '?': ++p; max = 1; break;
			case '{': {
				var b = p, n;
				++p;
				if ((min = max = parseNumber(-1)) < 0) {
					p = b;
					return null;
				}
				if (s[p] === ',') {
					++p;
					max = parseNumber(Infinity);
				}
				if (s[p] !== '}') {
					p = b;
					return null;
				}
				if (min > max) {
					throw SyntaxError("Min greater than max in regular expression quantifier"); // FIX
				}
				++p;
				break;
			}
			default: return null;
		}
		if (s[p] === '?') {
			++p;
			greedy = false;
		}
		return { min: min, max: max, greedy: greedy };
	}
	
	function parseLiteral() {
		var c0, c1, c2, sub;
		var literalSequence = [ ];
		while (p < e && (!isClass(c0 = s[p], SPECIAL_CHAR) || c0 === '\\')) {
			if (c0 === '\\') {
				if ((c1 = s[p + 1]) !== '0' && isClass(c1, DECIMAL_CHAR)) {
					break;
				} else if (c1 === '0' && !isClass(s[p + 2], DECIMAL_CHAR)) {
					literalSequence.push("\\0");
					p += 2;
				} else if (isClass(c1, ESCAPE_CHAR)) {
					literalSequence.push("\\" + c1);
					p += 2;
				} else if (c1 === 'c' && isClass(c2 = s[p + 2], LETTER_CHAR)) {
					literalSequence.push("\\x" + ("00" + (c2.charCodeAt(0) & 31).toString(16)).slice(-2));
					p += 3;
				} else if (c1 === 'x' && areClass(sub = s.substring(p + 2, p + 4), HEX_CHAR)) {
					literalSequence.push("\\x" + sub);
					p += 4;
				} else if (c1 === 'u' && areClass(sub = s.substring(p + 2, p + 6), HEX_CHAR)) {
					literalSequence.push("\\u" + sub);
					p += 6;
				// } else if (identity
				} else {
					break;
				}
			} else if (isClass(c0, OK_IN_STRING_LITERAL)) {
				literalSequence.push(c0);
				++p;
			} else if (c0 === '"') {
				literalSequence.push('\\"');
				++p;
			} else {
				literalSequence.push("\\u" + ("0000" + c0.charCodeAt(0).toString(16)).slice(-4));
				++p;
			}
		}
		return (literalSequence.length ? literalSequence : null);
	}

	function positionToCode(offset) {
		return (offset === 0 ? 'p' : 'p+' + offset);
	}

	function literalSequenceToCode(literalSequence, offset) {
		if (literalSequence.length === 0) {
			return 'true';
		} else if (literalSequence.length === 1) {
			return 's[' + positionToCode(offset) + ']==="' + literalSequence[0] + '"';
		} else if (literalSequence.length === 2) {
			return 's[' + positionToCode(offset) + ']==="' + literalSequence[0] + '" && s[' + positionToCode(offset + 1) + ']==="' + literalSequence[1] + '"';
		} else {
			return 'submatch(s,' + positionToCode(offset) + ',"' + literalSequence.join('') + '")';
		}
	}

	var functions = '', functionCounter = 0, captureCounter = 0, initCounters = '', maxBackReference = 0;

	function and(a, b) {
		switch (a) {
			case 'false': return 'false';
			case 'true': return b;
			default: return (b === 'true' ? a : a + ' && ' + b);
		}
	}

	function or(a, b) {
		switch (a) {
			case 'false': return b;
			case 'true': return 'true';
			default: return (b === 'false' ? a: a + ' || ' + b);
		}
	}
	
	function addFunction(name, definition) {
		functions += "\tfunction " + name + '(p) { ' + definition + "; }\n";
	}

	function quantify(code, offset, junction, repeatCode, quantity) {
		var functionName = 'q' + (++functionCounter);
		var tail = compileTerms(0, junction);
		if (quantity.greedy) {
			addFunction(functionName, 'var b=p+' + quantity.min + ',e=p+' + quantity.max + '; while (' + and('p<e', repeatCode) + ') ++p; while (' + and('p>=b', '!(' + tail + ')') + ') --p; return p>=b');
		} else {
			addFunction(functionName, 'var b=p+' + quantity.min + ',e=p+' + quantity.max + '; while (' + or('p<b', '!(' + tail + ')') + ') { if (' + or('p>=e', '!(' + repeatCode + ')') + ') return false; ++p }; return true');
		}
		return and(code, functionName + '(' + positionToCode(offset) + ')');
	}
	
	function captureWrap(fromCapture, toCapture, code) {
		if (fromCapture === toCapture) {
			return 'return ' + code;
		} else {
			var declares = '', captures = '', rollbacks = '';
			for (var i = fromCapture; i < toCapture; i += 2) {
				declares += (i > fromCapture ? ',r' : 'var r') + i + '=c' + i;
				captures += (i === fromCapture + 2 ? ',c' : 'c') + i + (i === fromCapture ? '=p' : '=');
				if (i === toCapture - 2 && fromCapture + 2 < toCapture) {
					captures += 'void 0';
				}
				rollbacks += (i > fromCapture ? ',c' : 'c') + i + '=r' + i;
			}
			return declares + '; return ' + captures + ', ' + or(code, '(' + rollbacks + ',false)');
		}
	}

	function charClassToCode(offset, mask, invert) {
		return (invert ? '!' : '!!') + '(CC[s[' + positionToCode(offset) + ']]&' + (invert ? mask | EMPTY_CHAR : mask) + ')';
	}

	function compileTerms(offset, junction) {
		var literalSequence, quantity, code = 'true', c, n;
		termLoop: for (;;) {
			if (literalSequence = parseLiteral()) {
				if (quantity = parseQuantifier()) {
					var repeatLiteral = literalSequence.pop();
					return quantify(and(code, literalSequenceToCode(literalSequence, offset))
							, offset + literalSequence.length, junction, literalSequenceToCode(repeatLiteral, 0)
							, quantity);
				}
				code = and(code, literalSequenceToCode(literalSequence, offset));
				offset += literalSequence.length;
			} else {
				switch ((c = s[p])) {
					case void 0: {
						code = and(code, '(e=' + positionToCode(offset) + ',true)');
						break termLoop;
					}
					case '^': {
						++p;
						code = and(code, positionToCode(offset) + '===0')
						break;
					}
					case '$': {
						++p;
						code = and(code, positionToCode(offset) + '===l');
						break;
					}
					case '\\': {
						++p;
						if ((n = parseNumber(-1)) >= 0) {
							if (n > maxBackReference) {
								maxBackReference = n;
							}
							n = (n - 1) * 2;
							var backMatchCode = 'submatch(s,' + positionToCode(offset) + ',s.substring(c' + n + ',c' + (n + 1) + '))';
							if (quantity = parseQuantifier()) {
								return quantify(code, offset, '', backMatchCode, quantity);
							}
							var tail = compileTerms(0, junction);
							var tailName = 't' + (++functionCounter);
							addFunction(tailName, 'return ' + tail);
							return and(code, '(' + or(and('c' + n + '===void 0', tailName + '(' + positionToCode(offset) + ')'), and(backMatchCode, tailName + '(' + positionToCode(offset) + '+c' + (n + 1) + '-c' + n + ')')) + ')');
						}
						c = s[p];
						// fall-through
					}
					case '.': {
						var rule;
						if (!(rule = CHAR_CLASS_RULES[c])) {
							break termLoop;
						}
						++p;
						if (quantity = parseQuantifier()) {
							return quantify(code, offset, junction, charClassToCode(0, rule[0], rule[1]), quantity);
						}
						code = and(code, charClassToCode(offset, rule[0], rule[1]));
						++offset;
						break;
					}
					case '(': {
						var b = p; // FIX : only used because of break termLoop further down...
						var functionNumber = ++functionCounter;
						var groupName = 'g' + functionNumber;
						var groupCall = groupName + '(' + positionToCode(offset) + ')';
						var junctionName = 'j' + functionNumber;
						var junctionCall = junctionName + '(' + positionToCode(offset) + ')';
						var doCapture = true, lookAhead = false, negativeLookAhead = false;
						++p;
						if (s[p] === '?') {
							switch (s[p + 1]) {
								case '!': negativeLookAhead = true;
								case '=': lookAhead = true;
								case ':': doCapture = false; p += 2;
							}
						}
						var openCaptureFrom = null, openCaptureTo = null, closeCaptureFrom = null, closeCaptureTo = null;
						if (doCapture) {
							closeCaptureTo = (closeCaptureFrom = (openCaptureFrom = (captureCounter++) * 2) + 1) + 2;
						}
						var disjunction = compileDisjunction(0, (lookAhead ? '' : junctionName));
						if (doCapture) {
							openCaptureTo = captureCounter * 2;
						}
						if (s[p] !== ')') {
							p = b; // FIX : a bit silly maybe, why not throw here directly?
							break termLoop; // FIX : following message doesn't really work if we are not truly at the end, e.g. (?%) aborts at ? and ends up here
							throw SyntaxError("Unterminated group in regular expression");
						}
						++p;
						var quantity = (lookAhead ? null : parseQuantifier());
						var tail = compileTerms(0, junction);
						var inner = disjunction;
						var tearDown = '';
						if (quantity && (quantity.max > 1 && quantity.max < Infinity || quantity.min > 1)) {
							initCounters += ',n' + functionNumber + '=0';
							if (quantity.max < Infinity) {
								inner = and('++n' + functionNumber + '<=' + quantity.max, inner);
							} else {
								inner = '++n' + functionNumber + ', ' + inner;
							}
							if (quantity.min > 1) {
								inner = and(inner, 'n' + functionNumber + '>=' + quantity.min, inner);
							}
							inner = or(inner, '(--n' + functionNumber + ',false)');
						}
						if (lookAhead) {
							inner = and((negativeLookAhead ? '!(' + inner + ')' : inner), junctionCall);
						}
						addFunction(groupName, captureWrap(openCaptureFrom, openCaptureTo, inner));
						if (quantity && quantity.max > 1) {
							var recursionCall = groupName + '(' + positionToCode(0) + ')';
							var inner = (quantity.greedy ? or(recursionCall, tail) : or(tail, recursionCall));
							addFunction(junctionName, captureWrap(closeCaptureFrom, closeCaptureTo, inner));
							code = and(code, (quantity.min === 0 ? junctionCall : groupCall));
						} else {
							addFunction(junctionName, captureWrap(closeCaptureFrom, closeCaptureTo, tail));
							if (quantity && quantity.min === 0) {
								code = and(code, (quantity.max === 0 ? junctionCall
										: '(' + (quantity.greedy ? or(groupCall, junctionCall) : or(junctionCall, groupCall)) + ')'));
							} else {
								code = and(code, groupCall);
							}
						}
						return code;
					}
					default: break termLoop;
				}
			}
		}
		return (junction === '' ? code : and(code, junction + '(' + positionToCode(offset) + ')'));
	}

	function compileDisjunction(offset, junction) {
		var code = 'false', minOffset = Infinity, maxOffset = 0;
		for (;;) {
			code = or(code, compileTerms(offset, junction));
			if (s[p] !== '|') {
				return code;
			}
			++p;
		}
	}

	var disjunction = compileDisjunction(0, ''); 
	if (p < e) {
		throw SyntaxError("Invalid regular expression");
	}
	if (maxBackReference > captureCounter) {
		throw SyntaxError("Invalid back reference in regular expression");
	}
	var code = "(function(s, p) {\n\tvar l=s.length,e";
	for (var i = 0; i < captureCounter * 2; ++i) code += ",c" + i;
	code += initCounters + ";\n" + functions + "\tif (" + disjunction + ") return [p,e";
	for (var i = 0; i < captureCounter * 2; ++i) code += ",c" + i;
	code += "];\n})";
print('');print(s);print('---');print(code);print('');
	return code;
}

function submatch(s, pos, str) { return (s.substr(pos, str.length) === str); }

function $isNaN(v) { return isNaN(v); }
function $isFinite(v) { return isFinite(v); }
function $floor(v) { return Math.floor(v); }

function $isPrimitive(v) {
	var v, t;
	return (((t = typeof v) !== "object" || v === null) && t !== "function");
}

function $toInteger(v) {
	return ($isNaN(v = +v) ? 0.0 : (!$isFinite(v) ? v : (v < 0 ? -$floor(-v) : $floor(v))));
}

function $toString(o) {
	return '' + ($isPrimitive(o) ? o : $support.toPrimitiveString(o));
}

var REG_EXP_FLAG_TO_PROPERTY = { 'g': 'global', 'i': 'ignoreCase', 'm': 'multiline' };

function MyRegExp(pattern, flags) {
	pattern = (pattern === void 0 ? '' : $toString(pattern));
	flags = (flags === void 0 ? '' : $toString(flags));
	this.global = this.ignoreCase = this.multiline = false;
	for (var i = flags.length - 1; i >= 0; --i) {
		var p;
		if (!(p = REG_EXP_FLAG_TO_PROPERTY[flags[i]]) || this[p]) {
			throw SyntaxError("Invalid regular expression flags");
		}
		this[p] = true; // FIX : { DontDelete, ReadOnly, DontEnum }.
	}
	this.source = pattern; // FIX : { DontDelete, ReadOnly, DontEnum }.
	this.match = eval(compileRegExp(pattern));
	this.lastIndex = 0; // FIX : { DontDelete, DontEnum }.
}

function $execRegExp(re, string) {
	var i, m, l;
	string = $toString(string);
	if ((i = (re.global ? $toInteger(re.lastIndex) : 0)) >= 0) {
		for (l = string.length; i <= l; ++i) {
			if (m = re.match(string, i)) {
				if (re.global) {
					re.lastIndex = m[1];
				}
				return m;
			}
		}
	}
	re.lastIndex = 0;
}

MyRegExp.prototype.exec = function(string) {
	var m, a = null;
	if (m = $execRegExp(this, string)) {
		(a = [ ]).input = string;
		a.index = m[0];
		for (var j = 0; j < m.length; j += 2) {
			a[a.length] = ((m[j] === void 0) ? void 0 : string.substring(m[j], m[j + 1]));
		}
	}
	return a;
};

MyRegExp.prototype.toString = function() {
	return '/' + this.source + '/' + (this.global ? 'g' : '') + (this.ignoreCase ? 'i' : '') + (this.multiline ? 'm' : '');
};

MyRegExp.prototype.test = function(string) { return $execRegExp(this, string) !== void 0; };

/*
print(compileRegExp("\\n\\rab\\nc\\u1234\\cA\\0\""));
print(compileRegExp("a"));
print(compileRegExp("ade...bfg*"));
print(compileRegExp("a\\n"));
print(compileRegExp("a|bdef.gh|iop.*"));
print(compileRegExp("qwerty(abc|defgh|x)yzzy"));

print(compileRegExp("qwerty(a(bc|pa..pa)poo*|defgh|x)yzzy(a|bc|def)*end$"));
print(compileRegExp("start((abc|de)*end)*end"));
print(compileRegExp("start(?:(?:abc|de)*end)*end"));
print(compileRegExp("start(?:x)+end"));
print(compileRegExp("((?:abc)+?)((?:abc)+)end"));
print(compileRegExp("((?:abc)+)((?:abc)+?)end"));
print(compileRegExp("(?:a)*e"));
print(compileRegExp("a*e"));
print(compileRegExp("s(a)?$"));
print(compileRegExp("s(a)??$"));
print(compileRegExp("s(?:a|)$"));
print(compileRegExp("(a*)*"));
print(compileRegExp("(z)(((?:a)+)?((?:b)+)?(c))*"));
print(compileRegExp("a(b)?c"));
print(compileRegExp("a(b)*c"));
print(compileRegExp("a(b(c)*)*d"));
print(compileRegExp("(abc(def)|(ghi)jkl)*"));

print(compileRegExp("b(a|b*|c)e"));
print(compileRegExp(".*"));
print(compileRegExp("s(a)??(a)?$"));
*/

function myescape(s) {
	var o = '', b = 0, c;
	for (var i = 0; i < s.length; ++i) {
		if ((c = s[i]) < ' ' || c > '\x7E') {
			o += s.substring(b, i) + ("\\u" + ("0000" + c.charCodeAt(0).toString(16)).slice(-4));
			b = i + 1;
		}
	}
	return o + (b > 0 ? s.substring(b, i) : s);
}

function testRE(re, s) {
	var m = re.exec(s);
	var r = '';
	if (!m) {
		r = 'null';
	} else {
		r = '{ ';
		for (var i = 0; i < m.length; ++i) {
			r += (i > 0 ? ', ' : '') + myescape('' + m[i]);
		}
		r += ' } @ ' + m.index + ' (next: ' + re.lastIndex + ')';
	}
	print(myescape(re.toString()) + " on " + myescape(s) + ": " + r + "\n");
}

var t = new MyRegExp("abcdef", "g");
var m;
testRE(t, "abcdef");
testRE(t, "abc");
testRE(t, "abcdeabcdefabcde");

var t = new MyRegExp("\\n\\rab\\nc\\u1234\\cA\\0\"");
testRE(t, "\n\rab\nc\u1234\x01\0\"");

var t = new MyRegExp("^abcdef");
testRE(t, "abcdef");
testRE(t, "abcdeabcdefabcde");

var t = new MyRegExp(".*$");
testRE(t, "abc\ndef");
testRE(t, "abc\u2029def");
testRE(t, "abc");

var t = new MyRegExp("^(a|bc|def)$");
testRE(t, "");
testRE(t, "a");
testRE(t, "bc");
testRE(t, "def");
testRE(t, "bcdef");

var t = new MyRegExp("^(|a|bc|def)$");
testRE(t, "");
testRE(t, "def");
testRE(t, "bcdef");

var t = new MyRegExp("\\w\\w\\w");
testRE(t, "");
testRE(t, "123");
testRE(t, "12a");
testRE(t, "12#");
var t = new MyRegExp("\\W\\W\\W");
testRE(t, "12#");
testRE(t, "#%)");
var t = new MyRegExp("\\w\\w\\w\\W\\s\\s\\s\\S\\S\\S\\d\\d\\d\\d\\D\\D\\D\\D");
testRE(t, "q9_!\n \u2028lm_9124qwer");

var t = new MyRegExp("(z)(((?:a)+)?((?:b)+)?(c))*");
testRE(t, "");
testRE(t, "zaacbbbcac");
t = new MyRegExp("s(a)??(a)?$");
testRE(t, "");
testRE(t, "s");
testRE(t, "sa");
testRE(t, "saa");
testRE(t, "saaa");

t = new MyRegExp("^(a){0}$");
testRE(t, "");
testRE(t, "a");
testRE(t, "aa");

t = new MyRegExp("^(a){1}$");
testRE(t, "");
testRE(t, "a");
testRE(t, "aa");

t = new MyRegExp("(a){3}");
testRE(t, "a");
testRE(t, "aa");
testRE(t, "aaa");
testRE(t, "aaaa");

t = new MyRegExp("(a){3,}");
testRE(t, "aa");
testRE(t, "aaa");
testRE(t, "aaaa");

t = new MyRegExp("(a){13,23}$");
testRE(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

t = new MyRegExp("(a){1,1}?$");
testRE(t, "a");
testRE(t, "aa");

t = new MyRegExp("(abc)\\1");
testRE(t, "abc");
testRE(t, "abcabc");
testRE(t, "qwertyabcabc");

t = new MyRegExp("^qwerty(abc|def)\\1$");
testRE(t, "qwertyabcabc");
testRE(t, "qwertydefdef");
testRE(t, "qwertyabcdef");
testRE(t, "qwertydefabc");

t = new MyRegExp("^(ab|def|ghij)?\\1{5}$");
testRE(t, "");
testRE(t, "ababababab");
testRE(t, "abababababab");

t = new MyRegExp("^(ab|def|ghij)?(\\1){5}$");
testRE(t, "");
testRE(t, "ababababab");
testRE(t, "abababababab");
testRE(t, "defdefdefdefdefdef");
testRE(t, "ghijghijghijghijghijghij");


t = new MyRegExp("^(ab)(abc)(\\1|\\2){5}abc$");
testRE(t, "abcd");
testRE(t, "ababcabababcabcabcabc");

t = new MyRegExp("^(ab|ab..)?(\\1){5}$");
testRE(t, "");
testRE(t, "abababababab");
testRE(t, "abababababababababababab");
testRE(t, "abcdabcdabcdabcdabcdabcd");

t = new MyRegExp("^(a|ab)(bc|c)(\\1\\2){5}$");
testRE(t, "abcabcabcabcabcabc");

t = new MyRegExp("((?:a)*)(\\1){3}$");
testRE(t, "");
testRE(t, "xxxxa");
testRE(t, "xxxxaaaa");
testRE(t, "xxxxaaaaaa");
testRE(t, "xxxxaaaaaaaa");

try { t = new MyRegExp("\\1"); } catch (x) { print(x); }
try { t = new MyRegExp("\\2(a)\\1"); } catch (x) { print(x); }

t = new MyRegExp("(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k)(l)\\1\\2\\3\\4\\5\\6\\7\\8\\9\\10\\11\\12");
testRE(t, "abcdefghijklabcdefghijkl");

t = new MyRegExp("^(?=abc)abcdef$");
testRE(t, 'def');
testRE(t, 'abcabcdef');
testRE(t, 'abcdef');

t = new MyRegExp("^(?=(abc|ab))abcdef\\1$");
testRE(t, 'abcdefabc');
testRE(t, 'abcdefab');
t = new MyRegExp("^(?=(ab|abc))abcdef\\1$");
testRE(t, 'abcdefabc');
testRE(t, 'abcdefab');

/*t = new MyRegExp("(?=((?:a)+))(?:a)*b\\1");
testRE(t, "baaabac");*/

try { t = new MyRegExp("^(?=ab)*abcdef$"); } catch (x) { print(x); }
try { t = new MyRegExp("(?%)"); } catch (x) { print(x); }
try { t = new MyRegExp("("); } catch (x) { print(x); }

t = new MyRegExp("^(?!abc)(.)+$");
testRE(t, "abc");
testRE(t, "ab");
testRE(t, "abcd");

t = new MyRegExp("^(?!(abc))(.)+$");
testRE(t, "abab");

t = new MyRegExp("^(?:a)+aa$");
testRE(t, "aaaaabc");

t = new MyRegExp("^a*$");
testRE(t, "");
testRE(t, "a");
testRE(t, "aaaaa");

t = new MyRegExp("^aaa*aa$");
testRE(t, "aaaaaaaaa");

t = new MyRegExp("^(aa)(a*)(aa)$");
testRE(t, "");
testRE(t, "aa");
testRE(t, "aaaa");
testRE(t, "aaaaaaaaa");

t = new MyRegExp("^(.*)\\1$");
testRE(t, "");
testRE(t, "aa");
testRE(t, "aaaa");
testRE(t, "qweqwe");
testRE(t, "qweqweqwe");
testRE(t, "qwertyqwertyqwertyqwerty");

t = new MyRegExp("^start(x+)end$");
testRE(t, "startend");
testRE(t, "startxend");
testRE(t, "startxxxxxend");
testRE(t, "startxxxxxen");

t = new MyRegExp("^start(x?)end$");
testRE(t, "startend");
testRE(t, "startxend");
testRE(t, "startxxxxxend");
testRE(t, "startxxxxxen");

t = new MyRegExp("^start(x{3,5})end$");
testRE(t, "startend");
testRE(t, "startxend");
testRE(t, "startxxend");
testRE(t, "startxxxend");
testRE(t, "startxxxxxend");
testRE(t, "startxxxxxxend");
testRE(t, "startxxxxxen");

t = new MyRegExp("^startx{3,5}?end$");
testRE(t, "startend");
testRE(t, "startxend");
testRE(t, "startxxend");
testRE(t, "startxxxend");
testRE(t, "startxxxxxend");
testRE(t, "startxxxxxxend");
testRE(t, "startxxxxxen");

t = new MyRegExp("^start(x{3,5})(x*)end$");
testRE(t, "startxxxxxxxxxxend");

t = new MyRegExp("^start(x{3,5}?)(x*)end$");
testRE(t, "startxxxxxxxxxxend");

t = new MyRegExp("(?=(a+))a*b\\1");
testRE(t, "baaabac");

t = new MyRegExp("^(?:(a)b|(ab))(?:\\2)+$");
testRE(t, "ababab");

t = new MyRegExp("(a*)*q");
testRE(t, "aaq");

t = new MyRegExp("(a*)b(\\1)+");
testRE(t, "baaaac");

