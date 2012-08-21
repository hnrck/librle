/**
 * @file   constants.h
 * @author Aurelien Castanie
 *
 * @brief  Definition of RLE context and status structure, functions and variables
 *
 *
 */

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#ifndef __KERNEL__

#include <stdlib.h>
#include <malloc.h>
#include <arpa/inet.h>

#else

#include <linux/vmalloc.h>

#endif

#ifndef _REENTRANT
#define _REENTRANT
#endif

#define C_TRUE		1
#define C_FALSE		0

#define C_OK		0
#define C_ERROR		-1
#define C_ERROR_DROP	-2

#define IP_VERSION_4	4
#define IP_VERSION_6	6

enum rle_packet_type {
	RLE_COMPLETE_PACKET = 0,
	RLE_START_PACKET,
	RLE_CONT_PACKET,
	RLE_END_PACKET
};

#ifndef __KERNEL__

#define MALLOC(size_bytes)	malloc(size_bytes);
#define FREE(buf_addr)		free(buf_addr);
#define PRINT(x...)		printf(x);

#else

/* vmalloc allocates size with 4K modulo so for 8*2565 = 20520B it would alloc 24K
 * kmalloc allocates size with power of two so for 20520B it would alloc 32K */
#define MALLOC(size_bytes)	kmalloc(size_bytes); /* vmalloc(size_bytes); */
#define FREE(buf_addr)		kfree(buf_addr); /* vfree(buf_addr); */
#define PRINT(x...)		printk(x);

#endif

#endif /* _CONSTANTS_H */

