#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include "mysql.h"

//
// This program is intended to help you test your web server.
// You can use it to test that you are correctly having multiple 
// threads handling http requests.
//
// htmlReturn() is used if client program is a general web client
// program like Google Chrome. textReturn() is used for a client
// program in a embedded system.
//
// Standalone test:
// # export QUERY_STRING="name=temperature&time=3003.2&value=33.0"
// # ./dataGet.cgi

// ------------------------------------------------------------------------
void mysql_error_detect(MYSQL *conn){
  fprintf(stderr, "%s\n", mysql_error(conn));
  mysql_close(conn);
  exit(1);
}

void htmlReturn(void)
{
  char content[MAXLINE];
  char *buf;
  char *ptr;

  /* Make the response body */
  sprintf(content, "%s<html>\r\n<head>\r\n", content);
  sprintf(content, "%s<title>CGI test result</title>\r\n", content);
  sprintf(content, "%s</head>\r\n", content);
  sprintf(content, "%s<body>\r\n", content);
  sprintf(content, "%s<h2>Welcome to the CGI program</h2>\r\n", content);
  buf = getenv("QUERY_STRING");
  sprintf(content,"%s<p>Env : %s</p>\r\n", content, buf);
  ptr = strsep(&buf, "&");
  while (ptr != NULL){
    sprintf(content, "%s%s\r\n", content, ptr);
    ptr = strsep(&buf, "&");
  }
  sprintf(content, "%s</body>\r\n</html>\r\n", content);
  
  /* Generate the HTTP response */
  printf("Content-Length: %lu\r\n", strlen(content));
  printf("Content-Type: text/html\r\n\r\n");
  printf("%s", content);
  fflush(stdout);
}

// --------- Currently, We don't use this part. But It will be used htmlReturn() ----

void textReturn(int *argc, char *argv[])
{
  char content[MAXLINE];
  char *buf;
  char *ptr;
  int index = 0;

  buf = getenv("QUERY_STRING");
  sprintf(content,"%sEnv : %s\n", content, buf);
  ptr = strsep(&buf, "&");
  while (ptr != NULL){
    sprintf(content, "%s%s\n", content, ptr);
    argv[index] = ptr;
    ptr = strsep(&buf, "&");
    index++;
  }

  /* Generate the HTTP response */
  //printf("Content-Length: %lu\n", strlen(content));
  //printf("Content-Type: text/plain\r\n\r\n");
 // printf("%s", content);
  //fflush(stdout);

  *argc = index;
}

#define DIFFMIN 9999

void getLIST(MYSQL *conn, int argc, char *argv[],char *content)
{
  MYSQL_RES *sql_result;
  MYSQL_ROW row;

  if(mysql_query(conn,"SELECT * FROM sensorList"))
    mysql_error_detect(conn);
  
  if((sql_result = mysql_store_result(conn))==NULL)
    mysql_error_detect(conn);

    while((row = mysql_fetch_row(sql_result)))
      sprintf(content,"%s%s ",content,row[0]);
    sprintf(content,"%s\n",content);
}

void getINFO(MYSQL *conn, int argc, char *argv[],char *content)
{
  for(int i=1;i<argc;i++){
    MYSQL_RES *sql_result_slist, *sql_result_sensor;
    MYSQL_ROW row_slist, row_sensor;

    float min = DIFFMIN ,max = 0,averate = 0;
    char name[MAXLINE];
    char sensorTable[MAXLINE] = "sensor";
    char sensorNum[5];
    int sensor_num = 1;
    int isEnd = 1;
    char query[MAXLINE];
    int n = 0;

    sscanf(argv[i],"name=%s",name);

    if(mysql_query(conn,"SELECT * FROM sensorList"))
      mysql_error_detect(conn);

    if((sql_result_slist = mysql_store_result(conn))==NULL)
      mysql_error_detect(conn);

    while((row_slist = mysql_fetch_row(sql_result_slist))){
      if(!strcasecmp(row_slist[0],name)){ 
        isEnd = 0;
        break;
      }
      sensor_num++;
    }
    if(isEnd == 1){
      sprintf(content,"** Cannot find sensor name;%s\n",name);
    } else {

      sprintf(content,"name\tid\tcount\tmin\taverate\tmax\n");
      if(sensor_num <10){
        sprintf(sensorNum,"0%d",sensor_num);
      } else {
        sprintf(sensorNum,"%d",sensor_num);
      }
      strcat(sensorTable,sensorNum);

      sprintf(query,"SELECT * FROM %s",sensorTable);
      if(mysql_query(conn,query))
        mysql_error_detect(conn);

      if((sql_result_sensor = mysql_store_result(conn))==NULL)
        mysql_error_detect(conn);

      while((row_sensor = mysql_fetch_row(sql_result_sensor))){
        float temp = atof(row_sensor[1]);
        averate += temp;
        
        if(temp < min) min = temp;
        if(temp > max) max = temp;
        
        n++;
      }
      averate = averate / n;
    }
    sprintf(content,"%s%s\t%d\t%s\t%.3f\t%.3f\t%.3f\n",content,name,sensor_num,row_slist[2],min,averate,max);
  }
}

void getGET(MYSQL *conn, int argc, char *argv[], char *content)
{
  MYSQL_RES *sql_result_slist, *sql_result_sensor;
  MYSQL_ROW row_slist, row_sensor;
  char name[MAXLINE];
  int count; 
  char sensorTable[MAXLINE] = "sensor";
  char sensorNum[5];
  int sensor_num = 1;
  int  isEnd = 1;
  char query[MAXLINE];
  int tableIndex = 0;

  sscanf(argv[1],"name=%s",name);

  if(mysql_query(conn,"SELECT * FROM sensorList"))
    mysql_error_detect(conn);
    
  if((sql_result_slist = mysql_store_result(conn))==NULL)
    mysql_error_detect(conn);

  while((row_slist = mysql_fetch_row(sql_result_slist))){
    if(!strcasecmp(row_slist[0],name)){ 
      isEnd = 0;
      break;
    }
    sensor_num++;
  }
  if(isEnd == 1){
    sprintf(content,"** Cannot find sensor name;%s\n",name);
  } else {
    if(sensor_num <10){
      sprintf(sensorNum,"0%d",sensor_num);
    } else {
      sprintf(sensorNum,"%d",sensor_num);
    }
    strcat(sensorTable,sensorNum);
    sprintf(query,"SELECT * FROM %s ORDER BY num DESC",sensorTable);

    if(mysql_query(conn,query))
      mysql_error_detect(conn);

    if((sql_result_sensor = mysql_store_result(conn))==NULL)
      mysql_error_detect(conn);

    if(argc == 2){
        row_sensor = mysql_fetch_row(sql_result_sensor);
        sprintf(content,"%s%s\t%s\n",content,row_sensor[0],row_sensor[1]);
    } else { // add n option
        sscanf(argv[2],"count=%d",&count);         
        while((row_sensor = mysql_fetch_row(sql_result_sensor))){
            if(count < tableIndex) break;
	          sprintf(content,"%s%s\t%s\n",content,row_sensor[0],row_sensor[1]);
            tableIndex++;
        }
    }
  }
}

void getDB(int argc, char *argv[],char *content)
{
  MYSQL *conn = mysql_init(NULL); 
  
  char password[MAXLINE] = "root";
    
  if(conn==NULL){
    fprintf(stderr,"%s\n",mysql_error(conn));
    exit(1);
  }
  
  if(mysql_real_connect(conn,"localhost","root",password,NULL,0, NULL, 0) == NULL)
    mysql_error_detect(conn);
  
  if(mysql_query(conn, "USE SENSORS"))
    mysql_error_detect(conn);
  
  if(!strcasecmp("command=LIST",argv[0])) getLIST(conn,argc,argv,content);
  else if(!strcasecmp("command=INFO",argv[0])) getINFO(conn,argc,argv,content);
  else /*!strcasecmp("command=GET", argv[0])*/ getGET(conn,argc,argv,content);

  mysql_close(conn);
}

#define MAXLINE_ARGS 80

int main(void)
{
  int argc;
  char *argv[MAXLINE_ARGS];
  char content[MAXLINE];
  //htmlReturn();
  textReturn(&argc, argv);
  getDB(argc,argv,content);

  printf("HTTP/1.1 200 OK\r\n");
  printf("Server: My Web Server\r\n");
  printf("Content-Length: %lu\r\n",strlen(content));
  printf("Content-Type: text/plain\r\n\r\n");
  printf("%s",content);
  
  fflush(stdout);
  return(0);
}
