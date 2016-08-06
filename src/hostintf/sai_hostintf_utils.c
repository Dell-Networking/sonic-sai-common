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
* @file sai_hostintf_utils.c
*
* @brief This file contains SAI Host Interface utility functions.
*
*************************************************************************/
#include "std_type_defs.h"
#include "std_mutex_lock.h"
#include "sai_hostif_main.h"
#include "sai_hostif_api.h"
#include "sai_gen_utils.h"
#include "sai_hostif_common.h"

#include "saihostintf.h"
#include "saiswitch.h"
#include "saitypes.h"
#include "saistatus.h"

#include<inttypes.h>

static const dn_sai_hostif_valid_traps_t valid_traps[] = {
    {SAI_HOSTIF_TRAP_ID_STP, SAI_PACKET_ACTION_DROP},
    {SAI_HOSTIF_TRAP_ID_LACP, SAI_PACKET_ACTION_DROP},
    {SAI_HOSTIF_TRAP_ID_EAPOL, SAI_PACKET_ACTION_DROP},
    {SAI_HOSTIF_TRAP_ID_LLDP, SAI_PACKET_ACTION_DROP},
    {SAI_HOSTIF_TRAP_ID_PVRST, SAI_PACKET_ACTION_DROP},
    {SAI_HOSTIF_TRAP_ID_IGMP_TYPE_QUERY, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_IGMP_TYPE_LEAVE, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V1_REPORT, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V2_REPORT, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V3_REPORT, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_SAMPLEPACKET, SAI_PACKET_ACTION_TRAP},
    {SAI_HOSTIF_TRAP_ID_ARP_REQUEST, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_ARP_RESPONSE, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_DHCP, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_OSPF, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_PIM, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_VRRP, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_BGP, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_DHCPV6, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_OSPFV6, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_VRRPV6, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_BGPV6, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_IPV6_NEIGHBOR_DISCOVERY, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_IPV6_MLD_V1_V2, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_IPV6_MLD_V1_REPORT, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_IPV6_MLD_V1_DONE, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_MLD_V2_REPORT, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_ID_L3_MTU_ERROR, SAI_PACKET_ACTION_TRAP},
    {SAI_HOSTIF_TRAP_ID_TTL_ERROR, SAI_PACKET_ACTION_TRAP}
};

static std_mutex_lock_create_static_init_fast(sai_hostintf_lock);

void sai_hostif_lock()
{
    std_mutex_lock (&sai_hostintf_lock);
}

void sai_hostif_unlock()
{
    std_mutex_unlock (&sai_hostintf_lock);
}

bool dn_sai_hostif_is_valid_trap_id(
                            sai_hostif_trap_id_t trap_id)
{
    uint_t index = 0;
    static const uint_t max_trap_count = sizeof(valid_traps) /
                                         sizeof(valid_traps[0]);

    for (index = 0; index < max_trap_count; index++) {
        if (trap_id == valid_traps[index].trapid) {
            return true;
        }
    }
    return false;
}

sai_packet_action_t dn_sai_hostif_get_default_action(
                               sai_hostif_trap_id_t trap_id)
{
    uint_t index = 0;
    static const uint_t max_trap_count = sizeof(valid_traps) /
                                         sizeof(valid_traps[0]);

    for (index = 0; index < max_trap_count; index++) {
        if (trap_id == valid_traps[index].trapid) {
            return valid_traps[index].def_action;
        }
    }
    return SAI_PACKET_ACTION_FORWARD;
}

sai_status_t sai_hostif_get_default_trap_group(sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    dn_sai_hostintf_info_t *hostif_info = dn_sai_hostintf_get_info();

    SAI_HOSTIF_LOG_TRACE("Get default switch trap group");
    sai_hostif_lock();
    attr->value.oid = hostif_info->default_trap_group_id;
    sai_hostif_unlock();

    return SAI_STATUS_SUCCESS;
}

sai_status_t dn_sai_hostif_validate_trapgroup_oid(const sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    dn_sai_hostintf_info_t *hostif_info = dn_sai_hostintf_get_info();

    if ((SAI_NULL_OBJECT_ID != attr->value.oid) &&
        (NULL == dn_sai_hostif_find_trapgroup(hostif_info->trap_group_tree,
                                              attr->value.oid))) {
        SAI_HOSTIF_LOG_ERR("Trap group %"PRIu64" not present",
                           attr->value.oid);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t dn_sai_hostif_validate_portlist(const sai_attribute_t *attr)
{
    unsigned int invalid_idx = 0;
    STD_ASSERT(attr != NULL);

    if(!sai_is_port_list_valid(&attr->value.objlist,&invalid_idx)) {
        SAI_HOSTIF_LOG_ERR("Invalid port in object list");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t dn_sai_hostif_validate_action(const sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);

    if ((SAI_PACKET_ACTION_DROP == attr->value.s32) ||
        (SAI_PACKET_ACTION_TRAP == attr->value.s32) ||
        (SAI_PACKET_ACTION_LOG == attr->value.s32) ||
        (SAI_PACKET_ACTION_FORWARD == attr->value.s32)) {
        return SAI_STATUS_SUCCESS;
    }

    SAI_HOSTIF_LOG_ERR("Invalid action %d", attr->value.s32);
    return SAI_STATUS_INVALID_ATTR_VALUE_0;
}

dn_sai_trap_group_node_t *dn_sai_hostif_find_trapgroup_by_queue(uint_t cpu_queue)
{
    dn_sai_hostintf_info_t *hostif_info = dn_sai_hostintf_get_info();

    dn_sai_trap_group_node_t *trap_group = (dn_sai_trap_group_node_t *)
                         std_rbtree_getfirst(hostif_info->trap_group_tree);

    while (trap_group != NULL) {
        if (trap_group->cpu_queue == cpu_queue) {
            SAI_HOSTIF_LOG_TRACE("Found trap group with queue = %u", cpu_queue);
            return trap_group;
        }
        trap_group = (dn_sai_trap_group_node_t *)std_rbtree_getnext(
                                               hostif_info->trap_group_tree, trap_group);
    }
    return trap_group;
}

