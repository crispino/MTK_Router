/*	
 * OneNet upgrade.
 * OneNet log upload
 * 2017-06-19
 * Ver: 1.0
 * more: 
*/
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

#include <wait.h>

#include "nvram.h"
#include "ont_handle.h"
//#include <network.h>

#define ONT_LOG printf
/*
 * name: device status change req
 * desc: 设备主动上报当前设备状态。正常或者升级中。
 		在线升级过程中，调用该接口上报当前升级状态，和deviceID，deviceID值可以传sn
 		status的状态根据当前升级状态返回:
 		1、准备下载固件前，返回升级中。
 		2、升级失败。
 		3、失败成功。
 * direction: WTP ----> AMNMP
 * other: 1:正常，2:升级中
*/
int ont_wtp_device_status_chang_req(ont_device_t *dev, char *status)
{
	int err;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	char msgSeq[32] = {0};
	srand((unsigned)time(NULL));
	ont_platform_snprintf(msgSeq, sizeof msgSeq, "%s-%d", DEVICE_STATUS_CHANGE, rand()%100000);

	addReqhead(json, VERSION_N, MSG_WTP_DEVICE_STATUS_CHANG_REQ, msgSeq);

	cJSON_AddItemToObject(json, "deviceID", cJSON_CreateString(g_sn));
	cJSON_AddItemToObject(json, "Status", cJSON_CreateString(status));

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("%s", jsonValue);
	err = ont_edp_send_transdata(dev, SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		return ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return ONT_CMD_OK;
}

/*
 * name: device status change rsp
 * desc: 上报当前设备状态后平台返回的消息。貌似不用做什么处理。
 * direction: WTP <---- AMNMP
 * other: 
*/
int ont_wtp_device_status_chang_rsp(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	return ONT_CMD_OK;
}

inline unsigned int getMTDPartSize(char *part)
{
	char buf[128], name[32], size[32], dev[32], erase[32];
	unsigned int result=0;
	FILE *fp = fopen("/proc/mtd", "r");
	if(!fp){
		fprintf(stderr, "mtd support not enable?");
		return 0;
	}
	while(fgets(buf, sizeof(buf), fp)){
		sscanf(buf, "%s %s %s %s", dev, size, erase, name);
		if(!strcmp(name, part)){
			result = strtol(size, NULL, 16);
			break;
		}
	}
	fclose(fp);
	return result;
}

/*
 *	Uboot image header format
 *	(ripped from mkimage.c/image.h)
 */
#define IH_MAGIC    0x27051956
#define IH_NMLEN    32
typedef struct image_header {
    uint32_t    ih_magic;   /* Image Header Magic Number    */
    uint32_t    ih_hcrc;    /* Image Header CRC Checksum    */
    uint32_t    ih_time;    /* Image Creation Timestamp */
    uint32_t    ih_size;    /* Image Data Size      */
    uint32_t    ih_load;    /* Data  Load  Address      */
    uint32_t    ih_ep;      /* Entry Point Address      */
    uint32_t    ih_dcrc;    /* Image Data CRC Checksum  */
    uint8_t     ih_os;      /* Operating System     */
    uint8_t     ih_arch;    /* CPU architecture     */
    uint8_t     ih_type;    /* Image Type           */
    uint8_t     ih_comp;    /* Compression Type     */
    uint8_t     ih_name[IH_NMLEN];  /* Image Name       */
} image_header_t;

/*
 *  taken from "mkimage -l" with few modified....
 */
int check(char *imagefile, int offset, int len, char *err_msg)
{
	struct stat sbuf;

	int  data_len;
	char *data;
	unsigned char *ptr;
	unsigned long checksum;

	image_header_t header;
	image_header_t *hdr = &header;

	int ifd;

	if ((unsigned)len < sizeof(image_header_t)) {
		sprintf (err_msg, "Bad size: \"%s\" is no valid image\n", imagefile);
		return 0;
	}

	ifd = open(imagefile, O_RDONLY);
	if(!ifd){
		sprintf (err_msg, "Can't open %s: %s\n", imagefile, strerror(errno));
		return 0;
	}

	if (fstat(ifd, &sbuf) < 0) {
		close(ifd);
		sprintf (err_msg, "Can't stat %s: %s\n", imagefile, strerror(errno));
		return 0;
	}

	ptr = (unsigned char *) mmap(NULL, sbuf.st_size, PROT_READ, MAP_SHARED, ifd, offset);
	if ((caddr_t)ptr == (caddr_t)-1) {
		close(ifd);
		sprintf (err_msg, "Can't mmap %s: %s\n", imagefile, strerror(errno));
		return 0;
	}
	ptr += offset;

	/*
	 *  handle Header CRC32
	 */
	memcpy (hdr, ptr, sizeof(image_header_t));

	if (ntohl(hdr->ih_magic) != IH_MAGIC) {
		munmap(ptr, len);
		close(ifd);
		sprintf (err_msg, "Bad Magic Number: \"%s\" is no valid image\n", imagefile);
		return 0;
	}

	data = (char *)hdr;

	checksum = ntohl(hdr->ih_hcrc);
	hdr->ih_hcrc = htonl(0);	/* clear for re-calculation */

	if (crc32 (0, data, sizeof(image_header_t)) != checksum) {
		munmap(ptr, len);
		close(ifd);
		sprintf (err_msg, "*** Warning: \"%s\" has bad header checksum!\n", imagefile);
		return 0;
	}

	/*
	 *  handle Data CRC32
	 */
	data = (char *)(ptr + sizeof(image_header_t));
	data_len  = len - sizeof(image_header_t) ;

	if (crc32 (0, data, data_len) != ntohl(hdr->ih_dcrc)) {
		munmap(ptr, len);
		close(ifd);
		sprintf (err_msg, "*** Warning: \"%s\" has corrupted data!\n", imagefile);
		return 0;
	}

	/*
	 * compare MTD partition size and image size
	 */
	if(len > getMTDPartSize("\"Kernel\"")){
		munmap(ptr, len);
		close(ifd);
		sprintf(err_msg, "*** Warning: the image file(0x%x) is bigger than Kernel MTD partition.\n", len);
		return 0;
	}

	munmap(ptr, len);
	close(ifd);

	return 1;
}

inline void write_flash_kernel_version(void)
{
	char cmd[512];
	char buf[512];
	FILE *fp = fopen("/proc/version", "r");
	fgets(buf, 512, fp);
	fclose(fp);

	sprintf(cmd, "nvram_set 2860 old_firmware \"%s\"", buf);
	system(cmd);

	return ;
}

inline int mtd_write_firmware(char *filename, int offset, int len)
{
	char cmd[512];
	int status;

	/* workaround: erase 8k sector by myself instead of mtd_erase */
	/* this is for bottom 8M NOR flash only */
	snprintf(cmd, sizeof(cmd), "/bin/flash -f 0x400000 -l 0x40ffff");
	system(cmd);

    snprintf(cmd, sizeof(cmd), "/bin/mtd_write -o %d -l %d write %s Kernel", offset, len, filename);
    status = system(cmd);

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		return -1;

    return 0;
}

/*
 * name: ont_download_upgrade
 * desc: 使用wget下载文件并执行升级动作。
 * direction: N/A
 * other:
*/
int ont_download_upgrade(char *url)
{
	if (NULL == url || 0 == strlen(url))
	{
		SL_DEBUG("Url error!");
		return ONT_URL_ERR;
	}

	char err_msg[256] = {0};
	int file_len = 0;

	int ifd;
	struct stat sbuf;

	/* init */
	do_system("rm -rf %s", UPLOAD_FILE);
	/* -t 0: 无限次尝试下载，适用于较差的网络环境。 */
	do_system("wget -t 0 -c -O %s %s", UPLOAD_FILE, url);

	/* Just waiting for wget. */

	ifd = open(UPLOAD_FILE, O_RDONLY);
	if(!ifd){
		sprintf (err_msg, "Can't open %s: %s\n", UPLOAD_FILE, strerror(errno));
		return ONT_FILE_ERR;
	}

	if (fstat(ifd, &sbuf) < 0) {
		close(ifd);
		sprintf (err_msg, "Can't stat %s: %s\n", UPLOAD_FILE, strerror(errno));
		return ONT_FILE_ERR;
	}

	file_len = sbuf.st_size;

	if(!check(UPLOAD_FILE, 0, file_len, err_msg) )
	{
		SL_DEBUG("Not a valid firmware. %s", err_msg);
		return ONT_FILE_ERR;
	}

	/*
	* write the current linux version into flash.
	*/
	write_flash_kernel_version();

	// flash write
	if(mtd_write_firmware(UPLOAD_FILE, 0, file_len) == -1)
	{
		SL_DEBUG("mtd_write fatal error! The corrupted image has ruined the flash!!");
		return ONT_WRITE_FLASH_ERR;
	}

	SL_DEBUG("Download and upgrade success!");
	
	return ONT_ERR_OK;
}

/*
 * name: get update info req
 * desc: 设备查询升级任务请求。
 * direction: WTP ----> AMNMP
 * other:
*/
int ont_get_devupdate_info_req(ont_device_t *dev)
{
	SL_DEBUG();
	int err;
	cJSON *json = NULL;
	char *jsonValue = NULL;

	 //2: 路由器 8：AP 16:Repeater
#if defined(CONFIG_PRODUCT_341AA) || defined(CONFIG_PRODUCT_340AA)
	char *type = "2";
#elif defined (CONFIG_PRODUCT_339AA) || defined (CONFIG_PRODUCT_339AA_JS) || defined (CONFIG_PRODUCT_331AA)
	char *type = "8";
#else
	/* Default */
	char *type = "2";
#endif

	SL_DEBUG("cur_ver: %s", fw_version);

	//timeout((timeout_fun *)&ont_get_devupdate_info_req, dev, ONE_DAY);

	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addReqhead(json, VERSION_N, MSG_GET_DEVUPDATE_INFO_REQ, DEV_UPDATE_INFO);

	cJSON_AddItemToObject(json, "devID", cJSON_CreateString(g_sn));
	cJSON_AddItemToObject(json, "type", cJSON_CreateString(type));
	cJSON_AddItemToObject(json, "currentVersion", cJSON_CreateString(fw_version));

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("%s", jsonValue);
	err = ont_edp_send_transdata(dev,SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		return ONT_RSP_ERR;
	}

	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return ONT_CMD_OK;
}

/*
 * name: get update info rsp
 * desc: 平台响应当前的升级任务信息
 * direction: WTP <---- AMNMP
 * other: 
*/
int ont_get_devupdate_info_rsp(ont_device_t *dev, const char *data, size_t data_len)
{
	int err;
	SL_DEBUG("check data: %s", data);

	cJSON *json = NULL;
	char *needUpdate = NULL, *updateUrl = NULL, *updateDescription = NULL; 

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		needUpdate = cJSON_GetObjectItem(json, "needUpdate")->valuestring;

		//0:should't update, 1:update forced, 2:update
		switch (atoi(needUpdate))
		{
			case 0:
				SL_DEBUG("no need update");
				break;
			case 2:
				SL_DEBUG("waiting for upgrading notice");
				break;
			case 1:
				SL_DEBUG("need update");
				
				updateUrl = cJSON_GetObjectItem(json, "updateUrl")->valuestring;
				SL_DEBUG("Upgrade URL: %s", updateUrl);
				updateDescription = cJSON_GetObjectItem(json, "updateDescription")->valuestring;
				
				ont_wtp_device_status_chang_req(dev, UPGRADING);
				ont_platform_sleep(100);

				err = ont_download_upgrade(updateUrl);
				if (ONT_CMD_OK == err)
				{
					ont_wtp_device_status_chang_req(dev, UPGRADE_COM);
					ONT_LOG("ont upgrade success");
					ont_sys_reboot(dev);
				}
				/* 细化升级失败的情况，只有写入FLASH失败才重启。 */
				else if (ONT_WRITE_FLASH_ERR == err)
				{
					ont_wtp_device_status_chang_req(dev, UPGRADE_FAL);
					ONT_LOG("ont upgrade failed, errno:(%d)", err);
					ont_sys_reboot(dev);
				}
				/* 其他情况，如URL不合法，下载文件失败都不重启。 */
				else
				{
					ont_wtp_device_status_chang_req(dev, UPGRADE_FAL);
					ONT_LOG("ont upgrade failed, errno:(%d)", err);
					do_system("rm -rf %s", UPLOAD_FILE);
				}
				break;

			default:
				SL_DEBUG("What do you want to do?");
				break;
		}
	}

	cJSON_Delete(json);

	return ONT_CMD_OK;
}

/*
 * name: send update rsp
 * desc: 平台通知终端升级后，终端应该先返回响应消息头。无可变消息。
 * direction: WTP ----> AMNMP
 * other: 
*/
static int ont_send_devupdate_rsp(ont_device_t *dev, char *msgSeq, char *errCode)
{
	int err;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addRsphead(json, VERSION_N, MSG_SEND_DEVUPDATE_RSP, msgSeq, errCode, "send devupdate rsp");

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("rsp: %s", jsonValue);
	err = ont_edp_send_transdata(dev,SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		return ONT_RSP_ERR;
	}
	
	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return ONT_CMD_OK;
}

/*
 * name: send update req
 * desc: 通知升级接口，平台主动向终端发起。
 		这个接口感觉和上面的重复。
 * direction: WTP <---- AMNMP
 * other: 
*/
int ont_send_devupdate_req(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	int err;
	cJSON *json = NULL;
	char *updateUrl = NULL, *updateDescription = NULL; 

	json = cJSON_Parse(data);
	if (!json) return ONT_JSON_ERR;
	
	ONT_CHECK_JSON(json) //failed
	{
		SL_DEBUG("cJSON format error");
		cJSON_Delete(json);
		return ONT_COMMON_ERR;
	}
	else
	{
		SL_DEBUG("update notice");
		updateUrl = cJSON_GetObjectItem(json, "updateUrl")->valuestring;
		SL_DEBUG("Upgrade URL: %s", updateUrl);
		updateDescription = cJSON_GetObjectItem(json, "updateDescription")->valuestring;

		ont_wtp_device_status_chang_req(dev, UPGRADING);
		ont_platform_sleep(100);

		ont_send_devupdate_rsp(dev, ONT_GET_MSGSEQ(json), ONT_SUCCESS);
		err = ont_download_upgrade(updateUrl);
		if (ONT_CMD_OK == err)
		{
			ont_wtp_device_status_chang_req(dev, UPGRADE_COM);
			ONT_LOG("ont upgrade success");
			ont_sys_reboot(dev);
		}
		/* 细化升级失败的情况，只有写入FLASH失败才重启。 */
		else if (ONT_WRITE_FLASH_ERR == err)
		{
			ont_wtp_device_status_chang_req(dev, UPGRADE_FAL);
			ONT_LOG("ont upgrade failed, errno:(%d)", err);
			ont_sys_reboot(dev);
		}
		/* 其他情况，如URL不合法，下载文件失败都不重启。 */
		else
		{
			ont_wtp_device_status_chang_req(dev, UPGRADE_FAL);
			ONT_LOG("ont upgrade failed, errno:(%d)", err);
			do_system("rm -rf %s", UPLOAD_FILE);
		}
	}

	cJSON_Delete(json);

	return ONT_CMD_OK;
}

/*
 * name: log upload rsp
 * desc: 上报日志后，平台的返回数据。貌似不用做处理，无可变消息体。
 * direction: WTP <---- AMNMP
 * other: 
*/
int ont_wtp_log_upload_rsp(ont_device_t *dev, const char *data, size_t data_len)
{
	SL_DEBUG("check data: %s", data);

	return ONT_CMD_OK;
}

/*
 * name: log upload req
 * desc: 终端定时主动向平台上报日志。包含如下信息:
 		timestamp	时间戳
 		deviceID	设备标识
 		type		日志类型
 		module		模块
 		log			日志内容
 		可以参考SDK已有的Log机制。
 * direction: WTP ----> AMNMP
 * other: 
*/
int ont_wtp_log_upload_req(ont_device_t *dev, const char *log_pkt)
{
	int err;
	cJSON *json = NULL;
	char *jsonValue = NULL;
	char timestamp[32] = {0};
	char level[4] = {0};
	char type[32] = {0};
	char module[32] = {0};
	char log[512] = {0};

	//timestamp|level|type|module|log
	//err = sscanf(log_pkt, "%[^|]|%[^|]|%[^|]|%[^|]|%[^\0]", timestamp, level, type, module, log);
	//if (5 == err)
	//	SL_DEBUG("[%s][%s][%s][%s][%s]", timestamp, level, type, module, log);

	json = cJSON_CreateObject();
	if (!json) return ONT_JSON_ERR;

	addReqhead(json, VERSION_N, MSG_WTP_LOG_UPLOAD_REQ, UPLOAD_LOG);

	ont_platform_snprintf(timestamp, sizeof(timestamp), "%d", ( int )time(NULL));
	cJSON_AddItemToObject(json, "timestamp", cJSON_CreateString(timestamp));
	cJSON_AddItemToObject(json, "deviceID", cJSON_CreateString(g_sn));
	cJSON_AddItemToObject(json, "type", cJSON_CreateString("INFO"));
	cJSON_AddItemToObject(json, "module", cJSON_CreateString("ONT_CLOUD"));
	cJSON_AddItemToObject(json, "log", cJSON_CreateString(log_pkt));

	jsonValue = cJSON_PrintUnformatted(json);
	SL_DEBUG("%s", jsonValue);
	err = ont_edp_send_transdata(dev,SVR_NAME, jsonValue, strlen(jsonValue));
	if (ONT_ERR_OK != err)
	{
		SL_DEBUG("Failed to send transdata , error=%d", err);
		return ONT_RSP_ERR;
	}

	ont_platform_free(jsonValue);
	cJSON_Delete(json);

	return ONT_CMD_OK;
}


