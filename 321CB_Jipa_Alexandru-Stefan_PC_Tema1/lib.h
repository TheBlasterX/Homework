#ifndef LIB
#define LIB

typedef struct {
    int len;
    char payload[1400];
} msg;

typedef struct {
	unsigned char SOH;
	unsigned char LEN;
	unsigned char SEQ;
	unsigned char TYPE;
	char DATA[250];
	unsigned short CHECK;
	unsigned char MARK;
}MINI_KERMIT;

typedef struct {
	unsigned char MAXL;
	unsigned char TIME;
	unsigned char NPAD;
	unsigned char PADC;
	unsigned char EOL;
	unsigned char QCTL;
	unsigned char QBIN;
	unsigned char CHKT;
	unsigned char REPT;
	unsigned char CAPA;
	unsigned char R;
}S;	//PACHET DE TIP S (SEND-INIT)

#define REPEAT while(1)

void init(char* remote, int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);
int send_message(const msg* m);
int recv_message(msg* r);
msg* receive_message_timeout(int timeout); //timeout in milliseconds
unsigned short crc16_ccitt(const void *buf, int len);

#endif