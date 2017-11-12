/*
 * clientGet.c: A very, very primitive HTTP client for console.
 * 
 * To run, prepare config-cg.txt and try: 
 *      ./clientGet
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * For testing your server, you will want to modify this client.  
 *
 * When we test your server, we will be using modifications to this client.
 *
 */


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include "stems.h"

/*
 * Send an HTTP request for the specified file 
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];  
  int length = 0;
  int n;
  
  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

/* currently, there is no loop. I will add loop later */
void userTask(char hostname[], int port, char webaddr[])
{
  int clientfd;

  clientfd = Open_clientfd(hostname, port);
  clientSend(clientfd, webaddr);
  clientPrint(clientfd);
  Close(clientfd);
}

void getargs_cg(char hostname[], int *port, char webaddr[])
{
  FILE *fp;

  fp = fopen("config-cg2.txt", "r");
  if (fp == NULL)
    unix_error("config-cg2.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", webaddr);
  fclose(fp);
}

// ------------------------ basic clientGet codes -----------------------

int cmd_HELP(int argc,char *argv[]){
  if(argc == 1) {
    printf("---------------[clientGet shell commands]----------------------\n");
    printf("LIST : Return sensorList table's sensor and name\n");   
    printf("INFO <sname> : sname's count and lately data\n");   
    printf("GET <sname> : sname's lately time and data\n");   
    printf("GET <sname> <n> : Reutrn the most recnt value and time of sname\n");   
    printf("quit / exit : finish clientGet\n"); 
    printf("----------------------------------------------------------------\n");
  } else 
    printf("%s : Usage : %s\n",argv[0],argv[0]);

  return 0;
}

int cmd_LIST(int argc, char *argv[], char *hostname, int port, char *webaddr){
  char sendArgs[MAXLINE];

  if(argc == 1){
    // LIST command execute.
    sprintf(sendArgs,"%s?command=%s",webaddr,argv[0]);

    userTask(hostname, port, sendArgs);
  } else 
    printf("%s : Usage : %s \n",argv[0],argv[0]);
  
  return 0;
}

int cmd_INFO(int argc, char *argv[], char *hostname, int port, char *webaddr){
  char sendArgs[MAXLINE];

  if(argc >= 2){
    // INFO <sname> execute.
    sprintf(sendArgs,"%s?command=%s",webaddr,argv[0]);
    for (int i = 2; i <= argc; i++)
      sprintf(sendArgs,"%s&name=%s",sendArgs,argv[i-1]);

    userTask(hostname, port, sendArgs);
  } else 
    printf("%s : Usage : %s <sname>\n",argv[0],argv[0]);

  return 0;
}

int cmd_GET(int argc, char *argv[], char *hostname, int port, char *webaddr){
  char sendArgs[MAXLINE];

  if (argc == 2){
    // GET <sname> execute.
    sprintf(sendArgs,"%s?command=%s",webaddr,argv[0]);
    sprintf(sendArgs,"%s&name=%s",sendArgs,argv[1]);

    userTask(hostname, port, sendArgs);
  } else if (argc == 3){
    // GET <sname> <n> execute.
    sprintf(sendArgs,"%s?command=%s",webaddr,argv[0]);
    sprintf(sendArgs,"%s&name=%s",sendArgs,argv[1]);
    sprintf(sendArgs,"%s&count=%s",sendArgs,argv[2]);

    userTask(hostname, port, sendArgs);
  } else {
    printf("%s : Usage1 : %s <sname>\n",argv[0],argv[0]);
    printf("%s : Usage2 : %s <sname> <n>\n",argv[0],argv[0]);
  }
  return 0;
}

int cmd_QUIT(int argc, char *argv[], pid_t ps_pid){
  if(argc > 1){
    printf("%s : Usage : %s\n",argv[0],argv[0]);
    return 0;
  }
  kill(ps_pid,SIGINT);

  return 1;
}

int cmdProcessing(char *hostname, int port, char *webaddr, pid_t ps_pid){
  int exitCode = 0;
  int tokenNum;
  char *token;
  char shell[MAXLINE];
  char delim[] = {" \t\n\r"};
  char *argv[MAX_TOKENS];
  int index;

  
  fputs("[clientGet@Shell v0.1] $ ",stdout);
  fgets(shell, MAXLINE, stdin);

  index = strlen(shell)-1;

  while(index){
    if((shell[index]==' ') || (shell[index]=='\t')) index--;
    else {
      shell[index]='\0';
      break;
    }
  }

  tokenNum = 0;

  token = strtok(shell, delim);
  while (token) {               
    argv[tokenNum++] = token;    
    token = strtok(NULL, delim);
  }
  argv[tokenNum] = NULL;

  if(tokenNum == 0) return 0;

  if(strcasecmp("HELP",argv[0])==0) exitCode = cmd_HELP(tokenNum,argv);
  else if((!strcasecmp("quit",argv[0]))||(!strcasecmp("exit",argv[0]))) exitCode = cmd_QUIT(tokenNum,argv,ps_pid);
  else if(!strcasecmp("LIST",argv[0])) exitCode = cmd_LIST(tokenNum,argv,hostname,port,webaddr);
  else if(!strcasecmp("INFO",argv[0])) exitCode = cmd_INFO(tokenNum,argv,hostname,port,webaddr);
  else if(!strcasecmp("GET", argv[0])) exitCode = cmd_GET(tokenNum,argv,hostname,port,webaddr); 
  else printf("%s: Cannot find this command.\n",argv[0]);

  return exitCode;
}

int main(void)
{
  pid_t ps_pid = Fork();
  char hostname[MAXLINE], webaddr[MAXLINE];
  int port;
  //int status;
  int isExit = 0;
  
  if(ps_pid == 0){ // child
    char push_server[MAXLINE] = "./pushServer";
    Execve(push_server,NULL,NULL);
  } else { // parent
    getargs_cg(hostname, &port, webaddr);

    printf("ClientGet shell Program is running. (version 0.1)\n");
    printf("If you don't know this shell commands, write 'help' command.\n");
    printf("And clientGet process execute pushServer.\n");
    printf("made by K.H Lee & J.H Nam. (2017-11-09)\n\n");

    while(!isExit){
      isExit = cmdProcessing(hostname, port, webaddr, ps_pid);
    }
  }
  printf("** clientGet & pushServer is terminated.\n");
  
  return(0);
}
