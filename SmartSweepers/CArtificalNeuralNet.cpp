#include "CArtificalNeuralNet.h"

// Neuron constructor
Neuron::Neuron(int NumInputs) 										
{
	// There is an additional attribute added for the bias
	numberOfInputs = NumInputs+1;	

	for (int i=0; i<numberOfInputs; i++)
	{
		float randomFloat = (float)((rand())/(RAND_MAX+1.0) - (rand())/(RAND_MAX+1.0));

		// Weights are pushed into the weight vector with random values between (-1, 1)
		weight.push_back(randomFloat);
	}
}


// NeuronLayer constructor
NeuronLayer::NeuronLayer(int numberOfNeuronsTemp, int numberOfInputsPerNeuron)
{
	numberOfNeurons = numberOfNeuronsTemp;
	for (int i=0; i<numberOfNeurons; i++){
		vecOfNeurons.push_back(Neuron(numberOfInputsPerNeuron));
	}
}


// ArtificalNerualNet constructor (creates an ANN with specified values)
CArtificalNeuralNet::CArtificalNeuralNet() 
{
	numberOfInputs = 4;
	numberOfOutputs = 2;
	numberOfHiddenLayers =	1;
	neuronsPerHiddenLayer =	6;
	bias = -1;

	BuildANN();

}

// Build ANN, initially set weights to random values between (-1, 1)
// Create Hidden and output layer of ANN
void CArtificalNeuralNet::BuildANN()
{
		// Create the layers of the ANN
		// Create the hidden layer
		vecOfLayers.push_back(NeuronLayer(neuronsPerHiddenLayer, numberOfInputs));
		//create output layer
		vecOfLayers.push_back(NeuronLayer(numberOfOutputs, neuronsPerHiddenLayer));
}


// Get the  ANN's Weights
vector<double> const CArtificalNeuralNet::GetANNWeights()
{
	// aNNWeights vector
	vector<double> aNNWeights;

	// One output Layer
	int outputLayer = 1;
	// Each layer (Hidden and Output)
	for (int indexLayer=0; indexLayer<numberOfHiddenLayers + outputLayer; indexLayer++)
	{
		// Each neuron
		for (int indexNeuron=0; indexNeuron<vecOfLayers[indexNeuron].numberOfNeurons; indexNeuron++)
		{
			// Each weight
			for (int indexWeight=0; indexWeight<vecOfLayers[indexNeuron].vecOfNeurons[indexNeuron].numberOfInputs; indexWeight++)
			{
				// Capture the Neurons weight in the aNNweights vector
				aNNWeights.push_back(vecOfLayers[indexNeuron].vecOfNeurons[indexNeuron].weight[indexWeight]);
			}
		}
	}

	return aNNWeights;
}

// Replace Weights in ANN with the new Vector of Weights (supplied to function)
void CArtificalNeuralNet::ReplaceWeightsInANN(vector<double> &newVectorOfWeights)
{
	int indexOfNewWeight = 0;
	
	// One output Layer
	int outputLayer = 1;
	// Each layer (Hidden and Output)
	for (int indexLayer=0; indexLayer<numberOfHiddenLayers + outputLayer; indexLayer++)
	{
		// Each neuron
		for (int indexNeuron=0; indexNeuron<vecOfLayers[indexLayer].numberOfNeurons; indexNeuron++)
		{
			// Each weight
			for (int indexWeight=0; indexWeight<vecOfLayers[indexLayer].vecOfNeurons[indexNeuron].numberOfInputs; indexWeight++)
			{
				// Replaces the Weights in the Ann with the vector of weights supplied
				vecOfLayers[indexLayer].vecOfNeurons[indexNeuron].weight[indexWeight] = newVectorOfWeights[indexOfNewWeight++];
			}
		}
	}
}


// Get the number of weights needed for the ANN
int const CArtificalNeuralNet::GetNumberOfWeightsNeeded()
{
	int weightsNeeded = 0;

	// One output Layer
	int outputLayer = 1;
	// Each layer (Hidden and Output)
	for (int indexLayer=0; indexLayer<numberOfHiddenLayers + outputLayer; indexLayer++)
	{
		// Each neuron
		for (int indexNeuron=0; indexNeuron<vecOfLayers[indexLayer].numberOfNeurons; indexNeuron++)
		{
			// Each weight
			for (int indexWeight=0; indexWeight<vecOfLayers[indexLayer].vecOfNeurons[indexNeuron].numberOfInputs; indexWeight++){
				// Increment the weights needed
				weightsNeeded++;
			}			
		}
	}

	return weightsNeeded;
}

// The ANNS output is calculated from the supplied inputs
vector<double> CArtificalNeuralNet::CalculateOutput(vector<double> &vectorOfInputs)
{
	// Vector containing the outputs from the Hidden Layer
	vector<double> vectorOfOutputs;

	int indexOfInputsWeight = 0;
		
	// One output Layer
	int outputLayer = 1;
	// Each layer (Hidden and Output)
	for (int indexLayer=0; indexLayer<numberOfHiddenLayers + outputLayer; indexLayer++)
	{		
		//Set the inputs equal to the outputs if the index layer is greater than 0
		if ( indexLayer > 0 ){
			vectorOfInputs = vectorOfOutputs;
		}
		// Clear the vector of outputs
		vectorOfOutputs.clear();
		
		indexOfInputsWeight = 0;

		// Each neuron
		// Sum weights of neuron multiplied by weights from input
		// Filter the new weights using sigmoid activation function and add them to the output vector
		for (int indexNeuron=0; indexNeuron<vecOfLayers[indexLayer].numberOfNeurons; indexNeuron++)
		{
			double aNNInput = 0;

			int	NumInputs = vecOfLayers[indexLayer].vecOfNeurons[indexNeuron].numberOfInputs;
			
			// Each weight
			for (int indexWeight=0; indexWeight<NumInputs - 1; indexWeight++)
			{
				// Form a new ANN input by adding the (weights x inputWeights) to it
				double inputWeight = vectorOfInputs[indexOfInputsWeight++];
				double weights = vecOfLayers[indexLayer].vecOfNeurons[indexNeuron].weight[indexWeight];
				aNNInput += (weights * inputWeight);
			}

			// Add the ANN's bias to the ANNInput by multiplying it by the weight of the Neuron
			aNNInput += (vecOfLayers[indexLayer].vecOfNeurons[indexNeuron].weight[NumInputs-1] * bias);

			// Filter the combined activation using the sigmoid function
			double sigmoidFilter = SigmoidFunction(aNNInput);
			
			// Push the output to a vector
			vectorOfOutputs.push_back(sigmoidFilter);

			indexOfInputsWeight = 0;
		}
	}
	// return the vector of outputs
	return vectorOfOutputs;
}

// Sigmoid Function
double CArtificalNeuralNet::SigmoidFunction(double aNNInput)
{
	return (1/(1 + exp(-aNNInput)));
}