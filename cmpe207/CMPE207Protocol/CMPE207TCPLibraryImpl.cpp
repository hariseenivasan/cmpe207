/* 
 * File:   library implementation
 * Author: Rashmi, Hari, Abhinav, Renoy
 *
 * Created on October 28, 2012, 6:27 PM
 */

#include <cstdlib>
#include <netinet/in.h>
#include "CMPE207TCPLibrary.h"
#include<stdio.h>
#include <map>
#include <queue>
#include "dataParser.h"
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include<iterator>
#include <unistd.h>
#include<signal.h>

#define NUMBER_OF_RETRANSMISSION 2
#define FIN_SENT 0x4
#define RECV_WAIT 5

using namespace std;
 struct connection{

    HDR_207 hdr207;
    struct sockaddr_in cliadd;
};

static queue<connection> connections;
static map<int,connection> hmap;
static map<int,char *> buffer_207; //Used for sending and reciving
const unsigned short buffer_207_size=0x7D0; //2K per map this indicates the segment size
static connection connected_node;
static unsigned short host_window_size; // This indicates the host window size.
int wrapper_recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen, int time_out);

int recv_check_fin(int sockfd, void *buf, size_t len, int flags, sockaddr* node_address, socklen_t* node_address_len);
int close_i(int sockfd, HDR_207* fin_packet);
//To calculate RTT
double t_sent;  
double t_recv; 

/**
 This manages port number for the connections,
 * 207header has different port number from UDP port number.
 * 207header's port number is just a dummy number.
 * SHOULD BE CALLED ONLY IN BIND & CONNECT
 */
unsigned short int  mapUDPport(unsigned short int headerserverport)
{
    
    if(headerserverport<=0x400)
        return 0x400+0x400;
    
    //Check if the maximum port number is given
    if(headerserverport>=(0xFFFF-0x64))
            return headerserverport-100;
    else
            return headerserverport+100;
    
}

void copyHeader(HDR_207* desthdr,const HDR_207* srchdr){
    desthdr->sourcePort =  srchdr->sourcePort ;
          desthdr->destPort =srchdr->destPort;     
          desthdr->seqNum =srchdr->seqNum;     
          desthdr->ackNum =srchdr->ackNum;           
         desthdr->hex10 =srchdr->hex10;          
               desthdr->urg =srchdr->urg;     
               desthdr->ack =srchdr->ack;     
               desthdr->psh =srchdr->psh;     
               desthdr->rst =srchdr->rst;     
               desthdr->syn =srchdr->syn;     
               desthdr->fin =srchdr->fin;     
          desthdr->windowSize =srchdr->windowSize;            
         desthdr->checksum =srchdr->checksum;              
          desthdr->urgPtr =srchdr->urgPtr    ;
}
void resetHeaderFlags(HDR_207* hdr){
    hdr->ack=0;
    hdr->psh=0; 
    hdr->syn=0; 
    hdr->rst=0;
    hdr->fin=0;
    hdr->urg=0;
}

void printheader(HDR_207 * hdr)
{
    if(hdr!=NULL)
        printf("\nSRC PORT:%d\tDEST PORT:%d \nSEQ NO.:%d\nACK NO.:%d\nHEX10:%d URG:%d ACK:%d PSH:%d RST:%d SYN:%d FIN:%d\tWND SIZE:%d\nCHECKSUM:%d\tURG PTR:%d\n", hdr->sourcePort,
          hdr->destPort,     
          hdr->seqNum,
          hdr->ackNum,      
         hdr->hex10,     
               hdr->urg,
               hdr->ack,
               hdr->psh,
               hdr->rst,
               hdr->syn,
               hdr->fin,
          hdr->windowSize,       
         hdr->checksum,         
          hdr->urgPtr   );
    else
        printf("\nHeader is Null Cannot print header\n");
}

/*
 * Input Parameters -
 *  socket_family should be AF_207TCP
 *  socket_type should be SOCK_207STREAM
 *  protocol should be IPPROTO_207TCP
 * 
 * Output - 
 *  UDP socket descriptor
 */
int create_socket(int socket_family, int socket_type, int protocol) {

  
   
    int sock_id;
    if (socket_family == AF_207TCP && socket_type == SOCK_207STREAM && protocol == IPPROTO_207TCP) 
            sock_id = socket(AF_INET, SOCK_DGRAM, 0); // Create UDP socket
    else
            sock_id = -1;
    
    return sock_id;

}

/*
 * Input Parameters -
 *  sockfd is UDP socket number
 *  
 * Output - 
 *  bind a UDP port by adding 100 to user defined port
 *  if bound successfully then return 0 else return -1
 */
int bind_socket(int sockfd, struct sockaddr_in * servaddr, socklen_t addrlen) {

    struct sockaddr_in server;
    int status = -1;
    if(servaddr != NULL) {
        memcpy(&server, servaddr, sizeof(servaddr));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(mapUDPport(ntohs(servaddr->sin_port)));
        printf("Bind: %d\n",ntohs(server.sin_port));
        status = bind (sockfd, (struct sockaddr *)&server, sizeof(server));        
    }
    
    return status;
    
}

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
int listen_socket(int sockfd, int backlog) {

    char buf[HEADER_LEN_207TCP];
    struct sockaddr_in cliaddr;
    HDR_207 * hdr=newHDR_PTR();


    socklen_t clientlen = sizeof(cliaddr);
    int n = recvfrom(sockfd, &buf, HEADER_LEN_207TCP,0, (struct sockaddr *)& cliaddr, &clientlen);
    printf("Obtained Client connection %d",n);
    
    if ( n> 0 ) {
        int rc=get_hdr_dat_frm_Buff(hdr,NULL,(const char*)&buf,sizeof(buf));
    
        printheader(hdr);
        printf("Get header data from buffer %d",rc );
        if(rc==0){
            if(hdr->syn && !hdr->ack && !hdr->rst && !hdr->urg && !hdr->fin){
                connection con;
                con.cliadd=cliaddr;
                
               copyHeader(&con.hdr207,hdr);
               // con.hdr207=hdr;
                printf("Pushing inside queue");
                connections.push(con);
                
            }
            else
            {
                fprintf(stderr,"the packet must be sync packet");
                hdr->rst=1;
                create_UDP_dat_ptr(buf,NULL,0,hdr);
                sendto(sockfd,buf,sizeof(buf),0,(struct sockaddr*)&cliaddr,sizeof(cliaddr));
                printf("Not valid SYN");
                return -1;
            }
            printf("Connections in queue:%d", connections.size());
        }

        
        //pendingQ [pendingConnections] = cliaddr;
        // call header parse function
        // pendingIncConnections [pendingConnections] =  parsed 207 header
        //pendingConnections++;
    }
    
}

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
    
int seqnumber;

int accept_connection(int sockfd, struct sockaddr_in *remoteaddr, socklen_t addrlen) {
    // CHECK!! When you reset, you must have poped out the client from listen queue.
    printf("Accept Entered");
    unsigned  int sequenceNumber=( unsigned  int)rand(); //make it random afterwards
//    int i = 0;
//    acceptQ [activeConnections] = pendingQ[i];
//    activeConnections ++;
    char sendbuff[20],ackbuff[20];
    char buf[20];
    int portno;
    HDR_207 * syn_ack=newHDR_PTR();
    HDR_207 * hdr=newHDR_PTR();
    

    struct sockaddr_in serveraddr;
    socklen_t len;
    connection con;
    int sockf;

    struct sockaddr_in clientaddr;
    printf("Accept connections: %d",connections.empty());
    if(!connections.empty())
    {
        printf("\ninside connection queue\n");
        con=connections.front();
        connections.pop();
        printf("\n connection header\n");
        HDR_207 hdrfromque = con.hdr207;
        printheader(&hdrfromque);
        clientaddr=con.cliadd;
        printf("%s\n",inet_ntoa(clientaddr.sin_addr));
        syn_ack->syn=1;
        syn_ack->ack=1;
        syn_ack->ackNum=con.hdr207.seqNum+1;
        
        syn_ack->seqNum=sequenceNumber;
        
        syn_ack->sourcePort=con.hdr207.destPort;
        syn_ack->destPort=con.hdr207.sourcePort;
        syn_ack->windowSize = buffer_207_size*5;
        
        printf("syn ack packet created\n");
        printheader(syn_ack);
        create_UDP_dat_ptr(sendbuff,NULL,0,syn_ack);
        sockf=socket(AF_INET,SOCK_DGRAM,0);
        bzero(&serveraddr,sizeof(serveraddr));

        serveraddr.sin_family=AF_INET;
        serveraddr.sin_addr.s_addr=INADDR_ANY;
        serveraddr.sin_port=htons(syn_ack->sourcePort+connections.size()+80);
        portno=serveraddr.sin_port;


        bind(sockf,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
       
        printf("\nSYN ACK packet created");
        printheader(syn_ack);
        
        sendto(sockf,&sendbuff,sizeof(sendbuff),0,(struct sockaddr*)&clientaddr,sizeof(clientaddr));
        printf("\nSYN ACK packet sent");
        
        socklen_t clilen = sizeof(clientaddr);
        int n = wrapper_recvfrom(sockf,&ackbuff,sizeof(ackbuff),0,(struct sockaddr*)&clientaddr,&clilen,5);
        if ( n>0 ) {
        int rc=get_hdr_dat_frm_Buff(hdr,NULL,(const char *)&ackbuff,sizeof(ackbuff));
        printf("\n ACK packet Recieved");
        printheader(hdr);
        
        if(rc==0){
            if(hdr->ack && !hdr->syn && !hdr->rst && !hdr->fin && !hdr->urg && !hdr->psh){
                printf("connection established");
                remoteaddr=(struct sockaddr_in *)malloc(addrlen);
                memcpy(remoteaddr,&clientaddr,sizeof(clientaddr));
                printf("remote address modified");
                connection connected_client;
                connected_client.cliadd=clientaddr;
                copyHeader(&connected_client.hdr207,hdr);
                //connected_client.hdr207=hdr;
                hmap[portno]=connected_client;
                //All formalities successful so store the required
                memcpy(&connected_node.hdr207,syn_ack,sizeof(HDR_207));
                memcpy(&connected_node.cliadd,&connected_client.cliadd,sizeof(connected_client.cliadd));
                host_window_size=hdr->windowSize;
                printf("Out of accept with host window size:%d\n",host_window_size);
                return sockf;
            }
            else{
                fprintf(stderr,"the packet must be ack packet");
                resetHeaderFlags(hdr);
                hdr->rst=1;
                create_UDP_dat_ptr(sendbuff,NULL,0,hdr);
                sendto(sockf,&sendbuff,sizeof(sendbuff),0,(struct sockaddr*)&clientaddr,sizeof(clientaddr));
                printf("\n Sending RESET PACKET");
                printheader(hdr);
            }
        }

    }

}
    return -1;
}

int connect_socket(int sockfd, struct sockaddr_in *remoteaddr, socklen_t addrlen){
    
    printf("connect_207server\n");
    //Syn Packet 
    HDR_207 synpacket=*newHDR_PTR();
    printheader(&synpacket);
    static unsigned short int random_207sourceport=rand();
    unsigned  int sequenceNumber= (unsigned  int)rand();//rand();
    
    synpacket.syn=1;
    synpacket.sourcePort=random_207sourceport;
    synpacket.destPort=ntohs(remoteaddr->sin_port);
    synpacket.seqNum = sequenceNumber;
    synpacket.ackNum = 0;
    
    printf("\nCreated SYN packet");
    printheader(&synpacket);
    
    struct sockaddr_in servaddr;
    memcpy((struct sockaddr_in*)&servaddr, (struct sockaddr_in*)remoteaddr, addrlen);
    servaddr.sin_port = htons(mapUDPport(ntohs(remoteaddr->sin_port)));  //In client this is happening first time to communicate with port created by bind call of server.
   
    
    char sendbuffer[20] ;
    //Syn Packet
    create_UDP_dat_ptr(sendbuffer,NULL,0,&synpacket);
    
    sendto(sockfd,&sendbuffer,sizeof(sendbuffer),0,
             (struct sockaddr*)&servaddr,addrlen);
    
    printf("\nSent SYN packet");
     
    
    //Expect Syn - ack
    char synackbuff[20];
    struct sockaddr_in newservaddr;
    
    recvfrom(sockfd, &synackbuff,sizeof(synackbuff),0,
             (struct sockaddr*)&newservaddr,&addrlen);
    
    printf("\nRetrieved SYN-ACK packet");
    
    HDR_207 synackpacket=*newHDR_PTR();
    get_hdr_dat_frm_Buff(&synackpacket,NULL,(const char*)&synackbuff,sizeof(synackbuff));
     
    printheader(&synackpacket);
    
    if(!((synackpacket.ackNum==synpacket.seqNum+1)&&(synackpacket.seqNum==synpacket.ackNum+1)))
    //What to do!!
        
    if(synackpacket.syn && synackpacket.ack && !( synackpacket.fin && synackpacket.rst && synackpacket.urg && synackpacket.psh))
    {
    
    
    //Send ACK
    
    HDR_207 ackpacket=*newHDR_PTR();
    ackpacket.ack=1;
    ackpacket.destPort=synackpacket.sourcePort;
     ackpacket.sourcePort=synackpacket.destPort;
    sequenceNumber++;
    ackpacket.seqNum = synackpacket.ackNum; // Because data is not there
    ackpacket.ackNum = synackpacket.seqNum+1; //Since ack flag is 1
    ackpacket.windowSize=buffer_207_size*5;
    printf("\nCreated ACK packet");
     printheader(&ackpacket);
    
    create_UDP_dat_ptr(sendbuffer,NULL,0,&ackpacket);
    sendto(sockfd,&sendbuffer,sizeof(sendbuffer),0,
            (struct sockaddr*) &newservaddr,addrlen);
    printf("\nSent ACK packet");
    //All formalities are over so store the required variables.
    memcpy(remoteaddr,&newservaddr,sizeof(newservaddr)); 
    
    memcpy(&connected_node.hdr207,&ackpacket,sizeof(ackpacket));
    memcpy(&connected_node.cliadd,&newservaddr,sizeof(newservaddr));
    host_window_size = synackpacket.windowSize;
    
    return 0;
    }
    else{
        printf("Wrong SYN ACK Packet");
        return -2;
    }
    
}

/*
 * close connection internal function
 *
 */

int close_i(int sockfd) {

    char buf_ack[HEADER_LEN_207TCP];
    char buf_fin[HEADER_LEN_207TCP];
    
    HDR_207 *fin_hdr = newHDR_PTR();
    HDR_207 *fin_ack_hdr = newHDR_PTR();
    
    socklen_t clientlen = sizeof(connected_node.cliadd);
    
    printf("\n Inside internal close_connection");
    // send fin + ack
    fin_hdr->fin = 1;
    fin_hdr->ack = 1;
    fin_hdr->sourcePort = sockfd; 
    fin_hdr->destPort = connected_node.hdr207.sourcePort; 
    printf("\nCreated FIN + ACK packet");
    printheader(fin_hdr);

    create_UDP_dat_ptr(buf_fin,NULL,0,fin_hdr);
    
    sendto(sockfd, &buf_fin, sizeof(buf_fin),0, 
           (struct sockaddr*)&connected_node.cliadd, clientlen);
    
    // send ack
    resetHeaderFlags(fin_hdr);
    strcpy(buf_fin,"");
    fin_hdr->ack = 1;
    fin_hdr->sourcePort = sockfd; 
    fin_hdr->destPort = connected_node.hdr207.sourcePort; 
    printf("\nCreated ACK packet");
    printheader(fin_hdr);

    create_UDP_dat_ptr(buf_fin,NULL,0,fin_hdr);
                
    sendto(sockfd, &buf_fin, sizeof(buf_fin),0, 
           (struct sockaddr*)&connected_node.cliadd, clientlen);

    // receive ack
    strcpy(buf_ack,"");
    resetHeaderFlags(fin_ack_hdr);
    recvfrom(sockfd, &buf_ack, sizeof(buf_ack),0,
            (struct sockaddr*)&connected_node.cliadd, &clientlen);
    get_hdr_dat_frm_Buff(fin_ack_hdr, NULL, (const char*)& buf_ack,
                        sizeof(buf_ack));
    if(fin_ack_hdr -> ack)
    {

            close(sockfd);
    }
}


/*
 * client - send FIN + ACK --->  server - receive FIN + ACK
 *          receive FIN + ACK <--- send FIN + ACK
 *          
 *          send ACK ---> receive ACK
 */

int close_connection(int sockfd) {

    char buf_ack[HEADER_LEN_207TCP];
    char buf_fin[HEADER_LEN_207TCP];
    
    HDR_207 *fin_hdr = newHDR_PTR();
    HDR_207 *fin_ack_hdr = newHDR_PTR();
    
    socklen_t clientlen = sizeof(connected_node.cliadd);
    
    printf("\n Inside close_connection");
   
    copyHeader(fin_hdr,&connected_node.hdr207);
    resetHeaderFlags(fin_hdr);
    // close connection is explicitely called by either client/server 
    // send fin packet to other side
    fin_hdr->fin = 1;
  //  fin_hdr->sourcePort = sockfd; 
   // fin_hdr->destPort = connected_node.hdr207.sourcePort; 
    printf("\nCreated FIN packet");
    printheader(fin_hdr);

    create_UDP_dat_ptr(&buf_fin,NULL,0,fin_hdr);
   // get_hdr_dat_frm_Buff(fin_hdr,NULL,&buf_fin,20);
    printheader(fin_hdr);
    int rc;  
      int portn=ntohs(connected_node.cliadd.sin_port);
      
     printf("Port Number:%d",portn);
     printf("Connected IP: %s\n", inet_ntoa(((struct sockaddr_in *)&connected_node.cliadd)->sin_addr));
     rc=sendto(sockfd, &buf_fin, sizeof(buf_fin),0,(struct sockaddr*)&connected_node.cliadd, clientlen);
    //printf("FIN Packet sent");
    printf("FIN Packet sent");
    // receive fin + ack from other side
    
    rc=recvfrom(sockfd, &buf_ack, sizeof(buf_ack),0,
            (struct sockaddr*)&connected_node.cliadd, &clientlen);
    get_hdr_dat_frm_Buff(fin_ack_hdr, NULL, (const char*)& buf_ack,
                        sizeof(buf_ack));
    
    // check if this header has fin + ack
    if(!(fin_ack_hdr->ack ))//&& fin_ack_hdr->fin))
       return close(sockfd);   //Not Grace full!
    
        printf("\nRecieved FIN ACK");
        printheader(fin_ack_hdr);
    
    //receive ack
    resetHeaderFlags(fin_ack_hdr);    
    recvfrom(sockfd, &buf_ack, sizeof(buf_ack),0,
            (struct sockaddr*)&connected_node.cliadd, &clientlen);
    
    get_hdr_dat_frm_Buff(fin_ack_hdr, NULL, (const char*)& buf_ack,
                        sizeof(buf_ack));
    
    // check if this header has ack
    if(fin_ack_hdr->fin)
    {
        printf("\nRecieved FIN Packet");
        // send an ack packet
        HDR_207* fin_ack_hdr2send=newHDR_PTR(); 
        copyHeader(fin_ack_hdr2send,fin_hdr);
         fin_ack_hdr2send->fin = 0;
        fin_ack_hdr2send->ack = 1;
        fin_ack_hdr2send->ackNum=fin_ack_hdr->seqNum+1;
        fin_ack_hdr2send->seqNum=fin_ack_hdr->ackNum;
        
        //fin_ack_hdr->sourcePort = fin_h; 
        //fin_ack_hdr->destPort = connected_node.hdr207.sourcePort;
       
        printf("\nCreated Ack packet for FIN request");
        printheader(fin_ack_hdr2send);
        
        create_UDP_dat_ptr(buf_ack, NULL, 0, fin_ack_hdr2send);
        sendto(sockfd, &buf_ack, sizeof(buf_ack),0,
        (struct sockaddr*)&connected_node.cliadd, clientlen);
        printf("Sent FIN ACK packet,\n Connection Gracefully closed\n");
        return close(sockfd);
    }
    printf("connection not closed Gracefully");
        return close(sockfd);

 }

/*
 * Input parameters-
 *  sockfd is equal to CHILD socket number
 *  buf is data to be sent
 *  len is length of data 
 *  flags set to NOTUSED now
 */
static int acktorecieve;
ssize_t send_data(int sockfd, const void *buf, size_t len, int flags){
    printf("Send_data Enters\n");
    int sentlen=0;
   /* if(memcmp(&connected_node,0,sizeof(connected_node)))
    {
        printf("Error: Connection Lost or it was never created. ");
        return -1;
        
    }*/
    
    int i,iteration=0, retransmission=0;
    //Segmenting
    printf("\nConnected Node header:");
    printheader(&connected_node.hdr207);
    
    unsigned short leastsize=0 ;//=()?buffer_207_size:host_window_size;
    printf("\nBuffer 207 size, host window size: %d,%d\n",buffer_207_size,host_window_size);
    if((unsigned short)buffer_207_size<host_window_size)
      leastsize=(unsigned short)buffer_207_size;
    else
       leastsize= host_window_size;
   
    unsigned short s=(int)len;
    int sizetoget=s;
    if(((int)s)>((int)leastsize)){ 
       
         sizetoget=leastsize;
        iteration=((float)len/(float)leastsize);
    }
    
    //Not required for else because iteration is 0 in for it checks for i<=0
   printf("Least Size Chosen: %d,len: %d\n",iteration,s);
        char* tempfragbuffer = (char *)malloc(leastsize);
    
        
     for(i=0;i<iteration ;i++){
       
            
            memcpy(tempfragbuffer,(char *)(buf+(i*(buffer_207_size))),sizetoget);
                        
            buffer_207[acktorecieve] = tempfragbuffer;
         
            //Send and recieve the buffer   
        char* buffertosend = (char*)malloc(leastsize+20);
        create_UDP_dat_ptr(buffertosend,(const char*)tempfragbuffer,leastsize,&connected_node.hdr207);
        
        HDR_207 headertosend;
        
        copyHeader(&headertosend,&connected_node.hdr207);
        resetHeaderFlags(&headertosend);
    
     //headertosend.ackNum=connected_node.hdr207+leastsize;
    
    
    printf("\nHeader to send created:");
    printheader(&headertosend);
    
    sendto(sockfd, buffertosend,leastsize+20, 0, (const struct sockaddr*)&connected_node.cliadd,sizeof(connected_node.cliadd));
    printf("\nData Sent with above header\n");
    
    acktorecieve=headertosend.seqNum+leastsize;
    //free(buffertosend);
    struct sockaddr_in ackserver;
    HDR_207 ackpacket;
    buffertosend = (char*)malloc(20);
    socklen_t ackserversoclen=sizeof(ackserver);
    
    recvfrom(sockfd, buffertosend,20, 0, ( struct sockaddr*)&ackserver,&ackserversoclen);
    printf("\nACK recieved\n");
    
    //PROBLEM!!!! WHILE USING NETWORK.
    if(memcmp(&ackserver,&connected_node.cliadd,sizeof(connected_node.cliadd)))
    { 
        //printf("ERROR: ACK FROM DIFFERENT SERVER!!! ");
        return -1;
    }
    
    
    
    get_hdr_dat_frm_Buff(&ackpacket,NULL,buffertosend,20);
    
    printheader(&ackpacket);
    if((ackpacket.ack) )
    {
        if(ackpacket.ackNum==acktorecieve){
            sentlen+=leastsize;
        headertosend.seqNum=ackpacket.ackNum; //Next header for the packet  
        headertosend.ackNum=ackpacket.seqNum+1;
        }
        else{
        retransmission++;
        if(retransmission>NUMBER_OF_RETRANSMISSION)
            return -1;
        i--;
        continue;}
    }
    copyHeader (&connected_node.hdr207,&headertosend);
    
            
            
        }
    
    //
        return sentlen;
    
}

/*
 *
 */



ssize_t recv_data(int sockfd, void *buf, size_t len, int flags){
  //NOTE which is used in the hashmap to predict the next contegues data that has to come.
    printf("Recv_Data enters");
    buffer_207.clear();
    int buffersize=0;
    char * bufrecieved=(char*)malloc(len);;
    struct sockaddr_in cliaddr;
   char buff[20+len];
    HDR_207 * hdr=newHDR_PTR();
    HDR_207 * send_hdr=&connected_node.hdr207;
    do{
    resetHeaderFlags(send_hdr);
     socklen_t clilen = sizeof(cliaddr);
     int n = recv_check_fin(sockfd, &buff, HEADER_LEN_207TCP+len,0, (struct sockaddr *)& cliaddr, &clilen);
     printf("Received data packet: %d",n);
     if(n==FIN_SENT){
         return -1;
     }
     if ( n<= 0 )
     {
         printf("error in receiving the data");
         return -1;
     } 
     int portn=ntohs(cliaddr.sin_port);
     printf("Received at Port Number:%d",portn);
    if(!(portn ==  ntohs(connected_node.cliadd.sin_port)) && !strcmp(inet_ntoa( cliaddr.sin_addr), inet_ntoa(connected_node.cliadd.sin_addr)))
     { 
         printf("Not from the desired client!!");
         return -1;
     }
     const int data_size=n-sizeof(HDR_207);
     char *data=(char*)malloc(data_size);   
     int rc=get_hdr_dat_frm_Buff(hdr,data,(const char*)&buff,sizeof(buff));
        if(rc<0)
            return -1;
        printf("\nThe header retrieved from is:");
        printheader(hdr);
        
        printf("\n inside receive:Get header data from buffer %d",rc );
        if(hdr->ackNum=connected_node.hdr207.seqNum+1)
        {
            printf("\n received the data: %s",data);
            send_hdr->ackNum=hdr->seqNum+data_size;
            send_hdr->seqNum=hdr->ackNum;
            send_hdr->ack=1;
            //send_hdr->windowSize=(buffer_207.co()*buffer_207_size)-(buffer_207.size()*buffer_207_size);
            char sendbuff[20];
            create_UDP_dat_ptr(&sendbuff,NULL,0,send_hdr);
            printf("ACK prepared for data:");
            printheader(send_hdr);
            /*Future
             * char* hashbuff=(char*) malloc(data_size);
                 memcpy(hashbuff,data,data_size);
                if(buffer_207.empty())
                  buffer_207[hdr->seqNum]=hashbuff;
                else            
                        if(!(buffer_207[ hdr->seqNum]==NULL))
                                buffer_207[hdr->seqNum]=hashbuff;
                        
        */
            sendto(sockfd, &sendbuff, HEADER_LEN_207TCP,0, (const struct sockaddr *)& cliaddr, clilen);
            printf("Sent ACK back");
            buffersize=buffersize+data_size;
            memcpy(bufrecieved,data,data_size);
            bufrecieved=bufrecieved+data_size;
            copyHeader(&connected_node.hdr207,send_hdr);
            //return data_size;
        }
    }while(buffersize<len);
    bufrecieved=bufrecieved-buffersize;
    memcpy(buf,bufrecieved,buffersize);
    return buffersize;
  /*  map<int,char *>::const_iterator it ;
    int i=0;
     for( it==buffer_207.begin();it!=buffer_207.end();it++)
    {
   // cout << (char*)it->second;
     memcpy(buf+(i*buffer_207_size),it->second,buffer_207_size);
     i++;
    }*/
        return -1;
     
}


int recv_check_fin(int sockfd, void *buf, size_t len, int flags,struct sockaddr* node_address, socklen_t* node_address_len){
int rc, n;
printf("\nRecieve Check fin enters\n");
n = recvfrom( sockfd, buf, len, flags, node_address, node_address_len);
if(n < 0)
    return n;
HDR_207 * fin_packet=newHDR_PTR();
char* buff= (char *)malloc(20);
rc=get_hdr_dat_frm_Buff(fin_packet,buff,(const char*)buf,20);
if(rc<0)
    return rc;


if(fin_packet->fin)
   
{
    printf("Recieved packet is FIN:\n");
    printheader(fin_packet);

    
    close_i(sockfd,fin_packet);
return FIN_SENT;
}
return n;

}
void sigcatcher(int sig) {

    printf("\nRecv No Data Encountered, exiting blocking state\n", sig);
}

int wrapper_recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen, int time_out)
{
    int n;
    struct sigaction sact;
    // time_t t;    
    sigemptyset(&sact.sa_mask);
    sact.sa_flags = 0;
    sact.sa_handler = sigcatcher;
    sigaction(SIGALRM, &sact, NULL);
    alarm(time_out); // wait for specified time for data from client
   
//    time(&t);
//    printf("before calling recvfrom() time is %s", ctime(&t));
    n = recvfrom(sockfd, buf, len, flags, (struct sockaddr *)& src_addr, addrlen);
    return n;   
}

/*
 * close connection internal function
 *
 */

int close_i(int sockfd, HDR_207* fin_packet) {

    char buf_ack[HEADER_LEN_207TCP];
    char buf_fin[HEADER_LEN_207TCP];
    
    HDR_207 *fin_hdr = newHDR_PTR();
    HDR_207 *fin_ack_hdr = newHDR_PTR();
    
    socklen_t clientlen = sizeof(connected_node.cliadd);
    
    printf("\n Inside internal close_connection");
    // send fin + ack
    copyHeader(fin_hdr,&connected_node.hdr207);
    resetHeaderFlags(fin_hdr);
    fin_hdr->fin = 1;
    fin_hdr->ack = 1;
    fin_hdr->seqNum=fin_packet->ackNum;
    fin_hdr->ackNum=fin_packet->seqNum+1;
    //fin_hdr->sourcePort = sockfd; 
    //fin_hdr->destPort = connected_node.hdr207.sourcePort; 
    printf("\nCreated FIN + ACK packet");
    
    
    create_UDP_dat_ptr(&buf_fin,NULL,0,fin_hdr);
    
    sendto(sockfd, &buf_fin, sizeof(buf_fin),0, 
           (struct sockaddr*)&connected_node.cliadd, clientlen);
    copyHeader(&connected_node.hdr207,fin_hdr) ;
    printf("\n Sent FIN + ACK packet");
    // send ack
    resetHeaderFlags(fin_hdr);
    strcpy(buf_fin,"");
    fin_hdr->fin = 1;
    
    //fin_hdr->sourcePort = sockfd; 
    //fin_hdr->destPort = connected_node.hdr207.sourcePort; 
    printf("\nCreated FIN packet");
    printheader(fin_hdr);

    create_UDP_dat_ptr(buf_fin,NULL,0,fin_hdr);
                
    sendto(sockfd, &buf_fin, sizeof(buf_fin),0, 
           (struct sockaddr*)&connected_node.cliadd, clientlen);
printf("\nSent FIN packet");
copyHeader(&connected_node.hdr207,fin_hdr) ;
    // receive ack
    strcpy(buf_ack,"");
    resetHeaderFlags(fin_ack_hdr);
    recvfrom(sockfd, &buf_ack, sizeof(buf_ack),0,
            (struct sockaddr*)&connected_node.cliadd, &clientlen);
    get_hdr_dat_frm_Buff(fin_ack_hdr, NULL, (const char*)& buf_ack,
                        sizeof(buf_ack));
    if(fin_ack_hdr -> ack ){//&& fin_ack_hdr -> fin){
     printf("Closed Grace fully");
     close(sockfd);//Gracefully close.   
     return FIN_SENT;
        
    }
    
    printf("Closed Non Grace fully");
     
     close(sockfd);//Not grace full close.
    return FIN_SENT;             
    
}