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
* @file  sai_qos_port.c
*
* @brief This file contains function definitions for SAI Qos port
*        Initialization functionality API implementation and
*        port specific qos settings.
*
*************************************************************************/
#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_buffer_util.h"
#include "sai_qos_api_utils.h"
#include "sai_port_utils.h"
#include "sai_qos_mem.h"
#include "sai_common_infra.h"

#include "saistatus.h"

#include "std_assert.h"

#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

static sai_status_t sai_qos_port_node_insert_into_port_db (sai_object_id_t port_id,
                                                dn_sai_qos_port_t *p_qos_port_node)
{
    sai_port_application_info_t  *p_port_node = NULL;

    STD_ASSERT (p_qos_port_node != NULL);

    p_port_node = sai_port_application_info_create_and_get (port_id);

    if (p_port_node == NULL) {
        SAI_QOS_LOG_ERR ("Port Info is NULL for port 0x%"PRIx64"", port_id);
        return SAI_STATUS_FAILURE;
    }

    p_port_node->qos_port_db = (void *)(p_qos_port_node);

    return SAI_STATUS_SUCCESS;
}

static void sai_qos_port_node_remove_from_port_db (sai_object_id_t port_id)
{
    sai_port_application_info_t  *p_port_node = NULL;

    p_port_node = sai_port_application_info_get (port_id);

    STD_ASSERT (p_port_node != NULL);

    STD_ASSERT (p_port_node->qos_port_db != NULL);

    p_port_node->qos_port_db = NULL;

    sai_port_application_info_remove(p_port_node);
}

static sai_status_t sai_qos_port_sched_group_dll_head_init (
                              dn_sai_qos_port_t *p_qos_port_node)
{
    uint_t        h_levels_per_port = sai_switch_max_hierarchy_levels_get ();
    uint_t        level = 0;

    if (! sai_qos_is_hierarchy_qos_supported ())
        return SAI_STATUS_SUCCESS;

    p_qos_port_node->sched_group_dll_head =
        ((std_dll_head *) calloc (h_levels_per_port, sizeof (std_dll_head)));

    if (NULL == p_qos_port_node->sched_group_dll_head) {
        SAI_QOS_LOG_CRIT ("Port sched group list memory allocation failed.");
        return SAI_STATUS_NO_MEMORY;
    }

    for (level = 0; level < h_levels_per_port; level++) {
        std_dll_init ((std_dll_head *)&p_qos_port_node->sched_group_dll_head[level]);
    }

    return SAI_STATUS_SUCCESS;
}

static void sai_qos_port_sched_group_dll_head_free (
                              dn_sai_qos_port_t *p_qos_port_node)
{
    if (! sai_qos_is_hierarchy_qos_supported ())
        return;

    STD_ASSERT(p_qos_port_node != NULL);

    free (p_qos_port_node->sched_group_dll_head);
    p_qos_port_node->sched_group_dll_head = NULL;
}

static sai_status_t sai_qos_port_node_init (dn_sai_qos_port_t *p_qos_port_node)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_qos_port_node != NULL);

    p_qos_port_node->drop_type = SAI_DROP_TYPE_TAIL;

    p_qos_port_node->is_app_hqos_init = false;

    std_dll_init (&p_qos_port_node->queue_dll_head);
    std_dll_init (&p_qos_port_node->pg_dll_head);

    sai_rc = sai_qos_port_sched_group_dll_head_init (p_qos_port_node);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_QOS_LOG_ERR ("Failed to init scheduler group list.");
    }

    return sai_rc;
}

static bool sai_qos_port_is_in_use (dn_sai_qos_port_t *p_qos_port_node)
{

    STD_ASSERT(p_qos_port_node != NULL);

    /* TODO
     * - Verify WRED/Scheduler/Policer/maps profile is assigned to port
     * - Verify scheduler groups associated etc.
     */
    return false;
}

static void sai_qos_port_free_resources (dn_sai_qos_port_t *p_qos_port_node)
{
    if (p_qos_port_node == NULL) {
        return;
    }

    /* TODO - Delete Port node from the WRED's port list,
     * if it was already added.
     *  sai_qos_wred_port_list_update (p_qos_port_node, false); */

    sai_qos_port_sched_group_dll_head_free (p_qos_port_node);

    sai_qos_port_node_free (p_qos_port_node);

    return;
}

sai_status_t sai_qos_port_queue_list_update (dn_sai_qos_queue_t *p_queue_node,
                                             bool is_add)
{
    sai_object_id_t     port_id = 0;
    dn_sai_qos_port_t  *p_qos_port_node = NULL;

    STD_ASSERT (p_queue_node != NULL);

    port_id = p_queue_node->port_id;

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        SAI_QOS_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                         port_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (is_add) {
        std_dll_insertatback (&p_qos_port_node->queue_dll_head,
                              &p_queue_node->port_dll_glue);
    } else {
        std_dll_remove (&p_qos_port_node->queue_dll_head,
                        &p_queue_node->port_dll_glue);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_port_sched_group_list_update (
                  dn_sai_qos_sched_group_t *p_sg_node, bool is_add)
{
    sai_object_id_t     port_id = 0;
    dn_sai_qos_port_t  *p_qos_port_node = NULL;
    uint_t              level = 0;

    STD_ASSERT (p_sg_node != NULL);

    port_id = p_sg_node->port_id;
    level   = p_sg_node->hierarchy_level;

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        SAI_QOS_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                         port_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (is_add) {
        std_dll_insertatback (&p_qos_port_node->sched_group_dll_head [level],
                              &p_sg_node->port_dll_glue);
    } else {
        std_dll_remove (&p_qos_port_node->sched_group_dll_head [level],
                        &p_sg_node->port_dll_glue);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_port_global_init (sai_object_id_t port_id)
{
    sai_status_t       sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_port_t  *p_qos_port_node = NULL;

    SAI_QOS_LOG_TRACE ("Qos Port 0x%"PRIx64" Global Init.", port_id);

    sai_qos_lock ();

    do {
        p_qos_port_node = sai_qos_port_node_alloc ();

        if (NULL == p_qos_port_node) {
            SAI_QOS_LOG_ERR ("Qos Port memory allocation failed.");
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        p_qos_port_node->port_id = port_id;

        sai_rc = sai_qos_npu_api_get()->qos_port_init (p_qos_port_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_ERR ("Qos Port init failed in NPU.");
            break;
        }

        SAI_QOS_LOG_TRACE ("Port Init completed in NPU.");

        sai_rc = sai_qos_port_node_init (p_qos_port_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_ERR ("Failed to init qos port node init.");
            break;
        }

        sai_rc = sai_qos_port_node_insert_into_port_db (port_id,
                                                        p_qos_port_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Queue insertion to tree failed.");
            break;
        }

    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_QOS_LOG_INFO ("Qos Port 0x%"PRIx64" global init success.",
                           port_id);
    } else {
        SAI_QUEUE_LOG_ERR ("Failed qos port global init.");
        sai_qos_port_free_resources (p_qos_port_node);
    }

    sai_qos_unlock ();

    return sai_rc;
}

static sai_status_t sai_qos_port_global_deinit (sai_object_id_t port_id)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_port_t  *p_qos_port_node = NULL;

    SAI_QOS_LOG_TRACE ("Qos Port 0x%"PRIx64" Global De-Init.", port_id);

    if (! sai_is_obj_id_port (port_id)) {
        SAI_QOS_LOG_ERR ("Port 0x%"PRIx64" is not a valid port obj id.",
                           port_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_qos_lock ();

    do {
        p_qos_port_node = sai_qos_port_node_get (port_id);

        if (NULL == p_qos_port_node) {
            SAI_QOS_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                               port_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        /* TODO - Check this is needed or not
        sai_rc = sai_npu_port_deinit (p_qos_port_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_ERR ("Qos Port 0x%"PRIx64" De-init failed in NPU.",
                                port_id);
            break;
        }
        */

        sai_qos_port_node_remove_from_port_db (port_id);

        sai_qos_port_free_resources (p_qos_port_node);

    } while (0);

    sai_qos_unlock ();

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_QOS_LOG_INFO ("Qos Port 0x%"PRIx64" De-Initialized.", port_id);
    } else {
        SAI_QOS_LOG_ERR ("Failed to De-Initialize Qos Port 0x%"PRIx64".", port_id);
    }


    return sai_rc;
}

static sai_status_t sai_qos_port_handle_init_failure (sai_object_id_t port_id,
                                                      bool is_global_port_init,
                                                      bool is_port_queue_init,
                                                      bool is_port_hierarchy_init)
{

    if (is_port_hierarchy_init) {
        /* De-Initialize Existing Hierarchy on port */
        sai_qos_port_hierarchy_deinit (port_id);
    }

    if (is_port_queue_init) {
        /* De-Initialize all queues on port */
        sai_qos_port_queue_all_deinit (port_id);
    }

    if (is_global_port_init) {
        sai_qos_port_global_deinit (port_id);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_port_handle_deinit_failure (sai_object_id_t port_id)
{
    /* TODO */
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_port_init (sai_object_id_t port_id)
{
    sai_status_t       sai_rc = SAI_STATUS_SUCCESS;
    bool               is_port_global_init = false;
    bool               is_port_queue_init = false;
    bool               is_port_hierarchy_init = false;

    SAI_QOS_LOG_TRACE ("Qos Port 0x%"PRIx64" Init.", port_id);

    do {
        sai_rc = sai_qos_port_global_init (port_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_CRIT ("SAI QOS port 0x%"PRIx64" global init failed.",
                               port_id);
            break;
        }

        is_port_global_init = true;

        /* Initialize all queues on port */
        sai_rc = sai_qos_port_queue_all_init (port_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_CRIT ("SAI QOS port 0x%"PRIx64" queue all init failed.",
                               port_id);
            break;
        }

        is_port_queue_init = true;

        if (sai_qos_is_hierarchy_qos_supported ()) {
            /* Initialize Default Scheduler groups on port */
            sai_rc = sai_qos_port_default_hierarchy_init (port_id);

            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QOS_LOG_CRIT ("SAI QOS port 0x%"PRIx64" default hierarchy "
                                   "init failed.", port_id);
                break;
            }
            is_port_hierarchy_init = true;
        }
    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_QOS_LOG_INFO ("Qos Port 0x%"PRIx64" init success.", port_id);
    } else {
        SAI_QUEUE_LOG_ERR ("Failed Qos Port 0x%"PRIx64" init.", port_id);

        sai_qos_port_handle_init_failure (port_id, is_port_global_init,
                                          is_port_queue_init,
                                          is_port_hierarchy_init);

    }
    return sai_rc;
}

static sai_status_t sai_qos_port_deinit (sai_object_id_t port_id)
{
    sai_status_t       sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_port_t *p_qos_port_node = NULL;

    SAI_QOS_LOG_TRACE ("Qos Port 0x%"PRIx64" De-Init.", port_id);

    if (! sai_is_obj_id_port (port_id)) {
        SAI_QOS_LOG_ERR ("Port 0x%"PRIx64" is not a valid port obj id.",
                           port_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    SAI_QOS_LOG_TRACE ("Qos Port De-Init. Port ID 0x%"PRIx64".", port_id);

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        SAI_QOS_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                         port_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (sai_qos_port_is_in_use (p_qos_port_node)) {
        SAI_QOS_LOG_ERR ("Port 0x%"PRIx64" can't be deleted, since port is "
                         "in use.", port_id);

        return SAI_STATUS_OBJECT_IN_USE;
    }

    do {

        if (sai_qos_is_hierarchy_qos_supported ()) {
            /* De-Initialize Existing Hierarchy on port */
            sai_rc = sai_qos_port_hierarchy_deinit (port_id);

            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QOS_LOG_ERR ("SAI QOS port 0x%"PRIx64" hierarchy de-init "
                                 "failed.", port_id);
                break;
            }
        }

        /* De-Initialize all queues on port */
        sai_rc = sai_qos_port_queue_all_deinit (port_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_ERR ("SAI QOS port 0x%"PRIx64" queue all de-init failed.",
                             port_id);
            break;
        }

        sai_rc = sai_qos_port_global_deinit (port_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_ERR ("SAI QOS port 0x%"PRIx64" global de-init failed.",
                             port_id);
            break;
        }
    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_QOS_LOG_INFO ("Qos Port 0x%"PRIx64" de-init success.", port_id);
    } else {
        SAI_QUEUE_LOG_ERR ("Failed qos port 0x%"PRIx64" de-init.", port_id);
        sai_qos_port_handle_deinit_failure (port_id);
    }

    return sai_rc;
}

sai_status_t sai_qos_port_all_init (void)
{
    sai_status_t    sai_rc = SAI_STATUS_FAILURE;
    sai_port_info_t *port_info = NULL;
    sai_object_id_t cpu_port_id = 0;

    SAI_QOS_LOG_TRACE ("Port All Qos Init.");

    cpu_port_id = sai_switch_cpu_port_obj_id_get();

    sai_rc = sai_qos_port_init (cpu_port_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_QOS_LOG_CRIT ("SAI QOS CPU Port 0x%"PRIx64" init failed.",
                          cpu_port_id);
        return sai_rc;
    }

    for (port_info = sai_port_info_getfirst(); (port_info != NULL);
         port_info = sai_port_info_getnext(port_info)) {

        /**
         * Reenable this once fanout problem is resolved.
        if (! sai_is_port_valid (port_info->sai_port_id)) {
            continue;
        }
        */

        sai_rc = sai_qos_port_init (port_info->sai_port_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_CRIT ("SAI QOS Port 0x%"PRIx64" init failed.",
                               port_info->sai_port_id);
            break;
        }
    }

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_QOS_LOG_INFO ("Port All Qos Init success.");
    } else {
        SAI_QUEUE_LOG_ERR ("Failed Port All Qos Init.");
    }

    return sai_rc;
}

sai_status_t sai_qos_port_all_deinit (void)
{
    sai_status_t       sai_rc = SAI_STATUS_FAILURE;
    dn_sai_qos_port_t *p_qos_port_node = NULL;
    dn_sai_qos_port_t *p_next_qos_port_node = NULL;
    sai_object_id_t    cpu_port_id = 0;
    sai_object_id_t    port_id = 0;

    SAI_QOS_LOG_TRACE ("Port All Qos De-Init.");

    cpu_port_id = sai_switch_cpu_port_obj_id_get();

    sai_rc = sai_qos_port_deinit (cpu_port_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_QOS_LOG_CRIT ("SAI QOS CPU Port 0x%"PRIx64" de-init failed.",
                          cpu_port_id);
        return sai_rc;
    }

    for (p_qos_port_node = sai_qos_port_node_get_first();
        (p_qos_port_node != NULL);
         p_qos_port_node = p_next_qos_port_node) {

        p_next_qos_port_node = sai_qos_port_node_get_next (p_qos_port_node);
        /**
         * Reenable this once fanout problem is resolved.
        if (! sai_is_port_valid (port_info->sai_port_id)) {
            continue;
        }
        */


        port_id = p_qos_port_node->port_id;

        sai_rc = sai_qos_port_deinit (port_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_ERR ("SAI QOS Port 0x%"PRIx64" de-init failed.", port_id);
            break;
        }
    }

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_QOS_LOG_INFO ("Port All Qos De-Init success.");
    } else {
        SAI_QUEUE_LOG_ERR ("Failed Port All Qos De-Init.");
    }

    return sai_rc;
}
sai_status_t sai_qos_port_buffer_profile_set (sai_object_id_t port_id,
                                              sai_object_id_t profile_id)
{
    dn_sai_qos_port_t *p_port_node = NULL;
    sai_status_t sai_rc;

    sai_qos_lock();
    p_port_node = sai_qos_port_node_get(port_id);

    if (p_port_node == NULL) {
        sai_qos_unlock ();
        SAI_BUFFER_LOG_ERR ("Object id 0x%"PRIx64" not found", port_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    sai_rc = sai_qos_obj_update_buffer_profile(port_id, profile_id);

    sai_qos_unlock();
    return sai_rc;
}
