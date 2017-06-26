#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_CLIENTS 100
#define BUFLEN 256
#define MAX(a,b) (((a)>(b))?(a):(b))  // Calculeaza maximul a 2 elemente
#define REPEAT while(1)
// structura de tip client cu toate campurile necesare
typedef struct{
	char nume[12];
	char prenume[12];
	int numar_card;
	unsigned short pin;
	char parola_secreta[16];
	double sold;
} client;

// delogarea clientului
void logout(char buffer[BUFLEN], int online[MAX_CLIENTS],
	  int sesiune_deschisa[MAX_CLIENTS], int descriptors[MAX_CLIENTS], int i) {
	memset(buffer, 0, BUFLEN);
	online[i] = 0;  // user delogat pe acest socket
	sesiune_deschisa[descriptors[i]] = 0;  // anunta delogarea user-ului
	descriptors[i] = -1;  // pe acest socket(i) nu mai este user logat
	sprintf(buffer, "%s", "ATM> Deconectare de la bancomat\n");
}

/* Primeste ca parametrii: cardul, baza de date si nr de clienti. Daca gaseste
cardul intoarce index-ul sau din baza de date, altfel intoarce -1.
*/
int check_nr_card(char *nr_card, client clienti[MAX_CLIENTS],
					short nr_clients) {
	int i;
	for (i = 0; i < nr_clients; i++) {
		if (clienti[i].numar_card == atoi(nr_card)) {
			return i;	// intorc pos din structura pt a stii numele userului
		}
	}
	return -1;  // card negasit
}

/* Primeste ca parametrii: pinul, baza de date si indexul cardului dat(care
este si valid) si intoarce index-ul pinului daca este corect, altfel -1.
*/
int check_pin(char *pin_nr, client clienti[MAX_CLIENTS], int i) {

	if (clienti[i].pin == atoi(pin_nr)) {
			return i;  // intorc si pos din structura pt a stii numele
	} 
	return -1;  // parola negasita
}

/* Elimina clientul client din vectorul de clienti */
void remove_client(int clienti[], int client, int nr_clienti) {
	int pos, it;
	for (it = 0; it < nr_clienti; it++) {
		if (clienti[it] == client) {
			pos = it;
			break;
		}
	}
	
	for (it = pos + 1; it < nr_clienti; it++){
		clienti[it - 1] = clienti[it];
	}
}

void error(char *msg) {
    perror(msg);  // afisare mesaj de eroare
    exit(1);
}

void database (client clienti[MAX_CLIENTS], FILE *fin, short nr_clients) {
	int i = 0;  // memoreaza baza de date
	for(i = 0; i < nr_clients; i++) {
		fscanf(fin, "%s", clienti[i].nume);
		fscanf(fin, "%s", clienti[i].prenume);
		fscanf(fin, "%d", &clienti[i].numar_card);
		fscanf(fin, "%hu", &clienti[i].pin);
		fscanf(fin, "%s", clienti[i].parola_secreta);
		fscanf(fin, "%lf", &clienti[i].sold);
	}
}

void send_tcp (int fd, char buffer[BUFLEN]) {
	int n = send(fd, buffer, strlen(buffer), 0);
	if (n < 0) {  // trimite mesaj de tip tcp catre client
		error("-10 : Eroare la apel send");
		exit(1);
	}
}

int main (int argc, char *argv[]) {
	int sockfd, newsockfd, portno;
	socklen_t clilen;
    char buffer[BUFLEN];  // buffer in care se primesc/dau comenzi
    char aux_buff[BUFLEN];
    struct sockaddr_in serv_addr, cli_addr;  // structura pt TCP
    int n, i, j = 0, last_id = -1;
    int nr_clienti = 0; // nr de clienti conectati la server
	int clienti[MAX_CLIENTS];  // memoreaza socketii folositi
	int sesiune_deschisa[MAX_CLIENTS];  // retine clientii logati(cu val 1)
	int carduri_blocate[MAX_CLIENTS];  // retine carduri blocate(cu val 1)
	int attempts[MAX_CLIENTS];  // retine nr de incercari pt 1 card
	int rez = 0;
	int online[MAX_CLIENTS];  // memorez socket-urile pe care sunt clienti on
	int descriptors[MAX_CLIENTS];  // legatura intre socket si user logat
	int pos = 0;
	int deblocare[MAX_CLIENTS];  // retine cardurile in curs de deblocare
    fd_set read_fds;	//multimea de citire folosita in select()
    fd_set tmp_fds;	//multime folosita temporar 
    int fdmax;		//valoare maxima file descriptor din multimea read_fds
	if (argc < 3) {  // in cazul unui apel gresit
		error("Inavlid arguments \n");
		return 1;
	}
	
	char *command = (char*)malloc(10 * sizeof(char));  // comanda primita
	char *card_nr = (char*)malloc(7 * sizeof(char));  // cardul introdus
	char *pass = (char*)malloc(16 * sizeof(char));  // parola introdusa
	char *pin_card = (char*)malloc(5 * sizeof(char));  // pinul introdus
	
	FILE *fin = fopen(argv[2], "r");  //pt citire din fisier
	short nr_clients;  // nr clienti din baza de date
	fscanf(fin, "%hu", &nr_clients);
	client data_clienti[nr_clients];  // structura cu toata baza de date
	database(data_clienti, fin, nr_clients);  // initializarea bazei de date
	fclose(fin);  // inchidere fisier
    //golim multimea de descriptori de citire (read_fds) si multimea tmp_fds
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
    /*---------------------------UDP------------------------*/
    int udp_sock = socket(PF_INET, SOCK_DGRAM, 0);  // udp socket
    if (udp_sock < 0) {
        error("-10 : Eroare la apel socket");
    }
    int enable_udp = 1;  // pt a refolosi socket-ul:portul nu va fi blocat
	if (setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &enable_udp,
		 sizeof(int)) < 0) {
    	error("-10 : Eroare la apel setsockopt");
    }
    struct sockaddr_in srv_addr;
    struct sockaddr_storage srv_store;
  	socklen_t size_addr;
  	srv_addr.sin_family = AF_INET;
  	srv_addr.sin_addr.s_addr = INADDR_ANY;
  	srv_addr.sin_port = htons(atoi(argv[1]));
  	memset(srv_addr.sin_zero, '\0', sizeof(srv_addr.sin_zero));
  	bind(udp_sock, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
  	size_addr = sizeof(srv_store);
    /*---------------------FINAL--UDP-----------------------*/

    /*---------------------------TCP------------------------*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // tcp socket
    if (sockfd < 0) {
        error("-10 : Eroare la apel socket");
    }
    int enable_tcp = 1;  // pt a refolosi socket-ul:portul nu va fi blocat
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable_tcp,
		sizeof(int)) < 0) {
    	error("-10 : Eroare la apel setsockopt");
    }
    portno = atoi(argv[1]);  // nr portului
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
    	sizeof(struct sockaddr)) < 0) { 
		error("-10 : Eroare la apel bind");
    }
    listen(sockfd, MAX_CLIENTS);
    /*---------------------FINAL--TCP-----------------------*/
    //adaugam noul file descriptor in multimea read_fds
    FD_SET(sockfd, &read_fds);  // PT TCP
    FD_SET(0, &read_fds);  // PT stdin
    FD_SET(udp_sock, &read_fds); //PT UDP
	/* La inceput toate sesiunile si incercarile de logare sunt setate pe 0*/
	for(i = 0; i < MAX_CLIENTS; i++){
		sesiune_deschisa[i] = 0;
		attempts[i] = 0;
		carduri_blocate[i] = 0;
		online[i] = 0;
		descriptors[i] = -1;
		deblocare[i] = -1;
	}
	// fdmax este maximul dintre socketii tcp si udp
	fdmax = MAX(sockfd, udp_sock);

    REPEAT {
		tmp_fds = read_fds;
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
			error("-10 : Eroare la apel select");
		}
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if(i == 0){  // citire de la tastatura
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN - 1, stdin);
					if(strcmp(buffer, "quit\n") == 0){  // inchidere server
						sprintf(buffer, "%s", "quit\n");
					// inchid toti socketii mai putin cei 2 creati la inceput
						for(j = 3; j <= fdmax; j++) {
							if (FD_ISSET(j, &read_fds) &&
								j != sockfd && j != udp_sock) {
								send_tcp(j, buffer);
								close(j);
								FD_CLR(j, &read_fds);  // scot sock din multime
							}
						}  // eliberez tot ce am alocat
						free(command);
						free(card_nr);
						free(pass);
						free(pin_card);
						FD_CLR(0, &read_fds);
						close(udp_sock);  // inchid socket UDP
						close(sockfd);  // inchid socket TCP
						return 0;		
					}    
				} else if (udp_sock == i){  // daca primesc pe UDP
					memset(buffer, 0, BUFLEN);  // primesc mesajul
					recvfrom(udp_sock, buffer, BUFLEN,
						0, (struct sockaddr *)&srv_store, &size_addr);
					// daca nu contine unlock primesc card si parola
					if (strstr(buffer, "unlock") == NULL) {
						memset(pass, 0, strlen(pass));
						memset(card_nr, 0, strlen(card_nr));
						strcpy(card_nr, strtok(buffer, " "));  // card-ul
						rez = check_nr_card(card_nr, data_clienti, nr_clients);
						sprintf(pass, "%s", strtok(NULL, " "));  // parola
						pass[strlen(pass) - 1] = '\0';  // elimin "\n"
						memset(buffer, 0, BUFLEN);  // verific parola introdusa
						if (strcmp(pass, data_clienti[deblocare[rez]].parola_secreta) != 0) {
							strcpy(buffer, "UNLOCK> -7 : Deblocare esuata\n");
						} else {
							carduri_blocate[deblocare[rez]] = 0;  // deblochez card-ul
							strcpy(buffer, "UNLOCK>  Client deblocat\n");
							deblocare[rez] = -1;  // il scot si din lista de asteptare de deblocare
						}  // trimit clientului mesajul aferent prin udp
						sendto(udp_sock, buffer, BUFLEN, 0, (struct sockaddr *)&srv_store, size_addr);	
					} else {  // primesc unlock card
						memset(command, 0, strlen(command));
						//* Se extrage comanda, folosind delimitatorul spatiu */
						strcpy(command, strtok(buffer, " "));  // comanda unlock
						memset(card_nr, 0, strlen(card_nr));  // card-ul
						sprintf(card_nr, "%s", strtok(NULL, " "));
						memset(buffer, 0, BUFLEN);
						rez = check_nr_card(card_nr, data_clienti, nr_clients);
						if(rez >= 0){  // verificare card existent
							if (carduri_blocate[rez] == 1) {  // pt card blocat
								strcpy(buffer, "UNLOCK> Trimite parola secreta\n");
								deblocare[rez] = rez;
							}
							else {  // pt card neblocat
								strcpy(buffer, "UNLOCK> -6 : Operatie esuata\n");
							}
						} else {  // card inexistent
							strcpy(buffer, "UNLOCK> -4 : Numar card inexistent\n");
						}  // trimit mesajul aferent prin UDP
						sendto(udp_sock, buffer, BUFLEN, 0, (struct sockaddr *)&srv_store, size_addr);
					}
					continue; // sar peste celelate else if-uri
				} else if (i == sockfd) {  // daca primesc pe TCP
					// a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
					// actiunea serverului: accept()
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error("-10 : Eroare la apel accept");
					} 
					else {
						//adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
					}
					/* Se adauga socketul nou  cu indicele aferent in vectorul de clienti 
						si se mareste numarul de clienti*/
					clienti[nr_clienti] = newsockfd;
					nr_clienti++;
				} else {
					memset(buffer, 0, BUFLEN);  // primesc mesaj prin TCP
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
						error("-10 : Eroare la apel recv");
					} else {
						memset(command, 0, strlen(command));
						//* Se extrage comanda, folosind delimitatorul spatiu */
						strcpy(command, strtok(buffer, " "));  // obtin comanda
						if (command != NULL && strstr(command, "login") != NULL) {
			/* Pt comanda login se extrag numele si parola trimisi ca parametru */
							memset(card_nr, 0, strlen(card_nr));
							memset(pin_card, 0, strlen(pin_card));
							sprintf(card_nr, "%s", strtok(NULL, " "));  // card
							sprintf(pin_card, "%s", strtok(NULL, " "));  // pin
							pin_card[strlen(pin_card) - 1] = '\0';  // scot "\n"
							if (card_nr != NULL && pin_card != NULL) {
								rez = check_nr_card(card_nr, data_clienti, nr_clients);
// daca se introduce alt card fata de cel dinainte, atunci se reseteaza nr de incercari
								if(rez != last_id && last_id != -1) {
									attempts[i] = 0;
								}
								last_id = rez;  // memorez ultimul card
								if (sesiune_deschisa[rez] == 1) {
									memset(buffer, 0, BUFLEN);  //  pt client deja logat
									strcpy(buffer, "ATM> -2 : Sesiune deja deschisa\n");
								} else if (rez >= 0) {  // pt card existent
									pos = rez;
									// verific pinul
									rez = check_pin(pin_card, data_clienti, rez);
									if (rez >= 0 && carduri_blocate[pos] == 0) {
										attempts[i] = 0;  // resetez nr de incercari
										online[i] = 1;
								  // fac corelatie intre socked is user logat curent
										descriptors[i] = rez;
										memset(buffer, 0, BUFLEN);
// Concatenez ATM> Welcome cu nume si cu prenume si "\n" si intorc acest buffer
										strcpy(buffer, "ATM> Welcome ");
										strcat(buffer, data_clienti[rez].nume);
										strcat(buffer, " ");
										strcat(buffer, data_clienti[rez].prenume);
										strcat(buffer, "\n");
										sesiune_deschisa[rez] = 1;  // aici se face logarea
									} else if (rez == -1 && carduri_blocate[pos] == 0){
										memset(buffer, 0, BUFLEN);  // pin gresit
										strcpy(buffer, "ATM> -3 : Pin gresit\n");
										attempts[i]++;  // incrementez nr de incercari
									} else {  // altfel am card blocat
										memset(buffer, 0, BUFLEN);
										strcpy(buffer, "ATM> -5 : Card blocat\n");
										attempts[i] = 0;  // resetez nr de incercari
									}
								} else {  // card inexistent
									memset(buffer, 0, BUFLEN);
									strcpy(buffer, "ATM> -4 : Numar card inexistent\n");
								}
							} // la 3 incercari consecutive pe acelasi card
							if (attempts[i] == 3) {
								memset(buffer, 0, BUFLEN);
								strcpy(buffer, "ATM> -5 : Card blocat\n");
								attempts[i] = 0;  // resetez nr de incercari
								carduri_blocate[pos] = 1;  // il blochez
							}
						} else if (command != NULL && strstr(command, "logout") != NULL) {
							memset(buffer, 0, BUFLEN);  // logout
							logout(buffer, online, sesiune_deschisa, descriptors, i);
						} else if (command != NULL && strstr(command, "listsold") != NULL) {
							memset(buffer, 0, BUFLEN);
							strcpy(buffer, "ATM> ");
							memset(aux_buff, 0, BUFLEN);
							sprintf(aux_buff, "%.2lf", data_clienti[descriptors[i]].sold);
							strcat(buffer, aux_buff);
							strcat(buffer, "\n");  // pun sold-ul in buffer
						} else if (command != NULL && strstr(command, "getmoney") != NULL) {
							memset(aux_buff, 0, BUFLEN);
							sprintf(aux_buff, "%s", strtok(NULL, " "));  // suma de retras
							aux_buff[strlen(aux_buff) - 1] = '\0';  // scot "\n"
							// Verific suma este multiplu de 10 
							if ((unsigned long long)atof(aux_buff) % 10 != 0) {
								memset(buffer, 0, BUFLEN);
								strcpy(buffer, "ATM> -9 Suma nu este multiplu de 10\n");
							} else if (atof(aux_buff) > data_clienti[descriptors[i]].sold) {
								memset(buffer, 0, BUFLEN);  // pt suma mai mare > sold
								strcpy(buffer, "ATM> -8 Fonduri insuficiente\n");
							} else {  // extragerea sumei
								data_clienti[descriptors[i]].sold -= atof(aux_buff);
								memset(buffer, 0, BUFLEN);
								strcpy(buffer, "ATM> Suma ");
								strcat(buffer, aux_buff);
								strcat(buffer, " retrasa cu succses\n");
							}
						} else if (command != NULL && strstr(command, "putmoney") != NULL) {
							memset(aux_buff, 0, BUFLEN);
							sprintf(aux_buff, "%s", strtok(NULL, " "));
							aux_buff[strlen(aux_buff) - 1] = '\0';
							data_clienti[descriptors[i]].sold += atof(aux_buff);
							memset(buffer, 0, BUFLEN);
							strcpy(buffer, "ATM> Suma depusa cu succses\n");
						} else if (command != NULL && strstr(command, "quit") != NULL) {
							remove_client(clienti, i, nr_clienti);  // rup conexiunea
							nr_clienti--;  // scad nr crt de clienti conectati
							memset(buffer, 0, BUFLEN);
							if (descriptors[i] >= 0) {  // logout pt useri online
								logout(buffer, online, sesiune_deschisa, descriptors, i);
							}
							memset(buffer, 0, BUFLEN);
							strcpy(buffer, "quit\n");  // transmit quit clientului
							send_tcp(i, buffer);  // dau send separat de aici pt ca
							close(i);  // inchid socket-ul i
							FD_CLR(i, &read_fds);  // si il scot din lista de socketi
							continue;
						}// trimite mesaj prin tcp(de la un dintre comenzi, mai putin quit)
						send_tcp(i, buffer);
					} 
				} 
			}
		}
    }
}
