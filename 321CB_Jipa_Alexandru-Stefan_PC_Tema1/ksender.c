#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000
#define Zero 0
#define L_MAXL 250
#define size_SI 11 //Dimensiunea structurii Send-Init
#define L_EOL 0x0D
#define SIZE_4 4 //(sizeof(MINI_KERMIT.SOH) + sizeof(MINI_KERMIT.LEN) + sizeof(MINI_KERMIT.TYPE) + sizeof(MINI_KERMIT.SEQ))
#define LOSS_PACKAGE 3
#define TIMER 5
#define MAX_SEQ 63

//Numar de secventa, modulo 64
int seq_64(int nr_seq){
    if(nr_seq == MAX_SEQ){
        return 0;
    }
    else return nr_seq + 1;
}

int main(int argc, char** argv) {
    msg t;
    MINI_KERMIT Package;
    S SI; //send init
    init(HOST, PORT);
    int nr_secv = Zero;
    int file_not_found = Zero;
    int count = Zero;
    int nr = 1;
    //Initializari ce sunt comune mai multor pachete
    //Send-init initializare
    SI.TIME = TIMER; //5 SECUNDE, tre sa inmultesc cu 1000
    SI.MAXL = L_MAXL;
    SI.NPAD = Zero;
    SI.PADC = Zero;
    SI.EOL = L_EOL;
    SI.QCTL = Zero;
    SI.QBIN = Zero;
    SI.CHKT = Zero;
    SI.REPT = Zero;
    SI.CAPA = Zero;
    SI.R = Zero;
    Package.SOH = 0x01;
    Package.SEQ = nr_secv;
    Package.MARK = SI.EOL;

    while (count < 4) {//PACHET S
        if (count == LOSS_PACKAGE) {//se permite max de 3 ori pierderea pachetelor
            return 0;
        }
        //Incarcare campuri in structura MINI_KERMIT si apoi punerea ei in payload
        memset(&Package.DATA, 0, sizeof (Package.DATA));  //DATA ARE SIZEOF = STRUCTURA SEND-INIT (NOTATA S)    
        Package.LEN = (unsigned char) (sizeof(Package.SEQ) + sizeof(Package.TYPE) + size_SI + sizeof(Package.CHECK) + sizeof(Package.MARK));
        Package.TYPE = 'S'; //PACHET DE TIP SEND_INIT
        memcpy(&Package.DATA, &SI, sizeof (SI));
        memset(&t, 0, sizeof(t));
        t.payload[0] = Package.SOH;
        t.payload[1] = Package.LEN;
        t.payload[2] = Package.TYPE;
        t.payload[3] = Package.SEQ;
        memcpy(t.payload + SIZE_4, &Package.DATA, size_SI);
        Package.CHECK = crc16_ccitt(t.payload, sizeof (t.payload));//CALCUL CHECK
        memcpy(t.payload + SIZE_4 + size_SI, &Package.CHECK, sizeof (Package.CHECK));
        memcpy(t.payload + SIZE_4 + size_SI + sizeof(Package.CHECK), &Package.MARK, sizeof (Package.MARK));
        t.len = strlen(t.payload);
        send_message(&t);//trimite pachet S
        msg *y = receive_message_timeout(5000);
        if (y == NULL) {
            count++;//pierdere pachet
            perror("receive error");
        } else {
            nr_secv = seq_64(nr_secv);//Incrementare Secventa
            if ((*y).payload[0] != 'N') {//ACK
                Package.SEQ = nr_secv;
                break;
            }
        }
    }

    //pt mai multe fisiere
    for (int i = 1; i < argc; i++) {
        //Pachet F
        int exit_f = Zero; //ies din WHILE-u de la F cand termin cu Z
        count = Zero;
        while (count < 4) {
            if (count == LOSS_PACKAGE) {//se permite max de 3 ori pierderea pachetelor
                return 0;
            }//Incarcare campuri in structura MINI_KERMIT si apoi punerea ei in payload
            memset(&Package.DATA, 0, sizeof (Package.DATA));//DATA ARE SIZEOF lungime numelui fisierului curent (strelen(argv[i]))
            Package.LEN = (unsigned char) (sizeof (Package.SEQ) + sizeof (Package.TYPE) + strlen(argv[i]) + sizeof (Package.CHECK) + sizeof (Package.MARK)); //pt data aloc spatiu cat ocupa numele fisierului
            Package.TYPE = 'F'; //PACHET DE TIP F (File Header)
            memset(&t, 0, sizeof(t));
            t.payload[0] = Package.SOH;
            t.payload[1] = Package.LEN;
            t.payload[2] = Package.TYPE;
            t.payload[3] = Package.SEQ;
            memcpy(&Package.DATA, argv[i], strlen(argv[i]));
            memcpy(t.payload + SIZE_4, &Package.DATA, strlen(argv[i]));
            Package.CHECK = crc16_ccitt(t.payload, sizeof (t.payload));
            memcpy(t.payload + SIZE_4 + strlen(argv[i]), &Package.CHECK, sizeof (Package.CHECK));
            t.payload[SIZE_4 + strlen(argv[i]) + sizeof (Package.CHECK)] = Package.MARK;
            t.len = strlen(t.payload);
            send_message(&t);//trimitere pachet de tip F
            msg *y = receive_message_timeout(5000);
            if (y == NULL) {
                count++;//pierdere pachet
                perror("receive error");
            } else {
                nr_secv = seq_64(nr_secv);
                if ((*y).payload[0] != 'N') {//ACK
                    //In caz de ack pt pachetul F, incep sa trimit efectiv datele utile (pachet D)               
                    Package.SEQ = nr_secv;
                    int fd = open(argv[i], O_RDONLY, 0777);//deschidere fisier din care se citeste
                    if (fd < 0) {//in caz ca nu se gaseste fisierul din care se citeste
                        file_not_found = 1;
                        break;
                    } else { 
                        //pachet D
                        nr = 1;
                        while (nr > Zero) {//pt tot fisierul                    
                            Package.TYPE = 'D'; //PACHET DE TIP D
                            memset(&Package.DATA, 0, sizeof (Package.DATA));
                            count = Zero;
                            while ((nr = read(fd, Package.DATA, SI.MAXL)) > Zero) { //pt un pachet de date, SI.MAXL=250, dimensiunea maxima a bufferului
                                if (count == LOSS_PACKAGE) {//se permite max de 3 ori pierderea pachetelor
                                    return 0;
                                }//Incarcare campuri in structura MINI_KERMIT si apoi punerea ei in payload
                                memset(&t, 0, sizeof(t));
                                t.payload[0] = Package.SOH;//DATA ARE SIZEOF NR, adica cat s-a citit din fisier
                                Package.LEN = (unsigned char) (sizeof (Package.SEQ) + sizeof (Package.TYPE) + nr + sizeof (Package.CHECK) + sizeof (Package.MARK)); //nr = sizeof(data)
                                t.payload[1] = Package.LEN;
                                t.payload[2] = Package.TYPE;
                                t.payload[3] = Package.SEQ;
                                memcpy(t.payload + SIZE_4, &Package.DATA, nr);
                                Package.CHECK = crc16_ccitt(t.payload, sizeof (t.payload));//CALCULARE CHECK
                                memcpy(t.payload + SIZE_4 + nr, &Package.CHECK, sizeof (Package.CHECK));
                                t.payload[SIZE_4 + nr + sizeof (Package.CHECK)] = Package.MARK;
                                t.len = strlen(t.payload);
                                memset(&Package.DATA, 0, sizeof (Package.DATA));
                                send_message(&t);
                                msg *y = receive_message_timeout(5000);
                                if (y == NULL) {
                                    count++;//Pierdere pachet
                                    perror("receive error");
                                } else {
                                    nr_secv = seq_64(nr_secv);
                                    if ((*y).payload[0] != 'N') {//ACK
                                        Package.SEQ = nr_secv;
                                        break;
                                    }
                                }
                            }//end_of_read    
                        }
                        count = Zero;
                        while (count < 4) {
                            if (count == LOSS_PACKAGE) {
                                return 0;
                            }//Incarcare campurilor din structura MINI-KERMIT si punerea lor in payload
                            memset(&Package.DATA, 0, sizeof (Package.DATA));//data este vid acum
                            Package.LEN = (unsigned char) (sizeof (Package.SEQ) + sizeof (Package.TYPE) + sizeof (Package.CHECK) + sizeof (Package.MARK));
                            Package.TYPE = 'Z'; //PACHET DE TIP Z
                            memset(&t, 0, sizeof (t));
                            t.payload[0] = Package.SOH;
                            t.payload[1] = Package.LEN;
                            t.payload[2] = Package.TYPE;
                            t.payload[3] = Package.SEQ;
                            memset(&Package.DATA, 0, sizeof (Package.DATA));
                            memcpy(t.payload + SIZE_4, &Package.DATA, 0);
                            Package.CHECK = crc16_ccitt(t.payload, sizeof (t.payload));//CALCULARE CHECK
                            memcpy(t.payload + SIZE_4, &Package.CHECK, sizeof (Package.CHECK)); //data = vid
                            t.payload[SIZE_4 + sizeof (Package.CHECK)] = Package.MARK;
                            t.len = strlen(t.payload);
                            send_message(&t);
                            msg *y = receive_message_timeout(5000);
                            if (y == NULL) {
                                count++;//pierdere pachet
                                perror("receive error");
                            } else {
                                nr_secv = seq_64(nr_secv);
                                if ((*y).payload[0] != 'N') {//ACK
                                    Package.SEQ = nr_secv;
                                    exit_f = 1;
                                    break;
                                }
                            }
                        }
                        close(fd);//inchidere fisier
                        if (exit_f) {
                            break;
                        }
                    }   
                }
            }
        }
    }
    if(file_not_found == Zero) {
        //Pachet B
        count = 0;
        while (count < 4) {
            if (count == LOSS_PACKAGE) {//Se permit maxim 3 pierderi de pachet
                return 0;
            }//Incarcare campurilor din structura MINI-KERMIT si punerea lor in payload
            memset(&Package.DATA, 0, sizeof (Package.DATA));//data este vid
            Package.LEN = (unsigned char) (sizeof (Package.SEQ) + sizeof (Package.TYPE) + sizeof (Package.CHECK) + sizeof (Package.MARK)); //pt data nu aloc spatiu fiindca e vid
            Package.TYPE = 'B'; //PACHET DE TIP B
            memset(&t, 0, sizeof(t));
            t.payload[0] = Package.SOH;
            t.payload[1] = Package.LEN;
            t.payload[2] = Package.TYPE;
            t.payload[3] = Package.SEQ;
            memcpy(t.payload + SIZE_4, &Package.DATA, 0);
            Package.CHECK = crc16_ccitt(t.payload, sizeof (t.payload));//CALCULARE CHECK
            memcpy(t.payload + SIZE_4, &Package.CHECK, sizeof (Package.CHECK));
            t.payload[SIZE_4 + sizeof(Package.CHECK)] = Package.MARK;
            t.len = strlen(t.payload);
            send_message(&t);//trimitere pachet de tip B
            msg *y = receive_message_timeout(5000);
            if (y == NULL) {
                count++;//pierdere pachet
                perror("receive error");
            } else {
                nr_secv = seq_64(nr_secv);
                if ((*y).payload[0] != 'N') {//ACK
                    Package.SEQ = nr_secv;
                    break;
                }
            }

        }
    }
    return 0;
}