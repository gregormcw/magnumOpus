/*****************************************************************************
 * 
 * UDP Server V3
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> /* for socket functions */
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include "linked_list.h"

#define PORT_SC    5001
#define PORT_C     5002
#define MAX_PACKET_SIZE (3*1276)

extern struct node *head;

/* check for a connect/disconnect message from client */
void check_for_client_msg(int sock_c)
{
    struct sockaddr_in frm_c;
    socklen_t frm_c_len = sizeof(struct sockaddr_in);
    /* set timeout for client port */
    unsigned char buf[MAX_PACKET_SIZE];
    int n;
    /* for timeout */
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 2 * 1000; // 2 milliseconds
    setsockopt(sock_c, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    /* Check if there is a message from client, with timeout */
    n = recvfrom(sock_c, buf, MAX_PACKET_SIZE, 0,
        (struct sockaddr *)&frm_c, &frm_c_len);
    if (n >= 0) {
        if (buf[0] == 'C') {
            printf("Client connected\n");
            insertInList(&frm_c);
        }
        if (buf[0] == 'D') {
            printf("Client disconnected\n");
            deleteFrmList(&frm_c);
        }
    }
    else if (n == -1) {
        ; //timeout from waiting
    }
    else {
        perror("recvfrom client");
        exit(0);
    }
}

/* send audio block to all clients */
void send_to_clients(int sock_c, unsigned char *buf, int n)
{
    socklen_t frm_c_len = sizeof(struct sockaddr_in);

    struct node *ptr = head;
    while(ptr != NULL) {
    if ( (n = sendto(sock_c, buf, n,
        0, (struct sockaddr *)&ptr->frm_c, frm_c_len)) < 0) {
        perror("sendto");
        exit(0);
    }
    ptr = ptr->next;
   }
}

int main(int argc, char *argv[])
{
    int sock_sc, sock_c, length, n;
    /* source client */
    struct sockaddr_in rcv_sc;
    struct sockaddr_in frm_sc;
    socklen_t frm_sc_len;
    /* client */
    struct sockaddr_in rcv_c;
    /* message */
    unsigned char buf[MAX_PACKET_SIZE];

    /* open rcv and snd sockets */
    if ( (sock_sc = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Opening source client socket");
        exit(0);
    }
    if ( (sock_c = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Opening client socket");
        exit(0);
    }

    /* bind source client port to socket */
    length = sizeof(rcv_sc);
    bzero(&rcv_sc,length);
    rcv_sc.sin_family = AF_INET;
    rcv_sc.sin_addr.s_addr = INADDR_ANY;
    rcv_sc.sin_port = htons(PORT_SC);
    if (bind(sock_sc,(struct sockaddr *)&rcv_sc,length)<0) {
        perror("Binding to source client socket");
        exit(0);
    }
    /* bind client port to socket */
    length = sizeof(rcv_c);
    bzero(&rcv_c,length);
    rcv_c.sin_family = AF_INET;
    rcv_c.sin_addr.s_addr = INADDR_ANY;
    rcv_c.sin_port = htons(PORT_C);
    if (bind(sock_c,(struct sockaddr *)&rcv_c, length)<0) {
        perror("Binding to client socket");
        exit(0);
    }

    /* initialize linked list of clients */
    initializeList();
         
    frm_sc_len = sizeof(struct sockaddr_in);
    int block = 0;
    while (1) {
        /* wait for data from source client */
        if ( (n = recvfrom(sock_sc, buf, MAX_PACKET_SIZE, 0,
            (struct sockaddr *)&frm_sc, &frm_sc_len)) < 0) {
            perror("recvfrom source client");
            exit(0);
        }
        if (block % 50 == 0) {
            printf("Source client %d\n", block);
        }

        /* check for connect/disconnect message from client */
        check_for_client_msg(sock_c);

        /* send to all clients in linked list of clients */        
        send_to_clients(sock_c, buf, n);

        block++;
    }
    return 0;
}
