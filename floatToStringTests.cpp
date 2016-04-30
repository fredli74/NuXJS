// Example program
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <limits>
#include "assert.h"
#include "MersenneTwister.h"

using namespace std;

std::string toString(double m) {
    std::string str;
    double p = 1.0, l = 0.0;

	int exp;
	std::cout << frexp(m, &exp) << ", " << exp << std::endl;
	std::cout << " m: " << m << std::endl;
	const double expsM[] = { 1e-256, 1e-128, 1e-64, 1e-32, 1e-16, 1e-8, 1e-4, 1e-2, 1e-1 };
	const double expsP[] = { 1e+256, 1e+128, 1e+64, 1e+32, 1e+16, 1e+8, 1e+4, 1e+2, 1e+1 };
	for (int i = 0; i < 9; ++i) {
		if (m > expsP[i]) {
			m *= expsM[i];
		}
	}
	for (int i = 0; i < 9; ++i) {
		if (m <= expsM[i]) {
			m *= expsP[i];
		}
	}
//	m = floor(m * 1e16 + 0.5);
	std::cout << " p: " << p << std::endl;
//	p = 1.0;
//	double nx;
//	for (int j = 0; j < 32 && (nx = floor(m / 10)) == m / 10; ++j) m = nx;
	std::cout << " m: " << m << std::endl;
	p = 1.0/1e16;
/*
	p = 1e-304;
	double f;
	while ((f = floor(m / p + 0.5)) * p != m) {
		p /= 10.0;
	}
	std::cout << " p: " << p << std::endl;
	std::cout << " m: " << m << std::endl;
	std::cout << " f: " << f << std::endl;
	*/
    for (int i = 0; i < 15; ++i) {
        if (true) {
			double n = floor(m * p);
			str += static_cast<char>(48 + (n - l * 10));
			p *= 10;
			l = n;
		} else if (false) {
			double v = fmod(m * p, 10);
			str = static_cast<char>(48 + floor(v)) + str;
			p *= 0.1;
		} else {
			int v = m;
			m -= v;
			m *= 1e8;
			
			char buffer[9];
			buffer[8] = 0;
			int n;
			for (n = 0; i < 24 && n < 8; ++n, ++i) {
				buffer[7 - n] = 48 + v % 10;
				v /= 10;
			}
			str += buffer + (8 - n);
		}
    }
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
  volatile double a = d;
  a = d / div;
  a *= exp;
  d = a;
  std::cout << "  toNumber: " << str << " -> " << d << std::endl;
  return d;
}

double toNumber2(const std::string& str, double exp) {
	double d = 0.0;
	for (int i = 0; i < str.size(); ++i) {
		d *= 10;
		d += (str[i] - 48);
	}
	return d * exp;
}

double toNumber3(const std::string& str, double exp) {
	double d = 0.0, v = 1.0;
	for (int i = str.size() - 1; i >= 0; --i) {
		d += v * (str[i] - 48);
		v *= 10;
	}
	return d * exp;
}

double toNumber4(const std::string& str, double exp) {
	double d = 0.0;
	const double exps[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30 };
	const int n = min(static_cast<int>(str.size()), 18);
	double v = exps[str.size() - 1];
	for (int i = 0; i < n; ++i) {
		d += exps[str.size() - 1 - i] * (str[i] - 48);
	}
	return d * exp;
}

double exactDecimalExponent(int n) {
	static const double plusExps[] = { 1e1, 1e2, 1e4, 1e8, 1e16, 1e32, 1e64, 1e128, 1e256 };
	double v = 1.0;
	if (n >= 0) {
		while (n >= 24) {
			v *= 1e24;
			n -= 24;
		}
		while (n >= 1) {
			v *= 10;
			n -= 1;
		}
		/*
		for (int i = 8; i >= 0; --i) {
			if ((n & (1 << i)) != 0) {
				v *= plusExps[i];
			}
		}*/
	}
	return v;
}

double toNumber5(const std::string& str, double exp) {
	double d = 0.0;
	const double exps[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30 };
	double c = 0.0;
	for (int i = 0; i < str.size(); ++i) {
		double x = exps[str.size() - 1 - i] * (str[i] - 48);
        double y = x - c;
        double t = d + y;
        c = (t - d) - y;
        d = t;
	}
	return d * exp;
}

double toNumber6(const std::string& str, double exp) {
	double d = 0.0;
	const double exps[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30 };
	double c = 0.0;
	double p = exps[str.size() - 1];
	for (int i = 0; i < str.size(); ++i) {
		double x = p * (str[i] - 48);
		p *= 0.1;
        double y = x - c;
        double t = d + y;
        c = (t - d) - y;
        d = t;
	}
	return d * exp;
}

double toNumber7(const std::string& str, double exp) {
	double d = 0.0;
	double c = 0.0;
	for (int i = 0; i < str.size(); ++i) {
		d *= 10.0;
		c *= 10.0;
		double x = (str[i] - 48);
        double y = x - c;
        double t = d + y;
        c = (t - d) - y;
        d = t;
	}
	return d * exp;
}

std::string toString2(double m, double exp) {
	std::cout << m << std::endl;
	const double exps[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30 };
	std::string s;
	double l = 0.0;
	double c = 0.0, d = 0.0, p = 1e17, r = m;
	for (int i = 0; i < 18; ++i) {
		std::cout << p << std::endl;
		double n = floor(r / p);
		int j = n;
		s += static_cast<char>(48 + j);
		std::cout << j << ' ' << n * p << std::endl;
		r -= n * p;

		double x = p * j;
        double y = x - c;
        double t = d + y;
        c = (t - d) - y;
        d = t;
		std::cout << "d: " << d << ": " << (d == m ? "true" : "false") << std::endl;

		if (d == m) break;
		
		p *= 0.1;
	}
	std::cout << "s: " << s << std::endl;
	return s;
}

std::string toString3(double m, double exp) {
	std::cout << m << std::endl;
	std::string s;
	double d = 0.0, r = m;// / 1e17;
	for (int i = 0; i < 18; ++i) {
		int c = static_cast<int>(r * (1.0 / 1e17));
		s += static_cast<char>(48 + c);
		std::cout << "r: " << r << std::endl;
		r = (r - c * 1e17) * 10;
		d = d * 10 + c;
		std::cout << "d: " << d << ": " << (d == m ? "true" : "false") << std::endl;
		if (d == m) break;
	}
	std::cout << "s: " << s << std::endl;
	return s;
}

double toNumber8(const std::string& s, double exp) {
	double d = 0.0, p = 1e17, e = 0.0;
	for (int i = 0; i < s.size(); ++i) {
		int c = s[i] - 48;
		double x = c * p;
        double y = x - e;
        double t = d + y;
        e = t - d;
		e -= y;
        d = t;
		p /= 10;
	}
	return d * exp;
}

std::string toString4(double m, double exp) {
	std::cout << m << std::endl;
	std::string s;
	double d = 0.0, p = 1e17 * exp, e = 0.0, e2 = 0.0;
	double r = m;
	for (int i = 0; i < 18; ++i) {
		int c = static_cast<int>(r / p);
		std::cout << c << std::endl;
		s += static_cast<char>(48 + c);
		std::cout << "r: " << r << std::endl;
		
		{
			double x = -c * p;
			double y = x - e2;
			double t = r + y;
			e2 = t - r;
			e2 -= y;
			r = t;
		}
		
		{
			double x = c * p;
			double y = x - e;
			double t = d + y;
			e = t - d;
			e -= y;
			d = t;
		}

		std::cout << "d: " << d << ": " << (d == m ? "true" : "false") << std::endl;
		p /= 10;

		if (d == m) break;
	}
	std::cout << "s: " << s << std::endl;
	return s;
}

double toNumber9(const std::string& s, double exp) {
	double d = 0.0, n = exp, e = 0.0;
	for (int i = 0; i < s.size(); ++i) {
		int c = s[i] - 48;
		double x = c * n;
        double y = x - e;
        double t = d + y;
        e = t - d;
		e -= y;
        d = t;
		n /= 10.0;
	}
	return d;
}

typedef unsigned int UInt32;
typedef int Int32;
typedef char Char;

static const Char* parseUnsignedInt(const Char* p, const Char* e, UInt32& i) {
	i = 0;
	for (; p != e && *p >= '0' && *p <= '9'; ++p) {
		i = i * 10 + (*p - '0');
	}
	return p;
}

const int MAX_EXPONENT = 308;
const int MIN_EXPONENT = -323;

struct QuickConstants {
	QuickConstants() {
		for (int i = MIN_EXPONENT; i <= MAX_EXPONENT; ++i) {
			exp10s[i - MIN_EXPONENT] = pow(10.0, static_cast<double>(i));
		}
	}
	double exp10s[MAX_EXPONENT - MIN_EXPONENT + 1];
} QUICK_CONSTANTS;

const Char* parseDouble(const Char* const b, const Char* const e, double& value) {
	Int32 exponent = -1;
	double sign = 1.0;
	const Char* significandBegin = b;

	const Char* p = b;
	if (p != e && (*p == '-' || *p == '+')) {
		sign = (*p == '-' ? -1.0 : 1.0);
		++p;
		significandBegin = p;
	}
			
	while (p != e && *p >= '0' && *p <= '9') {
		++exponent;
		++p;
	}
	if (p != e && *p == '.') {
		if (p == significandBegin) {
			++significandBegin;
		}
		++p;
		while (p != e && *p >= '0' && *p <= '9') {
			++p;
		}
	}

	if (p == significandBegin) {
		value = 0.0;
		return b;
	}

	const Char* significandEnd = p;
	const Char* numberEnd = p;

	if (e - p >= 2 && (*p == 'e' || *p == 'E')) {
		++p;
		Int32 sign = (*p == '-' ? -1 : 1);
		if (*p == '+' || *p == '-') {
			++p;
		}
		UInt32 ui;
		const Char* q = parseUnsignedInt(p, e, ui);
		if (q != p) {
			exponent += sign * static_cast<Int32>(ui);
			numberEnd = q;
		}
	}
	
	if (exponent > MAX_EXPONENT) {
		value = std::numeric_limits<double>::infinity();
	} else if (exponent < MIN_EXPONENT - 1) {
		value = 0.0;
	} else {
		double scale = (exponent < MIN_EXPONENT + 20 ? 0.1 : 1.0);	// An extra scaling step is necessary to reach the lowest denormal (approx 0.5e-324) since 1e-325 is out of range.
		double magnitude = QUICK_CONSTANTS.exp10s[exponent - MIN_EXPONENT + (exponent < MIN_EXPONENT + 20 ? 1 : 0)];
		volatile double compensation = 0.0;
		
		double d = 0.0;
		for (p = significandBegin; p != significandEnd; ++p) {
			if (*p != '.') {
				const double adjusted = (*p - '0') * magnitude * scale - compensation;	// Kahan summation for greater accuracy
				volatile double sum = d + adjusted;
				compensation = sum - d;
				compensation -= adjusted;
				d = sum;
				magnitude /= 10.0;
			}
		}
		value = d;
	}
	value *= sign;
	return numberEnd;
}

static Char* intToString(Char buffer[32], Int32 i) {
	Char* p = buffer + 32;
	*--p = "9876543210123456789"[9 + i % 10];																			// Mirrored string to handle negative x.
	Int32 x = i;
	while ((x /= 10) != 0) {																							// Mirrored string to handle negative x.
		*--p = "9876543210123456789"[9 + x % 10];
	}
	if (i < 0) {
		*--p = '-';
	}
	return p;
}

Char* convertDoubleToString(Char buffer[32], const double value) {
	double absValue = value;	
	Char* p = buffer;
	if (value < 0) {
		*p++ = '-';
		absValue = -value;
	}

	int base2Exponent;
	frexp(absValue, &base2Exponent);	// frexp is fast and precise and gives log2(x), log10(x) = log2(x) / log2(10)
	Int32 exponent = max(static_cast<Int32>(ceil(0.30102999566398119521 * (base2Exponent - 1))) - 1, MIN_EXPONENT - 1);
	if (exponent < MAX_EXPONENT && absValue >= QUICK_CONSTANTS.exp10s[exponent + 1 - MIN_EXPONENT]) {
		++exponent;
	}
	bool eNotation = (exponent < -6 || exponent >= 21);
	if (!eNotation && exponent < 0) {
		exponent = 0;
	}
	Char* periodPosition = p + (eNotation ? 0 : exponent) + 1;
	
	double scale = (exponent < MIN_EXPONENT + 20 ? 0.1 : 1.0);	// An extra scaling step is necessary to reach the lowest denormal (approx 0.5e-324) since 1e-325 is out of range.
	double magnitude = QUICK_CONSTANTS.exp10s[exponent - MIN_EXPONENT + (exponent < MIN_EXPONENT + 20 ? 1 : 0)];
	double residual = absValue;
	double reconstructed = 0.0;
	volatile double compensation = 0.0;
	while (p < buffer + 27 && magnitude != 0.0 && residual != 0.0) {	// p < buffer + 27 is an extra precaution if toNumber(toString(x)) == x is never met (e.g. because of too aggressive optimizations). 27 leaves room for longest exponent.
		if (p == periodPosition) {
			*p++ = '.';
		}
		Int32 digit = static_cast<Int32>(residual / magnitude / scale);	// In extreme circumstances (e.g. the lowest denormals), rounding causes residual / magnitude to be 1.0, hence the 9 limit.
		if (!(0 <= digit && digit <= 9)) {
			static int x;
			x = 1;
		}
		double x = digit * magnitude * scale;
		if (residual - x < 0 || digit == 10) {
			--digit;
			x = digit * magnitude * scale;
		}
		*p++ = '0' + digit;
		residual -= x;
		assert(residual >= 0);

		const double adjusted = x - compensation;	// Kahan summation for greater accuracy
		volatile double sum = reconstructed + adjusted;

		if (sum == absValue) {
			break;
		}
		if (digit < 9) {	// Catch shorter rounded solutions by trying one digit higher too (except if last digit was 9 in which case we should have caught the rounded solution in the previous step anyhow).
			const double altAdjusted = (digit + 1) * magnitude * scale - compensation;
			volatile double altSum = reconstructed + altAdjusted;
			if (altSum == absValue) {
				p[-1]++;
				break;
			}
		}

		compensation = sum - reconstructed;
		compensation -= adjusted;
		reconstructed = sum;
		magnitude /= 10;
	}
	if (p == buffer + 27) {
		static int x;
		x = 1;
		std::cout << "oops" << std::endl;
		// oops;
	}
	while (p < periodPosition) {
		*p++ = '0';
	}
	if (eNotation) {
		*p++ = 'e';
		*p++ = (exponent < 0 ? '-' : '+');
		// intToString fills the buffer from right (buffer + 32) to left and there should always be enough space
		p = std::copy(intToString(buffer, exponent < 0 ? -exponent : exponent), buffer + 32, p);
	}
	return p;
}

std::string toString5(double m, double exp) {
//	std::cout << m << std::endl;
	Char buffer[256];
	buffer[0] = '0';
	Char* b = buffer + 1;
	Char* p = b;
	double r = m, magnitude = exp, d = 0.0, compensation = 0.0;
	for (Int32 i = 0; i < 18; ++i) {
		Int32 digit = static_cast<Int32>(r / magnitude);
		*p++ = static_cast<Char>(48 + digit);
		r = (r - digit * magnitude);
// std::cout << c << std::endl;
		double prevD = d;
		{
			const double adjusted = digit * magnitude - compensation;
			const double sum = d + adjusted;
			compensation = sum - d;
			compensation -= adjusted;
			d = sum;
		}
		volatile double fff;
		fff = d;
// std::cout << " " << fff << std::endl;
		if (fff == m) {
// std::cout << "stop" << std::endl;
			break;
		}
		fff = (prevD + ((digit + 1) * magnitude - compensation));
		if (fff == m) {
			Char* q = p - 1;
			while (q > buffer && *q == '9') {
				--q;
			}
			++*q;
			if (q < b) {
				b = q;
			}
			p = q + 1;
// std::cout << "stop2" << std::endl;
			break;
		}
		magnitude /= 10;
	}
	return std::string(b, p);
}

double findExp1(double d) {
	int x2;
	frexp(d, &x2);
	int x10 = max(static_cast<int>(ceil(0.3010299956639811952137388947244930267682 * (x2 - 1))) - 2 - MIN_EXPONENT, 0); // FIX : -2? -1? -0??
	while (x10 < MAX_EXPONENT - MIN_EXPONENT && d >= QUICK_CONSTANTS.exp10s[x10 + 1]) {
		++x10;
	}
	return QUICK_CONSTANTS.exp10s[x10];
}

double findExp2(double d) {
	return pow(10, floor(log10(d)));
}

#include <iostream>
#include <ctime>
#include <cstring>

using namespace std;

int tezt() {
	struct tm tm;
	memset(&tm, 0, sizeof (tm));
	// 1995-03-30 20:11:33
	tm.tm_year = 105;
	tm.tm_mon = 1;
	tm.tm_mday = 30;
	tm.tm_isdst = -1;
	tm.tm_hour = 20;
	tm.tm_min = 11;
	tm.tm_sec = 33;
	time_t t = mktime(&tm);
	std::cout << tm.tm_isdst << std::endl;
	struct tm* tm2 = localtime(&t);
	std::cout << tm2->tm_isdst << std::endl;
	struct tm* tm3 = gmtime(&t);
	// 1995-07-30 20:11:33
	tm.tm_year = 95;
	tm.tm_mon = 6;
	tm.tm_mday = 30;
	tm.tm_isdst = -1;
	tm.tm_hour = 20;
	tm.tm_min = 11;
	tm.tm_sec = 33;
	t = mktime(&tm);
	std::cout << tm.tm_isdst << std::endl;
	tm2 = localtime(&t);
	std::cout << tm2->tm_isdst << std::endl;

	t = 89268740229;
	tm2 = gmtime(&t);
	t = 89268736629;
	tm2 = gmtime(&t);
	t = 89268747429;
	tm2 = gmtime(&t);
	std::cout << "done";

	return 0;
}

MTRand rnd(1234);

int main()
{
	srand(0);
	std::cout.precision(24);
//	const double d = 9182736454637;
	double d = 1.666666;
	std::cout << "d: " << d << std::endl;
	{
		double d;
		const Char* str = "1.5e-323";
		const Char* e = parseDouble(str, str + strlen(str), d);//toNumber8("1928374", 1e-11);
		if (e > str) {
			Char buffer[32];
			Char* end = convertDoubleToString(buffer, d);
			std::cout << std::string(buffer, end) << std::endl;
	double exp = findExp1(d);
	std::string s;
	std::cout << (s = toString5(d, exp)) << std::endl;
		}
	}
	std::cout << "d: " << d << std::endl;
	double exp = findExp1(d);
	std::cout << "exp: " << exp << std::endl;
	std::string s;
	std::cout << (s = toString5(d, exp)) << std::endl;
	std::cout << d << " = " << std::endl << toNumber9(s, exp) << std::endl;
	std::cout << numeric_limits<double>::min() << std::endl;
	std::cout << " " << 1e25 << std::endl << 1e33 << std::endl << 1e34 << std::endl << 1e37 << std::endl;
	
	{
		double d = 0;
		for (; true;) {
			char buffer[32];
			const double m = rnd.rand53();
			const int x = static_cast<int>(rnd.randInt(MAX_EXPONENT - MIN_EXPONENT)) + MIN_EXPONENT;
			const double rndD = m * pow(10.0, x);
			Char* end = convertDoubleToString(buffer, rndD);
		//	std::cout << std::string(buffer, end) << std::endl;
			double t;
			if (parseDouble(buffer, end, t) != end) {
				std::cout << "BAD!" << std::endl;
				return 1;
			}
			if (t != rndD) {
			std::cout << rndD << std::endl;
				std::cout << "BAD!" << std::endl;
				return 1;
			}
		}
	}

	{
		double d = 0;
		for (; true;) {
			char buffer[32];
			const int exp = (double)(static_cast<int>(rnd.randInt(200)) - 100);
			const double x = d * pow(10.0, exp);
			Char* end = convertDoubleToString(buffer, x);
			std::cout << std::string(buffer, end) << std::endl;
			double t;
			if (parseDouble(buffer, end, t) != end) {
				std::cout << "BAD!" << std::endl;
				return 1;
			}
			if (t != x) {
			std::cout << x << std::endl;
				std::cout << "BAD!" << std::endl;
				return 1;
			}
			if (d + 1 == d) {
				break;
			}
			++d;
		}
	}
	
	{
		static const double testExps[] = {
			1e-324, 1e-323, 1e-322, 1e-321, 1e-320, 1e-319, 1e-318, 1e-317, 1e-316, 1e-315, 1e-314, 1e-313, 1e-312, 1e-311, 1e-310, 1e-309, 1e-308, 1e-307, 1e-306, 1e-305, 1e-304, 1e-303, 1e-302, 1e-301, 1e-300, 1e-299, 1e-298, 1e-297, 1e-296, 1e-295, 1e-294, 1e-293, 1e-292, 1e-291, 1e-290, 1e-289, 1e-288, 1e-287, 1e-286, 1e-285, 1e-284, 1e-283, 1e-282, 1e-281, 1e-280, 1e-279, 1e-278, 1e-277, 1e-276, 1e-275, 1e-274, 1e-273, 1e-272, 1e-271, 1e-270, 1e-269, 1e-268, 1e-267, 1e-266, 1e-265, 1e-264, 1e-263, 1e-262, 1e-261, 1e-260, 1e-259, 1e-258, 1e-257, 1e-256, 1e-255, 1e-254, 1e-253, 1e-252, 1e-251, 1e-250, 1e-249, 1e-248, 1e-247, 1e-246, 1e-245, 1e-244, 1e-243, 1e-242, 1e-241, 1e-240, 1e-239, 1e-238, 1e-237, 1e-236, 1e-235, 1e-234, 1e-233, 1e-232, 1e-231, 1e-230, 1e-229, 1e-228, 1e-227, 1e-226, 1e-225, 1e-224, 1e-223, 1e-222, 1e-221, 1e-220, 1e-219, 1e-218, 1e-217, 1e-216, 1e-215, 1e-214, 1e-213, 1e-212, 1e-211, 1e-210, 1e-209, 1e-208, 1e-207, 1e-206, 1e-205, 1e-204, 1e-203, 1e-202, 1e-201, 1e-200, 1e-199, 1e-198, 1e-197, 1e-196, 1e-195, 1e-194, 1e-193, 1e-192, 1e-191, 1e-190, 1e-189, 1e-188, 1e-187, 1e-186, 1e-185, 1e-184, 1e-183, 1e-182, 1e-181, 1e-180, 1e-179, 1e-178, 1e-177, 1e-176, 1e-175, 1e-174, 1e-173, 1e-172, 1e-171, 1e-170, 1e-169, 1e-168, 1e-167, 1e-166, 1e-165, 1e-164, 1e-163, 1e-162, 1e-161, 1e-160, 1e-159, 1e-158, 1e-157, 1e-156, 1e-155, 1e-154, 1e-153, 1e-152, 1e-151, 1e-150, 1e-149, 1e-148, 1e-147, 1e-146, 1e-145, 1e-144, 1e-143, 1e-142, 1e-141, 1e-140, 1e-139, 1e-138, 1e-137, 1e-136, 1e-135, 1e-134, 1e-133, 1e-132, 1e-131, 1e-130, 1e-129, 1e-128, 1e-127, 1e-126, 1e-125, 1e-124, 1e-123, 1e-122, 1e-121, 1e-120, 1e-119, 1e-118, 1e-117, 1e-116, 1e-115, 1e-114, 1e-113, 1e-112, 1e-111, 1e-110, 1e-109, 1e-108, 1e-107, 1e-106, 1e-105, 1e-104, 1e-103, 1e-102, 1e-101, 1e-100, 1e-99, 1e-98, 1e-97, 1e-96, 1e-95, 1e-94, 1e-93, 1e-92, 1e-91, 1e-90, 1e-89, 1e-88, 1e-87, 1e-86, 1e-85, 1e-84, 1e-83, 1e-82, 1e-81, 1e-80, 1e-79, 1e-78, 1e-77, 1e-76, 1e-75, 1e-74, 1e-73, 1e-72, 1e-71, 1e-70, 1e-69, 1e-68, 1e-67, 1e-66, 1e-65, 1e-64, 1e-63, 1e-62, 1e-61, 1e-60, 1e-59, 1e-58, 1e-57, 1e-56, 1e-55, 1e-54, 1e-53, 1e-52, 1e-51, 1e-50, 1e-49, 1e-48, 1e-47, 1e-46, 1e-45, 1e-44, 1e-43, 1e-42, 1e-41, 1e-40, 1e-39, 1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33, 1e-32, 1e-31, 1e-30, 1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23, 1e-22, 1e-21, 1e-20, 1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1, 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36, 1e37, 1e38, 1e39, 1e40, 1e41, 1e42, 1e43, 1e44, 1e45, 1e46, 1e47, 1e48, 1e49, 1e50, 1e51, 1e52, 1e53, 1e54, 1e55, 1e56, 1e57, 1e58, 1e59, 1e60, 1e61, 1e62, 1e63, 1e64, 1e65, 1e66, 1e67, 1e68, 1e69, 1e70, 1e71, 1e72, 1e73, 1e74, 1e75, 1e76, 1e77, 1e78, 1e79, 1e80, 1e81, 1e82, 1e83, 1e84, 1e85, 1e86, 1e87, 1e88, 1e89, 1e90, 1e91, 1e92, 1e93, 1e94, 1e95, 1e96, 1e97, 1e98, 1e99, 1e100, 1e101, 1e102, 1e103, 1e104, 1e105, 1e106, 1e107, 1e108, 1e109, 1e110, 1e111, 1e112, 1e113, 1e114, 1e115, 1e116, 1e117, 1e118, 1e119, 1e120, 1e121, 1e122, 1e123, 1e124, 1e125, 1e126, 1e127, 1e128, 1e129, 1e130, 1e131, 1e132, 1e133, 1e134, 1e135, 1e136, 1e137, 1e138, 1e139, 1e140, 1e141, 1e142, 1e143, 1e144, 1e145, 1e146, 1e147, 1e148, 1e149, 1e150, 1e151, 1e152, 1e153, 1e154, 1e155, 1e156, 1e157, 1e158, 1e159, 1e160, 1e161, 1e162, 1e163, 1e164, 1e165, 1e166, 1e167, 1e168, 1e169, 1e170, 1e171, 1e172, 1e173, 1e174, 1e175, 1e176, 1e177, 1e178, 1e179, 1e180, 1e181, 1e182, 1e183, 1e184, 1e185, 1e186, 1e187, 1e188, 1e189, 1e190, 1e191, 1e192, 1e193, 1e194, 1e195, 1e196, 1e197, 1e198, 1e199, 1e200, 1e201, 1e202, 1e203, 1e204, 1e205, 1e206, 1e207, 1e208, 1e209, 1e210, 1e211, 1e212, 1e213, 1e214, 1e215, 1e216, 1e217, 1e218, 1e219, 1e220, 1e221, 1e222, 1e223, 1e224, 1e225, 1e226, 1e227, 1e228, 1e229, 1e230, 1e231, 1e232, 1e233, 1e234, 1e235, 1e236, 1e237, 1e238, 1e239, 1e240, 1e241, 1e242, 1e243, 1e244, 1e245, 1e246, 1e247, 1e248, 1e249, 1e250, 1e251, 1e252, 1e253, 1e254, 1e255, 1e256, 1e257, 1e258, 1e259, 1e260, 1e261, 1e262, 1e263, 1e264, 1e265, 1e266, 1e267, 1e268, 1e269, 1e270, 1e271, 1e272, 1e273, 1e274, 1e275, 1e276, 1e277, 1e278, 1e279, 1e280, 1e281, 1e282, 1e283, 1e284, 1e285, 1e286, 1e287, 1e288, 1e289, 1e290, 1e291, 1e292, 1e293, 1e294, 1e295, 1e296, 1e297, 1e298, 1e299, 1e300, 1e301, 1e302, 1e303, 1e304, 1e305, 1e306, 1e307, 1e308
		};
		for (int i = -324; i <= 308; ++i) {
			if (pow(10, static_cast<double>(i)) != testExps[i + 324]) {
				std::cout << pow(10, static_cast<double>(i)) << " != " << testExps[i + 324] << std::endl;
			}
		}
	}
	if (false) {
		for (int i = 0; i <= 308; ++i) {
			const double v = exactDecimalExponent(i);
			if (pow(10, static_cast<double>(i)) != v) {
				std::cout << v << " != " << pow(10, static_cast<double>(i)) << std::endl;
			}
		}
	}
	
	std::cout << "toNumber4: " << toNumber5("9014846160420776", 1.0) << std::endl;
	std::cout << "toNumber2: " << toNumber2("12345678901234578787476", 10e-245) << std::endl;
	std::cout << "toNumber3: " << toNumber3("12345678901234578787476", 10e-245) << std::endl;
	std::cout << "toNumber4: " << toNumber3("12345678901234578787476", 10e-245) << std::endl;
	std::cout << "toNumber2: " << toNumber2("89898999", 10e-33) << std::endl;
	std::cout << "toNumber3: " << toNumber3("89898999", 10e-33) << std::endl;
	std::cout << "toNumber4: " << toNumber3("89898999", 10e-33) << std::endl;
	std::cout << "toNumber2: " << toNumber2("19999123456798765435678", 10e33) << std::endl;
	std::cout << "toNumber3: " << toNumber3("19999123456798765435678", 10e33) << std::endl;
	std::cout << "toNumber4: " << toNumber4("19999123456798765435678", 10e33) << std::endl;
	std::cout << "toNumber2: " << toNumber2("8983771932771280445826", 10e140) << std::endl;
	std::cout << "toNumber3: " << toNumber3("8983771932771280445826", 10e140) << std::endl;
	std::cout << "toNumber4: " << toNumber4("8983771932771280445826", 10e140) << std::endl;
	std::cout << "toNumber5: " << toNumber5("8983771932771280445826", 10e140) << std::endl;
	std::cout << "toNumber6: " << toNumber6("8983771932771280445826", 10e140) << std::endl;
	std::cout << "toNumber7: " << toNumber7("8983771932771280445826", 10e140) << std::endl;

	std::cout << "--------" << std::endl;
	for (int i = 0; i < 10000; ++i) {
		std::string s;
		int l = (rnd.randInt(19)) + 3;
		for (int j = 0; j < l; ++j) s += static_cast<char>(48 + (rand() % 10));
		std::cout << s << std::endl;
		const double exp = 1.0;
		double n2 = toNumber2(s, exp);
		double n3 = toNumber3(s, exp);
		double n4 = toNumber4(s, exp);
		double n5 = toNumber5(s, exp);
		double n6 = toNumber6(s, exp);
		double n7 = toNumber7(s, exp);
		std::cout << "n2: " << n2 << std::endl;
		std::cout << "n3: " << n3 << std::endl;
		std::cout << "n4: " << n4 << std::endl;
		std::cout << "n5: " << n5 << std::endl;
		std::cout << "n6: " << n6 << std::endl;
		std::cout << "n7: " << n7 << std::endl;
	}
	std::cout << "--------" << std::endl;
	

	const double m = 8423768723648716237612813e141;
	s = "8423768723648716237612813e141";
	const double n = toNumber(s);
	std::cout << n << " ==\n" << m << " : " << (n == m ? "mmm" : "nää") << std::endl;
	std::cout << toString(1.2345678901234578e-222) << std::endl;
}
