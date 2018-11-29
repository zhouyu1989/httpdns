#include "../include/httpdns.h"
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
int g_httpdns_initd = 0;

int main(int argc, char *argv[])
{
    int i = 0;
    int ret = 0;
    int timeout = 10;
    char ip[32] = { 0 };

    if (1 != g_httpdns_initd)
    {
        g_httpdns_initd = 1;
        httpdns_service_init();
    }

    for (i = 0; i < 1000; i++)
    {
        httpdns_resolve_gslb("0502031835000248", "0ABA0AA4F71949C4A3FB0418BF025113", timeout);
        sleep(10);
        memset((char *)ip, 0, 32);
        ret = httpdns_getips_by_host("apigwrest.open.rokid.com", ip);
        printf("%s %d exetce =%d\n", __func__, __LINE__, i);
        if (-1 != ret)
        {
            printf("########## ip= %s\n", ip);
            printf("httpdns get ips is success \n");
        }
        else
        {
            printf("httpdns get ips is failed \n");
        }
    }
    return 0;
}
