/* Simple 207 server */

#include "../CMPE207Protocol/207Header.h"
#include "../CMPE207Protocol/dataParser.h"
#include "../CMPE207Protocol/CMPE207TCPLibrary.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

int main(int argc, char**argv)
{
  int msock; /* master socket */
  int ssock; /* slave socket */
  int portno; /* port to listen on */
  socklen_t clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
 
 
  char buf[400]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  msock = create_socket(AF_207TCP, SOCK_207STREAM, IPPROTO_207TCP);
  if (msock < 0)
    perror("ERROR opening socket");

  bzero((char *) &serveraddr, sizeof(serveraddr));

  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);
  if (bind_socket(msock, (struct sockaddr_in *) &serveraddr,
       sizeof(serveraddr)) < 0)
    perror("ERROR on binding");
 
 
 
  while (1) {
     
      if (listen_socket(msock, 5) < 0) 
                perror("ERROR on listen");
 
  clientlen = sizeof(clientaddr);
  ssock = accept_connection(msock, (struct sockaddr_in *) &clientaddr, clientlen);
  if (ssock < 0)
    perror("ERROR on accept");
 
  pid_t childpid=fork();
 
  if (childpid  == 0)
  {
      //close(ssock);
      //int abc;
      //scanf("%d",&abc);
      char buffer[4000],converted[4000];
      int nr = recv_data(ssock,buffer,sizeof(buffer),0);
     
      int i;
      for(i=0;i<nr;i++)
      converted[i]=  toupper(buffer[i]);
      printf("\nConverted %s",converted);
      printf("\n\n");
      send_data(ssock,converted,sizeof(converted),0);
      //close_connection(ssock);
            nr = recv_data(ssock,buffer,sizeof(buffer),0);
     exit(0);      
  }
  }
 
 // int i;
 // for(i=0;i<3000;i++)
 // printf("\nData Recieved: %d\n%s",buffer[i]);

}