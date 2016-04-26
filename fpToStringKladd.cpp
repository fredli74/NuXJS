// Example program
#include <iostream>
#include <string>
#include <cmath>

using namespace std;

std::string toString(double m) {
    double p;
    double s;
    int n;
    int k;
    if (floor(m) == m) {
        for (n = 0, p = 1.0; n >= -400 && floor(m / (p * 10)) == m / (p * 10); --n) p *= 10;
    } else {
        for (n = 0, p = 1.0; n < 400 && floor(m * p) != m * p; ++n) p *= 10;
        s = m * p;
    }
    if (true) {
        for (k = 1; k < 400 && (m / p) >= 10.0; ++k) p *= 10;
        s = m;
    } else {
        for (k = 1; k < 400 && m * p >= 10.0; ++k) p *= 0.1;
        s = m;
    }
    n = k - n;
    std::cout << "n:" << n << std::endl;
    std::string str;
    if (k <= n && n <= 21) {
        for (int i = 0; i < n; ++i) {
            str += (i < k ? static_cast<char>(48 + floor(fmod(m / p, 10))) : '0');
            p /= 10;
        }
    } else if (0 < n && n <= 21) {
      /*  for (var i = 0, v = s, p = 10; i < n; ++i) {
            v *= 10;
            str += String.fromCharCode(48 + floor(v));
            v -= floor(v);
        }
        str += '.';
        for (; i < k; ++i) {
            v *= 10;
            str += String.fromCharCode(48 + floor(v));
            v -= floor(v);
        }*/
    }
    return str;
}

double toNumber(const std::string& str) {
    double d = 0.0;
    double v = 1.0;
    for (int i = str.size() - 1; i >= 0; --i) {
        d += v * (str[i] - 48);
        v *= 10;
    }
  return d;
}

int main()
{
  std::string name;
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
        
}
