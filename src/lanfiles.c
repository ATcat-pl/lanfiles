#include "lanfiles.h"
#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <string.h>

#define MAX_MSG_LEN 1024*1024*10 //10MB
#define SERVER_PORT 45455
#define SERVER_IP "127.0.0.1"
#define MAX_CONNECTION 1

//crc implementation from https://lxp32.github.io/docs/a-simple-example-crc32-calculation/
uint32_t crc32_table[256];

void build_crc32_table(void) {
	for(uint32_t i=0;i<256;i++) {
		uint32_t ch=i;
		uint32_t crc=0;
		for(size_t j=0;j<8;j++) {
			uint32_t b=(ch^crc)&1;
			crc>>=1;
			if(b) crc=crc^0xEDB88320;
			ch>>=1;
		}
		crc32_table[i]=crc;
	}
}

uint32_t crc32(const char *s, size_t n) {
	uint32_t crc=0xFFFFFFFF;
	for(size_t i=0;i<n;i++){
		printf("%i\r",i);
		char ch=s[i];
		uint32_t t=(ch^crc)&0xFF;
		crc=(crc>>8)^crc32_table[t];
	}
	return ~crc;
}

int reciveFile(void) {
	/*struct sockaddr_in server = {
	  .sin_family = AF_INET,
	  .sin_port = htons( SERVER_PORT )
	  };
	  if( inet_pton( AF_INET, ip, & server.sin_addr ) <= 0 ) {
	  perror( "inet_pton() ERROR" );
	  exit( 1 );
	  }
	  const int socket_ = socket( AF_INET, SOCK_STREAM, 0 );
	  if( socket_ < 0 ) {
	  perror( "socket() ERROR" );
	  exit( 2 );
	  } 
	  socklen_t len = sizeof( server );
	  if( bind( socket_,( struct sockaddr * ) & server, len ) < 0 ) {
	  perror( "bind() ERROR" );
	  exit( 3 );
	  }
	  if( listen( socket_, MAX_CONNECTION ) < 0 ) {
	  perror( "listen() ERROR" );
	  exit( 4 );
	  }*/

	struct sockaddr_in server = {
		.sin_family = AF_INET,
		.sin_port = htons(45455)
	};

	if(inet_pton(AF_INET, ip, &server.sin_addr)<=0){
		printf("[31mERROR[0m");
		return -1;
	}
	const int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	printf("Connecting...\n");
	if(connect(clientSocket,(struct sockaddr *) & server, sizeof(server))){
		perror("ERROR: ");
		return -2;
	} else {
		printf("Connected\n");
	}

	/*struct sockaddr_in client = { };

	  const int clientSocket = accept( socket_,( struct sockaddr * ) & client, & len );
	  if( clientSocket < 0 )
	  {
	  perror( "accept() ERROR" );
	//continue;
	}*/

	//recive filename
	char filename[256] = { };
	recv(clientSocket, filename, sizeof(filename), 0);
	printf("Reciving file: \x1b[3m%s\x1b[23m\n",filename);

	//recive max block size
	uint32_t maxBlockSize = 0;
	recv(clientSocket, &maxBlockSize, 4, 0);
	printf("Max block size is: %dB\n",maxBlockSize);

	char *buffer = malloc(maxBlockSize);

	FILE *file = NULL;
	file = fopen(filename, "wb");
	if(file != NULL){
		while(1){
			//block number
			uint32_t blockNumber = 0;
			recv(clientSocket, &blockNumber, 4, 0);

			//block size
			uint64_t blockSize = 0;
			recv(clientSocket, &blockSize, 4, 0);
			if(blockSize==0){
				break;
			}
//			printf("Reciving block %i - %iB : ", blockNumber, blockSize);

			//data
			recv(clientSocket, buffer, blockSize, 0);

			//crc
			uint32_t crc = 0;
			recv(clientSocket, &crc, 4, 0);

			char recived_status = 0;
			uint32_t crcComputed = crc32(buffer,blockSize);
			if(crc==crcComputed){
				printf("Reciving block %i - %iB : \x1b[32mOK\x1b[0m\n", blockNumber, blockSize);
				for(int i = 0; i<blockSize; i++){
					fwrite(&buffer[i],1,1,file);
				}
				recived_status = 'K';
			} else {
				printf("Reciving block %i - %iB : \x1b[31mCRC error\x1b[0m\n");
				recived_status='E';
			}
			send(clientSocket, &recived_status, 1, 0)<=0;
		}
	}
	free(buffer);
}

//function to read file in block and send these blocks
int sendFile(char *path){
	//set up tcp server
	
	struct sockaddr_in server = {
		.sin_family = AF_INET,
		.sin_port = htons( SERVER_PORT ),
		.sin_addr.s_addr = INADDR_ANY
	};

	/*if( inet_pton( AF_INET, SERVER_IP, & server.sin_addr ) <= 0 ) {
		perror( "inet_pton() ERROR" );
		exit( 1 );
	}*/
	const int socket_ = socket( AF_INET, SOCK_STREAM, 0 );
	if( socket_ < 0 ) {
		perror( "socket() ERROR" );
		exit( 2 );
	}
	socklen_t len = sizeof( server );
	setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	if( bind( socket_,( struct sockaddr * ) & server, len ) < 0 ) {
		perror( "bind() ERROR" );
		exit( 3 );
	}
	if( listen( socket_, MAX_CONNECTION ) < 0 ) {
		perror( "listen() ERROR" );
		exit( 4 );
	}
	//while(1) {
	printf("Waiting for connection.\n");

	struct sockaddr_in client = {};
	const int clientSocket = accept(socket_, (struct sockaddr *) & client, & len);
	if( clientSocket<0) {
		perror("accept() ERROR");
	}

	char filename[256] = { };

	char inBuffer[256] = {};

	FILE *file = NULL;
	unsigned char buffer[maxBlockSize];
	/*size_t*/uint32_t bytesRead = 0;

	file = fopen(path, "rb");
	//memcpy(filename, path, sizeof(path));
	strcpy(filename,basename(path));

	if(file != NULL) {
		//send filename
		send(clientSocket, filename, sizeof(filename), 0);

		//send maxBlockSize
		send(clientSocket, &maxBlockSize, sizeof(maxBlockSize), 0) <= 0;
		
		//block counter
		uint32_t blockNumber = 0;

		while((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0){

						//temporary
			/*printf("------BLOCK-INFO-------------------\n");
			printf("bytesRead=%i\n",bytesRead);
			printf("------BEGIN-BLOCK------------------\n");
			  for(int i=0;i<bytesRead;i++){
			  printf("%02x",buffer[i]);
			  }
			printf("\n------CRC-CHECKSUM-----------------\n");
			printf("%x\n",crc32(buffer,bytesRead));
			printf("------END-BLOCK--------------------\n\n");*/

//			printf("Sending block %i - %iB : ", blockNumber, bytesRead);
			//send block of data
			while(1){
				//block number
				send(clientSocket, &blockNumber, sizeof(bytesRead), 0)<=0;
				//block size
				send(clientSocket, &bytesRead, sizeof(bytesRead), 0) <= 0;
				//data itself
				send(clientSocket, buffer, bytesRead, 0) <= 0;
				//crc checksum
				uint32_t crc_sum_int =  crc32(buffer,bytesRead);
				send(clientSocket, &crc_sum_int, sizeof(crc_sum_int), 0)<=0;

				//wait for response
				char recived_msg = 0;
				recv(clientSocket, &recived_msg, 1, 0) <=0;
				if(recived_msg=='K'){//client says block recived correctly
					printf("Sending block %i - %iB : \x1b[32mOK\x1b[0m\n", blockNumber, bytesRead);
					break;
				} else {
					//client said something else, retransmit
					printf("Sending block %i - %iB : \x1b[40C\x1b[31mERROR\x1b[0m, retransmitting.\n", blockNumber, bytesRead);
				}
			}
			blockNumber++;

		}
		fclose(file);
	}

	shutdown(clientSocket, SHUT_RDWR);
	//}

	shutdown(socket_, SHUT_RDWR);
}

static int parse_options(int key, char *arg, struct argp_state *state) {
	switch(key) {
		case 'i':
			//ip = arg
			strcpy(ip, arg);
			printf("IP set to %s\nIP variable is %s\n",arg,ip);
			break;
			/*case 'u':
			//encypt = no
			action = action & ~ACTION_ENCRYPT;
			break;*/
		case 'f':
			//filePath = arg
			filepath = arg;
			break;
		case 's':
			//action send
			action = action | ACTION_SEND;
			break;
		case 'r':
			//action recive
			action = action | ACTION_RECIVE;
			break;
		case 'b':
			//set block size
			maxBlockSize = atoi(arg);
	}
	return 0;
}

static void decode_options(int argc, char **argv) {
	//set default options
	action = 0;
	action = action | ACTION_ENCRYPT; //encrytion is on by default
					  //set up argp
	struct argp_option options[] = {
		{"send", 's', 0, 0, "Send files specified by file option"},
		{"tx", 0, 0, OPTION_ALIAS, ""},
		{"recive", 'r', 0, 0, "Recive files specified by ip"},
		{"rx", 0, 0, OPTION_ALIAS, ""},
		{"ip", 'i', "IP", 0, "Sets IP address for transmission"},
		{"file", 'f', "PATH", 0, "File ar folder to transfer"},
		/*{"unencrypted", 'u', 0, 0, "Disables transport encryption"},*/
		{"block-size", 'b', "SIZE", 0, "Max block size for sending"},
		{0 }
	};
	struct argp argp = { options, parse_options };
	argp_parse(&argp, argc, argv, 0, 0, 0);
}

int main(int argc, char **argv) {
	build_crc32_table();
	decode_options(argc, argv);
	if(ACTION_SEND & action){
		if(filepath == NULL){
			printf("\x1b[31mERROR\x1b[0m: File must be specified when sending.");
			return -1;
		} else {
			printf("Sending file\nFilePath = %s\nMaxBlockSize = %iB\n", filepath, maxBlockSize);
			sendFile(filepath);
		}
	} else if(ACTION_RECIVE & action) {
		printf("Reciving file\n");
		printf("from IP: %s \n",ip);
		return reciveFile();
	} else {
		printf("ERROR: Action not specified, or multiple actions specified.\n");
		return -1;
	}
	return 0;
}
