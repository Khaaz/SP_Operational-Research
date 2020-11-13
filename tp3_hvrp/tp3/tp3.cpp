#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

const int MAX = 500;
const int CLIENT_TOURNEE = 25;

struct Instance {
    int numberClients; // n // number of clients = number of nodes
	int truckTypes; // nc // nombre typecamions
    int** distances; // distance
    int* trucksCapacities; // capacites
    int* trucksVariableCost; //cv // cout variable 
    int* trucksFixedCost; //cf // cout fixe
    int* trucksPerTypes; // ntc // nombre vehicule par type
    int* quantityPerClient; // quantite a collecter / client

	Instance(int nodes, int types) : numberClients(nodes), truckTypes(types) {
		trucksCapacities = new int[types];
		trucksVariableCost = new int[types];
		trucksFixedCost = new int[types];
		trucksPerTypes = new int[types];

		distances = new int* [nodes + 1];
		
		quantityPerClient = new int[nodes + 1];

		for (int i = 0; i < nodes + 1; ++i) {
			distances[i] = new int[nodes + 1];
		}
	}
	~Instance() {
		delete[] trucksCapacities;
		delete[] trucksVariableCost;
		delete[] trucksFixedCost;
		delete[] trucksPerTypes;
	}
};

struct Tournee {
    int nbClients = 0; // nb client
    int cost = 0;
    int truck; // le camion (type de camion)
    int *list; // clients
	Tournee(int nbClient = CLIENT_TOURNEE) : nbClients(0) {
		truck = 0;
		cost = 0;
		list = new int[nbClient];
	}
	~Tournee() {
		delete[] list;
	}
};

struct Solution {
    int nbTournee = 0; // nombre de tournee
    int cost = 0; // cout total
	Tournee* *list; //nombre de camion * types]; // ensemble de tournee
	Solution(int nbTypes, int* nbTrucksPerTypes) {
		int nbTrucks = 0;
		for (int i = 0; i < nbTypes; ++i) {
			nbTrucks += nbTrucksPerTypes[i];
		}
		list = new Tournee*[nbTrucks];
		int cost = 0;
	}
	~Solution() {
		delete[] list;
	}
};

Instance& readInstance(std::string path) {
	std::string line;

	Instance* instance;
	int nodes;
	int types;
	int clients;
	int tmp;

	std::ifstream infile(path);

	std::getline(infile, line);
	std::istringstream iss(line);

	iss >> nodes >> types;
	instance = new Instance(nodes, types);

	for (int i = 0; i < types; ++i) { // init trucks types
		std::getline(infile, line);
		std::istringstream iss(line);
		iss >> instance->trucksPerTypes[i] >> instance->trucksCapacities[i] >> instance->trucksFixedCost[i] >> instance->trucksVariableCost[i];
	}

	for (int i = 0; i < nodes + 1; ++i) { // init distancess
		std::getline(infile, line);
		std::istringstream iss(line);
		for (int j = 0; j < nodes + 1; ++j) {
			iss >> instance->distances[i][j];
		}
	}

	instance->quantityPerClient[0] = -1; // valeur impossible, on prends les valeurs du tableau a partir de 1 pour faire correspondre les numeros des clients
	for (int i = 1; i < instance->numberClients + 1; ++i) { // init clients capacity
		std::getline(infile, line);
		std::istringstream iss(line);
		iss >> tmp >> instance->quantityPerClient[i];
	}

	return *instance;
}

// plus proche voisins
int nextNearNeighbour(Instance& instance, bool *visited, int start) { // plus proche voisin
	int nearestIndex = -1, nearestValue = INT_MAX;
	
	for (int i = 1; i < instance.numberClients + 1; ++i) { // on commence a 1 pour enlever le depot
		
		if(!visited[i] && i != start && instance.distances[start][i] < nearestValue){
			nearestIndex = i;
			nearestValue = instance.distances[start][i];
		};
	}
	return nearestIndex;
}

Tournee& nearNeighbours(Instance& instance, Solution& solution) {
	int nbCurClient = 0, curIndex = 0, newIndex = 0;
	Tournee* tournee = new Tournee(instance.numberClients);

	bool* visited = new bool[instance.numberClients]{false};

	for (int i = 0; i < instance.numberClients - 1; ++i) {
		newIndex = nextNearNeighbour(instance, visited, curIndex); // todo return 2 valeurs (aussi la distance)
		visited[newIndex] = true;
		tournee->list[tournee->nbClients++] = newIndex;
		tournee->cost += instance.distances[curIndex][newIndex];
		curIndex = newIndex;
	}

	// retour au depot
	tournee->cost += instance.distances[curIndex][0];
	tournee->list[tournee->nbClients++] = 0;

	// ajout a la solution
	solution.cost += tournee->cost;
	solution.list[solution.nbTournee] = tournee;

	return *tournee;
}


// plus proche voisins random
int next5NearNeighbours(Instance& instance, bool* visited, int start) { // plus proche voisin
	int nearestIndexes[5] = { -1, -1, -1, -1, -1 }, nearestValues[5] = {-1, INT_MAX, INT_MAX, INT_MAX, INT_MAX };
	int randIndex;

	for (int i = 1; i < instance.numberClients + 1; ++i) { // on commence a 1 pour enlever le depot

		if (!visited[i] && i != start && instance.distances[start][i] < nearestValues[4]) {
			sortedInsert(nearestIndexes, nearestValues, instance.distances[start][i], i);
		};
	}
	randIndex = rand() % 5;
	return nearestIndexes[randIndex];
}

void sortedInsert(int *tabValues, int *tabIndexes, int value, int index){
	int i = 5;
	while(value < tabValues[i]){
		i--;
	}
	tabValues[5] = tabValues[i+1];
	tabValues[i + 1] = 2;
}

Tournee& nearNeighboursRandom(Instance& instance, Solution& solution) {
	int nbCurClient = 0, curIndex = 0, newIndex = 0;
	Tournee* tournee = new Tournee(instance.numberClients);

	bool* visited = new bool[instance.numberClients]{ false };

	for (int i = 0; i < instance.numberClients - 1; ++i) {
		newIndex = next5NearNeighbours(instance, visited, curIndex); // todo return 2 valeurs (aussi la distance)
		visited[newIndex] = true;
		tournee->list[tournee->nbClients++] = newIndex;
		tournee->cost += instance.distances[curIndex][newIndex];
		curIndex = newIndex;
	}

	// retour au depot
	tournee->cost += instance.distances[curIndex][0];
	tournee->list[tournee->nbClients++] = 0;

	// ajout a la solution
	solution.cost += tournee->cost;
	solution.list[solution.nbTournee] = tournee;

	return *tournee;
}

int main()
{
	Instance& instance = readInstance("data.txt");
	Solution* solution = new Solution(instance.truckTypes, instance.trucksPerTypes);
    std::cout << "clients: " << instance.numberClients << "; trucks " << instance.truckTypes << std::endl;
	std::cout << "d12: " << instance.distances[1][2] << "; capacities[3] " << instance.trucksCapacities[3] << "; quantityPerClient[10] " << instance.quantityPerClient[10] << std::endl;

	nearNeighbours(instance, *solution);
	std::cout << solution->cost << std::endl;
}
