
/**************************************************************
 * Copyright (c) 2018-2020,Hangzhou Rokid Tech. Co., Ltd.
 * All rights reserved.
 *
 * FileName: httpdns.c
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

#include <pthread.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "httpdns.h"
#include "cJSON.h"

#define HTTP_HEARDR_SIZE 1024
#define RKLog(...) ({ \
            printf("[%s] [%d] ",__func__,__LINE__),\
            printf(__VA_ARGS__);\
        })

static char httpdns_url[HTTP_HEARDR_SIZE] = {0,};
static pthread_mutex_t resolve_mutex;
static long httpdns_timeout = 0;
static ips_list g_hostips;

typedef enum
{
    HTTPDNS_RESOLVE_END = 0,
    HTTPDNS_RESOLVE_ING
}resolve_status_t;

resolve_status_t resolve_status = HTTPDNS_RESOLVE_END;

static int32_t httpdns_header_callback(char *data, size_t size, size_t nitems, void *userp)
{
    int32_t realsize = nitems * size;
    if (0 == realsize)
    {
        return 0;
    }

    RKLog("httpdns_header =%s \n",data);
    return realsize;
}

static int32_t httpdns_response_callback(void *data, int32_t size, int32_t nmemb, void *userp)
{
    int32_t realsize = size * nmemb;
    int32_t ips_array_size = 0;
    char *value = NULL;
    char *p  = NULL;
    cJSON *item = NULL, *it = NULL, *js_host = NULL, *js_ips = NULL, *sub = NULL;
    int cnt = 0, len = 0;

    if (0 == realsize)
    {
        return 0;
    }

    RKLog("\n %s \n",data);
    pthread_mutex_lock(&resolve_mutex);
    cJSON *root = cJSON_Parse(data);
    if (NULL == root)
    {
        printf("httpdns reponse get root faild !\n");
        pthread_mutex_unlock(&resolve_mutex);
        return -1;
    }

    cJSON *js_dns = cJSON_GetObjectItem(root, "dns");
    if (NULL == js_dns)
    {
        RKLog("httpdns no dns key!\n");
        pthread_mutex_unlock(&resolve_mutex);
        return -1;
    }

    int32_t array_size = cJSON_GetArraySize(js_dns);
    RKLog("host array num is %d\n",array_size);

    g_hostips.host_num = array_size;
    g_hostips.host = (host_ips **)malloc(sizeof(host_ips *) * array_size);

    for (int i = 0; i < array_size; i++)
    {
        item = cJSON_GetArrayItem(js_dns, i);
        if (NULL == item)
        {
           pthread_mutex_unlock(&resolve_mutex);
           return -1;
        }

        p = cJSON_PrintUnformatted(item);
        it = cJSON_Parse(p);
        if (NULL == it)
        {
            continue ;
        }

        js_host = cJSON_GetObjectItem(it, "host");
        len = strlen(js_host->valuestring);
        g_hostips.host[i] = (host_ips*)malloc(sizeof(host_ips));
        g_hostips.host[i]->h_name = (char*)malloc(len+1);

        memset(g_hostips.host[i]->h_name, 0, len+1);
        memcpy(g_hostips.host[i]->h_name, js_host->valuestring, len);
        RKLog("g_hostips.host[i]->h_name =%s \n", g_hostips.host[i]->h_name);

        js_ips = cJSON_GetObjectItem(it, "ips");
        ips_array_size = cJSON_GetArraySize(js_ips);
        RKLog("ips is array  %d\n", ips_array_size);

        g_hostips.host[i]->ips_num = ips_array_size;
        if (ips_array_size > 0)
        {
            g_hostips.host[i]->h_ips = (char **)malloc(sizeof(char *) * ips_array_size);
            for (cnt=0; cnt < ips_array_size; cnt++)
            {
                sub = cJSON_GetArrayItem(js_ips, cnt);
                if (NULL == sub )
                {
                    continue;
                }

                value = sub->valuestring;
                len = strlen(value);
                g_hostips.host[i]->h_ips[cnt] = (char*)malloc(len+1);
                memset(g_hostips.host[i]->h_ips[cnt], 0, len+1);
                memcpy(g_hostips.host[i]->h_ips[cnt], value, len);
            }
        }
    }

    cJSON_Delete(root);
    pthread_mutex_unlock(&resolve_mutex);
    return realsize;
}

static int httpdns_debug_callback(CURL *curl, curl_infotype type, char *data, size_t size, void *userp)
{
    char *url = NULL;
    curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
    RKLog("[LCHTTP DEBUG]: %s, %s", url, data);

    return 0;
}

static int httpdns_build_url_format(char *host, char* server_ip)
{
    memset(httpdns_url, 0, HTTP_HEARDR_SIZE);
    sprintf(httpdns_url, "http://%s/%d/resolve?host=%s", server_ip,ACCOUNT_ID, host);
    RKLog("httpdns url = %s\n", httpdns_url);

    return 0;
}

static void httpdns_free_host_ips(void)
{
    /*free the last saved mem and pay attention to reentrant problem.
    Please remove the data in time.*/
    pthread_mutex_lock(&resolve_mutex);
    if (0 != g_hostips.host_num)
    {
        for (int i = 0; i < g_hostips.host_num; i++)
        {
            //free all host mem
            if (NULL != g_hostips.host[i])
            {
                if (NULL != g_hostips.host[i]->h_name)
                {
                    //free the space occupied by the name.
                    free(g_hostips.host[i]->h_name);
                    g_hostips.host[i]->h_name = NULL;
                }

                if (0 != g_hostips.host[i]->ips_num)
                {
                    for (int k=0; k < g_hostips.host[i]->ips_num; k++)
                    {
                        //free the space occupied by the IP address.
                        if (NULL != g_hostips.host[i]->h_ips[k])
                        {
                            free(g_hostips.host[i]->h_ips[k]);
                            g_hostips.host[i]->h_ips[k] = NULL;
                        }
                    }
                    //free one host's ip list
                    if (NULL != g_hostips.host[i]->h_ips)
                    {
                        free(g_hostips.host[i]->h_ips);
                        g_hostips.host[i]->h_ips = NULL;
                    }
                }

                if (NULL != g_hostips.host[i])
                {
                    free(g_hostips.host[i]);
                    g_hostips.host[i] = NULL;
                }
            }
        }

        if (NULL != g_hostips.host)
        {
            free(g_hostips.host);
            g_hostips.host = NULL;
        }
    }

    g_hostips.host_num = 0;
    pthread_mutex_unlock(&resolve_mutex);
    return;
}

static int32_t httpdns_request_ips(char *host, char *server_ip)
{
    long response_code = 0;
    if (NULL == host || NULL == server_ip)
    {
        RKLog("request httpdns Parameter error\n");
        return -1;
    }

    CURL *curl = curl_easy_init();
    httpdns_build_url_format(host,server_ip);

    curl_easy_setopt(curl, CURLOPT_URL, httpdns_url);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, httpdns_timeout);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, httpdns_header_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpdns_response_callback);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, httpdns_debug_callback);

    CURLcode code = curl_easy_perform(curl);
    if (code == CURLE_OK)
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        RKLog("httpdnscurl response_code = %d \n",response_code);
    }
    else
    {
        RKLog("httpdns error info = %s\n",curl_easy_strerror(code));
    }

    // cleanup curl object
    curl_easy_cleanup(curl);
    if (200 != response_code)
    {
        return -1;
    }

    return 0;
}

static void *resolve_hosts_thread(void *host)
{
    int32_t cnt = 0;
    int32_t ret = 0;

    if (NULL == host)
    {
        RKLog("please check your hostlist !!!\n");
        return;
    }

    RKLog("hostlist =%s\n ",(char*)host);
    httpdns_free_host_ips();
    for (cnt=0; cnt < HTTPDNS_SERVER_NUM; cnt++)
    {
        ret = httpdns_request_ips(host, httdns_server_list[cnt]);
        if (0 == ret)
        {
            RKLog("Hi httpdns Request success !!!\n");
            resolve_status = HTTPDNS_RESOLVE_END;
            return;
        }
    }

    if (cnt >= HTTPDNS_SERVER_NUM)
    {
        RKLog("Sorry httpdns Request failure !!!\n");
        resolve_status = HTTPDNS_RESOLVE_END;
        return;
    }
}


int32_t httpdns_service_init(void)
{
    CURLcode ecode;
    if ((ecode = curl_global_init(CURL_GLOBAL_ALL)) != CURLE_OK)
    {
        RKLog("curl_global_init failure, code:%d %s.\n", ecode, curl_easy_strerror(ecode));
        return -1;
    }
    pthread_mutex_init(&resolve_mutex, NULL);
    return 0;
}

int32_t httpdns_service_destroy(void)
{
    curl_global_cleanup();
    pthread_mutex_destroy(&resolve_mutex);
    return 0;
}

int32_t httpdns_resolve_hosts(char *host, int32_t timeout)
{
    pthread_t thread;
    if(NULL == host)
    {
        return -1;
    }

    RKLog("pthread_create is start \n");
    if(resolve_status == HTTPDNS_RESOLVE_ING)
    {
        return -1;
    }
    else
    {
        resolve_status = HTTPDNS_RESOLVE_ING;
    }

    //default curl execte timeout  is 5 seconds
    httpdns_timeout = timeout > 0 ? timeout : 5;

    if (pthread_create(&thread, NULL, resolve_hosts_thread, host))
    {
        RKLog("pthread_create is faild \n");
        resolve_status = HTTPDNS_RESOLVE_END;
        return -1;
    }
    return 0;
}

ips_list *httpdns_getips_by_host(void)
{
    pthread_mutex_lock(&resolve_mutex);
    if (0 != g_hostips.host_num)
    {
        pthread_mutex_unlock(&resolve_mutex);
        return &g_hostips;
    }
    else
    {
        pthread_mutex_unlock(&resolve_mutex);
        return NULL;
    }
}


