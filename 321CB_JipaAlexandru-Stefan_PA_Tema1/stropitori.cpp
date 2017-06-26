// "Copyright [April 9th 2017] >Copyright Stefan Jipa"
#include <cstdio>
#include <algorithm>
#include <vector>
#include <iostream>
#include <cstring>

/* Functia primeste 2 vectori cu coordonate si puteri ce sunt asociate
si lungimea stadionului si intoarce maxim de stropitori care pot uda gazonul.
   Intai verific daca pot uda la stanga si apoi in caz ca nu, la dreapta.
   Incep parcurgerea stropitorilor de la stanga.
*/
double max_stropitori(std::vector<double> coords,
                        std::vector<double> power, double len_size) {
/* Folosesc un vector de pozitii, care imi indica starea fiecarei stropitori.
   -1, la stanga
   0, blocata
   1, la dreapta
*/    
    std::vector<double> pos;
    double nr_pct = coords.size();  // nr de coordonate
    double nr_stropitori = 0;  // nr de stropitori care va fi returnat
/* Verific intai prima stropitoare separat, pt ca aceasta poate uda la stanga
pana la coordonata stanga a stadionului (marginea din stanga). Daca nu se poate
la stanga verific daca pot la dreapta (pana la stropitoarea urmatoare), altfel
este blocata.
*/
    if (coords.at(0) - power.at(0) >= 0) {
        nr_stropitori++;
        pos.push_back(-1);  // stanga
    } else if (coords.at(0) + power.at(0) < coords.at(1)) {
        nr_stropitori++;
        pos.push_back(1);  // dreapta
    } else {
        pos.push_back(0);  // blocat
    }
/* In for parcurg n-2 stropitori (fara prima si ultima). Pt fiecare stropitoare
am un set de reguli:
   Verific intai daca stropitoarea anterioara uda la dreapta. Daca da, am grija
ca din coord curenta sa scad puterea ei si sa fie > ca si coord trecuta +
puterea ei. Daca nu se poate la stanga, verifica la dreapta (pana la coord
urmatoare). Daca nici asa nu se poate atunci stropitoarea este blocata.
   Daca stropitoarea anterioara nu uda la dreapta atunci pot verifica daca
coord curenta - puterea ei > ca si coord trecuta. Celelalte 2 reguli sunt ca
mai sus.
*/
    for (double i = 1; i < nr_pct - 1; i++) {  // fara prima si ultima
        if (pos.at(i - 1) == 1) {  // daca anteriorul e la dreapta orientat
            if (coords.at(i) - power.at(i) >
                coords.at(i - 1) + power.at(i - 1)) {
                nr_stropitori++;
                pos.push_back(-1);  // stanga
            } else if (coords.at(i) + power.at(i) < coords.at(i + 1)) {
                nr_stropitori++;
                pos.push_back(1);  // dreapta
            } else {
                pos.push_back(0);  // blocat
            }
        } else {
            if (coords.at(i) - power.at(i) > coords.at(i - 1)) {
                nr_stropitori++;
                pos.push_back(-1);  // stanga
            } else if (coords.at(i) + power.at(i) < coords.at(i + 1)) {
                nr_stropitori++;
                pos.push_back(1);  // dreapta
            } else {
                pos.push_back(0);  // blocat
            }
        }
    }  // endfor
/* Punctul final il tratez diferit pt ca verific daca el incape pana la
marginea (inclusiv) din dreapta a stadionului. Aici, verific intai la dreapta
daca poate uda, in caz ca nu, verific la stanga. La stanga am 2 cazuri, la fel
ca mai sus (for). In caz ca nu se poate nici la stanga, atunci stropitoare este
este blocata.
*/
    if (coords.at(nr_pct - 1) + power.at(nr_pct - 1) <= len_size) {
        nr_stropitori++;
        pos.push_back(1);  // dreapta
    } else if (pos.at(nr_pct - 2) == 1) {
        if (coords.at(nr_pct - 1) - power.at(nr_pct - 1) >
                coords.at(nr_pct - 2) + power.at(nr_pct - 2)) {
            nr_stropitori++;
            pos.push_back(-1);  // stanga
        }
    } else if (coords.at(nr_pct - 1) - power.at(nr_pct - 1) >
            coords.at(nr_pct - 2)) {
        nr_stropitori++;
        pos.push_back(-1);  // stanga
    } else {
        pos.push_back(0);  // blocat
    }

    return nr_stropitori;  // nr maxim de stropitori care pot uda gazonul
}

int main() {
    double n;  // nr de stropitori
    double l;  // lungimea stadionului
    char *sir_control = (char*) malloc(sizeof (char) * 128);
    FILE * fin = fopen("stropitori.in", "r");
    FILE * fout = fopen("stropitori.out", "w");
    fgets(sir_control, 128, fin);  // citesc numele stadionului
// creez acest char* pt a folosi exact atat cat spatiu ocupa numele stadionului
    char *nume_stad = (char*) malloc(sizeof (char) * strlen(sir_control));
    strcpy(nume_stad, sir_control);
    free(sir_control);  // eliberez sirul vechi
    std::vector<double> coords;
    std::vector<double> power;
    fscanf(fin, "%lf%lf", &n, &l);
    double pct;
// citesc coordonatele
    for (double i = 0; i < n; i++) {
        fscanf(fin, "%lf", &pct);
        coords.push_back(pct);
    }
// citesc puterile
    for (double i = 0; i < n; i++) {
        fscanf(fin, "%lf", &pct);
        power.push_back(pct);
    }
// afisez nr maxim de stropitori care pot uda gazonul
    fprintf(fout, "%f", max_stropitori(coords, power, l));
    free(nume_stad);  // eliberez memoria ocupata de numele satdionului
    fclose(fin);
    fclose(fout);

    return 0;
}
