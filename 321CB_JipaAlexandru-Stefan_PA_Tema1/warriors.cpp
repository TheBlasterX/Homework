// "Copyright [April 10th 2017] >Copyright Stefan Jipa"
#include <cstdio>
#include <algorithm>
#include <vector>
#include <iostream>

/* Calcul combinari de n luate cate k, unde n este medium si k = 0 : lifes - 1
Ma duc direct pe linia n (medium) din triunghiul lui Pascal si incep sa
calculez toate combinarile pana cand suma lor este > nr. de nivele(level_range)
Intorc suma catre functia min_games
*/
double search(double level_range,
    double lifes, double medium) {
    unsigned long long comb = 1;  // var in care se actualizeaza combinarea
    double sum = 0;  // suma combinarilor pe linie
    double k = 0;  // de la 0 la lifes - 1
/* am pus conditia comb!=0 pt ca poate ajunge > maximul lui unsigned long
am facut cast pt level_range pt ca sum este de acest tip
am facut cast pt la impartire pt a obtine rezultat de tip intreg*/
    while (k < lifes && sum <= level_range) {
        comb = comb * (medium - k);
        comb /= (unsigned long long)(k + 1);  // Comb de medium luate cate k
        if (comb <= 0)
            return sum;
        sum += comb;
        k++;
    }
    return sum;  // intorc suma combinarilor pe linie
}

/* Imparte intervalul de nivele la 2 cat timp marginea inferioara - 1 este mai
mica decat marginea superioara. Daca rezultatul intors de search este mai mare
sau egal decat level_range atunci marginea superioara este medium, altfel
marginea inferioara este medium. In top se gaseste nr minim de meciuri.
*/
double min_games(double lifes,
    double level_range) {
    double bottom = 1;  // corespunde nivelului 1
    double top = level_range;  // corespunde nivelului N
    double medium = (unsigned long long)(bottom + top) / 2;  // mijlocul
    do {
        medium = bottom + (unsigned long long)(top - bottom) / 2;  // mijlocul
// am facut la intreg pt a nu face impartire de tip real
        if (search(level_range, lifes, medium) >= level_range) {
            top = medium;  // actualizare margine superioara
        } else {
            bottom = medium;  // actualizare margine inferioara
        }
    } while (bottom  + 1 < top);  // conditia de oprire
    return top;  // nr minim de meciuri
}

int main() {
    double level_range;  // de la 1 la n intervalul de nivele
    double lifes;  // nr de vieti
    FILE * fin = fopen("warriors.in", "r");
    FILE * fout = fopen("warriors.out", "w");

    fscanf(fin, "%lf%lf", &level_range, &lifes);  // citire din fisier

    fprintf(fout, "%lf", min_games(lifes, level_range));  // scrierea in fisier
    fclose(fin);  // inchiderea fisierului input
    fclose(fout);  // inchiderea fisierului input
    return 0;
}
