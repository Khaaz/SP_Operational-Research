#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

const int TAILLE_MAX = 100;
const int TAILLE_MAX_N = 50;
const int TAILLE_MAX_M = 50;

struct Instance_t {
	int n; // nombre de jobs
	int m; // nombre de machines
	int M[TAILLE_MAX_M + 1][TAILLE_MAX_N + 1]; // Matrice M[job][i] = machine (le numero de chaque machine)
	int P[TAILLE_MAX_M + 1][TAILLE_MAX_N + 1]; // Matrice P[job][i] = cout (le cout pour chaque machine)
};

struct Couple_t {
	int a;
	int b;
};

/*
* Vecteur de Bierwith
* Contient les numeros de produit
* rencontre i du numero de produit = machine i
*/
struct Vecteur_t {
	int V[TAILLE_MAX_N * (TAILLE_MAX_M + 1)]; // vecteur de Bierwith
	int cout = 0;
	int startingTime[TAILLE_MAX][TAILLE_MAX]; // date de debuts
	Couple_t pere[TAILLE_MAX][TAILLE_MAX];
};

Vecteur_t& generateBierwith(Instance_t& instance) {
	Vecteur_t *vb = new Vecteur_t;
	int max = instance.n;
	int count = 0;

	int* a = new int[instance.n];
	int* b = new int[instance.n];

	int randomNbr;

	std::fill_n(a, instance.n, instance.m);
	for (int i = 0; i < instance.n; ++i) {
		b[i] = i + 1;
	}

	while (max > 0) {
		randomNbr = rand() % max;
		
		vb->V[count] = b[randomNbr];

		if (a[randomNbr] == 1) {
			a[randomNbr] = a[max - 1];
			b[randomNbr] = b[max - 1];
			--max;
		}
		else {
			--a[randomNbr];
		}
		++count;
	}
	return *vb;
}

Instance_t& readInstance(std::string path) {
	std::string line;

	Instance_t *instance = new Instance_t();

	std::ifstream infile(path);
	std::getline(infile, line);

	std::istringstream iss(line);

	iss >> instance->n >> instance->m;

	std::cout << "n: " << instance->n << " m: " << instance->m << std::endl;

	for (int i = 0; i < instance->n; ++i) {
		std::getline(infile, line);
		std::istringstream iss(line);

		std::cout << "LINE " << i << std::endl;

		for (int j = 0; j < instance->m; ++j) {
			iss >> instance->M[i][j] >> instance->P[i][j];
			++instance->M[i][j];
			
			std::cout << "NUMBER " << j << std::endl;
		}
	}

	return *instance;
}

void evaluate(Instance_t& instance, Vecteur_t& vector) {
	int cptTraitementPiece[TAILLE_MAX_N + 1] = { 0 }; // np

	int mp[TAILLE_MAX_N + 1] = { 0 }; // mp

	int st[TAILLE_MAX][TAILLE_MAX];
	int current;
	int departPrecedent;
	int finPrecendent;

	for (int i = 0; i < TAILLE_MAX; ++i) {
		std::fill_n(st[i], TAILLE_MAX, -100000);
	}

	for (int i = 1; i <= instance.n; ++i) {
		current = vector.V[i - 1];
		++cptTraitementPiece[current];
		if (cptTraitementPiece[current] > 1) {
			departPrecedent = st[current][cptTraitementPiece[current] - 1];
			finPrecendent = departPrecedent + instance.P[cptTraitementPiece[current] - 1];
		}
	}
}

void localSearch() {

}

int main()
{
	Instance_t& instance = readInstance("data_test.txt");
	std::cout << "n: " << instance.M[1][1] << " m: " << instance.P[1][1] << std::endl;
	Vecteur_t& VB = generateBierwith(instance);

	for (int i = 0; i <	instance.n * instance.m; ++i) {
		std::cout << VB.V[i];
	}
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
