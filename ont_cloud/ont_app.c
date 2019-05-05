/*
 * 
 * Desc: APP <----> 终端。
 * Date: 2017-06-25
 * Ver: 1.0
 * More: 
 */

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include "nvram.h"
#include "ont/platform.h"
#include "ont_handle.h"

#define MAX_LEN			1024

ONT_HANDLE ont_app_handle[] =
{
	{MSG_WTP_SCAN_DEVICES_REQ, &ont_wtp_scan_devices_req, "APP discovery", 0},

};


/*
 * name: get initinfo req
 * desc: 设备接入平台第一件事是获取初始化信息。
 * direction: WTP ----> APP
 * other: 
*/
void send_msg_to_APP(int sockfd, char *jsonValue, struct sockaddr *cliaddr)
{
	int err;

	SL_DEBUG("jsonValue len: %d", strlen(jsonValue));
	SL_DEBUG("client IP: %s, PORT: %u", 
				inet_ntoa(((struct sockaddr_in *)cliaddr)->sin_addr), ntohs(((struct sockaddr_in *)cliaddr)->sin_port));
	err = sendto(sockfd, jsonValue, strlen(jsonValue), 0, cliaddr, sizeof(struct sockaddr_in));
	if (-1 == err)
	{
		SL_DEBUG("sendto failed");
		perror("sendto");
	}
}

/* APP 与终端的接口不同，不用按照终端与平台的接口规则，加消息头。 */
/*
 * name: app scan devices req
 * desc: APP发现，终端响应APP的接口。
 * direction: WTP <---- APP
 * other: 
*/
int ont_wtp_scan_devices_req(int sockfd, const char *rcvMsg, struct sockaddr *cliaddr)
{
	SL_DEBUG("check rcvMsg: %s", rcvMsg);

	cJSON *json = NULL;
	char *jsonValue = NULL;
	char *ssid = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	ssid = nvram_get(RT2860_NVRAM, "SSID1");
	if (ssid == NULL || strlen(ssid) == 0)
	{
		ssid = "CMCC_unknown";
	}

	cJSON_AddItemToObject(json, "msgType", cJSON_CreateString(MSG_WTP_ONLINE_NOTIFICATION_REQ));
	cJSON_AddItemToObject(json, "name", cJSON_CreateString(ssid));
	cJSON_AddItemToObject(json, "MAC", cJSON_CreateString(g_mac));
	cJSON_AddItemToObject(json, "IP", cJSON_CreateString(get_ipaddr(get_ifname()))); //repeater mode, wan_ip
	cJSON_AddItemToObject(json, "deviceID", cJSON_CreateString(g_sn));

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("%s, json len: %d", jsonValue, strlen(jsonValue));

	send_msg_to_APP(sockfd, jsonValue, cliaddr);

	SL_DEBUG();
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return ONT_CMD_OK;
}

void start_ont_app(void)
{
	int sockfd = 0;
	struct sockaddr_in svraddr, cliaddr;
	int n, i;
	socklen_t len;
	char rcvMsg[MAX_LEN];
	cJSON *json = NULL;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sockfd)
	{
		SL_DEBUG("create socket failed");
		perror("socket");
		return ;
	}

	memset(&svraddr, 0x0, sizeof(svraddr));
	svraddr.sin_family = AF_INET;
	svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svraddr.sin_port = htons(ONT_APP_PORT);

	if (-1 == bind(sockfd, (struct sockaddr *)&svraddr, sizeof(svraddr)))
	{
		perror("bind");
		close(sockfd);
		return ;
	}

	for (;;)
	{
		SL_DEBUG();
		n = recvfrom(sockfd, rcvMsg, MAX_LEN, 0, (struct sockaddr *)&cliaddr, &len);
		if (-1 == n)
		{
			perror("recvfrom");
			printf("ont app recvfrom error");
			close(sockfd);
			return ;
		}
		SL_DEBUG("client IP: %s, PORT: %u", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
		cliaddr.sin_port = htons(ONT_APP_PORT); //set port
		SL_DEBUG("rcvMsg: %s", rcvMsg);
		rcvMsg[n] = '\0';
		SL_DEBUG("cheeck rcvMsg: %s", rcvMsg);

		json = cJSON_Parse(rcvMsg);
		if (!json) 
		{
			SL_DEBUG("json Parse error");
			continue; //to wait recvfrom
		}

		ONT_CHECK_JSON(json) //failed
		{
			SL_DEBUG("cJSON format error");
			cJSON_Delete(json);
			continue; //to wait recvfrom
		}
		else
		{
			char *msm_type = cJSON_GetObjectItem(json, "msgType")->valuestring;
			SL_DEBUG("msgType data: %s\n", msm_type);
			for (i = 0; i < ONT_ARRAY_SIZE(ont_app_handle); i++)
			{
				if (!memcmp(msm_type, ont_app_handle[i].name, strlen(ont_app_handle[i].name)))
				{
					SL_DEBUG("Matched_handle name: %s", ont_app_handle[i].name);
					ont_app_handle[i].func(sockfd, rcvMsg, (struct sockaddr *)&cliaddr);
				}
			}
		}
		
		cJSON_Delete(json);
		ont_platform_sleep(30);
	}

	return ;
}

void thread_ont_app(void)
{
	while (1)
	{
		ont_platform_sleep(5*1000);

		start_ont_app();
	}
}

