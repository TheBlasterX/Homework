// "Copyright [May 7th 2017] >Copyright Stefan Jipa"
#include <bits/stdc++.h>
#include <stdio.h>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <iostream>
#include <cstring>
#define pb push_back
#define len 100  // nr maxim de caractere al unui cuvant

using namespace std;

int V = 26;  // nr de caractere ale alfabetului englez
list<int> *graph;  // (graf sub forma de lista de adiacenta)

// adauga muchie in graf
void addMuchie(int node1, int node2) {
    graph[node1].pb(node2);
}

/*  Functia este apelata dupa ce graful este construit. Folosesc sortate
topologica, algoritmul lui Khan. Scriu in fisier Imposibil daca detectez ciclu.
Daca nu detectez ciclu atunci afisez alfabetul obtinut. Functia permutari imi
transforma literele in numere de la 0 la 25, iar eu la afisare adun 97 si apoi
il convertesc la char. Literele care nu sunt folosite sunt puse in ordinea
alfabetului englez standard.
*/
void sortare() {
    FILE* fout = fopen("permutari.out", "w");
    vector<int> vecini(V, 0);  // initializare nr de muchii pt fiecare nod cu 0
    vector <int> result;  // vector ce va contine rezultatul
    int nr = 0;  // contor cu care verific daca am ciclu
    list<int>::iterator iter;  // nr de muchii din fiecare nod
    for (int i = 0; i < V; i++) {
        for (iter = graph[i].begin(); iter != graph[i].end(); iter++) {
            vecini[*iter]++;
        }
    }
    // Coada in care se adauga toate elementele
    queue<int> que;
    for (int i = 0; i < V; i++) {
        if (vecini[i] == 0) {
            que.push(i);
        }
    }

    while (!que.empty()) {  // Cat timp coada nu e vida
        int elem = que.front();  // obtin primul element din coada
        que.pop();  // il scot din coada
        result.pb(elem);  // il pun in rezultat
        // trec prin toti vecinii nodului ales, scot cate o muchie
        // daca nr de muchii dintr-un nod ajunge la 0 atunci pun nodul in coada
        for (iter = graph[elem].begin(); iter != graph[elem].end(); iter++) {
            vecini[*iter]--;
            if (vecini[*iter] == 0) {
                que.push(*iter);
            }
        }
        nr++;  // incrementez nr de noduri vizitate
    }

    if (nr != V) {  // in caz ca detectez ciclu
        fprintf(fout, "Imposibil");
        fclose(fout);
        return;
    }

    // Afisez alfabetul
    for (int i = 0; i < (int)result.size(); i++) {
        fprintf(fout, "%c", (char)(result[i] + 97));
    }
    fclose(fout);
}

/*  Functia primeste ca parametru o matrice de cuvinte si nr de cuvinte.
Parcurg cate 2 linii mereu si daca gasesc o diferenta o pun ca o muchie in
graf. Iau valoarea ei ascii din care scad 97, pt a lucra mai usor cu graful.
Dupa ce termin de parcurs matricea de cuvinte apelez functia de sortare
topologica. Tot aici tratez cazul in care daca al 2-lea cuv este mai mic decat
primul si primul contine de la inceput al 2-lea cuvant, intorc imposibil
deoarece nu se poate realiza sortarea.
*/
void permutari(char** mat, int n) {
    FILE* fout = fopen("permutari.out", "w");
    graph = new list<int>[V];  // initializare graf
    char word[100];
    for (int i = 0; i < n - 1; i++) {
        memset(word, 0, len);
        strncpy(word, mat[i], strlen(mat[i + 1]));
        if (strlen(mat[i]) > strlen(mat[i + 1])
            && (strcmp(mat[i + 1], word) == 0)) {
            fprintf(fout, "Imposibil");
            fclose(fout);
            return;
        } else {
            for (int j = 0; j < (int)strlen(mat[i])
                && j < (int)strlen(mat[i + 1]) ; j++) {
                if ((int)mat[i][j] != (int)mat[i + 1][j]) {
                    addMuchie((int)mat[i][j] - 97, (int)mat[i + 1][j] - 97);
                }
            }
        }
    }
    fclose(fout);
    sortare();  // sortare topologica
}

int main() {
    int n;  // nr de cuvinte
    FILE* fin = fopen("permutari.in", "r");
    char word[100];

    fscanf(fin, "%d", &n);  // citesc nr de cuvinte
    // alocare matrice dinamica pt cuvinte
    char** mat = (char**)malloc(n * sizeof(char*));
    for (int i = 0; i < n; i++) {
        mat[i] = (char*)malloc(len);
    }
    fgets(word, len, fin);  // citesc restul randului ca sa ajung la cuvinte
    for (int i = 0; i < n; i++) {
        memset(word, 0, len);
        fscanf(fin, "%s", word);  // citesc fiecare cuvant
        strcpy(mat[i], word);  // adaug cuvant in matrice
    }

    permutari(mat, n);  // apelez functia de permutari
    fclose(fin);
    // eliberez memoria alocata
    for (int i = 0; i < n; i++) {
        free(mat[i]);
    }

    free(mat);

    return 0;
}
