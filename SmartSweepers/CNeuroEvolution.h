#ifndef CUNEALG_H
#define CUNEALG_H

//Neuro Evolution class - Used to adjust genome weights in a neural network (feedforward).

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "utils.h"

using namespace std;

// Genome struct with weight and fitness attributes
struct Genome
{
	vector <double>	weights;
	double genomeFitness;

	Genome(){
		genomeFitness = 0;	
	}
	// Genome stores a weight and fitness
	Genome( vector <double> weightsTemp, double fitness){
		weights = weightsTemp;
		genomeFitness = fitness;
	}

	// Overload the "<" operator (Genome sorting)
	friend bool operator<(const Genome& lhs, const Genome& rhs)
	{
		return (lhs.genomeFitness < rhs.genomeFitness);
	}
};


//	Neuro Evolution class
class CNeuroEvolution
{
private:
	
	// This holds the population of chromosomes in a vector
	vector <Genome> chromosomePopulationVector;

	// Size of chromosome population
	int chromosomePopulationSize;
	
	// Number of weights per chromosome
	int chromosomeLength;

	// total fitness of population
	double totalPopulationFitness;

	// Mutate a chromosome
	void mutateChromosome(vector<double> &chromosome);
	
	// Get Chromos with Highest & Second Highest Fitness
	Genome GetChromoWithHighestFitness();
	Genome GetChromoWithSecondHighestFitness();

	// Total Fitness
	void CalculateTotalFitness();

	// Reset Total Fitness
	void ResetTotalFitness();


public:
		
	CNeuroEvolution(int populationSize, int numberOfWeights);

	// One Neuro Evolution Cycle.
	vector<Genome> neuroEvolveCycle(vector<Genome> &oldPopulation);

	// Get the chromosone population vector
	vector<Genome> const GetChromosomePopulationVector(){return chromosomePopulationVector;}
};

#endif



