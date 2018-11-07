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

void printUsageMessage() {
	std::cout << "More info: https://github.com/Rayurn/assembly-delayloop" << std::endl << std::endl;

	std::cout << "-c/--cycles     To use a number of clock cycles as input. Don't use together with -t or -f." << std::endl;
	std::cout << "-t/--time       To specify which argument is the time, by default it is the left one." << std::endl;
	std::cout << "-f/--frequency  To specify which argument is the frequency, by default it is the right one." << std::endl;
	std::cout << "-s/--short      Gives only the loop parameters as output. the last element is the number of additional nop instructions." << std::endl;
	std::cout << "-r/--register   To choose which registers to use, default is 16." << std::endl;
	std::cout << "-h/--help       Display usage, and information on the commandline flags." << std::endl << std::endl;

	std::cout << "When using time and frequency, units are required while SI-prefixes are optional. Units for time are seconds(s), minutes(min)," << std::endl;
	std::cout << "hours(h) and days(d); frequency is in Hz. SI-prefixes are available from femto to Peta (for micro, instead of 'μ' use 'u')." << std::endl << std::endl;

	std::cout << "Examples:" << std::endl << std::endl;
	std::cout << "Default case" << std::endl;
	std::cout << "delayloop 500ms 16MHz" << std::endl << std::endl;

	std::cout << "Shortened output" << std::endl;
	std::cout << "delayloop 500ms 16MHz -s" << std::endl << std::endl;

	std::cout << "Input clock cycles instead of time and frequency" << std::endl;
	std::cout << "delayloop -c 16000000" << std::endl << std::endl;

	std::cout << "Specify the argument for time and set starting register to 20" << std::endl;
	std::cout << "delayloop 16MHz -t 500ms -r 20" << std::endl << std::endl;
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
		output << "\t\"" << (i == 0 ? "L:" : "") << "\tdec  r" << a.size() + rFlag - 2 - i << "\t\\n\"\n";
		output << "\t\"\tbrne L\t\t\\n\"\n";
	}
	for (int i = 0; i < a[a.size() - 1]; i++){
		output << "\t\"\tnop\t\t\\n\"\n";
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
	if (argc == 1) {
		std::cerr << "Unrecognised command, see 'assembly-delayloop --help' for usage" << std::endl;
		exit(1);
	}

	unsigned long cycles = 0;
	double time          = 0;
	double frequency     = 0;

	std::regex pattern = std::regex("([0-9]+\\.?[0-9]*)(f|p|n|u|m(?!in)|k|M|G|T|P?)(Hz|s|min|h|d)?");
	std::cmatch cmatchTime;
	std::cmatch cmatchFreq;

	std::map<std::string, int> prefixes{
			{ "P", 15 },
			{ "T", 12 },
			{ "G", 9 },
			{ "M", 6 },
			{ "k", 3 },
			{ "0", 0 },
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
				printUsageMessage();
				exit(0);
			case 't': {
				std::regex_search(optarg, cmatchTime, pattern);
				time = std::stod(cmatchTime[1].str()) * correctTime[cmatchTime[3].str()];
				tFlag = 1;
				break;}
			case 'f':
				std::regex_search(optarg, cmatchFreq, pattern);
				frequency = std::stod(cmatchFreq[1].str());
				fFlag = 1;
				break;
			case 'r':
				rFlag = std::stoi(optarg);
				break;
			default:
				std::cerr << "Unrecognised command, see 'assembly-delayloop --help' for usage" << std::endl;
				exit(1);
		}
	}

	if (cFlag == 1 && (tFlag == 1 || fFlag == 1)){
		std::cerr << "Unrecognised command, see 'assembly-delayloop --help' for usage" << std::endl;
		exit(1);
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
		int timePrefix = 0;
		int frequencyPrefix = 0;
		if (cmatchTime[3].str().empty()) {
			timePrefix = prefixes["0"];
		} else {
			timePrefix = prefixes[cmatchTime[2].str()];
		}
		if (cmatchFreq[3].str().empty()) {
			frequencyPrefix = prefixes["0"];
		} else {
			frequencyPrefix = prefixes[cmatchFreq[2].str()];
		}
		cycles = static_cast<unsigned long>(time * frequency * pow(10, timePrefix + frequencyPrefix));
	}

	int n = nestedLoopsRequired(cycles);
	cycles -= n - 1;

	std::vector<unsigned long> a = calculateLoop(n, cycles);

	verify(a, cycles + n - 1);

	std::cout << generateOutput(a) << std::endl;

	exit(0);
}