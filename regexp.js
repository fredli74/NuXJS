"use strict";

var OK_IN_STRING_LITERAL = 1, SPECIAL_CHAR = 2, DECIMAL_CHAR = 4, HEX_CHAR = 8, ESCAPE_CHAR = 16, LETTER_CHAR = 32, NEWLINE_CHAR = 64, SPACE_CHAR = 128, WORD_CHAR = 256, EMPTY_CHAR = 512; // FIX : sort, make array?
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

function compileRegExp(s) {
// [\0-\60\72-\101\133-\137\140\173-\180]

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
				if ((min = max = parseNumber(+void 0)) !== min) { // !isNaN
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
			return "";
		} else if (literalSequence.length === 1) {
			return 's[' + positionToCode(offset) + ']==="' + literalSequence[0] + '"';
		} else if (literalSequence.length === 2) {
			return 's[' + positionToCode(offset) + ']==="' + literalSequence[0] + '" && s[' + positionToCode(offset + 1) + ']==="' + literalSequence[1] + '"';
		} else {
			return 'submatch(s,' + positionToCode(offset) + ',"' + literalSequence.join('') + '")';
		}
	}

	var functions = '';
	var functionCounter = 0;
	var captureCounter = 0;
	var initCounters = '';

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

	function join(a, junction, offset) { // FIX : no sub
		return (junction === '' ? a : and(a, junction + '(' + positionToCode(offset) + ')'));
	}
	
	function addFunction(definition) {
		functions += "\tfunction " + definition + "\n";
	}

	function quantify(code, offset, junction, repeatCode, repeatLength) {
		var functionName = 'f' + (functionCounter++);
		var tail = compileTerms(0, junction);
		addFunction(functionName + '(p) { return ' + or(and(repeatCode, functionName + '(' + positionToCode(repeatLength) + ')'), tail.code) + '; }');
		return { code: and(code, functionName + '(' + positionToCode(offset) + ')'), offset: offset };
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
			return declares + '; return ' + captures + ', ' + code + ' || (' + rollbacks + ',false)';
		}
	}

	function charClassToCode(offset, mask, invert) {
		return (invert ? '!' : '!!') + '(CC[s[' + positionToCode(offset) + ']]&' + (invert ? mask | EMPTY_CHAR : mask) + ')';
	}

	function compileTerms(offset, junction) {
		var literalSequence, quantity, code = 'true', c;
		termLoop: for (;;) {
			if (literalSequence = parseLiteral()) {
				if (quantity = parseQuantifier()) {
					var repeatLiteral = literalSequence.pop();
					return quantify(and(code, literalSequenceToCode(literalSequence, offset))
							, offset + literalSequence.length, junction, literalSequenceToCode(repeatLiteral, 0)
							, repeatLiteral.length);
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
					case '\\': c = s[++p]; // fall-through
					case '.': {
						var rule;
						if (!(rule = CHAR_CLASS_RULES[c])) {
							break termLoop;
						}
						++p;
						if (quantity = parseQuantifier()) {
							return quantify(code, offset, junction, charClassToCode(0, rule[0], rule[1]), 1);
						}
						code = and(code, charClassToCode(offset, rule[0], rule[1]));
						++offset;
						break;
					}
					case '(': {
						var thisFunctionNumber = ++functionCounter;
						var groupName = 'g' + thisFunctionNumber;
						var junctionName = 'j' + thisFunctionNumber;
						var doCapture = true;
						++p;
						if (s[p] === '?') {
							switch (s[++p]) {
								case ':': doCapture = false; break;
								default: throw "error"; // FIX
							}
							++p;
						}
						var openCaptureFrom = null, openCaptureTo = null, closeCaptureFrom = null, closeCaptureTo = null;
						if (doCapture) {
							closeCaptureTo = (closeCaptureFrom = (openCaptureFrom = (captureCounter++) * 2) + 1) + 2;
						}
						var disjunc = compileDisjunction(0, junctionName);
						if (doCapture) {
							openCaptureTo = captureCounter * 2;
						}
						if (s[p] !== ')') {
							throw SyntaxError("Unterminated group in regular expression");
						}
						++p;
						var quantity = parseQuantifier();
						var tail = compileTerms(0, junction);
						var inner = disjunc.code;
						var tearDown = '';
						if (quantity && (quantity.max > 1 && quantity.max < Infinity || quantity.min > 1)) {
							initCounters += ',n' + thisFunctionNumber + '=0';
							if (quantity.max < Infinity) {
								inner = and('++n' + thisFunctionNumber + '<=' + quantity.max, inner);
							}
							if (quantity.min > 1) {
								inner = and(inner, 'n' + thisFunctionNumber + '>=' + quantity.min, inner);
							}
							inner = or(inner, '(--n' + thisFunctionNumber + ',false)');
						}
						addFunction(groupName + '(p) { ' + captureWrap(openCaptureFrom, openCaptureTo, inner) + '; }');
						if (quantity && quantity.max > 1) {
							var recursionCall = groupName + '(' + positionToCode(0) + ')';
							var inner = (quantity.greedy ? or(recursionCall, tail.code) : or(tail.code, recursionCall));
							addFunction(junctionName + '(p) { '+ captureWrap(closeCaptureFrom, closeCaptureTo, inner) + '; }');
							code = and(code, (quantity.min === 0 ? junctionName : groupName) + '(' + positionToCode(offset) + ')');
						} else {
							addFunction(junctionName + '(p) { ' + captureWrap(closeCaptureFrom, closeCaptureTo, tail.code) + '; }');
							var groupCall = groupName + '(' + positionToCode(offset) + ')';
							if (quantity && quantity.min === 0) {
								var junctionCall = junctionName + '(' + positionToCode(offset) + ')';
								code = and(code, '(' + (quantity.greedy ? or(groupCall, junctionCall) : or(junctionCall, groupCall)) + ')');
							} else {
								code = and(code, groupCall);
							}
						}
						return { code: code, offset: offset };
					}
					default: break termLoop;
				}
			}
		}
		return { code: (junction === '' ? code : and(code, junction + '(' + positionToCode(offset) + ')')), offset: offset };
	}

	function compileDisjunction(offset, junction) {
		var code = 'false', minOffset = Infinity, maxOffset = 0;
		for (;;) {
			var t = compileTerms(offset, junction);
			code = or(code, t.code);
			minOffset = Math.min(minOffset, t.offset);
			maxOffset = Math.max(maxOffset, t.offset);
			if (s[p] === '|') {
				++p;
			} else {
				return { code: code, minOffset: minOffset, maxOffset: maxOffset };
			}
		}
	}

	var disjunc = compileDisjunction(0, ''); 
	if (p < e) {
		throw SyntaxError("Invalid regular expression");
	}
	var code = "(function(s, p) {\n\tvar l=s.length,e";
	for (var i = 0; i < captureCounter * 2; ++i) code += ",c" + i;
	code += initCounters + ";\n" + functions + "\tif (" + disjunc.code + ") return [p,e";
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

t = new MyRegExp("(?:abc)def");
print(t.match);

t = new MyRegExp("(?:abc)def");
print(t.match);

