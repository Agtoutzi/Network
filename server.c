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
#define threshold_l 0.6
#define threshold_h 0.8

time_t database_timestamps[database_size];
int vt;
int mt;
double vhrt;
double vhrt_old;
int ir_elements[database_size];


void send_ir(int sockfd){
  char *str;
  char str2[6];
  int number,i;
  
  for(i=0,number=0;i<database_size;i++){
    if(ir_elements[i]==1){number++;}
  }
  
  str=(char *)calloc(35,number*sizeof(char));
  
  strcpy(str,"IR: ");
  for(i=0;i<database_size;i++){
    if(ir_elements[i]==1){
      sprintf(str2, "%d ", i);
      strcat(str,str2);
      strcat(str,ctime(&(database_timestamps[i])));
    }
  }
  
  write(sockfd, str, strlen(str));
  printf("Invalidation report: %s",str);
  free(str);
}

void *change_ir(){
  int i;
  int sockfd=0;
  double L=5000000;
  struct sockaddr_in client_addr;
  
  sleep(20);
  
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0){
      printf("\n Error : Could not create socket \n");
      exit(0);
  }
  
  client_addr.sin_family = AF_INET;
  client_addr.sin_port = htons(5001);
  client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  if(connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr))<0){
    printf("\n Error : Connect Failed \n");
    exit(0);
  }
  
  while(1){
   vhrt_old=vhrt;
   if(mt!=0&&vt!=0){vhrt=(double)vt/(vt+mt);}
   printf("vt:%d  mt:%d \n",vt,mt);
   printf("vhrt: %f\n",vhrt);
   vt=0;
   mt=0;
   
   send_ir(sockfd);		//send ir
   
   for(i=0;i<database_size;i++){
    ir_elements[i]=0;
   }
   if(vhrt_old!=0.00){
    if(vhrt<=threshold_l){
     L=(vhrt/vhrt_old)*L;
    }else if(vhrt>threshold_h){
     L=(vhrt/vhrt_old)*L;
    }
   }
   printf("\nL: %lf\n",L);
   usleep(L);
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
    ir_elements[i]=1;
    printf("\t\t\t\titem %d changed at time: %s",i,ctime(&new_matrix[i]));
    sleep(5);
  }
}

int main(void){
  
  int listenfd = 0,n=0;
  int connfd=0;
  int i;
  struct sockaddr_in serv_addr;
 
  char sendBuff[150];
  char recvBuff[150];
  int val_element,req_element;
  char *timestamp;

  pthread_t change, dir;
  
  for(i=0;i<database_size;i++){
    database_timestamps[i]=time(NULL);
    ir_elements[i]=0;
  }
 
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
  
  if(listen(listenfd, 1) == -1){
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
      if(strcmp(strtok(ctime(&(database_timestamps[val_element])),"\n"),timestamp)<=0){
	vt++;
	strcpy(sendBuff, "Element valid\n\0");
      }else{
	strcpy(sendBuff, "Element invalid\0");
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

