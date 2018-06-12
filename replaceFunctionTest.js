function $substring(s, b, e) { return s.slice(b, e); }

s = 'bangla dangla hoopa';
r = "hello $$ $$$ $$$$ $& <- replaced  capture 1 -> $1 $0 $2 $3, again: $01 $001 $04 $00.. up until: $`, rest: $'";

function matchOffset(args) { return args[args.length - 2]; }
function capture(args, i) { var c; return (i >= 1 && i < args.length - 2 ? ((c = args[i]) === void 0 ? '' : c) : null); }
replaceFunction = function(m) {
	var t = '', p = 0, ch, ch2, c, args;
	while ((ch = r[p]) !== void 0) {
		if (ch === '$') {
			switch (ch = r[++p]) {
				case '$': t += '$'; break;
				case '&': t += m; break;
				case '`': t += $substring(s, 0, matchOffset(arguments)); break;
				case "'": t += $substring(s, matchOffset(arguments) + m.length, s.length); break;
				default: {
					if (ch >= '0' && ch <= '9') {
						if ((ch2 = r[p + 1]) >= '0' && ch2 <= '9') {
							if ((c = capture(arguments, +(ch + ch2))) !== null) {
								++p;
								t += c;
								break;
							}
						} else {
							if ((c = capture(arguments, +ch)) !== null) {
								t += c;
								break;
							}
						}
					}
					t += '$' + ch;
					break;
				}
			}
		} else {
			t += ch;
		}
		++p;
	}
	return t;
}

print(replaceFunction.apply(null, ["dangla", "dang", void 0, "honk", 7, s]));
