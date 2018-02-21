#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <omp.h>
#include <string.h>
#define TRUE 1
#define TWO 2

int NUM_THREADS;

/* functia verifica daca un punct al sarpelui are vecin/vecini
 daca intoarce 0 inseamna ca este sarpe de lungime 1
 daca intoarce 1 atunci punctul este coada
 daca intoarce un nr > 1 atunci nu este coada
 functia primeste deja din main pozitiile corecte ale vecinilor
 punctului dat(inclusiv ale punctelor aflate pe marginile hartii)*/
int neighbor(int north, int south, int west, int east, int snake) {
	int ok = 0;
	if (north == snake) { // daca am vecin la nord
		ok++;
	}
	if (south == snake) { // daca am vecin la sud
		ok++;
	}
	if (west == snake) { // daca am vecin la vest
		ok++;
	}
	if (east == snake) { // daca am vecin la est
		ok++;
	}
	return ok;
}

/* functia este apelata in caz de detectare de coliziune
ultima coada stearsa (pentru sarpele care a cauzat coliziunea) din matrice o refac
apoi pentru toti ceilalti serpi mutati la runda in care s-a detectat coliziunea
pun capul anterior si coada anterioara si sterg ultimul cap actualizat */ 
void last(int **world, struct snake *snakes, int **heads,
int **tails, int crt_s, int num_lines, int num_cols) {
	int i;
	int x = tails[crt_s + 1][0];
	int y = tails[crt_s + 1][1];
	world[x][y] = snakes[crt_s].encoding; // refac coada ultimului sarpe miscat
	#pragma omp parallel num_threads(NUM_THREADS)
	#pragma omp for private(i)
 // repozitionez capetele serpilor ce au fost mutati pana la coliziune
	for (i = 0; i < crt_s; i++) {
		if (snakes[i].direction == 'E') {
			if (snakes[i].head.col == 0) {
				snakes[i].head.col = num_cols - 1;
			} else {
				snakes[i].head.col -= 1;
			}
		} else if (snakes[i].direction == 'V') {
			if (snakes[i].head.col == num_cols - 1) {
				snakes[i].head.col = 0;
			} else {
				snakes[i].head.col += 1;
			}
		} else if (snakes[i].direction == 'N') {
			if (snakes[i].head.line == num_lines - 1) {
				snakes[i].head.line = 0;
			} else {
				snakes[i].head.line += 1;
			}
		} if (snakes[i].direction == 'S') {
			if (snakes[i].head.line == 0) {
				snakes[i].head.line = num_lines - 1;
			} else {
				snakes[i].head.line -= 1;
			}
		}
	}
	#pragma omp parallel num_threads(NUM_THREADS)
	#pragma omp for private(i)
	for (i = crt_s; i > 0; i--) {
// refac coada anterioara a serpilor mutati pana la coliziune
		world[tails[i][0]][tails[i][1]] = snakes[i - 1].encoding;
// sterg capetele serpilor miscati pana in momentul coliziunii
		world[heads[i][0]][heads[i][1]] = 0;
	}
}

/* returneaza pozitia noii cozi(punctul dat fiind vechea coada), coordonatele
punctului dat sunt x si y. Mai intai calculez coordonatele vecinilor punctului
dat si apoi caut coordonatele noi cozi (trimit prin referinta noul x sau y),
deoarece se poate modifica doar una dintre cele 2 coordonate, noua coada
fiind vecina cu cea veche. Daca intorc 0 atunci coada este si cap (sarpele
are lungime 1). Daca intorc 1 sarpele are lungime > 1.
*/
int get_new_tail(int snake, int *new_x, int *new_y,
int x, int y, int num_lines, int num_cols, int **world) {
	int north, south, west, east;
	int ok = 0;
	if (x == 0) {
		north = world[num_lines - 1][y];
	} else {
		north = world[x - 1][y];
	}
	if (x == num_lines - 1) {
		south = world[0][y];
	} else {
		south = world[x + 1][y];
	}
	if (y == 0) {
		west = world[x][num_cols - 1];
	} else {
		west = world[x][y - 1];
	}
	if (y == num_cols - 1) {
		east = world[x][0];
	} else {
		east = world[x][y + 1];
	}
	if (north == snake) {
		if (x == 0)
			*new_x = num_lines - 1;
		else *new_x = x - 1;	
		ok++;
	}
	if (south == snake) {
		if (x == num_lines)
			*new_x = 0;
		else *new_x = x + 1;
		ok++;
	}
	if (west == snake) {
		if (y == 0)
			*new_y = num_cols - 1;
		else *new_y = y - 1;
		ok++;
	}
	if (east == snake) {
		if (y == num_cols - 1)
			*new_y = 0;
		else *new_y = y + 1;
		ok++;
	}
	return ok;
}

// simuleaza mutarea serpilor
void run_simulation(int num_lines, int num_cols, int **world, int num_snakes,
	struct snake *snakes, int step_count, char *file_name) 
{
	int i, j, rounds, s_c, name, x, y, x_aux, y_aux;
	int last_x, last_y, ok, new_x, new_y, exit = 0;
	char dir; // directia
	NUM_THREADS = omp_get_num_threads(); // numar de thread-uri
// coordonatele cozilor serpilor
	int **tail = (int **)malloc((num_lines + 1)* sizeof(int *));
// coordonatele capurilor serpilor la pasul k + 1 (cel de coliziune) 
	int **last_heads = (int **)malloc((num_lines + 1)* sizeof(int *));
// coordonatele cozilor serpilor la pasul k + 1 (cel de coliziune)
	int **last_tail = (int **)malloc((num_lines + 1)* sizeof(int *));
	#pragma omp parallel num_threads(NUM_THREADS)
	#pragma omp for private(i)
	for (i = 0; i < num_lines + 1; i++) {
		tail[i] = (int *)calloc(TWO, sizeof(int));
		last_heads[i] = (int *)calloc(TWO, sizeof(int));
		last_tail[i] = (int *)calloc(TWO, sizeof(int));
	}

	// caut cozile fiecarui sarpe, doar ca am tratat separat marginile hartii
	for (int i = 1; i < num_lines - 1; i++) {
		for (j = 1; j < num_cols - 1; j++) {
			if (world[i][j] != 0) {
				ok = neighbor(world[i - 1][j], world[i + 1][j],
					world[i][j - 1], world[i][j + 1], world[i][j]);
				// verific sa nu fie capul(si el este punct terminal)
				if (ok == 0 || ((snakes[world[i][j] - 1].head.line != i
					|| snakes[world[i][j] - 1].head.col != j) && ok == 1)) {
					tail[world[i][j]][0] = i;
					tail[world[i][j]][1] = j;
				}
			} 
		}
	}
	// north-west corner
	if (world[0][0] != 0) {
		ok = neighbor(world[num_lines - 1][0], world[1][0],
			world[0][num_cols - 1], world[0][1], world[0][0]);
		if (ok == 0 || ((snakes[world[0][0] - 1].head.line != 0
			|| snakes[world[0][0] - 1].head.col != 0) && ok == 1)) {
			tail[world[0][0]][0] = 0;
			tail[world[0][0]][1] = 0;
		}
	}

	// north-east corner
	if (world[0][num_cols - 1] != 0) {
		ok = neighbor(world[num_lines - 1][num_cols - 1],world[1][num_cols - 1]
			, world[0][num_cols - 2], world[0][0], world[0][num_cols - 1]);
		if (ok == 0 || ((snakes[world[0][num_cols - 1] - 1].head.line != 0
			|| snakes[world[0][num_cols - 1] - 1].head.col != (num_cols - 1))
			&& ok == 1)) {
			tail[world[0][num_cols - 1]][0] = 0;
			tail[world[0][num_cols - 1]][1] = num_cols - 1;
		}
	}

	// south-west corner
	if (world[num_lines - 1][0] != 0) {
		ok = neighbor(world[num_lines - 2][0], world[0][0],
			world[num_lines - 1][num_cols - 1], world[num_lines - 1][1],
			world[num_lines - 1][0]);
		if (ok == 0 ||
			((snakes[world[num_lines - 1][0] - 1].head.line != (num_lines - 1)
			|| snakes[world[num_lines - 1][0] - 1].head.col != 0) && ok == 1)){
			tail[world[num_lines - 1][0]][0] = num_lines - 1;
			tail[world[num_lines - 1][0]][1] = 0;
		}
	}

	// south-east corner
	if (world[num_lines - 1][num_cols - 1] != 0) {
		ok = neighbor(world[num_lines - 2][num_cols - 1],world[0][num_cols - 1]
			, world[num_lines - 1][num_cols - 2], world[num_lines - 1][0],
			world[num_lines - 1][num_cols - 1]);
		if (ok == 0 ||
			((snakes[world[num_lines - 1][num_cols - 1] - 1].head.line
			!= (num_lines - 1) ||
			snakes[world[num_lines - 1][num_cols - 1] - 1].head.col
			!= (num_cols - 1)) && ok == 1)) {
			tail[world[num_lines - 1][num_cols - 1]][0] = num_lines - 1;
			tail[world[num_lines - 1][num_cols - 1]][1] = num_cols - 1;
		}
	}

//prima si ultima linie si prima si ultima coloana, mai putin colturile hartii 
	for (i = 1; i < num_lines - 1; i++) {
		if (world[0][i] != 0) {  
			ok = neighbor(world[num_lines - 1][i], world[1][i], world[0][i - 1]
				, world[0][i + 1], world[0][i]);
			if (ok == 0 || ((snakes[world[0][i] - 1].head.line != 0 ||
				snakes[world[0][i] - 1].head.col != i) && ok == 1)) {
				tail[world[0][i]][0] = 0;
				tail[world[0][i]][1] = i;
			}
		}
		if (world[num_lines - 1][i] != 0) {
			ok = neighbor(world[num_lines - 2][i], world[0][i],
			world[num_lines - 1][i - 1], world[num_lines - 1][i + 1],
			world[num_lines - 1][i]);
			if (ok == 0 || ((snakes[world[num_lines - 1][i] - 1].head.line
				!= num_lines - 1 || snakes[world[num_lines - 1][i] -1].head.col
				!= i) && ok == 1)) {
				tail[world[num_lines - 1][i]][0] = num_lines - 1;
				tail[world[num_lines - 1][i]][1] = i;
			}
		}
		if (world[i][0] != 0) {
			ok = neighbor(world[i - 1][0], world[i + 1][0],
				world[i][num_cols - 1], world[i][1], world[i][0]);
			if (ok == 0 || ((snakes[world[i][0] - 1].head.line != i ||
				snakes[world[i][0] - 1].head.col != 0) && ok == 1)) {
				tail[world[i][0]][0] = i;
				tail[world[i][0]][1] = 0;
			}
		}
		if (world[i][num_cols - 1] != 0) {
			ok = neighbor(world[i - 1][num_cols - 1], world[i + 1][num_cols -1]
				, world[i][num_cols - 2], world[i][0], world[i][num_cols - 1]);
			if (ok == 0 || ((snakes[world[i][num_cols - 1] - 1].head.line != 0
				|| snakes[world[i][num_cols - 1] - 1].head.col != num_cols - 1)
				&& ok == 1)) {
				tail[world[i][num_cols - 1]][0] = i;
				tail[world[i][num_cols - 1]][1] = num_cols - 1;
			}
		}
	}

	for (rounds = 0; rounds < step_count; rounds++) {  // numarul de runde
		for (s_c = 0; s_c < num_snakes; s_c++) {  // pentru fiecare sarpe
			name = snakes[s_c].encoding;
			x = snakes[s_c].head.line;
			y = snakes[s_c].head.col;
			dir = snakes[s_c].direction;
			x_aux = tail[snakes[s_c].encoding][0];
			y_aux = tail[snakes[s_c].encoding][1];
			if (world[x_aux][y_aux] != 0) {
				new_x = -1, new_y = -1; // caut noua coada
				ok = get_new_tail(world[x_aux][y_aux], &new_x, &new_y, x_aux,
					y_aux, num_lines, num_cols, world);
				if (ok == 0 || ((snakes[world[x_aux][y_aux] - 1].head.line
					!= x_aux || snakes[world[x_aux][y_aux] - 1].head.col
					!= y_aux) && ok == 1)) { // actualizare noua coada
					if (new_x != -1) tail[snakes[s_c].encoding][0] = new_x;
					if (new_y != -1) tail[snakes[s_c].encoding][1] = new_y;
				}
			}
	/* memorez pozitiile cozilor sterse in caz ca am coliziune
	si trebuie sa refac matricea*/
			last_tail[name][0] = x_aux;
			last_tail[name][1] = y_aux;
			world[x_aux][y_aux] = 0; // sterg coada veche
			if (dir == 'N') {  // muta nord
				if (x == 0) {	// traverseaza marginea de sus
					if (world[num_lines - 1][y]) { // detectie coliziune
						last(world, snakes, last_heads, last_tail, s_c,
							num_lines, num_cols);
						exit = 1;
						break;
						s_c = num_snakes;
					}  // actualizare cap si il pastrez in caz ca trebuie sters
					last_heads[name][0] = num_lines - 1;
					last_heads[name][1] = y;
					snakes[s_c].head.line = num_lines - 1; // jos de tot
					world[num_lines - 1][y] = name;
				} else {
					if (world[x - 1][y]) {  // detectie coliziune
						last(world, snakes, last_heads, last_tail, s_c,
							num_lines, num_cols);
						exit = 1;
						break;
							s_c = num_snakes;
						}
					// actualizare cap si il pastrez in caz ca trebuie sters
						last_heads[name][0] = x - 1;
						last_heads[name][1] = y;
						snakes[s_c].head.line -= 1;
						world[x - 1][y] = name;
				}
			}
			if (dir == 'S') {  // muta sud
				if (x == num_lines - 1) {  // traverseaza marginea de jos
					if (world[0][y]) {  // detectie coliziune
						last(world, snakes, last_heads, last_tail, s_c,
							num_lines, num_cols);
						exit = 1;
						break;
						s_c = num_snakes;
					}  // actualizare cap si il pastrez in caz ca trebuie sters
					last_heads[name][0] = 0;
					last_heads[name][1] = y;
					snakes[s_c].head.line = 0;
					world[0][y] = name;
				} else {
					if (world[x + 1][y]) {  // detectie coliziune
						last(world, snakes, last_heads, last_tail, s_c,
							num_lines, num_cols);
						exit = 1;
						break;
						s_c = num_snakes;
					}  // actualizare cap si il pastrez in caz ca trebuie sters
					last_heads[name][0] = x + 1;
					last_heads[name][1] = y;
					snakes[s_c].head.line += 1;
					world[x + 1][y] = name;
				}
			}
			if (dir == 'E') {  // muta est
				if (y == num_cols - 1) {  // traverseaza marginea din dreapta
					if (world[x][0]) {  // detectie coliziune
						last(world, snakes, last_heads, last_tail, s_c,
							num_lines, num_cols);
						exit = 1;
						break;
						s_c = num_snakes;
					}  // actualizare cap si il pastrez in caz ca trebuie sters
					last_heads[name][0] = x;
					last_heads[name][1] = 0;
					snakes[s_c].head.col = 0;
					world[x][0] = name;
					} else {
					if (world[x][y + 1]) {  // detectie coliziune
						last(world, snakes, last_heads, last_tail, s_c,
							num_lines, num_cols);
						exit = 1;
						break;
						s_c = num_snakes;
					}  // actualizare cap si il pastrez in caz ca trebuie sters
					last_heads[name][0] = x;
					last_heads[name][1] = y + 1;
					snakes[s_c].head.col += 1;
					world[x][y + 1] = name;
				}
			}
			if (dir == 'V') {  // muta vest
				if (y == 0) {  // traverseaza marginea din stanga
					if (world[x][num_cols - 1]) {  // detectie coliziune
						last(world, snakes, last_heads, last_tail, s_c,
							num_lines, num_cols);
						exit = 1;
						break;
						s_c = num_snakes;
					}  // actualizare cap si il pastrez in caz ca trebuie sters
					last_heads[name][0] = x;
					last_heads[name][1] = num_cols - 1;
					snakes[s_c].head.line = num_cols - 1;
					world[x][num_cols - 1] = name;
				} else {
					if (world[x][y - 1]) {  // detectie coliziune
						last(world, snakes, last_heads, last_tail, s_c,
							num_lines, num_cols);
						exit = 1;
						break;
						s_c = num_snakes;
					}  // actualizare cap si il pastrez in caz ca trebuie sters
					last_heads[name][0] = x;
					last_heads[name][1] = y - 1;
					snakes[s_c].head.col -= 1;
					world[x][y - 1] = name;
				}
			}				
		}
		// coliziune detectata(exit = 1), deci nu mai execut restul rundelor
		if (exit == 1) {
			break;
		}
	}
}
