#include "stdio.h"
#include "stdlib.h"

#include "sys/types.h"
#include "sys/socket.h"

#include "netinet/in.h"
#include "string.h"
#include "poll.h"

#define MSG_LEN 2

int connected_clients_no = 0;
int client_sockets[2];
int server_socket;
char server_message[256];
int turn = 0;

void init_sockets(){
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    listen(server_socket, 2);
}

void wait_for_clients(){
    strcpy(server_message, "You have reached server");
    client_sockets[connected_clients_no] = accept(server_socket, NULL, NULL);
    send(client_sockets[0], "O", sizeof ("0"), 0);
    connected_clients_no++;

    client_sockets[connected_clients_no] = accept(server_socket, NULL, NULL);
    send(client_sockets[1], "X", sizeof ("X"), 0);

}

void client_respond(int client, char server_response[MSG_LEN]){
    send(client, server_response, MSG_LEN, 0);
}

void clients_listen(){



    while (1){
        char server_response[MSG_LEN];
        struct pollfd *sockets = malloc(2*sizeof (struct pollfd));
        sockets[0].fd = client_sockets[0];
        sockets[0].events = POLLIN;
        sockets[1].fd = client_sockets[1];
        sockets[1].events = POLLIN;

        poll(sockets, 2, -1);

        int socket;
        for (int i = 0; i < 2; i++){
            if (sockets[i].revents == POLLIN){
                socket = sockets[i].fd;
            }
        }
        recv(socket, server_response, MSG_LEN, 0);

        printf("Client send data: %s\n", server_response);
        turn = (turn+1)%2;

        client_respond(client_sockets[turn], server_response);

        free(sockets);
    }

}

int main(){


    init_sockets();

    wait_for_clients();

    clients_listen();

    return 0;
}