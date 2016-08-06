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
* @file sai_qos_api_utils.h
*
* @brief This file contains macro definitions and function prototypes
* used in the SAI Qos API implementation.
*
*************************************************************************/
#ifndef __SAI_QOS_API_UTILS_H__
#define __SAI_QOS_API_UTILS_H__

#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "std_config_node.h"
#include "sai_common_utils.h"

#include "saitypes.h"
#include "saistatus.h"

#define SAI_QOS_MAPS_MAX_ATTR_COUNT               (2)
#define SAI_QOS_MAX_QUEUE_IDX_SEPARATE            (2)
#define SAI_QOS_MAX_QUEUE_IDX_COMBINED            (1)
#define SAI_QOS_DLFT_SCHED_GROUPS_PER_LEVEL       (1)
#define SAI_QOS_CHILD_COUNT_ONE                   (1)
#define SAI_QOS_POLICER_MAX_ATTR_COUNT            (11)
#define SAI_QOS_POLICER_TYPE_INVALID              (-1)
#define SAI_QOS_WRED_MAX_ATTR_COUNT               (14)
#define SAI_WRED_INVALID_COLOR                    (-1)
#define SAI_MAX_WRED_DROP_PROBABILITY             (100)
#define SAI_SCHED_GROUP_MANDATORY_ATTR_COUNT      (3)

sai_status_t sai_qos_port_all_init (void);

sai_status_t sai_qos_port_all_deinit (void);

sai_status_t sai_qos_port_queue_all_init (sai_object_id_t port_id);

sai_status_t sai_qos_port_queue_all_deinit (sai_object_id_t port_id);

sai_status_t sai_qos_port_queue_list_update (dn_sai_qos_queue_t *p_queue_node,
                                             bool is_add);

sai_status_t sai_qos_port_sched_groups_init (sai_object_id_t port_id);

sai_status_t sai_qos_port_sched_groups_deinit (sai_object_id_t port_id);

sai_status_t sai_qos_port_sched_group_list_update (
                                    dn_sai_qos_sched_group_t *p_sg_node,
                                    bool is_add);

sai_status_t sai_qos_port_default_hierarchy_init (sai_object_id_t port_id);

sai_status_t sai_qos_port_hierarchy_deinit (sai_object_id_t port_id);

sai_status_t sai_qos_first_free_queue_get (sai_object_id_t port_id,
                                           sai_queue_type_t queue_type,
                                           sai_object_id_t *p_queue_id);

sai_status_t sai_qos_queue_id_list_get (sai_object_id_t port_id,
                                        uint_t queue_id_list_count,
                                        sai_object_id_t *p_queue_id_list);

sai_status_t sai_qos_sched_group_id_list_per_level_get (sai_object_id_t port_id,
                                                        uint_t level,
                                                        uint_t sg_id_list_count,
                                                        sai_object_id_t *p_sg_id_list);
sai_status_t sai_qos_indexed_sched_group_id_get(sai_object_id_t port_id,
                                                uint_t level,
                                                uint_t child_idx,
                                                sai_object_id_t *p_sg_id);

uint_t sai_qos_dlft_sched_groups_per_level_get (sai_object_id_t port_id,
                                                uint_t level);

sai_status_t sai_add_child_object_to_group (sai_object_id_t sg_id,
                                             uint32_t child_count,
                                             const sai_object_id_t* child_objects);

sai_status_t sai_remove_child_object_from_group (sai_object_id_t sg_id,
                                                  uint32_t child_count,
                                                  const sai_object_id_t* child_objects);

sai_status_t sai_qos_map_type_attr_set(dn_sai_qos_map_t *p_map_node,
                                       sai_qos_map_type_t map_type);

void sai_qos_map_free_resources(dn_sai_qos_map_t *p_map_node);

sai_status_t sai_qos_map_on_port_set(sai_object_id_t port_id,
                                     const sai_attribute_t *attr,
                                     sai_qos_map_type_t map_type);

sai_status_t sai_qos_map_process_port_set (sai_object_id_t port_id,
                                           sai_object_id_t map_id,
                                           sai_qos_map_type_t map_type);

sai_status_t sai_qos_map_port_list_update(dn_sai_qos_map_t *p_map);

sai_status_t sai_qos_port_scheduler_set (sai_object_id_t port_id,
                                         const sai_attribute_t *p_attr);

sai_status_t sai_qos_queue_scheduler_set (dn_sai_qos_queue_t *p_queue_node,
                                          const sai_attribute_t *p_attr);

sai_status_t sai_qos_sched_group_scheduler_set (dn_sai_qos_sched_group_t *p_sg_node,
                                                const sai_attribute_t *p_attr);

sai_status_t sai_qos_scheduler_reapply (dn_sai_qos_scheduler_t *p_old_sched_node,
                                        dn_sai_qos_scheduler_t *p_new_sched_node);

sai_status_t sai_policer_acl_entries_update(dn_sai_qos_policer_t *p_policer,
                                            dn_sai_qos_policer_t *p_policer_new);

sai_status_t sai_port_attr_storm_control_policer_set(sai_object_id_t port_id,
                                                     const sai_attribute_t *attr);

sai_status_t sai_qos_wred_set_on_queue(sai_object_id_t queue_id,
                                       const sai_attribute_t *attr);

sai_status_t sai_port_attr_wred_profile_set(sai_object_id_t port_id,
                                            const sai_attribute_t *attr);

sai_status_t sai_switch_set_qos_default_tc(uint_t default_tc);

sai_status_t sai_qos_hierarchy_handler(std_config_node_t hqos_node,
                                       dn_sai_qos_hierarchy_t *hqos);

sai_status_t sai_qos_port_buffer_profile_set (sai_object_id_t obj_id,
                                              sai_object_id_t profile_id);

sai_status_t sai_qos_obj_update_buffer_profile (sai_object_id_t object_id,
                                               sai_object_id_t profile_id);

sai_status_t sai_buffer_init (void);

sai_status_t sai_qos_pg_init (void);

sai_status_t sai_qos_pg_attr_set (sai_object_id_t ingress_pg_id,
                                  const sai_attribute_t *attr);

sai_status_t sai_qos_pg_attr_get (sai_object_id_t ingress_pg_id,
                                  uint32_t attr_count,
                                  sai_attribute_t *attr_list);

sai_status_t sai_qos_pg_stats_get (sai_object_id_t pg_id, const
                                   sai_ingress_priority_group_stat_counter_t *counter_ids,
                                   uint32_t number_of_counters, uint64_t* counters);

sai_status_t sai_qos_pg_stats_clear (sai_object_id_t pg_id, const
                                     sai_ingress_priority_group_stat_counter_t *counter_ids,
                                     uint32_t number_of_counters);

sai_status_t sai_qos_pg_node_insert_to_tree (dn_sai_qos_pg_t *p_pg_node);

void sai_qos_pg_node_remove_from_tree (dn_sai_qos_pg_t *p_pg_node);

sai_status_t sai_qos_buffer_pool_node_insert_to_tree (dn_sai_qos_buffer_pool_t *p_buffer_pool_node);

void sai_qos_buffer_pool_node_remove_from_tree (dn_sai_qos_buffer_pool_t
                                                *p_buffer_pool_node);

sai_status_t sai_qos_buffer_profile_node_insert_to_tree (dn_sai_qos_buffer_profile_t
                                                         *p_buffer_profile_node);

sai_status_t sai_qos_buffer_profile_node_remove_from_tree (dn_sai_qos_buffer_profile_t
                                                          *p_buffer_profile_node);

sai_status_t sai_qos_update_buffer_pool_node (dn_sai_qos_buffer_pool_t
                                             *p_buf_pool_node,
                                              uint32_t attr_count,
                                              const sai_attribute_t *attr_list);
sai_status_t sai_qos_read_buffer_pool_node (dn_sai_qos_buffer_pool_t
                                            *p_buf_pool_node,
                                            uint32_t attr_count,
                                            sai_attribute_t *attr_list);

sai_status_t sai_qos_update_buffer_profile_node (dn_sai_qos_buffer_profile_t
                                                 *p_buf_profile_node,
                                                 uint32_t attr_count,
                                                 const sai_attribute_t *attr_list);

sai_status_t sai_qos_read_buffer_profile_node (dn_sai_qos_buffer_profile_t
                                               *p_buf_profile_node,
                                               uint32_t attr_count,
                                               sai_attribute_t *attr_list);

void sai_qos_init_buffer_pool_node (dn_sai_qos_buffer_pool_t *p_buf_pool_node);

sai_status_t sai_qos_obj_update_buffer_profile_node (sai_object_id_t obj_id,
                                                     sai_object_id_t buf_profile_id);

void sai_qos_init_buffer_profile_node (dn_sai_qos_buffer_profile_t
                                       *p_buf_profile_node);

bool sai_qos_is_buffer_pool_available (sai_object_id_t pool_id, uint_t size);

void sai_qos_get_default_buffer_profile (dn_sai_qos_buffer_profile_t *p_profile_node);

bool sai_qos_is_object_buffer_profile_compatible (sai_object_id_t object_id,
                                                  dn_sai_qos_buffer_profile_t
                                                  *p_profile_node);

bool sai_qos_is_buffer_pool_compatible (sai_object_id_t pool_id_1,
                                        sai_object_id_t pool_id_2, bool check_th);

bool sai_qos_buffer_profile_is_valid_threshold_applied (dn_sai_qos_buffer_profile_t
                                                        *p_profile_node,
                                                        const sai_attribute_t *attr);

sai_status_t sai_qos_buffer_profile_is_mandatory_th_attr_present (uint32_t attr_count,
                                                                  const sai_attribute_t
                                                                  *attr_list);

sai_status_t sai_qos_buffer_profile_is_pfc_threshold_valid (uint32_t attr_count,
                                                            const sai_attribute_t
                                                            *attr_list);

sai_status_t sai_qos_map_pfc_pri_to_queue_default_set (dn_sai_qos_map_t *p_map_node);

sai_status_t sai_qos_map_pfc_pri_to_queue_map_value_set (dn_sai_qos_map_t *p_map_node,
                                                         sai_qos_map_list_t map_list,
                                                         dn_sai_operations_t op_type);

sai_status_t sai_qos_map_tc_to_pg_default_set (dn_sai_qos_map_t *p_map_node);

sai_status_t sai_qos_map_tc_to_pg_map_value_set (dn_sai_qos_map_t *p_map_node,
                                                 sai_qos_map_list_t map_list,
                                                 dn_sai_operations_t op_type);

sai_status_t sai_qos_map_list_alloc(dn_sai_qos_map_t *p_map_node, uint_t count);

sai_status_t sai_qos_create_default_scheduler(sai_object_id_t *default_sched_id);

sai_status_t sai_qos_remove_default_scheduler();

static inline uint_t sai_qos_maps_get_max_queue_idx()
{
    /** @TODO: Value to be obtained from config file */
    return SAI_QOS_MAX_QUEUE_IDX_SEPARATE;
}

#endif /* __SAI_QOS_API_UTILS_H__ */
