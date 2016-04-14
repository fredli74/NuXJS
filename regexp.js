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
		var c0;
		switch (c0 = s[p]) {
			case '*': ++p; return [ 0, +Infinity ];
			case '+': ++p; return [ 1, +Infinity ];
			case '?': ++p; return [ 0, 1 ];
			// case '{'
		}
	}
	
	function sequence() {
		var c0, c1, c2, sub, n;
		var seq = [ ];
		while (p < e && (!isClass(c0 = s[p], SPECIAL_CHAR) || c0 === '\\')) {
			if (c0 === '\\') {
				if ((c1 = s[p + 1]) !== '0' && isClass(c1, DECIMAL_CHAR)) {
					break;
				} else if (c1 === '0' && !isClass(s[p + 2], DECIMAL_CHAR)) {
					seq.push("\\0");
					p += 2;
				} else if (isClass(c1, ESCAPE_CHAR)) {
					seq.push("\\" + c1);
					p += 2;
				} else if (c1 === 'c' && isClass(c2 = s[p + 2], LETTER_CHAR)) {
					seq.push("\\x" + ("00" + (c2.charCodeAt(0) & 31).toString(16)).slice(-2));
					p += 3;
				} else if (c1 === 'x' && areClass(sub = s.substring(p + 2, p + 4), HEX_CHAR)) {
					seq.push("\\x" + sub);
					p += 4;
				} else if (c1 === 'u' && areClass(sub = s.substring(p + 2, p + 6), HEX_CHAR)) {
					seq.push("\\u" + sub);
					p += 6;
				// } else if (identity
				// CharacterClass
				} else {
					throw "bad"; // FIX
				}
			} else if (isClass(c0, NO_ESCAPE)) {
				seq.push(c0);
				++p;
			} else if (c0 === '"') {
				seq.push('\\"');
				++p;
			} else {
				seq.push("\\u" + ("0000" + c0.charCodeAt(0).toString(16)).slice(-4));
				++p;
			}
		}
		return (seq.length ? seq : null);
	}

	function posToCode(offset) {
		return (offset === 0 ? 'p' : 'p+' + offset);
	}

	function seqToCode(seq, offset) {
		if (seq.length === 0) {
			return "";
		} else if (seq.length === 1) {
			return 's[' + posToCode(offset) + '] === "' + seq[0] + '"';
		} else if (seq.length === 2) {
			return 's[' + posToCode(offset) + '] === "' + seq[0] + '" && s[' + posToCode(offset + 1) + '] === "' + seq[1] + '"';
		} else {
			return 'submatch(s,' + posToCode(offset) + ',"' + seq.join('') + '")';
		}
	}

	var funcs = [ ];

	function and(a, b) {
		return (a === '' ? b : (b === '' ? a : a + ' && ' + b));
	}

	function or(a, b) {
		return (a === '' ? b : a + ' || ' + (b === '' ? true : b));
	}

	function terms() {
		var seq, q, c, code = '', offset = 0;
		while (true) {
			if (seq = sequence()) {
				var repeatCall = '';
				if (q = quantifier()) {
					var rep = seq.pop();
					var tail = terms(tail);
					repeatCall = 'rep(' + posToCode(rep.length) + ')';
					funcs.push('function rep() { return ' + or(and(seqToCode(rep, 0), repeatCall), tail.code) + ' }');
				}
				code = and(and(code, seqToCode(seq, offset)), repeatCall);
				offset += seq.length;
			} else {
				switch (c = s[p]) {
					case '.': ++p; ++offset; break;
					default: return { code: code, minLength: offset, maxLength: offset };
				}
			}
		}
	}

	function disjunction() {
		var code = '', minLength = +Infinity, maxLength = 0;
		while (true) {
			var t = terms();
			code = or(code, t.code);
			minLength = Math.min(minLength, t.minLength);
			maxLength = Math.max(maxLength, t.maxLength);
			if (s[p] === '|') {
				++p;
			} else {
				return { code: code, minLength: minLength, maxLength: maxLength };
			}
		}
	}

	var d = disjunction(); 
	for (var i = 0; i < funcs.length; ++i) print(funcs[i]);
	return d.code;
}
print(compileRegExp("\\n\\rab\\nc\\u1234\\cA\\0\""));
print(compileRegExp("a"));
print(compileRegExp("ade...bfg*"));
print(compileRegExp("a\\n"));
print(compileRegExp("a|bdef.gh|iopq*"));
