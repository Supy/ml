#pragma once
#include <vector>
class CNode
{
public:
	CNode(int numWeights);
	~CNode(void);

	int numWeights;
	double * weights;
	double delta, output;
};