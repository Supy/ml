#pragma once
#include "CNode.h"
#include <vector>
#include <array>
#include "CCollisionObject.h"
#include <math.h>
#include "TrainingData.h"

class CMlp{
private:
	std::vector<int> numNodes;
	std::vector<TrainingData> trainingData;

	CNode *** nodes;
	double learningRate;
	double maxError;

	void LoadTrainingExamples();

public:
	CMlp(void);
	~CMlp(void);

	void		Train();
	void		SetNodeInput(int nodeIndex, bool minePresent);
	void		CalculateOutput();	
	double		GetOutput(int outputIndex);

};

