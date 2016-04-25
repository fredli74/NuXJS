var s = '';

var m = 1.0e+100;
if (Math.floor(m) === m) {
  if (m >= 1e+100) {
    n = -Math.ceil(Math.log10(m));
    s = m / Math.pow(10, -n);
  } else {
    for (var n = 0, p = 1.0; n >= -400 && Math.floor(m / (p * 10)) === m / (p * 10); --n) p *= 10;
    s = m / p;
  }
} else {
  for (var n = 0, p = 1.0; n < 400 && Math.floor(m * p) !== m * p; ++n) p *= 10;
  s = m * p;
}
for (var k = 0, p = 1.0; k < 400 && s >= p; ++k) p *= 10;
n=k-n;
document.getElementById('out').innerHTML = JSON.stringify({ m:m, s:s, k:k, n:n, test:s*Math.pow(10, n-k) });
