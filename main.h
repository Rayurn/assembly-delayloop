#ifndef ASSEMBLY_DELAYLOOP_MAIN_H
#define ASSEMBLY_DELAYLOOP_MAIN_H

#include <iostream>
#include <regex>
#include <getopt.h>

template<typename T>
std::ostream& operator<<(std::ostream& s, const std::vector<T>& v);

/**
 * @brief			Prints information about this tool. This function is called when the --help flag is used.
 * @author 			Christopher Field
 */
void printUsageMessage();

/**
 *
 * @brief 			Translates the input vector into an std::string of the full inline assembler code or only the loop
 * 					parameters, depending on the -s or --short flag.
 * @author 			Christopher Field
 * @param a 		std::vector containing the loop parameters, the last element is the number of required nop instructions.
 * @return 			Returns an std::string, either in full inline assembler or just the loop parameters themselves.
 */
std::string generateOutput(const std::vector<unsigned long> &a);

/**
 *
 * @brief			Calculates the maximum number of clock cycles that a given number of nested loops can take.
 * 					This is the equivalent of all the loop parameters being zero.
 * @author 			Christopher Field
 * @param n 		The number of nested loops
 * @return 			Returns the number of clock cycles
 */
unsigned long maxDelay(int n);

/**
 *
 * @brief			Calculates how many nested loops are required to wait the given amount of clock cycles.
 * @author 			Christopher Field
 * @param cycles 	The total number of clock cycles
 * @return 			The number of nested loops required
 */
int nestedLoopsRequired(unsigned long cycles);

/**
 *
 * @brief			Calculates the loop parameters required to wait the given amount of clock cycles.
 * @author 			Christopher Field
 * @param n 		The number of nested loops
 * @param cycles 	The amount of clock cycles that have to be waited.
 * @return 			The loop parameters
 *
 * 					The algorithm first assumes all the nested loops take the maximum amount of time (i.e. the loop
 * 					parameters are zero and every loop runs the full 256 times). Then the loop parameter for the
 * 					outermost loop is chosen so that the delay is just over what is required. These additional clock
 * 					cycles are then removed on the inner loops until the additional loops are zero or negative. If the
 * 					additional cycles are negative it means too many cycles have been removed and they have to be added
 * 					back through nop instructions.
 */
std::vector<unsigned long> calculateLoop(int n, unsigned long cycles);

/**
 *
 * @brief			Calculates the delay a loop with the given parameters would have in clock cycles.
 * @author 			Christopher Field
 * @param a 		std::vector containing the loop parameters, the last element is the number of required nop instructions.
 * @return 			The number of clock cycles.
 */
unsigned long calculateDelay(const std::vector<unsigned long> &a);

/**
 * @brief			Verifies that the calculated loop parameters actually produce a loop that delays the expected number
 * 					of clock cycles. Does nothing if the actual number of clock cycles matches the expected number. Is
 * 					that not the case a error message gets printed with the number of expected versus actual clock cycles.
 * @author 			Christopher Field
 * @param a 		The loop parameters of a delay loop.
 * @param cycles 	The number of cycles expected.
 */
void verify(const std::vector<unsigned long> &a, unsigned long cycles);

int main(int argc, char **argv);

#endif //ASSEMBLY_DELAYLOOP_MAIN_H
