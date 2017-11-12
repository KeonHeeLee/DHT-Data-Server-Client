#include "stems.h"

void parseData(char *buf, char *name, char *date, char *value){
    char *token, *temp_ptr;
    
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
}

int main(void){
    char buf[MAXLINE],msg[MAXLINE];
    char name[MAXLINE], date[MAXLINE], value[MAXLINE];

    strcpy(buf,getenv("PUSH_DATA"));
    parseData(buf,name,date,value);
    printf("HTTP/1.1 200 OK\r\n");

    sprintf(msg,"WARNING: %s로부터 %s 시각에 %s라는 값이 발생했습니다.\n",name,date,value);
    write(STDERR_FILENO,msg,strlen(msg));
    
    fflush(stdout);
    return 200;
}