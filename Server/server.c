#include "stems.h"
#include "request.h"

// 
// To run:
// 1. Edit config-ws.txt with following contents
//    <port number>
// 2. Run by typing executable file
//    ./server
// Most of the work is done within routines written in request.c
//

typedef struct {
	int fd;
	long time;
} data;        // To manage send() or recv() parameters.

typedef struct {
  data *queue;
  int front, rear;
} Queue;

int N, P;
Queue q;
pthread_t *thread;
sem_t mutex, empty, full;

// ----------------------------- Sentence :: Global variable ---------

void getargs_ws(int *port, int *P, int *N)
{
  FILE *fp;

  if ((fp = fopen("config-ws.txt", "r")) == NULL)
    unix_error("config-ws.txt file does not open.");

  fscanf(fp, "%d", port);
  fscanf(fp, "%d", P);
  fscanf(fp, "%d", N);
  fclose(fp);
}

void consumer(int connfd, long arrivalTime)
{
  requestHandle(connfd, arrivalTime);
  Close(connfd);
}

// ----------------------------------------------------------------------


void initQueue(Queue *q, int N) {
	q->front = q->rear = 0;
	q->queue = (data *)malloc(sizeof(q->queue)*N);
}

void enqueue(Queue *q, data send) {
	q->rear = (q->rear + 1) % N;
	q->queue[q->rear] = send;
}

data dequeue(Queue *q) {
	q->front = (q->front + 1) % N;
	return q->queue[q->front];
}

// -------------- Queue part -------------------------------------

void SendData(int connfd, long time) { // producer : main thread
	sem_wait(&empty);
	sem_wait(&mutex);

	data temp;
  temp.fd = connfd; 
  temp.time = time;
	enqueue(&q, temp);

	sem_post(&mutex);
	sem_post(&full);
}

void RecvData(int *connfd, long *time) { // consumer : worker thread
	sem_wait(&full);
	sem_wait(&mutex);

	data temp = dequeue(&q);
	*connfd = temp.fd;
  *time = temp.time;

	sem_post(&mutex);
	sem_post(&empty);
}

// ----------- thread func -----------------------------------------

/* 
  GetByProducer : Waiting for data in Queue.
  In while(1), if Queue have data, RecvData execute.
  But if Queue have not data, semaphore;empty will lock. 
  Therefore thread is waiting for enqueue func, 
  and will get data from dequeue.
*/

void *GetByProducer(void *ptr){ 
  int connfd;
  long time;
  RecvData(&connfd, &time);
  
  while(1){
    consumer(connfd, time);
    RecvData(&connfd, &time);
  }
}

// ------------- send & recv -------------------------------------

int main(void)
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  int status;

  initWatch();
  getargs_ws(&port, &P, &N);

  initQueue(&q, N);
  thread = (pthread_t *)malloc(sizeof(pthread_t)*P);

  sem_init(&mutex, 0, 1);
	sem_init(&full, 0, 0);
	sem_init(&empty, 0, N);

  listenfd = Open_listenfd(port);

  pid_t alarm_pid = Fork();
  
  if(alarm_pid == 0){
    char alarmClient[MAXLINE] = "./alarmClient";
    Execve(alarmClient,NULL,NULL);
  }  
  else {

    for(int i=0; i<P; i++){
      if(pthread_create(&thread[i], NULL, GetByProducer, NULL)){
        printf("pthread_create() :  function error.\n");
        exit(1);
      }
    }

    while (1) {
      clientlen = sizeof(clientaddr);
      connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
      SendData(connfd, getWatch());
    } 

    Wait(&status);

    for(int i=0;i<P;i++)
      pthread_detach(thread[i]);
  }
  return(0);
}
