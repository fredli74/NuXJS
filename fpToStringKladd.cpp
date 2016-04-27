// Example program
#include <iostream>
#include <string>
#include <cmath>

using namespace std;

std::string toString(double m) {
    double p;
    double s = m;
    double nx;
    int n;
    int k;
    if (floor(m) == m) {
/*        for (n = 0, p = 1.0; n >= -400 && floor(s / (p * 10)) == s / (p * 10); --n) p *= 10;
        s = floor(s / p);*/
        for (n = 0; n >= -400 && (nx = floor(s / 1e20)) == s / 1e20; n-=20) s = nx;
    } else {
        for (n = 0; n < 400 && floor(s + 0.5) != s; n += 20) s *= 1e20;
        s=floor(s + 0.5);
    }
    for (; n >= -400 && (nx = floor(s / 10 + 0.5)) == s / 10; --n) s = nx;

      for (k = 1, p = 10.0; k < 17 && s >= p; ++k) p *= 10;
      //p = 10.0 / p;
    std::cout << " s:" << s << std::endl;
      s /= p;
      p = 10.0;
    
    n = k - n;
    std::cout << " k:" << k << " n:" << n << " s:" << s << std::endl;
    std::string str;
  double l = 0.0;
  if (k <= n && n <= 21) {
    for (int i = 0; i < n; ++i) {
      double v = (nx = floor(s * p)) - l * 10;
      str += (i < k ? static_cast<char>(48 + v) : '0');
      p *= 10;
      l = nx;
    }
  } else if (0 < n && n <= 21) {
      int i;
    for (i = 0; i < n; ++i) {
      double v = (nx = floor(s * p)) - l * 10;
      str += static_cast<char>(48 + v);
      p *= 10;
      l = nx;
    }
    str += '.';
    for (; i < k; ++i) {
      double v = (nx = floor(s * p)) - l * 10;
      str += static_cast<char>(48 + v);
      p *= 10;
      l = nx;
    }
  } else if (-6 < n && n <= 0) {
    str += "0.";
    for (int i = 0; i < -n; ++i) {
      str += '0';
    }
    for (int i = 0; i < k; ++i) {
      double v = (nx = floor(s * p)) - l * 10;
      str += static_cast<char>(48 + v);
      p *= 10;
      l = nx;
    }
  } else {
      int i;
    for ( i = 0; i < 1; ++i) {
      double v = (nx = floor(s * p)) - l * 10;
      str += static_cast<char>(48 + v);
      p *= 10;
      l = nx;
    }
    if (k > 1) str += '.';
    for (; i < k; ++i) {
      double v = (nx = floor(s * p)) - l * 10;
      str += static_cast<char>(48 + v);
      p *= 10;
      l = nx;
    }
    str += 'e';
    if (n - 1 >= 0) str += '+';
    str += std::to_string(n - 1);
  }
  std::cout << "  toString: " << m << " -> " << str << std::endl;

    return str;
}

double toNumber(const std::string& str) {
  double div = 1, exp = 1.0, d = 0, v = 1;
  for (int i = str.size() - 1; i >= 0; --i) {
    if (str[i] == '-') {
      d = -d;
    } else if (str[i] == '+') {
    } else if (str[i] == 'e' || str[i] == 'E') {
      exp = pow(10,d);
      std::cout << " exp: " << exp << std::endl;
      d = 0.0;
      v = 1.0;
    } else if (str[i] == '.') {
      div = v;
    } else {
      d += v * (str[i] - 48);
      v *= 10;
    }
  }
  std::cout << "  toNumber: " << str << " -> " << d * (exp / div) << std::endl;
  return d * (exp / div);
}

int main()
{
    std::cout.precision(21);
    /*
  double v = 9.99e23;
  std::cout << toString(v) << std::endl;
  int exp;
  double d = frexp(v, &exp);
  d *= 2; --exp;
  std::cout << d << std::endl;
  std::cout << log10(v) << std::endl;
  std::cout << (exp * 0.301029995663981
        + (d - 1.5) * 0.289529654602168
        + 0.1760912590558) << std::endl;
    std::cout << 1e105 << std::endl;
    std::cout << pow(10,105) << std::endl;
*/    
    std::cout << toString(toNumber("1e105")) << std::endl;
    std::cout << toString(toNumber("1.234e69")) << std::endl;
    std::cout << toString(toNumber("1.23456789012345678901234567890")) << std::endl;
    std::cout << toString(toNumber("1.234567890123457")) << std::endl;
    std::cout << toString(toNumber("0.000234058234")) << std::endl;
    std::cout << toString(toNumber("0.000000000248598234")) << std::endl;
    std::cout << toString(1.23456e-33) << std::endl;
    std::cout << toString(toNumber("1.23456e-33")) << std::endl;

    toNumber(toString(1.23456789012345678901234567890));
}
