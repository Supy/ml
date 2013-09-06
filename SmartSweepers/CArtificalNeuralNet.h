#ifndef CARTIFIALNEURALNET_H
#define CARTIFIALNEURALNET_H

#include <vector>
#include <fstream>
#include <math.h>
#include "utils.h"

using namespace std;


// Neuron Struct
struct Neuron
{
	// Number of inputs that the neuron receives
	int	numberOfInputs;

	// Neuron weight vector for each input
	vector<double> weight;

	// Neuron constructor
	Neuron(int numberOfInputs);
};


// Neuron Layer Struct
struct NeuronLayer
{
	// Number of neurons in NeuronLayer
	int	numberOfNeurons;

	//Neuron layer of its containing neurons (vector)
	vector<Neuron> vecOfNeurons;

	// Neuron Layer constructor
	NeuronLayer(int NumNeurons, int NumInputsPerNeuron);
};


// Artifical Neural Network Class
class CArtificalNeuralNet
{
	
private:	
	int numberOfInputs;
	int numberOfOutputs;
	int numberOfHiddenLayers;
	int neuronsPerHiddenLayer;
	int bias;

	// Vector of Neuron Layers (includes output layer)
	vector<NeuronLayer> vecOfLayers;

public:

	CArtificalNeuralNet();

	// Build Artifical Neural Network
	void BuildANN();

	// Get weights from the ANN
	vector<double> const GetANNWeights();

	// Get number of weights needed in the ANN
	int const GetNumberOfWeightsNeeded();

	// Replace the weights in the ANN
	void ReplaceWeightsInANN(vector<double> &weights);

	// Calculates ANN outputs from provided inputs
	vector<double> CalculateOutput(vector<double> &inputs);

	// Sigmoid Function
	double SigmoidFunction(double input);

};
				
#endif