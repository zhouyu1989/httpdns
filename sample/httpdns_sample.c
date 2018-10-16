
#include <math.h>
#include "../include/httpdns.h"

ips_list *ips = NULL;
int main(int argc, char *argv[])
{
	int i = 0;
	int k = 0, j = 0;

	char *host = "apigwrest.open.rokid.com,device-account.rokid.com,\
				 apigwws.open.rokid.com,wormhole-registry.rokid.com,wormhole.rokid.com";
	int timeout = 8;
    for (i = 0; i < 10000; i++)
    {
		httpdns_resolve_hosts(host, timeout);
	    sleep(10);
	    ips = httpdns_getips_by_host();
		printf("%s %d exetce =%d\n", __func__, __LINE__, i);
		if (NULL != ips)
		{
			printf("ips_list->host_num =%d \n", ips->host_num);
			for (k = 0; k < ips->host_num; k++)
			{
				printf("h_name=%s %d\n", ips->host[k]->h_name, ips->host[k]->ips_num);
				for (j = 0; j < ips->host[k]->ips_num; j++)
				{
					printf("h_ips =%s\n", ips->host[k]->h_ips[j]);
				}
			}
		}

    }
    return 0;
}
