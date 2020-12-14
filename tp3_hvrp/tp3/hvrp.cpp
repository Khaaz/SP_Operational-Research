#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "hvrp.h"

Instance& readInstance(std::string path) {
	std::string line;

	Instance* instance;
	int nodes;
	int types;
	int tmp;

	std::ifstream infile(path);

	std::getline(infile, line);
	std::istringstream iss(line);

	iss >> nodes >> types;
	instance = new Instance(nodes, types);

	for (int i = types - 1; i >= 0; --i) { // init trucks types
		std::getline(infile, line);
		std::istringstream iss2(line);
		iss2 >> instance->trucksPerTypes[i] >> instance->trucksCapacities[i] >> instance->trucksFixedCost[i] >> instance->trucksVariableCost[i];
	}

	for (int i = 0; i < nodes + 1; ++i) { // init distances
		std::getline(infile, line);
		std::istringstream iss2(line);
		
		for (int j = 0; j <= nodes; ++j) { // nodes = clients (sans le depot)
			iss2 >> tmp;
			instance->distances[i][j] = (float)tmp / 1000.0;
		}
	}

	instance->quantityPerClient[0] = -1; // valeur impossible, on prends les valeurs du tableau a partir de 1 pour faire correspondre les numeros des clients
	for (int i = 1; i < instance->numberClients + 1; ++i) { // init clients capacity
		std::getline(infile, line);
		std::istringstream iss2(line);
		iss2 >> tmp >> instance->quantityPerClient[i];
	}

	return *instance;
}

// PLUS PROCHES VOISINS
int nextNearNeighbour(Instance& instance, bool* visited, int start) { // plus proche voisin
	int nearestIndex = -1, nearestValue = INT_MAX;

	// <= car nb clients + depot
	for (int i = 1; i <= instance.numberClients; ++i) { // on commence a 1 pour enlever le depot

		if (!visited[i] && i != start && instance.distances[start][i] < nearestValue) {
			nearestIndex = i;
			nearestValue = instance.distances[start][i];
		};
	}
	return nearestIndex;
}

Tour& nearNeighbours(Instance& instance, Solution& solution) {
	int nbCurClient = 0, curIndex = 0, newIndex = 0;
	Tour* tour = new Tour;

	bool* visited = new bool[instance.numberClients + 1]{ false };
	tour->list.push_back(0);

	for (int i = 0; i < instance.numberClients; ++i) {
		newIndex = nextNearNeighbour(instance, visited, curIndex); // todo return 2 valeurs (aussi la distance)
		visited[newIndex] = true;
		tour->list.push_back(newIndex);
		tour->distance += instance.distances[curIndex][newIndex];
		curIndex = newIndex;
	}

	// retour au depot
	tour->distance += instance.distances[curIndex][0];

	// ajout a la solution
	// solution.cost += tour->distance;
	solution.tourGeant = tour;

	return *tour;
}

// PLUS PROCHES VOISINS RANDOM
void sortedInsert(int* tabIndexes, int* tabValues, int value, int index, int size) {
	int i = size - 1;
	while (i > 0 && value < tabValues[i - 1]) {
		--i;
	}

	for (int j = size - 1; j > i; --j) {
		tabValues[j] = tabValues[j - 1];
		tabIndexes[j] = tabIndexes[j - 1];
	}
	tabValues[i] = value;
	tabIndexes[i] = index;
}

int nextXNearNeighbours(Instance& instance, bool* visited, int start, int size) { // plus proche voisin
	int nearestIndexes[5] = { -1, -1, -1, -1, -1 };
	int nearestValues[5] = { INT_MAX , INT_MAX, INT_MAX, INT_MAX, INT_MAX };
	int randIndex;

	// <= car nb clients + depot
	for (int i = 1; i <= instance.numberClients; ++i) { // on commence a 1 pour enlever le depot

		if (!visited[i] && i != start && instance.distances[start][i] < nearestValues[4]) {
			sortedInsert(nearestIndexes, nearestValues, instance.distances[start][i], i, size);
		}
	}
	randIndex = rand() % 5;
	return nearestIndexes[randIndex];
}

Tour& nearNeighboursRandom(Instance& instance, Solution& solution) {
	const int SIZE = 5;
	int nbCurClient = 0, curIndex = 0, newIndex = 0, lenghtIndexes = 0;
	int leftIndexes[SIZE] = { -1,-1,-1,-1,-1 }, randIndex;

	Tour* tour = new Tour;

	bool* visited = new bool[instance.numberClients + 1]{ false };
	tour->list.push_back(0);

	// -6 = -1 (dernier client) -5 (5 dernier voisins)
	for (int i = 0; i < instance.numberClients - SIZE; ++i) {
		newIndex = nextXNearNeighbours(instance, visited, curIndex, SIZE); // todo return 2 valeurs (aussi la distance)
		visited[newIndex] = true;
		tour->list.push_back(newIndex);
		tour->distance += instance.distances[curIndex][newIndex];
		curIndex = newIndex;
	}

	for (int i = 1; i < instance.numberClients; ++i) {
		if (!visited[i]) {
			leftIndexes[lenghtIndexes] = i;
			++lenghtIndexes;
		}
	}


	for (int i = 0; i < SIZE; ++i) {
		//Genere

		randIndex = lenghtIndexes != 1 ? rand() % (lenghtIndexes - 1) : 0;
		newIndex = leftIndexes[randIndex];

		// reformer tableau indexs
		leftIndexes[randIndex] = leftIndexes[lenghtIndexes - 1];
		--lenghtIndexes;

		//Ajoute au tour	
		tour->list.push_back(newIndex);
		tour->distance += instance.distances[curIndex][newIndex];
		curIndex = newIndex;
	}

	// retour au depot
	tour->distance += instance.distances[curIndex][0];

	// ajout a la solution
	// solution.cost += tour->distance;
	solution.tourGeant = tour;

	return *tour;
}

// LOCAL SEARCH
void swap(int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

float dist(Instance& instance, int a, int b) {
	return instance.distances[a][b];
}

void reverse(std::vector<int>& arr, int start, int end) {
	int j = 0;
	for (int i = start; i <= (start + end) / 2; ++i) {
		swap(&(arr[start + j]), &(arr[end - j]));
		++j;
	}
}

bool twoOpt(Instance &instance, Tour& tour) {
	int size = tour.list.size();
	float bestDiff = 0;
	int bestI = 0;
	int bestJ = 0;

	for (int i = 0; i < size - 2; ++i) {

		for (int j = i + 2; j < size; ++j) {

			if ((j + 1) % size != i) { // verifie que les segments ne sont pas adjacents (circulaire)		
				float oldDist = dist(instance, tour.list[i], tour.list[i + 1]) + dist(instance, tour.list[j], tour.list[(j + 1) % size]);
				float newDist = dist(instance, tour.list[i], tour.list[j]) + dist(instance, tour.list[i + 1], tour.list[(j + 1) % size]);
				float diff = oldDist - newDist; // + diff est grand et mieux c'est
				
				if (diff > bestDiff) {
					bestDiff = diff;
					bestJ = j;
					bestI = i;
				}
			}
		}
	}

	if (bestDiff > 0) {
		swap(&(tour.list[bestI + 1]), &(tour.list[bestJ]));
		// decaler
		reverse(tour.list, bestI + 2, bestJ - 1);
		tour.distance -= bestDiff;
		tour.setCost(instance.trucksVariableCost, instance.trucksFixedCost);
		return true;
	}

	return false;
}

/*
* Insert le point ameliorant le plus de tous
*/
bool insertion(Instance& instance, Tour& tour) {
	int size = tour.list.size();

	float bestDiff = 0;
	int bestToInsert = -1;
	int bestWhereInsert = -1;
	
	for (int i = 1; i < size; ++i) {

		int G = tour.list[i];
		int a = tour.list[i - 1];
		int b = tour.list[(i + 1) % size];

		float distGagne = dist(instance, a, G) + dist(instance, G, b) - dist(instance, a, b);
		for (int j = 0; j < size; ++j) {
			if (j == i || j == i - 1) { // on est sur le point actuel
				continue;
			}
			
			int x = tour.list[j];
			int y = tour.list[(j + 1) % size];
			float distPerdu = dist(instance, x, G) + dist(instance, G, y) - dist(instance, x, y);
			
			if (distGagne > distPerdu) {
				float diff = distGagne - distPerdu;
				if (diff > bestDiff) {
					bestDiff = diff;
					bestWhereInsert = j;
					bestToInsert = i;
				}
			}
		}
	}

	if (bestDiff > 0) {
		tour.list.insert(tour.list.begin() + bestWhereInsert + 1, tour.list[bestToInsert]);
		
		if (bestWhereInsert < bestToInsert) {
			tour.list.erase(tour.list.begin() + bestToInsert + 1);
		}
		else {
			tour.list.erase(tour.list.begin() + bestToInsert);
		}

		tour.distance -= bestDiff;
		tour.setCost(instance.trucksVariableCost, instance.trucksFixedCost);
		return true;
	}

	return false;
}

void twoOptInter() {

}

void localSearch(Instance& instance, Solution& solution, int iter) {
	for (int i = 0; i < iter; ++i) {
		float r = (float)rand() / (float)RAND_MAX;
		if (r < 0.3) {
			for (int j = 0; j < solution.nbTour; ++j) {
				float cost = (solution.list[j])->cost;
				if (twoOpt(instance, *(solution.list[j])) ) {
					solution.cost += solution.list[j]->cost - cost;
				}
			}
		}
		else if (r < 0.6) {
			for (int j = 0; j < solution.nbTour; ++j) {
				float cost = (solution.list[j])->cost;
				if (insertion(instance, *(solution.list[j]))) {
					solution.cost += solution.list[j]->cost - cost;
				}
			}
		}
		else {
			twoOptInter();
		}
	}
}

// SPLIT

bool betterSameTrucks(int* trucks1, int* trucks2, int numberTrucks) {
	for (int i = 0; i < numberTrucks; ++i) {
		if (trucks1[i] < trucks2[i]) {
			return false;
		}
	}
	return true;
}

bool betterTrucks(int* trucks1, int* trucks2, int numberTrucks) {
	for (int i = 0; i < numberTrucks; ++i) {
		if (trucks1[i] <= trucks2[i]) {
			return false;
		}
	}
	return true;
}

/*
* truks 1 == trucks2 => -1
* sinon return index different
*/
int sameTrucks(int* trucks1, int* trucks2, int numberTrucks) {
	for (int i = 0; i < numberTrucks; ++i) {
		if (trucks1[i] != trucks2[i]) {
			return i;
		}
	}
	return -1;
}
/*
* vrai si nouveau label > label existant
*/
bool betterLabel(Label& label, int* nbTrucksLeftCopy, float cost, int numberTrucks) {
	if (label.cost < cost && betterSameTrucks(label.nbTrucksLeft, nbTrucksLeftCopy, numberTrucks)) {
		return false;
	}
	if (label.cost <= cost && betterTrucks(label.nbTrucksLeft, nbTrucksLeftCopy, numberTrucks)) {
		return false;
	}
	return true;

}
/*
* faux si AU MOINS 1 label existant est > a notre label
* 
* si meme trucks => comparer distance
*	si distance existant <= distance --> faux
* si meme distance => comparer les trucks
*	si trucks <= trucks existant --> faux
*/
//bool betterLabels(std::vector<Label>& nextClientLabels, int* nbTrucksLeftCopy, float cost, int numberTrucks) {
//	for (Label& label : nextClientLabels) {
//		if (!betterLabel(label, nbTrucksLeftCopy, cost, numberTrucks)) {
//			return false;
//		}
//	}
//	return true;
//}

bool betterLabels(std::vector<Label>& nextClientLabels, int* nbTrucksLeftCopy, float cost, int nbType) {
	int res = 1;
	int i = 0;
	while (res != 0 && i != nextClientLabels.size()) {
		Label& a = nextClientLabels[i];
		int h1 = 1; // hypotèse a < b
		int h2 = 2; // hypotèse b < a
		int nA = 0;
		int nB = 0;

		for (int j = 0; j < nbType; ++j) {
			nA += a.nbTrucksLeft[j];
			nB += nbTrucksLeftCopy[j];
		}

		if (nB > nA || cost < a.cost) {
			h1 = 0;
		}
		if (nA > nB || a.cost < cost) {
			h2 = 0;
		}

		res = h1 + h2;
		++i;
	}
	return res == 1;
}

int minLabel(std::vector<Label>& labels) {
	int minIndex = -1;
	float minValue = INT_MAX;
	for (int i = 0; i < labels.size(); ++i) {
		if (labels[i].cost < minValue) {
			minIndex = i;
			minValue = labels[i].cost;
		}
	}
	return minIndex;
}

float calculateCost(float* trucksVariableCost, float* trucksFixedCost, int truck, float distance) {
	return (trucksVariableCost[truck] * distance) + trucksFixedCost[truck];
}

void split(Instance& instance, Solution& solution) {
	std::vector<Label>** labels = new std::vector<Label>*[instance.numberClients + 1];
	float distance = 0;
	int quantity = 0;
	int* prevLabels = new int[instance.truckTypes]{ 0 };
	for (int i = 0; i < instance.numberClients + 1; ++i) {
		labels[i] = new std::vector<Label>;
	}

	labels[0]->push_back(Label{ instance.truckTypes, instance.trucksPerTypes, 0, 0, 0 , 0, 0 });

	for (int i = 0; i <= instance.numberClients; ++i) { // pour chaque clients
		int client = solution.tourGeant->list[i]; // le client selon l'ordre du tour geant

		for (int j = 0; j < labels[client]->size(); ++j) { // pour chaque labels
			Label& curLabel = (*labels[client])[j];

			int size = instance.truckTypes;

			int k = 1; // profondeur de la tournee actuelle
			
			for (int w = 0; w < instance.truckTypes; ++w) {
				prevLabels[w] = j;
			}

			bool go = true;
			while (go && size > 0 && (i + k) <= instance.numberClients) { // tant que il reste des camions

				int l = 0;
				
				int curClient = solution.tourGeant->list[i + k];
				int prevClient = solution.tourGeant->list[i + k - 1];

				if (k == 1) {
					distance = instance.distances[0][curClient] * 2;
					quantity = instance.quantityPerClient[curClient];
				}
				else {
					// avec 0 A B => distance += distance AB - distance A0 + distance B0
 					distance += instance.distances[prevClient][curClient] - instance.distances[0][prevClient] + instance.distances[0][curClient];
					quantity += instance.quantityPerClient[curClient];
				}

				go = false;
				while (l < instance.truckTypes && instance.trucksCapacities[l] >= quantity) { // tant que ce type de camion peut faire la tournee
					if (prevLabels[l] == -1) { // Si on a pas pu ajouter un label avant car moins bon
						++l;
						continue;
					}

					int* prevClientTrucksLeft = (*labels[prevClient])[prevLabels[l]].nbTrucksLeft;
					if (prevClientTrucksLeft[l] > 0) {
						// add label
						int* nbTrucksLeftCopy;
						if (k == 1) {
							nbTrucksLeftCopy = new int[instance.truckTypes];
							memcpy(nbTrucksLeftCopy, curLabel.nbTrucksLeft, instance.truckTypes * sizeof(int));
							--nbTrucksLeftCopy[l];
						}
						else {
							nbTrucksLeftCopy = prevClientTrucksLeft;
						}

						float cost = calculateCost(instance.trucksVariableCost, instance.trucksFixedCost, l, distance);

						if (betterLabels(*labels[curClient], nbTrucksLeftCopy, cost, instance.truckTypes)) {
							labels[curClient]->push_back(Label{ instance.truckTypes, nbTrucksLeftCopy, distance, cost, quantity, prevClient, prevLabels[l] });
							prevLabels[l] = labels[curClient]->size() - 1;
							go = true;
						}
						else {
							prevLabels[l] = -1;
						}
					}
					else {
						prevLabels[l] = -1;
					}
					++l;
				}
				size = l;
				++k;
			}
		}
	}

	int clientIndex = solution.tourGeant->list[instance.numberClients];

	int rtf = minLabel(*labels[clientIndex]);
	Label& current = (*labels[clientIndex])[rtf];
		
	Tour *t = new Tour;
	t->distance = instance.distances[0][clientIndex];

	// Tant qu'on est pas au dépot
	while (current.indexFather != 0 ){
	
		Label& father = (*labels[current.indexFather])[current.indexLabelFather];
		
		t->list.push_back(clientIndex);

		int res = sameTrucks(current.nbTrucksLeft, father.nbTrucksLeft, instance.truckTypes);
		if (res != -1) { // si ils sont differents
			// Fin d'une tournée
			t->distance += instance.distances[clientIndex][0];
			t->truck = res;
			t->setCost(instance.trucksVariableCost, instance.trucksFixedCost);
			solution.cost += t->cost;
			solution.list[solution.nbTour++] = t;

			// Nouvelle tournee
			t = new Tour;
			current = father;
			clientIndex = current.indexFather;
			t->distance = instance.distances[0][clientIndex];
		}
		else { // Toujours la meme tournee
			t->distance += instance.distances[clientIndex][father.indexFather];
			current = father;
			clientIndex = current.indexFather;
		}
	}
}

// Grasp

Tour* splitToGiantTour(Solution &solution){
	Tour *t = new Tour();
	t->list.push_back(0);
	for(int i = 0; i < solution.nbTour; ++i){
		for(int j = 0; j < solution.list[i]->list.size(); ++j){
			t->list.push_back(solution.list[i]->list[j]);
		}
	}
	return t;
}

Tour** genNeighbours(Tour& tour, int nbNeighbours) {
	Tour** tours = new Tour* [nbNeighbours];
	
	for (int i = 0; i < nbNeighbours; ++i) {	
		tours[i] = tour.copyTour();
	}
	return tours;
}

Solution& grasp(Instance& instance, Solution& solution, int nbNeighbours, int iter, int localSearchIter) {
	//global best
	Solution* bestOfBestSolution = &solution;
	//local best
	Solution* bestSolution = &solution;
	//array of neighbours
	Tour** tours = nullptr;
	Solution** solutions = new Solution*[nbNeighbours];
	Tour * tmp;

	for (int i = 0; i < iter; ++i) {
		tmp = splitToGiantTour(*bestSolution);

		tours = genNeighbours(*tmp, nbNeighbours);

		for (int i = 0; i < nbNeighbours; ++i) {
			solutions[i] = new Solution(instance.truckTypes, instance.trucksPerTypes);
			solutions[i]->tourGeant = tours[i];

			split(instance, *(solutions[i]));
			localSearch(instance, *(solutions[i]), localSearchIter);
			
			if (solutions[i]->cost < bestSolution->cost) {
				bestSolution = solutions[i];
			}
		}

		if (bestSolution->cost < bestOfBestSolution->cost) {
			bestOfBestSolution = bestSolution;
		}
	}

	return *bestOfBestSolution;
}