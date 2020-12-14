#include <iostream>

#include "hvrp.h"

int main()
{
	srand(1);
	Instance& instance = readInstance("data.txt");
	Solution* solution = new Solution(instance.truckTypes, instance.trucksPerTypes);
    std::cout << "clients: " << instance.numberClients << "; trucks " << instance.truckTypes << std::endl;
	std::cout << "d12: " << instance.distances[1][2] << "; capacities[3] " << instance.trucksCapacities[3] << "; quantityPerClient[10] " << instance.quantityPerClient[10] << std::endl;

	nearNeighbours(instance, *solution);
	//nearNeighboursRandom(instance, *solution);
	split(instance, *solution);
	
	localSearch(instance, *solution, 1000);

	std::cout << solution->cost << std::endl;

	Solution& solution2 = grasp(instance, *solution, 10, 100, 100);
	std::cout << solution2.cost << std::endl;
}
