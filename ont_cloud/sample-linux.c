#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <ont/edp.h>
#include <ont/log.h>

#include "nvram.h"
#include "ont_handle.h"

char fw_version[32] = {0};
char g_sn[32] = {0};
char g_mac[18] = {0};

ONT_HANDLE ont_handle[] =
{
	/*   */
	{ MSG_GET_INITINFO_RSP , &ont_get_initinfo_rsp, "initialization", 0 }, //初始化信息，并将服务器返回的值设置成功。
	{ MSG_WTP_ABILITY_NOTIFY_RSP , &ont_wtp_ability_notify_rsp, "", 0 }, //完成属性的上报，方能正常使用。
#if defined (CONFIG_PRODUCT_339AA) || defined (CONFIG_PRODUCT_339AA_JS) || defined (CONFIG_PRODUCT_331AA)
	{ MSG_AP_GET_WIFI_SWITCH_STATUS_REQ , &ont_ap_get_wifi_switch_status_req, "Get AP WiFi switch status", 0 },
	{ MSG_WTP_GET_DEVICE_WIFI_INFO_REQ , &ont_ap_get_wifi_info_req, "Get AP WiFi info", 0 },
	{ MSG_WTP_SET_DEVICE_WIFI_REQ , &ont_ap_set_wifi_info_req, "Set AP WiFi info", 0 },
	{ MSG_WTP_GET_DEVICE_INFO_REQ , &ont_wtp_get_device_info_req, "Get device info", 0 },
#endif
	{ MSG_WTP_GET_DEVICE_WIFI_INFO_NEW_REQ , &ont_wtp_get_device_wifi_info_req, "Get WiFi info", 0 },
	{ MSG_WTP_SET_DEVICE_WIFI_NEW_REQ , &ont_wtp_set_device_wifi_req, "Set WiFi info", 0 },
	{ MSG_WTP_WIFI_ACCELERATE_REQ , &ont_wtp_wifi_accelerate_req, "WiFi accelerate", 0 },
	{ MSG_WTP_DEVICE_OPERATE_REQ , &ont_wtp_device_operate_req, "device operate", 0 },
	
	{ MSG_WTP_GET_ROUTE_OVERVIEW_REQ , &ont_wtp_get_route_overview_req, "platform/app request router overview", 0 },
	
	{ MSG_WTP_GET_BROADBAND_SETTING_REQ , &ont_wtp_get_boardband_setting_req, "platform/app get wan information", 0 },

	{ MSG_WTP_SET_BROADBAND_SETTING_REQ , &ont_wtp_set_boardband_setting_req, "platform/app set wan information", 0 },

	{ MSG_WTP_GET_ROUTE_BASE_INFO_REQ , &ont_wtp_get_base_info_req, "platform get route base information", 0 },

    { MSG_WTP_GET_CLIENT_ACCESS_CONTROL_INFO_REQ, &ont_wtp_get_access_control_info_req, "platform get access control information", 0 },

    { MSG_WTP_SET_CLIENT_ACCESS_CONTORL_REQ, &ont_wtp_set_acccess_control_req, "platform set access control information", 0 },

    { MSG_WTP_SET_QOS_REQ, &ont_wtp_set_qos_req, "platform set QoS rule", 0 },

	/*upgrade*/
	{ MSG_WTP_DEVICE_STATUS_CHANG_RSP , &ont_wtp_device_status_chang_rsp, "Device status, normal or upgrading", 0 },
	{ MSG_GET_DEVUPDATE_INFO_RSP , &ont_get_devupdate_info_rsp, "Upgrade status", 0 },
	{ MSG_SEND_DEVUPDATE_REQ , &ont_send_devupdate_req, "notice upgrade", 0 },

	/*log upload*/
	{ MSG_WTP_LOG_UPLOAD_RSP , &ont_wtp_log_upload_rsp, "Log upload rsp", 0 },
};

ONT_HANDLE ont_cmds[] = 
{
	/*CMD test*/
	{ "test" , &ont_cmd_test, "for test", 0 },
	{ "reset" , &ont_cmd_factory_reset, "factory reset", 0 },
	{ "reboot" , &ont_cmd_reboot, "reboot", 0 },
};

static void sample_log(void *ctx, ont_log_level_t ll , const char *format, ...)
{
    static const char *level[] = {
	"DEBUG", "INFO", "WARNING", "ERROR", "FATAL"
    };
    va_list ap;
    struct timeval tv;
    time_t t;
    struct tm *st;

    gettimeofday(&tv, NULL);

    t = tv.tv_sec;
    st = localtime(&t);

    printf("[%02d:%02d:%02d.%03d] [%s] ", st->tm_hour, st->tm_min,
	    st->tm_sec, (int)tv.tv_usec/1000, level[ll]);

    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
    printf("\n");
}
void trans_data_cb_func(ont_device_t *dev, const char*svr_name,size_t svr_len,
	const char* data,size_t data_len)
{
	char *buf = NULL;
    int i = 0;
	static int supported;
    printf("********trans_cb**********\n");
    printf("get %d bytes svr_name: ",(int)svr_len);
    while(i<svr_len)
    {
	printf("%c",svr_name[i++]);
    }
    printf("\n");
    i = 0;
    printf("get %d bytes data: ",(int)data_len);
    while(i<data_len)
    {
	printf("%c",data[i++]);
    }
    printf("\n");
	buf = ont_platform_malloc(data_len+1);
	if (!buf)
	{
		SL_DEBUG("no space");
        return ONT_ERR_NOMEM;
	}
//#define RSP_DATE_TEST 
#ifdef RSP_DATE_TEST
    //char *ptr2 = "{\"msgType\":\"MSG_WTP_GET_DEVICE_INFO_REQ\",\"erroCode\":\"0x0000\",\"description\":\"get device info\"}";
    char *ptr2 = "{\"msgType\":\"MSG_WTP_GET_ROUTE_OVERVIEW_REQ\",\"erroCode\":\"0000\",\"description\":\"get route info\"}";
	SL_DEBUG("check ptr2 : %s__\n", ptr2);
	ont_platform_snprintf(buf, strlen(ptr2)+1, "%s", ptr2);
	//SL_DEBUG("check buf: %s__\n", buf);
#else
	ont_platform_snprintf(buf, data_len+1, "%s", data);
	//SL_DEBUG("check buf: %s__\n", buf);
#endif

	SL_DEBUG();
	cJSON *json = NULL;
	json = cJSON_Parse(buf);
	SL_DEBUG();
	if (!json) 
	{
		ont_platform_free(buf);
		return ONT_JSON_ERR;
	}

	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG();
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		ont_platform_free(buf);
		return ONT_COMMON_ERR;
	}
	else
	{
		SL_DEBUG();
		char *msm_type = cJSON_GetObjectItem(json, "msgType")->valuestring;
		SL_DEBUG("msgType data: %s\n", msm_type);
		supported = 0;
		for (i = 0; i < ONT_ARRAY_SIZE(ont_handle); i++)
		{
			if (!memcmp(msm_type, ont_handle[i].name, strlen(ont_handle[i].name)))
			{
				supported = 1;
				SL_DEBUG("Matched_handle name: %s", ont_handle[i].name);
				ont_handle[i].func(dev, buf, data_len+1);
			}
			else
			{
				//SL_DEBUG("Unsupported handle");
			}
		}
		if (0 == supported) //not supported
		{
			SL_DEBUG("Unsuppored msgType: %s", msm_type);
			ont_unsupported_msgtype_rsp(dev, msm_type, ONT_GET_MSGSEQ(json));
		}
	}

	ont_platform_free(buf);

	return ;
}
void pushdata_cb_func(const char*src_devid,size_t len,
	const char* data,size_t data_len)
{
    int i = 0;
    printf("********push_cb**********\n");
    printf("get %d bytes devid: ",(int)len);
    while(i<len)
    {
	printf("%c",src_devid[i++]);
    }
    printf("\n");
    i = 0;
    printf("get %d bytes data: ",(int)data_len);
    while(i<data_len)
    {
	printf("%c",data[i++]);
    }
    printf("\n");

}

int start_edp()
{
	ont_device_t *dev;
	int err, i;
	struct hostent *hosts;
    int32_t next_dp_time;
	char* trans_data = "I'm Andmu router data";
	ont_device_cmd_t *cmd = NULL;
	
	int flags;

#ifdef PRODUCT_RELEASE
	err = ont_device_create(91867, ONTDEV_EDP, "auto_register", &dev);
#else
	err = ont_device_create(85, ONTDEV_EDP, "auto_register", &dev);
#endif

	if (ONT_ERR_OK != err) {
		ONT_LOG1(ONTLL_ERROR, "Failed to create device instance, error=%d", err);
		return -1;
	}

	char auth_buf[256] = {0};
	//g_sn = "13111710999997";
	//g_mac = "F4:4C:70:51:19:B9";
	SL_DEBUG("g_sn: %s, g_mac: %s", g_sn, g_mac);

	if (g_sn == NULL || strlen(g_sn) == 0 ||
			g_mac == NULL || strlen(g_mac) == 0)
	{
		SL_DEBUG("No sn or mac.");
		goto err_exit;
	}

	ont_platform_snprintf(auth_buf, sizeof(auth_buf), 
			"{\"SN\":\"%s\",\"MAC\":\"%s\"}", 
			g_sn, g_mac);
	SL_DEBUG("auth:%s", auth_buf);

#ifdef PRODUCT_RELEASE
    hosts = gethostbyname(ONT_SERVER_ADDRESS);
	if (NULL == hosts)
	{
		SL_DEBUG("Failed to get the IP of the server('%s')",
			ONT_SERVER_ADDRESS);
		goto err_exit;
	}

	for (i = 0; hosts->h_addr_list[i]; ++i)
	{
	if (AF_INET == hosts->h_addrtype)
		break;
	}

	if (NULL == hosts->h_addr_list[i])
	{
		SL_DEBUG("No invalide address of the server.");
		goto err_exit;
	}
#endif
		
	ont_edp_set_transdata_cb(dev,trans_data_cb_func);
	ont_edp_set_pushdata_cb(dev,pushdata_cb_func);

    err = ont_device_connect_without_autoReg(dev,
#ifdef PRODUCT_RELEASE
		inet_ntoa(*(struct in_addr*)hosts->h_addr_list[i]),
#else
		"183.230.40.149",
#endif
		ONT_SERVER_PORT,
#ifdef PRODUCT_RELEASE
		"zuwang",
#else
	    "omq_test",
#endif
		auth_buf,
		30);

	if (ONT_ERR_OK != err)
	{
		ONT_LOG1(ONTLL_ERROR, "Failed to connect to the server, error=%d", err);
		goto err_exit;
	}

	err = ont_get_initinfo_req(dev);

	/* init */
	ont_wtp_device_status_chang_req(dev, UPGRADE_COM);
	
	/* do something about upload log */
	err = ont_wtp_log_upload_req(dev, "upload log test");
	if (ONT_ERR_OK != err)
		SL_DEBUG("ont_wtp_log_upload_req failed!");

    next_dp_time = (int32_t)time(NULL) + 3;

	while (1)
	{
		err = ont_device_keepalive(dev);
		if (ONT_ERR_OK != err)
		{
			ONT_LOG1(ONTLL_ERROR, "device is not alive, error=%d", err);
			break;
		}

		if (next_dp_time <= time(NULL))
		{
			next_dp_time = (int32_t)time(NULL) + 3;

			static unsigned int flag = 0;

			//request upgrade info every day
			// 3 second one time
			if (10 == (flag % (ONE_DAY/3)))
			{
				err = ont_get_devupdate_info_req(dev);
				if (ONT_ERR_OK != err)
					SL_DEBUG("ont_get_devupdate_info_req failed");
			}

			flag++;
			//SL_DEBUG("Send data points successfully.");
		}

		ont_platform_sleep(50);/* 50 ms*/
	}

err_exit:
	ont_device_destroy(dev);

	return -1;
}

char *get_ifname(void)
{
	char *opMode = NULL;
	static char *ifname = "br0";
	opMode = nvram_get(RT2860_NVRAM, "OperationMode");
	const char *vlanEnble, *vlanID;
	vlanEnble = nvram_bufget(RT2860_NVRAM, "VLANEnable");
	vlanID = nvram_bufget(RT2860_NVRAM, "VLANID");
	
	if (strlen(opMode))
	{
		if (!strcmp(opMode, "0")) //bridge
			ifname = "br0";
		else if (!strcmp(opMode, "1")) //gateway. do something, get UPLINK TYPE
		{
			if(0==strncmp(vlanEnble,"1",2))
			{
				static char if_name_buff[64] = {0};
				sprintf(if_name_buff,"eth2.2.%s",vlanID);
				ifname = if_name_buff;
			}
			else
			{
				ifname = "eth2.2";
			}
		}
		else if (!strcmp(opMode, "3")) //AP client
			ifname = "apcli0";
		else
			ifname = "br0";
	}

	return ifname;
}

void ont_init(void)
{
	while(0)
	{
		/* wait network. do something */
	}

	char buf[32];
	/* get fw_version */	
	FILE *fp = fopen("/etc_ro/version", "r");
	if (fp != NULL)
	{
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			if (strstr(buf, "_V"))
				{
					sscanf(buf, "%s", fw_version);
					break;
				}
		}

		fclose(fp);
	}
	SL_DEBUG("ver: %s", fw_version);

	/* get sn */
	memset(buf, 0x0, sizeof(buf));
	char *ptr = NULL;

	FILE *pp = popen("eth_mac g sn", "r");
	if (pp)
	{
		while(fgets(buf, sizeof(buf), pp))
		{
			if (strlen(buf))
			{
				ptr = strstr(buf, "SN:");
				if (ptr && (strlen(ptr) > 4))
				{
					ptr += strlen("SN:");
					strncpy(g_sn, ptr, SN_LEN);
					g_sn[strlen(g_sn)-1] = '\0'; //cut '\n'
				}
			}
		}

		pclose(pp);
	}
	SL_DEBUG("sn: %s", g_sn);

	/* g_mac 一定要跟出货时，提供的MAC、SN信息对应上。 */
	char *pmac = NULL;
	pmac = get_macaddr("eth2");
	if (NULL != pmac)
	{
		memset(g_mac, 0, sizeof(g_mac));
		if(strlen(pmac) < 18)
		{
			strncpy(g_mac, pmac, strlen(pmac));
		}
	}
	
	SL_DEBUG("mac: %s", g_mac);

	return ;
}

void* thread_ont_cloud(void* arg)
{
    ont_init();
    while(1)
    {   
        start_edp();
		SL_DEBUG("edp try again!");
        ont_platform_sleep(5*1000);
    }   
}


int main( int argc, char *argv[] )
{

#define NUM_THREADS 2
	int rc, t;
	void *status;
	pthread_t thread[NUM_THREADS];

    /*初始化NVRAM*/
	nvram_init(RT2860_NVRAM);
    //start_edp();


    rc = pthread_create(&thread[0], NULL, thread_ont_cloud, &t);
#if defined (CONFIG_PRODUCT_339AA) || defined (CONFIG_PRODUCT_339AA_JS) || defined (CONFIG_PRODUCT_331AA)
    rc = pthread_create(&thread[1], NULL, thread_ont_app, &t);
#endif
    
    pthread_join(thread[0], &status);
#if defined (CONFIG_PRODUCT_339AA) || defined (CONFIG_PRODUCT_339AA_JS) || defined (CONFIG_PRODUCT_331AA)
    pthread_join(thread[1], &status);
#endif

	nvram_close(RT2860_NVRAM);
    SL_DEBUG("ont_cloud exit.\n");
    return 0;
}

