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
#include <wiringPi.h>
#include <string.h>
#include <stdint.h>
#include "stems.h"

#define MAXTIMINGS 83
#define DHTPIN 7
int dht11_dat[5] = {0, 0, 0, 0, 0};

void read_dht11_dat()
{
	while(1)
	{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;
	
	memset(dht11_dat, 0, sizeof(int) * 5);
	pinMode(DHTPIN, OUTPUT);
	digitalWrite(DHTPIN, LOW);
	delay(18);
	digitalWrite(DHTPIN, HIGH);
	delayMicroseconds(30);
	pinMode(DHTPIN, INPUT);
	for (i = 0; i < MAXTIMINGS; i++) {
		counter = 0;
		while (digitalRead(DHTPIN) == laststate) {
			counter++;
			delayMicroseconds(1);
			if (counter == 200) break;
		}
		laststate = digitalRead(DHTPIN);
		if (counter == 200) break; // if while breaked by timer, break for
		if ((i >= 4) && (i % 2 == 0)) {
			dht11_dat[j / 8] <<= 1;
			if (counter > 20) dht11_dat[j / 8] |= 1;
				j++;
			}
		}
		if ((j >= 40) && (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] +dht11_dat[3]) & 0xff))) {
			//printf("humidity = %d.%d %% Temperature = %d.%d *C \n", dht11_dat[0],dht11_dat[1], dht11_dat[2], dht11_dat[3]);
			break;
		}
	}
}

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
  sprintf(buf, "%sContent-Length: %d\n\r\n", buf, strlen(body));
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
    //printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    //printf("%s", buf);
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
void getargs_pi(char *hostname, int *port, char *filename, int *period)
{
  FILE *fp;
  fp = fopen("config-pi.txt", "r");
  if (fp == NULL)
    unix_error("config-pi.txt file does not open.");
  
  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", filename);
  fscanf(fp, "%d", period);
  fclose(fp);
}


int main(void)
{
  char hostname[MAXLINE], filename[MAXLINE], time[MAXLINE];
  char humi[MAXLINE], temp[MAXLINE];
  int port, period;
  float vhumi, vtemp;
  
  getargs_pi(hostname, &port, filename, &period);
  if (wiringPiSetup() == -1) {
    printf("Cannot set up wiringPi()\n");
    exit(1);
  }
  while(1) 
  {
    char buf[MAXLINE];
    getTime(time); 
    sprintf(buf,"Time : %s\n",time);
    read_dht11_dat();
	  
    delay(period/2 *1000);
    sprintf(humi,"%d.%d",dht11_dat[0],dht11_dat[1]); // humidity value
    vhumi=atof(humi);
    userTask("humidityPI", hostname, port, filename, time, vhumi);
    sprintf(buf,"%shumidity = %s *C\n",buf, humi);
    
    delay(period/2 *1000);
    sprintf(temp,"%d.%d",dht11_dat[2],dht11_dat[3]); // temperature value
    vtemp=atof(temp);
    userTask("temperaturePI", hostname, port, filename, time, vtemp);
    sprintf(buf,"%stemperature = %s%%\n",buf, temp);

    printf("Header: HTTP/1.0 200 OK\r\n");
    printf("Header: Server: My Web Server\r\n");
    printf("Header: Content-Length: %d\r\n",strlen(buf));
    printf("Header: Content-Type: text/plain\r\n");
    printf("%s\n",buf);
  }
 
  return(0);
}

