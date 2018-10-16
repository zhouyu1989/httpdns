
/**************************************************************
 * Copyright (c) 2018-2020,Hangzhou Rokid Tech. Co., Ltd.
 * All rights reserved.
 *
 * FileName: httpdns.h
 * Description: Request Request parsing DNS form aliyun server
 *
 * Date: 2018.10.09
 * Author: shuai.gu
 * Modification: Collate code standard
 *
 * Data: 2018.09.15
 * Author: shuai.gu
 * Modification: create
 **************************************************************/

#ifndef __HTTPDNS_API_H
#define __HTTPDNS_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define ACCOUNT_ID 131709
#define HTTPDNS_SERVER_NUM 5

typedef struct
{
    char  *h_name;
    char  **h_ips;
    int32_t ips_num;
}host_ips;

typedef struct
{
    host_ips **host;
    int32_t host_num;
}ips_list;

char httdns_server_list[HTTPDNS_SERVER_NUM][32]={
    "203.107.1.33",
    "203.107.1.34",
    "203.107.1.65",
    "203.107.1.1",
    "101.37.99.33",
};

/*******************************************************
* Function name: httpdns_service_init
* Description: init curl
* Parameter:
* Return: 0 success, other fail
********************************************************/
int32_t httpdns_service_init(void);

/*******************************************************
* Function name: httpdns_service_destroy
* Description: destory curl
* Parameter:
* Return: 0 success, other fail
********************************************************/
int32_t httpdns_service_destroy(void);

/*******************************************************
* Function name: httpdns_getips_by_host
* Description: get host ip list
* Parameter:
* Return: NULL fail, other success
********************************************************/
ips_list *httpdns_getips_by_host(void);

/*******************************************************
* Function name: httpdns_resolve_hosts
* Description: async resolve hosts ip
* Parameter：
*	@host: hosname list
*	@timeout: curl request timeout
* Return: 0 success, other fail
**********************************************************/
int32_t httpdns_resolve_hosts(char *host, int32_t timeout);


#ifdef __cplusplus
}
#endif

#endif
