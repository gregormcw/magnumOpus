#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "fifo.h"

/* FIFO */
struct node *head;

void initializeFifo()
{
   head = NULL;
}

/* write newest to tail of FIFO */
void writeToFifo(unsigned char *buf, int len) 
{
   struct node *tail, *new_node;

   if (len > TYP_PKT_SIZE) {
      fprintf(stderr, "pkt size too large: %d\n", len);
      exit(0);
   }
   /* create new node */
   if ( (new_node = (struct node *)malloc(sizeof(struct node))) == NULL ) {
      fprintf(stderr, "ERROR: cannot malloc new node\n");
      exit(0);
   }
   /* copy to new node */
   new_node->len = len; 
   for (int i=0; i<len; i++) {
      new_node->buf[i] = buf[i];
   }

   /* get fifo tail */
   tail = getFifoTail();

   if (tail == NULL) {
      /* list was empty */
      head = new_node;
      head->next = NULL;
   }
   else {
      /* link to new node */
      tail->next = new_node;
      new_node->next = NULL;
   }
}

/* read oldest from head of FIFO */
int readFrmFifo(unsigned char *buf) 
{
   if (head == NULL) {
      /* list is empty */
      return 0;
   }

   /* copy from head buf */
   int len = head->len; 
   for (int i=0; i<len; i++) {
      buf[i] = head->buf[i];
   }

   /* next is new head */
   struct node *prev_head = head;
   head = prev_head->next;

   /* free storage */
   free(prev_head);  

   return len;
}


struct node *getFifoTail()
{
   struct node *current;
   if (head == NULL) {
      return NULL;
   }   
   for(current = head; current->next != NULL; current = current->next) {
   }
   return current;
}

//display the list
void printFifo(void) 
{
   char c;
   struct node *ptr = head;
   struct node *tail = getFifoTail();
   printf("[\n");
   
   //start from the beginning
   int i=0;
   while(ptr != NULL) {
      if (ptr == head) 
         c = 'H';
      else if (ptr == tail)
         c = 'T';
      else 
         c = ' ';
      printf( "%d %d %d %c\n", i++, ptr->buf[0], ptr->len, c);
      ptr = ptr->next;
   }
   
   printf("]\n");
}

//is list empty
bool fifoIsEmpty() 
{
   return (head == NULL);
}

//list length
int fifoLength() 
{
   int length = 0;
   struct node *current;
   
   for(current = head; current != NULL; current = current->next) {
      length++;
   }
   
   return length;
}

//free storage in list data structure
// int fifoNumFree() 
// {
//    int num_free = 0;
   
//    for(int i=0; i<FIFO_MAX_LEN; i++) {
//       if (fifoBuf[i].free)
//          num_free++;
//    }
   
//    return num_free;
// }

// #ifdef NOT_USED
// //initialize fifo
// void initializeFifo()
// {
//    head = NULL;
//    for (int i=0; i<FIFO_MAX_LEN; i++) {
//       fifoBuf[i].free = true;
//    }
// }

// //write a new UDP audio packet to FIFO
// //len is length of buf, including sequence number
// void writeToFifo(unsigned char *buf, int len) 
// {
//    int i;
//    bool inserted;
//    struct node *ptr;
//    struct node *previous;

//    //is item already in list?
//    ptr = head;
//    while(ptr != NULL) {
//       //check for match
//       if ( buf[0] == ptr->buf[0] ) {
//          //aready in list, so just return
//          pstatus("Already in FIFO list");
//          return;
//       }
//       ptr = ptr->next;
//    }

//    //find free storage
//    for (i=0; i<FIFO_MAX_LEN; i++) {
//       if ( fifoBuf[i].free == true ) {
//          //printf("Free storage at %d\n", i);
//          break; //found free storage
//       }
//    }
//    if (i == FIFO_MAX_LEN) {
//       //end of list with no free, so just return
//       pstatus("No free FIFO storage");
//       return;
//    }

//    //link is new storage, so copy buf to link
//    struct node *link = &fifoBuf[i];
//    link->free = false; 
//    link->len = len; 
//    for (int i=0; i<len; i++) {
//       link->buf[i] = buf[i];
//    }

//    //insert link node into list in seq num order
//    //seq num increase from head to tail
//    if (head == NULL) {
//       //if list is empty, link is new head
//       link->next = NULL;
//       head = link;
//       // printf("inserted at head\n");
//       // printf( "Inserted %3ld %d\n", 
//       //    (link - &fifoBuf[0]), link->buf[0] );
//       return;
//    }
//    ptr = head;
//    previous = head;
//    inserted = false;
//    while(ptr != NULL) {
//       if ( buf[0] < ptr->buf[0] ) {
//          //link goes before ptr
//          if (ptr == head) {
//             //if this is before head, then link is new head
//             link->next = head;
//             head = link;
//             inserted = true;
//             // printf("inserted before head\n");
//          }
//          else {
//             //else link is between previous and ptr
//             previous->next = link;
//             link->next = ptr;
//             inserted = true;
//             // printf("inserted between %ld and %ld\n",
//             //    (previous - &fifoBuf[0]), (ptr - &fifoBuf[0]) );
//          }
//       }
//       previous = ptr;
//       ptr = ptr->next;
//    }
//    if (inserted == false) {
//       //we got to end of list without finding place for link, 
//       //so link goes at end of list
//       previous->next = link;
//       link->next = NULL;
//       // printf("inserted at tail\n");
//    }

//    // printf( "Inserted %3ld %d\n", 
//    //    (link - &fifoBuf[0]), link->buf[0] );
// }

// //read UDP audio packet from FIFO 
// //return length of buf, including sequence number
// int readFrmFifo(unsigned char *buf) 
// {
//    struct node *ptr;

//    //if fifo is empty
//    if (fifoIsEmpty()) {
//       return 0;
//    }

//    //head is fifo output, so copy head to buf
//    ptr = head;
//    int n = ptr->len;
//    for (int i=0; i<n; i++) {
//       buf[i] = ptr->buf[i];
//    }
//    //mark storage as free
//    ptr->free = true;

//    if (fifoLength() == 1) {
//       //mark fifo as empty
//       head = NULL;
//    }
//    else {
//       head = ptr->next;
//    }

//    return ptr->len; 
// }
// #endif

