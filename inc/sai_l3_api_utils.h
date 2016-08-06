/************************************************************************
* LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file sai_l3_api_utils.h
*
* @brief This file contains macro definitions and function prototypes
* used in the SAI L3 API implementation.
*
*************************************************************************/
#ifndef __SAI_L3_API_UTILS_H__
#define __SAI_L3_API_UTILS_H__

#include "saiswitch.h"
#include "sairouterintf.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_struct_utils.h"
#include "std_bit_masks.h"
#include "std_type_defs.h"
#include "sai_l3_common.h"
#include "sai_fdb_common.h"
#include "sai_lag_callback.h"

/*
 * Virtual Router functionality related macros.
 */
#define SAI_FIB_VRF_MAX_ATTR_COUNT         (5)
#define SAI_FIB_VRF_MANDATORY_ATTR_COUNT   (0)

#define SAI_FIB_RDX_MAX_NAME_LEN           (64)

/*
 * Router Interface functionality related macros.
 */
#define SAI_FIB_RIF_MAX_ATTR_COUNT         (8)
#define SAI_FIB_RIF_MANDATORY_ATTR_COUNT   (3)
#define SAI_FIB_RIF_TYPE_MAX               (SAI_ROUTER_INTERFACE_TYPE_VLAN)
#define SAI_FIB_RIF_TYPE_NONE              (SAI_FIB_RIF_TYPE_MAX + 1)

/*
 * Route functionality related macros.
 */
#define SAI_FIB_ROUTE_MAX_ATTR_COUNT       (4)
#define SAI_FIB_ROUTE_DFLT_PKT_ACTION      (SAI_PACKET_ACTION_FORWARD)
#define SAI_FIB_ROUTE_DFLT_TRAP_PRIO       (0)

#define SAI_FIB_ROUTE_TREE_KEY_SIZE \
        (sizeof (sai_fib_route_key_t) * BITS_PER_BYTE)


static inline uint_t sai_fib_route_key_len_get (uint_t prefix_len)
{
    return (((STD_STR_SIZE_OF(sai_ip_address_t, addr_family)) * BITS_PER_BYTE)
            + prefix_len);
}

/* Util function to update the next hop node owner_flag field */
static inline void sai_fib_set_next_hop_owner (sai_fib_nh_t *p_next_hop,
                                               sai_fib_nh_owner_flag flag)
{
    if (p_next_hop) {

        p_next_hop->owner_flag |= (0x1 << flag);
    }
}

static inline void sai_fib_reset_next_hop_owner (sai_fib_nh_t *p_next_hop,
                                                 sai_fib_nh_owner_flag flag)
{
    if (p_next_hop) {

        p_next_hop->owner_flag &= ~(0x1 << flag);
    }
}

sai_status_t sai_fib_next_hop_ip_address_validate (
                                            const sai_ip_address_t *p_ip_addr);

sai_status_t sai_fib_next_hop_rif_set (sai_fib_nh_t *p_nh_info,
                                       const sai_attribute_value_t *p_value,
                                       bool is_create_req);

sai_fib_nh_t *sai_fib_ip_next_hop_node_create (sai_object_id_t vrf_id,
                                               sai_fib_nh_t *p_nh_info,
                                               sai_status_t *p_status);

sai_status_t sai_fib_check_and_delete_ip_next_hop_node (
                                            sai_object_id_t vrf_id,
                                            sai_fib_nh_t *p_nh_node);

void sai_fib_next_hop_log_error (sai_fib_nh_t *p_next_hop,
                                 const char *p_error_str);

void sai_fib_next_hop_log_trace (sai_fib_nh_t *p_next_hop,
                                 const char *p_trace_str);

sai_status_t sai_fib_vrf_rif_list_update (
                                 sai_fib_router_interface_t *p_rif_node,
                                 bool is_add);

sai_status_t sai_router_ecmp_max_paths_set (uint_t max_paths);

sai_status_t sai_rif_lag_callback (sai_object_id_t lag_id, sai_object_id_t rif_id,
                                   const sai_object_list_t *port_list,
                                   sai_lag_operation_t lag_operation);

sai_status_t sai_neighbor_fdb_callback (uint_t num_upd,
                                        sai_fdb_notification_data_t *fdb_upd);


#endif /* __SAI_L3_API_UTILS_H__ */
