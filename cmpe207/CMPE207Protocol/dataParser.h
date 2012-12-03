/* 
 * File:   dataParser.h
 * Author: Sri Hari Haran
 *
 * Created on October 31, 2012, 9:40 PM
 */

#ifndef DATAPARSER_H
#define	DATAPARSER_H
#include "207Header.h"
#ifdef	__cplusplus
extern "C" {
#endif
HDR_207 *newHDR_PTR();
 
// PKT_207 *newPKT_PTR();
   
  int get_hdr_dat_frm_Buff( HDR_207* pkt,char* data_buffer,const char* buffer, size_t bufferlength);
  
  int create_UDP_dat_ptr(void* data_with_hdr, const char* data,  size_t length,const HDR_207* hdr);
 #ifdef	__cplusplus
}
#endif

#endif	/* DATAPARSER_H */

