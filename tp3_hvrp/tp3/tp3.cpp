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

struct t_tournee {
    int n; // nb client
    int list[CLIENT_TOURNEE]; // clients
    int c; // le camion (type de camion)
    int cout;
};

struct t_solution {
    int nbTournee; // nombre de tournee
	t_tournee liste[MAX]; //nombre de camion * types]; // ensemble de tournee
    int cout; // cout total
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



int main()
{
	Instance instance = readInstance("data.txt");
    std::cout << "clients: " << instance.numberClients << "; trucks " << instance.truckTypes << std::endl;
	std::cout << "d12: " << instance.distances[1][2] << "; capacities[3] " << instance.trucksCapacities[3] << "; quantityPerClient[10] " << instance.quantityPerClient[0] << std::endl;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
