#include "utils.h"
#include <stdlib.h>

static int set_default(void)
{
	FILE *fp;
	int i;

	//retry 15 times (15 seconds)
	for (i = 0; i < 15; i++) {
		fp = fopen("/var/run/nvramd.pid", "r");
		if (fp == NULL) {
			if (i == 0)
				fprintf(stderr, "lighttpd: waiting for nvram_daemon ");
			else
				fprintf(stdout, ". ");
		}
		else {
			fclose(fp);
#if defined (RT2860_MBSS_SUPPORT)
			int max_bss_num = 8;
			int bssidnum = strtol(nvram_get(RT2860_NVRAM, "BssidNum"), NULL, 10);
			char newBssidNum[3];

			nvram_init(RT2860_NVRAM);
#if defined (RT2860_NEW_MBSS_SUPPORT)
			max_bss_num = 16;
#endif
#ifdef CONFIG_RT2860V2_AP_MESH
			max_bss_num--;
#endif
#if defined (RT2860_APCLI_SUPPORT)
			max_bss_num--;
#endif
			if (bssidnum > max_bss_num)
				bssidnum = max_bss_num;
			sprintf(newBssidNum, "%d", bssidnum);
			nvram_set(RT2860_NVRAM, "BssidNum", newBssidNum);
#endif
			nvram_init(RT2860_NVRAM);
#if defined (CONFIG_RTDEV) || defined (CONFIG_RT2561_AP) || defined (CONFIG_RT2561_AP_MODULE)
			nvram_init(RTDEV_NVRAM);
#endif
			nvram_close(RT2860_NVRAM);

			return 0;
		}
		sleep(1);
	}
	fprintf(stderr, "goahead: please execute nvram_daemon first!");
	return (-1);
}

/*
 * len < Max length
 * return value: 1 success, 0 failed
 */
#define BUF_MAX_LEN		32
int get_popen_str(char *cmd, char *str, char *out, int len)
{
	int ret = 0;
	char buf[BUF_MAX_LEN] = {0};
	char *ptr = NULL;

	FILE *pp = popen(cmd, "r");
	if (pp)
	{
		while(fgets(buf, sizeof(buf), pp))
		{
			if (strlen(buf))
			{
				ptr = strstr(buf, str);
				if (ptr)
				{
					ptr += strlen(str);
					memcpy(out, ptr, len);
					out[len] = '\0';
					ret = 1;
				}
			}
		}

		pclose(pp);
	}

	return ret;
}

/*
 * return value: 1 success, 0 failed
 *
 */
static int init_network(void)
{
	char gwip[32] = {"8.8.8.8"};
	int ret = 0;

	if (get_popen_str("cat /etc/resolv.conf", "nameserver ", gwip, sizeof(gwip)))
	{
		//printf("GWIP: %s", gwip);
		/* 不等于默认地址，则视为从上级获取的地址 */
		if (strncmp("8.8.8.8", gwip, strlen("8.8.8.8")))
		{
			ret = 1;
		}
	}

	//printf("ret: %d", ret);

	return ret;
}

static void init_internet(void)
{
	do_system("internet.sh");
	do_system("/etc_ro/lighttpd/www/cgi-bin/wireless.cgi init");
	if (init_network())
	{
		do_system("/etc_ro/lighttpd/www/cgi-bin/firewall.cgi init");
#if defined CONFIG_RALINKAPP_SWQOS || defined CONFIG_RALINKAPP_HWQOS
		do_system("/etc_ro/lighttpd/www/cgi-bin/qos.cgi init"); 
#elif CONFIG_CUSTOM_QOS
		do_system("/sbin/qos_init.sh");
		do_system("/sbin/qos_custom.sh");
#endif
	}

	do_system("/etc_ro/lighttpd/www/cgi-bin/adm.cgi init");
	do_system("/etc_ro/lighttpd/www/cgi-bin/internet.cgi init");
#if defined CONFIG_USER_STORAGE
	do_system("/etc_ro/lighttpd/www/cgi-bin/nas.cgi init");
	do_system("/etc_ro/lighttpd/www/cgi-bin/usb.cgi init");
#endif
	do_system("/sbin/wifi_guest.sh");
}
static void usage(void)
{
	printf("Usage:\n");
	printf("\tinitial: init_system start\n");
	printf("\trestart: init_system restart\n");
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		usage();
		return -1;
	}
	if (!strcmp(argv[1], "start")) {
		if (set_default() < 0)
			return -1;
/* 	TODO:
	//signal(SIGTSTP, dhcpc_handler);
#if (defined CONFIG_USB) || (defined CONFIG_MMC)
	signal(SIGTTIN, hotPluglerHandler);
#endif
	goaInitGpio();
	signal(SIGXFSZ, WPSSingleTriggerHandler);
*/
		init_internet();
		do_system("lighttpd -f /etc_ro/lighttpd/lighttpd.conf -m /etc_ro/lighttpd/lib");
	} else if (!strcmp(argv[1], "restart")) {
/*Add factory test msg*/
		if(atoi(nvram_get(RT2860_NVRAM, "OperationMode")) == 0)
			printf("Set bridge mode success, waiting network restart\n");
		init_internet();
	} else {
		usage();
	}

	return 0;
}
