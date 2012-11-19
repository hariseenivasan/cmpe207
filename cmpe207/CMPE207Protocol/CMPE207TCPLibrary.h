/* 
 * File:   CMPE207Header.h
 * Author: Rashmi
 *
 * Created on October 28, 2012, 6:37 PM
 */

#ifndef CMPE207TCPLIBRARY_H
#define	CMPE207TCPLIBRARY_H
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include "207Header.h"

#define BACKLOG 5
#define AF_207TCP 207
#define SOCK_207STREAM 2007
#define IPPROTO_207TCP 20007
#define HEADER_LEN_207TCP 20
#define MAXACTIVECONN 10

#ifdef	__cplusplus
extern "C" {
#endif
    
    // For accepted connections
 /*   static int activeConnections = 0;
    struct sockaddr_in acceptQ [MAXACTIVECONN]; 
    HDR_207 accIncConnections [MAXACTIVECONN];
    
    // For backlog
    static int pendingConnections = 0;
    struct sockaddr_in pendingQ [BACKLOG];
    HDR_207 pendingIncConnections [BACKLOG];
   /* 
    struct packet{
        header hdr;
        char msg[MAXPACKETSIZE - sizeof(struct header)];
    };*/


/*
 * Server Side -
 * 1. Create a 207socket 
 * 2. bind to a socket
 * 3. listen on a socket
 * 4. accept a connection 
 * 5. receive calls UDP recvfrom()
 * 6. send calls UDP sendto()
 *
 */

/*
 * Input Parameters -
 *  socket_family should be AF_207TCP
 *  socket_type should be SOCK_207STREAM
 *  protocol should be IPPROTO_207TCP
 * 
 * Output - 
 *  UDP socket descriptor
 */
int create_socket(int socket_family, int socket_type, int protocol);


/*
 * Input Parameters -
 *  sockfd is UDP socket number
 *  
 * Output - 
 *  bind a UDP port to a fixed port say 65000
 *  if bound successfully then return 0 else return -1
 */
int bind_socket(int sockfd, struct sockaddr_in * servaddr, socklen_t addrlen);


/*
 * Input Parameters -
 *  sockfd will be equal to UDP socket id
 *  backlog will be number of clients a server can accept an incoming connection from
 * 
 * Algorithm -
 *  Note : Since we will be dealing with UDP socket we cannot listen on this port
 *  
 *  1. Depending on backlog number allocate memory & initialize for header structures
 *  2. Do recvfrom() save struct sockaddr of this connection to tcp207Header
 *  3. send char* buffer received to (struct header*)parseheader(char *) function
 *  4. check if it has SYN flag set and nothing else is set if not discard this header
 *  5. copy the received structure to the queue
 *  
 */
int listen_socket(int sockfd, int backlog);


/*
 * Input Parameters - 
 *  sockfd will be equal to UDP socket id
 *  sockaddr will have client IP and port number of accepted connection copy from 
 *  tcp207Header
 * 
 * Algorithm- 
 * 1. Take first header from listen queue
 * 2. send SYN,ACK packet (sendto)
 * 3. Wait for ACK packet (recvfrom)
 * 4. create accept queue, save this client's tcp207Header
 * 5. Create a child UDP port
 * 6. return this new UDP port number
 * 
 */
    
int accept_connection(int sockfd, struct sockaddr_in *remoteaddr, socklen_t addrlen);

/**
 * 
 * @param sockfd Socket file discriptor
 * @param servaddr Server socket address structure pointer
 * @param len_servaddr Server socket address length
 * @return integer indicating the connection status.
 */
int connect_socket(int sockfd, struct sockaddr_in * servaddr, socklen_t len_servaddr);   

/*
 * Input parameters-
 *  sockfd is equal to CHILD socket number
 *  buf is data to be sent
 *  len is length of data 
 *  flags set to NOTUSED now
 */
ssize_t send_data(int sockfd, const void *buf, size_t len, int flags);

/*
 *
 */

ssize_t recv_data(int sockfd, void *buf, size_t len, int flags);


#ifdef	__cplusplus
}
#endif

#endif	/* CMPE207TCPLIBRARY_H */

