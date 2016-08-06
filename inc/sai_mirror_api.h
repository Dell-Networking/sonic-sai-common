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
* @file sai_mirror_api.h
*
* @brief This file contains the prototype declarations for common mirror
*        APIs
*
*************************************************************************/
#ifndef __SAI_MIRROR_API_H__
#define __SAI_MIRROR_API_H__

#include "sai_mirror_defs.h"

/*
 * Allocate memory for mirror session node
 * return Allocated memory of type (sai_mirror_session_info_t *)
 */
sai_mirror_session_info_t *sai_mirror_session_node_alloc (void);

/*
 * Free memory for mirror session node
 * param - p_session_info pointer to be freed
 */
void sai_mirror_session_node_free (sai_mirror_session_info_t *p_session_info);

/*
 * Allocate memory for source port node
 * return Allocated memory of type (sai_mirror_port_info_t *)
 */
sai_mirror_port_info_t * sai_source_port_node_alloc (void);

/*
 * Free memory for source port node
 * param - p_source_info pointer to be freed
 */
void sai_source_port_node_free (sai_mirror_port_info_t *p_source_info);

/*
 * Attach a source port to a mirror session
 * param - session_id Mirror session Id
 * param - mirror_port_id Mirror source port Id
 * param - direction Mirroring direction of the source port
 * return - SAI_STATUS_SUCCESS if successful otherwise appropriate error code
 */
sai_status_t sai_mirror_session_port_add  (sai_object_id_t session_id,
                                           sai_object_id_t mirror_port_id,
                                           sai_mirror_direction_t direction);

/*
 * Detach a source port to a mirror session
 * param - session_id Mirror session Id
 * param - mirror_port_id Mirror source port Id
 * param - direction Mirroring direction of the source port
 * return - SAI_STATUS_SUCCESS if successful otherwise appropriate error code
 */
sai_status_t sai_mirror_session_port_remove (sai_object_id_t session_id,
                                             sai_object_id_t mirror_port_id,
                                             sai_mirror_direction_t direction);

/*
 * Handle port mirroring for a port
 * param - port_id Mirror source port Id
 * param - attr Attribute value to be set (Mirror session list) on the port
 * param - mirror_direction Mirroring direction of the source port
 * return - SAI_STATUS_SUCCESS if successful otherwise appropriate error code
 */
sai_status_t sai_mirror_handle_per_port (sai_object_id_t port_id,
                                         const sai_attribute_t *attr,
                                         sai_mirror_direction_t mirror_direction);

/*
 * Mutex lock for SAI Mirror Session Datastructure
 */
void sai_mirror_lock(void);

/*
 * Mutex Unlock for SAI Mirror Session Datastructure
 */
void sai_mirror_unlock(void);

/*
 * Mutex lock initialization for SAI Mirror Session Datastructure
 */
void sai_mirror_mutex_lock_init (void);

/*
 * Checks if Mirror session is available
 * param - mirror_object Mirror session Id
 * return - true if mirror object is valid false otherwise
 */
bool sai_mirror_is_valid_mirror_session (sai_object_id_t mirror_object);

/*
 * Retrieves the mirror datastructure
 */
rbtree_handle sai_mirror_sessions_db_get (void);

void sai_mirror_dump (void);
#endif /* __SAI_MIRROR_API_H__ */
