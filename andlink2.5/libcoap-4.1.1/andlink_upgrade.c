/*	
 * andlink download and upgrade
 * 2017-10-26 liyue3@zowee.com.cn
 * more: 
 *
*/
#include <stdio.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

#include <wait.h>

#include "andlink.h"

#define __DEBUG
#ifdef __DEBUG
	#define SL_DEBUG(format, ...)	printf("__LINE: %d__FUNC :%s__"format"\n", __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
	#define SL_DEBUG(format, ...)
#endif

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
 * name: andlink_download_upgrade
 * desc: 使用wget下载文件并执行升级动作。
 * direction: N/A
 * other:
*/
int andlink_download_upgrade(char *url)
{
	coap_printf("");
	if (NULL == url)
	{
		SL_DEBUG("Url error!");
		return ONT_URL_ERR;
	}

	char err_msg[256] = {0};
	int file_len = 0;

	int ifd;
	struct stat sbuf;

	/* init */
	char tmp_buf[512] = {0};
	sprintf(tmp_buf, "rm -rf %s", UPLOAD_FILE);
	system(tmp_buf);
	/* -t 0: 无限次尝试下载，适用于较差的网络环境。 */
	memset(tmp_buf, 0x0, sizeof(tmp_buf));
	sprintf(tmp_buf, "wget -t 10 -c -O %s %s", UPLOAD_FILE, url);
	system(tmp_buf);

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
		coap_printf("Not a valid firmware. %s", err_msg);
		return ONT_FILE_ERR;
	}

	device_inform_data(g_deviceId, gwip, EVENT_DEVICE_FILE, FILE_TRANSFER_COMPLETE, "FILE_TRANSFER_COMPLETE");

	/*
	* write the current linux version into flash.
	*/
	write_flash_kernel_version();

	// flash write
	if(mtd_write_firmware(UPLOAD_FILE, 0, file_len) == -1)
	{
		coap_printf("mtd_write fatal error! The corrupted image has ruined the flash!!");
		return ONT_WRITE_FLASH_ERR;
	}

	coap_printf("Download and upgrade success!");
	
	return UPGRADE_SUCCESS;
}

int andlink_download(const char *url, int *file_len)
{
	if (NULL == url || NULL == file_len)
	{
		coap_printf("Url error!");
		return UNKOWN_ERROR;
	}

	char err_msg[256] = {0};
	//int file_len = 0;

	int ifd;
	struct stat sbuf;

	/* init */
	char tmp_buf[512] = {0};
	sprintf(tmp_buf, "rm -rf %s", UPLOAD_FILE);
	system(tmp_buf);
	/* -t 0: 无限次尝试下载，适用于较差的网络环境。 */
	memset(tmp_buf, 0x0, sizeof(tmp_buf));
	sprintf(tmp_buf, "wget -t 10 -c -O %s %s", UPLOAD_FILE, url);
	system(tmp_buf);

	/* Just waiting for wget. */

	ifd = open(UPLOAD_FILE, O_RDONLY);
	if(!ifd){
		sprintf (err_msg, "Can't open %s: %s\n", UPLOAD_FILE, strerror(errno));
		return UNKOWN_ERROR;
	}

	if (fstat(ifd, &sbuf) < 0) {
		close(ifd);
		sprintf (err_msg, "Can't stat %s: %s\n", UPLOAD_FILE, strerror(errno));
		return UNKOWN_ERROR;
	}

	*file_len = sbuf.st_size;

	if(!check(UPLOAD_FILE, 0, *file_len, err_msg) )
	{
		coap_printf("Not a valid firmware. %s", err_msg);
		return TRANSFER_INTERRUPT;
	}

	return TRANSFER_SUCCESS;
}

int andlink_upgrade(int file_len)
{
	/*
	* write the current linux version into flash.
	*/
	write_flash_kernel_version();

	// flash write
	if(mtd_write_firmware(UPLOAD_FILE, 0, file_len) == -1)
	{
		coap_printf("mtd_write fatal error! The corrupted image has ruined the flash!!");
		return UNKOWN_ERROR;
	}

	coap_printf("Download and upgrade success!");
	
	return UPGRADE_SUCCESS;
}



