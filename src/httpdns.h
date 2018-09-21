#ifndef LIBHTTPDNS_API_H
#define LIBHTTPDNS_API_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define ALIYUN_SERVER_IP "203.107.1.33"   //阿里云httpDNS的服务器地址
#define ACCOUNT_ID 131709                 //account_id 帐号

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

#define HTTPDNS_SERVER_NUM 5

char httdns_server_list[HTTPDNS_SERVER_NUM][32]={
	"203.107.1.33",
	"203.107.1.34",
	"203.107.1.65",
	"203.107.1.1",
	"101.37.99.33",
};


int32_t   httpdns_service_init(void);
int32_t   httpdns_service_destroy(void);
ips_list* httpdns_getips_by_host(void);
int32_t   httpdns_resolve_hosts(char *host, int32_t timeout);
#ifdef __cplusplus
}
#endif

#endif
