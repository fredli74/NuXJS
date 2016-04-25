var s = '';

var m = 0.99;
if (Math.floor(m) === m) {
  {
    /*for (var n = 0, p = 1.0; n >= -400 && Math.floor(m / (p * 10) + 0.5) === m / (p * 10); --n) p *= 10;
    s = m / p;*/
        for (var n = 0, p = 1.0; n >= -400 && Math.floor(m * (p * 0.1) + 0.5) === m * (p * 0.1); --n) p *= 0.1;
    s = Math.floor(m * p + 0.5);
  }
} else {
  for (var n = 0, p = 1.0; n < 400 && Math.floor(m * p) !== m * p; ++n) p *= 10;
  s = m * p;
}
for (var k = 0, q = 1.0; k < 400 && s >= 1; ++k) s *= 0.1;
n=k-n;
str='';
for (var i = 0; i < n; ++i) {
  str += String.fromCharCode(48 + Math.floor())
}
document.getElementById('out').innerHTML = JSON.stringify({ m:m, s:s, k:k, n:n, test:s*Math.pow(10, n-k), p:p, q:q, test2:Math.floor(m * (p * 0.1))===(m * (p * 0.1)) });

