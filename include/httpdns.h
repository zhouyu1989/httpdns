/**
 * Copyright (C) 2018 The YodaOS
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __HTTPDNS_API_H
#define __HTTPDNS_API_H

/**
 * @file httpdns.h
 * @brief Definition dns resolve function and struct
 * @author shuai.gu
 * @version 1.0
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

/**
 * @struct host_ips httpdns.h
 * @brief record request host and ip addr
 */
typedef struct
{
    char *h_name;    /**< resolve host name */
    char **h_ips;    /**< resolve host appropriate ip addr */
    int32_t ips_num; /**< ip addr num */
} host_ips;

/**
 * @struct ips_list httpdns.h
 * @brief record request host and host number
 */
typedef struct
{
    host_ips **host;  /**< host list */
    int32_t host_num; /**< host num */
} ips_list;

/**
 * @brief  resolve service finish notify user result
 * @param status get service status
 * @param userdata user data
 * @return if 0 success otherwise -1 fail
 */
typedef int (*httpdns_finished_notify)(int status, void *userdata);

/**
 * @brief init httpdns service source
 * @return if 0 success otherwise -1 fail
 */
int32_t httpdns_service_init(void);

/**
 * @brief destory httpdns service source
 * @return if 0 success otherwise -1 fail
 */
int32_t httpdns_service_destroy(void);

/**
 * @brief get ip addr by hostname
 * @param host_name domain name
 * @param ip return ip addr result
 * @return if 0 success otherwise -1 fail
 */
int httpdns_getips_by_host(char *host_name, char *ip);

/**
 * @brief start resolve dns form gslb
 * @param sn device serial number
 * @param device_type device type id
 * @param timeout request timeout time(Millisecond)
 * @param userdata users need to process
 * @return if 0 success otherwise -1 fail
 */
int32_t httpdns_resolve_gslb(char *sn, char *device_type, int timeout_ms, httpdns_finished_notify cb, void *userdata);

#ifdef __cplusplus
}
#endif

#endif
