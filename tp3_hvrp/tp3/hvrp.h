#pragma once

#include <vector>

const int MAX = 500;
const int CLIENT_TOUR = 25;

struct Instance {
	int numberClients; // n // number of clients = number of nodes
	int truckTypes; // nc // nombre typecamions
	float** distances; // distance
	int* trucksCapacities; // capacites
	float* trucksVariableCost; //cv // cout variable 
	float* trucksFixedCost; //cf // cout fixe
	int* trucksPerTypes; // ntc // nombre vehicule par type
	int* quantityPerClient; // quantite a collecter / client

	Instance(int nodes, int types) : numberClients(nodes), truckTypes(types) {
		trucksCapacities = new int[types];
		trucksVariableCost = new float[types];
		trucksFixedCost = new float[types];
		trucksPerTypes = new int[types];

		distances = new float* [nodes + 1];

		quantityPerClient = new int[nodes + 1];

		for (int i = 0; i < nodes + 1; ++i) {
			distances[i] = new float[nodes + 1];
		}
	}
	~Instance() {
		delete[] trucksCapacities;
		delete[] trucksVariableCost;
		delete[] trucksFixedCost;
		delete[] trucksPerTypes;
	}
};

struct Tour {
	float distance = 0;
	int truck; // le camion (type de camion)
	float cost;
	std::vector<int> list; // clients
	Tour() {
		truck = 0;
		distance = 0;
		cost = 0;
	}
	~Tour() {
	}

	Tour* copyTour() {

		Tour *t = new Tour();
		t->distance = distance;
		t->truck = truck;
		t->cost = cost;
		t->list = std::vector<int>(list);
		
		int r1 = rand() % (list.size()) +1;
		int r2 = rand() % (list.size()) +1;

		int tmp = t->list[r1];
		t->list[r1] = t->list[r2];
		t->list[r2] = tmp;

		return t;
	}
	void setCost(float* trucksVariableCost, float* trucksFixedCost) {
		cost = (trucksVariableCost[truck] * distance) + trucksFixedCost[truck];
	}
	
};

struct Solution {
	int nbTour = 0; // nombre de tournee
	float cost = 0; // cout total
	Tour* tourGeant;
	Tour** list; //nombre de camion * types]; // ensemble de tournee
	Solution(int nbTypes, int* nbTrucksPerTypes) {
		int nbTrucks = 0;
		for (int i = 0; i < nbTypes; ++i) {
			nbTrucks += nbTrucksPerTypes[i];
		}
		tourGeant = nullptr;
		list = new Tour*[nbTrucks + 1];
		cost = 0;
	}
	~Solution() {
		delete tourGeant;
		for (int i = 0; i < nbTour; ++i) {
			delete list[i];
		}
		delete[] list;
	}
};

struct Label {
	int* nbTrucksLeft; // tableau
	int size;
	float distance;
	float cost; // cout variable * distance + cout fixe (pour chaque camion et tournée etc)
	int quantity; // quantite dans le camion
	int indexFather;
	int indexLabelFather;
	Label(int size, int* nbTrucksLeft, float distance, float cost, int quantity, int indexFather, int indexLabelFather) : size(size), distance(distance), cost(cost), quantity(quantity), indexFather(indexFather), indexLabelFather(indexLabelFather) {
		this->nbTrucksLeft = new int[size];
		for (int i = 0; i < size; ++i) {
			this->nbTrucksLeft[i] = nbTrucksLeft[i];
		}
		// memcpy(this->nbTrucksLeft, nbTrucksLeft, size * sizeof(int));
	}
	Label(const Label& l) {
		size = l.size;
		distance = l.distance;
		cost = l.cost;
		quantity = l.quantity;
		indexFather = l.indexFather;
		indexLabelFather = l.indexLabelFather;
		nbTrucksLeft = new int[l.size];
		for (int i = 0; i < size; ++i) {
			nbTrucksLeft[i] = l.nbTrucksLeft[i];
		}
	}
	~Label() {
		delete[] nbTrucksLeft;
	}
};

Instance& readInstance(std::string path);

Tour& nearNeighbours(Instance& instance, Solution& solution);
Tour& nearNeighboursRandom(Instance& instance, Solution& solution);

void split(Instance& instance, Solution& solution);

void localSearch(Instance& instance, Solution& solution, int iter);
bool twoOpt(Instance& instance, Tour& tour);
bool insertion(Instance& instance, Tour& tour);

Solution& grasp(Instance& instance, Solution& solution, int nbNeighbours, int iter, int localSearchIter);
