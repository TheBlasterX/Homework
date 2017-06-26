#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>
#include <stdio.h>
#include "hlt.hpp"
#include "networking.hpp"
#include <string.h>

typedef struct nod {
    unsigned char direction;
    unsigned char ID;
    unsigned char strength;
    unsigned char production;
} NOD;

bool permission_1(hlt::GameMap &presentMap, const hlt::Location& loc, int direction, unsigned char myID);
int the_closest_point_on_map(hlt::GameMap &presentMap, const hlt::Location& loc, unsigned char myID);

/*Primeste inaltimea sau latimea mapei(size), si 2 coordonate ce pot fi de tip x asu y
afla daca distanta minima se obtine prin marginile mapei, sau direct prin mapa*/
bool get_shortest_distance(int size, int coord1, int coord2) {
    short distance = abs(coord1 - coord2);  //distanta intre x-csi sau y-greci
    if (distance > size / 2) return false;  //distanta de obtine prin harta
    return true; //distanta se obtine prin marginile hartii
}

//Alege directia dintre sus si jos folosind functia get_shortest_distance 
int get_up_or_down_direction(hlt::GameMap& map, const hlt::Location& l1, const hlt::Location& l2) {
    if (l1.y < l2.y) {
        if (get_shortest_distance(map.height, l1.y, l2.y)) {
            return 3;
        }
        return 1;
    }

    if (l1.y > l2.y) {
        if (get_shortest_distance(map.height, l1.y, l2.y)) {
            return 1;
        }
        return 3;
    }

    return 0;
}

//Alege directia dintre stanga si dreapta folosind functia get_shortest_distance 
int get_left_or_right_direction(hlt::GameMap& map, const hlt::Location& l1, const hlt::Location& l2) {

    if (l1.x < l2.x) {
        if (get_shortest_distance(map.width, l1.x, l2.x)) {
            return 2;
        }
        return 4;
    }

    if (l1.x > l2.x) {
        if (get_shortest_distance(map.width, l1.x, l2.x)) {
            return 4;
        }
        return 2;
    }

    return 0;
}

//Alege directia optima intre 2 puncte folosind functiile de mai sus
int get_final_direction(hlt::GameMap& map, const hlt::Location& l1, const hlt::Location& l2) {
    if (l1.x < l2.x || l1.x > l2.x) {
        return get_left_or_right_direction(map, l1, l2);
    }

    if (l1.x == l2.x) {
        return get_up_or_down_direction(map, l1, l2);
    }

    return 0;
}

//Verifica toti vecinii unui punct si intoarce directia catre cel mai bun vecin
int best_neighbour(hlt::GameMap & presentMap, const hlt::Location& loc, unsigned char myID) {

    unsigned short maxDistance = (std::max)(presentMap.width, presentMap.height);
    unsigned char direction = STILL;
    unsigned short val = 0, val2 = 0, maxVal = 0;
    unsigned short distance = 0;
    unsigned short prod1 = 0, prod2 = 0;
    //verifica toate cele 4 directii din jurul punctului curent pana gasesc un punct neutru
    for (int d : CARDINALS) {
        distance = 0;
        hlt::Location current = loc;
        hlt::Site nextSite = presentMap.getSite(loc, d);
        //parcurge pana gaseste un vecin in directia d
        while (nextSite.owner == myID && distance < maxDistance) {
            ++distance;
            current = presentMap.getLocation(current, d);
            nextSite = presentMap.getSite(current);
        }
        /*daca gaseste un punct neutru cu productie > 0 ii atribuie o valoare
        calculata cu urmatoarele formule*/
        if (nextSite.owner != myID && nextSite.production > 2) {
            val = ((255 - nextSite.strength)) / 5 - ((distance + 1));
            val2 = (nextSite.production) * 5;
            val = std::max(val, val2);
        }
        if (val >= maxVal) {
            if (val == maxVal) {//daca val gasita == val max prod2 are productia din patratica gasita
                prod2 = nextSite.production;
                if (prod2 > prod1) {//alege patratia cu productia cea mai mare
                    direction = d; //alege noua directie
                    maxVal = val; //actualizeaza val max
                }
            } else {
                prod1 = nextSite.production;
                direction = d; // alege noua directie
                maxVal = val; // actualizeaza val max
            }
        }

    }
    return direction;
}

// calculeaza suma puterilor pe linie (intr-o anumita directie) pana la un punct neutru
int sum_until_neighbour(hlt::GameMap &presentMap, hlt::Site &nextSite,
        const hlt::Location& loc, unsigned char myID, int direction, int &distance) {
    hlt::Location current = loc; //locul curent
    current = presentMap.getLocation(current, direction);
    hlt::Site nextSite2 = presentMap.getSite(current);
    unsigned short maxDistance = (std::min)(presentMap.width, presentMap.height) / 2;
    int sum = presentMap.getSite(loc).strength;
    while (nextSite2.owner == myID && distance < maxDistance) {
        distance++;
        sum += nextSite.strength;
        current = presentMap.getLocation(current, direction);
        nextSite2 = presentMap.getSite(current);
    }
    nextSite = nextSite2; //actualizezaza pozitia punctului de atacat
    return sum;
}

// Obtine directia catre cel mai apropiat punct neutru
int the_closest_border(hlt::GameMap &presentMap, const hlt::Location& loc, unsigned char myID) {
    int maxDistance = (std::min)(presentMap.width, presentMap.height) / 2;
    int dist = maxDistance; //valoarea maxima
    int distance = 0;
    int direction = 0;
    int val1 = 0, val2;
    int prod1 = 0, prod2 = 0;
    for (int d : CARDINALS) {
        distance = 0;
        hlt::Location current = loc;
        hlt::Site nextSite = presentMap.getSite(current, d);
        while (nextSite.owner == myID && distance < maxDistance) {
            ++distance;
            current = presentMap.getLocation(current, d);
            nextSite = presentMap.getSite(current);
        }
        if (nextSite.owner != myID && dist >= distance && nextSite.production > 1) {
            // daca distantele sunt egale, verifica in functie de valoarea punctului calculata cu formula de mai jos            
            if (dist == distance) {
                val2 = nextSite.production * 5 - (nextSite.strength * 7 / 10) + 50;
                if (val2 >= val1) {
                    if (val1 == val2) {
                        prod2 = nextSite.production;
                        // daca valorile sunt egale, verifica in functie de productia punctului                       
                        if (prod2 > prod1) {
                            dist = distance;
                            direction = d;
                        }
                    } else {
                        prod1 = nextSite.production;
                        dist = distance;
                        direction = d;
                    }
                }

            } else {
                dist = distance;
                direction = d;
                val1 = nextSite.production * 5 - (nextSite.strength * 7 / 10) + 50;
            }
        }
    }
    // daca nu se gaseste niciun punct neutru atunci alege cel mai apropiat punct de pe harta
    if (direction == 0) {
        direction = the_closest_point_on_map(presentMap, loc, myID);

    }

    return direction;
}

// parcurge toata harta si gaseste cel mai apropiat punct neutru de pe toata harta
int the_closest_point_on_map(hlt::GameMap &presentMap, const hlt::Location& loc, unsigned char myID) {
    float minDistance = 1000; //initializarea distantei minime care va scadea cand gaseste un puncts
    hlt::Location current2;
    for (unsigned short a = 0; a < presentMap.height; a++) {
        for (unsigned short b = 0; b < presentMap.width; b++) {
            if (presentMap.getSite({b, a}).owner != myID) {
                //variabila get_next_lociliara care se compara cu minimul global
                float minD = presentMap.getDistance({b, a}, loc);
                if (minDistance > minD && presentMap.getSite({b, a}).production > 0) {
                    minDistance = minD;
                    current2 = {b, a};
                }
            }
        }
    }
    // apelul de mai jos returneaza directia catre punctul gasit
    return get_final_direction(presentMap, loc, current2);
}

/*verifica pe diagonala sa nu se ciocneasca patratele cu strength >= 150, adica, ca exemplu
daca una are ca directie dreapta, iar cealalta sus, iar locatia viitoare ar fi aceeasi pt ambele,
face ca sa se evite*/
bool permission_1(hlt::GameMap &presentMap, const hlt::Location& loc, int direction, unsigned char myID) {

    hlt::Location loc1 = presentMap.getLocation(loc, direction);
    hlt::Location current1;
    hlt::Location current2;
    hlt::Location current3;
    hlt::Location current4;
    hlt::Location current33;
    hlt::Location current44;

    current1 = presentMap.getLocation(loc, 1); //directie sus  
    current1 = presentMap.getLocation(current1, 2); //directie dreapta

    current2 = presentMap.getLocation(current1, 3); //directie jos
    current2 = presentMap.getLocation(current2, 3); //directie jos

    current3 = presentMap.getLocation(current2, 4); // directie stanga
    current3 = presentMap.getLocation(current3, 4); // directie stanga

    // verifica coltul de coord (X - 1, y + 1) (+y deoarece y creste in jos) , in current33 va fi patratelul in care se va deplasa
    if (presentMap.getSite(current3).owner == myID && presentMap.getSite(current3).strength >= 150) {
        int direction3 = the_closest_border(presentMap, current3, myID);
        current33 = presentMap.getLocation(current3, direction3);
    }

    current4 = presentMap.getLocation(current3, 1);
    current4 = presentMap.getLocation(current4, 1);

    // verifica coltul de coord (X + 1, y - 1) (+y deoarece y creste in jos) , in current44 va fi patratelul in care se va deplasa
    if (presentMap.getSite(current4).owner == myID && presentMap.getSite(current4).strength >= 150) {
        int direction4 = the_closest_border(presentMap, current4, myID);
        current44 = presentMap.getLocation(current4, direction4);
    }

    // verifica daca pozitia patratelului curent dupa o deplasare va fi egala cu cea a patratelului de pe pozitia current33 sau current4
    return !((current33.x == loc1.x && current33.y == loc1.y) || (current44.x == loc1.x && current44.y == loc1.y));
}

// apeleaza 1 functie care face sa nu se ciocneasca 2 patratele mari care sunt ale noastre
bool do_not_eat_yourself(hlt::GameMap &presentMap, const hlt::Location& loc, int direction, unsigned char myID) {
    return permission_1(presentMap, loc, direction, myID);
}

// functia intoarce directia catre cel mai favorabil loc de atacat
int overkill(hlt::GameMap &presentMap, const hlt::Location& loc, unsigned char myID) {
    int direction = 0;
    int sum_strength = 0;
    int max_sum_strength = 0;
    for (int d : CARDINALS) { // parcurg toti vecinii
        sum_strength = 0; // reinitializez sum_strength pt fiecare directie
        hlt::Site pos_to_atack3 = presentMap.getSite(loc, d);
        // verifica daca exista un element neutru cu strength > 40 nu mai atac
        if (pos_to_atack3.owner == 0 && pos_to_atack3.strength > 40) {
            continue;
        }
        hlt::Site pos_to_atack = presentMap.getSite(presentMap.getLocation(loc, d), d);
        // Verific daca este inamic
        if (pos_to_atack.owner != 0 && pos_to_atack.owner != myID) {
            sum_strength += pos_to_atack.strength; // adun strength-ul patratului inamic
            hlt::Location newloc = presentMap.getLocation(loc, d);
            for (int d2 : CARDINALS) { // parcurg toti vecinii patratelului de atacat
                hlt::Site pos_to_atack2 = presentMap.getSite(newloc, d2);
                // Verific daca si acest patrat este inamic
                if (pos_to_atack2.owner != 0 && pos_to_atack2.owner != myID) {
                    sum_strength += pos_to_atack2.strength; // adun strength-ul patratului inamic
                }
                if (sum_strength >= max_sum_strength) {
                    max_sum_strength = sum_strength; // actualizez suma maxima
                    direction = d; // si directia
                }
            }
        }
    }
    return direction; // directia de intors  
}

// primeste un loc si o directie si intoarce urmatorul loc in functie de directie
void get_next_loc(int &x, int &y, int d) {
    switch (d) {
        case 1:
            x = x - 1;
            y = y;
            break;
        case 2:
            x = x;
            y = y + 1;
            break;
        case 3:
            x = x + 1;
            y = y;
            break;
        case 4:
            x = x;
            y = y - 1;
            break;
    }
}

int main() {
    srand(time(NULL));

    std::cout.sync_with_stdio(0);
    unsigned char myID;
    hlt::GameMap presentMap;
    getInit(myID, presentMap);
    sendInit("S.A.M.");
    nod MAP[presentMap.height][presentMap.width];
    memset(MAP, 0, sizeof (MAP));
    std::set<hlt::Move> moves;
    while (true) {
        moves.clear();
        getFrame(presentMap);
        // creeam o matrice cu toate proprietatile unui punct (+directia unui punct)
        for (unsigned short a = 0; a < presentMap.height; a++) {
            for (unsigned short b = 0; b < presentMap.width; b++) {
                unsigned short maxDistance = (std::min)(presentMap.width, presentMap.height) / 2;
                unsigned char direction = STILL;
                if (presentMap.getSite({b, a}).owner == myID) { //verifica daca este patratelul nostru
                    int x;
                    MAP[a][b].ID = presentMap.getSite({b, a}).owner;
                    MAP[a][b].strength = presentMap.getSite({b, a}).strength;
                    MAP[a][b].production = presentMap.getSite({b, a}).production;
                    if (presentMap.getSite({b, a}).strength < presentMap.getSite({b, a}).production * 6) {
                        MAP[a][b].direction = 0;
                        continue;
                    }
                    // verific daca este un vecin langa mine, si daca este atunci il atac
                    if ((x = overkill(presentMap,{b, a}, myID)) != 0) {
                        MAP[a][b].direction = (unsigned char) (x);
                    }//strength-ul sa fie >=  4 (pt a se putea deplasa) si strength-ul >= 3 * productie pt a fi favorabila deplasarea 
                    else if (presentMap.getSite({b, a}).strength >= presentMap.getSite({b, a}).production * 3) {
                        if (presentMap.getSite({b, a}).strength < 150) { // daca patratelul inca are strength relativ mic
                            //directia = cel mai bun vecin
                            direction = best_neighbour(presentMap,{b, a}, myID);
                            hlt::Location current = {b, a};
                            current = presentMap.getLocation(current, direction);
                            hlt::Site nextSite = presentMap.getSite(current);
                            hlt::Site nextSite2 = presentMap.getSite(current);
                            //daca urmatoarea patratica in care s-ar deplasa ar fi tot a noastra, facem suma catre directia data (linie/coloana)
                            if (nextSite.owner == myID) {
                                int distance = 0;
                                //calculez suma si schimba distanta
                                int sum = sum_until_neighbour(presentMap, nextSite2,{b, a}, myID, direction, distance);
                                //daca suma > patratelul de atacat si distanta nu e maxima si suma < 300 si sa nu se manance patratele mari intre ele
                                if (sum > nextSite2.strength && distance != maxDistance && sum < 300
                                        && do_not_eat_yourself(presentMap,{b, a}, direction, myID)) {
                                    MAP[a][b].direction = (unsigned char) (direction);
                                } else {
                                    if (distance == maxDistance) {
                                        //daca distanta e maxima, directia e data de cea mai apropiata frontiera
                                        direction = the_closest_border(presentMap,{b, a}, myID);
                                        if (do_not_eat_yourself(presentMap,{b, a}, direction, myID)) {
                                            MAP[a][b].direction = (unsigned char) (direction);
                                        } else {
                                            MAP[a][b].direction = 0;
                                        }

                                    } else {
                                        MAP[a][b].direction = 0;
                                    }
                                }
                                //daca strength-ul patratelei curente >= ca strength-ul patratelei de atacat, atunci ataca
                            } else if (presentMap.getSite({b, a}).strength >= nextSite.strength && nextSite.owner != myID) {
                                MAP[a][b].direction = (unsigned char) (direction);
                            } else
                                MAP[a][b].direction = 0;
                        } else {//pt patratele cu strength >= 150
                            //directia este data de cea mai apropiata frontiera, din cauza ca vrem sa iasa cat mai repede din zona noastra

                            //verifica daca se mananca, daca nu face mutarea            
                            direction = the_closest_border(presentMap,{b, a}, myID);
                            if (do_not_eat_yourself(presentMap,{b, a}, direction, myID)) {
                                MAP[a][b].direction = (unsigned char) (direction);
                            } else {
                                MAP[a][b].direction = 0;
                            }
                        }
                    } else {
                        MAP[a][b].direction = (unsigned char) (0);
                    }
                } else {
                    MAP[a][b].ID = presentMap.getSite({b, a}).owner;
                    MAP[a][b].strength = presentMap.getSite({b, a}).strength;
                    MAP[a][b].production = presentMap.getSite({b, a}).production;
                    MAP[a][b].direction = -1;
                }
            }
        }
        
        //Se verifica inca o data daca patratelele mele nu se manannca intre ele
        for (unsigned short a = 1; a < presentMap.height - 1; a++) {
            for (unsigned short b = 1; b < presentMap.width - 1; b++) {
                if (MAP[a][b].ID == myID) {
                    int x = a, y = b;
                    get_next_loc(x, y, MAP[a][b].direction);
                    for (int d : CARDINALS) {
                        int x1 = x, y1 = y;
                        get_next_loc(x1, y1, d);
                        if (x1 != a && y1 != b && MAP[a][b].direction != 0) {
                            int x2 = x1, y2 = y1;
                            get_next_loc(x2, y2, MAP[x1][y1].direction);
                            if (MAP[x2][y2].ID == myID && x2 == x && y2 == y && MAP[x1][y1].strength + MAP[a][b].strength >= 280) {
                                if (MAP[a][b].direction == 0) {
                                    MAP[x1][y1].direction = 0;
                                } else {
                                    MAP[a][b].direction = 0;
                                }
                            }

                        }
                    }
                }
            }
        }

        // seteaza directia patratelelor
        for (unsigned short a = 0; a < presentMap.height; a++) {
            for (unsigned short b = 0; b < presentMap.width; b++) {
                if (presentMap.getSite({b, a}).owner == myID) {
                    moves.insert({{ b, a}, MAP[a][b].direction});
                }
            }
        }
        sendFrame(moves);
    }
    return 0;
}
