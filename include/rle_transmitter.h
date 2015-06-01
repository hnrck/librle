/**
 * @file   rle_transmitter.h
 * @brief  Definition of RLE transmitter context and status structure, functions and variables
 * @author Aurelien Castanie, Henrick Deschamps
 * @date   03/2015
 * @copyright
 *   Copyright (C) 2015, Thales Alenia Space France - All Rights Reserved
 */

#ifndef __RLE_TRANSMITTER_H__
#define __RLE_TRANSMITTER_H__

#include <stddef.h>
#include "rle_ctx.h"
#include "header.h"

/** RLE transmitter link status
 * holds the sum of all statistics
 * of each fragment_id */
struct transmitter_link_status {
	/** Total number of packets sent/received
	 * successfully */
	uint64_t counter_ok;
	/** Total number of dropped packets */
	uint64_t counter_dropped;
	/** Total number of lost packets */
	uint64_t counter_lost;
	/** Total number of bytes sent/received */
	uint64_t counter_bytes;
};

/**
 * RLE transmitter module used
 * for encapsulation & fragmentation.
 * Provides a context structure for each
 * fragment_id, with a list of contexts
 * free to use and with a configuration
 * structure.
 * A mutex is used for synchronize
 * access to free contexts.
 *
 */
struct rle_transmitter {
	struct rle_ctx_management rle_ctx_man[RLE_MAX_FRAG_NUMBER];
	struct rle_configuration *rle_conf;
	uint8_t free_ctx;
};

/**
 *  @brief Create a RLE transmitter module
 *
 *  @warning
 *
 *  @return Pointer to the transmitter module
 *
 *  @ingroup
 */
struct rle_transmitter *rle_transmitter_module_new(void);

/**
 *  @brief Initialize a RLE transmitter module
 *
 *  @warning
 *
 *  @param _this	The transmitter module to initialize
 *
 *  @ingroup
 */
void rle_transmitter_module_init(struct rle_transmitter *_this);

/**
 *  @brief Destroy a RLE transmitter module
 *
 *  @warning
 *
 *  @param _this	The transmitter module to destroy
 *
 *  @ingroup
 */
void rle_transmitter_module_destroy(struct rle_transmitter *_this);

/**
 *  @brief Encapsulate data into an RLE packet
 *
 *  @warning
 *
 *  @param _this	The transmitter module to use for encapsulation
 *  @param data_buffer	Data buffer's address to encapsulate
 *  @param data_length	Data length to encapsulate
 *
 *  @return	C_ERROR enable_crc_check is an invalid flag
 *		C_OK	Otherwise
 *
 *  @ingroup
 */
int rle_transmitter_encap_data(struct rle_transmitter *_this, void *data_buffer, size_t data_length,
                               uint16_t protocol_type,
                               uint8_t frag_id);

/**
 *  @brief Fill burst payload with an RLE packet
 *
 *  @warning
 *
 *  @param _this		The transmitter module to use for encapsulation
 *  @param burst_buffer		Burst buffer's address to fill
 *  @param burst_length		Burst length available
 *  @param fragment_id		Fragment id to use
 *  @param protocol_type	Protocol type to use in proto_type field
 *
 *  @return	C_ERROR enable_crc_check is an invalid flag
 *		C_OK	Otherwise
 *
 *  @ingroup
 */
int rle_transmitter_get_packet(struct rle_transmitter *_this, void *burst_buffer,
                               size_t burst_length, uint8_t fragment_id,
                               uint16_t protocol_type);

/**
 *  @brief Set to idle the fragment context
 *
 *  @warning
 *
 *  @param _this	The transmitter module to use for deencapsulation
 *  @param fragment_id	Fragmentation context to use to get the PDU
 *
 *  @ingroup
 */
void rle_transmitter_free_context(struct rle_transmitter *_this, uint8_t fragment_id);

/**
 *  @brief Get a queue (frag_id) state, filled or empty
 *
 *  @warning
 *
 *  @param _this		The transmitter module
 *  @param fragment_id		Fragment id to use
 *
 *  @return	C_TRUE if the queue is empty
 *		C_FALSE if the queue is full or has remaining PDU data
 *
 *  @ingroup
 */
int rle_transmitter_get_queue_state(struct rle_transmitter *_this, uint8_t fragment_id);

/**
 *  @brief Get occupied size of a queue (frag_id)
 *
 *  @warning
 *
 *  @param _this		The transmitter module
 *  @param fragment_id		Fragment id to use
 *
 *  @return	Number of Bytes present in a queue
 *
 *  @ingroup
 */
uint32_t rle_transmitter_get_queue_size(struct rle_transmitter *_this, uint8_t fragment_id);

/**
 *  @brief Get total number of successfully
 *  sent/received packets
 *
 *  @warning
 *
 *  @param _this		The transmitter module
 *
 *  @return	Number of packets sent/received successfully
 *
 *  @ingroup
 */
uint64_t rle_transmitter_get_counter_ok(struct rle_transmitter *_this);

/**
 *  @brief Get total number of dropped packets
 *
 *  @warning
 *
 *  @param _this		The transmitter module
 *
 *  @return	Number of dropped packets
 *
 *  @ingroup
 */
uint64_t rle_transmitter_get_counter_dropped(struct rle_transmitter *_this);

/**
 *  @brief Get total number of lost packets
 *
 *  @warning
 *
 *  @param _this		The transmitter module
 *
 *  @return	Number of lost packets
 *
 *  @ingroup
 */
uint64_t rle_transmitter_get_counter_lost(struct rle_transmitter *_this);

/**
 *  @brief Get total number of sent/received Bytes
 *
 *  @warning
 *
 *  @param _this		The transmitter module
 *
 *  @return	Number of Bytes sent/received
 *
 *  @ingroup
 */
uint64_t rle_transmitter_get_counter_bytes(struct rle_transmitter *_this);

/**
 *  @brief Dump all frag_id contents
 *
 *  @warning
 *
 *  @param _this		The transmitter module
 *
 *  @ingroup
 */
void rle_transmitter_dump(struct rle_transmitter *_this);

/**
 *  @brief         Dump an ALPDU from a context link to a frag id of a transmitter in a buffer.
 *
 *                 This is intended to help testing encapsulation only. Please don't use this after
 *                 fragmentation and take care if you want to use it in another way.
 *
 *  @param[in]     _this               The transmitter module.
 *  @param[in]     frag_id             The fragment id with the context that will be dump.
 *  @param[in,out] alpdu_buffer        A preallocated buffer that will contain the ALPDU.
 *  @param[in]     alpdu_buffer_size   The size of the preallocated buffer
 *  @param[out]    alpdu_length        The size of the ALPDU
 */
void rle_transmitter_dump_alpdu(struct rle_transmitter *_this, uint8_t frag_id,
                                unsigned char alpdu_buffer[], const size_t alpdu_buffer_size,
                                size_t *const alpdu_length);

/**
 *  @brief         Check the fragementation integrity in a frag id of a transmitter
 *
 *  @param[in]     _this               The transmitter with the frag id context to check.
 *  @param[in]     frag_id             The frag id.
 *
 *  @return        FRAG_STATUS_OK if fragmentation in OK, else FRAG_STATUS_KO.
 */
enum check_frag_status rle_transmitter_check_frag_integrity(
        const struct rle_transmitter *const _this, uint8_t frag_id);

#endif /* __RLE_TRANSMITTER_H__ */
