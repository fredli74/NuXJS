function compileRegExp(s) {
	print('');
	print(s);
	print("-------------");
	var NO_ESCAPE = 1, SPECIAL_CHAR = 2, DECIMAL_CHAR = 4, HEX_CHAR = 8, ESCAPE_CHAR = 16, LETTER_CHAR = 32;
	var CHAR_CLASSES = { };

	function setupCharClass(mask, chars) {
		for (var i in chars) CHAR_CLASSES[chars[i]] |= mask;
	}
	function isClass(char, mask) {
		return ((CHAR_CLASSES[char] & mask) !== 0);
	}
	function areClass(str, mask) {
		for (var i = str.length - 1; i >= 0; --i) {
			if ((CHAR_CLASSES[str[i]] & mask) === 0) {
				return false;
			}
		}
		return true;
	}
	setupCharClass(SPECIAL_CHAR, "\\^$.*+?()[]{}|");
	setupCharClass(DECIMAL_CHAR, "0123456789");
	setupCharClass(HEX_CHAR, "0123456789abcdefABCDEF");
	setupCharClass(ESCAPE_CHAR, "fnrtv");
	setupCharClass(LETTER_CHAR, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	for (var i = 32; i <= 126; ++i) {
		var c = String.fromCharCode(i);
		if (c !== '"') {
			CHAR_CLASSES[String.fromCharCode(i)] |= NO_ESCAPE;
		}
	}
// [\0-\60\72-\101\133-\137\140\173-\180]

	var p = 0, e = s.length;

	function quantifier() {
		var q;
		switch (s[p]) {
			case '*': ++p; q = { min: 0, max: +Infinity, greedy: true }; break;
			case '+': ++p; q = { min: 1, max: +Infinity, greedy: true }; break;
			case '?': ++p; q = { min: 0, max: 1, greedy: true }; break;
			// case '{'
			default: return null;
		}
		if (s[p] === '?') {
			++p;
			q.greedy = false;
		}
		return q;
	}
	
	function literal() {
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
				// CharacterClass
				} else {
					throw "bad"; // FIX
				}
			} else if (isClass(c0, NO_ESCAPE)) {
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

	var funcs = [ ];
	var functionCounter = 0;
	var captureCounter = 0;

	function and(a, b) {
		return (a === '' ? b : (b === '' ? a : a + ' && ' + b));
	}

	function or(a, b) {
		return (a === '' ? '' : a + ' || ') + (b === '' ? true : b);
	}

	function join(a, junction, offset) { // FIX : no sub
		return (junction === '' ? a : and(a, junction + '(' + positionToCode(offset) + ')'));
	}

	function quantify(code, offset, junction, repeatCode, repeatLength) {
		var functionName = 'f' + (functionCounter++);
		var tail = terms(0, junction);
		funcs.push('function ' + functionName + '(p) { return ' + or(and(repeatCode, functionName + '(' + positionToCode(repeatLength) + ')'), tail.code) + '; }');
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

	function terms(offset, junction) {
		var literalSequence, quantity, code = '';
		while (true) {
			if (literalSequence = literal()) {
				if (quantity = quantifier()) {
					var repeatLiteral = literalSequence.pop();
					return quantify(and(code, literalSequenceToCode(literalSequence, offset))
							, offset + literalSequence.length, junction, literalSequenceToCode(repeatLiteral, 0)
							, repeatLiteral.length);
				}
				code = and(code, literalSequenceToCode(literalSequence, offset));
				offset += literalSequence.length;
			} else {
				switch (s[p]) {
					case '*': case '+': case '?': case '{': throw "Nothing to repeat"; // FIX
					case '$': {
						++p;
						code = and(code, positionToCode(offset) + '===l');
						break;
					}
					case '.': {
						++p;
						if (quantity = quantifier()) {
							return quantify(code, offset, junction, positionToCode(0) + ' < l', 1);
						}
						code = and(code, positionToCode(offset) + ' < l');
						++offset;
						break;
					}
					case '(': {
						var groupName = 'g' + (++functionCounter);
						var junctionName = 'j' + (++functionCounter);
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
						var disjunc = disjunction(0, junctionName);
						if (doCapture) {
							openCaptureTo = captureCounter * 2;
						}
						if (s[p] !== ')') {
							throw "err"; // FIX
						}
						++p;
						var quantity = quantifier();
						var tail = terms(0, junction);
						funcs.push('function ' + groupName + '(p) { ' + captureWrap(openCaptureFrom, openCaptureTo, or('', disjunc.code)) + '; }');
						if (quantity && quantity.max > 1) {
							var recursionCall = groupName + '(' + positionToCode(0) + ')';
							var inner = (quantity.greedy ? or(recursionCall, tail.code) : or(tail.code, recursionCall));
							funcs.push('function ' + junctionName + '(p) { '+ captureWrap(closeCaptureFrom, closeCaptureTo, inner) + '; }');
							code = and(code, (quantity.min === 0 ? junctionName : groupName) + '(' + positionToCode(offset) + ')');
						} else {
							funcs.push('function ' + junctionName + '(p) { ' + captureWrap(closeCaptureFrom, closeCaptureTo, or('', tail.code)) + '; }');
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
					case void 0: code = and(code, '(e=' + positionToCode(offset) + ',true)'); /* fall through */
					default: return { code: join(code, junction, offset), offset: offset };
				}
			}
		}
	}

	function disjunction(offset, junction) {
		var code = '', minOffset = +Infinity, maxOffset = 0;
		while (true) {
			var t = terms(offset, junction);
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

	var disjunc = disjunction(0, ''); 
	if (p < e) throw "error"; // FIX
	var s = "function test(s, p) {\n\tvar l=s.length,e";
	for (var i = 0; i < captureCounter * 2; ++i) s += ",c" + i;
	s += ";\n";
	for (var i = 0; i < funcs.length; ++i) s += "\t" + funcs[i] + "\n";
	s += "\treturn [ (" + disjunc.code + ') ,e';
	for (var i = 0; i < captureCounter * 2; ++i) s += ",c" + i;
	s += "];\n}\n";
	return s;
}
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
