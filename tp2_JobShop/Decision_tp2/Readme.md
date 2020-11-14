# TP2 - Recherche Operationnelle

Rapport - BELLEC Louison et BOUVARD Alexandre

<!-- omit in toc -->
## Table of Contents

- [TP2 - Recherche Operationnelle](#tp2---recherche-operationnelle)
  - [Descriptif](#descriptif)
    - [Evaluation du graphe](#evaluation-du-graphe)
    - [Generation d'un graphe](#generation-dun-graphe)
    - [Conception d'une recherche locale](#conception-dune-recherche-locale)
    - [Conception d'un GRASP](#conception-dun-grasp)
    - [Remarques sur l'optimisation du JS](#remarques-sur-loptimisation-du-js)
  - [Description algorithmique](#description-algorithmique)
    - [Evaluer](#evaluer)
    - [Recherche Locale](#recherche-locale)
    - [GRASP](#grasp)
    - [Hash Set](#hash-set)
  - [Etude](#etude)

## Descriptif

Le but du Job Shop est de trouver l'agencement optimisé des pièces et machines pour réduire les coûts.  
On commence par générer un vecteur de Bierwirth qui a pour propriété de donner un ordre des pièces sur les machines, assuré sans cycles.

### Evaluation du graphe

On calcule les dates de début d'un vecteur pour obtenir le coût final à partir d'un vecteur de Bierwirth. Cette fonction est importante car elle permet de facilement comparer deux vecteurs. On peut ainsi savoir lequel a le coût le plus faible.

### Generation d'un graphe

On génère le graphe dans la partie père ou l'on trace nos arcs disjonctifs et horizontaux. Cela nous permet de remonter facilement le chemin critique pour ensuite tenter de l'améliorer.  

### Conception d'une recherche locale

La recherche locale à pour but de remonter le chemin critique en inversant les arcs disjonctifs pour tenter d'optimiser la solution. Elle permet très souvent d'améliorer le cout initial du vecteur généré par l'évaluation.  
On inverse les arcs disjonctifs 2 à 2 et on vérifie si il y a une amélioration. On essaye ainsi sur tous les arcs afin d'obtenir la solution la plus optimisé a partir d'un veteur de base.

### Conception d'un GRASP

Le GRASP à pour but d'optimiser le JobShop en générant un grand nombre de solutions. En effet, elle génère plusieurs voisins de la solutions actuelle aléatoirement. Puis elle effectue la recherche locale sur chaque voisin afin d'avoir la meilleure solution possible pour ce voisin. Enfin, elle stockent le meilleur des vecteurs au final.  
Creer X voisins randomisé pour un vecteur permet de garder une diversité des solutions et donc d'améliorer plus le vecteur. La recherche locale seule n'est pas assez puissante car elle ne modifie que les arcs disjonctifs du chemin critique.  

### Remarques sur l'optimisation du JS

L'aléatoire joue une grande part dans la génération du vecteur de Bierwirth de base. Il est donc possible d'avoir une solution suboptimale des le début.
La recherche locale permet d'améliorer cette solution de base.
Le GRASP va génerer de nombreux voisins et donc réduire l'influence de l'aléatoire de la premiere solution.

Il est important de noter que de choisir une seed particulière et donc l'aléatoire de base pour la génération du vecteur ou la séquence des swaps sur le GRASP permet d'améliorer grandement (ou inversement) le résultat.

## Description algorithmique

Nous allons décrire en francais de manière très abstraite le fonctionnement des differents algorithmes utilisés.  
Les details d'implémentation peuvent être lu directement dans le code.  

### Evaluer

Evaluer à pour but de calculer le coût d'un vecteur de Bierwirth, de créer le chemin critique et de renvoyer la pièce et l'opération finale

```html
POUR chaque élement du vecteur de Bierwirth:

    SI ce n'est pas la première opération:
        Le pere est l'operation précedente de la pièce
        
        SI la date de fin du pere est supérieure à la date de début du fils:
		 	On décale la date de début du fils à celle de la fin du père
			On met le père en tant que père dans l'opération critique
			
            SI la date de fin du fils est plus élevée que le cout:
				Le coût devient la date de fin du fils
				On sauvegarde la nouvelle pièce et opération de fin
			FSI
            
		FSI
	FSI
    
    // il y a déjà eu une opération sur cette machine
	SI on est pas le premier sur la machine:
        Le père est la pièce et l'opération qui était précedement sur cette machine
		
        SI la date de fin du père est supérieure à la date de début du fils:
            On décale la date de début du fils à celle de la fin du père
			On met le père en tant que père dans l'opération critique
            
			SI la date de fin du fils est plus élevé que le coût:
				Le coût devient la date de fin du fils
				On sauvegarde la pièce et l'opération de fin
			FSI

		FSI
	FSI
    
    On sauvegarde que cette pièce et cette opération sont passés sur la machine
FPOUR
On renvoi la pièce et l'opération de fin
```

### Recherche Locale

Cette fonction evalue un vecteur et essaye ensuite de l'améliorer en effectuant des permutations (swap) sur le chemin critique.

```html
On évalue une première fois le vecteur
TANTQUE on est pas au premier élèment ou au nombre maximum d'iterations:
    On copie le vecteur de base dans un nouveau vecteur
    O   n recherche le pere et le fils pour la meilleure opération limitante
    
    // père est différent de fils donc c'est une pièce différente
    SI ce n'est pas un arc horizontal: 
        On fait une permutation (swap) de père et fils sur le nouveau vecteur

        On evalue le nouveau vecteur
        
        SI le coût du nouveau vecteur est supérieur au coût du vecteur de base:
            On garde le nouveau vecteur
            On garde l'opération et la pièce limitante
        SINON
            On avance au père du vecteur de base
        FSI
    SINON
        On avance au père du vecteur de base
    FSI
FTANTQUE
```

### GRASP

Le GRASP va évaluer un vecteur, générer plusieurs voisins, appliquer la recherche locale a chacun d'entre eux et garder le meilleur. Il va recommencer sur plusieurs itérations. Un vecteur est voisin d'un autre s'il sont les même à un swap près.

```html
On applique la recherche locale à notre vecteur de base
POUR le nombre d'itérations:
    On génère un nombre donné de voisins
    POUR chaque voisin:
        On effectue une recherche locale
        On stocke le meilleur de ces voisins (au plus faible coût)
    FPOUR
    SI le meilleur vecteur local est meilleur que le meilleur vecteur global:
        Le meilleur vecteur local devient le global
    FSI
FPOUR
On retourne le meilleur vecteur global (au plus faible coût)
```

### Hash Set

Lorsque on creer nos voisins, il faut être sur que on ne creer pas 2 fois le meme voisin. Pour ce faire, cette fonction genere un voisin qui n'existe pas deja en utilisant un Hash Set.

```
On prend deux pièces aléatoirement dans le vecteur de Bierwith
On s'assure qu'elles sont différentes
On échange ces deux pièces dans notre vecteur
On génère le hash a partir de ce nouveau vecteur

SI le hash n'est pas present dans le HashSet:
    On l'ajoute et on retourne le nouveau vecteur
SINON
    On recommence la procedure
FSI
```

Détails d'implementation de "génèrer hash":  
Cette fonction transforme un tableau d'entier en une chaine de caractere utilisee comme hash.

```
string genererHash(int[] tableau)
    POUR chaque élement du tableau:
		On concatène sa valeur dans un string
	FPOUR
	On retourne le string (qui est le hash unique du tableau)
F
```

## Etude

- evaluer vs localsearch
- grasp avec evaluer vs grasp avec localsearch
- opti grasp avec changement iter, changement voisins, changement seed
- plafond du nombre d'iter etc

- on augment les nombre d'iter du grasp mieux c'est. Mais ca plafonne vite (du coup pas ouf)
- varier le nombre de voisins (15 par exemple)
- varier la seed (la sequence aleatoire)
- On devrait faire plusieurs tours de grasp avec plusieurs seed differente et garder la sol la plus opti.