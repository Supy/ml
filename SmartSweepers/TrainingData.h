#ifndef TRAININGDATA_H
#define TRAININGDATA_H

#include <array>

struct TrainingData
{
public:

	std::array<bool, 4> inputs;
	double steeringOutput, speedOutput;

	TrainingData(std::array<bool, 4> in, double steeringOut, double speedOut) : inputs(in), steeringOutput(steeringOut), speedOutput(speedOut){
		// Do nothing.
	}
};
#endif