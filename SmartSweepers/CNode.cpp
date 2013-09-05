#include "CNode.h"


CNode::CNode(int nWeights) : numWeights(nWeights), weights(new double[numWeights]), delta(0.0), output(0.0)
{
	// Setup random weights
	for(int i = 0; i < numWeights; i++)
		weights[i] = -1.0 + (double)rand() / ((double)RAND_MAX / 2.0);
}


CNode::~CNode(void)
{

}
