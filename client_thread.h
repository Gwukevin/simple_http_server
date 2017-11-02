#include<pthread.h>
#ifndef CLIENT_THREAD_H
#define CLIENT_THREAD_H

#define  NUM_CORES 4

typedef struct{
    pthread_t pid;
    int fd;
} p_client_str;

struct req_list {
    int fd;
    struct req_list *next_req;
};
typedef struct req_list Req_List;
void client_process_thread(void * p_client);



pthread_t thread_pool[NUM_CORES];

int init_thread_pool(pthread_t * p_thread_pool,Req_List * head);
void * single_thread_task(void * head);

void  thread_pool_process(int accept_fd);



int add_req_node(Req_List * head, Req_List * node);
Req_List* get_del_req_firstnode(Req_List * head);

#endif