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
#include <netdb.h> 

#define BUFLEN 256
#define MAX(a,b) (((a)>(b))?(a):(b))  // intoarce maximul dintre 2 elemente
#define REPEAT while(1)

/* Creaza numele fisierului conform cerintei */
char *create_file () {
	int id = getpid();  // obtin pid-ul procesului
	char *name = (char*)malloc(16 * sizeof(char));
	char *pid = (char*)malloc(10 * sizeof(char));
	sprintf(pid, "%d", id);
	strcpy(name, "client-<");
	name = strcat(name, pid);
	name = strcat(name, ">.log");
    free(pid);
	return name;
}

void error(char *msg) {
    perror(msg);  // afisez mesaj de eroare
    exit(1);
}

int main (int argc, char *argv[]) {
    int sockfd, n;
    struct sockaddr_in serv_addr;
    if (argc < 2) {  // in cazul unui apel gresit
       error("Usage server_address server_port\n");
       return 0;
    }

    /* Obtine numele fisierului de log si il deschide*/
    char* filename = create_file();    // creez fisierul cu numele specificat
    FILE *log_file = fopen(filename, "w");  // scriu un fisier
    if (!log_file) {  // in caz de eroare
    	error("Fisierul de log nu a putut fi creat\n");
    	return 0;
    }
    char *save_command = (char*)malloc(100 * sizeof(char));
    char *command = (char*)malloc(100 * sizeof(char));
    char buf[BUFLEN];
    char *last_card = (char*)malloc(25 * sizeof(char));
    int logged = 0;
    char buffer[BUFLEN];
    char *aux_buff = (char*)malloc(30 * sizeof(char));
    int fdmax;
    //golim multimea de descriptori de citire (read_fds) si multimea tmp_fds 
    fd_set fds_read;
    fd_set fds_tmp;
    FD_ZERO(&fds_read);
    FD_ZERO(&fds_tmp);
    FD_SET(0, &fds_read);
    /*---------------------------TCP------------------------*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("-10 : Eroare la apel socket");
    }
    FD_SET(sockfd, &fds_read);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));  // portul
    inet_aton(argv[1], &serv_addr.sin_addr);  // argv[1] = adresa
    
    
    if (connect(sockfd,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        error("-10 : Eroare la apel connect");
    }
    /*---------------------FINAL--TCP-----------------------*/

    /*---------------------------UDP------------------------*/
    int udp_socket;
    struct sockaddr_in srv_addr;
    socklen_t size_addr;
    udp_socket = socket(PF_INET, SOCK_DGRAM, 0);  // socket de tip UDP
    if (udp_socket < 0) {
        error("-10 : Eroare la apel socket");
    }
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &srv_addr.sin_addr);
    memset(srv_addr.sin_zero, '\0', sizeof(srv_addr.sin_zero));
    size_addr = sizeof(srv_addr);
    FD_SET(udp_socket, &fds_read); //PT UDP
    /*---------------------FINAL--UDP-----------------------*/
    fdmax = MAX(sockfd, udp_socket);
    // fdmax este maxim dintre socketii UDP si TCP
    REPEAT {
    	memset(buffer, 0 , BUFLEN);
        fds_tmp = fds_read;
        if (select(fdmax + 1, &fds_tmp, NULL, NULL, NULL) == -1) {
            error("-10 : Eroare la apel select");
        }
        // citesc de la tastatura
        if (FD_ISSET(0, &fds_tmp)) {
            memset(command, 0, strlen(command));
            fgets(buffer, BUFLEN - 1, stdin);  // pun comanda in buffer
            memset(save_command, 0 , strlen(save_command));
        	strcpy(save_command, buffer);  // ii fac o copie
        	if(save_command != NULL) {
        		strcpy(buf, buffer);
        		strcpy(command, strtok(buf, " "));
        	}
        	fprintf(log_file, "%s", buffer);  // scriu comanda in fisier
        // Dc am comanda login pastrez ultimul card in last_card
            if(strstr(command, "login") != NULL) {
                sprintf(last_card, "%s", strtok(NULL, " "));
            }
/* Dc am comanda unlock concatenez ultimul card si il trimit prin UDP la server
Se trimite mesajul unlock numar_card*/
            if(strstr(command, "unlock") != NULL) {
                memset(buffer, 0, BUFLEN);
                strcpy(buffer, "unlock");
                strcat(buffer, " ");
                strcat(buffer, last_card);
                sendto(udp_socket, buffer, BUFLEN, 0,
                       (struct sockaddr *)&srv_addr, size_addr);
            } else if (strcmp(command, "login") == 0 && logged == 1) {
/* Daca dau login dintr-un terminal cu un client logat, nu mai trimit mesaj la
server si intorc codul de eroare -2*/
                printf("%s\n", "-2 : Sesiune deja deschisa");
                fprintf(log_file, "%s\n", "-2 : Sesiune deja deschisa");
                continue;
            } else if ((strstr(command, "logout") != NULL ||
                        strstr(command, "listsold") != NULL ||
                        strstr(command, "getmoney") != NULL ||
                        strstr(command, "putmoney") != NULL) && logged == 0) {
/* In caz ca dintr-un terminal cu niciun user conectat pt oricare din comenzile
logout, listsold, getmoney, putmoney intorc codul de eroare -1, fara a
transmite serverului comanda*/
                printf("%s\n", "-1 : Clientul nu este autentificat");
                fprintf(log_file,
                        "%s\n", "-1 : Clientul nu este autentificat");
                continue;
            }// trimit prin TCP comanda la server (mai putin unlock)
            n = send(sockfd, buffer, strlen(buffer), 0);
            if (n < 0) {
                error("-10 : Eroare la apel send");
            }
        }
// Cand primesc mesaj prin UDP
        if (FD_ISSET(udp_socket, &fds_tmp)) {
            recvfrom(udp_socket, buffer, BUFLEN, 0, NULL, NULL);
// afisez si scriu in fisier buffer-ul primit
            printf("%s", buffer);
            fprintf(log_file, "%s", buffer);
/* Dc primesc mesajul UNLOCK> Trimite parola secreta atunci trimit serverului
prin UDP mesajul nr_card parola_secreta, dar de la tastatura citesc doar
parola secreta. La fiecare comanda login retin card-ul tastat. Iar pe
ultimul in pun in buffer*/
            if (strstr(buffer, "UNLOCK> Trimite parola secreta")) {
                memset(buffer, 0 , BUFLEN);
                memset(aux_buff, 0 , strlen(aux_buff));
                fgets(buffer, BUFLEN - 1, stdin);
                strcpy(aux_buff, last_card);
                strcat(aux_buff, " ");
                strcat(aux_buff, buffer);
                sendto(udp_socket, aux_buff, strlen(aux_buff), 0,
                       (struct sockaddr *)&srv_addr, size_addr);
            }
        }
// Dc primesc mesaj pe socket-ul TCP
        if (FD_ISSET(sockfd, &fds_tmp)) {
            if ((n = recv(sockfd, buffer, sizeof(buffer), 0)) <= 0) {
                error("-10 : Eroare la apel recv");
            }  // Comanda quit (inchide clientul)
            if (strcmp(buffer, "quit\n") == 0) {
                free(last_card);
                free(aux_buff);
                free(save_command);
                free(command);
                free(filename);
                fclose(log_file);  // inchid fisierul in care scriu
                close(udp_socket);  // inchid scoket UDP
                close(sockfd);  // inchid socket TCP
                FD_CLR(0, &fds_read);
                return 0;
            } else if (strcmp(command, "login") == 0 && logged == 0) {
// Dc dau comanda login si nu sunt logat
            	if (strcmp(buffer, "ATM> -4 : Numar card inexistent\n") == 0
                    || strcmp(buffer, "ATM> -3 : Pin gresit\n") == 0) {
/* Verific daca mesajul primit este unul dinte codurile de eroare -3 sau -4
si dc da, atunci il afisez si scriu in fisier.*/
            		printf("%s", buffer);
            		fprintf(log_file, "%s", buffer);
/* La fel si in cazul in care este codul de eroare -5*/
            	} else if (strcmp(buffer, "ATM> -5 : Card blocat\n") == 0) {
            		printf("%s", buffer);
            		fprintf(log_file, "%s", buffer);
/* Verific daca este intors codul de eroare -2. Dc nu este inseamna ca
a reusit logarea, afisez si scriu in fisier mesajul primit.*/
            	} else if (strcmp(buffer,
                     "ATM> -2 : Sesiune deja deschisa\n") != 0) {
/* Daca datele introduse sunt corecte, clientul va figura ca logat flagul
logged se seteaza pe 1*/
            		printf("%s", buffer);
            		fprintf(log_file, "%s", buffer);
	            	logged = 1;
/* Altfel inseamna ca am client logat in alt terminal. Afisez si scriu in
fisier.*/
	            } else {
                    printf("%s\n", "ATM> -2 : Sesiune deja deschisa");
                    fprintf(log_file,
                        "%s\n", "ATM> -2 : Sesiune deja deschisa");
                }
            } else if (strcmp(command, "logout\n") == 0 && logged == 1) {
                /* Se reseteaza flagul logged */
                /* Si afisez mesajul pt delogare*/
                logged = 0;
                printf("%s", buffer);
                fprintf(log_file, "%s", buffer);
/* Pt restul comenzilor doar afisez ce contine buffer-ul primit de la server.*/
            } else if (strstr(command, "listsold") != NULL) {
                printf("%s", buffer);
                fprintf(log_file, "%s", buffer); 
            } else if (strstr(command, "putmoney") != NULL) {
                printf("%s", buffer);
                fprintf(log_file, "%s", buffer); 
            } else if (strstr(command, "getmoney") != NULL) {
                printf("%s", buffer);
                fprintf(log_file, "%s", buffer);
            } 
        }
    }
}
