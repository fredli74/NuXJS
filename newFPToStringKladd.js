function toString(m) {
    "use strict";
    var s;
    if (Math.floor(m) === m) {
        {
            if (true) {
                for (var n = 0, p = 1.0; n >= -400 && Math.floor(m / (p * 10)) === m / (p * 10); --n) p *= 10;
                  s = Math.floor(m / p);
            } else {
                for (var n = 0, p = 1.0; n >= -400 && Math.floor(m * (p * 0.1)) === m * (p * 0.1); --n) p *= 0.1;
                 s = Math.floor(m * p + 0.5);
            }
        }
      console.log('s:'+s);
        for (var k = 1,p=1; k < 400 && (s / p) >= 10.0; ++k) p *= 10;
    } else {
        for (var n = 0, p = 1.0; n < 400 && Math.floor(m * p) !== m * p; ++n) p *= 10;
      console.log(m*p);
        for (var k = 1; k < 400 && m * p >= 10.0; ++k) p *= 0.1;
    }
    n = k - n;
    str = '';
  console.log('k:'+k+',n:'+n)
    if (k <= n && n <= 21) {
        var l = 0.0;
        for (var i = n - 1, p=1; i >= 0; --i) {
            if (false) {
              str += (i < k ? String.fromCharCode(48 + Math.floor((s / p) % 10)) : '0');
              p /= 10;
            } else {
              str = (i < k ? String.fromCharCode(48 + (s * p) % 10) : '0') + str;
              if (i < k) p *= 0.1;
            }
        }
    } else if (0 < n && n <= 21) {
        for (var i = 0; i < n; ++i) {
           str += (i < k ? String.fromCharCode(48 + Math.floor((m / p) % 10)) : '0');
              p /= 10;
        }
        str += '.';
        for (;i < k; ++i) {
           str += (i < k ? String.fromCharCode(48 + Math.floor((m / p) % 10)) : '0');
              p /= 10;
        }
    }
 /*   document.getElementById('out').innerHTML = JSON.stringify({
        m: m,
        str: str,
        s: s,
        k: k,
        n: n,
        test: s * Math.pow(10, n - k),
        p: p,
        test2: Math.floor(m * (p * 0.1)) === (m * (p * 0.1))
    });*/
    console.log('t:'+str);
    return str;
}

function toNumber(str) {
    "use strict";
	//str="123456789012345670000";
  var div = 1;
	for (var i = str.length - 1, d = 0, v = 1; i >= 0; --i) {
      if (str[i] === '.') { div = v }
      else {
	    d += v * (str.charCodeAt(i) - 48);
	    v *= 10;
      }
	}
  	console.log('d:'+d/div);
  return d/div;
}

console.log('---');
var m= 1.234567890123456785e19;
console.log('m:'+m);
var str = toString(m);
var num = toNumber(str);
var str = toString(num);
var num = toNumber(str);
var str = toString(num);
console.log(str);
/*
v = 1;
for (i = 0; i < 100; ++i) { console.log(i +':'+(v===eval("1.0e"+i))); v*=10; }*/
/*
v = 1;
for (i = 0; i < 100; ++i) { console.log(i +':'+v+'='+(v===eval("1.0e-"+i))); v*=0.1; }
*/