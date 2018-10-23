
# HTTPDNS

---

## httpDNS功能简介
    - 1: HTTPDNS 是一款递归DNS服务，与权威DNS不同，HTTPDNS 并不具备决定解析结果的能力，而是主要负责解析过程的实现。
    - 2：使用 HTTP 协议访问阿里云的服务端，获得域名解析结果，绕过运营商的 Local DNS ，避免域名劫持。
    - 3：HTTPDNS 能够直接得到客户端的出口网关 IP，从而更准确地判断客户端地区和运营商，得到更精准的解析结果。
    - 4: HTTPDNS 支持全网域名的解析，包括在阿里云（万网）注册的域名，和其它第三方的域名。
## 使用HTTPDNS解析域名
    - 使用HTTPDNS解析域名，请求示例：http://203.107.1.33/100000/d?host=www.aliyun.com
    - 解析多个域名：http://203.107.1.33/100000/resolve?host=www.aliyun.com,www.taobao.com
    - 指定多个来源IP：http://203.107.1.33/100000/resolve?host=www.aliyun.com&ip=42.120.74.99,218.16.248.58
## API访问说明
    HTTPDNS通过HTTP接口对外提供域名解析服务，服务接入直接使用IP地址，服务IP有多个，这里以203.107.1.33这个服务IP为例，说明HTTPDNS服务的访问方式。

    请求方式：HTTP GET或HTTPS GET（两种请求方式的收费价格不同，请参考计费说明）

    HTTP服务URL：http://203.107.1.33/{account_id}/d

    HTTPS服务URL：https://203.107.1.33/{account_id}/d

    其中的{account_id}需要替换为用户的HTTPDNS Account ID，在HTTPDNS控制台上可以获得这个ID。
## API响应格式

-  解析结果JSON格式示例如下：
         {
          "dns": [
             {
               "host": "www.aliyun.com",
               "client_ip": "42.120.74.99",
               "ips": [
                 "140.205.32.12"
               ],
              "ttl": 106,
              "origin_ttl": 120
             },
             {
              "host": "www.taobao.com",
              "client_ip": "42.120.74.99",
               "ips": [
                 "140.205.16.92"
               ],
               "ttl": 46,
               "origin_ttl": 60
             }
           ]
         }
- 请求失败
请求失败时，HTTP响应的状态码为4xx/5xx，同时也返回具体的错误码，响应结果用JSON格式表示。

请求失败的响应示例：

    {
      "code": "MissingArgument"
    }


## C库提供的HTTPDNS 接口说明
- #define ALIYUN_SERVER_IP "203.107.1.33"   //阿里云httpDNS的服务器地址
- #define ACCOUNT_ID 131709                 //account_id 帐号

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

- typedef int (*httpdns_get_ips_callback)(ips_list *ips, void *ptr);

- int32_t  httpdns_service_init();

| 0个参数 |  类型  | 参数含义 | 数值说明 | 备注 |
|:----:|:----:|:----:|:------:|:------:|
| 参数 |  | httpdns 使用curl初始化类型| CURL_GLOBAL_ALL |  |

- int32_t httpdns_service_destroy()

| 0个参数 |  类型  | 参数含义 | 数值说明 | 备注 |
|:----:|:----:|:----:|:------:|:------:|
| 参数 |  | httpdns使用的curl释放空间的线程 |    |    |

- ips_list *httpdns_getips_by_host(char *host,int32_t timeout)

| 2个参数 |  类型  | 参数含义 | 数值说明 | 备注 |
|:----:|:----:|:----:|:------:|:------:|
| 参数1 | char*  | apigwrest.open.rokid.com,apigwws.open.rokid.com|所要请求的host|每次请求不能超过5个host |
| 参数2 | timeout  |每次请求超时的时长| |

-int httpdns_getips_by_host(httpdns_get_ips_callback callback, void *ptr);

| 1个返回值 |  类型  | 参数含义 | 数值说明 | 备注 |
|:----:|:----:|:----:|:------:|:------:|
| 返回值 | int | -1 失败， 0 成功 |所要请求的host|每次请求不能超过5个host |
| 参数 1 | httpdns_get_ips_callback |回调函数| |在回调里处理完成数据copy |
| 参数 2 | ptr | |上层拷贝ips 的内存指针| 在回调函数使用，将数据拷贝到自己的空间|


- 测试代码以及实例
1. sample 目录下的测试文件httpdns_samplae.c
2. 测试方式,测试调用接口已经在测试代码写好，如果测试 host name 直接编译运行就行
3. make httpdns-rebuild
4. ./httpdns_samplae
5. 测试host是否可以正常解析ip：curl http://203.107.1.33/131709/resolve?host=apigwrest.open.rokid.com

