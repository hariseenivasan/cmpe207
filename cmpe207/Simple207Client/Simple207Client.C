
/* Simple 207 client */

#include "../CMPE207Protocol/207Header.h"
#include "../CMPE207Protocol/dataParser.h"
#include "../CMPE207Protocol/CMPE207TCPLibrary.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   
   char recvline[1000];

   if (argc != 3)
   {
      printf("usage:  client <IP address> <portnumber>\n%d",argc);
      exit(1);
   }

   sockfd=create_socket(AF_207TCP, SOCK_207STREAM, IPPROTO_207TCP);
   if(sockfd<0)
       perror("Error creating socket");
   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr= inet_addr(argv[1]);
   servaddr.sin_port=htons(atoi(argv[2]));
  
   int i;
    for (i=0;i <=2;i++) 
   {
        printf("THIS IS NEW CLIENT CONNECTION: %d",i);
 //  pid_t childpid=fork();
 
 // if (childpid  == 0)
  {
   int rc = connect_socket(sockfd, (struct sockaddr_in *)&servaddr, sizeof(servaddr));
   if(rc<0)
       printf("Error Connect");
 
   char sendline[4000];//="hari";
   int i;
   for(i=0;i<4000;i++)
      sendline[i]=toascii((i%26)+97);
   
  // printf("\n%s\n",sendline);
      send_data(sockfd,sendline,sizeof(sendline),0);
      printf("\nData sent");
      char buffer[4000];
       int nr = recv_data(sockfd,buffer,sizeof(buffer),0);
 //printf("\nData Recievedfrom server: %d\n",nr);
 printf("\n%s",buffer);
 printf("\n\n");
      //n=recv_data(sockfd,recvline,1000,0,NULL,NULL);
     //recvline[n]=0;
    //  fputs(recvline,stdout);
  // }
 close_connection(sockfd);
 //exit(0);
  }
    }
}