#include "../src/httpdns.h"
#include <math.h>
char *host[]={
	"rapi.service.rokid.com",
	"www.aliyun.com",
	"www.baidu.com",
	"www.taobao.com",
	"apigwrest.open.rokid.com",
	NULL
};
char response[512] = {0,};
char **host_name =NULL;
int num =5;

ips_list *ips=NULL;
int main(int argc, char *argv[]){

	int i=0;
	int k =0,j=0;
#if 0
	host_name = (char **)malloc(sizeof(char *) * 5);
	for(i=0;i<6;i++)
	{

    	host_name[i] = (char *)malloc(sizeof(char) * 64); //分配每个指针所指向的数组
    	memset(host_name[i],0,64);
    	if(i==0) {
    		strcpy(host_name[i],"apigwrest.open.rokid.com");
        } 
        else if(i==1) {
        	strcpy(host_name[i],"rapi.service.rokid.com");
        }
        else if(i==2) {
        	strcpy(host_name[i],"account.service.rokid.com");
        }
        else if(i==3){
        	strcpy(host_name[i],"www.baidu.com");
        }
        else if(i==4) {
        	strcpy(host_name[i],"www.aliyun.com");
        }
        else {
        	strcpy(host_name[i],"www.taobao.com");
        }


    	printf("host_name[%d] = %s\n",i,host_name[i]);
	}
#endif
	char *host = "apigwrest.open.rokid.com,device-account.rokid.com,apigwws.open.rokid.com,wormhole-registry.rokid.com,wormhole.rokid.com";
	int timeout = 8;
    for(i=0;i<10000;i++){
     	httpdns_resolve_hosts(host,timeout);
	    sleep(10);
	    ips=httpdns_getips_by_host();
		printf("%s %d exetce num i==%d\n",__func__ , __LINE__,i);
		if(ips != NULL) {

			printf("ips_list->host_num =%d \n",ips->host_num);
			for(k =0;k<ips->host_num;k++)
			{
				printf("h_name=%s %d\n",ips->host[k]->h_name,ips->host[k]->ips_num);
				for(j=0;j<ips->host[k]->ips_num;j++)
				{
					printf("h_ips =%s\n",ips->host[k]->h_ips[j]);
				}
			}
		}

    }
    return 0;
}
