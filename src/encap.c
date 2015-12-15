/**
 * @file   encap.c
 * @brief  RLE encapsulation functions
 * @author Aurelien Castanie, Henrick Deschamps
 * @date   03/2015
 * @copyright
 *   Copyright (C) 2015, Thales Alenia Space France - All Rights Reserved
 */

#ifndef __KERNEL__

#include <stdlib.h>
#include <stdio.h>
#include <net/ethernet.h>

#else

#include <linux/types.h>

#endif

#include "encap.h"
#include "constants.h"
#include "rle_ctx.h"
#include "zc_buffer.h"
#include "rle_conf.h"
#include "rle_header_proto_type_field.h"

#define MODULE_NAME "ENCAP"

static int create_header(struct rle_ctx_management *rle_ctx, struct rle_configuration *rle_conf,
                         void *data_buffer, size_t data_length,
                         uint16_t protocol_type)
{
#ifdef DEBUG
	PRINT("DEBUG %s %s:%s:%d:\n",
	      MODULE_NAME,
	      __FILE__, __func__, __LINE__);
#endif

	size_t size_header = RLE_COMPLETE_HEADER_SIZE;
	size_t ptype_length = 0;
	uint8_t proto_type_supp = RLE_T_PROTO_TYPE_NO_SUPP;

	/* map RLE header to the already allocated buffer */
	struct zc_rle_header_complete_w_ptype *rle_hdr =
	        (struct zc_rle_header_complete_w_ptype *)rle_ctx->buf;
	uint8_t label_type;

	/* don't fill ALPDU ptype field if given ptype
	 * is equal to the default one and suppression is active,
	 * or if given ptype is for signalling packet */
	if (!ptype_is_omissible(protocol_type, rle_conf)) {
		/* remap a complete header with ptype field */
		struct rle_header_complete_w_ptype *rle_c_hdr =
		        (struct rle_header_complete_w_ptype *)&rle_hdr->header;

		if (rle_conf_get_ptype_compression(rle_conf)) {
			ptype_length = RLE_PROTO_TYPE_FIELD_SIZE_COMP;
			if (rle_header_ptype_is_compressible(protocol_type) == C_OK) {
				rle_c_hdr->ptype_c_s.c.proto_type = rle_header_ptype_compression(
				        protocol_type);
			} else {
				rle_c_hdr->ptype_c_s.e.proto_type = 0xFF;
				rle_c_hdr->ptype_c_s.e.proto_type_uncompressed = ntohs(
				        protocol_type);
				ptype_length += RLE_PROTO_TYPE_FIELD_SIZE_UNCOMP;
			}
		} else {
			rle_c_hdr->ptype_u_s.proto_type = ntohs(protocol_type);
			rle_ctx_set_proto_type(rle_ctx, protocol_type);
			ptype_length = RLE_PROTO_TYPE_FIELD_SIZE_UNCOMP;
		}
	} else {
		/* no protocol type in this packet */
		proto_type_supp = RLE_T_PROTO_TYPE_SUPP;
	}
	rle_ctx_set_proto_type(rle_ctx, protocol_type);

	/* update total header size */
	size_header += ptype_length;

	/* initialize payload pointers */
	rle_hdr->ptrs.start = NULL;
	rle_hdr->ptrs.end = NULL;
	/* fill RLE complete header */
	rle_hdr->header.head.b.start_ind = 1;
	rle_hdr->header.head.b.end_ind = 1;
	rle_header_all_set_packet_length(&(rle_hdr->header.head), data_length);
	SET_PROTO_TYPE_SUPP(rle_hdr->header.head.b.LT_T_FID, proto_type_supp);

	/* fill label_type field accordingly to the
	 * given protocol type (signal or implicit/indicated
	 * by the NCC */
	if (protocol_type == RLE_PROTO_TYPE_SIGNAL_UNCOMP) {
		SET_LABEL_TYPE(rle_hdr->header.head.b.LT_T_FID, RLE_LT_PROTO_SIGNAL); /* RCS2 requirement */
	} else if (proto_type_supp == RLE_T_PROTO_TYPE_SUPP) {
		SET_LABEL_TYPE(rle_hdr->header.head.b.LT_T_FID, RLE_LT_IMPLICIT_PROTO_TYPE);
	} else {
		SET_LABEL_TYPE(rle_hdr->header.head.b.LT_T_FID, RLE_T_PROTO_TYPE_NO_SUPP);
	}

	/* update rle configuration */
	/* rle_conf_set_ptype_suppression(rle_conf, proto_type_supp); */

	/* set start & end PDU data pointers */
	rle_hdr->ptrs.start = (char *)data_buffer;
	rle_hdr->ptrs.end = (char *)((char *)data_buffer + data_length);
	/* update rle context */
	rle_ctx_set_end_address(rle_ctx,
	                        (char *)((char *)rle_ctx->buf + size_header));
	rle_ctx_set_is_fragmented(rle_ctx, C_FALSE);
	rle_ctx_set_frag_counter(rle_ctx, 1);
	rle_ctx_set_nb_frag_pdu(rle_ctx, 1);
	rle_ctx_set_use_crc(rle_ctx, C_FALSE);
	rle_ctx_set_pdu_length(rle_ctx, data_length);
	rle_ctx_set_remaining_pdu_length(rle_ctx, data_length);
	rle_ctx_set_alpdu_length(rle_ctx, data_length + ptype_length);
	rle_ctx_set_remaining_alpdu_length(rle_ctx, data_length + ptype_length);
	/* RLE packet length is the sum of packet label,
	 * protocol type & payload length */
	rle_ctx_set_rle_length(rle_ctx,
	                       (data_length + ptype_length), ptype_length);
	label_type = GET_LABEL_TYPE(rle_hdr->header.head.b.LT_T_FID);
	rle_ctx_set_label_type(rle_ctx, label_type);
	rle_ctx_set_qos_tag(rle_ctx, 0); /* TODO update */

	return C_OK;
}

int encap_encapsulate_pdu(struct rle_ctx_management *rle_ctx, struct rle_configuration *rle_conf,
                          void *pdu_buffer, size_t pdu_length,
                          uint16_t protocol_type)
{
#ifdef DEBUG
	PRINT("DEBUG %s %s:%s:%d:\n",
	      MODULE_NAME,
	      __FILE__, __func__, __LINE__);
#endif
	rle_ctx_incr_counter_in(rle_ctx);
	rle_ctx_incr_counter_bytes_in(rle_ctx, pdu_length);

	if (encap_check_pdu_validity(pdu_length) == C_ERROR) {
		rle_ctx_incr_counter_dropped(rle_ctx);
		rle_ctx_incr_counter_bytes_dropped(rle_ctx, pdu_length);
		return C_ERROR;
	}

	if (create_header(rle_ctx, rle_conf,
	                  pdu_buffer, pdu_length,
	                  protocol_type) == C_ERROR) {
		rle_ctx_incr_counter_dropped(rle_ctx);
		rle_ctx_incr_counter_bytes_dropped(rle_ctx, pdu_length);
		return C_ERROR;
	}

	/* set PDU buffer address to the rle_ctx ptr */
	rle_ctx->pdu_buf = pdu_buffer;

	return C_OK;
}

int encap_check_pdu_validity(const size_t pdu_length)
{
#ifdef DEBUG
	PRINT("DEBUG %s %s:%s:%d:\n",
	      MODULE_NAME,
	      __FILE__, __func__, __LINE__);
#endif

	if (pdu_length > RLE_MAX_PDU_SIZE) {
		PRINT("ERROR %s %s:%s:%d: PDU too large for RL Encapsulation,"
		      " size [%zu]\n",
		      MODULE_NAME,
		      __FILE__, __func__, __LINE__,
		      pdu_length);
		return C_ERROR;
	}

	return C_OK;
}
