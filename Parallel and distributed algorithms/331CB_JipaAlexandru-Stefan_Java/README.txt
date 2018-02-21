// Jipa Alexandru-Stefan 331CB

	In implementare temei am folosit cunostine de POO.
	In cadrul temei am implementat toate functiile necesare pentru a functiona
in orice caz implementarea secventiala.
	Am creat clasa Database.java in care am scris operatiile de initDb, stopDb,
createTable, select, update, insert.
	Am creat clasa Table.java care reprezinta structura unei tabele cu
campurile necesare acesteia.
	Am verificat pe masina locala functia sanityCheck() iar aceasta are
output-ul dorit. Am creat si o functie de afisare a tuturor tabelelor pentru a
verifica corectitudinea programului. Functia printTables() se regaseste in
fisierul Database.java iar pentru testare am adaugat antetul acesteia in clasa
MyDatabase.java, clasa despre care am inteles ca se va suprascrie la testare.
	Am creat un fisier build.xml care compileaza fisierele sursa si creeza
fisierele .class si fisierul database.jar in directorul curent (cel din care
se da comanda).
	Mod utilizare:
	ant compile jar //compileaza fisierele si creeza fisierele .class +
fisierul database.jar
	java -jar database.jar // ruleaza tema
	ant clean // sterge fisierele .class si fisierul database.jar