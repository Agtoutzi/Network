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
#include <pthread.h>

//#define cache_hit_ratio 80
#define cache_size 4
#define elements 10

int cache[cache_size]={0,1,2,3};
char *cache_timestamps[cache_size];
int requests[elements]={0,0,0,0,0,0,0,0,0,0};


void change_cache(int element, char matrix[150]){
  int i;
  int min=0;
  for(i=0;i<cache_size;i++){
    printf("\t\t\t\t\titem: %d\tts %s\n",cache[i],cache_timestamps[i]);
  }
  for(i=1;i<cache_size;i++){
//    printf("\t\t\t\t\tcomparison: %d\n",strcmp(cache_timestamps[min],cache_timestamps[i]));
    if(strcmp(cache_timestamps[min],cache_timestamps[i])>=0){
      min=i;
    }
  }
  cache[min]=element;
  cache_timestamps[min]=strdup(matrix);
   
  printf("\t\t\t\tinserted in cache: item %d ",element);
  printf("with ts %s",cache_timestamps[min]);
  printf("new cache: ");
  for(i=0;i<cache_size;i++){
    printf("%d ",cache[i]);
  }
  printf("\n");
}

int main(void){
  int sockfd = 0;
  char recvBuff[150];
  char sendBuff[150];
  struct sockaddr_in serv_addr;
  time_t t;
  
  int reqElem;
  int i;
  int cache_flag;
  int cache_pos;
  char str[10];
 
  memset(recvBuff, '\0' ,sizeof(recvBuff));
  memset(sendBuff, '\0' ,sizeof(sendBuff));
  
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0){
      printf("\n Error : Could not create socket \n");
      return 1;
  }

  for(i=0;i<cache_size;i++){
//    t=time(NULL);
    cache_timestamps[i]=strdup("0");
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
      change_cache(reqElem,recvBuff);
    }else{	//send validation request
      strcpy(sendBuff, "Validation Request: ");
      sprintf(str, "%d ", reqElem);
      strcat(sendBuff, str);
      strcat(sendBuff, cache_timestamps[cache_pos]);
      write(sockfd, sendBuff, strlen(sendBuff));
      read(sockfd, recvBuff, sizeof(recvBuff)-1);
      for(i=0;i<cache_size;i++){
	  if(cache[i]==reqElem){break;}
      }
      if(strstr(recvBuff,"invalid")!=NULL){
	printf("Server message: %s",recvBuff);
	printf("\n");
	memset(sendBuff, '\0' ,sizeof(sendBuff));
	strcpy(sendBuff, "Data Request: ");
	sprintf(str, "%d", reqElem);
	strcat(sendBuff, str);
	write(sockfd, sendBuff, strlen(sendBuff));
	memset(recvBuff, '\0' ,sizeof(recvBuff));
	read(sockfd, recvBuff, sizeof(recvBuff)-1);
	cache_timestamps[i]=strdup(recvBuff);
      }else{
	t=time(NULL);
	cache_timestamps[i]=strdup(ctime(&t));
      }
    }
    
    printf("Server message: %s",recvBuff);
    printf("\n");
    memset(recvBuff, '\0' ,sizeof(recvBuff));
    memset(sendBuff, '\0' ,sizeof(sendBuff));
    sleep(2);
  }
 
  return 0;
}

