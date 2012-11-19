
/* Simple 207 server */

#include "../CMPE207Protocol/207Header.h"
#include "../CMPE207Protocol/dataParser.h"
#include "../CMPE207Protocol/CMPE207TCPLibrary.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <ctype.h>

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
  if (listen_socket(msock, 5) < 0) /* allow 5 requests to queue up */ 
    perror("ERROR on listen");
  clientlen = sizeof(clientaddr);
  
  ssock = accept_connection(msock, (struct sockaddr_in *) &clientaddr, clientlen);
  if (ssock < 0) 
    perror("ERROR on accept");
  char buffer[10],converted[10];
  int nr = recv_data(ssock,buffer,10,0);
  printf("\nData Recieved: %d\n%s",nr,buffer);
  int i;
  for(i=0;i<nr;i++)
    converted[i]=  toupper(buffer[i]);
  printf("\nConverted %s",converted);
  send_data(ssock,converted,10,0);
 // toupper();
//  close(ssock);
}