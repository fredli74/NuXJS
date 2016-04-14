function compileRegExp(s) {
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

	var b, p = 0, e = s.length;

	function quantifier() {
		switch (s[p]) {
			case '*': ++p; return [ 0, +Infinity ];
			case '+': ++p; return [ 1, +Infinity ];
			case '?': ++p; return [ 0, 1 ];
			// case '{'
		}
	}
	
	function literal() {
		var c0, c1, c2, sub, n;
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
			return 's[' + positionToCode(offset) + '] === "' + literalSequence[0] + '"';
		} else if (literalSequence.length === 2) {
			return 's[' + positionToCode(offset) + '] === "' + literalSequence[0] + '" && s[' + positionToCode(offset + 1) + '] === "' + literalSequence[1] + '"';
		} else {
			return 'submatch(s,' + positionToCode(offset) + ',"' + literalSequence.join('') + '")';
		}
	}

	var funcs = [ ];
	var functionCounter = 0;

	function and(a, b) {
		return (a === '' ? b : (b === '' ? a : a + ' && ' + b));
	}

	function or(a, b) {
		return (a === '' ? '' : a + ' || ') + (b === '' ? true : b);
	}

	function join(a, junction, offset) {
		return (junction === '' ? a : and(a, junction + '(' + positionToCode(offset) + ')'));
	}

	function quantify(code, offset, junction, repeatCode, repeatLength) {
		var functionName = 'f' + (functionCounter++);
		var tail = terms(0, junction);
		funcs.push('function ' + functionName + '(p) { return ' + or(and(repeatCode, functionName + '(' + positionToCode(repeatLength) + ')'), tail.code) + '; }');
		var code = and(code, functionName + '(' + positionToCode(offset) + ')');
		return { code: code, offset: offset };
	}

	function terms(offset, junction) {
		var literalSequence, quantity, c, code = '';
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
					case '.': {
						++p;
						var repeatCall = '';
						if (quantity = quantifier()) {
							return quantify(code, offset + 1, junction, 's[' + positionToCode(0) + '] !== void 0', 1);
						}
						code = and(and(code, 's[' + positionToCode(offset) + '] !== void 0'), repeatCall);
						++offset;
						break;
					}
					case '(': {
						var functionName = 'f' + (functionCounter++);
						++p;
						var d = disjunction(offset, functionName);
						code = and(code, '(' + d.code + ')');
						if (s[p] !== ')') {
							throw "err"; // FIX
						}
						++p;
						var tail = terms(0, junction);
						funcs.push('function ' + functionName + '(p) { return ' + tail.code + '; }');
						return { code: code, offset: offset };
					}
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

	var d = disjunction(0, ''); 
	for (var i = 0; i < funcs.length; ++i) print(funcs[i]);
	return d.code;
}
/*print(compileRegExp("\\n\\rab\\nc\\u1234\\cA\\0\""));
print(compileRegExp("a"));
print(compileRegExp("ade...bfg*"));
print(compileRegExp("a\\n"));
print(compileRegExp("a|bdef.gh|iop.*"));
print(compileRegExp("qwerty(abc|defgh|x)yzzy"));
*/
print(compileRegExp("qwerty(a(bc|papapa)poo*|defgh|x)yzzy.*"));
