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
* @file sai_stp_api.h
*
* @brief This file contains the prototype declarations for common stp
*        APIs
*
*************************************************************************/
#ifndef __SAI_STP_API_H__
#define __SAI_STP_API_H__

#include "sai_stp_defs.h"

/*
 * Allocate memory for stp info node
 */
dn_sai_stp_info_t *sai_stp_info_node_alloc (void);

/*
 * Free memory for stp info node
 */
void sai_stp_info_node_free (dn_sai_stp_info_t *p_info_info);

/*
 * Allocate memory for vlan node
 */
sai_vlan_id_t * sai_stp_vlan_node_alloc (void);

/*
 * Free memory for vlan node
 */
void sai_stp_vlan_node_free (sai_vlan_id_t *p_vlan_node);

/*
 * Mutex lock for SAI STP
 */
void sai_stp_lock(void);

/*
 * Mutex Unlock for SAI STP
 */
void sai_stp_unlock(void);

/*
 * Mutex lock initialization for SAI STP
 */
void sai_stp_mutex_lock_init (void);

/*
 * Remove the association of any stp instance to the give vlan
 */
sai_status_t sai_stp_vlan_destroy (sai_vlan_id_t vlan_id);

/*
 * Update the stp instance to the given vlan
 */
sai_status_t sai_stp_vlan_handle (sai_vlan_id_t vlan_id, sai_object_id_t stp_inst_id);

/*
 * Update the default stp instance association to the given vlan
 */
sai_status_t sai_stp_default_stp_vlan_update (sai_vlan_id_t vlan_id);

/*
 * Retrieve the stp instance of the given vlan
 */
sai_status_t sai_stp_vlan_stp_get (sai_vlan_id_t vlan_id, sai_object_id_t *p_stp_id);

/*
 * Retrieve the default stp instance
 */
sai_status_t sai_l2_stp_default_instance_id_get (sai_attribute_t *attr);

/*
 * Software cache dump
 */
void sai_stp_dump(void);

#endif /* __SAI_STP_API_H__ */
