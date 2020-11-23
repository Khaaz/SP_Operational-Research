#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <time.h>
#include <unordered_set>

/**
* M:
*		   o0   o1   o2
* piece0 = M0 - M2 - M1
* piece1 = M1 - M0 - M2
* piece2 = M0 - M1 - M2
*
* P: matric de cout
* 
* M[0][1] = M2 aka: machine 2 pour l'operation 1 de la piece 0
*/
struct Instance {
	int pieces; // nombre de pieces
	int machines; // nombre de machines
	int** M; // machine pour chaque operation : M[piece][operation] = machine (le numero de chaque machine)
	int** P; // cout d'une operation par machine : P[piece][operation] = cout (le cout pour chaque machine)
	
	Instance(int pieces, int machines) : pieces(pieces), machines(machines) {
		M = new int*[pieces];
		P = new int*[pieces];
		
		for (int i = 0; i < pieces; i++) {
			M[i] = new int[machines];
			P[i] = new int[machines];
		}
	}
	~Instance() {
		for (int i = 0; i < pieces; ++i) {
			delete[] M[i];
			delete[] P[i];
		}
		delete[] M;
		delete[] P;
	}
};

struct Couple {
	int piece;
	int operation;
};

struct SonFather {
	int son;
	int father;
};

/*
* Vecteur de Bierwith
* Contient les numeros de pieces
* rencontre i du numero de piece = operation i
*/

struct Vector {
	int *V; // vecteur de Bierwith
	int size; // taille du vecteur
	int pieces; // pieces
	int machines; // machines

	// va etre reset a chaque copie
	int cost = 0; // cout du job shop
	int** startingTime; // startingTime[piece][machine] = heure de debut pour cette piece sur cette machine
	Couple** pere; // pere[piece][machine] = {piece, operation} parente (limitant en temps) de cette piece sur cette machine
	
	Vector(const Vector& v) : Vector(v.pieces, v.machines) {
		// memcpy(this->V, v.V, this->size * sizeof(int));
		for (int i = 0; i < size; ++i) {
			V[i] = v.V[i];
		}
	}
	Vector(int pieces, int machines) : pieces(pieces), machines(machines), size(pieces * machines), cost(0) {
		V = new int[size];
		
		startingTime = new int*[pieces];
		pere = new Couple * [pieces];
		
		for (int i = 0; i < pieces; ++i) {
			startingTime[i] = new int[machines];
			pere[i] = new Couple[machines];

			startingTime[i][0] = 0;
			pere[i][0] = { -1, -1 };
		}
	}
	~Vector() {
		delete[] V;

		for (int i = 0; i < pieces; ++i) {
			delete[] startingTime[i];
			delete[] pere[i];
		}
		delete[] startingTime;
		delete[] pere;
	}
};

/**
 * @brief 
 * 
 * @param path 
 * @return Instance& 
 */
Instance& readInstance(std::string path) {
	std::string line;

	Instance* instance;
	int pieces, machines;

	std::ifstream infile(path);
	std::getline(infile, line);

	std::istringstream iss(line);

	iss >> pieces >> machines;
	instance = new Instance(pieces, machines);

	std::cout << "READ INSTANCE = pieces: " << instance->pieces << " machines: " << instance->machines << std::endl;

	for (int i = 0; i < instance->pieces; ++i) {
		std::getline(infile, line);
		std::istringstream iss(line);

		for (int j = 0; j < instance->machines; ++j) {
			iss >> instance->M[i][j] >> instance->P[i][j];
		}
	}

	return *instance;
}

/**
 * @brief 
 * 
 * @param instance 
 * @return Vector& 
 */
Vector& generateBierwirth(Instance& instance) {
	Vector *vb = new Vector(instance.pieces, instance.machines);
	int max = instance.pieces;
	int count = 0;

	// a[i] < instance.machines && >= 0 ==> a[i] = X il reste X pieces du type b[i]
	int* a = new int[instance.pieces]; // le nombre de pieces restantes a ajouter dans mon vecteur
	int* b = new int[instance.pieces]; // le numero de piece correspondant

	int randomNbr;

	std::fill_n(a, instance.pieces, instance.machines);
	for (int i = 0; i < instance.pieces; ++i) {
		b[i] = i;
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

/**
 * @brief 
 * 
 * @param instance 
 * @param vector 
 * @return Couple 
 */
Couple evaluate(Instance& instance, Vector& vector) {
	int *nbOperation = new int[instance.pieces] { 0 }; // nbOperation[i] = l'operation courante pour la piece i

	int curPiece, prevPiece, curOp, prevOp, curST, prevST, newCurST, curMachine;
	Couple indexCritic = { 0, 0 };
	
	// mp[a][0] = la derniere piece qui a utilis� la machine a; 
	// mp[a][1] = la derniere operation faites sur la machine a; 
	int **mp = new int*[instance.machines];

	// init mp
	for(int i = 0; i < instance.machines; ++i){
		mp[i] = new int[2];
		mp[i][0] = -1;
		mp[i][1] = -1;
	}

	for (int i = 0; i < vector.size; ++i) { // pour chaque elem de bierwith
		curPiece = vector.V[i];

		curOp = nbOperation[curPiece]++; // a check ici au cas ou
		curMachine = instance.M[curPiece][curOp];

		if (curOp > 0) { // on traite le cas de l'arc horizontal

			prevST = vector.startingTime[curPiece][curOp - 1]; // st de l'op pere
			curST = vector.startingTime[curPiece][curOp]; // st de l'op courante
			newCurST = prevST + instance.P[curPiece][curOp - 1]; // st de op pere + duree op pere
			
			if (newCurST > curST) {
				vector.startingTime[curPiece][curOp] = newCurST;
				vector.pere[curPiece][curOp] = { curPiece, curOp - 1 };
				
				if(newCurST + instance.P[curPiece][curOp] > vector.cost) {
					vector.cost = newCurST + instance.P[curPiece][curOp];
					indexCritic = { curPiece, curOp };
				}
			}
		}

		if(mp[curMachine][0] != -1) { // on traite le cas de l'arc disjonctif (il y a deja eu une operation sur cette machine)
			prevPiece = mp[curMachine][0];
			prevOp = mp[curMachine][1];

			prevST = vector.startingTime[prevPiece][prevOp]; // st de l'op pere
			curST = vector.startingTime[curPiece][curOp]; // st de l'op courante
			newCurST = prevST + instance.P[prevPiece][prevOp]; // st du pere + duree de l'op
			
			// starting time du pere + la dur�e de son op > starting time de l'op courante
			if (newCurST > curST) {
				vector.startingTime[curPiece][curOp] = newCurST;
				vector.pere[curPiece][curOp] = { prevPiece, prevOp };

				//Si le starting time de notre piece + le cout de l'op� est sup au cout total
				if(newCurST + instance.P[curPiece][curOp] > vector.cost){
					vector.cost = newCurST + instance.P[curPiece][curOp];
					indexCritic = { curPiece, curOp };
				}
			}
		}

		mp[curMachine][0] = curPiece;
		mp[curMachine][1] = curOp;
	}

	return indexCritic;
}
/**
 * @brief 
 * 
 * @param vector 
 * @param piece 
 * @param op 
 * @return SonFather index du fils et du pere dans le vecteur de bierwith
 */
SonFather searchSonAndFather(Vector &vector, int piece, int op){
	int iFather = 0, iSon = 0, nbOccurrFather = 0, nbOccurrSon = 0;
	int i = 0;
	bool foundSon = false, foundFather = false;
	Couple father = vector.pere[piece][op];
	
	while (i < vector.size && !(foundFather && foundSon) ) {
		if(!foundFather && vector.V[i] == father.piece) {
			++nbOccurrFather;
			if(nbOccurrFather == father.operation) {
				iFather = i;
				foundFather = true;
			}
		}
		if(!foundSon && vector.V[i] == piece) {
			++nbOccurrSon;
			if(nbOccurrSon == op) {
				iSon = i;
				foundSon = true;
			}
		}
		++i;
	}
	return { iSon, iFather };
} 
/**
 * @brief 
 * 
 * @param instance 
 * @param vector 
 * @param nbMaxIter 
 */
void localSearch(Instance& instance, Vector& vector, int nbMaxIter) {
	int curIter = 0, tmp;
	Vector *newVector;
	Couple cur, best;
	SonFather sonFather;
	
	best = evaluate(instance, vector);

	// std::cout << "LocalSearch.INITIALCOST = " << vector.cost << std::endl;
	
	while(vector.pere[best.piece][best.operation].piece != -1 && curIter < nbMaxIter) { // Tant qu'on est pas au d�but (tout � gauche) ou au nb max d'iter
		newVector = new Vector(vector);
		sonFather = searchSonAndFather(vector, best.piece, best.operation);
		
		//Si c'est pas un arc horizontal => psk le swap fait rien si c'est la meme piece
		if (vector.V[sonFather.son] != vector.V[sonFather.father]) { 

			// On swap notre arc disjonctif
			tmp = vector.V[sonFather.son];
			newVector->V[sonFather.son] = newVector->V[sonFather.father];
			newVector->V[sonFather.father] = tmp;

			// On r��value avec notre nouveau vecteur
			cur = evaluate(instance, *newVector);

			//Si le nouveau vecteur � un meilleur cout
			if (newVector->cost < vector.cost) {

				//Le nouveau remplace l'ancien 
				// on delete pas vector
				vector = *newVector;
				best = cur;
			}
			// Sinon on passe a la pi�ce p�re 
			else {
				best = vector.pere[best.piece][best.operation];
				delete newVector; // on delete mais bon
			}
		} 
		// Sinon on passe a la pi�ce p�re 
		else {
			best = vector.pere[best.piece][best.operation];
			delete newVector; // on delete mais bon
		}
		++curIter;
	}
	// std::cout << "LocalSearch.NEWCOST = " << vector.cost << std::endl;
}

/**
 * @brief
 *
 * @param arr
 * @param len
 * @return long
 */
std::string hashFunction(int* arr, int len) {
	std::string str;
	for (int i = 0; i < len; ++i) {
		str.push_back(arr[i] + '0');
	}
	return str;
}

/**
 * @brief 
 * 
 * @param vector 
 * @return Vector 
 */
Vector* genOneNeighbour(Vector& vector, std::unordered_set<std::string> hashSet) {
	int tmp;
	std::string key;
	Vector *newV = new Vector(vector);

	// Gen�re les nombre randoms
	int rand1 = rand() % vector.size;
	int rand2 = rand() % vector.size;

	// swap
	tmp = newV->V[rand1];
	while (tmp == newV->V[rand2]) { // V[rand1] != V[rand2] (on ne swap pas la meme piece)
		rand2 = rand() % vector.size;
	}
	newV->V[rand1] = newV->V[rand2];
	newV->V[rand2] = tmp;

	key = hashFunction(newV->V, vector.size);

	//Tant qu'on as pas un voisin qui n'existe pas en reg�n�re un nouveau
	while (hashSet.find(key) != hashSet.end()) {
		// inverse le swap / reset newV
		newV->V[rand1] = vector.V[rand1];
		newV->V[rand2] = vector.V[rand2];

		// Regenere les nombre randoms
		rand1 = rand() % vector.size;
		rand2 = rand() % vector.size;

		// swap
		tmp = newV->V[rand1];
		while (tmp == newV->V[rand2]) {
			rand2 = rand() % vector.size;
		}
		newV->V[rand1] = newV->V[rand2];
		newV->V[rand2] = tmp;

		key = hashFunction(newV->V, vector.size);
	}

	// new unique vector created => add to the set
	hashSet.insert(key);
	
	return newV;
}

/**
 * @brief 
 * 
 * @param vector 
 * @param nbNeighbours 
 * @return Vector* 
 */
Vector** genNeighbours(Vector& vector, int nbNeighbours) {
	Vector** vectors = new Vector*[nbNeighbours]; 
	std::unordered_set<std::string> hashSet;

	for (int i = 0; i < nbNeighbours; ++i) {
		vectors[i] = genOneNeighbour(vector, hashSet);
	}
	return vectors;
}

/**
 * @brief 
 * 
 * @param instance 
 * @param nbNeighbours 
 * @param iter 
 * @return Vector& 
 */
Vector& grasp(Instance& instance, Vector &vector, int nbNeighbours, int iter, int localSearchIter) {
	//global best
	Vector* bestOfBestVector = nullptr;
	//local best
	Vector* bestVector = nullptr;
	//array of neighbours
	Vector** vectors = nullptr;
	
	localSearch(instance, vector, localSearchIter);

	bestOfBestVector = &vector;
	bestVector = &vector;

	for (int i = 0; i < iter; ++i) {
		vectors = genNeighbours(*bestVector, nbNeighbours);

		for (int i = 0; i < nbNeighbours; ++i) {
			localSearch(instance, *vectors[i], localSearchIter);
			if (vectors[i]->cost < bestVector->cost) {
				bestVector = vectors[i];
			}
		}

		if (bestVector->cost < bestOfBestVector->cost) {
			bestOfBestVector = bestVector;
		}
	}

	return *bestOfBestVector;
}

/**
 * @brief
 *
 * @param instance
 * @param nbNeighbours
 * @param iter
 * @return Vector&
 */
Vector& graspWOLocalSearch(Instance& instance, Vector& vector, int nbNeighbours, int iter, int localSearchIter) {
	//global best
	Vector* bestOfBestVector = nullptr;
	//local best
	Vector* bestVector = nullptr;
	//array of neighbours
	Vector** vectors = nullptr;

	localSearch(instance, vector, localSearchIter);

	bestOfBestVector = &vector;
	bestVector = &vector;

	for (int i = 0; i < iter; ++i) {
		vectors = genNeighbours(*bestVector, nbNeighbours);

		for (int i = 0; i < nbNeighbours; ++i) {
			evaluate(instance, *vectors[i]);
			if (vectors[i]->cost < bestVector->cost) {
				bestVector = vectors[i];
			}
		}

		if (bestVector->cost < bestOfBestVector->cost) {
			bestOfBestVector = bestVector;
		}
	}

	return *bestOfBestVector;
}

void resultStudy(int nbSeq){

	int* evalCosts = new int[nbSeq];
	int* localSearchCosts = new int[nbSeq];
	int* graspCosts = new int[nbSeq];
	int* graspWOlsCosts = new int[nbSeq];

	Vector *vectorEvaluate = nullptr, *vectorLocalSearch = nullptr, *vectorGrasp = nullptr, *vectorGraspWOls = nullptr;
	Instance *instance = nullptr;
	std::string numInstance;

	for(int i = 1; i <= nbSeq; i++){
		//Car toutes les instances sont 0i avant 10.
		numInstance = ((i < 10) ? "0" : "") + std::to_string(i);
		instance = &(readInstance("LA" + numInstance + ".txt"));
		
		vectorEvaluate = &(generateBierwirth(*instance));
		vectorLocalSearch = new Vector(*vectorEvaluate);
		vectorGrasp = new Vector(*vectorEvaluate);
		vectorGraspWOls = new Vector(*vectorEvaluate);

		evaluate(*instance, *vectorEvaluate);
		evalCosts[i - 1] = vectorEvaluate->cost;

		localSearch(*instance, *vectorLocalSearch, 100);
		localSearchCosts[i - 1] = vectorLocalSearch->cost;

		vectorGrasp = &(grasp(*instance, *vectorGrasp, 10, 100, 100));
		graspCosts[i - 1] = vectorGrasp->cost;

		vectorGraspWOls = &(graspWOLocalSearch(*instance, *vectorGraspWOls, 10, 100, 100));
		graspWOlsCosts[i - 1] = vectorGraspWOls->cost;

	}	

	for(int i = 0; i < nbSeq; i++){
		std::cout << "LA0" << i + 1 << std::endl;
		std::cout << "Eval : " << evalCosts[i] << " || Local Search : " << localSearchCosts[i] << std::endl;
		std::cout << "GRASP w/o LS: " << graspWOlsCosts[i] << " || GRASP LS: " << graspCosts[i] << std::endl;
		std::cout << std::endl;
	}

}

int main()
{
	srand(1234);
	Instance& instance = readInstance("LA20.txt");
	Vector& vector = generateBierwirth(instance);

	//std::cout << "BIERWITH: " << std::endl;
	//for (int i = 0; i <	vector.size; ++i) {
	//	std::cout << vector.V[i];
	//}
	//std::cout << std::endl;

	// test evaluate
	// Couple indexCrit = evaluate(instance, vector);
	// std::cout << "VECTOR COST: " << vector.cost << std::endl;

	// test localSearch
	// localSearch(instance, vector, 100);
	// std::cout << "LOCAL SEARCH COST: " << vector.cost << std::endl;
	
	// test grasp
	Vector& v = grasp(instance, vector, 15, 100, 1000);
	std::cout << "GRASP.FINALCOST = " << v.cost << std::endl;

	
	// Etude des resultats (LS / no LS / GRASP etc...)
	// resultStudy(10);
	
	// test hashFunction
	//std::string hash;
	//int arr[11] ={0,1,2,2,0,3,1,2,3,3,0};
	//hash = hashFunction(arr, 11);
	//std::cout << hash << std::endl;
}
