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
#include "utils.h"
#define MAX_CLIENTS 10 // numarul maxim de clienti in asteptare


void send_message(int socket, const void *buffer){
    int ret;
    int len = strlen(buffer) + 1;
    ret = send(socket, &len, sizeof(int), 0);
    if (ret < 0) {
        printf("Eroare trimitere lungime mesaj la client");
        exit(1);
    }
   
    ret = send(socket, buffer, len, 0);
    
    if (ret < 0) {
        printf("Eroare trimitere mesaj la client");
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
            printf("Eroare citire de la server");
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

//cautare client dupa id
int search_client_by_id(client *clients, int n, char *id) {
    int i;
    for (i = 0; i < n; i++) {
        if (!strcmp(clients[i].id, id)) {
            return i;
        }
    }
    return -1;
}

//cautare client dupa socket
int search_client_by_socket(client *clients, int n, int socket) {
    int i;
    for (i = 0; i < n; i++) {
        if (clients[i].socket == socket) {
            return i;
        }
    }
    return -1;
}

//creare client nou
client* create_client(int new_tcp_client, char * buffer){
    client* new_client = calloc(1, sizeof(client));
    if (new_client == NULL) {
        printf("Error creating client");
        exit(1);
    }

    memcpy(new_client->id, buffer, strlen(buffer) + 1);
    new_client->socket = new_tcp_client;
    new_client->nr_topics = 0;
    new_client->d_messages = NULL;
    new_client->topics = NULL;
    new_client->nr_msg = 0;

    return new_client;
}

//adaugare client nou
int add_client(client** clients, int* n, int newsockfd, char* buffer) {
    client *new_client = create_client(newsockfd, buffer);
    if (new_client == NULL) {
        printf("Eroare la crearea clientului");
        exit(1);
        return 0;
    }
                
    (*n)++;
    //adaugam clientul in vectorul de clienti conectati
    client *tmp = realloc(*clients, *n * sizeof(client));
    if (tmp == NULL) {
        printf("Eroare la realocarea memoriei pentru clienti");
        exit(1);
        return 0;
    } 

    *clients = tmp;
    memcpy(&((*clients)[*n - 1]), new_client, sizeof(client)); 
    //eliberez memoria            
    free(new_client);
    return 1;
}

//cautare topic dupa nume in vectorul de topicuri al clientului
int search_topic(client * client, char * topic){
    for(int i = 0; i < client->nr_topics; i++){
        if(!strcmp(client->topics[i].topic, topic)) {
            return i;
        }
    }
    return -1;
}

//adaugare topic nou in vectorul de topicuri al clientului
int add_topic(client **clients, int index_client, char *t, int sf) {
    (*clients)[index_client].topics = realloc((*clients)[index_client].topics, ((*clients)[index_client].nr_topics + 1) * sizeof(topic));                        
    if ((*clients)[index_client].topics == NULL) {
        printf("Eroare la realocarea memoriei pentru topicuri");
        return 0;
    }
    
    strcpy((*clients)[index_client].topics[(*clients)[index_client].nr_topics].topic, t);
    (*clients)[index_client].topics[(*clients)[index_client].nr_topics].flag = sf;
    (*clients)[index_client].nr_topics++;
    
    return 1;
}


//stergere topic din vectorul de topicuri al clientului
int delete_topic(client **clients, int index_client, int index_topic, char *t) {
    if((*clients)[index_client].topics == NULL) {
        return 0;
    }
    
    //mutam toate topicurile cu un index mai mare decat indexul topicului pe care vrem sa il stergem
    int remaining_topics = (*clients)[index_client].nr_topics - index_topic - 1;
    if (remaining_topics > 0) {
        memmove((*clients)[index_client].topics + index_topic, (*clients)[index_client].topics + index_topic + 1, remaining_topics * sizeof(topic));
    }
    
    //reducem numarul de topicuri
    (*clients)[index_client].nr_topics--;
    
    //realocam memoria
    if((*clients)[index_client].nr_topics > 0) {
        topic *tmp = realloc((*clients)[index_client].topics, (*clients)[index_client].nr_topics * sizeof(topic));
        if (tmp != NULL) {
            (*clients)[index_client].topics = tmp;
        } else {
            printf("Failed to reallocate memory for topics");
            return 0;
        }
    } else {
        //daca nu mai avem topicuri, eliberam memoria
        free((*clients)[index_client].topics);
        (*clients)[index_client].topics = NULL;
        (*clients)[index_client].nr_topics = 0;
    }

    return 1;
}

int add_to_offline_messages(client *clients_dis, int nr_clients_d,  char * message, char * buffer){
    for(int i = 0; i < nr_clients_d; i++) {
        for (int j = 0; j < clients_dis[i].nr_topics; j++) {
            if (strcmp(clients_dis[i].topics[j].topic, message) == 0) {
                if(clients_dis[i].topics[j].flag == 1) {
                    clients_dis[i].nr_msg = clients_dis[i].nr_msg + 1;
                    disconnected * tmp;
                    tmp = realloc(clients_dis[i].d_messages, clients_dis[i].nr_msg * sizeof(disconnected));
                    if(tmp == NULL) {
                         printf("realloc failed\n");
                    }   
                    clients_dis[i].d_messages = tmp;
                    strcpy(clients_dis[i].d_messages[clients_dis[i].nr_msg - 1].message, buffer);
                }
            }
        }
    }

    return 1;
}

void send_udp_message_to_tcp(client * clients_con, client* clients_dis, int nr_clients_c, int nr_clients_d, char * message, char *buffer) {
   //trimitem mesaajul daca clientul este conectat
    for(int i = 0; i < nr_clients_c; i++){
        for(int j = 0; j < clients_con[i].nr_topics; j++){
            if(strcmp(clients_con[i].topics[j].topic, message) == 0) {
                send_message(clients_con[i].socket, buffer);   
            }
        }
    }
    //daca clientul este deconectat, adaugam mesajul in vectorul de mesaje offline
    if(nr_clients_d > 0) {
        add_to_offline_messages(clients_dis, nr_clients_d, message, buffer);
    }
}

//functie care returneaza formatul mesajului in functie de tipul acestuia
char* get_type_format(udp_msg * message, struct sockaddr_in * udp_info, char * payload){
    char * format = calloc(sizeof(udp_msg), sizeof(char));
    char * type_string = calloc(BUFLEN, sizeof(char));
    udp_msg * mess = (udp_msg *)payload;
    switch (message->type)
    {
    case 0:
        {
            uint8_t sign = (uint8_t)payload[51];
            uint32_t * number = (uint32_t *)(payload + 51 + (sizeof(sign)));
            int final = ntohl(*number);
            sprintf(format, "INT - %d",(sign == 1) ? -1 * final : final);
            break;
        }
    case 1:
        {
            uint16_t  short_real;
            memcpy(&short_real, mess->payload, sizeof(uint16_t));
            short_real = ntohs((short_real));
            float result = short_real / 100.0;
            sprintf(format, "SHORT_REAL - %.2f", result);
            break;
        }
    case 2:
        {
            uint8_t  sign = (uint8_t)payload[51];
            uint32_t  number;
            memcpy(&number, mess->payload + sizeof(sign), sizeof(uint32_t));
            uint8_t exp;
            memcpy(&exp, mess->payload + sizeof(sign) + sizeof(uint32_t), sizeof(uint8_t));
            float divide = 1;
            for(uint8_t i = 0; i < exp; i++) divide *= 10;
            float res = ntohl((number)) / divide;
            sprintf(format,"FLOAT - %.4f", (sign == 1) ? -1 * res : res);
            break;
        }
    case 3:
        {
            sprintf(format, "STRING - %s", mess->payload);
            break;
        }
    default:
        {
            sprintf(type_string, "%s:%hu - %s - %s",inet_ntoa(udp_info->sin_addr), ntohs(udp_info->sin_port), message->topic, format);
        }
    }

    sprintf(type_string, "%s:%hu - %s - %s",inet_ntoa(udp_info->sin_addr), ntohs(udp_info->sin_port), message->topic, format);
    return type_string;
}

//functie care inchide socketii si eliberaza memoria
int exit_all(int tcp_sockfd, int udp_sockfd, client *online, client *offline, int nr_online, int nr_offline, char * buffer) {
    close(tcp_sockfd);
    close(udp_sockfd);
    strcpy(buffer, "exit");
    for (int j = 0; j < nr_online; j++) {
        send_message(online[j].socket, buffer);
        free(online[j].topics);
        free(online[j].d_messages);
    }
    for(int j = 0; j < nr_offline; j++) {
        send_message(offline[j].socket, buffer);
        free(offline[j].topics);
        free(offline[j].d_messages);
    }
    
    free(online);
    free(offline);

    return 1;
}

int main(int argc, char *argv[]) {

    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if (argc != 2) {
        fprintf(stderr,"Usage: %s server_port\n", argv[0]);
        exit(0);
    }

    int tcp_sockfd, udp_sockfd, newsockfd;
    char buffer[BUFLEN];
    struct sockaddr_in tcp_server, udp_server;
    socklen_t tcp_len, udp_len; //lungimea adresei clientului
    int ret;
    client* online = NULL; //clienti conectati
    client* offline =  NULL; //clienti deconectati
    int nr_online = 0;
    int nr_offline = 0;

    fd_set read_fds;    // multimea de citire folosita in select()
    fd_set tmp_fds;        // multime folosita temporar
    int fdmax;  // valoare maxima file descriptor din multimea read_fds


    // se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    // se deschide socketul TCP
    tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sockfd < 0) {
        printf("ERROR opening TCP socket");
        exit(1);
    }

    // se deschide socketul UDP
    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) {
        printf("ERROR opening UDP socket");
        exit(1);
    }

    // se completeaza informatiile despre adresa serverului
    uint16_t server_port = atoi(argv[1]);
    if (server_port < 1024) {
        fprintf(stderr, "Port number must be greater than 1024\n");
        exit(0);
    }

    //se completeaza informatiile despre adresa serverului TCP
    tcp_server.sin_family = AF_INET;
    tcp_server.sin_port = htons(server_port);
    tcp_server.sin_addr.s_addr = INADDR_ANY;

    //se completeaza informatiile despre adresa serverului UDP
    udp_server.sin_family = AF_INET;
    udp_server.sin_port = htons(server_port);
    udp_server.sin_addr.s_addr = INADDR_ANY;


    //se face bind pe socketul TCP
    ret = bind(tcp_sockfd, (struct sockaddr *) &tcp_server, sizeof(struct sockaddr));
    if (ret < 0) {
        printf("ERROR on binding TCP");
        exit(1);
    }

    //se face bind pe socketul UDP
    ret = bind(udp_sockfd, (struct sockaddr *) &udp_server, sizeof(struct sockaddr));
    if (ret < 0) {
        printf("ERROR on binding UDP");
        exit(1);
    }

    //se asculta pe socketul TCP
    ret = listen(tcp_sockfd, MAX_CLIENTS);
    if (ret < 0) {
        printf("ERROR on listening TCP");
        exit(1);
    }

    // se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds  
    FD_SET(tcp_sockfd, &read_fds);
    FD_SET(udp_sockfd, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    fdmax = tcp_sockfd > udp_sockfd ? tcp_sockfd : udp_sockfd;

    // dezactivaare algoritmului lui Neagle
    int flag = 1;
    int result = setsockopt(tcp_sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
    
    if (result < 0) {
        printf("ERROR on setsockopt");
        exit(1);
    }

    // main loop
    while(1) {
        tmp_fds = read_fds;
        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        if (ret < 0) {
            printf("ERROR in select");
            exit(1);
        }
       
        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmp_fds)) {
                //daca am primit ceva de la un socket UDP
                if (i == udp_sockfd) {
                    memset(buffer, 0, BUFLEN);
                    udp_len = sizeof(udp_server);
                    udp_msg  udp_msg_received;
                    ret = recvfrom(udp_sockfd, buffer, BUFLEN, MSG_WAITALL, (struct sockaddr *) &udp_server, &udp_len);
                    if (ret < 0) {
                        printf("ERROR in recvfrom");
                        exit(1);
                    }
                    //copiem informatiile din mesajul primit
                    strcpy(udp_msg_received.ip, inet_ntoa(udp_server.sin_addr));
                    udp_msg_received.port = ntohs(udp_server.sin_port);
                    memcpy(udp_msg_received.topic, buffer, 50);
                    udp_msg_received.type = (uint8_t)buffer[50];
                    char* message = get_type_format(&udp_msg_received, &udp_server, buffer);
                    if(ret == -1){
                        printf("ERROR in parse_message_for_tcp");
                        exit(1);
                    }
                    //trimit mesajul UDP la toti clientii 
                    send_udp_message_to_tcp(online, offline, nr_online, nr_offline, udp_msg_received.topic, message);
                }
                //daca am primit ceva de la un socket TCP
                else if (i == tcp_sockfd) {
                    // a venit o cerere de conexiune pe socketul inactiv (cel cu listen), pe care serverul o accepta
                    // se va crea un nou socket pe care se va face comunicarea cu clientul
                    // si se va adauga noul socket intors de accept() la multimea descriptorilor de citire
                    tcp_len = sizeof(tcp_server);
                    newsockfd = accept(tcp_sockfd, (struct sockaddr *) &tcp_server, &tcp_len);
                    if (newsockfd < 0) {
                        printf("ERROR in accept");
                        exit(1);
                    } 
 
                    // se dezactiveaza algoritmul lui Neagle
                    int flag = 1;
                    int result = setsockopt(newsockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
                    
                    if (result < 0) {
                        printf("ERROR on setsockopt");
                        exit(1);
                    }

                    // se primeste id-ul clientului
                    recv_message(newsockfd, buffer);
                    
            
                    //cautam clientul
                    int idx_client_online = -1;
                    int idx_client_offline = -1;
                    if (online != NULL) {
                        idx_client_online = search_client_by_id(online, nr_online, buffer);
                    }
                    if (offline != NULL) {
                        idx_client_offline = search_client_by_id(offline, nr_offline, buffer);
                    }

                    //daca clientule exista deja, ii trimitem un mesaj de eroare
                    if (idx_client_online != -1 && idx_client_offline == -1) {
                        printf("Client %s already connected.\n", buffer);
                        strcpy(buffer, "exit");
                        send_message(newsockfd, buffer);
                        continue;

                    } else if (idx_client_offline != -1 && idx_client_online == -1) {
                        //daca clientul exista in vectorul de clienti deconectati, il adaugam in cel de clienti conectati
                        offline[idx_client_offline].socket = newsockfd;
                        if (offline[idx_client_offline].nr_msg > 0) {
                            //trimitem mesajele offline
                            for (int j = 0; j < offline[idx_client_offline].nr_msg; j++) {
                                send_message(offline->socket, offline[idx_client_offline].d_messages[j].message);
                            }
                            free(offline->d_messages);
                            offline->nr_msg = 0;
                            offline->d_messages = NULL;
                        }
                        
                        //stergem clientul din vectorul de clienti deconectati
                        int after_this = nr_offline - idx_client_offline - 1;
                        if (after_this > 0) {
                            memmove(offline + idx_client_offline, offline + idx_client_offline + 1, after_this * sizeof(client));
                        }
                        //adaugam clientul in vectorul de clienti conectati
                        nr_online++;
                        client *tmp = realloc(online, nr_online * sizeof(client));
                        if (tmp == NULL) {
                            printf("ERROR in realloc pl");
                            exit(1);
                        }
                        online = tmp;
                        if (memcpy(&online[nr_online - 1], &offline[idx_client_offline], sizeof(client)) == NULL) {
                            printf("ERROR in memcpy pl");
                            exit(1);
                        }
                        //eliberez memoria in vectorul de clienti deconectati
                        nr_offline--;
                        if(nr_offline > 0) {
                            client *tmp = realloc(offline, nr_offline * sizeof(client));
                            if (tmp != NULL) {
                                offline = tmp;
                            } else {
                                printf("Failed to reallocate memory for offline");
                                exit(EXIT_FAILURE);
                            }
                        } else {
                            free(offline);
                            nr_offline = 0;
                            offline = NULL;
                        }

                    //daca nu exista, il adaugam in vectorul de clienti conectati
                    } else if (idx_client_online == -1 && idx_client_offline == -1) {
                        add_client(&online, &nr_online, newsockfd, buffer);
                    }
                    //adaugam socketul nou in multimea descriptorilor de citire
                    FD_SET(newsockfd, &read_fds);
                    if (newsockfd > fdmax) {
                        fdmax = newsockfd;
                    }

                    //afisam mesajul
                    printf("New client %s connected from %s:%d.\n", buffer, inet_ntoa(tcp_server.sin_addr), ntohs(tcp_server.sin_port));

                //daca este un mesaj de la stdin
                } else if (i == STDIN_FILENO) {
                    fgets(buffer, BUFLEN - 1, stdin);
            
                    if (strncmp(buffer, "exit", 4) == 0) {
                        exit_all(tcp_sockfd, udp_sockfd, online, offline, nr_online, nr_offline, buffer);
                        return 0;
                    } else {
                        printf("command not found");
                    }

                } else  {
                    //comanda de la client TCP

                    ret = recv_message(i, buffer);
                 
                    int idx_client_online = 0;
                    int idx_client_offline = 0;
                    if (online != NULL) {
                        idx_client_online = search_client_by_socket(online, nr_online, i);
                    }
                    if (offline != NULL) {
                         idx_client_offline = search_client_by_socket(offline, nr_offline, i);
                    }
                    //daca clientul nu exista, il adaugam in vectorul de clienti deconectati

                    if (idx_client_online == -1 && idx_client_offline == -1) {
                        printf("Client not found\n");
                        continue;
                    }
                    //daca clientul s a decconectat, il stergem din vectorul de clienti conectati si il adaugam in cel deconectati
                    if (ret == 0 || strcmp(buffer, "exit") == 0) {
                        nr_offline++;
                        client *tmp = realloc(offline, nr_offline * sizeof(client));
                        if (tmp == NULL) {
                            printf("ERROR in realloc pl");
                            exit(1);
                        }
                        offline = tmp;
                    
                        if (memcpy(&offline[nr_offline - 1], &online[idx_client_online], sizeof(client)) == NULL) {
                            printf("ERROR in memcpy pl");
                            exit(1);
                        }
                       
                        if (idx_client_online < nr_online) {
                            int after_this = nr_online - idx_client_online - 1;
                            if (after_this > 0) {
                                memmove(online + idx_client_online, online + idx_client_online + 1, after_this * sizeof(client));
                            }
                        }
            
                        nr_online--;
                        if(nr_online > 0) {
                            tmp = realloc(online, nr_online * sizeof(client));
                            if (tmp != NULL) {
                                online = tmp;
                            } else {
                                printf("Failed to reallocate memory for offline");
                                exit(EXIT_FAILURE);
                            }
                        } 
                        printf("Client %s disconnected.\n", online[idx_client_online].id);
                        FD_CLR(i, &read_fds);
                    }
                    else {
                        char *token = strtok(buffer, " ");
                    
                        //daca clientul este conectat, ii trimitem mesajul
                        if (strcmp(token, "subscribe") == 0) {
                
                            char *t = strtok(NULL, " ");
                            char *token = strtok(NULL, " ");
                            int sf = atoi(token);
                        
                            //daca clientul este conectat, ii adaugam topicul in vectorul de topicuri
                            if (idx_client_online != -1) {
                                int index_topic = search_topic(&online[idx_client_online], t);
                                if (index_topic == -1) {
                                    add_topic(&online, idx_client_online, t, sf);
                                } else {
                                    online[idx_client_online].topics[index_topic].flag = sf;
                                }
                            } 

                        } else if (strcmp(token, "unsubscribe") == 0) {
                            char *t = strtok(NULL, "");
                            if (idx_client_online != -1) {
                                int index_topic = search_topic(online, t);
                                if (index_topic != -1) {
                                    delete_topic(&online, idx_client_online, index_topic, t);
                                } else {
                                    printf("Index not found");
                                }
                            } 

                        } else {
                            printf("Invalid command!\n");
                            return 0;
                        }
                    }
                }
            }
        }
    }
}
