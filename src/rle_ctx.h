/**
 * @file   rle_ctx.h
 * @brief  Definition of RLE context and status structure, functions and variables
 * @author Aurelien Castanie, Henrick Deschamps
 * @date   03/2015
 * @copyright
 *   Copyright (C) 2015, Thales Alenia Space France - All Rights Reserved
 */

#ifndef __RLE_CTX_H__
#define __RLE_CTX_H__

#ifndef __KERNEL__

#include <stdint.h>

#else

#include <linux/types.h>

#endif

#include "rle_conf.h"
#include "constants.h"
#include "fragmentation_buffer.h"


/*------------------------------------------------------------------------------------------------*/
/*--------------------------------- PUBLIC STRUCTS AND TYPEDEFS ----------------------------------*/
/*------------------------------------------------------------------------------------------------*/

/** RLE link status counters */
struct link_status {
	/** Number of SDUs received (partially received) for transmission (reception) */
	uint64_t counter_in;
	/** Number of SDUs sent/received successfully */
	uint64_t counter_ok;
	/** Number of dropped SDUs */
	uint64_t counter_dropped;
	/** Number of lost SDUs */
	uint64_t counter_lost;
	/** Number of bytes received (partially received) for transmission (reception) */
	uint64_t counter_bytes_in;
	/** Number of bytes of transmitted/received SDUs */
	uint64_t counter_bytes_ok;
	/** Number of bytes dropped */
	uint64_t counter_bytes_dropped;
};

/** RLE context management structure */
struct rle_ctx_management {
	/** specify fragment id the structure belongs to */
	uint8_t frag_id;
	/** next sequence number for frag_id */
	uint8_t next_seq_nb;
	/** PDU fragmentation status */
	int is_fragmented;
	/** current number of fragments present in queue */
	uint16_t frag_counter;
	/** Fragment counter from the first START frag of a fragmented PDU */
	int nb_frag_pdu;
	/** specify PDU QoS tag */
	uint32_t qos_tag;
	/** CRC32 trailer usage status */
	int use_crc;
	/** Fragmentation/Reassembly buffer. */
	void *buff;
	/** size of received PDU or PDU to send */
	uint32_t pdu_length;
	/** size of remaining data to send or to receive */
	uint32_t remaining_pdu_length;
	/** size of last RLE packet/fragment received/sent */
	uint32_t rle_length;
	/** size of the ALPDU fragmented/to fragment */
	uint32_t alpdu_size;
	/** remaining ALPDU size to send/receive */
	uint32_t remaining_alpdu_size;
	/** PDU protocol type */
	uint16_t proto_type;
	/** PDU Label type */
	uint8_t label_type;
	/** Pointer to PDU buffer */
	void *pdu_buf;
	/** Buffer containing PDU refs and
	 * headers/trailer */
	void *buf;
	/** End address of last fragment
	 * constructed in buffer */
	char *end_address;
	/** Current octets counter. */
	size_t current_counter;
	/** Type of link TX or RX */
	int lk_type;
	/** Fragmentation context status */
	struct link_status lk_status;
};


/*------------------------------------------------------------------------------------------------*/
/*--------------------------------------- PUBLIC FUNCTIONS ---------------------------------------*/
/*------------------------------------------------------------------------------------------------*/

/**
 * @brief  Initialize RLE context structure with fragmentation buffers.
 *
 * @param[out]    _this  Pointer to the RLE context structure
 *
 * @return  C_ERROR  If initilization went wrong
 *          C_OK     Otherwise
 *
 * @ingroup RLE context
 */
int rle_ctx_init_f_buff(struct rle_ctx_management *_this);

/**
 * @brief  Initialize RLE context structure with reassembly buffers.
 *
 * @param[out]    _this  Pointer to the RLE context structure
 *
 * @return  C_ERROR  If initilization went wrong
 *          C_OK     Otherwise
 *
 * @ingroup RLE context
 */
int rle_ctx_init_r_buff(struct rle_ctx_management *_this);

/**
 * @brief  Destroy RLE context with fragmentation buffers structure and free memory
 *
 * @param[out]   _this  Pointer to the RLE context structure
 *
 * @return  C_ERROR  If destruction went wrong
 *          C_OK     Otherwise
 *
 * @ingroup RLE context
 */
int rle_ctx_destroy_f_buff(struct rle_ctx_management *_this);

/**
 * @brief  Destroy RLE context with reassembly buffers structure and free memory
 *
 * @param[out]   _this  Pointer to the RLE context structure
 *
 * @return  C_ERROR  If destruction went wrong
 *          C_OK     Otherwise
 *
 * @ingroup RLE context
 */
int rle_ctx_destroy_r_buff(struct rle_ctx_management *_this);

/**
 *  @brief	Set fragment id
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	New fragment id value
 *
 *  @ingroup
 */
void rle_ctx_set_frag_id(struct rle_ctx_management *_this, uint8_t val);

/**
 *  @brief	Get current fragment id
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	Fragment id
 *
 *  @ingroup
 */
uint8_t rle_ctx_get_frag_id(struct rle_ctx_management *_this);

/**
 *  @brief	Set sequence number
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	New sequence number value
 *
 *  @ingroup
 */
void rle_ctx_set_seq_nb(struct rle_ctx_management *_this, uint8_t val);

/**
 *  @brief	Get current sequence number
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	Sequence number
 *
 *  @ingroup
 */
uint8_t rle_ctx_get_seq_nb(const struct rle_ctx_management *const _this);

/**
 *  @brief	Increment by one current sequence number
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
void rle_ctx_incr_seq_nb(struct rle_ctx_management *_this);

/**
 *  @brief	Set CRC usage flag for a specific RLE context
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	New boolean value representing CRC usage
 *
 *  @ingroup
 */
void rle_ctx_set_use_crc(struct rle_ctx_management *_this, int val);

/**
 *  @brief	Get current CRC usage flag
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	CRC usage boolean
 *
 *  @ingroup
 */
int rle_ctx_get_use_crc(const struct rle_ctx_management *const _this);

/**
 *  @brief	Get RLE packet length
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	Current RLE packet length
 *
 *  @ingroup
 */
uint32_t rle_ctx_get_rle_length(struct rle_ctx_management *_this);

/**
 *  @brief	increment ALPDU length
 *
 *  @warning
 *
 *  @param	_this       Pointer to the RLE context structure
 *  @param	val         ALPDU length incremented of val
 *
 *  @ingroup
 */
void rle_ctx_incr_alpdu_length(struct rle_ctx_management *const _this, const uint32_t val);

/**
 *  @brief	Get ALPDU length
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	Current ALPDU length
 *
 *  @ingroup
 */
uint32_t rle_ctx_get_alpdu_length(const struct rle_ctx_management *const _this);

/**
 *  @brief	decrement remaining ALPDU length
 *
 *  @warning
 *
 *  @param	_this       Pointer to the RLE context structure
 *  @param	val         remaining ALPDU length decremented of val
 *
 *  @ingroup
 */
void rle_ctx_decr_remaining_alpdu_length(struct rle_ctx_management *const _this, const uint32_t val);

/**
 *  @brief	Get remaining ALPDU length
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	Current remaining ALPDU length
 *
 *  @ingroup
 */
uint32_t rle_ctx_get_remaining_alpdu_length(const struct rle_ctx_management *const _this);

/**
 *  @brief	Get Protocol Type value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	Current Protocol Type value
 *
 *  @ingroup
 */
uint16_t rle_ctx_get_proto_type(struct rle_ctx_management *_this);

/**
 *  @brief	Set Label Type value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *		val	New Label Type value
 *
 *  @ingroup
 */

/**
 *  @brief	Set the number of SDUs received (partially received) for transmission (reception)
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	New counter value
 *
 *  @ingroup
 */
static inline void rle_ctx_set_counter_in(struct rle_ctx_management *const _this,
                                          const uint64_t val)
{
	_this->lk_status.counter_in = val;

	return;
}


/**
 *  @brief	Reset the number of SDUs received (partially received) for transmission (reception)
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
static inline void rle_ctx_reset_counter_in(struct rle_ctx_management *const _this)
{
	rle_ctx_set_counter_in(_this, 0L);

	return;
}


/**
 *  @brief	Increment by one number of SDUs received (partially received) for transmission
 *  (reception)
 *
 *		counter value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
static inline void rle_ctx_incr_counter_in(struct rle_ctx_management *const _this)
{
	_this->lk_status.counter_in++;

	return;
}


/**
 *  @brief	Get current counter value for SDU to be transmitted/received
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	SDU to be transmitted/received counter value
 *
 *  @ingroup
 */
static inline uint64_t rle_ctx_get_counter_in(const struct rle_ctx_management *const _this)
{
	return _this->lk_status.counter_in;
}


/**
 *  @brief	Set SDU successfully transmitted/received counter value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	New counter value
 *
 *  @ingroup
 */
static inline void rle_ctx_set_counter_ok(struct rle_ctx_management *const _this,
                                          const uint64_t val)
{
	_this->lk_status.counter_ok = val;

	return;
}


/**
 *  @brief	Reset SDU successfully transmitted/received counter value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
static inline void rle_ctx_reset_counter_ok(struct rle_ctx_management *const _this)
{
	rle_ctx_set_counter_ok(_this, 0L);

	return;
}


/**
 *  @brief	Increment by one number of SDU successfully transmitted/received
 *		counter value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
static inline void rle_ctx_incr_counter_ok(struct rle_ctx_management *const _this)
{
	_this->lk_status.counter_ok++;

	return;
}


/**
 *  @brief	Get current counter value for SDU successfully transmitted/received
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	SDU successfully transmitted/received counter value
 *
 *  @ingroup
 */
static inline uint64_t rle_ctx_get_counter_ok(const struct rle_ctx_management *const _this)
{
	return _this->lk_status.counter_ok;
}


/**
 *  @brief	Set dropped SDU counter value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	New counter value
 *
 *  @ingroup
 */
static inline void rle_ctx_set_counter_dropped(struct rle_ctx_management *const _this,
                                               const uint64_t val)
{
	_this->lk_status.counter_dropped = val;

	return;
}


/**
 *  @brief	Reset dropped SDU counter value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
static inline void rle_ctx_reset_counter_dropped(struct rle_ctx_management *const _this)
{
	rle_ctx_set_counter_dropped(_this, 0L);

	return;
}


/**
 *  @brief	Increment by one dropped SDU counter value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
static inline void rle_ctx_incr_counter_dropped(struct rle_ctx_management *const _this)
{
	_this->lk_status.counter_dropped++;

	return;
}


/**
 *  @brief	Get current dropped SDU counter value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	Dropped SDU counter value
 *
 *  @ingroup
 */
static inline uint64_t rle_ctx_get_counter_dropped(const struct rle_ctx_management *const _this)
{
	return _this->lk_status.counter_dropped;
}


/**
 *  @brief	Set lost SDU counter value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	New counter value
 *
 *  @ingroup
 */
static inline void rle_ctx_set_counter_lost(struct rle_ctx_management *const _this,
                                            const uint64_t val)
{
	_this->lk_status.counter_lost = val;

	return;
}


/**
 *  @brief	Reset lost SDU counter value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
static inline void rle_ctx_reset_counter_lost(struct rle_ctx_management *const _this)
{
	rle_ctx_set_counter_lost(_this, 0L);

	return;
}


/**
 *  @brief	Increment by one lost SDU counter value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
static inline void rle_ctx_incr_counter_lost(struct rle_ctx_management *const _this,
                                             const uint64_t val)
{
	_this->lk_status.counter_lost += val;

	return;
}


/**
 *  @brief	Get current lost SDU counter value
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	Lost SDU counter value
 *
 *  @ingroup
 */
static inline uint64_t rle_ctx_get_counter_lost(const struct rle_ctx_management *const _this)
{
	return _this->lk_status.counter_lost;
}


/**
 *  @brief	Set to be sent/partially received SDUs bytes
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	New counter value
 *
 *  @ingroup
 */
static inline void rle_ctx_set_counter_bytes_in(struct rle_ctx_management *const _this,
                                                const uint64_t val)
{
	_this->lk_status.counter_bytes_in = val;

	return;
}


/**
 *  @brief	Reset to be sent/partially received SDUs bytes
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
static inline void rle_ctx_reset_counter_bytes_in(struct rle_ctx_management *const _this)
{
	rle_ctx_set_counter_bytes_in(_this, 0L);

	return;
}


/**
 *  @brief	Increment by given value to be sent/partially received SDUs bytes
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	Number of bytes to add to the to be sent/partially received SDUs bytes counter
 *
 *  @ingroup
 */
static inline void rle_ctx_incr_counter_bytes_in(struct rle_ctx_management *const _this,
                                                 const uint64_t val)
{
	_this->lk_status.counter_bytes_in += val;

	return;
}


/**
 *  @brief	Get current number of to be sent/partially received SDUs bytes
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	Number of to be sent/partilly received SDUs Bytes
 *
 *  @ingroup
 */
static inline uint64_t rle_ctx_get_counter_bytes_in(const struct rle_ctx_management *const _this)
{
	return _this->lk_status.counter_bytes_in;
}


/**
 *  @brief	Set successfully sent/received number of bytes
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	New counter value
 *
 *  @ingroup
 */
static inline void rle_ctx_set_counter_bytes_ok(struct rle_ctx_management *const _this,
                                                const uint64_t val)
{
	_this->lk_status.counter_bytes_ok = val;

	return;
}


/**
 *  @brief	Reset successfully sent/received number of bytes
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
static inline void rle_ctx_reset_counter_bytes_ok(struct rle_ctx_management *const _this)
{
	rle_ctx_set_counter_bytes_ok(_this, 0L);

	return;
}


/**
 *  @brief	Increment by given value sent/received bytes
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	Number of Bytes to add to the current bytes counter
 *
 *  @ingroup
 */
static inline void rle_ctx_incr_counter_bytes_ok(struct rle_ctx_management *const _this,
                                                 const uint64_t val)
{
	_this->lk_status.counter_bytes_ok += val;

	return;
}


/**
 *  @brief	Get current number of sent/partially received SDUs bytes
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	Number of sent/partilly received SDUs Bytes
 *
 *  @ingroup
 */
static inline uint64_t rle_ctx_get_counter_bytes_ok(const struct rle_ctx_management *const _this)
{
	return _this->lk_status.counter_bytes_ok;
}


/**
 *  @brief	Set successfully sent/received number of Bytes
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	New counter value
 *
 *  @ingroup
 */
static inline void rle_ctx_set_counter_bytes_dropped(struct rle_ctx_management *const _this,
                                                     const uint64_t val)
{
	_this->lk_status.counter_bytes_dropped = val;

	return;
}


/**
 *  @brief	Reset successfully sent/received number of Bytes
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
static inline void rle_ctx_reset_counter_bytes_dropped(struct rle_ctx_management *const _this)
{
	rle_ctx_set_counter_bytes_dropped(_this, 0L);

	return;
}


/**
 *  @brief	Increment by given value dropped bytes counter
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *  @param	val	Number of bytes to add to the current dropped bytes counter
 *
 *  @ingroup
 */
static inline void rle_ctx_incr_counter_bytes_dropped(struct rle_ctx_management *const _this,
                                                      const uint64_t val)
{
	_this->lk_status.counter_bytes_dropped += val;

	return;
}


/**
 *  @brief	Get current number of dropped SDUs bytes
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @return	Number of to be dropped SDUs Bytes
 *
 *  @ingroup
 */
static inline uint64_t rle_ctx_get_counter_bytes_dropped(
        const struct rle_ctx_management *const _this)
{
	return _this->lk_status.counter_bytes_dropped;
}


/**
 *  @brief	Reset all counters
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
static inline void rle_ctx_reset_counters(struct rle_ctx_management *_this)
{
	rle_ctx_reset_counter_in(_this);
	rle_ctx_reset_counter_ok(_this);
	rle_ctx_reset_counter_dropped(_this);
	rle_ctx_reset_counter_lost(_this);
	rle_ctx_reset_counter_bytes_in(_this);
	rle_ctx_reset_counter_bytes_ok(_this);
	rle_ctx_reset_counter_bytes_dropped(_this);

	return;
}

/**
 *  @brief	Dump & print to stdout the content of a specific RLE context
 *
 *  @warning
 *
 *  @param	_this   Pointer to the RLE context structure
 *
 *  @ingroup
 */
void rle_ctx_dump(struct rle_ctx_management *_this, struct rle_configuration *rle_conf);

/**
 *  @brief         Dump an ALPDU from a context in a buffer.
 *
 *                 This is intended to help testing encapsulation only. Please don't use this after
 *                 fragmentation and take care if you want to use it in another way.
 *
 *  @param[in]     protocol_type       The theorical protocol_type in the context.
 *                                     (You can use rle_ctx_get_proto_type)
 *                                     For now, this param is useful, but it can evolve in the
 *                                     future.
 *  @param[in]     _this               The RLE context
 *  @param[in]     rle_conf            The RLE configuration
 *  @param[in,out] alpdu_buffer        A preallocated buffer that will contain the ALPDU.
 *  @param[in]     alpdu_buffer_size   The size of the preallocated buffer
 *  @param[out]    alpdu_length        The size of the ALPDU
 */
void rle_ctx_dump_alpdu(const uint16_t protocol_type, const struct rle_ctx_management *const _this,
                        struct rle_configuration *const rle_conf, unsigned char alpdu_buffer[],
                        const size_t alpdu_buffer_size,
                        size_t * const alpdu_length);

/** Status for the fragmentation checking */
enum check_frag_status {
	FRAG_STATUS_OK, /**< Fragementation is ok. */
	FRAG_STATUS_KO  /**< Error case.           */
};


/** States of fragmentation */
enum frag_states {
	FRAG_STATE_UNINIT, /**< Fragmentation not started */
	FRAG_STATE_START,  /**< Fragmentation is in starting state   */
	FRAG_STATE_CONT,   /**< Fragmentation is in continuing state */
	FRAG_STATE_END,    /**< Fragmentation is in ending state     */
	FRAG_STATE_COMP    /**< No fragmentation */
};

/**
 *  @brief         Check if a fragmentation transition is OK.
 *
 *  @param[in]     current_state       The current state.
 *  @param[in]     next_state          The future state.
 *
 *  @return        FRAG_STATUS_OK if legal transition, else FRAG_STATUS_KO.
 */
enum check_frag_status check_frag_transition(const enum frag_states current_state,
                                             const enum frag_states next_state);

/**
 *  @brief         Check the fragmentation integrity
 *
 *  @param[in]     _this               The context with the buffer to check.
 *
 *  @return        FRAG_STATUS_OK if fragmentation in OK, else FRAG_STATUS_KO.
 */
enum check_frag_status rle_ctx_check_frag_integrity(const struct rle_ctx_management *const _this);

/**
 *  @brief         Get the type of the fragment in the buffer
 *
 *  @param[in]     buffer              The buffer
 *
 *  @return        the fragment type @see enum frag_states
 */
enum frag_states get_fragment_type(const unsigned char *const buffer);

/**
 *  @brief         Get the length of the fragment in the buffer
 *
 *  @param[in]     buffer              The buffer
 *
 *  @return        the fragment type @see enum frag_states
 */
size_t get_fragment_length(const unsigned char *const buffer);

/**
 *  @brief         Get the fragment id of the fragment in the buffer
 *
 *  @param[in]     buffer              The buffer
 *
 *  @return        the fragment id of the fragment
 */
uint8_t get_fragment_frag_id(const unsigned char *const buffer);

/**
 *  @brief         Get the state of the frag_id-nth context.
 *
 *  @param[in]     contexts              The contexts
 *  @param[in]     frag_id               The frag_id of the context checked
 *
 *  @return        C_FALSE if the context is in use, else C_TRUE if free.
 */
static inline int rle_ctx_is_free(uint8_t contexts, const size_t frag_id)
{
	int context_is_free = C_FALSE;

	if (((contexts >> frag_id) & 0x1) == 0) {
		context_is_free = C_TRUE;
	}

	return context_is_free;
}

/**
 *  @brief         Set the state of the frag_id-nth context to NON FREE.
 *
 *  @param[in]     contexts              The contexts
 *  @param[in]     frag_id               The frag_id of the context to be set
 */
static inline void rle_ctx_set_nonfree(uint8_t *const contexts, const size_t frag_id)
{
	*contexts |= (1 << frag_id);

	return;
}

/**
 *  @brief         Set the state of the frag_id-nth context to FREE.
 *
 *  @param[in]     contexts              The contexts
 *  @param[in]     frag_id               The frag_id of the context to be set
 */
static inline void rle_ctx_set_free(uint8_t *const contexts, const size_t frag_id)
{
	*contexts &= ~(1 << frag_id);

	return;
}

#endif /* __RLE_CTX_H__ */
