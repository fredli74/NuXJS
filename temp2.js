function submatch(s, pos, str) { return (s.substr(pos, str.length) === str); }
function test(s, p, captures) {
var c0,c1,c2,c3;
function g3(p) { return submatch(s,p,"abc") && j4(p+3); }
function j4(p) { return g3(p) || j2(p); }
function g1(p) { var r=c0; return c0=p, g3(p) || (c0=r, false); }
function g7(p) { return submatch(s,p,"abc") && j8(p+3); }
function j8(p) { return j6(p) || g7(p); }
function g5(p) { var r=c2; return c2=p, g7(p) || (c2=r, false); }
function j6(p) { var r=c3; return c3=p, submatch(s,p,"end") || (c3=r, false); }
function j2(p) { var r=c1; return c1=p, g5(p) || (c1=r, false); }
return g1(p)
}

for (var i = 0; i < 1000000; ++i) test('abcabcabcend', 0)
