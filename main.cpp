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

std::ostringstream generateOutput(const std::vector<unsigned long> &a) {
	std::ostringstream output;

	output << "asm volatile (\n";
	for (int i = 0; i < a.size() - 1; i++){
		output << "\t\"\tldi  r" << i + 16 << ", " << a[i] << "\t\\n\"\n";
	}
	for (int i = 0; i < a.size() - 1; i++){
		output << "\t\"" << (i == 0 ? "L:" : "") << "\tdec  r" << a.size() + 15 - i << "\t\t\\n\"\n";
		output << "\t\"\tbrne L\t\t\t\\n\"\n";
	}
	for (int i = 0; i < a[a.size() - 1]; i++){
		output << "\t\"\tnop\t\t\t\t\\n\"\n";
	}

	output << ");";

	return output;
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

std::vector<unsigned long> calculateLoop(int n, unsigned long cycles) {
	std::vector<unsigned long> a(static_cast<unsigned long>(n + 1));
	long additionalCycles;

	a[0] = (cycles + maxDelay(n - 1)) / maxDelay(n - 1);
	additionalCycles = a[0] * maxDelay(n - 1) - cycles;

	for (int i = 1; i < n; i++){
		a[i] = 256 - additionalCycles / maxDelay(n - i - 1);
		additionalCycles -= (256 - a[i]) * maxDelay(n - i - 1);
	}
	if (additionalCycles > 0) {
		a[a.size() - 2] -= 1;
		additionalCycles -= maxDelay(0);
	}
	a[a.size() - 1] = static_cast<unsigned long>(-additionalCycles);

	return a;
}

unsigned long calculateDelay(const std::vector<unsigned long> &a) {
	unsigned long actual = 0;
	for (int i = 0; i < a.size() - 1; i++) {
		actual += 3 + (a[a.size() - i - 2] - 1) * maxDelay(i);
	}
	actual += a[a.size() - 1];
	return actual;
}

int main(int argc, char **argv){
	if (argc < 3){
		std::cerr << "Usage: assembly-delayloop <TIME> <CLOCK>" << std::endl;
		exit(1);
	}
	std::map<std::string, int> prefixes{
			{ "P", 15 },
			{ "T", 12 },
			{ "G", 9 },
			{ "M", 6 },
			{ "k", 3 },
			{ "m", -3 },
			{ "u", -6 },
			{ "n", -9 },
			{ "p", -12 },
			{ "f", -15 }};

	std::map<std::string, int> correctTime{
			{ "s", 1 },
			{ "min", 60 },
			{ "h", 60 * 60 },
			{ "d", 60 * 60 * 24 }};

	std::regex pattern = std::regex("([0-9]+\\.?[0-9]*)([^0-9]?)(Hz|s|min|h|d)?");
	std::cmatch cmatchTime;
	std::cmatch cmatchFreq;
	std::regex_search(argv[1], cmatchTime, pattern);
	std::regex_search(argv[2], cmatchFreq, pattern);

	double time = std::stod(cmatchTime[1].str());
	double frequency = std::stod(cmatchFreq[1].str());

	unsigned long cycles = static_cast<unsigned long>(time * correctTime[cmatchTime[3].str()] * frequency * std::pow(10, prefixes[cmatchTime[2].str()] + prefixes[cmatchFreq[2].str()]));
//	unsigned long cycles = 480000000009;
	int n = nestedLoopsRequired(cycles);
	cycles -= n - 1;

	std::vector<unsigned long> a = calculateLoop(n, cycles);

	if (calculateDelay(a) != cycles + n - 1){
		std::cerr << "Error" << std::endl;
		std::cerr << "Expected: " << cycles + n - 1 << std::endl;
		std::cerr << "Actual: " << calculateDelay(a) << std::endl;
	}
	std::cout << generateOutput(a).str() << std::endl;
//	std::cout << a << std::endl;

	return 0;
}