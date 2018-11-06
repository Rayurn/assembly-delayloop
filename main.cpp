#include <iostream>
#include <regex>

template<typename T>
std::ostream& operator<<(std::ostream& s, const std::vector<T>& v) {
	s.put('[');
	char comma[3] = {'\0', ' ', '\0'};
	for (const auto& e : v) {
		s << comma << e;
		comma[0] = ',';
	}
	return s << ']';
}

unsigned long maxDelay(int n) {
	if (n < 1) {
		return 3;
	}
	return maxDelay(--n) * 256 + 2;
}

int nestedLoopsRequired(unsigned long cycles) {
	int loopsRequired = 0;
	while (cycles > maxDelay(loopsRequired)) {
		loopsRequired++;
	}
	return loopsRequired;
}

bool verify(const std::vector<unsigned long> &a, long additionalCycles, unsigned long expected) {
	unsigned long actual = 0;
	for (int i = 0; i < a.size(); i++) {
		actual += 3 + (a[a.size() - i - 1] - 1) * maxDelay(i);
	}
	actual -= additionalCycles;
	return actual == expected;
}

int main(int argc, char **argv){
	if (argc < 3){
		std::cout << "Usage: assembly-delayloop <TIME> <CLOCK>" << std::endl;
		exit(1);
	}
//	std::regex pattern = std::regex("([0-9]+)([^0-9]?)(Hz|s)?");
//	std::cmatch cmatch;
//	std::regex_search(argv[1], cmatch, pattern);

	unsigned long time = std::strtoul(argv[1], nullptr, 10);
	unsigned long frequency = std::strtoul(argv[2], nullptr, 10) * 1000000;

	unsigned long cycles = time * frequency;
//	unsigned long cycles = 480000000009;
	int n = nestedLoopsRequired(cycles);
	cycles -= n - 1;
	long additionalCycles = 0;

	std::vector<unsigned long> a(static_cast<unsigned long>(n));
	a[0] = (cycles + maxDelay(n - 1))/ maxDelay(n - 1);
	additionalCycles = a[0] * maxDelay(n - 1) - cycles;

	for (int i = 1; i < n; i++){
		a[i] = 256 - additionalCycles / maxDelay(n - i - 1);
		additionalCycles -= (256 - a[i]) * maxDelay(n - i - 1);
	}
	if (additionalCycles > 0) {
		a[a.size() - 1] -= 1;
		additionalCycles -= maxDelay(0);
	}
	if (!verify(a, additionalCycles, cycles + n - 1)){
		exit(2);
	}
	std::cout << a << std::endl;

	return 0;
}