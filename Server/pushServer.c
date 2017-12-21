
#include "stems.h"

/*  time related functions */
enum {STATIC, DYNAMIC};

struct timeval startTime;

void initWatch(void)
{
  gettimeofday(&startTime, NULL);
}

double getWatch(void)
{
  struct timeval curTime;
  double tmp;

  gettimeofday(&curTime, NULL);
  tmp = (curTime.tv_sec - startTime.tv_sec) * 1000.0;
  return (tmp - (curTime.tv_usec - startTime.tv_usec) / 1000.0);
}
/* web request process routines */


void requestError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) 
{
  char buf[MAXLINE], body[MAXBUF];

  printf("Request ERROR\n");

  // Create the body of the error message
  sprintf(body, "<html><title>CS537 Error</title>");
  sprintf(body, "%s<body bgcolor=""fffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr>CS537 Web Server\r\n", body);

  // Write out the header information for this response
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  printf("%s", buf);

  sprintf(buf, "Content-Type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  printf("%s", buf);

  sprintf(buf, "Content-Length: %lu\r\n\r\n", (long unsigned int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  printf("%s", buf);

  // Write out the content
  Rio_writen(fd, body, strlen(body));
  printf("%s", body);

}

void requestReadhdrs(rio_t *rp, int *length)
{
  char buf[MAXLINE];

  *length = -1;
  Rio_readlineb(rp, buf, MAXLINE);
  while (strcmp(buf, "\r\n")) {
    sscanf(buf, "Content-Length: %d", length);
    Rio_readlineb(rp, buf, MAXLINE);
  }
  return;
}

int parseURI(char *uri, char *filename, char *cgiargs) 
{
  if (!strstr(uri, "cgi")) {
    // static
    strcpy(cgiargs, "");
    sprintf(filename, ".%s", uri);
    if (uri[strlen(uri)-1] == '/') {
      strcat(filename, "index.html");
    }
    return STATIC;
  } else {
    char *token;
    
    token=strtok(uri,"?");
    sprintf(filename,".%s",uri);
    
    if((token=strtok(NULL,"?"))!=NULL) {
      sprintf(cgiargs,"%s",token);
    } 
    
    else
      sprintf(cgiargs,"%s"," ");

    return DYNAMIC;
  }
}


void getargs_ps(int *port)
{
  FILE *fp;

  char hostname[MAXLINE];

  if ((fp = fopen("config-ps.txt", "r")) == NULL)
    unix_error("config-ps.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fclose(fp);
}

//
// Fills in the filetype given the filename
//
void requestGetFiletype(char *filename, char *filetype)
{
  if (strstr(filename, ".html")) 
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif")) 
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".jpg")) 
    strcpy(filetype, "image/jpeg");
  else 
    strcpy(filetype, "test/plain");
}

void requestServeStatic(int fd, char *method, char *filename, int filesize, double arrivalTime)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  // Rather than call read() to read the file into memory, 
  // which would require that we allocate a buffer, we memory-map the file
  requestGetFiletype(filename, filetype);
  srcfd = Open(filename, O_RDONLY, 0);
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  Close(srcfd);

  // put together response
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: My Web Server\r\n", buf);

  // Your statistics go here -- fill in the 0's with something useful!
  sprintf(buf, "%sStat-req-arrival: %lf\r\n", buf, arrivalTime);
  // Add additional statistic information here like above
  // ...
  //
  
  sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-Type: %s\r\n\r\n", buf, filetype);

  Rio_writen(fd, buf, strlen(buf));

  // Writes out to the client socket the memory-mapped file 
  Rio_writen(fd, srcp, filesize);
  Munmap(srcp, filesize);
}

void requestServeDynamic(rio_t *rio, int fd, char *method, char *filename, char *cgiargs, int bodyLength, double arrivalTime)
{
  pid_t pid = Fork();
  int status;
  char buf[MAXLINE];

  if(pid == 0){
    if(!strcasecmp(method,"POST")) {
      Rio_readrestb(rio,buf);
      buf[strlen(buf)-2] = '\0';
      Setenv("PUSH_DATA",buf,1);
      
      Dup2(fd,STDIN_FILENO);
      Dup2(fd,STDOUT_FILENO);
      Execve(filename,NULL,environ);
    } 
    else {
      printf("REQUEST_METHOD is not 'POST'!!\n");
      exit(0);
    }
  } 
  else 
    Wait(&status);
  
}


// handle a request
void requestHandle(int connfd, double arrivalTime)
{
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  rio_t rio;
  char filename[MAXLINE], cgiargs[MAXLINE];
  int reqType;
  struct stat sbuf;
  int bodyLength;

  Rio_readinitb(&rio, connfd);
  Rio_readlineb(&rio, buf, MAXLINE);
  sscanf(buf, "%s %s %s", method, uri, version);
  printf("%s %s %s\n", method, uri, version);
  
  requestReadhdrs(&rio, &bodyLength);
  reqType = parseURI(uri, filename, cgiargs);

  if ((strcasecmp(method,"POST")!=0)){
    requestError(connfd, method, "501", "Not Implemented",
		 "My Server does not implement this method");
    return;
  }

  if (stat(filename, &sbuf) < 0) {
    requestError(connfd, filename, "404", "Not found", "My Server could not find this file");
    return;
  }

  if (reqType == STATIC) {
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      requestError(connfd, filename, "403", "Forbidden", "My Server could not read this file");
      return;
    }
    requestServeStatic(connfd, method, filename, sbuf.st_size, arrivalTime);
  } else {
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
      requestError(connfd, filename, "403", "Forbidden", "My Server could not run this CGI program");
      return;
    }
    requestServeDynamic(&rio, connfd, method, filename, cgiargs, bodyLength, arrivalTime);
  }

}

void consumer(int connfd, long arrivalTime)
{
  requestHandle(connfd, arrivalTime);
  Close(connfd);
}


int main(void)
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;

  initWatch();
  getargs_ps(&port);

  listenfd = Open_listenfd(port);

  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    consumer(connfd, getWatch());
  }

  return(0);
}
