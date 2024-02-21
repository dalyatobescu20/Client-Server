#ifndef UTILS_H
#define UTILS_H
#define BUFLEN 2000

// structura pentru mesajul de la udp
typedef struct __udp_msg {
    char topic[50];
    uint8_t type;
    char payload[1501];
    char ip[16];
    int port;
}udp_msg;

//structura pentru un topic
typedef struct __topic {
    char topic[50];
    int flag;
}topic;

//structura pentru un mesajele clientilor care sunt offline
//si care trebuie trimise cand se reconecteaza
typedef struct __disconnected {
   char message[BUFLEN];
}disconnected;

//structura pentru un client
typedef struct __client {
    int socket;
    char id[10];
    topic *topics;
    int nr_topics;
    disconnected *d_messages;
    int nr_msg;
}client;

#endif //UTILS_H
