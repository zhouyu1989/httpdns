#include "../include/httpdns.h"
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
int g_httpdns_initd = 0;

static int callback(int status, void *userdata)
{
    int ret = 0;
    char ip[32] = { 0 };
    if (0 == status)
    {
        printf("%s %d status =%d\n", __func__, __LINE__, status);
        memset((char *)ip, 0, 32);
        ret = httpdns_getips_by_host("apigwrest.open.rokid.com", ip);
        if (-1 != ret)
        {
            printf("ip= %s\n", ip);
            printf("httpdns get ips is success \n");
        }
        else
        {
            printf("httpdns get ips is failed \n");
        }
    }
    else
    {
        printf("%s %d status =%d\n", __func__, __LINE__, status);
    }
}

int main(int argc, char *argv[])
{
    int i = 0;
    int timeout_ms = 5*1000;

    if (1 != g_httpdns_initd)
    {
        g_httpdns_initd = 1;
        httpdns_service_init();
    }

    for (i = 0; i < 1000; i++)
    {
        httpdns_resolve_gslb("0502031835000248", "", timeout_ms, callback, NULL);
        sleep(10);
    }
    return 0;
}
