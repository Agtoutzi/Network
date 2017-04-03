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

#define database_size 10
 
clock_t database_timestamps[database_size]; 
 
int main(void)
{
  int listenfd = 0,connfd = 0,n=0;
  int i;
  struct sockaddr_in serv_addr;
 
  char sendBuff[30];
  char recvBuff[30];
  int val_element;
  clock_t timestamp;
  
  for(i=0;i<database_size;i++){database_timestamps[i]=0;}
  database_timestamps[1]=3;
 
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
      strcpy(sendBuff, "Data Reply");
      write(connfd, sendBuff, strlen(sendBuff));
    }else if(strstr(recvBuff,"Validation Request")!=NULL){
      strtok(recvBuff," ");
      strtok(NULL," ");
      val_element=atoi(strtok(NULL," "));
      timestamp=atoi(strtok(NULL," "));
      printf("\t\tval_element: %d\ttimestamp: %d",val_element,timestamp);
      if(database_timestamps[val_element]<=timestamp){
	strcpy(sendBuff, "Element valid");
      }else{
	strcpy(sendBuff, "Element invalid");
      }
      write(connfd, sendBuff, strlen(sendBuff));
    }
    printf("\n");
    memset(recvBuff, '\0' ,sizeof(recvBuff));
    memset(sendBuff, '\0' ,sizeof(sendBuff));

  }
  close(connfd);
  return 0;
}

