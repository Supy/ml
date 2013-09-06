#include "CNeuroEvolution.h"

// Neuro Evolution Algorithm
// Create population with weights initialised inbetween (-1 and 1) and fitness set to 0
CNeuroEvolution::CNeuroEvolution(int populationSize, int numberOfweights){
	chromosomePopulationSize = populationSize;
	chromosomeLength = numberOfweights;
	totalPopulationFitness = 0;

	// Population initialised with chromosomes consisting of random weights (between -1 and 1) and with fitnesses set to 0
	for (int i=0; i<chromosomePopulationSize; ++i)
	{
		chromosomePopulationVector.push_back(Genome());

		for (int j=0; j<chromosomeLength; ++j)
		{
			// Random double between (-1,1)
			double randomDouble = (rand())/(RAND_MAX+1.0)-(rand())/(RAND_MAX+1.0);
			chromosomePopulationVector[i].weights.push_back(randomDouble);
		}
	}
}


// Mutate after cross-over (merging of two chromosomes) by perturbing the chromosomes weights by less than the max perturbation rate
void CNeuroEvolution::mutateChromosome(vector<double> &chromosome)
{
	// Random number must be less than 0.3 to be perturbed
	double maxPerturbingValue = 0.3;
	// Mutate the chromosomes weights according to the mutation rate
	for (int i=0; i<(int)chromosome.size(); ++i)
	{
		// Perturb the weight if the mutation rate is bigger than the random double between (0,1)
		double mutationRate = 0.7;
		double randomDouble = (rand())/(RAND_MAX+1.0);
		if (randomDouble < mutationRate)
		{
			// Random double between (-1,1)
			double randomDouble = (rand())/(RAND_MAX+1.0) - (rand())/(RAND_MAX+1.0);
			// Depending on the random double, perturb the weight by adding (or subtracting) a small ammount
			chromosome[i] += (randomDouble * maxPerturbingValue);
		}
	}
}

// Returns a chromo which contains the highest fitness
Genome CNeuroEvolution::GetChromoWithHighestFitness()
{
	//new chromosome representing the highest fitness
	Genome newChromosome;
	
	int highestFitnessIndex = 0;
	int secondHighestFitnessIndex = 0;
	for (int i=0; i<chromosomePopulationSize; ++i)
	{
		if (chromosomePopulationVector[i].genomeFitness>highestFitnessIndex){
			secondHighestFitnessIndex = highestFitnessIndex;
			highestFitnessIndex = i;
		}
	}

	newChromosome = chromosomePopulationVector[highestFitnessIndex];
	
	return newChromosome;
}

// Returns a chromo which contains the second highest fitness
Genome CNeuroEvolution::GetChromoWithSecondHighestFitness()
{
	//new chromosome representing the second highest fitness
	Genome newChromosome;
	
	int highestFitnessIndex = 0;
	int secondHighestFitnessIndex = 0;
	for (int i=0; i<chromosomePopulationSize; ++i)
	{
		if (chromosomePopulationVector[i].genomeFitness>highestFitnessIndex){
			secondHighestFitnessIndex = highestFitnessIndex;
			highestFitnessIndex = i;
		}
	}

	newChromosome = chromosomePopulationVector[secondHighestFitnessIndex];

	return newChromosome;
}

// neuro Evolve Cycle - evolves the given population and returns the evolved population of chromosomes
vector<Genome> CNeuroEvolution::neuroEvolveCycle(vector<Genome> &population)
{
	// Copy the given population vector into a local variable
	chromosomePopulationVector = population;

	// Reset the total population fitness
	ResetTotalFitness();

	//sort the chromosome population vector for picking the strongest two chromosomes (calculating total fitness)
	sort(chromosomePopulationVector.begin(), chromosomePopulationVector.end());

	// Calculate the total fitness
	CalculateTotalFitness();
  
	// New population vector for the evolved chromosomes (temporary)
	vector <Genome> newChromosomePopulationVector;

	// The 4 fittest genomes are inserted into a new chromosome population vector (elitism)
	// This removes the chance of losing the best genomes
	int numElite = 4;
		
	while(numElite--)
	{
		newChromosomePopulationVector.push_back(chromosomePopulationVector[(chromosomePopulationSize - 1) - numElite]);
	}

	// Evolve the population (repeat until the evolved population has been generated)
	while ((int)newChromosomePopulationVector.size() < chromosomePopulationSize)
	{
		//Get 2 best chromosomes (highest and second highest fitness)
		Genome bestChromosome = GetChromoWithHighestFitness();
		Genome secondBestChromosome = GetChromoWithSecondHighestFitness();

		// Create 2 new merged chromosomes of the 2 fittest chromosomes (Accomplished by cross-over)
		vector<double> mergedChromosome1, mergedChromosome2;
		// (0, chromosomeLength-1)
		// rand()%((b-a)+1)+a
		int randomInt = rand()%((chromosomeLength - 1)-0+1)+0;
		// Cross-over point (0, chromosomeLength-1) - Not that accurate (splitting a chromosome)
		int crossOverPoint = randomInt;

		// Create 2 merged chromosomes (offsprings)
		for (int i=0; i<crossOverPoint; ++i)
		{
			mergedChromosome1.push_back(bestChromosome.weights[i]);
			mergedChromosome2.push_back(secondBestChromosome.weights[i]);
		}

		for (int i=crossOverPoint; i<(int)bestChromosome.weights.size(); ++i)
		{
			mergedChromosome1.push_back(secondBestChromosome.weights[i]);
			mergedChromosome2.push_back(bestChromosome.weights[i]);
		}

		//mutate the new merged chromosomes
		mutateChromosome(mergedChromosome1);
		mutateChromosome(mergedChromosome2);

		// push the merged and mutated chromosomes into a new vector containing the new population
		newChromosomePopulationVector.push_back(Genome(mergedChromosome1, 0));
		newChromosomePopulationVector.push_back(Genome(mergedChromosome2, 0));
	}
	// The population is now set equal to the new evolved population
	chromosomePopulationVector = newChromosomePopulationVector;

	return chromosomePopulationVector;
}

//	calculates total fitness score
void CNeuroEvolution::CalculateTotalFitness()
{
	totalPopulationFitness = 0;
	
	for (int i=0; i<chromosomePopulationSize; ++i)
	{	
		totalPopulationFitness	+= chromosomePopulationVector[i].genomeFitness;
	}
}

// Reset the total fitness
void CNeuroEvolution::ResetTotalFitness()
{
	totalPopulationFitness = 0;
}



