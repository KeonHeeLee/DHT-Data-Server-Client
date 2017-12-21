#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include "mysql.h"

//
// This program is intended to help you test your web server.
// 
void mysql_error_detect(MYSQL *conn){
  fprintf(stderr, "%s\n", mysql_error(conn));
  mysql_close(conn);
  exit(1);
}

void insertToMysql(MYSQL *conn,char *name, char *value, char *date){
  int sensor_num=1;
  char sensorTable[MAXLINE] = "sensor";
  char sensorNum[3];
  char insert_query[MAXLINE];
  char sensorList_query[MAXLINE];
  MYSQL_RES *sql_result;
  MYSQL_ROW row;
  int isEnd = 1;

  if(mysql_query(conn,"SELECT * FROM sensorList")){
    mysql_query(conn,"CREATE TABLE sensorList ( name varchar(80) not null, id int(2) not null AUTO_INCREMENT, count int(3) not null, PRIMARY KEY (id) )AUTO_INCREMENT=1;");
    mysql_query(conn,"SELECT * FROM sensorList");
  }

  if((sql_result = mysql_store_result(conn))==NULL)
    mysql_error_detect(conn);

  while((row = mysql_fetch_row(sql_result))){
    if(!strcasecmp(row[0],name)){ 
      isEnd = 0;
      break;
    }
    sensor_num++;
  }
  if(sensor_num <10){
    sprintf(sensorNum,"0%d",sensor_num);
  } else {
    sprintf(sensorNum,"%d",sensor_num);
  }
  strcat(sensorTable,sensorNum);

  if(isEnd == 0){
    sprintf(insert_query,"INSERT INTO %s VALUES('%s',%s, null)",sensorTable,date,value);
    if(mysql_query(conn,insert_query))
      mysql_error_detect(conn);

    sprintf(sensorList_query,"UPDATE sensorList set count = count+1 WHERE name = '%s'",name);
    if(mysql_query(conn,sensorList_query))
      mysql_error_detect(conn);
  } else { // isEnd == 1
    char newTable[MAXLINE];
    sprintf(newTable,"CREATE TABLE %s( time varchar(12) not null,data float not null,num int(3) not null auto_increment,PRIMARY KEY (num) )AUTO_INCREMENT=1;",sensorTable);
    if(mysql_query(conn,newTable))
      mysql_error_detect(conn);

    sprintf(insert_query,"INSERT INTO %s VALUES('%s',%s, null)",sensorTable,date,value);
    if(mysql_query(conn,insert_query))
      mysql_error_detect(conn);

    char newSensor[MAXLINE];
    sprintf(newSensor,"INSERT INTO sensorList VALUES('%s',id,1)",name);
    if(mysql_query(conn,newSensor))
      mysql_error_detect(conn);
  }
}

int main(int argc, char *argv[])
{
  int size;
  char *buf;
  char pipeArgs[MAXLINE];
  char len[MAXLINE];

  sprintf(len, "%s", getenv("CONTENT_LENGTH"));
  size = atoi(len);
  buf = (char *)malloc(sizeof(char)*size);

  printf("HTTP/1.0 200 OK\r\n");
  printf("Server: My Web Server\r\n");
  printf("Content-Length: %s\r\n",len);
  printf("Content-Type: text/plain\r\n\r\n");

  if(size >= MAXLINE) {
    char *rest_buf = (char *)malloc(sizeof(char)*(size-MAXLINE));
    sprintf(buf, "%s", getenv("CONTENT_IN_BUF"));
    Read(STDIN_FILENO,rest_buf,size-MAXLINE);
    strcat(buf,rest_buf);
    free(rest_buf);
  }
  else 
    sprintf(buf, "%s", getenv("CONTENT_IN_BUF"));
  
  printf("... Sending %s\n",buf);
  strcpy(pipeArgs,buf);

  // ----------------------------------- connecting database ------

  char *token;
  char *temp_ptr;

  char name[MAXLINE],value[MAXLINE],date[MAXLINE];

  MYSQL *conn = mysql_init(NULL); 

  char password[MAXLINE] = "root";
  
  if(conn==NULL){
	  fprintf(stderr,"%s\n",mysql_error(conn));
	  exit(1);
  }

  if(mysql_real_connect(conn,"localhost","root",password,NULL,0, NULL, 0) == NULL)
    mysql_error_detect(conn);

  if(mysql_query(conn, "USE SENSORS")){
    mysql_query(conn,"CREATE DATABASE SENSORS");
    mysql_query(conn, "USE SENSORS");
  }

  token = strtok(buf,"&");
  temp_ptr = token+5;
  strcpy(name,temp_ptr);

  token = strtok(NULL,"&");
  temp_ptr = token+22;
  for(int i=0;i<8;i++)
    date[i] = *(temp_ptr + i);
  date[8] = '\0';

  token = strtok(NULL,"&");
  temp_ptr = token+6;
  strcpy(value,temp_ptr);
  //value[strlen(value)-2] = '\0';

  insertToMysql(conn,name,value,date);

  mysql_close(conn);

  // ------------------------------------------------ connecting pipe -----------------

  int pipe;
  int pipeLength = strlen(pipeArgs);
  int nwrite;

  if(mkfifo(FIFO,0666) == -1){
    if(unlink(FIFO)==-1){
      printf("unlink() : Dosen't delete %s\n",FIFO);
      exit(0);
    }

    if(mkfifo(FIFO,0666) == -1){
      printf("mkfifo() : Cannot make pipe.\n");
      exit(0);
    }
    pipe = Open(FIFO,O_RDWR,O_TRUNC);
 
  } 
  else 
    pipe = Open(FIFO,O_RDWR,O_TRUNC);
 
  if((nwrite = Write(pipe,(int *)&pipeLength,sizeof(int))) < 0) {
    printf("Write : Cannot write in pipe(%s).\n",FIFO);
    exit(0);
  }
  if((nwrite = Write(pipe,pipeArgs,pipeLength)) < 0 ) {
    printf("Write : Cannot write in pipe(%s).\n",FIFO);
    exit(0);
  }

  sleep(1);
  Close(pipe);
  
  fflush(stdout);
  return 200;
}
