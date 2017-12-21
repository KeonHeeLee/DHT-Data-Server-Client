/*
 * clientPost.c: A very, very primitive HTTP client for sensor
 * 
 * To run, prepare config-cp.txt and try: 
 *      ./clientPost
 *
 * Sends one HTTP request to the specified HTTP server.
 * Get the HTTP response.
 */


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/time.h>
#include "stems.h"

void getTime(char *curTime){
  time_t ptm;
  struct tm *cur;
  char *week[7] = {"Sun","Mon","Tue","Wed","Tue","Fri","Sat"};
  char *mon[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  char day[3];
  char hour[3];
  char min[3];
  char sec[3];

  ptm = time(NULL);
  cur = localtime(&ptm);

  if(cur->tm_mday < 10) sprintf(day,"0%d",cur->tm_mday);
  else sprintf(day,"%d",cur->tm_mday);

  if(cur->tm_hour < 10) sprintf(hour,"0%d",cur->tm_hour);
  else sprintf(hour,"%d",cur->tm_hour);

  if(cur->tm_min < 10) sprintf(min,"0%d",cur->tm_min);
  else sprintf(min,"%d",cur->tm_min);

  if(cur->tm_sec < 10) sprintf(sec,"0%d",cur->tm_sec);
  else sprintf(sec,"%d",cur->tm_sec);

  sprintf(curTime,"%s, %s %s %d %s:%s:%s +0900",week[cur->tm_wday],day,mon[cur->tm_mon],
              (1900+cur->tm_year),hour,min,sec);
}

void clientSend(int fd, char *filename, char *body)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "POST %s HTTP/1.1\n", filename);
  sprintf(buf, "%sHost: %s\n", buf, hostname);
  sprintf(buf, "%sContent-Type: text/plain; charset=utf-8\n", buf);
  sprintf(buf, "%sContent-Length: %lu\n\r\n", buf, strlen(body));
  sprintf(buf, "%s%s\n", buf, body);
  Rio_writen(fd, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];  
  int n;
  
  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    /* If you want to look for certain HTTP tags... */
    // if (sscanf(buf, "Content-Length: %d ", &length) == 1)
    //   printf("Length = %d\n", length);
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

/* currently, there is no loop. I will add loop later */
void userTask(char *myname, char *hostname, int port, char *filename, char *time, float value)
{
  int clientfd;
  char msg[MAXLINE];

  sprintf(msg, "name=%s&time=%s&value=%f", myname, time, value);
  clientfd = Open_clientfd(hostname, port);
  clientSend(clientfd, filename, msg);
  clientPrint(clientfd);
  Close(clientfd);
}

void getargs_cp(char *myname, char *hostname, int *port, char *filename, float *value)
{
  FILE *fp;

  fp = fopen("config-cp.txt", "r");
  if (fp == NULL)
    unix_error("config-cp.txt file does not open.");

  
  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", filename);
  fscanf(fp, "%s", myname);
  fscanf(fp, "%f", value);
  fclose(fp);
}

void help(void)
{
  printf("---------------[clientPost shell commands]---------------\n");
  printf("-help: list available commands.\n");
  printf("-name: print current sensor name.\n");
  printf("-name <sensor>: change sensor name to <sensor>.\n");
  printf("-value: print current value of sensor.\n");
  printf("-value <n>: set sensor value to <n>.\n");
  printf("-send: send (current sensor name, time, value) to server.\n");
  printf("-quit: quit the program.\n");
  printf("---------------------------------------------------------\n");
}

int main(void)
{
  char myname[MAXLINE], hostname[MAXLINE], filename[MAXLINE], time[MAXLINE];
  int port, isExit = 0;
  float value;
  
  getargs_cp(myname, hostname, &port, filename, &value);

  printf("ClientPost shell Program is running. (version 0.1)\n");
  printf("If you don't know this shell commands, write 'help' command.\n");
  printf("made by K.H Lee & J.H Nam. (2017-10-16)\n\n");

  while(!isExit) {
    int tokenNum;
    char *token;
    char shell[MAXLINE];
    char delim[] = {" \t\n\r"};
    char *argv[MAX_TOKENS];
    int index;

    fputs("[clientPost@Shell v0.1] $ ",stdout);
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

    if(tokenNum == 0) continue; 

    if(strcasecmp("help",argv[0])==0) {
      if(tokenNum == 1) help();
      else 
        printf("help: Usage: help\n");
    }

    else if(strcasecmp("quit",argv[0])==0) {
      if(tokenNum == 1){
        printf("** The program will be terminated.\n");
        isExit = 1;
      } else
        printf("quit: Usage: quit\n"); 
    }

    else if(strcasecmp("name",argv[0])==0){
      if(tokenNum < 3){
        if(tokenNum == 2) { // name <transform name>
          strcpy(myname, argv[1]);
          printf("Sensor name is changed to '%s'\n", myname);
        } else {
          printf("Sensor name is %s\n",myname);
        }
      }
      else {
        printf("name: Usage1: name\n");
        printf("name: Usage2: name <Sensor>\n");
      }
    } 

    else if(strcasecmp("value",argv[0])==0) {
      if(tokenNum < 3){
        if(tokenNum == 2)	{
          float f = atof(argv[1]);
          value =f;
          printf("Sensor value is changed to %lf\n",value);
        }
        else
          printf("Current value of sensor is %lf\n",value);
      }
      else {
        printf("value: Usage1: value\n");
        printf("value: Usage2: value <n>");
      }
    } 
    else if(strcasecmp("send", argv[0])==0) {
      if(tokenNum == 1) {
        getTime(time);
        userTask(myname, hostname, port, filename, time, value);
      }	
      else printf("send: Usage: send\n");	
    } 
    else  // nothing
      printf("%s: Cannot find this command.\n",argv[0]);
  }

  return(0);
}
