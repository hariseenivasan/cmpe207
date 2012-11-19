/* 
 * File:   CMPE207Header.h
 * Author: root
 *
 * Created on October 28, 2012, 6:37 PM
 */

#ifndef CMPE207STRUCTS_H
#define	CMPE207STRUCTS_H
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#define MAXPACKETSIZE 300	//Defined for the UDP packet Size

#ifdef	__cplusplus
extern "C" {
#endif


         typedef struct tcp207Header{
//Unsigned because if the number exceeds 65535 then the value is in  negative so we can validate the port number
        unsigned short int sourcePort;		//2 bytes       
        unsigned short int destPort;		//2 bytes       

        unsigned int seqNum;          		//4 bytes
        unsigned int ackNum;          		//4 bytes
        unsigned short int hex10:10,         	//2 bytes 6 bits flags + 10 bits padding
              urg:1,
              ack:1,
              psh:1,
              rst:1,
              syn:1,
              fin:1;
        unsigned short int windowSize;       //2 bytes
        unsigned short int checksum;         //2 bytes
        unsigned short int urgPtr;          //2 bytes - used to store address, which is used by a pointer to denote urgent data; not used for now.
      //  struct sockaddr* udprecieve_sock; // Store the socket address from where it was recieved
    }HDR_207;
     
     
  /*  typedef struct tcp207Packet{
        header* hdr;
      
        //  char msg[MAXPACKETSIZE - sizeof(header)]; //Lets not use this, its little different, MAXPACKETSIZE is different. We have to implement WINDOW_SIZE and UDP_MAX_PACKET_SIZE, so it changes, its not constant. Implementation requires pointers
       
        char* msg;
        int msgLength;
        
    }packet;

 
 typedef struct tcp207Header HDR_207;
 typedef struct tcp207Packet PKT_207;
 */
  
 
#ifdef	__cplusplus
}
#endif

#endif	/* CMPE207STRUCTS_H */

