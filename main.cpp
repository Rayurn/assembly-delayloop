#include "main.h"

int cFlag = 0;
int sFlag = 0;
int tFlag = 0;
int fFlag = 0;
int rFlag = 16;

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

std::string generateOutput(const std::vector<unsigned long> &a) {
	std::ostringstream output;

	if (sFlag != 0) {
		output << a;
		return output.str();
	}

	output << "asm volatile (\n";
	for (int i = 0; i < a.size() - 1; i++){
		output << "\t\"\tldi  r" << i + rFlag << ", " << a[i] << "\t\\n\"\n";
	}
	for (int i = 0; i < a.size() - 1; i++){
		output << "\t\"" << (i == 0 ? "L:" : "") << "\tdec  r" << a.size() + rFlag - 2 - i << "\t\t\\n\"\n";
		output << "\t\"\tbrne L\t\t\t\\n\"\n";
	}
	for (int i = 0; i < a[a.size() - 1]; i++){
		output << "\t\"\tnop\t\t\t\t\\n\"\n";
	}

	output << ");";

	return output.str();
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

void verify(const std::vector<unsigned long> &a, unsigned long cycles) {
	if (calculateDelay(a) != cycles){
		std::cerr << "Error" << std::endl;
		std::cerr << "Expected: " << cycles << std::endl;
		std::cerr << "Actual: " << calculateDelay(a) << std::endl;
	}
}

int main(int argc, char **argv){

	unsigned long cycles = 0;
	double time          = 0;
	double frequency     = 0;

	std::regex pattern = std::regex("([0-9]+\\.?[0-9]*)([^0-9]?)(Hz|s|min|h|d)?");
	std::cmatch cmatchTime;
	std::cmatch cmatchFreq;

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

	int c;
	while (true) {
		int option_index = 0;
		static struct option long_options[] = {
				{ "cycles",     required_argument,  nullptr, 'c' },
				{ "short",      no_argument,        nullptr, 's' },
				{ "help",       no_argument,        nullptr, 'h' },
				{ "time",       required_argument,  nullptr, 't' },
				{ "frequency",  required_argument,  nullptr, 'f' },
				{ "register",   required_argument,  nullptr, 'r' }
		};
		c = getopt_long(argc, argv, "c:sht:f:r:", long_options, &option_index);
		if (c == -1) {
			break;
		}
		switch (c) {
			case 'c':
				cycles = std::stoul(optarg);
				cFlag = 1;
				break;
			case 's':
				sFlag = 1;
				break;
			case 'h':
				std::cerr << "Usage message" << std::endl;
				break;
			case 't':
				std::regex_search(optarg, cmatchTime, pattern);
				time = std::stod(cmatchTime[1].str()) * correctTime[cmatchTime[3].str()];
				tFlag = 1;
				break;
			case 'f':
				std::regex_search(optarg, cmatchFreq, pattern);
				frequency = std::stod(cmatchFreq[1].str());
				fFlag = 1;
				break;
			case 'r':
				rFlag = std::stoi(optarg);
				break;
			default:
				std::cerr << "Invalid command, see 'assembly-delayloop --help' for usage" << std::endl;
		}
	}

	if(cFlag == 0) {
		if (optind < argc) {
			if (tFlag == 0){
				std::regex_search(argv[optind++], cmatchTime, pattern);
				time = std::stod(cmatchTime[1].str()) * correctTime[cmatchTime[3].str()];
			}
			if (fFlag == 0) {
				std::regex_search(argv[optind++], cmatchFreq, pattern);
				frequency = std::stod(cmatchFreq[1].str());
			}
		}
		cycles = static_cast<unsigned long>(time * frequency * std::pow(10, prefixes[cmatchTime[2].str()] + prefixes[cmatchFreq[2].str()]));
	}

	int n = nestedLoopsRequired(cycles);
	cycles -= n - 1;

	std::vector<unsigned long> a = calculateLoop(n, cycles);

	verify(a, cycles + n - 1);

	std::cout << generateOutput(a) << std::endl;

	return 0;
}