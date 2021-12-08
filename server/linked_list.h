/* list node struct */
struct node {
   bool free;  // indicates that this storage is not allocated
   struct sockaddr_in frm_c; //key and data
   struct node *next;
};

/* storage for list */
#define MAX_LIST_LEN 25

/* function prototypes */
void initializeList(void);
void printList(void);
void insertInList(struct sockaddr_in *pfrm_c);
void deleteFrmList(struct sockaddr_in *pfrm_c);
bool match_addr_port(struct sockaddr_in *p1, struct sockaddr_in *p2);
bool isEmpty(void);
int length(void);
struct node *getListTail();

/*******************************
struct sockaddr_in {
   sa_family_t    sin_family;
   in_port_t      sin_port;   //16 bit port 
   struct in_addr sin_addr;
}
Internet address
struct in_addr {
   uint32_t    s_addr;        //32 bit IP addr
}
*******************************/

/* 
In main() as global variable:
extern struct node *head;

In linked_list.c
struct node *head;
struct node *current;
struct node client_list[MAX_LIST_LEN];
 */
