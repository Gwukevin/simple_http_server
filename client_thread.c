#include <stdlib.h>
#include<stdio.h>
#include<util.h>
#include <server.h>
#include<client_thread.h>
// pthread_mutex_t worker_mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t cond;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
void client_process_thread(void * p_client){
    p_client_str * p_cli = (p_client_str *)p_client;
    printf("client_process_thread is running, thread id is:%d,fd=%d\n",p_cli->pid,p_cli->fd);
    client_process(p_cli->fd);
    free(p_cli);
    printf("client_process_thread:p_id has been freed...\n");
}


int init_thread_pool(pthread_t * p_thread_pool,Req_List * head){
    for(int i = 0;i < NUM_CORES; i++){
        if(pthread_create(&(p_thread_pool[i]),NULL,(void*)single_thread_task,(void*)head)){
            printf("init_thread_pool failed...\n");
            abort();
            return 0;
        }
    }
    return 1;
}

void  thread_pool_process(int accept_fd){
    // pthread_mutex_t worker_mutex = PTHREAD_MUTEX_INITIALIZER;
    // pthread_cond_t cond;
    int connections_num = 0;
    Req_List * p_head = NULL;
    Req_List * node = NULL;
    int fd;
    p_head = (Req_List *)malloc(sizeof(Req_List));
    if(!p_head){
        printf("thread_pool_process:malloc head Req_list error");
        abort();
        return ;
    }
    p_head->fd = -1;
    p_head->next_req = NULL;
    if(!init_thread_pool(thread_pool,p_head)){
        printf("thread_pool_process:init thread pool failed...\n");
        return;
    }

    while(1){
        fd = server_accept(accept_fd);
        printf("connextion count:%d\n",++connections_num);
        if(fd < 0){
            continue;
        }
        node = (Req_List *)malloc(sizeof(Req_List));
        if(!node){
            printf("thread_pool_process:malloc Req_list error");
            abort();
            return;
        }
        node->fd = fd;
        node->next_req = NULL;
        //lock the list to add request
        pthread_mutex_lock(&mutex);
        add_req_node(p_head,node);
        pthread_mutex_unlock(&mutex);

        pthread_cond_broadcast(&cond);

    }
    return;
}

void * single_thread_task(void * head){
    Req_List * node;
    while(1){
        pthread_mutex_lock(&mutex);
        
            while(!((Req_List*)head)->next_req){
                pthread_cond_wait(&cond, &mutex);
            }
        
            node = get_del_req_firstnode((Req_List *)head);

            client_process(node->fd);
            free(node);
            pthread_mutex_unlock(&mutex);
    }
}



int add_req_node(Req_List * head, Req_List * node){
   while(head->next_req){
       head = head->next_req;
   }

   head->next_req = node;
   return 1;
}
Req_List* get_del_req_firstnode(Req_List * head){
    Req_List * temp = NULL;
    if(!head->next_req){
        return temp;
    }
    temp = head->next_req;
    head->next_req = temp->next_req;
    return temp;
}