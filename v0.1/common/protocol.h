#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define PREFIX_SIZE 8
#define HEADER_SIZE	(PREFIX_SIZE+6)	//2ea : and SERIAL_NO_SIZE:4bytes
#define SERIAL_NO_SIZE	32

//prefix
#define PRF_SERIALNO	"SERIALNO"
#define PRF_REQ_STAT	"REQ_STAT"
#define PRF_ACK_STAT	"ACK_STAT"

#endif