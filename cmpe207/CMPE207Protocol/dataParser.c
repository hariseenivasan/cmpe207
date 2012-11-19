/* ===================================================================== */
/*                                                                       */
/*   HeaderParse.c                                                       */
/*   Sri Hari Haran                                                      */
/*                                                                       */
/*  Done                                                                 */
/*                                                                       */
/* ===================================================================== */


#include"dataParser.h"

//extern "C" int headerParse( PKT_207* pkt,const char* buffer, size_t bufferlength);
int get_hdr_dat_frm_Buff( HDR_207* pkt,char* data_buffer,const char* buffer, size_t bufferlength);
int create_UDP_dat_ptr(void* data_with_hdr, const char* data,  size_t length,const HDR_207* hdr);


HDR_207 *newHDR_PTR();
//PKT_207 *newPKT_PTR();


HDR_207 *newHDR_PTR(){
    HDR_207 *hdr= (HDR_207*)malloc(sizeof(HDR_207));
    hdr->sourcePort=0;
          hdr->destPort=0;
          hdr->seqNum=0;
          hdr->ackNum=0;      
         hdr->hex10=0;
               hdr->urg=0;
               hdr->ack=0;
               hdr->psh=0;
               hdr->rst=0;
               hdr->syn=0;
               hdr->fin=0;
          hdr->windowSize=0;       
         hdr->checksum=0;         
          hdr->urgPtr=0;  
          return hdr;
}

/*PKT_207 *newPKT_PTR(){
    return (PKT_207 *)malloc(sizeof(PKT_207));
}
/*int headerParse( PKT_207* pkt,const char* buffer, size_t bufferlength)
{

    headerParse(&pkt->hdr,buffer,sizeof(HDR_207));
    buffer+=sizeof(HDR_207);
    memcpy(pkt->msg,buffer,bufferlength-sizeof(HDR_207));  
}

int get_packet_from_buffer(PKT_207* pkt, const char* buffer, size_t bufferlength)
{
   /* PKT_207 pakt;
    char* data;
   if(!get_hdr_dat_frm_Buff(pakt.hdr ,  buffer,  bufferlength))
       return -1;
   pakt.msg=buffer+sizeof(HDR_207);
   pakt.msgLength=bufferlength;
   return 0;
}*/

int  get_hdr_dat_frm_Buff(HDR_207* hdr, char* data_buffer,const char* buffer, size_t bufferlength)

{

    if(bufferlength <sizeof(HDR_207))
    return -1; 
    memcpy(hdr, buffer,sizeof(HDR_207));
    //buffer+=sizeof(HDR_207);
    int extra_bytes = bufferlength -sizeof(HDR_207);
    if(extra_bytes>0){
       // char buf [extra_bytes];
      //  data_buffer=&buf;
    memcpy(data_buffer, buffer+sizeof(HDR_207),extra_bytes);
    }
    return 0;

}


/**
*Lets not use this, this is just for Logic
*
char* headerCreate(const PKT_207* pkt)
{
return headerCreate(pkt->msg,  sizeof(pkt->msg),pkt->hdr);
}*/


/**
*Header Create 
* Arguements: Data with buffer, data, buffer length and header.
* Return: 0 if success, -1 if header is empty
*/
int create_UDP_dat_ptr(void* data_with_hdr, const char* data,  size_t data_length,const HDR_207* hdr)
{
     //data_with_hdr=((char*)malloc(data_length+sizeof(HDR_207)));
    if(hdr==0)
    return -1; 
 
    memcpy(data_with_hdr, hdr,sizeof(HDR_207));
 
    if(data_length!=0)
    {
        
        
        
        memcpy(data_with_hdr+sizeof(HDR_207), data,data_length);
        
        
        
    }

    
    return 0;

}




