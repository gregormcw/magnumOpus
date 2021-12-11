/* FIFO
 * head of fifo is output
 * tail of fifo is input 
 * sequence numbers increase from head to tail
 */

#define MAX_PKT_SIZE (3*1276)
#define TYP_PKT_SIZE (3*256)

/* list node struct */
struct node {
//   bool free;  // indicates that this storage is not allocated
   int len;  //length of buf, including sequence number
   unsigned char buf[TYP_PKT_SIZE];  //first char is sequence number, rest is Opus data
   struct node *next;
};

/* storage for fifo */
#define FIFO_MAX_LEN    150
#define FIFO_HI         75
#define FIFO_LO         25

/* function prototypes */
void initializeFifo(void);
void writeToFifo(unsigned char *buf, int len);
int  readFrmFifo(unsigned char *buf);
struct node *getFifoTail(void);
void printFifo(void);
bool fifoIsEmpty(void);
int  fifoLength(void);
