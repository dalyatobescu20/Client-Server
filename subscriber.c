#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <netdb.h>
#include <math.h>
#include "utils.h"
#define MAX_CLIENTS 10 // number of clients in waiting

void send_message(int socket, char * buffer){
    int ret;
    int len = strlen(buffer) + 1;
    ret = send(socket, &len, sizeof(int), 0);
    if (ret < 0) {
        printf("Eroare trimitere lungime mesaj la client\n");
        exit(1);
    }

    ret = send(socket, buffer, len, 0);
    
    if (ret < 0) {
        printf("Eroare trimitere mesaj la client\n");
        exit(1);
    }
}

int recv_message(int sockfd, char* message){
	memset(message,0,BUFLEN);
	//Nr de octeti primit
	int received = 0;
	//Asteapta lungimea pachetului
	while(received < sizeof(int)){
		int ret = recv(sockfd, message + received, sizeof(int) - received,0);
        if (ret < 0) {
            printf("Eroare citire de la server\n");
            exit(1);
        }
        if(ret == 0)
			return 0;
		received += ret;
	}

	int size = 0;
	memcpy(&size,(int *)message,sizeof(int));
	memset(message,0,BUFLEN);
	received = 0;

	//Asteapta receive pana primeste pachetul intreg(de dimensiune size)
	while(received < size){
		int ret = recv(sockfd,message + received, size - received,0);
		received += ret;
	}

	return size;
}

int subscribe(int sockfd, char *topic, char *buffer) {
    char *token = strtok(NULL, " ");
    if (token == NULL) {
        printf("Invalid \n");
        return -1;
    }

    topic = strtok(NULL, " ");
    if (topic == NULL) {
        printf("Invalid topic\n");
        return -1;
    }

    int sf = atoi(topic);
    if (sf != 0 && sf != 1) {
        printf("Invalid sf\n");
        return -1;
    } 

    snprintf(buffer, BUFLEN, "%s %s %d", "subscribe", token, sf);
    //trimit mesajul
    send_message(sockfd, buffer);
    //afisez am dat subscribe la mesaj
    printf("Subscribed to topic.\n");

    return 0;
}


int unsubscribe (int sockfd, char *topic, char *buffer) {
    char *token = strtok(NULL, " ");
    if (token == NULL) {
        printf("Empty topic\n");
        return -1;
    }
  
    snprintf(buffer, BUFLEN, "%s %s", "unsubscribe", token);
    //trimit mesajul
    send_message(sockfd, buffer);
    //afisez am dat unsubscribe la mesaj
    printf("Unsubscribed from topic.\n");
   

    return 0;
}

int main(int argc, char *argv[]) {
          
        setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    
        if (argc != 4) {
            fprintf(stderr,"Usage: %s server_address server_port\n", argv[0]);
            exit(0);
        }
        
        int sockfd;
        // se converteste numarul portului din format string in format int
        int portno = atoi(argv[3]);
        if (portno < 1024) {
            exit(1);
        }

        int ret;
        char buffer[BUFLEN];
        struct sockaddr_in client_addr;

        //se deschide socketul
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("ERROR opening socket");
            exit(1);
        }

        // se completeaza informatiile despre adresa clientului
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(atoi(argv[3]));

        // se converteste adresa IP a clientului din format dotted-decimal in binar
        inet_aton(argv[2], &client_addr.sin_addr);
    
        // se conecteaza clientul la server
        ret = connect(sockfd, (struct sockaddr*) &client_addr, sizeof(client_addr));
        if (ret < 0) {
            perror("ERROR connecting");
            exit(1);
        }

        // se dezactiveaza algoritmul lui Neagle
        int ok = 1;
        ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (void *)&ok, sizeof(ok));

        if(ret < 0) {
            perror("ERROR setsockopt");
            exit(1);
        }

        struct pollfd pfds[32];
        int nfds = 0;

        pfds[nfds].fd = STDIN_FILENO;
        pfds[nfds].events = POLLIN;
        nfds++;

        pfds[nfds].fd = sockfd;
        pfds[nfds].events = POLLIN;
        nfds++;

        send_message(sockfd, argv[1]);

        while(1) {

            poll(pfds, nfds, -1);
            if(pfds[0].revents & POLLIN) {
                memset(buffer, 0, BUFLEN);
                fgets(buffer, BUFLEN, stdin);
                buffer[strlen(buffer) - 1] = 0;

                if(strcmp(buffer, "exit") == 0) {
                    send_message(sockfd, buffer);
                    return 0;
                } 
                char *token = strtok(buffer, " ");
                if (token == NULL) {
                    printf("Invalid command\n");
                    return -1;
                }
        
                
                if (strcmp(token, "subscribe") == 0) {
                
                    ret = subscribe(sockfd, token, buffer);
                    if (ret < 0) {
                       perror("ERROR writing to socket!\n");
                        exit(1);
                   } 
                } else if (strcmp(token, "unsubscribe") == 0) {
                
                    ret = unsubscribe(sockfd, token, buffer);
                    if (ret < 0) {
                        perror("ERROR writing to socket!\n");
                        exit(1);
                    } 
                }
                else {
                    printf("Invalid command\n");
                    continue;
                }
                
            } else if(pfds[1].revents & POLLIN) {
                memset(buffer, 0, BUFLEN);
                int ret = recv_message(sockfd, buffer);
                
                if(strcmp(buffer, "exit") == 0 || ret == 0) {
                    return 0;
                } else {
                    printf("%s\n", buffer);
                }
            }
        }

    close(sockfd);
    return 0;
}
