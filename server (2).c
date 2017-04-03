#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

#define database_size 10
 
time_t database_timestamps[database_size];
int vt;
int mt;
double vhrt;

void *change_ir(){
  sleep(20);
  
  while(1){
   vhrt=(double)vt/(vt+mt);
   printf("vt:%d  mt:%d \n",vt,mt);
   printf("vhrt: %f\n",vhrt);
   vt=0;
   mt=0;
   sleep(10+vhrt);
  }
}


void *change_database(void *matrix){
  int i;
  time_t *new_matrix = (time_t *)matrix;
  time_t t;
  
  while(1){
    srand((unsigned) time(&t));
    i=rand()%database_size;
    new_matrix[i]=time(NULL);
    printf("\t\t\t\titem %d changed at time: %s",i,ctime(&new_matrix[i]));
    sleep(5);
  }
}

int main(void){
  
  int listenfd = 0,connfd = 0,n=0;
  int i;
  struct sockaddr_in serv_addr;
 
  char sendBuff[150];
  char recvBuff[150];
  int val_element,req_element;
  char *timestamp;

  pthread_t change, dir;
  
  for(i=0;i<database_size;i++){database_timestamps[i]=time(NULL);}
 
  pthread_create(&change, NULL, change_database, database_timestamps);
  pthread_create(&dir, NULL, change_ir, NULL);
 
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  
  memset(&serv_addr, '\0', sizeof(serv_addr));
  memset(sendBuff, '\0', sizeof(sendBuff));
  memset(recvBuff, '\0', sizeof(recvBuff));
  
  serv_addr.sin_family = AF_INET;    
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  serv_addr.sin_port = htons(5000);    
 
  bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
  
  if(listen(listenfd, 10) == -1){
      printf("Failed to listen\n");
      return -1;
  }

  connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);
      
  while((n = read(connfd, recvBuff, sizeof(recvBuff))) > 0){

    printf("Client message: %s", recvBuff);
    if(strstr(recvBuff,"Data Request")!=NULL){
      mt++;
      strtok(recvBuff," ");
      strtok(NULL," ");
      req_element=atoi(strtok(NULL," "));
      strcpy(sendBuff, ctime(&(database_timestamps[req_element])));
      write(connfd, sendBuff, strlen(sendBuff));
    }else if(strstr(recvBuff,"Validation Request")!=NULL){
      strtok(recvBuff," ");
      strtok(NULL," ");
      val_element=atoi(strtok(NULL," "));
      timestamp=strdup(strtok(NULL,"\n"));
      printf("\t\t\t\t\tcomparison: %d\n",strcmp(strtok(ctime(&(database_timestamps[val_element])),"\n"),timestamp));
      if(strcmp(strtok(ctime(&(database_timestamps[val_element])),"\n"),timestamp)<=0){
	vt++;
	strcpy(sendBuff, "Element valid\n");
      }else{
	strcpy(sendBuff, "Element invalid");
      }
      write(connfd, sendBuff, strlen(sendBuff));
      free(timestamp);
    }
    printf("\n");
    memset(recvBuff, '\0' ,sizeof(recvBuff));
    memset(sendBuff, '\0' ,sizeof(sendBuff));
  }
  close(connfd);
  return 0;
}

