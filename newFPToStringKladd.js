function toString(m) {
  "use strict";
  var s = m;
  if (Math.floor(m) === m) {
    var n = 0, p = 1.0, happening = false;;
    do {
      happening = false;
      for (p = 1.0; n >= -400 && Math.floor(s / (p * 10)) === s / (p * 10); --n) { happening = true; p *= 10; }
      s = Math.floor(s / p);
      console.log('s:'+s)
    } while (happening);
  } else {
    for (var n = 0, p = 1.0; n < 400 && Math.floor(m * p) !== m * p; ++n) p *= 10;
    s = Math.floor(m * p);
  }

  console.log('s:' + s);
    for (var k = 1, p = 10.0; k < 400 && s >= p; ++k) p *= 10;
    //p = 10.0 / p;
  s /= p;
  p = 10.0;
  
  n = k - n;
  str = '';
  console.log('k:' + k + ',n:' + n)
  var l = 0.0, nx;
  if (k <= n && n <= 21) {
    for (var i = 0; i < n; ++i) {
      var v = (nx = Math.floor(s * p)) - l * 10;
      str += (i < k ? String.fromCharCode(48 + v) : '0');
      p *= 10;
      l = nx;
    }
  } else if (0 < n && n <= 21) {
    for (var i = 0; i < n; ++i) {
      var v = (nx = Math.floor(s * p)) - l * 10;
      str += String.fromCharCode(48 + v);
      p *= 10;
      l = nx;
    }
    str += '.';
    for (; i < k; ++i) {
      var v = (nx = Math.floor(s * p)) - l * 10;
      str += String.fromCharCode(48 + v);
      p *= 10;
      l = nx;
    }
  } else if (-6 < n && n <= 0) {
    str += '0.';
    for (var i = 0; i < -n; ++i) {
      str += '0';
    }
    for (var i = 0; i < k; ++i) {
      var v = (nx = Math.floor(s * p)) - l * 10;
      str += String.fromCharCode(48 + v);
      p *= 10;
      l = nx;
    }
  } else {
    for (var i = 0; i < 1; ++i) {
      var v = (nx = Math.floor(s * p)) - l * 10;
      str += String.fromCharCode(48 + v);
      p *= 10;
      l = nx;
    }
    if (k > 1) str += '.';
    for (; i < k; ++i) {
      var v = (nx = Math.floor(s * p)) - l * 10;
      str += String.fromCharCode(48 + v);
      p *= 10;
      l = nx;
    }
    str += 'e' + (n - 1 >= 0 ? '+' : '') + (n - 1)
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
  console.log('t:' + str);
  return str;
}

function toNumber(str) {
  "use strict";
  //str="123456789012345670000";
  var div = 1, exp = 1.0;
  for (var i = str.length - 1, d = 0, v = 1; i >= 0; --i) {
    if (str[i] === '-') {
      d = -d;
    } else if (str[i] === '+') {
      ;
    } else if (str[i] === 'e' || str[i] === 'E') {
      exp = Math.floor(Math.pow(10.0, d));
      console.log('exp:'+exp);
      d = 0.0;
      v = 1.0;
    } else if (str[i] === '.') {
      div = v
    } else {
      d += v * (str.charCodeAt(i) - 48);
      v *= 10;
    }
  }
  console.log('d:' + d + ', exp:'+ exp + ',div:'+div);
  return d * (exp / div);
}

console.log('---');
var m = 1e45;
console.log('m:' + m);
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
