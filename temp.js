
//function submatch(s, pos, str) { return (s.substr(pos, str.length) === str) }
/*function submatch(s, pos, str) {
	for (var i = str.length - 1; i >= 0; --i) {
		if (str[i] !== s[i]) {
			return false;
		}
	}
	return true;
}*/
// (abc|abcdef)((def|(x|u(zonk)*e)*)(x|y)|ghi)$

function test(s, pos) {
	function match(pos, str) {
		return submatch(s, pos, str);
	}
	function alt1(pos) { // (x|y)$
		return s[pos] === 'x' && s[pos + 1] === void 0
				|| s[pos] === 'y' && s[pos + 1] === void 0;
	}
	function rep1(pos) { // (zonk)*e)*(x|y)$
		return match(pos, "zonk") && rep1(pos + 4)
				|| s[pos] === 'e' && rep0(pos + 1);
	}
	function rep0(pos) { // (x|(u(zonk)*e)*)(x|y)$
		return s[pos] === 'x' && rep0(pos + 1)
				|| s[pos] === 'u' && rep1(pos + 1)
				|| alt1(pos);
	}
	function alt0(pos) { // ((def|(x|u(zonk)*e)*)(x|y)|ghi)$ 
		return (match(pos, "def") && alt1(pos + 3)
				|| rep0(pos))
				|| match(pos, "ghi") && s[pos + 3] === void 0;
	}
	return match(pos, "abc") && alt0(pos + 3)
			|| match(pos, "abcdef") && alt0(pos + 6);
}
function tezt(s) { print(s + ': ' + test(s, 0)); }
tezt('abc');
tezt('abcdef');
tezt('abcdefghi');
tezt('abcdefghij');
tezt('abcdefx');
tezt('abcdefy');
tezt('abcx');
tezt('abcxx');
tezt('abcxxxxxxx');
tezt('abcxy');
tezt('abcxxxxxy');
tezt('abcdefxxxxxy');
tezt('abcdefuffx');
tezt('abcdefzonkzonk');
tezt('abcdefzonkzonky');
tezt('abcdefzonkzon');
tezt('abcdefzonkxzonk');
tezt('abcdefzonkxzonkx');
tezt('abcdefuzonkeuzonke');
tezt('abcdefuzonkeuzonkey');
tezt('abcdefuey');
tezt('abcdefuexuzonkexx');

for (var i = 0; i < 100000; ++i) test('abcdefxxxxxy', 0)





