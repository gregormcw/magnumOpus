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


