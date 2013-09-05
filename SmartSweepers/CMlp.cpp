#include "CMlp.h"


CMlp::CMlp(void) : learningRate(0.6), maxError(0.001)
{

	// 4 input nodes, 10 hidden nodes and 2 output nodes (direction and speed).
	numNodes.push_back(4);
	numNodes.push_back(10);
	numNodes.push_back(2);

	nodes = new CNode ** [numNodes.size()];
	
	// Setup the nodes in each layer.
	for(int i = 0; i < numNodes.size(); i++){
		nodes[i] = new CNode * [numNodes[i]];
		for(int j=0; j < numNodes[i]; j++){
			if(i > 0)
				nodes[i][j] = new CNode(numNodes[i-1]);
			else
				nodes[i][j] = new CNode(0);
		}
	}

	LoadTrainingExamples();
}


CMlp::~CMlp(void)
{
	for(int i=0; i < numNodes.size(); i++){
		delete [] nodes[i];
	}
}

void CMlp::Train(){

	bool errors = false;
	do{
		errors = false;
		// Ensure all our training examples pass.
		for(auto it = trainingData.begin(); it != trainingData.end(); it++){

			// Set the values of our input nodes.
			for(int inputIndex = 0; inputIndex < numNodes[0]; inputIndex++){
				SetNodeInput(inputIndex, it->inputs[inputIndex]);
			}

			double actualSteeringOutput = GetOutput(0);
			double targetSteeringOutput = it->steeringOutput;

			double actualSpeedOutput = GetOutput(1);
			double targetSpeedOutput = it->speedOutput;

			// If the error margin is too great, we need to do another iteration of learning.
			if(abs(targetSteeringOutput - actualSteeringOutput) > maxError || abs(targetSpeedOutput - actualSpeedOutput) > maxError)
				errors = true;

			// Propagate the error backwards
			for(int layer = numNodes.size()-1; layer >= 1; layer--){
				for(int node = 0; node < numNodes[layer]; node++){
					CNode & currentNode = *nodes[layer][node];

					// Output node calculates the error differently 
					if(layer == numNodes.size()-1){
						// Node 0 = direction output node, node 1 = speed output node
						if(node == 0)
							currentNode.delta = learningRate * currentNode.output * (1-currentNode.output) * (targetSteeringOutput-currentNode.output);
						else
							currentNode.delta = learningRate * currentNode.output * (1-currentNode.output) * (targetSpeedOutput-currentNode.output);
					}else{
						double deltaSum = 0.0;

						// Hidden layer nodes' errors are the sum of their error contribution to the next layer.
						for(int u = 0; u < numNodes[layer+1]; u++){
							deltaSum += nodes[layer+1][u]->delta * nodes[layer+1][u]->weights[node];
						}

						currentNode.delta = deltaSum * learningRate * currentNode.output * (1.0 - currentNode.output);
					}
				}
			}

			// Update our weights
			for(int layer = numNodes.size()-1; layer >= 1; layer--){
				for(int node = 0; node < numNodes[layer]; node++){
					CNode & currentNode = *nodes[layer][node];

					for(int wi = 0; wi < currentNode.numWeights; wi++){
						currentNode.weights[wi] += nodes[layer-1][wi]->output * currentNode.delta;
					}
				}
			}
		}
	}while(errors);
}

// Sets the input of the appropriate input node. The input to the node is whether a mine is present in that quadrant.
void CMlp::SetNodeInput(int nodeIndex, bool minePresent){
	if(nodeIndex >= 0 && nodeIndex < numNodes[0]){
		nodes[0][nodeIndex]->output = minePresent ? 1.0 : 0.0;
	}
}

double CMlp::GetOutput(int outputIndex){

	for(int layer = 1; layer < numNodes.size(); layer++){
		for(int node = 0; node < numNodes[layer]; node++){
			double input = 0;
			CNode & currentNode = *nodes[layer][node];

			// Sum up the inputs from the previous layer and their respective weights to this node.
			for(int wi=0; wi < currentNode.numWeights; wi++){
				input += nodes[layer-1][wi]->output * currentNode.weights[wi];
			}

			double result = 1.0 / (1 + exp(-input));
			currentNode.output = result;
		}
	}

	// Get the value of the first (and only) output node.
	return nodes[numNodes.size()-1][outputIndex]->output;
}

// A comprehensive list of training examples. Contains scenarios the sweepers might find themselves in.
void CMlp::LoadTrainingExamples(){
	using namespace std;

	// Each true/false value represents whether there is a mine in front of the sweeper in a 
	// particular quadrant. Quadrants are 4 "regions" in front of the mine sweeper.
	array<bool, 4> i1 = {false, false, false, false};
	double steering1 = 0.5;
	double speed1 = 0.9;
	TrainingData t1(i1, steering1, speed1);

	array<bool, 4> i2 = {true, false, false, false};
	double steering2 = 0.5;
	double speed2 = 0.8;
	TrainingData t2(i2, steering2, speed2);

	array<bool, 4> i3 = {true, true, false, false};
	double steering3 = 0.6;
	double speed3 = 0.5;
	TrainingData t3(i3, steering3, speed3);

	array<bool, 4> i4 = {true, true, true, false};
	double steering4 = 0.3;
	double speed4 = 0.3;
	TrainingData t4(i4, steering4, speed4);

	array<bool, 4> i5 = {true, true, true, true};
	double steering5 = 0.8;
	double speed5 = 0.2;
	TrainingData t5(i5, steering5, speed5);

	array<bool, 4> i6 = {false, true, true, true};
	double steering6 = 0.2;
	double speed6 = 0.3;
	TrainingData t6(i6, steering6, speed6);

	array<bool, 4> i7 = {false, false, true, true};
	double steering7 = 0.4;
	double speed7 = 0.5;
	TrainingData t7(i7, steering7, speed7);

	array<bool, 4> i8 = {false, false, false, true};
	double steering8 = 0.5;
	double speed8 = 0.8;
	TrainingData t8(i8, steering8, speed8);

	array<bool, 4> i9 = {false, true, true, false};
	double steering9 = 0.8;
	double speed9 = 0.5;
	TrainingData t9(i9, steering9, speed9);

	array<bool, 4> i10 = {true, false, false, true};
	double steering10 = 0.5;
	double speed10 = 0.7;
	TrainingData t10(i10, steering10, speed10);

	array<bool, 4> i11 = {false, false, true, false};
	double steering11 = 0.4;
	double speed11 = 0.7;
	TrainingData t11(i11, steering11, speed11);

	array<bool, 4> i12 = {false, true, false, false};
	double steering12 = 0.6;
	double speed12 = 0.7;
	TrainingData t12(i12, steering12, speed12);

	array<bool, 4> i13 = {true, false, true, false};
	double steering13 = 0.8;
	double speed13 = 0.6;
	TrainingData t13(i13, steering13, speed13);

	array<bool, 4> i14 = {false, true, false, true};
	double steering14 = 0.2;
	double speed14 = 0.6;
	TrainingData t14(i14, steering14, speed14);

	array<bool, 4> i15 = {true, true, false, true};
	double steering15 = 0.8;
	double speed15 = 0.3;
	TrainingData t15(i15, steering15, speed15);

	array<bool, 4> i16 = {true, false, true, true};
	double steering16 = 0.2;
	double speed16 = 0.3;
	TrainingData t16(i16, steering16, speed16);

	trainingData.push_back(t1);
	trainingData.push_back(t2);
	trainingData.push_back(t3);
	trainingData.push_back(t4);
	trainingData.push_back(t5);
	trainingData.push_back(t6);		
	trainingData.push_back(t7);
	trainingData.push_back(t8);
	trainingData.push_back(t9);
	trainingData.push_back(t10);
	trainingData.push_back(t11);
	trainingData.push_back(t12);
	trainingData.push_back(t13);
	trainingData.push_back(t14);
	trainingData.push_back(t15);
	trainingData.push_back(t16);

	/*	trainingData.push_back(t1);
	trainingData.push_back(t2);
	//trainingData.push_back(t3);
	//trainingData.push_back(t4);
	trainingData.push_back(t5);
	//trainingData.push_back(t6);		
	//trainingData.push_back(t7);
	trainingData.push_back(t8);
	trainingData.push_back(t9);
	//trainingData.push_back(t10);
	trainingData.push_back(t11);
	trainingData.push_back(t12);
//	trainingData.push_back(t13);
	//trainingData.push_back(t14);
	//trainingData.push_back(t15);
	//trainingData.push_back(t16);*/
}
