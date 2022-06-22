#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

#include "sys/types.h"
#include "sys/socket.h"

#include "netinet/in.h"
#include "poll.h"
#include "string.h"

#define MSG_LEN 2

int network_socket;
int turn = 0;
int player;
int opponent;

int board[3][3];

void connect_to_server(){

    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int connection_status = connect(network_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    if (connection_status == -1){
        perror("Error in connection\n");
        exit(1);
    }
}

void display(){
    char str_board[3][4];
    str_board[0][3] = '\n';
    str_board[1][3] = '\n';
    str_board[2][3] = '\n';

    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            if (board[i][j] == -1){
                str_board[i][j] = ' ';
            } else if (board[i][j] == 0){
                str_board[i][j] = 'O';
            } else if (board[i][j] == 1){
                str_board[i][j] = 'X';
            }
        }
    }

    for (int i = 0; i < 3; i++){
        printf("%s", str_board[i]);
    }
}

int check_board(){
    for (int i = 0; i < 3; i++){
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != -1){
            return 1;
        }
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != -1){
            return 1;
        }
    }
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != -1){
        return 1;
    }
    return 0;
}

int push_board(char move, int moving_player){
    int m = move - '0';
    if (board[m/3][m%3] == -1){
        board[m/3][m%3] = moving_player;
        return 0;
    } else {
        return 1;
    }

}

void make_move(){
    int move;
    if (turn > 0) {
        printf("Pass your move ");
        scanf("%d", &move);
        char buff[MSG_LEN];
        sprintf(buff, "%d", move);
        if (push_board(buff[0], player)){
            printf("You lost\n")
            send(network_socket, "W", MSG_LEN, 0);
            exit(0);
        }
        if (check_board()){
            printf("You won\n");
            send(network_socket, "L", MSG_LEN, 0);
            exit(0);
        }
        send(network_socket, buff, MSG_LEN, 0);
    }
    turn++;
}

void server_listen(){


    while (1){
        char server_response[MSG_LEN];
        struct pollfd *sockets = malloc(sizeof (struct pollfd));
        sockets->fd = network_socket;
        sockets->events = POLLIN;

        poll(sockets, 1, -1);
        recv(network_socket, server_response, MSG_LEN, 0);

        printf("Server send data: %s\n", server_response);
        if (strcmp(server_response, "O") == 0) {
            turn = 1;
            player = 0;
            opponent = 1;
        } else if (strcmp(server_response, "X") == 0){
            turn = 0;
            player = 1;
            opponent = 0;
        } else if (strcmp(server_response, "L") == 0){
            printf("You lost\n");
            exit(0);
        } else if (strcmp(server_response, "W") == 0){
            printf("You won\n");
            exit(0);
        } else if (server_response[0] >= '0' && server_response[0] < '9'){
            push_board(server_response[0], opponent);
        }
        printf("turn %d\n", turn);

        display();

        make_move();

        free(sockets);
    }
}


int main(){

    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            board[i][j] = -1;
        }
    }

    connect_to_server();

    server_listen();

    return 0;
}