/**
 * @file   rle_receiver.c
 * @brief  RLE receiver functions
 * @author Aurelien Castanie, Henrick Deschamps
 * @date   03/2015
 * @copyright
 *   Copyright (C) 2015, Thales Alenia Space France - All Rights Reserved
 */

#ifndef __KERNEL__

#include <stdlib.h>
#include <stdio.h>

#endif

#ifdef TIME_DEBUG
#include <sys/time.h>
#endif
#include "rle_receiver.h"
#include "rle_ctx.h"
#include "constants.h"
#include "header.h"
#include "trailer.h"
#include "deencap.h"
#include "reassembly.h"

#define MODULE_NAME "RECEIVER"


static int is_context_free(struct rle_receiver *_this, size_t index)
{
	int context_is_free = C_FALSE;

	if (index >= RLE_MAX_FRAG_NUMBER) {
		goto error;
	}

	if (((_this->free_ctx >> index) & 0x1) == 0) {
		context_is_free = C_TRUE;
	}

error:
	return context_is_free;
}


static int get_first_free_frag_ctx(struct rle_receiver *_this)
{
	int i;

	for (i = 0; i < RLE_MAX_FRAG_NUMBER; i++) {
		if (((_this->free_ctx >> i) & 0x1) == 0) {
			return i;
		}
	}

	return C_ERROR;
}

static void set_nonfree_frag_ctx(struct rle_receiver *_this, int index)
{
	_this->free_ctx |= (1 << index);
}

static void set_free_frag_ctx(struct rle_receiver *_this, int index)
{
	_this->free_ctx &= ~(1 << index);
}

static int get_recvd_fragment_type(void *data_buffer)
{
	union rle_header_all *head = (union rle_header_all *)data_buffer;
	int type_rle_frag = C_ERROR;

#ifdef DEBUG
	PRINT("DEBUG %s %s:%s:%d: RLE packet S %d E %d \n",
	      MODULE_NAME,
	      __FILE__, __func__, __LINE__,
	      head->b.start_ind, head->b.end_ind);
#endif

	switch (head->b.start_ind) {
	case 0x0:
		if (head->b.end_ind == 0x0) {
			type_rle_frag = RLE_PDU_CONT_FRAG;
		} else {
			type_rle_frag = RLE_PDU_END_FRAG;
		}
		break;
	case 0x1:
		if (head->b.end_ind == 0x0) {
			type_rle_frag = RLE_PDU_START_FRAG;
		} else {
			type_rle_frag = RLE_PDU_COMPLETE;
		}
		break;
	default:
		PRINT("ERROR %s %s:%s:%d: invalid/unknown RLE fragment"
		      " type S [%x] E [%x]\n",
		      MODULE_NAME,
		      __FILE__, __func__, __LINE__,
		      head->b.start_ind, head->b.end_ind);
		break;
	}

	return type_rle_frag;
}

static uint16_t get_fragment_id(void *data_buffer)
{
	union rle_header_all *head = (union rle_header_all *)data_buffer;

	return head->b.LT_T_FID;
}

int rle_receiver_deencap_data(struct rle_receiver *_this, void *data_buffer, size_t data_length,
                              int *index_ctx)
{
#ifdef DEBUG
	PRINT("DEBUG %s %s:%s:%d:\n",
	      MODULE_NAME,
	      __FILE__, __func__, __LINE__);
#endif

#ifdef TIME_DEBUG
	struct timeval tv_start = { .tv_sec = 0L, .tv_usec = 0L };
	struct timeval tv_end = { .tv_sec = 0L, .tv_usec = 0L };
	gettimeofday(&tv_start, NULL);
#endif

	int ret = C_ERROR;
	int frag_type = 0;

	if (!data_buffer) {
		PRINT("ERROR %s %s:%s:%d: data buffer is invalid\n",
		      MODULE_NAME,
		      __FILE__, __func__, __LINE__);
		return ret;
	}

	if (!_this) {
		PRINT("ERROR %s %s:%s:%d: receiver module is invalid\n",
		      MODULE_NAME,
		      __FILE__, __func__, __LINE__);
		return ret;
	}

	/* check PPDU validity */
	if (data_length > RLE_MAX_PDU_SIZE) {
		PRINT("ERROR %s %s:%s:%d: Packet too long [%zu]\n",
		      MODULE_NAME,
		      __FILE__, __func__, __LINE__,
		      data_length);
		return ret;
	}

	/* retrieve frag id if its a fragmented packet to append data to the
	 * right frag id context (SE bits)
	 * or
	 * search for the first free frag id context to put data into it */
	frag_type = get_recvd_fragment_type(data_buffer);
	*index_ctx = -1;

	switch (frag_type) {
	case RLE_PDU_COMPLETE:
		*index_ctx = get_first_free_frag_ctx(_this);
		if (*index_ctx < 0) {
			PRINT("ERROR %s %s:%s:%d: no free reassembly context available "
			      "for deencapsulation\n",
			      MODULE_NAME, __FILE__, __func__, __LINE__);
			return C_ERROR;
		}
		break;
	case RLE_PDU_START_FRAG:
		*index_ctx = get_fragment_id(data_buffer);
#ifdef DEBUG
		PRINT("DEBUG %s %s:%s:%d: fragment_id 0x%0x frag type %d\n",
		      MODULE_NAME, __FILE__, __func__, __LINE__, *index_ctx, frag_type);
#endif
		if ((*index_ctx < 0) || (*index_ctx > RLE_MAX_FRAG_ID)) {
			PRINT("ERROR %s %s:%s:%d: invalid fragment id [%d]\n",
			      MODULE_NAME, __FILE__, __func__, __LINE__, *index_ctx);
			return C_ERROR;
		}
		if (is_context_free(_this, *index_ctx) == C_FALSE) {
			struct rle_ctx_management *const rle_ctx = &_this->rle_ctx_man[*index_ctx];
			PRINT("ERROR %s %s:%s:%d: invalid Start on context not free, frag id [%d]\n",
			      MODULE_NAME, __FILE__, __func__, __LINE__, *index_ctx);
			/* Context is not free, whereas it must be. an error must have occured. */
			/* Freeing context, updating stats, andrestarting receiving. */
			rle_ctx_incr_counter_dropped(rle_ctx);
			rle_ctx_incr_counter_bytes_dropped(rle_ctx, rle_ctx_get_remaining_alpdu_length(rle_ctx));
			rle_receiver_free_context(_this, *index_ctx);
		}
		break;
	case RLE_PDU_CONT_FRAG:
	case RLE_PDU_END_FRAG:
		*index_ctx = get_fragment_id(data_buffer);
#ifdef DEBUG
		PRINT("DEBUG %s %s:%s:%d: fragment_id 0x%0x frag type %d\n",
		      MODULE_NAME, __FILE__, __func__, __LINE__, *index_ctx, frag_type);
#endif
		if ((*index_ctx < 0) || (*index_ctx > RLE_MAX_FRAG_ID)) {
			PRINT("ERROR %s %s:%s:%d: invalid fragment id [%d]\n",
			      MODULE_NAME, __FILE__, __func__, __LINE__, *index_ctx);
			return C_ERROR;
		}
		if (is_context_free(_this, *index_ctx) == C_TRUE) {
			struct rle_ctx_management *const rle_ctx = &_this->rle_ctx_man[*index_ctx];
			PRINT("ERROR %s %s:%s:%d: invalid %s on context free, frag id [%d]\n",
			      MODULE_NAME, __FILE__, __func__, __LINE__,
			      frag_type == RLE_PDU_CONT_FRAG ? "Cont" : "End", *index_ctx);
			/* Context is free, whereas it must not. an error must have occured. */
			/* Freeing context and updating stats. At least one packet is partialy lost.*/

			rle_ctx_incr_counter_dropped(rle_ctx);
			rle_ctx_incr_counter_lost(rle_ctx, 1);
			rle_ctx_incr_counter_bytes_dropped(rle_ctx, rle_ctx_get_remaining_alpdu_length(rle_ctx));
			rle_receiver_free_context(_this, *index_ctx);

			return C_ERROR;
		}
		break;
	default:
		return C_ERROR;
		break;
	}

	/* set the previously free frag ctx
	 * or force already
	 * set frag ctx to 'used' state */
	set_nonfree_frag_ctx(_this, *index_ctx);

	/* reassemble all fragments */
	ret = reassembly_reassemble_pdu(&_this->rle_ctx_man[*index_ctx],
	                                _this->rle_conf[*index_ctx],
	                                data_buffer,
	                                data_length,
	                                frag_type);

	if ((ret != C_OK) && (ret != C_REASSEMBLY_OK)) {
		/* received RLE packet is invalid,
		 * we have to flush related context
		 * for this frag_id */
		rle_ctx_invalid_ctx(&_this->rle_ctx_man[*index_ctx]);
		rle_ctx_flush_buffer(&_this->rle_ctx_man[*index_ctx]);
		set_free_frag_ctx(_this, *index_ctx);
		PRINT("ERROR %s %s:%s:%d: cannot reassemble data, error type %d\n",
		      MODULE_NAME,
		      __FILE__, __func__, __LINE__,
		      ret);
	}

#ifdef TIME_DEBUG
	struct timeval tv_delta;
	gettimeofday(&tv_end, NULL);
	tv_delta.tv_sec = tv_end.tv_sec - tv_start.tv_sec;
	tv_delta.tv_usec = tv_end.tv_usec - tv_start.tv_usec;
	PRINT("DEBUG %s %s:%s:%d: duration [%04ld.%06ld]\n",
	      MODULE_NAME,
	      __FILE__, __func__, __LINE__,
	      tv_delta.tv_sec, tv_delta.tv_usec);
#endif

	return ret;
}

int rle_receiver_get_packet(struct rle_receiver *_this, uint8_t fragment_id, void *pdu_buffer,
                            int *pdu_proto_type,
                            uint32_t *pdu_length)
{
#ifdef DEBUG
	PRINT("DEBUG %s %s:%s:%d:\n", MODULE_NAME,
	      __FILE__, __func__, __LINE__);
#endif

#ifdef TIME_DEBUG
	struct timeval tv_start = { .tv_sec = 0L, .tv_usec = 0L };
	struct timeval tv_end = { .tv_sec = 0L, .tv_usec = 0L };
	gettimeofday(&tv_start, NULL);
#endif

	int ret = reassembly_get_pdu(&_this->rle_ctx_man[fragment_id],
	                             pdu_buffer,
	                             pdu_proto_type,
	                             pdu_length);

/*        if (ret == C_OK) {*/
/*                |+ reset buffer content +|*/
/*                rle_ctx_flush_buffer(&_this->rle_ctx_man[fragment_id]);*/
/*                set_free_frag_ctx(_this, fragment_id);*/
/*        }*/

#ifdef TIME_DEBUG
	struct timeval tv_delta;
	gettimeofday(&tv_end, NULL);
	tv_delta.tv_sec = tv_end.tv_sec - tv_start.tv_sec;
	tv_delta.tv_usec = tv_end.tv_usec - tv_start.tv_usec;
	PRINT("DEBUG %s %s:%s:%d: duration [%04ld.%06ld]\n",
	      MODULE_NAME,
	      __FILE__, __func__, __LINE__,
	      tv_delta.tv_sec, tv_delta.tv_usec);
#endif

	return ret;
}

void rle_receiver_free_context(struct rle_receiver *_this, uint8_t fragment_id)
{
	/* set to idle this fragmentation context */
	rle_ctx_flush_buffer(&_this->rle_ctx_man[fragment_id]);
	set_free_frag_ctx(_this, fragment_id);
}
