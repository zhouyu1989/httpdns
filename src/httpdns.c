#include "httpdns.h"
#include "cjson/cJSON.h"
#include "rklog/RKLog.h"
#include <curl/curl.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define HTTP_HEARDR_SIZE 256
#define SN_MAX_SIZE 64
#define TYPE_MAX_SIZE 64

static ips_list g_hostips;
static long g_resolve_timeout = 0;
static pthread_mutex_t g_resolve_mutex;

static char g_sn[SN_MAX_SIZE] = { 0 };
static char g_device_type[TYPE_MAX_SIZE] = { 0 };
static char g_resoleve_url[HTTP_HEARDR_SIZE] = { 0 };

typedef enum {
    HTTPDNS_RESOLVE_END = 0,
    HTTPDNS_RESOLVE_ING
} resolve_status_t;

resolve_status_t resolve_status = HTTPDNS_RESOLVE_END;

static int32_t httpdns_header_callback(char *data, size_t size, size_t nitems, void *userp)
{
    int32_t realsize = nitems * size;
    if (0 == realsize)
    {
        return 0;
    }

    RKLog("httpdns_header =%s \n", data);
    return realsize;
}

static int32_t httpdns_response_callback(void *data, int32_t size, int32_t nmemb, void *userp)
{
    int32_t realsize = size * nmemb;
    int32_t ips_array_size = 0;
    char *value = NULL;
    char *p = NULL;
    cJSON *item = NULL, *it = NULL, *js_host = NULL, *js_ips = NULL, *sub = NULL;
    int cnt = 0, len = 0;

    if (0 == realsize)
    {
        return 0;
    }

    RKLog("\n %s \n", (char *)data);
    pthread_mutex_lock(&g_resolve_mutex);
    cJSON *root = cJSON_Parse(data);
    if (NULL == root)
    {
        printf("httpdns reponse get root faild !\n");
        pthread_mutex_unlock(&g_resolve_mutex);
        return -1;
    }

    cJSON *js_dns = cJSON_GetObjectItem(root, "data");
    if (NULL == js_dns)
    {
        RKLog("httpdns no dns key!\n");
        pthread_mutex_unlock(&g_resolve_mutex);
        return -1;
    }

    int32_t array_size = cJSON_GetArraySize(js_dns);
    RKLog("host array num is %d\n", array_size);

    g_hostips.host_num = array_size;
    g_hostips.host = (host_ips **)malloc(sizeof(host_ips *) * array_size);

    for (int i = 0; i < array_size; i++)
    {
        item = cJSON_GetArrayItem(js_dns, i);
        if (NULL == item)
        {
            pthread_mutex_unlock(&g_resolve_mutex);
            return -1;
        }

        p = cJSON_PrintUnformatted(item);
        it = cJSON_Parse(p);
        if (NULL == it)
        {
            continue;
        }

        js_host = cJSON_GetObjectItem(it, "host");
        len = strlen(js_host->valuestring);
        g_hostips.host[i] = (host_ips *)malloc(sizeof(host_ips));
        g_hostips.host[i]->h_name = (char *)malloc(len + 1);

        memset(g_hostips.host[i]->h_name, 0, len + 1);
        memcpy(g_hostips.host[i]->h_name, js_host->valuestring, len);
        RKLog("g_hostips.host[i]->h_name =%s \n", g_hostips.host[i]->h_name);

        js_ips = cJSON_GetObjectItem(it, "ips");
        ips_array_size = cJSON_GetArraySize(js_ips);
        RKLog("ips is array  %d\n", ips_array_size);

        g_hostips.host[i]->ips_num = ips_array_size;
        if (ips_array_size > 0)
        {
            g_hostips.host[i]->h_ips = (char **)malloc(sizeof(char *) * ips_array_size);
            for (cnt = 0; cnt < ips_array_size; cnt++)
            {
                sub = cJSON_GetArrayItem(js_ips, cnt);
                if (NULL == sub)
                {
                    continue;
                }

                value = sub->valuestring;
                len = strlen(value);
                g_hostips.host[i]->h_ips[cnt] = (char *)malloc(len + 1);
                memset(g_hostips.host[i]->h_ips[cnt], 0, len + 1);
                memcpy(g_hostips.host[i]->h_ips[cnt], value, len);
                RKLog("GET ip = %s\n", g_hostips.host[i]->h_ips[cnt]);
            }
        }
    }

    cJSON_Delete(root);
    pthread_mutex_unlock(&g_resolve_mutex);
    return realsize;
}

static int httpdns_debug_callback(CURL *curl, curl_infotype type, char *data, size_t size, void *userp)
{
    char *url = NULL;
    curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
    RKLog("[LCHTTP DEBUG]: %s, %s", url, data);

    return 0;
}

static int httpdns_build_url_format(void)
{
    memset(g_resoleve_url, 0, HTTP_HEARDR_SIZE);
    sprintf(g_resoleve_url, "https://gslb-dev.rokid.com/api/v1/?sn=%s&devicetype=%s", g_sn, g_device_type);
    RKLog("httpdns url = %s\n", g_resoleve_url);

    return 0;
}

static void httpdns_free_host_ips(void)
{
    /*free the last saved mem and pay attention to reentrant problem.
    Please remove the data in time.*/
    pthread_mutex_lock(&g_resolve_mutex);
    if (0 != g_hostips.host_num)
    {
        for (int i = 0; i < g_hostips.host_num; i++)
        {
            // free all host mem
            if (NULL != g_hostips.host[i])
            {
                if (NULL != g_hostips.host[i]->h_name)
                {
                    // free the space occupied by the name.
                    free(g_hostips.host[i]->h_name);
                    g_hostips.host[i]->h_name = NULL;
                }

                if (0 != g_hostips.host[i]->ips_num)
                {
                    for (int k = 0; k < g_hostips.host[i]->ips_num; k++)
                    {
                        // free the space occupied by the IP address.
                        if (NULL != g_hostips.host[i]->h_ips[k])
                        {
                            free(g_hostips.host[i]->h_ips[k]);
                            g_hostips.host[i]->h_ips[k] = NULL;
                        }
                    }
                    // free one host's ip list
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
    pthread_mutex_unlock(&g_resolve_mutex);
    return;
}

static int httpdns_lookup_host_ip(char *host_name, char *ip)
{
    if ((NULL == host_name) || (NULL == ip))
    {
        RKLog("lookup httpdns parameter error !\n");
        return -1;
    }

    int k = 0;
    if (0 != g_hostips.host_num)
    {
        RKLog("ips_list->host_num =%d \n", g_hostips.host_num);
        for (k = 0; k < g_hostips.host_num; k++)
        {
            RKLog("h_name=%s %d\n", g_hostips.host[k]->h_name, g_hostips.host[k]->ips_num);
            if (0 == strncmp(host_name, g_hostips.host[k]->h_name, strlen(host_name)))
            {
                memcpy(ip, g_hostips.host[k]->h_ips[0], strlen(g_hostips.host[k]->h_ips[0]));
                RKLog("ip=%s %s\n", ip, g_hostips.host[k]->h_ips[0]);
                return 0;
            }
        }
    }

    return -1;
}

static int32_t httpdns_request_ips(void)
{
    long response_code = 0;

    CURL *curl = curl_easy_init();
    httpdns_build_url_format();

    curl_easy_setopt(curl, CURLOPT_URL, g_resoleve_url);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, g_resolve_timeout);

    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, httpdns_header_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpdns_response_callback);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, httpdns_debug_callback);

    CURLcode code = curl_easy_perform(curl);
    if (code == CURLE_OK)
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        RKLog("httpdnscurl response_code = %ld\n", response_code);
    }
    else
    {
        RKLog("httpdns error info = %s\n", curl_easy_strerror(code));
    }

    // cleanup curl object
    curl_easy_cleanup(curl);

    RKLog("httpdnscurl response_code = %ld %d\n", response_code, code);
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
    int try_times = 3;

    httpdns_free_host_ips();
    for (cnt = 0; cnt < try_times; cnt++)
    {
        ret = httpdns_request_ips();
        if (0 == ret)
        {
            resolve_status = HTTPDNS_RESOLVE_END;
            RKLog("Celebrate httpdns Request OK cnt = %d !!!\n", cnt);
            return NULL;
        }
    }

    if (cnt >= try_times)
    {
        RKLog("Sorry httpdns Request failure cnt = %d !!!\n", cnt);
    }

    resolve_status = HTTPDNS_RESOLVE_END;
    return NULL;
}

int32_t httpdns_service_init(void)
{
    CURLcode ecode;
    if ((ecode = curl_global_init(CURL_GLOBAL_ALL)) != CURLE_OK)
    {
        RKLog("curl_global_init failure, code:%d %s.\n", ecode, curl_easy_strerror(ecode));
        return -1;
    }
    pthread_mutex_init(&g_resolve_mutex, NULL);
    return 0;
}

int32_t httpdns_service_destroy(void)
{
    curl_global_cleanup();
    pthread_mutex_destroy(&g_resolve_mutex);
    return 0;
}

int32_t httpdns_resolve_gslb(char *sn, char *device_type, int timeout)
{
    pthread_t thread;
    if (NULL == sn)
    {
        RKLog("resolve httpdns SN parameter error !\n");
        return -1;
    }

    if (resolve_status == HTTPDNS_RESOLVE_ING)
    {
        RKLog(" httpdns is resolveing !\n");
        return -1;
    }
    else
    {
        RKLog("pthread_create is start \n");
        resolve_status = HTTPDNS_RESOLVE_ING;
    }

    // default curl execte timeout  is 10 seconds
    g_resolve_timeout = timeout > 0 ? timeout : 10;

    memset(g_sn, 0, SN_MAX_SIZE);
    memcpy(g_sn, sn, strlen(sn));
    memset(g_device_type, 0, TYPE_MAX_SIZE);
    if (NULL != device_type)
    {
        memcpy(g_device_type, device_type, strlen(device_type));
        RKLog("g_device_type =%s \n", g_device_type);
    }
    RKLog("g_sn= %s g_resolve_timeout =%d \n", g_sn, g_resolve_timeout);

    if (pthread_create(&thread, NULL, resolve_hosts_thread, NULL))
    {
        RKLog("pthread_create is faild \n");
        resolve_status = HTTPDNS_RESOLVE_END;
        return -1;
    }
    return 0;
}

int httpdns_getips_by_host(char *host_name, char *ip)
{
    int ret = 0;
    if (NULL == host_name)
    {
        return -1;
    }
    RKLog("host_name =%s\n", host_name);
    pthread_mutex_lock(&g_resolve_mutex);
    if (0 != g_hostips.host_num)
    {
        ret = httpdns_lookup_host_ip(host_name, ip);
        pthread_mutex_unlock(&g_resolve_mutex);
        return ret;
    }
    else
    {
        pthread_mutex_unlock(&g_resolve_mutex);
        return -1;
    }
}
