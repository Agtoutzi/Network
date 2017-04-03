#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#define cache_hit_ratio 80
#define cache_size 4
#define elements 10

int main(void){
  int sockfd = 0;
  char recvBuff[50];
  char sendBuff[50];
  struct sockaddr_in serv_addr;
  time_t t;
  int cache[cache_size]={0,1,2};
  clock_t cache_timestamps[cache_size]={0,0,0};
  int reqElem;
  int i;
  int cache_flag;
  int cache_pos;
  char str[10];
 
  memset(recvBuff, '\0' ,sizeof(recvBuff));
  memset(sendBuff, '\0' ,sizeof(sendBuff));
  
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
      printf("\n Error : Could not create socket \n");
      return 1;
    }
 
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(5000);
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
 

  if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){
    printf("\n Error : Connect Failed \n");
    return 1;
  }
  
  while(1){

    srand((unsigned) time(&t));
    reqElem=rand()%elements;
    
    cache_flag=0;
    for(i=0;i<cache_size;i++){if(cache[i]==reqElem){cache_flag=1;}}
    for(i=0;i<cache_size;i++){if(cache[i]==reqElem){cache_pos=i;}}
    
    printf("Request element: %d\n",reqElem);
      
    if(cache_flag==0){	//send data request
      strcpy(sendBuff, "Data Request: ");
      sprintf(str, "%d", reqElem);
      strcat(sendBuff, str);
      write(sockfd, sendBuff, strlen(sendBuff));
      read(sockfd, recvBuff, sizeof(recvBuff)-1);
    }else{	//send validation request
      strcpy(sendBuff, "Validation Request: ");
      sprintf(str, "%d", reqElem);
      strcat(sendBuff, str);
      sprintf(str, " %d", cache_timestamps[cache_pos]);
      strcat(sendBuff, str);
      write(sockfd, sendBuff, strlen(sendBuff));
      read(sockfd, recvBuff, sizeof(recvBuff)-1);
    }
    
    printf("Server message: %s",recvBuff);
    printf("\n");
    memset(recvBuff, '\0' ,sizeof(recvBuff));
    memset(sendBuff, '\0' ,sizeof(sendBuff));
    sleep(2);
  }
 
  return 0;
}

