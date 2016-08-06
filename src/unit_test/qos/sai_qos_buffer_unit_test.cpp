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
* @file  sai_qos_buffer_unit_test.cpp
*
* @brief This file contains tests for qos buffer APIs
*
*************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"

extern "C" {
#include "sai_qos_unit_test_utils.h"
#include "sai.h"
#include "saibuffer.h"
#include "saiport.h"
#include "saiqueue.h"
#include <inttypes.h>
}

//#define sai_buffer_pool_test_size_1    1048576
//#define sai_buffer_pool_test_size_2    2097152
//#define sai_buffer_pool_test_size_3    1023
//#define sai_buffer_profile_test_size_1 1024
//#define sai_buffer_profile_test_size_2 2048
//#define sai_buffer_profile_test_size_3 524288

#define LOG_PRINT(msg, ...) \
    printf(msg, ##__VA_ARGS__)

unsigned int sai_total_buffer_size = 0;
unsigned int sai_buffer_pool_test_size_1 = 0;
unsigned int sai_buffer_pool_test_size_2 = 0;
unsigned int sai_buffer_pool_test_size_3 = 0;

unsigned int sai_buffer_profile_test_size_1 = 1024;
unsigned int sai_buffer_profile_test_size_2 = 2048;
unsigned int sai_buffer_profile_test_size_3 = 4096;
/*
 * API query is done while running the first test case and
 * the method table is stored in sai_switch_api_table so
 * that its available for the rest of the test cases which
 * use the method table
 */
class qos_buffer : public ::testing::Test
{
    protected:
        static void SetUpTestCase()
        {
            sai_attribute_t get_attr;
            ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_query(SAI_API_SWITCH,
                      (static_cast<void**>(static_cast<void*>(&sai_switch_api_table)))));

            ASSERT_TRUE(sai_switch_api_table != NULL);

            EXPECT_TRUE(sai_switch_api_table->initialize_switch != NULL);
            EXPECT_TRUE(sai_switch_api_table->shutdown_switch != NULL);
            EXPECT_TRUE(sai_switch_api_table->connect_switch != NULL);
            EXPECT_TRUE(sai_switch_api_table->disconnect_switch != NULL);
            EXPECT_TRUE(sai_switch_api_table->set_switch_attribute != NULL);
            EXPECT_TRUE(sai_switch_api_table->get_switch_attribute != NULL);

            sai_switch_notification_t notification;

            notification.on_switch_state_change = sai_switch_operstate_callback;
            notification.on_fdb_event = sai_fdb_evt_callback;
            notification.on_port_state_change = sai_port_state_evt_callback;
            notification.on_switch_shutdown_request = sai_switch_shutdown_callback;
            notification.on_port_event = sai_port_evt_callback;
            notification.on_packet_event = sai_packet_event_callback;

            if(sai_switch_api_table->initialize_switch) {
                sai_switch_api_table->initialize_switch(0,NULL,NULL,&notification);
            }

            ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_query(SAI_API_BUFFERS,
                      (static_cast<void**>(static_cast<void*>(&sai_buffer_api_table)))));

            ASSERT_TRUE(sai_buffer_api_table != NULL);

            EXPECT_TRUE(sai_buffer_api_table->create_buffer_pool != NULL);
            EXPECT_TRUE(sai_buffer_api_table->remove_buffer_pool != NULL);
            EXPECT_TRUE(sai_buffer_api_table->set_buffer_pool_attr != NULL);
            EXPECT_TRUE(sai_buffer_api_table->get_buffer_pool_attr != NULL);
            EXPECT_TRUE(sai_buffer_api_table->set_ingress_priority_group_attr != NULL);
            EXPECT_TRUE(sai_buffer_api_table->get_ingress_priority_group_attr != NULL);
            EXPECT_TRUE(sai_buffer_api_table->create_buffer_profile != NULL);
            EXPECT_TRUE(sai_buffer_api_table->remove_buffer_profile != NULL);
            EXPECT_TRUE(sai_buffer_api_table->set_buffer_profile_attr != NULL);
            EXPECT_TRUE(sai_buffer_api_table->get_buffer_profile_attr != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_PORT,
                      (static_cast<void**>(static_cast<void*>(&sai_port_api_table)))));

            ASSERT_TRUE(sai_port_api_table != NULL);

            EXPECT_TRUE(sai_port_api_table->set_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_stats != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_QUEUE,
                      (static_cast<void**>(static_cast<void*>(&sai_queue_api_table)))));

            ASSERT_TRUE(sai_queue_api_table != NULL);

            EXPECT_TRUE(sai_queue_api_table->set_queue_attribute != NULL);
            EXPECT_TRUE(sai_queue_api_table->get_queue_attribute != NULL);
            EXPECT_TRUE(sai_queue_api_table->get_queue_stats != NULL);
            memset(&get_attr, 0, sizeof(get_attr));
            get_attr.id = SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE;
            ASSERT_EQ(SAI_STATUS_SUCCESS,
                      sai_switch_api_table->get_switch_attribute(1, &get_attr));
            sai_total_buffer_size = (get_attr.value.u32)*1024;
            sai_buffer_pool_test_size_1 = (sai_total_buffer_size/10);
            sai_buffer_pool_test_size_2 = (sai_total_buffer_size/5);
            sai_buffer_pool_test_size_3 = 1023;

        }

        static sai_switch_api_t* sai_switch_api_table;
        static sai_buffer_api_t* sai_buffer_api_table;
        static sai_port_api_t* sai_port_api_table;
        static sai_queue_api_t* sai_queue_api_table;
};

sai_switch_api_t* qos_buffer ::sai_switch_api_table = NULL;
sai_buffer_api_t* qos_buffer ::sai_buffer_api_table = NULL;
sai_port_api_t* qos_buffer ::sai_port_api_table = NULL;
sai_queue_api_t* qos_buffer ::sai_queue_api_table = NULL;

sai_status_t sai_create_buffer_pool(sai_buffer_api_t* buffer_api_table,
                                    sai_object_id_t *pool_id, unsigned int size,
                                    sai_buffer_pool_type_t type,
                                    sai_buffer_threshold_mode_t th_mode)
{
    sai_status_t sai_rc;

    sai_attribute_t attr[3];

    attr[0].id = SAI_BUFFER_POOL_ATTR_TYPE;
    attr[0].value.s32 = type;

    attr[1].id = SAI_BUFFER_POOL_ATTR_SIZE;
    attr[1].value.u32 = size;

    attr[2].id = SAI_BUFFER_POOL_ATTR_TH_MODE;
    attr[2].value.s32 = th_mode;

    sai_rc = buffer_api_table->create_buffer_pool(pool_id, 3, attr);
    return sai_rc;
}

sai_status_t sai_create_buffer_profile(sai_buffer_api_t* buffer_api_table,
                                       sai_object_id_t *profile_id, unsigned int attr_bmp,
                                       sai_object_id_t pool_id, unsigned int size, int th_mode,
                                       int dynamic_th, unsigned int static_th,
                                       unsigned int xoff_th, unsigned int xon_th)
{
    sai_status_t sai_rc;

    sai_attribute_t attr[6];
    unsigned int attr_idx = 0;

    if (attr_bmp & (1 << SAI_BUFFER_PROFILE_ATTR_POOL_ID)) {
        attr[attr_idx].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
        attr[attr_idx].value.oid = pool_id;
        attr_idx++;
    }

    if (attr_bmp & (1 << SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE)) {
        attr[attr_idx].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
        attr[attr_idx].value.u32 = size;
        attr_idx++;
    }

    if (attr_bmp & (1 << SAI_BUFFER_PROFILE_ATTR_TH_MODE)) {
        attr[attr_idx].id = SAI_BUFFER_PROFILE_ATTR_TH_MODE;
        attr[attr_idx].value.s32 = th_mode;
        attr_idx++;
    }

    if (attr_bmp & (1 << SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH)) {
        attr[attr_idx].id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
        attr[attr_idx].value.s8 = dynamic_th;
        attr_idx++;
    }

    if (attr_bmp & (1 << SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH)) {
        attr[attr_idx].id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
        attr[attr_idx].value.u32 = static_th;
        attr_idx++;
    }

    if (attr_bmp & (1 << SAI_BUFFER_PROFILE_ATTR_XOFF_TH)) {
        attr[attr_idx].id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
        attr[attr_idx].value.u32 = xoff_th;
        attr_idx++;
    }

    if (attr_bmp & (1 << SAI_BUFFER_PROFILE_ATTR_XON_TH)) {
        attr[attr_idx].id = SAI_BUFFER_PROFILE_ATTR_XON_TH;
        attr[attr_idx].value.u32 = xon_th;
        attr_idx++;
    }

    sai_rc = buffer_api_table->create_buffer_profile(profile_id, attr_idx, attr);
    return sai_rc;
}


TEST_F(qos_buffer, buffer_pool_create_test)
{
    sai_attribute_t get_attr;
    sai_attribute_t create_attr[4];
    sai_object_id_t pool_id = 0;

    create_attr[0].id = SAI_BUFFER_POOL_ATTR_TH_MODE;
    create_attr[0].value.u32 = SAI_BUFFER_THRESHOLD_MODE_DYNAMIC;

    create_attr[1].id = SAI_BUFFER_POOL_ATTR_SIZE;
    create_attr[1].value.u32 = sai_buffer_pool_test_size_1;

    /** Mandatory attribute cases */
    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              sai_buffer_api_table->create_buffer_pool (&pool_id, 2,
                                      (const sai_attribute_t *)create_attr));

    create_attr[1].id = SAI_BUFFER_POOL_ATTR_TYPE;
    create_attr[1].value.u32 = SAI_BUFFER_POOL_INGRESS;

    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              sai_buffer_api_table->create_buffer_pool (&pool_id, 2,
                                       (const sai_attribute_t *)create_attr));

    create_attr[2].id = SAI_BUFFER_POOL_ATTR_SIZE;
    create_attr[2].value.u32 = sai_buffer_pool_test_size_1;

    create_attr[3].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    create_attr[3].value.u32 = sai_buffer_pool_test_size_1;

    ASSERT_EQ(SAI_STATUS_CODE(SAI_STATUS_CODE(SAI_STATUS_INVALID_ATTRIBUTE_0)+3),
              sai_buffer_api_table->create_buffer_pool (&pool_id, 4,
                                       (const sai_attribute_t *)create_attr));
    create_attr[0].id = SAI_BUFFER_POOL_ATTR_SIZE;
    create_attr[0].value.u32 = sai_buffer_pool_test_size_1;

    create_attr[1].id = SAI_BUFFER_POOL_ATTR_TYPE;
    create_attr[1].value.u32 = SAI_BUFFER_POOL_INGRESS;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->create_buffer_pool (&pool_id, 2,
                                       (const sai_attribute_t *)create_attr));

    /** Check if default mode is dynamic */
    get_attr.id = SAI_BUFFER_POOL_ATTR_TH_MODE;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr(pool_id,
                                     1, &get_attr));
    ASSERT_EQ (get_attr.value.s32, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (pool_id));

    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_buffer_api_table->remove_buffer_pool (pool_id));
}

TEST_F(qos_buffer, buffer_pool_attr_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;
    sai_object_id_t pool_id = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_EGRESS, SAI_BUFFER_THRESHOLD_MODE_STATIC));

    set_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    set_attr.value.u32 = sai_buffer_pool_test_size_2;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attr(pool_id,
                                     (const sai_attribute_t *)&set_attr));

    get_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr(pool_id,
                                     1, &get_attr));
    ASSERT_EQ (get_attr.value.u32, sai_buffer_pool_test_size_2);

    set_attr.id = SAI_BUFFER_POOL_ATTR_TYPE;
    set_attr.value.u32 = SAI_BUFFER_POOL_EGRESS;
    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_pool_attr(pool_id,
                                     (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_POOL_ATTR_TH_MODE;
    set_attr.value.u32 = SAI_BUFFER_THRESHOLD_MODE_DYNAMIC;
    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_pool_attr(pool_id,
                                     (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    set_attr.value.u32 = sai_buffer_pool_test_size_2;
    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_pool_attr(pool_id,
                                     (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (pool_id));
}

TEST_F(qos_buffer, max_ing_num_buffer_pool_test)
{
    sai_attribute_t get_attr;
    unsigned int num_pools = 0;
    unsigned int pool_idx = 0;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_switch_api_table->get_switch_attribute(1, &get_attr));

    num_pools = get_attr.value.u32;
    sai_object_id_t pool_id[num_pools+1];

    for(pool_idx = 0; pool_idx < num_pools; pool_idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &pool_id[pool_idx], sai_buffer_pool_test_size_1,
                  SAI_BUFFER_POOL_INGRESS, SAI_BUFFER_THRESHOLD_MODE_STATIC));
    }
    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[num_pools], sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_INGRESS,
              SAI_BUFFER_THRESHOLD_MODE_STATIC));
    for(pool_idx = 0; pool_idx < num_pools; pool_idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_buffer_api_table->remove_buffer_pool (pool_id[pool_idx]));
    }


}

TEST_F(qos_buffer, max_egr_num_buffer_pool_test)
{
    sai_attribute_t get_attr;
    unsigned int num_pools = 0;
    unsigned int pool_idx = 0;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_switch_api_table->get_switch_attribute(1, &get_attr));

    num_pools = get_attr.value.u32;
    sai_object_id_t pool_id[num_pools+1];

    for(pool_idx = 0; pool_idx < num_pools; pool_idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[pool_idx],
                    sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_EGRESS,
                    SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));
    }
    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[num_pools], sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_EGRESS,
              SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    for(pool_idx = 0; pool_idx < num_pools; pool_idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_buffer_api_table->remove_buffer_pool (pool_id[pool_idx]));
    }

}

TEST_F(qos_buffer, max_buffer_size_test)
{
    unsigned int buf_size = 0;
    sai_object_id_t pool_id[4];
    sai_attribute_t set_attr;
    unsigned int pool_idx = 0;

    buf_size = sai_total_buffer_size*2;

    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[0], buf_size, SAI_BUFFER_POOL_EGRESS,
              SAI_BUFFER_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[2], buf_size, SAI_BUFFER_POOL_INGRESS,
              SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    buf_size = (sai_total_buffer_size*3)/4;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[0], buf_size, SAI_BUFFER_POOL_EGRESS,
              SAI_BUFFER_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[2], buf_size, SAI_BUFFER_POOL_INGRESS,
              SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[1], buf_size, SAI_BUFFER_POOL_EGRESS,
              SAI_BUFFER_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[3], buf_size, SAI_BUFFER_POOL_INGRESS,
              SAI_BUFFER_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (pool_id[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (pool_id[2]));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[1], buf_size, SAI_BUFFER_POOL_EGRESS,
              SAI_BUFFER_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[3], buf_size, SAI_BUFFER_POOL_INGRESS,
              SAI_BUFFER_THRESHOLD_MODE_STATIC));

    buf_size = sai_total_buffer_size/4;
    set_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    set_attr.value.u32 = buf_size;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attr(pool_id[1],
                                     (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attr(pool_id[3],
                                     (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[0], buf_size, SAI_BUFFER_POOL_EGRESS,
              SAI_BUFFER_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[2], buf_size, SAI_BUFFER_POOL_INGRESS,
              SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    for(pool_idx = 0; pool_idx < 4; pool_idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_buffer_api_table->remove_buffer_pool (pool_id[pool_idx]));
    }
}

TEST_F(qos_buffer, buffer_profile_create_test)
{
    sai_attribute_t create_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_EGRESS, SAI_BUFFER_THRESHOLD_MODE_STATIC));

    create_attr[0].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    create_attr[0].value.u32 = sai_buffer_profile_test_size_1;

    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              sai_buffer_api_table->create_buffer_profile (&profile_id, 1,
                                      (const sai_attribute_t *)create_attr));

    create_attr[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    create_attr[0].value.oid = pool_id;

    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              sai_buffer_api_table->create_buffer_profile (&profile_id, 1,
                                      (const sai_attribute_t *)create_attr));

    create_attr[0].value.oid = SAI_NULL_OBJECT_ID;
    create_attr[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    create_attr[1].value.u32 = sai_buffer_profile_test_size_1;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTR_VALUE_0,
              sai_buffer_api_table->create_buffer_profile (&pool_id, 2,
                                       (const sai_attribute_t *)create_attr));

    create_attr[0].value.oid = pool_id;
    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              sai_buffer_api_table->create_buffer_profile (&profile_id, 2,
                                      (const sai_attribute_t *)create_attr));

    create_attr[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    create_attr[2].value.u8 = 1;

    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              sai_buffer_api_table->create_buffer_profile (&profile_id, 3,
                                      (const sai_attribute_t *)create_attr));

    create_attr[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    create_attr[2].value.u32 = sai_buffer_profile_test_size_3;

    create_attr[3].id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
    create_attr[3].value.u32 = sai_buffer_profile_test_size_1;

    ASSERT_EQ(SAI_STATUS_CODE(SAI_STATUS_CODE(SAI_STATUS_INVALID_ATTRIBUTE_0)+3),
              sai_buffer_api_table->create_buffer_profile (&profile_id, 4,
                                      (const sai_attribute_t *)create_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->create_buffer_profile (&profile_id, 3,
                                      (const sai_attribute_t *)create_attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              sai_buffer_api_table->remove_buffer_pool (pool_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_profile(profile_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (pool_id));
}

TEST_F(qos_buffer, egr_buffer_profile_attr_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t pool_id_2 = 0;
    sai_object_id_t pool_id_3 = 0;
    sai_object_id_t pool_id_4 = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_EGRESS, SAI_BUFFER_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_2, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_INGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_3, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_EGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_4, sai_buffer_pool_test_size_2,
                                     SAI_BUFFER_POOL_EGRESS, SAI_BUFFER_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_profile(sai_buffer_api_table, &profile_id, 0x13 ,pool_id_1,
                                        sai_buffer_profile_test_size_1, 0,
                                        0, sai_buffer_profile_test_size_3, 0,0));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_XON_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_1;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_2;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTR_VALUE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                               (const sai_attribute_t *)&set_attr));

    set_attr.value.oid = pool_id_3;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTR_VALUE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                               (const sai_attribute_t *)&set_attr));

    set_attr.value.oid = pool_id_4;

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                               (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    set_attr.value.u8 = 1;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_3;

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    get_attr[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    get_attr[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    get_attr[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_profile_attr (profile_id, 3, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, pool_id_4);
    EXPECT_EQ (get_attr[1].value.u32, sai_buffer_profile_test_size_1);
    EXPECT_EQ (get_attr[2].value.u32, sai_buffer_profile_test_size_3);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_2));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_3));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_4));
}

TEST_F(qos_buffer, ing_buffer_profile_attr_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t pool_id_2 = 0;
    sai_object_id_t pool_id_3 = 0;
    sai_object_id_t pool_id_4 = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_INGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_2, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_EGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_3, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_INGRESS, SAI_BUFFER_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_4, sai_buffer_pool_test_size_2,
                                     SAI_BUFFER_POOL_INGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_profile(sai_buffer_api_table, &profile_id, 0xb ,pool_id_1,
                                        sai_buffer_profile_test_size_1, 0, 1, 0, 0, 0));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    set_attr.value.u32 = sai_buffer_profile_test_size_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_1;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_2;
    ASSERT_EQ (SAI_STATUS_INVALID_ATTR_VALUE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                               (const sai_attribute_t *)&set_attr));

    set_attr.value.oid = pool_id_3;
    ASSERT_EQ (SAI_STATUS_INVALID_ATTR_VALUE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                               (const sai_attribute_t *)&set_attr));

    set_attr.value.oid = pool_id_4;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                               (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    set_attr.value.u8 = 2;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_3;
    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    get_attr[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    get_attr[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    get_attr[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    get_attr[3].id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_profile_attr (profile_id, 4,
                                                              get_attr));

    EXPECT_EQ (get_attr[0].value.oid, pool_id_4);
    EXPECT_EQ (get_attr[1].value.u32, sai_buffer_profile_test_size_2);
    EXPECT_EQ (get_attr[2].value.u8, 2);
    EXPECT_EQ (get_attr[3].value.u32, sai_buffer_profile_test_size_1);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_2));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_3));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_4));
}

sai_status_t sai_qos_buffer_get_first_pg (sai_port_api_t* sai_port_api_table,
                                          sai_object_id_t port_id, sai_object_id_t *pg_obj)
{
    unsigned int num_pg = 0;
    sai_attribute_t get_attr[1];
    sai_status_t sai_rc;

    get_attr[0].id = SAI_PORT_ATTR_NUMBER_OF_PRIORITY_GROUPS;
    sai_rc = sai_port_api_table->get_port_attribute (port_id, 1, get_attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    num_pg = get_attr[0].value.u32;
    sai_object_id_t pg_id[num_pg];

    get_attr[0].id = SAI_PORT_ATTR_PRIORITY_GROUP_LIST;
    get_attr[0].value.objlist.count = num_pg;
    get_attr[0].value.objlist.list = pg_id;
    sai_rc = sai_port_api_table->get_port_attribute (port_id, 1, get_attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }
    *pg_obj = get_attr[0].value.objlist.list[0];
    return SAI_STATUS_SUCCESS;
}


TEST_F(qos_buffer, buffer_profile_pg_basic_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_INGRESS,
                                  SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id_1, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attr (pg_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id);

    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
}

TEST_F(qos_buffer, buffer_profile_pool_size_test)
{
    sai_object_id_t pool_id = 0;
    sai_object_id_t profile_id = 0;
    sai_attribute_t set_attr;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id, sai_buffer_pool_test_size_3,
                                     SAI_BUFFER_POOL_INGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj));
    /** Create with small space in pool and check insuffcient resources is returned*/
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_INSUFFICIENT_RESOURCES, sai_buffer_api_table->
               set_ingress_priority_group_attr (pg_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id));
}


TEST_F(qos_buffer, buffer_profile_pg_size_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t pool_id_2 = 0;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
              sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_INGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_2,
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_INGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id_1, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_buffer_pool_attr (pool_id_1, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_1 - (sai_buffer_profile_test_size_2 + sai_buffer_profile_test_size_1)));

    /** Change size of PG */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    set_attr.value.u32 = sai_buffer_profile_test_size_1;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               set_buffer_profile_attr (profile_id, (const sai_attribute_t *)&set_attr));
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id_1, 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_1 - (sai_buffer_profile_test_size_1 + sai_buffer_profile_test_size_1)));

    /** Change Xoff size of PG */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_2;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               set_buffer_profile_attr (profile_id, (const sai_attribute_t *)&set_attr));
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id_1, 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_1 - (sai_buffer_profile_test_size_1 + sai_buffer_profile_test_size_2)));

    /** Move PG to a different pool */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (
                                              profile_id, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_pool_attr (pool_id_1, 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32, sai_buffer_pool_test_size_1);
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_pool_attr (pool_id_2, 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_2 - (sai_buffer_profile_test_size_1 + sai_buffer_profile_test_size_2)));

    /** Change pool size */
    set_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    set_attr.value.u32 = sai_buffer_pool_test_size_1;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attr (
                                              pool_id_2, (const sai_attribute_t *)&set_attr));
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_buffer_pool_attr (pool_id_2, 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_1 - (sai_buffer_profile_test_size_1 + sai_buffer_profile_test_size_2)));

    /** Remove PG from pool */
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_buffer_pool_attr (pool_id_2, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.u32, sai_buffer_pool_test_size_1);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_2));
}

TEST_F(qos_buffer, pg_buffer_profile_replace_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id[5] = {0};
    sai_object_id_t pool_id[4] = {0};
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[0],
              sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_INGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[1],
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_INGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[2],
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_INGRESS, SAI_BUFFER_THRESHOLD_MODE_STATIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[3],
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_EGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[0],
              0x6b ,pool_id[0], sai_buffer_profile_test_size_2, 0, 1, 0,
              sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[1],
              0x6b ,pool_id[0], sai_buffer_profile_test_size_1, 0, 1, 0,
              sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[2],
              0x6b ,pool_id[1], sai_buffer_profile_test_size_1, 0, 1, 0,
              sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[3],
              0x73 ,pool_id[2], sai_buffer_profile_test_size_2, 0, 0, sai_buffer_profile_test_size_3,
              sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[4],
              0xb ,pool_id[3], sai_buffer_profile_test_size_2, 0, 1, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id[0];
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    set_attr.value.oid =  profile_id[3];
    ASSERT_EQ (SAI_STATUS_INVALID_PARAMETER, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    set_attr.value.oid =  profile_id[4];
    ASSERT_EQ (SAI_STATUS_INVALID_PARAMETER, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    set_attr.value.oid = profile_id[1];
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_ingress_priority_group_attr (
                                               pg_obj, 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.oid, profile_id[1]);

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id[0], 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_1 - (sai_buffer_profile_test_size_1 + sai_buffer_profile_test_size_1)));

    set_attr.value.oid = profile_id[2];
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id[0], 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32,sai_buffer_pool_test_size_1);

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_pool_attr (pool_id[1], 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_2 - (sai_buffer_profile_test_size_1 + sai_buffer_profile_test_size_1)));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[0]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[1]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[2]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[3]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[4]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[0]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[1]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[2]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[3]));
}

TEST_F(qos_buffer, buffer_profile_pg_profile_th_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id_1 = 0;
    sai_object_id_t profile_id_2 = 0;
    sai_object_id_t profile_id_3 = 0;
    sai_object_id_t profile_id_4 = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t pool_id_2 = 0;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_INGRESS,
                                  SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_2,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_INGRESS,
                                  SAI_BUFFER_THRESHOLD_MODE_STATIC));

    /* Local profile is static while pool is dynamic */
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_1,
                                  0x17 ,pool_id_1, sai_buffer_profile_test_size_2,
                                  SAI_BUFFER_THRESHOLD_MODE_STATIC, 0,
                                  sai_buffer_profile_test_size_3, 0, 0));

    /* Local profile is dynamic while pool is static */
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_2,
                                  0xf ,pool_id_2, sai_buffer_profile_test_size_2,
                                  SAI_BUFFER_THRESHOLD_MODE_DYNAMIC, 1, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_3,
                                  0xb ,pool_id_1, sai_buffer_profile_test_size_2,
                                  0, 1, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_4,
                                  0x13 ,pool_id_2, sai_buffer_profile_test_size_2,
                                  0, 0, sai_buffer_profile_test_size_3, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj));
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id_1;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attr (pg_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_1);

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attr (pg_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_2);

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id_3;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attr (pg_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_3);

    /* Error should be returned since profile th is not enabled*/
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    set_attr.value.u32 = 0;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    /* Set profile th mode and change profile value. Setting to 0 which should be setting to unlimited value */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_TH_MODE;
    set_attr.value.oid = SAI_BUFFER_THRESHOLD_MODE_STATIC;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    set_attr.value.u32 = 0;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    /* Change buffer pool to a differnt type since profile th is enabled */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_2;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id_4;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attr (pg_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_4);

    /* Error should be returned since profile th is not enabled*/
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    set_attr.value.u32 = 2;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));

    /* Set profile th mode and change profile value. */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_TH_MODE;
    set_attr.value.oid = SAI_BUFFER_THRESHOLD_MODE_DYNAMIC;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    set_attr.value.u32 = 2;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));

    /* Change buffer pool to a differnt type since profile th is enabled */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_1;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_2));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_3));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_4));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_2));
}
sai_status_t sai_qos_buffer_get_first_queue (sai_port_api_t* sai_port_api_table,
                                             sai_object_id_t port_id, sai_object_id_t *queue_obj)
{
    unsigned int num_queue = 0;
    sai_attribute_t get_attr[1];
    sai_status_t sai_rc;

    get_attr[0].id = SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES;
    sai_rc = sai_port_api_table->get_port_attribute (port_id, 1, get_attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    num_queue = get_attr[0].value.u32;
    sai_object_id_t queue_id[num_queue];

    get_attr[0].id = SAI_PORT_ATTR_QOS_QUEUE_LIST;
    get_attr[0].value.objlist.count = num_queue;
    get_attr[0].value.objlist.list = queue_id;
    sai_rc = sai_port_api_table->get_port_attribute (port_id, 1, get_attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }
    *queue_obj = get_attr[0].value.objlist.list[0];
    return SAI_STATUS_SUCCESS;
}

TEST_F(qos_buffer, buffer_profile_queue_basic_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t queue_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_EGRESS,
                                  SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0xb ,pool_id_1, sai_buffer_profile_test_size_2, 0, 1, 0, 0, 0));


    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &queue_obj));
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->
               get_queue_attribute (queue_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id);

    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
}

TEST_F(qos_buffer, buffer_profile_queue_profile_th_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id_1 = 0;
    sai_object_id_t profile_id_2 = 0;
    sai_object_id_t profile_id_3 = 0;
    sai_object_id_t profile_id_4 = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t pool_id_2 = 0;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t queue_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_EGRESS,
                                  SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_2,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_EGRESS,
                                  SAI_BUFFER_THRESHOLD_MODE_STATIC));

    /* Local profile is static while pool is dynamic */
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_1,
                                  0x17 ,pool_id_1, sai_buffer_profile_test_size_2,
                                  SAI_BUFFER_THRESHOLD_MODE_STATIC, 0,
                                  sai_buffer_profile_test_size_3, 0, 0));

    /* Local profile is dynamic while pool is static */
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_2,
                                  0xf ,pool_id_2, sai_buffer_profile_test_size_2,
                                  SAI_BUFFER_THRESHOLD_MODE_DYNAMIC, 1, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_3,
                                  0xb ,pool_id_1, sai_buffer_profile_test_size_2,
                                  0, 1, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_4,
                                  0x13 ,pool_id_2, sai_buffer_profile_test_size_2,
                                  0, 0, sai_buffer_profile_test_size_3, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &queue_obj));
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id_1;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->
               get_queue_attribute (queue_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_1);

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->
               get_queue_attribute (queue_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_2);

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id_3;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->
               get_queue_attribute (queue_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_3);

    /* Error should be returned since profile th is not enabled*/
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    set_attr.value.u32 = 0;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    /* Set profile th mode and change profile value. Setting to 0 which should be setting to unlimited value */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_TH_MODE;
    set_attr.value.oid = SAI_BUFFER_THRESHOLD_MODE_STATIC;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    set_attr.value.u32 = 0;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    /* Change buffer pool to a differnt type since profile th is enabled */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_2;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id_4;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->
               get_queue_attribute (queue_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_4);

    /* Error should be returned since profile th is not enabled*/
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    set_attr.value.u32 = 2;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));

    /* Set profile th mode and change profile value. */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_TH_MODE;
    set_attr.value.oid = SAI_BUFFER_THRESHOLD_MODE_DYNAMIC;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    set_attr.value.u32 = 2;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));

    /* Change buffer pool to a differnt type since profile th is enabled */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_1;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_2));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_3));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_4));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_2));
}

TEST_F(qos_buffer, buffer_profile_queue_size_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t pool_id_2 = 0;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t queue_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_EGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_2,
              sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_EGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0xb ,pool_id_1, sai_buffer_profile_test_size_1, 0, 1, 0, 0,0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &queue_obj));

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_buffer_pool_attr (pool_id_1, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_2 - sai_buffer_profile_test_size_1));

    /** Change size of QUEUE */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    set_attr.value.u32 = sai_buffer_profile_test_size_2;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               set_buffer_profile_attr (profile_id, (const sai_attribute_t *)&set_attr));
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id_1, 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_2 - sai_buffer_profile_test_size_2));

    /** Move QUEUE to a different pool */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (
                                              profile_id, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_pool_attr (pool_id_1, 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32, sai_buffer_pool_test_size_2);

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_pool_attr (pool_id_2, 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_1 - sai_buffer_profile_test_size_2));

    /** Change pool size */
    set_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    set_attr.value.u32 = sai_buffer_pool_test_size_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attr (
                                              pool_id_2, (const sai_attribute_t *)&set_attr));
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_buffer_pool_attr (pool_id_2, 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_2 - sai_buffer_profile_test_size_2));

    /** Remove QUEUE from pool */
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_buffer_pool_attr (pool_id_2, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.u32, sai_buffer_pool_test_size_2);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_2));
}

TEST_F(qos_buffer, queue_buffer_profile_replace_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id[5] = {0};
    sai_object_id_t pool_id[4] = {0};
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t queue_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[0],
              sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_EGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[1],
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_EGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[2],
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_EGRESS, SAI_BUFFER_THRESHOLD_MODE_STATIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[3],
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_INGRESS, SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[0],
              0xb ,pool_id[0], sai_buffer_profile_test_size_2, 0, 1, 0, 0, 0));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[1],
              0xb ,pool_id[0], sai_buffer_profile_test_size_1, 0, 1, 0, 0, 0));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[2],
              0xb ,pool_id[1], sai_buffer_profile_test_size_1, 0, 1, 0, 0, 0));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[3],
              0x13 ,pool_id[2], sai_buffer_profile_test_size_2, 0, 0, sai_buffer_profile_test_size_3,0,0));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[4],
              0xb ,pool_id[3], sai_buffer_profile_test_size_2, 0, 1, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &queue_obj));

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id[0];
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    set_attr.value.oid =  profile_id[3];
    ASSERT_EQ (SAI_STATUS_INVALID_PARAMETER, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    set_attr.value.oid =  profile_id[4];
    ASSERT_EQ (SAI_STATUS_INVALID_PARAMETER, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));
    set_attr.value.oid = profile_id[1];
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->get_queue_attribute (
                                               queue_obj, 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.oid, profile_id[1]);

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id[0], 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_1 - sai_buffer_profile_test_size_1));

    set_attr.value.oid = profile_id[2];
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id[0], 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32,sai_buffer_pool_test_size_1);

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_pool_attr (pool_id[1], 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (sai_buffer_pool_test_size_2 - sai_buffer_profile_test_size_1));

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[0]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[1]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[2]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[3]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[4]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[0]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[1]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[2]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[3]));
}

TEST_F (qos_buffer, ingress_buffer_pool_stats_get)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t pg_id;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id = 0;
    sai_attribute_t set_attr;
    sai_buffer_pool_stat_counter_t counter_id[] =
                   {SAI_BUFFER_POOL_STAT_CURR_OCCUPANCY_BYTES,
                    SAI_BUFFER_POOL_STAT_WATERMARK_BYTES
                    };
    uint64_t counter_val;
    unsigned int idx = 0;
    unsigned int num_counters;

    num_counters =  sizeof(counter_id)/sizeof(sai_buffer_pool_stat_counter_t);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_INGRESS,
                                  SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_id));
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_id, (const sai_attribute_t *)&set_attr));

    for(idx = 0; idx < num_counters; idx++) {
        sai_rc = sai_buffer_api_table->get_buffer_pool_stats(pool_id, &counter_id[idx],
                                                            1, &counter_val);
        if(sai_rc == SAI_STATUS_SUCCESS) {
             printf("Get counter ID %d is supported. Val:0x%"PRIx64"\r\n",counter_id[idx],counter_val);
        } else if( sai_rc == SAI_STATUS_NOT_SUPPORTED) {
             printf("Get counter ID %d is not supported.\r\n",counter_id[idx]);
        } else {
             printf("Get counter ID %d get returned error %d\r\n",counter_id[idx], sai_rc);
        }
    }
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_id, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id));

}

TEST_F (qos_buffer, egress_buffer_pool_stats_get)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t queue_id;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id = 0;
    sai_attribute_t set_attr;
    sai_buffer_pool_stat_counter_t counter_id[] =
                   {SAI_BUFFER_POOL_STAT_CURR_OCCUPANCY_BYTES,
                    SAI_BUFFER_POOL_STAT_WATERMARK_BYTES
                    };
    uint64_t counter_val;
    unsigned int idx = 0;
    unsigned int num_counters;

    num_counters =  sizeof(counter_id)/sizeof(sai_buffer_pool_stat_counter_t);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_EGRESS,
                                  SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0xb ,pool_id, sai_buffer_profile_test_size_2, 0, 1, 0, 0, 0));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &queue_id));


    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_id, (const sai_attribute_t *)&set_attr));


    for(idx = 0; idx < num_counters; idx++) {
        sai_rc = sai_buffer_api_table->get_buffer_pool_stats(pool_id, &counter_id[idx],
                                                            1, &counter_val);
        if(sai_rc == SAI_STATUS_SUCCESS) {
             printf("Get counter ID %d is supported. Val:0x%"PRIx64"\r\n",counter_id[idx],counter_val);
        } else if( sai_rc == SAI_STATUS_NOT_SUPPORTED) {
             printf("Get counter ID %d is not supported.\r\n",counter_id[idx]);
        } else {
             printf("Get counter ID %d get returned error %d\r\n",counter_id[idx], sai_rc);
        }
    }
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_id, (const sai_attribute_t *)&set_attr));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id));

}
TEST_F (qos_buffer, pg_stats_get)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_id;
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id = 0;
    sai_attribute_t set_attr;

    sai_ingress_priority_group_stat_counter_t counter_id[] =
                   {SAI_INGRESS_PRIORITY_GROUP_STAT_PACKETS,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_WATERMARK_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_WATERMARK_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_WATERMARK_BYTES
                    };
    uint64_t counter_val;
    unsigned int idx = 0;
    unsigned int num_counters;

    num_counters =  sizeof(counter_id)/sizeof(sai_ingress_priority_group_stat_counter_t);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_INGRESS,
                                  SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_id));
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_id, (const sai_attribute_t *)&set_attr));
    for(idx = 0; idx < num_counters; idx++) {
        sai_rc = sai_buffer_api_table->get_ingress_priority_group_stats(pg_id, &counter_id[idx],
                                                            1, &counter_val);
        if(sai_rc == SAI_STATUS_SUCCESS) {
             printf("Get counter ID %d is supported. Val:0x%"PRIx64"\r\n",counter_id[idx],counter_val);
        } else if( sai_rc == SAI_STATUS_NOT_SUPPORTED) {
             printf("Get counter ID %d is not supported.\r\n",counter_id[idx]);
        } else {
             printf("Get counter ID %d get returned error %d\r\n",counter_id[idx], sai_rc);
        }
    }
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_id, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id));

}

TEST_F(qos_buffer, pg_stats_clear)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_id;
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id = 0;
    sai_attribute_t set_attr;
    sai_ingress_priority_group_stat_counter_t counter_id[] =
                   {SAI_INGRESS_PRIORITY_GROUP_STAT_PACKETS,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_WATERMARK_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_WATERMARK_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_WATERMARK_BYTES
                    };
    unsigned int idx = 0;
    unsigned int num_counters;

    num_counters =  sizeof(counter_id)/sizeof(sai_ingress_priority_group_stat_counter_t);

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_INGRESS,
                                  SAI_BUFFER_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_id));
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_id, (const sai_attribute_t *)&set_attr));

    for(idx = 0; idx < num_counters; idx++) {
        sai_rc = sai_buffer_api_table->clear_ingress_priority_group_stats(pg_id, &counter_id[idx],1);
        if(sai_rc == SAI_STATUS_SUCCESS) {
             printf("Clear counter ID %d is supported\r\n",counter_id[idx]);
        } else if( sai_rc == SAI_STATUS_NOT_SUPPORTED) {
             printf("Clear counter ID %d is not supported.\r\n",counter_id[idx]);
        } else {
             printf("Clear counter ID %d get returned error %d\r\n",counter_id[idx], sai_rc);
        }
    }

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_id, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id));
}

