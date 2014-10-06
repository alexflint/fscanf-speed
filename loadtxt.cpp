#include <cstdlib>
#include <cstdio>
#include <vector>
#include <cassert>
#include <string>

static const double kMinExponent = -300;
static const double kMaxExponent = 300;

inline double Pow10Positive(int e) {
	static std::vector<double> cache = {1.};
	while (cache.size() < e + 1) {
		cache.push_back(cache.back() * 10.);
	}
	return cache[e];
}

inline double Pow10Negative(int e) {
	static std::vector<double> cache = {1.};
	while (cache.size() < e + 1) {
		cache.push_back(cache.back() * .1);
	}
	return cache[e];
}

inline int ConsumeInteger(const char*& p, int& ndigits) {
	ndigits = 0;
	int sum = 0;
	while (*p >= '0' && *p <= '9') {
		sum = sum * 10 + (*p - '0');
		++p;
		++ndigits;
	}
	return sum;
}

inline int ConsumeSignedInteger(const char*& p, int& ndigits) {
	// Consume sign
	int sign = 1;
	if (*p == '-') {
		sign = -1;
		++p;
	} else if (*p == '+') {
		sign = 1;
		++p;
	}
	return ConsumeInteger(p, ndigits) * sign;
}

inline double ConsumeDouble(const char*& p) {
	// Consume whole part
	int ndigits_whole;
	int whole_part = ConsumeSignedInteger(p, ndigits_whole);

	// Consume fractional part
	double val = whole_part;
	if (*p == '.') {
		++p;
		int ndigits_fractional = 0;
		int fractional_part = ConsumeInteger(p, ndigits_fractional);
		if (ndigits_whole == 0 && ndigits_fractional == 0) {
			printf("Found period with no digits either before or after\n");
			exit(-1);
		} else if (ndigits_fractional > 0) {
			val += fractional_part * Pow10Negative(ndigits_fractional);
		}
	} else if (ndigits_whole == 0) {
		printf("Found neither whole part nor period\n");
		exit(-1);
	}

	// Consume exponent
	if (*p == 'e' || *p == 'E') {
		++p;
		int ndigits_exponent;
		int exponent = ConsumeSignedInteger(p, ndigits_exponent);
		if (ndigits_exponent == 0) {
			printf("Found exponent char but no exponent\n");
			exit(-1);
		} else if (exponent > kMaxExponent || exponent < kMinExponent) {
			printf("Exponent out of range\n");
			exit(-1);
		} else if (exponent > 0) {
			val *= Pow10Positive(exponent);
		} else if (exponent < 0) {
			val *= Pow10Negative(-exponent);
		}
	}
	return val;
}

void ProcessLine(std::vector<double>& out, const char* line, size_t len) {
	const char* p = line;
	while (p < line + len) {
		out.push_back(ConsumeDouble(p));
		if (*p != ' ' && *p != '\n') {
			printf("Expected whitespace but found %c\n", *p);
			exit(-1);
		}
		while (*p == ' ' || *p == '\n') {
			++p;
		}
	}
}

int LoadText_Direct(const char* path) {
	std::vector<double> items;
	int nrows = 0;
	FILE* fd = fopen(path, "r");
	while (!feof(fd)) {
		size_t len = 0;
		const char* line = fgetln(fd, &len);
		if (line == nullptr) {
			break;
		}
		items.clear();
		ProcessLine(items, line, len);
		++nrows;
	}
	fclose(fd);
	return nrows;
}

int LoadText_Printf(const char* path) {
	int nrows = 0;
	std::vector<double> line(5);
	FILE* fd = fopen(path, "r");
	assert(fd);
	while (!feof(fd)) {
		fscanf(fd, "%lf %lf %lf %lf %lf\n",
					 &line[0], &line[1], &line[2], &line[3], &line[4]);
		++nrows;
	}
	fclose(fd);
	return nrows;
}

void PrintUsageAndExit() {
	printf("Usage: loadtxt [--direct | --printf]  PATH\n");
	exit(-1);
}

int main(int argc, char **argv) {
	if (argc != 3) {
		PrintUsageAndExit();
	}

	std::string mode(argv[1]);

	int nrows;
	if (mode == "--direct") {
		nrows = LoadText_Direct(argv[2]);
	} else if (mode == "--printf") {
		nrows = LoadText_Printf(argv[2]);
	}

	printf("Read %d rows\n", nrows);

	return 0;
}
