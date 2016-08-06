/************************************************************************
* LEGALESE:   "Copyright (c); 2015, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file sai_hostif_main.h
*
* @brief This private header file contains SAI hostif data structure
* definitions and static inline functions.
*
*******************************************************************************/

#ifndef _SAI_HOSTIF_MAIN_H_
#define _SAI_HOSTIF_MAIN_H_

#include "std_rbtree.h"
#include "std_llist.h"
#include "std_assert.h"
#include "std_error_codes.h"
#include "std_type_defs.h"
#include "sai_hostif_common.h"

#include "saitypes.h"
#include "saistatus.h"
#include "saiswitch.h"
#include <string.h>

#define DN_HOSTIF_MAX_TRAP_GROUPS          (128)
#define DN_HOSTIF_DEFAULT_MIN_PRIO         (0)
#define DN_HOSTIF_DEFAULT_TRAP_GROUP_PRIO  (0)
#define DN_HOSTIF_DEFAULT_TRAP_GROUP_ATTRS (3)
typedef struct _dn_sai_hostif_pkt_attr_property_t {
    sai_attr_id_t attr_id;
    bool valid_on_send;
    bool mandatory_on_send;
} dn_sai_hostif_pkt_attr_property_t;

typedef sai_status_t (*dn_sai_hostif_check_attribute_fn) (
                            const sai_attribute_t *attr);

typedef struct _dn_sai_hostif_trap_attr_property_t {
    sai_attr_id_t                    attr_id;
    dn_sai_hostif_check_attribute_fn check_attr_func;
} dn_sai_hostif_trap_attr_property_t;

typedef struct _dn_sai_hostif_trap_group_attr_property_t {
    sai_attr_id_t attr_id;
    bool mandatory_in_create;
    bool valid_in_set;
} dn_sai_hostif_trap_group_attr_property_t;

typedef struct _dn_sai_hostif_info_t {
    rbtree_handle             trap_tree;
    rbtree_handle             trap_group_tree;
    void                     *trap_group_bitmap;
    sai_object_id_t           default_trap_group_id;
    sai_hostif_trap_channel_t default_trap_channel;
    uint_t                    mandatory_send_pkt_attr_count;
    uint_t                    mandatory_trap_group_attr_count;
    uint_t                    max_pkt_attrs;
    uint_t                    max_trap_group_attrs;
    uint_t                    max_trap_attrs;
} dn_sai_hostintf_info_t;

typedef struct _dn_sai_hostif_valid_traps_t {
    sai_hostif_trap_id_t trapid;
    sai_packet_action_t def_action;
} dn_sai_hostif_valid_traps_t;

/***************STATIC INLINE FUNCTIONS********************/
static inline void dn_sai_hostif_add_trap_to_trapgroup(
                            dn_sai_trap_node_t *trap_node,
                            dn_sai_trap_group_node_t *trap_group)
{
    STD_ASSERT(trap_group != NULL);
    STD_ASSERT(trap_node != NULL);

    std_dll_insert(&trap_group->trap_list, (std_dll *)trap_node);
    trap_group->trap_count++;
}

static inline void dn_sai_hostif_remove_trap_to_trapgroup(
                            dn_sai_trap_node_t *trap_node,
                            dn_sai_trap_group_node_t *trap_group)
{
    STD_ASSERT(trap_group != NULL);
    STD_ASSERT(trap_node != NULL);

    std_dll_remove(&trap_group->trap_list, (std_dll *)trap_node);
    trap_group->trap_count--;
}

static inline sai_status_t dn_sai_hostif_add_trapgroup(
                             rbtree_handle trap_group_tree,
                             dn_sai_trap_group_node_t * trap_group)
{
    STD_ASSERT(trap_group != NULL);
    STD_ASSERT(trap_group_tree != NULL);
    return (std_rbtree_insert(trap_group_tree, trap_group)
            == STD_ERR_OK ? SAI_STATUS_SUCCESS: SAI_STATUS_FAILURE);
}

static inline dn_sai_trap_group_node_t *dn_sai_hostif_remove_trapgroup(
                              rbtree_handle trap_group_tree,
                              dn_sai_trap_group_node_t * trap_group)
{
    STD_ASSERT(trap_group != NULL);
    STD_ASSERT(trap_group_tree != NULL);
    return ((dn_sai_trap_group_node_t *)std_rbtree_remove(
                                     trap_group_tree, trap_group));
}

static inline dn_sai_trap_group_node_t *dn_sai_hostif_find_trapgroup(
                                                rbtree_handle trap_group_tree,
                                                sai_object_id_t trap_group_id)
{
    STD_ASSERT(trap_group_tree != NULL);
    dn_sai_trap_group_node_t trap_group;

    memset(&trap_group, 0, sizeof(dn_sai_trap_group_node_t));
    trap_group.key.trap_group_id = trap_group_id;
    return ( dn_sai_trap_group_node_t *) std_rbtree_getexact(trap_group_tree,
                                                             &trap_group);
}

static inline dn_sai_trap_node_t * dn_sai_hostif_find_trap_node(
                                                          rbtree_handle trap_tree,
                                                          sai_hostif_trap_id_t trapid)
{
    STD_ASSERT(trap_tree != NULL);
    dn_sai_trap_node_t temp_node;

    memset(&temp_node, 0, sizeof(dn_sai_trap_node_t));
    temp_node.key.trap_id = trapid;

    return (dn_sai_trap_node_t *) std_rbtree_getexact(trap_tree, &temp_node);
}

/***************Function prototypes*************************/
void sai_hostif_lock();
void sai_hostif_unlock();
dn_sai_hostintf_info_t * dn_sai_hostintf_get_info();
bool dn_sai_hostif_is_valid_trap_id(sai_hostif_trap_id_t trap_id);
sai_packet_action_t dn_sai_hostif_get_default_action(sai_hostif_trap_id_t trap_id);
sai_status_t dn_sai_hostif_validate_trapgroup_oid(const sai_attribute_t *attr);
sai_status_t dn_sai_hostif_validate_portlist(const sai_attribute_t *attr);
sai_status_t dn_sai_hostif_validate_action(const sai_attribute_t *attr);
dn_sai_trap_group_node_t *dn_sai_hostif_find_trapgroup_by_queue(uint_t cpu_queue);
#endif /*_SAI_HOSTIF_MAIN_H_*/
