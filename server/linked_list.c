#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>     /* for socket functions */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include "linked_list.h"

struct node *head;
struct node *current;
struct node client_list[MAX_LIST_LEN];

//initialize list
void initializeList()
{
   head = NULL;
   current = NULL;
   for (int i=0; i<MAX_LIST_LEN; i++) {
      client_list[i].free = true;
      client_list[i].next = NULL;
   }
}

//insert new node at head of list
void insertInList(struct sockaddr_in *pfrm_c) {
   int i;

   //is item already in list?
   struct node *ptr = head;
   //start from the beginning
   while(ptr != NULL) {
      //check for match
      if ( match_addr_port(pfrm_c, &ptr->frm_c) ) {
         //aready in list, so just return
         printf("Already in list\n");
         return;
      }
      ptr = ptr->next;
   }

   //not in list, so add it
   //find free storage
   for (i=0; i<MAX_LIST_LEN; i++) {
      //printf("Check for free storage %d\n", i);
      if ( client_list[i].free == true ) {
         break; //found free storage
      }
   }
   if (i == MAX_LIST_LEN) {
      //end of list with no free, so just return
      printf("No free storage\n");
      return;
   }

   //link is new storage
   struct node *link = &client_list[i];
   link->free = false;	
   memcpy( &link->frm_c, pfrm_c, sizeof(struct sockaddr_in) );

   //point it to old first node
   link->next = head;
	
   //point head to new node
   head = link;

   printf( "Inserted %3ld %s:%d\n", 
      (head - &client_list[0]),
      inet_ntoa(link->frm_c.sin_addr),
      (int)link->frm_c.sin_port );
}

//delete node from list
void deleteFrmList(struct sockaddr_in *pfrm_c) {
   //is item in list?
   struct node *ptr = head;
   struct node *previous = head;

   //check head of list
   if ( match_addr_port(pfrm_c, &ptr->frm_c) ) {
      //found it
      printf( "Deleted %3ld %s:%d\n", 
         (ptr - &client_list[0]),
         inet_ntoa(ptr->frm_c.sin_addr),
         (int)ptr->frm_c.sin_port );
      //mark storage as free
      ptr->free = true;
      //next is new head
      head = ptr->next;
      return;
   }

   //not at head, so start from next
   ptr = ptr->next;
   while(ptr != NULL) {
      if ( match_addr_port(pfrm_c, &ptr->frm_c) ) {
         //found it
         break;
         printf( "Deleted %3ld %s:%d\n", 
            (ptr - &client_list[0]),
            inet_ntoa(ptr->frm_c.sin_addr),
            (int)ptr->frm_c.sin_port );
         //mark storage as free
         ptr->free = true;
         //bridge over deleted link
         previous->next = ptr->next;
         return;
      }
      previous = ptr;
      ptr = ptr->next;
   }
   if (ptr == NULL) {
      //not found, so just return
      return;
   }
}

struct node *getListTail()
{
   struct node *current;
   if (head == NULL) {
      return NULL;
   }   
   for(current = head; current->next != NULL; current = current->next) {
   }
   return current;
}

 
//match list key
bool match_addr_port(struct sockaddr_in *p1, struct sockaddr_in *p2)
{
   if ( (p1->sin_port == p2->sin_port) &&
      (p1->sin_addr.s_addr == p2->sin_addr.s_addr) ) {
      return true;
   }
   else {
      return false;
   }
}

//display the list
void printList() {
   struct node *ptr = head;
   printf("[\n");
   
   //start from the beginning
   while(ptr != NULL) {
      printf( "%3ld %s:%d\n", 
         (ptr - &client_list[0]),
         inet_ntoa(ptr->frm_c.sin_addr),
         (int)ptr->frm_c.sin_port );
      ptr = ptr->next;
   }
   
   printf("]\n");
}

//is list empty
bool isEmpty() {
   return (head == NULL);
}

//list length
int length() {
   int length = 0;
   struct node *current;
   
   for(current = head; current != NULL; current = current->next) {
      length++;
   }
   
   return length;
}

