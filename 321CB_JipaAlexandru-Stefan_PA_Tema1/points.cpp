// "Copyright [April 8th 2017] >Copyright Stefan Jipa"
#include <cstdio>
#include <algorithm>
#include <vector>
#include <iostream>

struct interval {  // structura de tip interval in care retin capetele
    unsigned int start;
    unsigned int end;

    interval(unsigned int start, unsigned int end) : start(start), end(end) {
    }
    // comparator pentru a sorta intervalele
    // intai sortez dupa capatul din stanga si apoi dupa dimensiune
    bool operator<(const interval & other) const {
        if (start == other.start)
            return (end - start + 1) > (other.end - other.start + 1);
        return start < other.start;
    }
};
  // functie auxiliara cu ajutorul careia verifica daca am un interval care
  // se regaseste in intregime in reuniunea altor intervale
bool repeated(std::vector<interval> used, int crt, int i, int j) {
    if (((used[crt].start >= used[i].start && used[crt].start < used[i].end) &&
        (used[crt].end > used[j].start && used[crt].end <= used[j].end)) ||
        ((used[crt].start >= used[j].start && used[crt].start < used[j].end) &&
        (used[crt].end > used[i].start && used[crt].end <= used[i].end))) {
            return true;
    }
    return false;
}


// primeste ca param punctele si intervalele si intoarce nr minim de intervale
int result(std::vector<unsigned int> points, std::vector<interval> interv) {
    int nr_intervs = 0;  // va retine nr de intervale folosite
    std::vector<interval> used;  // va retine toate intervalele folosite
    std::sort(interv.begin(), interv.end());  // sortare
    int ok = 0;  // verific daca exista puncte de interes in acel interval
    unsigned int j = 0;  // parcurg toate punctele cu j
    for (unsigned int i = 0; i < interv.size(); i++) {  // parcurg intervalele
        if (j == points.size() - 1) {  // conditie de oprire
            break;  // ma opresc cand am folosit toate numerele
        }
        ok = 0;  // resetez pt fiecare interval
// verific daca intervalul nou se extinde mai mult decat cel precedent
// elimina optiunea de a selecta un interval care este inclus total in altul
        if (interv[i].start <= points.at(j) || interv[i].end >= points.at(j)) {
            for (; j < points.size(); j++) {  // j <= nr total de nr
                if (points.at(j) > interv[i].end) {
                    break;  // cand ajung la un nr > decat capatul dreapta
                }  // incrementez ok pt fiecare nr din interval
                if (points.at(j) >= interv[i].start
                    && points.at(j) <= interv[i].end) {
                    ok++;
                }
            }
        }  // daca am gasit numere in interval
        if (ok != 0) {
            if (ok > 1) {
                j--;
            }
// avand sortarea dupa capatul stanga cresc pt primul punct putem avea mai
// intervale si il aleg pe cel care are capatul mai mare la dreapta
            if (i >= 1 && interv[i].end >= interv[i - 1].end
                && interv[i].start < points.at(0)) {
                used.erase(used.begin() + i - 1);
                nr_intervs--;
            }
            used.push_back(interv.at(i));  // folosesc intervalul
            nr_intervs++;  // incrementez nr de intervale
        }
    }
//  pt 2 intevale nu pot avea unul inclus in celalt, lucru tratat mai sus
    if (used.size() < 3) {
        return nr_intervs;
    }
//  daca rezultatul contine cel putin 3 intervale, caut sa le elimin pe cele
//  care se regasesc in reuniunea altora
    unsigned int i = 0;
    ok = 0;  // verific daca am sters un inerval
    do {
        i = 0;
        do {
            ok = 0;
            if (i >= 1 && i < (used.size() - 1)) {
                if (repeated(used, i, i - 1, i + 1)) {
                    nr_intervs--;  // scad nr de intervale
                    ok = 1;  // sterg intevalul
                    used.erase(used.begin() + i);
                }
            }
            if (i == 0 && repeated(used, i, i + 1, i + 1)) {
                nr_intervs--;  // scad nr de intervale
                ok = 1;  // sterg intevalul
                used.erase(used.begin() + i);
            }
            if (i == (used.size() - 1) && i >= 2
                && repeated(used, i, i - 1, i - 2)) {
                nr_intervs--;  // scad nr de intervale
                ok = 1;  // sterg intevalul
                used.erase(used.begin() + i);
            }  // contorul il reset doar cand nu steg niciun interval
            if (!ok) {  // cand sterg isi da resize
                i++;
            }
        } while (i <= used.size());
    } while (ok != 0);
// ma opresc cand la o parcurege completa nu mai am niciun interval de eliminat
    return nr_intervs;  // intorc numarul minim de intervale
}

int main() {
    unsigned int n;  // nr de puncte
    int m;  // nr de intervale
    std::vector<interval> interv;

    FILE * fin = fopen("points.in", "r");
    FILE * fout = fopen("points.out", "w");

    fscanf(fin, "%d%d", &n, & m);
    std::vector<unsigned int> points;  // retin punctele
    unsigned int pct;
    for (unsigned int i = 0; i < n; i++) {
        fscanf(fin, "%d", &pct);
        points.push_back(pct);
    }

    for (int i = 0; i < m; ++i) {
        unsigned int start;
        unsigned int end;
        // retin intervalele
        fscanf(fin, "%d%d", & start, & end);
        interv.push_back(interval(start, end));
    }
    fprintf(fout, "%d", result(points, interv));  // afisez nr de intervale
    fclose(fin);  // inchiderea fisierelor input/output
    fclose(fout);

    return 0;
}
