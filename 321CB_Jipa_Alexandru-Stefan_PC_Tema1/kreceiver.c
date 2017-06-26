#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001
#define SIZE_3 3 //(sizeof(MINI_KERMIT.CHECK) + sizeof(MINI_KERMIT.MARK))
#define REPEAT while(1)

int main(int argc, char** argv) {
    msg r, t;
    init(HOST, PORT);
    int nr_seq = 0;

    memset(&r, 0, sizeof (r));
    do {//pt pachet send-init (S)
        if (recv_message(&r) < 0) {
            perror("Receive message");
            return -1;
        }
        unsigned short check; //crc-ul calculat in sender
        memcpy(&check, r.payload + r.payload[1] - 1, 2); //2 = sizeof check
        memset(r.payload + r.payload[1] - 1, 0, SIZE_3);//FAC MARK SI CHECK = 0
        unsigned short crc = crc16_ccitt(r.payload, sizeof (r.payload));
        if (crc == check) {//ACK
            sprintf(t.payload, "ACK");
            t.len = strlen(t.payload);
            send_message(&t);
            break;
        } else {//NAK
            sprintf(t.payload, "NAK");
            t.len = strlen(t.payload);
            send_message(&t);
        }
    } REPEAT;

    do {//ASTEPT PACHET DE TIP F
        if (recv_message(&r) < 0) {
            perror("Receive message");
            return -1;
        }
        if (r.payload[2] == 'B') {
            break;
        }
        unsigned short check;//CHECK-ul vechi din sender
        memcpy(&check, r.payload + r.payload[1] - 1, sizeof (check));
        memset(r.payload + r.payload[1] - 1, 0, SIZE_3);//FAC MARK SI CHECK= 0
        unsigned short crc = crc16_ccitt(r.payload, sizeof (r.payload));
        if (crc == check) {//ACK si apoi scriu in fisier
            unsigned char name_len = r.payload[1];
            char name_file[name_len];//numele fisierului primit
            memset(name_file, 0, name_len);
            memcpy(name_file, r.payload + 4, name_len - 5);
            char out_name[name_len];
            memset(out_name, 0, name_len);
            memcpy(out_name, "recv_", 5 * sizeof (char));
            strcat(out_name, name_file);//adaug recv_ inainte numelui
            sprintf(t.payload, "ACK");
            t.len = strlen(t.payload);
            send_message(&t);
            int fd = open(out_name, O_WRONLY | O_CREAT, 0777);//desh fisier
            if (fd < 0) {//caz de eroare
                return 0;
            }
            do {//pt pachet D
                if (recv_message(&r) < 0) {//astept Date
                    perror("Receive message");
                    return -1;
                }
                if (r.payload[2] == 'Z' || r.payload[2] == 'B') {
                    break;
                }
                nr_seq++;
                unsigned char data_len = r.payload[1];
                unsigned short check;//CHECK-ul vechi din sender
                memcpy(&check, r.payload + data_len - 1, sizeof (check));
                memset(r.payload + data_len - 1, 0, SIZE_3);//MARK SI CHECK=0
                unsigned short crc = crc16_ccitt(r.payload, sizeof (r.payload));
                if (crc == check) {
                    write(fd, r.payload + 4, data_len - 5);//scriu in fisier
                    sprintf(t.payload, "ACK");//ACK
                    t.len = strlen(t.payload);
                    send_message(&t);
                    //break;
                } else {
                    sprintf(t.payload, "NAK");//NAK
                    t.len = strlen(t.payload);
                    send_message(&t);
                }

            } REPEAT;
            close(fd);
        } else {
            sprintf(t.payload, "NAK");//NAK de la F(HEADER)
            t.len = strlen(t.payload);
            send_message(&t);
        }

        if (r.payload[2] == 'B') {
            break;
        }
        do {//Primesc pachet de tip Z
            unsigned short check;//CHECK-ul vechi din sender
            memcpy(&check, r.payload + r.payload[1] - 1, sizeof (check));
            memset(r.payload + r.payload[1] - 1, 0, SIZE_3);//MARK SI CHECK=0
            unsigned short crc = crc16_ccitt(r.payload, sizeof (r.payload));
            if (crc == check) {//ACK
                sprintf(t.payload, "ACK");
                t.len = strlen(t.payload);
                send_message(&t);
                break;
            } else {//NAK
                sprintf(t.payload, "NAK");
                t.len = strlen(t.payload);
                send_message(&t);
            }
            if (recv_message(&r) < 0) {
                perror("Receive message");
                return -1;
            }
        } REPEAT;
    } while (r.payload[2] != 'B');

    do {//Pachet de tip B
        unsigned short check;//CHECK-ul vechi din sender
        memcpy(&check, r.payload + r.payload[1] - 1, sizeof (check));
        memset(r.payload + r.payload[1] - 1, 0, SIZE_3);//MARK SI CHECK = 0
        unsigned short crc = crc16_ccitt(r.payload, sizeof (r.payload));
        if (crc == check) {//ACK
            sprintf(t.payload, "ACK");
            t.len = strlen(t.payload);
            send_message(&t);
            break;
        } else {//NAK
            sprintf(t.payload, "NAK");
            t.len = strlen(t.payload);
            send_message(&t);
        }
        if (recv_message(&r) < 0) {
            perror("Receive message");
            return -1;
        }

    } REPEAT;

    return 0;
}