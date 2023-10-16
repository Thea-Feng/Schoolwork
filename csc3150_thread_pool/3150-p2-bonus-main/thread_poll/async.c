
#include <stdlib.h>
#include <pthread.h>
#include "async.h"
#include "utlist.h"
pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;
int count = 0;
my_queue_t q;

int cnt, real_cnt;

void *init( void *t ){
    long tid = (long)t;
    my_item_t *q_item;
    int args;
    int ccnt = 0;
    void (*handler) (int) = NULL;

    // printf("This is %d thread\n", (int) tid);
    while(1){
        pthread_mutex_lock(&count_mutex);
        if(q.size == 0)
        pthread_cond_wait(&count_threshold_cv, &count_mutex);
        // printf("This is %d thread executing work\n", (int) tid);  
        // if(q.size == 0) printf("This is wrong\n");    
        if(q.size > 0){
            q_item = q.head;
            handler = q_item->handler;
            args = q_item->args;   
            DL_DELETE(q.head, q_item);
            q.size--;
            // cnt++;
            // printf("%d request coming in, %d thread with qsize %d\n", cnt, (int)t, q.size);
        }  else {handler = NULL;}
               
        pthread_mutex_unlock(&count_mutex);
        if(handler != NULL)handler(args); 

    }
    
}



void async_init(int num_threads) {
    int rc;
    pthread_t threads[num_threads+1];
    pthread_mutex_init(&count_mutex, NULL);
    pthread_cond_init(&count_threshold_cv, NULL);
	for(long i = 0; i < num_threads; i++){
		rc = pthread_create(&threads[i], NULL, init, (void*)i);
		if(rc){
			printf("ERROE: return code from pthread_create() is %d\n", rc);
			exit(1);
		}
	}
    return;
    /** TODO: create num_threads threads and initialize the thread pool **/
}

void async_run(void (*handler)(int), int args) {
    
    // printf("Work coming in\n");
    my_item_t *item;
    item = (struct my_item*)malloc(sizeof(struct my_item));
    item->args = args;
    item->handler = handler;
	pthread_mutex_lock(&count_mutex);
    q.size ++;
    DL_APPEND(q.head, item);
    pthread_cond_signal(&count_threshold_cv);
    pthread_mutex_unlock(&count_mutex);
    // handler(args);
    /** TODO: rewrite it to support thread pool **/
}