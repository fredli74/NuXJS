function match(pattern, text) {
	if (pattern.length === 0) {
		return true;
	}
	if (pattern[0] === '*') {
		for (var i = 0; i < text.length; ++i) {
			if (match(pattern.substr(1), text.substr(i))) {
				return true;
			}
		}
		return false;
	}
	switch (pattern[0]) {
		default: {
			if (pattern[0] !== text[0]) {
				return false;
			} else {
				return match(pattern.substr(1), text.substr(1));
			}
		}
		case '.': {
			return true;
		}
	}
}

(.*)(korv|ballong){2,5}ballongmos

b0 = o;
for (var o0 = 0; o0 < len; ++o0) {
	o = o0;
	b1 = o;
	for (var i1 = 0; i1 < 5; ++i1) {
		if (submatch(s, o, 4, "korv")) {
			o += 4;
		}
		else if (submatch(s, o, 7, "ballong")) {
			o += 7;
		}
		else break;
	}
	if (i >= 2) break;
	o = b;
}

korvballongkorvballongmos
                      ^
               ^


(abc|abcdef)def$

disjunction0: for (var alt0 = 0; alt0 < 2; ++alt0) {
	switch (alt0) {
		case 0: if (!submatch(s, pos0, "abc")) continue disjunction0; pos1 = pos0 + 3; break;
		case 1: if (!submatch(s, pos0, "abcdef")) continue disjunction0; pos1 = pos0 + 6; break;
	}
	if (!submatch(s, pos1, "def")) continue disjunction0; pos2 = pos1 + 3;
	if (pos2 !== len) continue disjunction0; pos3 = pos2 + 0;
	return true;
}
return false;


(abc|abcdef)(def|ghi)$

for (var alt0 = 0; alt0 < 2; ++alt0) {
	switch (alt0) {
		case 0: if (!submatch(s, pos0, "abc")) continue; pos1 = pos0 + 3; break;
		case 1: if (!submatch(s, pos0, "abcdef")) continue; pos1 = pos0 + 6; break;
	}
	for (var alt1 = 0; alt1 < 2; ++alt1) {
		switch (alt1) {
			case 0: if (!submatch(s, pos1, "def")) continue; pos2 = pos1 + 3; break;
			case 1: if (!submatch(s, pos1, "ghi")) continue; pos2 = pos1 + 3; break;
		}
		if (pos2 !== len) continue; pos3 = pos2 + 0;
		return true;
	}
}
return false;


(abc|abcdef)((def|q+)x|ghi)$

function submatch(s, pos, str) { return (s.substr(pos, str.length) === str) }
/*function submatch(s, pos, str) {
	for (var i = str.length - 1; i >= 0; --i) {
		if (str[i] !== s[i]) {
			return false;
		}
	}
	return true;
}*/
function test(s, pos0) {
	var m = submatch;
	var len = s.length;
	var pos1, pos2, pos3, n, alt0, alt1, alt2, alt3
	for (alt0 = 0; alt0 < 2; ++alt0) {
		switch (alt0) {
			case 0: if (!m(s, pos0, "abc")) continue; pos1 = pos0 + 3; break;
			case 1: if (!m(s, pos0, "abcdef")) continue; pos1 = pos0 + 6; break;
		}
		for (alt1 = 0; alt1 < 2; ++alt1) {
			switch (alt1) {
				case 0: {
					for (alt2 = 0; alt2 < 2; ++alt2) {
						switch (alt2) {
							case 0: if (!m(s, pos1, "def")) continue; pos3 = pos1 + 3; break;
							case 1: {
								pos3 = pos1;
								n = 0;
								while (pos3 < len) {
									if (s[pos3] !== 'q') break;
									pos3 = pos3 + 1;
									++n;
								}
								if (n < 1) continue;
								break;
							}
						}
						if (s[pos3] !== 'x') continue; pos2 = pos3 + 1;
						break;
					}
					if (alt2 === 2) continue;
					break;
				}
				case 1: if (!m(s, pos1, "ghi")) continue; pos2 = pos1 + 3; break;
			}
			if (pos2 !== len) continue; pos3 = pos2 + 0;
			break;
		}
		if (alt1 === 2) continue;
		break;
	}
	return alt0 !== 2;
}




abcdefdef


(aba|abacda)cda

abacdacda




(abc|abcdef)((def|(x|zonk)*)(x|y)|ghi)$
 
function submatch(s, pos, str) { return (s.substr(pos, str.length) === str) }
/*function submatch(s, pos, str) {
	for (var i = str.length - 1; i >= 0; --i) {
		if (str[i] !== s[i]) {
			return false;
		}
	}
	return true;
}*/
function test(s, pos) {
	function match(pos, str) {
		return submatch(s, pos, str);
	}
	function rep0(pos) {
		return alt1(pos)
				|| s[pos] === 'x' && rep0(pos + 1)
				|| match(pos, "zonk") && rep0(pos + 4);
	}
	function alt1(pos) {
		return s[pos] === 'x' && s[pos + 1] === void 0
				|| s[pos] === 'y' && s[pos + 1] === void 0;
	}
	function alt0(pos) {
		return (match(pos, "def") && alt1(pos + 3)
				|| rep0(pos))
				|| match(pos, "ghi") && s[pos + 3] === void 0;
	}
	return match(pos, "abc") && alt0(pos + 3)
			|| match(pos, "abcdef") && alt0(pos + 6);
}
print(test('abc', 0));
print(test('abcdef', 0));
print(test('abcdefghi', 0));
print(test('abcdefghij', 0));
print(test('abcdefx', 0));
print(test('abcdefy', 0));
print(test('abcx', 0));
print(test('abcxx', 0));
print(test('abcxxxxxxx', 0));
print(test('abcxy', 0));
print(test('abcxxxxxy', 0));
print(test('abcdefxxxxxy', 0));



function rep1(pos) { // (zonk)*e)*(x|y)$
    while (!(s[pos] === 'e' && rep0(pos + 1))) {
        if (match(pos, "zonk")) pos += 4;
        else return false;
    }
    return true;
}
function rep0(pos) { // (x|(u(zonk)*e)*)(x|y)$
    while (!alt1(pos)) {
		if (s[pos] === 'x') pos += 1;
		if (s[pos] === 'u' && rep1(pos + 1)) {
			return true;
		}
    }
   	return true;
}


		return alt1(pos)
				|| s[pos] === 'x' && rep0(pos + 1)
				|| s[pos] === 'u' && rep1(pos + 1)


(a*)*

function rep1(pos) {
	return s[pos] === 'a' && rep1(pos + 1)
			|| rep0(pos);
}

function rep0(pos) {
	return rep1(pos);
}



move ((white)|red) piece

function junc1(pos) {
	return captures[1] = pos, match(pos, ' piece');
}

return match(pos, 'move ') && captures[0] = pos + 5, (captures[2] = pos + 5, match(pos + 5, 'white') && captures[3] = pos + 10, junc1(pos + 10) || captures.length = 2, match(pos + 5, 'red') && junc1(pos + 8))


// (abc|abcdef)((def|(x|u(zonk)*e)*)(x|y)|ghi)$
   0          124    6   8    9  7 51   1    3
                                    0   1
function test(s, pos, captures) {
	function match(pos, str) {
		return submatch(s, pos, str);
	}
	function junction3(pos) {
		return captures[3] = pos, s[pos] === void 0;
	}
	function junction2(pos) {
		return captures[11] = pos, junction3(pos);
	}
	function junction1(pos) { // (x|y)$
		return captures[5] = pos, captures[10] = pos, s[pos] === 'x' && junction2(pos + 1)
				|| s[pos] === 'y' && junction2(pos + 1);
	}
	function rep1(pos) { // (zonk)*e)*(x|y)$
		return captures.length = 9, captures[8] = pos, match(pos, "zonk") && rep1(pos + 4)
				|| s[pos] === 'e' && rep0(pos + 1);
	}
	function rep0(pos) { // (x|(u(zonk)*e)*)(x|y)$
		return captures.length = 7, captures[6] = pos, (s[pos] === 'x' && rep0(pos + 1)
				|| captures.length = 7, s[pos] === 'u' && rep1(pos + 1)
				|| junction1(pos));
	}
	function junction0(pos) { // ((def|(x|u(zonk)*e)*)(x|y)|ghi)$ 
		return captures[1] = pos, captures[2] = pos, (captures[4] = pos, (match(pos, "def") && junction1(pos + 3)
				|| captures.length = 5, rep0(pos))
				|| captures.length = 3, match(pos, "ghi") && junction3(pos + 3));
	}
	return captures[0] = pos, (match(pos, "abc") && junc0(pos + 3)
			|| captures.length = 1, match(pos, "abcdef") && junc0(pos + 6));
}


/*
      start((abc|de)*end)*end
repeat     01     
junction              1    0
capture    02       3    1

startabcabcdefendabcendendabcend
*/

function submatch(s, pos, str) { return (s.substr(pos, str.length) === str); }

function test(s, pos, captures) {
	function match(pos, str) {
		return submatch(s, pos, str);
	}
	function junction0(pos) {
		return match(pos, 'end');
	}
	function junction1(pos) {
		return match(pos, 'end') && repeat0(pos + 3);
	}
	function repeat1(pos) {
        var prev2 = captures[2], prev3 = captures[3];
		return match(pos, 'abc') && (captures[2] = pos, repeat1(pos + 3) || (captures[2] = prev2, false))
				|| match(pos, 'de') && (captures[2] = pos, repeat1(pos + 2) || (captures[2] = prev2, false))
				|| (captures[3] = pos, junction1(pos) || (captures[3] = prev3, false));
	}
	function repeat0(pos) {
		return repeat1(pos) || junction0(pos);
	}
	return match(pos, 'start') && repeat0(pos + 5);
}

function tezt(s) {
    var captures = [ ];
	console.log(s + " : " + RegExp("start((abc|de)*end)*end").test(s) + ", " + test(s, 0, captures));
    console.log(captures);
}

tezt("end");
tezt("start");
tezt("startend");
tezt("startdeend");
tezt("startdeendend");
tezt("startabcabcdeenddeendendabcend");







/*
      start((abc|de)*end)*end
repeat     01     
junction             1    0
capture    02      3    1

startabcabcdefendabcendendabcend
*/

function submatch(s, pos, str) { return (s.substr(pos, str.length) === str); }

function test(s, pos, captures) {
	function match(pos, str) {
		return submatch(s, pos, str);
	}
	function repeat1(pos) {
        var prev2 = captures[2];
        captures[2] = pos;
		var success = match(pos, 'abc') && repeat1(pos + 3)
				|| match(pos, 'de') && repeat1(pos + 2);
        if (!success) {
          captures[2] = prev2;
          var prev3 = captures[3];
          captures[3] = pos;
          success = match(pos, 'end') && repeat0(pos + 3);
          if (!success) captures[3] = prev3;
        }
        return success;
	}
	function repeat0(pos) {
        var prev0 = captures[0];
        captures[0] = pos;
		var success = repeat1(pos);
        if (!success) {
          captures[0] = prev0;
          var prev1 = captures[1];
          captures[1] = pos;
          success = match(pos, 'end');
          if (!success) captures[1] = prev1;
        }
        return success;
	}
	return match(pos, 'start') && repeat0(pos + 5);
}

function tezt(s) {
    var captures = [ ];
	console.log(s + " : " + RegExp("start((abc|de)*end)*end").test(s) + ", " + test(s, 0, captures));
    console.log(captures);
}

tezt("end");
tezt("start");
tezt("startend");
tezt("startdeend");
tezt("startdeendend");
tezt("startabcabcdeenddeendendabcend");



/*
      start((abc|de)*end)*end
repeat     01     
capture    02      3    1

startabcabcdefendabcendendabcend
*/

function submatch(s, pos, str) { return (s.substr(pos, str.length) === str); }

function test(s, pos, captures) {
	function match(pos, str) {
		return submatch(s, pos, str);
	}
/*	function repeat1(pos) {
        var prev2 = captures[2];
        captures[2] = pos;
		var success = match(pos, 'abc') && repeat1(pos + 3)
				|| match(pos, 'de') && repeat1(pos + 2);
        if (!success) {
          captures[2] = prev2;
          var prev3 = captures[3];
          captures[3] = pos;
          success = match(pos, 'end') && repeat0(pos + 3);
          if (!success) captures[3] = prev3;
        }
        return success;
	}*/
	function repeat1(pos) {
        var rollback2 = captures[2];
        captures[2] = pos;
		if (match(pos, 'abc') && repeat1(pos + 3)
				|| match(pos, 'de') && repeat1(pos + 2)) {
          return true;
        }
        captures[2] = rollback2;
        var rollback3 = captures[3];
        captures[3] = pos;
        if (match(pos, 'end') && repeat0(pos + 3)) return true;
        captures[3] = rollback3;
        return false;
	}
	
	function repeat0(pos) {
        var prev0 = captures[0];
        captures[0] = pos;
		var success = repeat1(pos);
        if (!success) {
          captures[0] = prev0;
          var prev1 = captures[1];
          captures[1] = pos;
          success = match(pos, 'end');
          if (!success) captures[1] = prev1;
        }
        return success;
	}
	return match(pos, 'start') && repeat0(pos + 5);
}

function tezt(s) {
    var captures = [ ];
    var m = s.match("start((abc|de)*end)*end");
	console.log(s + " : " + (m !== null) + ", " + test(s, 0, captures));
    if (m !== null) {
      for (var i = 0; i < captures.length; i += 2) {
        console.log(i / 2 + ': ' + m[1 + i / 2] + ', ' + (captures[i] === void 0 ? 'undefined' : s.substring(captures[i], captures[i + 1])));
      }
    }
}

tezt("end");
tezt("start");
tezt("startend");
tezt("startdeend");
tezt("startdeendend");
tezt("startabcabcabcendend");
tezt("startabcabcdeenddeendendabcend");
tezt("startabcabcdeenddeendendabcendend");





function test(s, pos, captures) {
	function match(pos, str) {
		return submatch(s, pos, str);
	}
	function repeat1(pos) {
        var rollback;
        return (rollback = captures[2], captures[2] = pos, match(pos, 'abc') && repeat1(pos + 3) || match(pos, 'de') && repeat1(pos + 2))
        	|| (captures[2] = rollback, rollback = captures[3], captures[3] = pos, match(pos, 'end') && repeat0(pos + 3))
        	|| (captures[3] = rollback, false);
	}
	function repeat0(pos) {
        var rollback;
        return (rollback = captures[0], captures[0] = pos, repeat1(pos))
        	|| (captures[0] = rollback, rollback = captures[1], captures[1] = pos, match(pos, 'end'))
        	|| (captures[1] = rollback, false);
	}
	return match(pos, 'start') && repeat0(pos + 5);
}

function tezt(s) {
    var captures = [ ];
    var m = s.match("start((abc|de)*end)*end");
	console.log(s + " : " + (m !== null) + ", " + test(s, 0, captures));
    if (m !== null) {
      for (var i = 0; i < captures.length; i += 2) {
        console.log(i / 2 + ': ' + m[1 + i / 2] + ', ' + (captures[i] === void 0 ? 'undefined' : s.substring(captures[i], captures[i + 1])));
      }
    }
}

tezt("end");
tezt("start");
tezt("startend");
tezt("startdeend");
tezt("startdeendend");
tezt("startabcabcabcendend");
tezt("startabcabcdeenddeendendabcend");
tezt("startabcabcdeenddeendendabcendend");



.*e


function repeat0(pos) {
	return s[pos] !== void 0 && repeat0(pos + 1) || s[pos] === 'e';
}



single char: s[p] === char && rest(p + 1)
multiple chars: match(p, <chars>) && rest(p + <chars>.length)
any char (.): s[p] !== void 0 && rest(p + 1)
end char ($): s[p] === void 0 && rest(p)
a|b: a || b

function compileRegExp(s) {
	var NO_ESCAPE = 1, SPECIAL_CHAR = 2, DECIMAL_CHAR = 4, ESCAPE_CHAR = 8, LETTER_CHAR = 16, CHAR_CLASSES = { };

	function setupCharClass(mask, chars) {
		for (var i in chars) CHAR_CLASSES[chars[i]] = mask;
	}
	function isClass(char, mask) {
		return ((CHAR_CLASSES[char] & mask) !== 0);
	}
	setupCharClass(SPECIAL_CHAR, "\\^$.*+?()[]{}|");
	setupCharClass(DECIMAL_CHAR, "0123456789");
	setupCharClass(ESCAPE_CHAR, "fnrtv");
	setupCharClass(LETTER_CHAR, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	for (var i = 32; i <= 126) CHAR_CLASSES[i] |= NO_ESCAPE;
// [\0-\60\72-\101\133-\137\140\173-\180]

	var b, p = 0, e = source.length;
	
	function literal() {
		var c0, c1, c2, n;
		var escaped = '';
		while (p < e && !isClass(c0 = s[p], SPECIAL_CHAR) || c0 === '\\')) {
			if (c0 === '\\') {
				if ((c1 = s[p + 1]) === '0' && !isClass(s[p + 2], DECIMAL_CHAR)) {
					escaped += "\\0";
					p += 2;
				} else if (isClass(c1, DECIMAL_CHAR)) {
					return escaped;
				} else if (isClass(c1, ESCAPE_CHAR)) {
					escaped += "\\" + c1;
					p += 2;
				} else if (c1 === 'c' && isClass(c2 = s[p + 2], LETTER_CHAR)) {
					escaped += "\\x" + c2.charCodeAt(0) & 31).toString(16);
					p += 3;
				} else if (c1 === 'x') {
					escaped += "\\x" + s[p + 2] + s[p + 3];
					p += 4;
				} else if (c1 === 'u') {
					escaped += "\\u" + s.substring(p + 2, p + 6);
					p += 6;
				// } else if (identity
				} else {
					throw "bad" // FIX
				}
			} else if (isClass(c0, NO_ESCAPE)) {
				escaped += c0;
			} else {
				escaped += "\\u" + ("0000" + n.toString(16)).slice(-4);
			}
		}
		return escaped;
	}

	function matchRule() {
		switch (s[p]) {
			default: {

				return "submatch(s, pos, '" + 
	}

	while (p < e) {
		pattern();
}
