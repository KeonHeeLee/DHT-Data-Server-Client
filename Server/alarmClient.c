#include "stems.h"

void getargs_ps(char *hostname, int *port, char *filename, float *thresHold)
{
  FILE *fp;

  fp = fopen("config-ps.txt", "r");
  if (fp == NULL)
    unix_error("config-ps.txt file does not open.\n");

  
  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", filename);
  fscanf(fp, "%f", thresHold);
  fclose(fp);
}

void parseData(char *parsingData, char *sensorname, char *date, float *sensorValue){
    char *token, *temp_ptr;
    char value[MAXLINE];
    
    token = strtok(parsingData,"&");
    temp_ptr = token+5;
    strcpy(sensorname,temp_ptr);
  
    token = strtok(NULL,"&");
    temp_ptr = token+22;
    for(int i=0;i<8;i++)
      date[i] = *(temp_ptr + i);
    date[8] = '\0';
  
    token = strtok(NULL,"&");
    temp_ptr = token+6;
    strcpy(value,temp_ptr);

    *sensorValue = atof(value); 
}

int main(void){
    char hostname[MAXLINE], filename[MAXLINE];
    int port; 
    float thresHold;
 
    int pipe;

    getargs_ps(hostname,&port,filename,&thresHold);

    while(1){
        while((pipe = open(FIFO,O_RDWR))== -1){ 
            /* blocking For 'Not send case'. */
        }

        sleep(1);
        
        int len, nread;
        char parsingData[MAXLINE];

        while ( (nread = Read(pipe,&len,sizeof(int))) < 0 ){}
        while  ( (nread = Read(pipe,parsingData,len)) < 0 ){};

        Close(pipe);
        unlink(FIFO);

        // ------- get Data --------------------------------------------

        char sensorname[MAXLINE],date[MAXLINE];
        char buf[MAXLINE], sendData[MAXLINE];
        float sensorValue;
        char host[MAXLINE];
        char response[MAXLINE];

        strcpy(sendData,parsingData);
        parseData(parsingData,sensorname,date,&sensorValue);

        if(sensorValue > thresHold){
            int clientfd = Open_clientfd(hostname, port);

            Gethostname(host, MAXLINE);

            sprintf(buf,"POST %s HTTP/1.1\r\n",filename);
            sprintf(buf,"%sHost: %s:%d\r\n",buf,host,port);
            sprintf(buf,"%sConnection: keep-alive\r\n",buf);
            sprintf(buf,"%sContent-Length: %d\r\n",buf,len);
            sprintf(buf,"%s\r\n",buf);
            sprintf(buf,"%s%s",buf,sendData);
            Rio_writen(clientfd, buf, strlen(buf));

            read(clientfd, response, MAXLINE);
            printf("%s",response);
            Close(clientfd);
        }
    }


    return 0;
}