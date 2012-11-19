
/* Simple 207 client */

#include "../CMPE207Protocol/207Header.h"
#include "../CMPE207Protocol/dataParser.h"
#include "../CMPE207Protocol/CMPE207TCPLibrary.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
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

   int rc = connect_socket(sockfd, (struct sockaddr_in *)&servaddr, sizeof(servaddr));
   if(rc<0)
       printf("Error Connect");
   //printf("Connected veetuku polam: %s",sendline);
   //int filefd = open(argv[2], O_RDONLY);
   //Lets read only 1000 for now and send
  // cnt = read(filefd, sendline, 1000);
   //printf("Data to be sent: %s",sendline);
   
  // while (fgets(sendline, 10000,stdin) != NULL)
  // {
   char* sendline="hari";
      send_data(sockfd,sendline,10,0);
      printf("\nData sent");
      char buffer[10];
       int nr = recv_data(sockfd,buffer,10,0);
  printf("\nData Recievedfrom server: %d\n%s",nr,buffer);
 
   //   n=recv_data(sockfd,recvline,1000,0,NULL,NULL);
    //  recvline[n]=0;
    //  fputs(recvline,stdout);
  // }
}
