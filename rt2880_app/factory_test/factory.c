#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <sys/ioctl.h>

#include "ra_ioctl.h"

#include <linux/autoconf.h>

#define FILE_MACADDR "/etc/sysmac"
#define FILE_SN "/etc/syssn"
//#define FILE_SDCARD "/mnt/mmcblk0p1"
#define FILE_SDCARD "/dev/mmcblk0p1"
#define FILE_USB "../test/usb*"
#define MAC_LINE 32
#define SN_LINE 32
typedef struct cmd_node {
	char *cmd_name;
	struct cmd_node *next;
	int next_count;
	int (*call_back)(char *str);
}cmd_node_t;

int do_display_version(char *str)
{
	//printf("display version v1.0 -");
	system("cat /etc/sdk.version > /tmp/sysver");
//	system("cat /etc/openwrt_version >>/tmp/sysver");
	system("cat /proc/version >>/tmp/sysver");
	system("cat /tmp/sysver");
	//printf("\n");
	return 0;
}
int do_display_macaddr(char *str)
{
	char line[4][MAC_LINE];
	FILE *fp;
	int i;

	if(access(FILE_MACADDR, F_OK) == 0) {
		if( (fp = fopen(FILE_MACADDR, "r")) != NULL) {
			fseek(fp, 0 ,SEEK_SET);
			for (i = 0; i < 4; i++) {
				memset(&line[i], '\0', MAC_LINE);
				fgets(line[i], MAC_LINE, fp);
			}
			printf("LAN MAC:     %s", line[0]);
			printf("WAN MAC:     %s", line[1]);
			printf("WIFI2.4 MAC: %s", line[2]);
			printf("WIFI5 MAC:   %s", line[3]);
		}
		fclose(fp);
	} else {
		printf("open file %s error\n", FILE_MACADDR);
		return -1;
	}
	return 0;
}
int do_display_sn(char *str)
{
	char line[SN_LINE];
	FILE *fp;

	if(access(FILE_SN, F_OK) == 0) {
		if( (fp = fopen(FILE_SN, "r")) != NULL) {
			fseek(fp, 0 ,SEEK_SET);
			memset(&line, '\0', SN_LINE);
			fgets(line, SN_LINE, fp);
			printf("SN:%s\n", line);
		}
		fclose(fp);
	} else {
		printf("open file %s error\n", FILE_SN);
		return -1;
	}
	return 0;
}

int esw_fd;

void switch_init(void)
{
	esw_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (esw_fd < 0) {
		perror("socket");
		exit(0);
	}
}

void switch_fini(void)
{
	close(esw_fd);
}

int reg_read(int offset, int *value)
{
	struct ifreq ifr;
	esw_reg reg;

	if (value == NULL)
		return -1;
	reg.off = offset;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_REG_READ, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(0);
	}
	*value = reg.val;
	return 0;
}

int do_display_portstatus()
{
	#define bit25_link0  1 << 25
	#define bit26_link1  1 << 26
	#define bit27_link2  1 << 27
	#define bit28_link3  1 << 28

	#define bit0_speed0  1 << 0
	#define bit1_speed1  1 << 1
	#define bit2_speed2  1 << 2
	#define bit3_speed3  1 << 3
	
	int portstatus = 0;
	switch_init();

	reg_read(0x80,&portstatus);

	if(portstatus & bit25_link0)
		{
			printf("Port0: Link UP ");
			if(portstatus & bit0_speed0)
				{
					printf(" 100M\n");
				}
			else
				{
					printf(" 10M\n");
				}
		}
	else
		{
			printf("Port0: Link DOWN\n");
		}

	if(portstatus & bit26_link1)
		{
			printf("Port1: Link UP ");
			if(portstatus & bit1_speed1)
				{
					printf(" 100M\n");
				}
			else
				{
					printf(" 10M\n");
				}
		}
	else
		{
			printf("Port1: Link DOWN\n");
		}

	if(portstatus & bit27_link2)
		{
			printf("Port2: Link UP ");
			if(portstatus & bit2_speed2)
				{
					printf(" 100M\n");
				}
			else
				{
					printf(" 10M\n");
				}
		}
	else
		{
			printf("Port2: Link DOWN\n");
		}

	if(portstatus & bit28_link3)
		{
			printf("Port3: Link UP ");
			if(portstatus & bit3_speed3)
				{
					printf(" 100M\n");
				}
			else
				{
					printf(" 10M\n");
				}
		}
	else
		{
			printf("Port3: Link DOWN\n");
		}

	
//	printf("portstatus:%x\n",portstatus);

	switch_fini();
}

cmd_node_t cmd_display[] = {
	{"version", NULL, 0, do_display_version},
	{"macaddr", NULL, 0, do_display_macaddr},
	{"sn",		NULL, 0, do_display_sn},
	{"portstatus",		NULL, 0, do_display_portstatus},
};

int check_mac(char *mac)
{
	int i;
	if (strlen(mac) == 17) {
		for (i = 0; i < 17; i++) {
			if (!((i+1)%3)) {
				if (mac[i] != ':') {
					return -1;
				}
			} else {
				if (!(((mac[i] >= '0') && (mac[i] <= '9')) 
					|| ((mac[i] >= 'a') && (mac[i] <= 'f'))
					|| ((mac[i] >= 'A') && (mac[i] <= 'F')))) {
					return -1;
				}
			}
		}
	} else {
		return -1;
	}
	return 0; 
}
int set_mac(int mac_type, char *str)
{
	char line[4][MAC_LINE];
	char buf[32];
	FILE *fp;
	int i;
	if (check_mac(str) == -1) {
		printf("Input MAC format error!\nSet MAC address fail\n");
		return -1;
	}
	if (access(FILE_MACADDR, F_OK) != 0) {
		system("touch /etc/sysmac");
	}

	if( (fp = fopen(FILE_MACADDR, "r+")) != NULL) {
		fseek(fp, 0 ,SEEK_SET);
		for (i = 0; i < 4; i++) {
			memset(&line[i], '\0', MAC_LINE);
			fgets(line[i], MAC_LINE, fp);
		}
	}
	fclose(fp);
	
	if(mac_type == 0 )
		{
		sprintf(buf,"LAN:%s\n",str);
		}
	else if(mac_type == 1 )
		{
		sprintf(buf,"WAN:%s\n",str);
		}
	else if(mac_type == 2 )
		{
		sprintf(buf,"WIFI2.4G:%s\n",str);
		}
	else if(mac_type == 3 )
		{
		sprintf(buf,"WIFI5G:%s\n",str);
		}

	if((fp=fopen(FILE_MACADDR,"w"))==NULL)/*打开文件写模式*/
	{
		printf("cannot open the file %s \n", FILE_MACADDR);/*判断文件是否正常打开*/
		return -1;
	}
	
	for (i = 0; i < 4; i++) {
		fputs(line[i], fp); /*将字符串写入文件*/		
	}
	fputs(buf,fp);
	
	fclose(fp);/*关闭文件*/
	printf("set mac address success\n");
	return 0;
}

int do_set_maclan(char *str)
{
	unsigned char sbuf[100];
	
	sprintf(sbuf,"iwpriv ra0 e2p 28=%c%c%c%c\n",str[3],str[4],str[0],str[1]);
	system(sbuf);
	sprintf(sbuf,"iwpriv ra0 e2p 2A=%c%c%c%c\n",str[9],str[10],str[6],str[7]);
	system(sbuf);	
	sprintf(sbuf,"iwpriv ra0 e2p 2C=%c%c%c%c\n",str[15],str[16],str[12],str[13]);
	system(sbuf);

	return set_mac(0, str);
}
int do_set_macwan(char *str)
{
	unsigned char sbuf[100];
	
	sprintf(sbuf,"iwpriv ra0 e2p 2E=%c%c%c%c\n",str[3],str[4],str[0],str[1]);
	system(sbuf);
	sprintf(sbuf,"iwpriv ra0 e2p 30=%c%c%c%c\n",str[9],str[10],str[6],str[7]);
	system(sbuf);	
	sprintf(sbuf,"iwpriv ra0 e2p 32=%c%c%c%c\n",str[15],str[16],str[12],str[13]);
	system(sbuf);

	return set_mac(1, str);
}

//01:34:67:9a:cd:f16
int do_set_macwifi2_4(char *str)
{
	unsigned char sbuf[100];
//	sprintf(sbuf,"iwpriv ra0 e2p 4=%02hhx%02hhx\n",str[1],str[0]);	
	sprintf(sbuf,"iwpriv ra0 e2p 4=%c%c%c%c\n",str[3],str[4],str[0],str[1]);
	system(sbuf);
	sprintf(sbuf,"iwpriv ra0 e2p 6=%c%c%c%c\n",str[9],str[10],str[6],str[7]);
	system(sbuf);	
	sprintf(sbuf,"iwpriv ra0 e2p 8=%c%c%c%c\n",str[15],str[16],str[12],str[13]);
	system(sbuf);

	return set_mac(2, str);
}
int do_set_macwifi5(char *str)
{
	unsigned char sbuf[100];
	
	sprintf(sbuf,"iwpriv rai0 e2p 4=%c%c%c%c\n",str[3],str[4],str[0],str[1]);
	system(sbuf);
	sprintf(sbuf,"iwpriv rai0 e2p 6=%c%c%c%c\n",str[9],str[10],str[6],str[7]);
	system(sbuf);	
	sprintf(sbuf,"iwpriv rai0 e2p 8=%c%c%c%c\n",str[15],str[16],str[12],str[13]);
	system(sbuf);

	return set_mac(3, str);
}

//e2p addr [0x1f0~0x1f8]	[WEB登录密码最大位数为8]
#define WEBPWD_MAX_LEN		8
#define WEBPWD_ADDR_START	0x1f0
#define WEBPWD_ADDR_END		0x1f5
int do_set_webpwd(char *str)
{
	unsigned char sbuf[100];
	int loop=0;
	int addr=WEBPWD_ADDR_START;

	loop=strlen(str);
	if(loop > WEBPWD_MAX_LEN)
	{
		printf("webpwd len error:%d\n", loop);
		return -1;
	}

	char tmp[32]={0};
	char buf[64]={0};
	char tmp_asc[8]={0};
	int ascval=0, count=0;

	strncpy(tmp,str,32);
	//printf("tmp:%s\n",tmp);

	do
	{
		ascval = tmp[count];
		sprintf(tmp_asc,"%x",ascval);
		strcat(buf,tmp_asc);

		count++;

	} while(tmp[count]!='\0');

	//printf("buf:%d\n",strlen(buf)%4);
	if(strlen(buf)%4 == 1)
		strcat(buf,"FFF");//fill it
	else if(strlen(buf)%4 == 2)
		strcat(buf,"FF");
	else if(strlen(buf)%4 == 3)
		strcat(buf,"F");

	loop = strlen(buf)/4;
	count=0;
	//clean addr[0x1e0~0x1fe]

	do
	{
		sprintf(sbuf,"iwpriv ra0 e2p %x=FFFF\n",addr);
		system(sbuf);
		addr += 2;
	} while( addr <= WEBPWD_ADDR_END);

	//write sn 

	addr=WEBPWD_ADDR_START;
	do
	{
		sprintf(sbuf,"iwpriv ra0 e2p %x=%c%c%c%c\n",addr,buf[count+0],buf[count+1],buf[count+2],buf[count+3]);
		system(sbuf);
		addr += 2;
		count+=4;
		loop--;
	} while(loop && addr <= WEBPWD_ADDR_END);

	printf("set webpwd success\n");
	return 0;
}

//Modified start by Brcue, 20161228
#if 0
int do_set_sn(char *str)
{
	FILE *fp;

	if((fp=fopen(FILE_SN,"w"))==NULL)/*打开文件写模式*/
	{
		printf("cannot open the file %s \n", FILE_SN);/*判断文件是否正常打开*/
		return -1;
	}
	
	fputs(str, fp); /*将字符串写入文件*/
	
	fclose(fp);/*关闭文件*/
	printf("set SN success\n");
	return 0;
}
#else
//e2p addr [0x1e0~0x1ef]  [SN:最大位数为16]
#define SN_MAX_LEN 16
int do_set_sn(char *str)
{
	unsigned char sbuf[100];
	int loop=0;
	int addr=0x1e0;

	loop=strlen(str);
	if(loop > SN_MAX_LEN)
		{
			printf("sn len error:%d\n",loop);
			return -1;
		}
  char tmp[32]={0};
  char buf[64]={0};
  char tmp_asc[8]={0};
  char tmp_c= 0;
  int ascval=0, count=0;

  strncpy(tmp,str,32);
  //printf("tmp:%s\n",tmp);

  do
  {

    tmp_c = tmp[count];

    if( tmp_c >= 'A' && tmp_c <= 'Z')
    {
    ascval = tmp_c -'A';

    if(ascval >= 0)
        ascval += 65;
    }else if(tmp_c >= 'a' && tmp_c <= 'z')
    {
    ascval = tmp_c -'a';

    if(ascval >= 0)
        ascval += 97;

    }else if(tmp_c >= '0' && tmp_c <= '9')
    {
    ascval = tmp_c -'0';

    if(ascval >= 0)
        ascval += 48;

    }else if(tmp_c >= '_')
    {
    ascval = 95;
    }
	

    //printf("tmp[%d] ascii val is %x\n",count,ascval);

    sprintf(tmp_asc,"%x",ascval);

    strcat(buf,tmp_asc);
   // printf("buf:%s\n",buf);

    count++;

  }while(tmp[count]!='\0');

	//printf("buf:%d\n",strlen(buf)%4);
	if(strlen(buf)%4 == 1)
		strcat(buf,"FFF");//fill it
	else if(strlen(buf)%4 == 2)
		strcat(buf,"FF");
	else if(strlen(buf)%4 == 3)
		strcat(buf,"F");

	loop = strlen(buf)/4;
	count=0;
//clean addr[0x1e0~0x1fe]
	
	do
		{
		sprintf(sbuf,"iwpriv ra0 e2p %x=FFFF\n",addr);
		system(sbuf);
		addr += 2;
		}while( addr <= 0x1ef);

//write sn 

	addr=0x1e0;
	do
		{
		sprintf(sbuf,"iwpriv ra0 e2p %x=%c%c%c%c\n",addr,buf[count+0],buf[count+1],buf[count+2],buf[count+3]);
		system(sbuf);
		addr += 2;
		count+=4;
		loop--;
		}while(loop && addr <= 0x1ef);
	
	printf("set SN success\n");
	return 0;
}

#endif

int check_psk(char *psk)
{
	int i;
	if (strlen(psk) == 8) {
		for (i = 0; i < 8; i++) {
			/*0-9, a-z*/
			if (!(((psk[i] >= '0') && (psk[i] <= '9')) 
				|| ((psk[i] >= 'a') && (psk[i] <= 'z')))) {
				return -1;
			}
			/*excluded 0/o/Q, B/8, 1/l/I*/
			if ((psk[i] == '0') || (psk[i] == 'o') || (psk[i] == 'Q') ||\
				(psk[i] == 'B') || (psk[i] == '8') || \
				(psk[i] == '1') || (psk[i] == 'l') || (psk[i] == 'I')) {
				return -1;
			}
		}
	} else {
		return -1;
	}
	return 0; 
}

int do_set_psk(char *str, int start_addr)
{
	int addr=start_addr;
	unsigned char sbuf[100];
	int loop=0;
	
	//if (0 != check_psk(str))
	//{
	//	printf("psk error!\n");
	//	return -1;
	//}

	  char tmp[32]={0};
	  char buf[64]={0};
	  char tmp_asc[8]={0};
	  char tmp_c= 0;
	  int ascval=0, count=0;
	  
	  loop=strlen(str);
	  
	  strncpy(tmp,str,32);
	  //printf("tmp:%s\n",tmp);
	
	  do
	  {
	
		tmp_c = tmp[count];
	
		if( tmp_c >= 'A' && tmp_c <= 'Z')
		{
		ascval = tmp_c -'A';
	
		if(ascval >= 0)
			ascval += 65;
		}else if(tmp_c >= 'a' && tmp_c <= 'z')
		{
		ascval = tmp_c -'a';
	
		if(ascval >= 0)
			ascval += 97;
	
		}else if(tmp_c >= '0' && tmp_c <= '9')
		{
		ascval = tmp_c -'0';
	
		if(ascval >= 0)
			ascval += 48;
	
		}else if(tmp_c >= '_')
		{
		ascval = 95;
		}
		
	
		//printf("tmp[%d] ascii val is %x\n",count,ascval);
	
		sprintf(tmp_asc,"%x",ascval);
	
		strcat(buf,tmp_asc);
	   // printf("buf:%s\n",buf);
	
		count++;
	
	  }while(tmp[count]!='\0');
	
		//printf("buf:%d\n",strlen(buf)%4);
		if(strlen(buf)%4 == 1)
			strcat(buf,"FFF");//fill it
		else if(strlen(buf)%4 == 2)
			strcat(buf,"FF");
		else if(strlen(buf)%4 == 3)
			strcat(buf,"F");
	
		loop = strlen(buf)/4;
		count=0;
	//clean addr[0x1d0~0x1d7]
		
		do
			{
			sprintf(sbuf,"iwpriv ra0 e2p %x=FFFF\n",addr);
			system(sbuf);
			addr += 2;
			}while( addr <= (start_addr+7) );
	
	//write psk 
	
		addr=start_addr;
		do
			{
			sprintf(sbuf,"iwpriv ra0 e2p %x=%c%c%c%c\n",addr,buf[count+0],buf[count+1],buf[count+2],buf[count+3]);
			system(sbuf);
			addr += 2;
			count+=4;
			loop--;
			}while(loop && addr <= (start_addr+7));

	return 0;
}

//e2p addr [0x1d0~0x1d7]
int do_set_psk1(char *str)
{
	int flag = -1;

	flag = do_set_psk(str, 0x1d0);

	if(flag == 0)
		{
		printf("set psk1 success\n");
	}
	else
		printf("set psk1 fail\n");
	
	return flag;
}

//e2p addr [0x1d8~0x1df]
int do_set_psk2(char *str)
{
	int flag = -1;

	flag = do_set_psk(str, 0x1d8);

	if(flag == 0)
		{
		printf("set psk2 success\n");
	}
	else
		printf("set psk2 fail\n");
	
	return flag;
}

//Modified end
cmd_node_t cmd_set[] = {
	{"maclan", NULL, 0, do_set_maclan},
	{"macwan", NULL, 0, do_set_macwan},
	{"macwifi2.4", NULL, 0, do_set_macwifi2_4},
	{"macwifi5", NULL, 0, do_set_macwifi5},
	{"sn", 		NULL, 0, do_set_sn},
	{"psk1", 	NULL, 0, do_set_psk1},
	{"psk2", 	NULL, 0, do_set_psk2},
	{"webpwd",	NULL, 0, do_set_webpwd},
};

int do_led_recover(char *str)
{
	system("rm /tmp/led_control");
#if defined (CONFIG_PRODUCT_340AA) || defined (CONFIG_PRODUCT_341AA) \
	||defined (CONFIG_PRODUCT_340AA_JS) || defined (CONFIG_PRODUCT_341AA_JS)
	system("gpio 4 | gpio 2 |gpio 6 &");
	if(access("/tmp/wpsok",0)==0)
		{
			system("gpio 0&");
		}
#elif defined (CONFIG_PRODUCT_339AA) || defined (CONFIG_PRODUCT_339AA_JS)
	if(access("/tmp/wpsok",0)==0)
		{
			system("gpio 0");
		}
	system("reg s 0xb0000060 | reg w 4 0x0 &"); //set wifi led gpio mode	
	system("gpio 4&");
#elif defined (CONFIG_PRODUCT_331AA) || defined (CONFIG_PRODUCT_AN1303) || defined (CONFIG_PRODUCT_331AA_JS)
	system("reg s 0xb0000060&"); //set wifi led gpio mode	
	system("reg w 4 0x0 &"); //set wifi led gpio mode	
	system("gpio 4 &"); //set wifi led gpio mode	
#if defined (CONFIG_PRODUCT_AN1303)
	system("gpio 0 &"); // set power led mode
#endif 
#endif

	printf("All led recover\n");
	return 0;
}


int do_led_on(char *str)
{
	system("touch /tmp/led_control");
#if defined (CONFIG_PRODUCT_340AA) || defined (CONFIG_PRODUCT_341AA)
	system("gpio 0 | gpio 8 | gpio 4 ");
#elif defined (CONFIG_PRODUCT_339AA) || defined (CONFIG_PRODUCT_339AA_JS)
	system("reg s 0xb0000060&"); //set wifi led gpio mode	
	system("reg w 4 0x1 &"); //set wifi led gpio mode	
	system("gpio 0 | gpio 4 | gpio 2&");
#elif defined (CONFIG_PRODUCT_331AA) || defined (CONFIG_PRODUCT_AN1303)
	system("gpio 2 &"); //set wifi led gpio mode	
	system("gpio 4 &"); //set power led gpio mode	
	system("gpio 0 &"); //set eth led gpio mode	
	system("reg s 0xb0000060 | reg w 4 0x11&"); //set wifi led gpio mode	
#endif

	printf("All led on\n");
	return 0;
}


int do_led_off(char *str)
{
	system("touch /tmp/led_control");
#if defined (CONFIG_PRODUCT_340AA) || defined (CONFIG_PRODUCT_341AA)
		system("gpio 1 | gpio 9 | gpio 5");
#elif defined (CONFIG_PRODUCT_339AA) || defined (CONFIG_PRODUCT_339AA_JS)
		 system("reg s 0xb0000060&"); //set wifi led gpio mode	 
		 system("reg w 4 0x1 &"); //set wifi led gpio mode	 
		 system("gpio 1 | gpio 5 | gpio 3&");
#elif defined (CONFIG_PRODUCT_331AA) || defined (CONFIG_PRODUCT_AN1303)
	system("gpio 3 &"); //set wifi led gpio mode	
	system("gpio 5 &"); //set power led gpio mode	
	system("reg s 0xb0000060 | reg w 4 0x11&"); //set wifi led gpio mode	
	system("gpio 1 &"); //set eth led gpio mode		
#if defined (CONFIG_PRODUCT_AN1303)
	system("reg w 4 0x101 &"); 
#endif
	system("gpio 1 &"); //set eth led gpio mode		
	system("gpio 3 &"); //set wifi led gpio mode	
	system("gpio 5 &"); //set power led gpio mode	
#endif

	 printf("All led off\n");
	 return 0;
}


cmd_node_t cmd_led[] = {
	{"on", 	NULL, 0, do_led_on},
	{"off", NULL, 0, do_led_off},
	{"recover",NULL,0,do_led_recover},
};

int do_key_on(char *str)
	{
		system("killall nvram_daemon");
		system("killall monitor");
		printf("GPIO test mode on\n");
		return 0;
	}


int do_key_off(char *str)
	{
		system("gpio t&");
		printf("GPIO test mode off\n");
		return 0;
	}


cmd_node_t cmd_key[] = {
	{"on", NULL, 0, do_key_on},
	{"off", NULL, 0, do_key_off},
};

int do_usb_test(char *str)
{
	system("find /dev -name sd* -maxdepth 1 > /tmp/usb");

	char line[128];
	unsigned char flg_find = 0;
	FILE *fp;
	if(access("/tmp/usb", F_OK) == 0) {
		if( (fp = fopen("/tmp/usb", "r")) != NULL) {

		fseek(fp, 0 ,SEEK_SET);
		memset(&line, '\0', 128);

			while(fgets(line, 128, fp) != NULL) {
				if (strstr(line, "sd")) {
					printf("USB detect success\n");
					flg_find = 1;
					break;
				}
			}
			
		}
		fclose(fp);
	}
	if (!flg_find) {
		printf("USB detect fail, please insert the disk\n");
	}
	return 0; 
}
cmd_node_t cmd_usb[] = {
	{"test", NULL, 0, do_usb_test},
};

int do_tf_test(char *str)
{
	if(access(FILE_SDCARD, F_OK) == 0) {
		printf("SD card detect success\n");
	} else {
		printf("SD card detect fail, plesse insert the SD card\n");
	}
	return 0;

}

cmd_node_t cmd_tf[] = {
	{"test", NULL, 0, do_tf_test},
};

int do_restore_default(char *str)
{
	system("/bin/ralink_init clear 2860");
	system("/bin/nvram_set factorydefault 1");
	system("nvram_set telnetd_flag 0");

#if defined CONFIG_LAN_WAN_SUPPORT || defined CONFIG_MAC_TO_MAC_MODE
	system("/bin/ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan");
#elif defined CONFIG_ICPLUS_PHY
	system("/bin/ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_oneport");
#else
	system("/bin/ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_novlan");
#endif
#if defined CONFIG_RTDEV
	system("/bin/ralink_init clear rtdev");
	system("/bin/ralink_init renew rtdev /etc_ro/Wireless/iNIC/RT2860AP.dat");
#endif
#if defined CONFIG_MTK_VOIP
	system("/bin/ralink_init clear voip");
	system("/bin/ralink_init renew voip /etc_ro/voip_default_settings");
#endif

	printf("restore default success\n");
	system("/sbin/reboot");
	
	return 0;
}

int do_restore_default_only(char *str)
{
	system("/bin/ralink_init clear 2860");
	system("/bin/nvram_set factorydefault 1");
//	system("nvram_set telnetd_flag 0");

#if defined CONFIG_LAN_WAN_SUPPORT || defined CONFIG_MAC_TO_MAC_MODE
	system("/bin/ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan");
#elif defined CONFIG_ICPLUS_PHY
	system("/bin/ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_oneport");
#else
	system("/bin/ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_novlan");
#endif
#if defined CONFIG_RTDEV
	system("/bin/ralink_init clear rtdev");
	system("/bin/ralink_init renew rtdev /etc_ro/Wireless/iNIC/RT2860AP.dat");
#endif
#if defined CONFIG_MTK_VOIP
	system("/bin/ralink_init clear voip");
	system("/bin/ralink_init renew voip /etc_ro/voip_default_settings");
#endif

	printf("restore default success\n");
	system("/sbin/reboot");
	
	return 0;
}


cmd_node_t cmd_restore[] = {
	{"default", NULL, 0, do_restore_default},
	{"default_only", NULL, 0, do_restore_default_only},//It will not disable telnetd
};

cmd_node_t cmds[] = {
	{"display", 	(struct cmd_node *)&cmd_display, 4, NULL},
	{"set", 		(struct cmd_node *)&cmd_set, 8, NULL},
	{"led", 		(struct cmd_node *)&cmd_led, 3, NULL},
	{"key", 		(struct cmd_node *)&cmd_key, 2, NULL},
	{"usb", 		(struct cmd_node *)&cmd_usb, 1, NULL},
	{"tf", 			(struct cmd_node *)&cmd_tf, 1, NULL},
	{"restore", 	(struct cmd_node *)&cmd_restore, 2, NULL},
};

int print_usage()
{
	cmd_node_t *p_cmd;
	char *p_cmd_name;
	int level = 0;
	int ctr = 0;
	int len = 0;
	int cmd1_ctr = sizeof(cmds)/sizeof(cmd_node_t);
	printf("command format:\n");
	for (level = 0; level < cmd1_ctr; level++) {
		p_cmd = &cmds[level];
		len = p_cmd->next_count;
		p_cmd_name = p_cmd->cmd_name;
		p_cmd = p_cmd->next;
		for (ctr = 0; ctr < len; ctr++) {
			printf("    %s  %s\n", p_cmd_name, (p_cmd+ctr)->cmd_name);
		}
	}
	return 0; 
}

int parse_cmd(int argc, char **argv)
{
	int ctr = 0;
	unsigned char flg_find = 0;
	int cmd1_ctr = sizeof(cmds)/sizeof(cmd_node_t);
	int len;
	int ret = -1;
	
	for (ctr = 0; ctr < cmd1_ctr; ctr++) {
		//printf("compare %s and %s\n", argv[1], cmds[ctr].cmd_name);
		if (!strcmp(argv[1], cmds[ctr].cmd_name)) {
			flg_find = 1;
			break;
		}
	}
	//printf("flg_find = %d, ctr = %d\n", flg_find, ctr);
	if (flg_find) {
		flg_find = 0;
		len = cmds[ctr].next_count;
		cmd_node_t *p_cmd = (cmd_node_t*)cmds[ctr].next;
		for (ctr = 0; ctr < len; ctr++) {
			//printf("compare %s and %s\n", argv[2], (p_cmd+ctr)->cmd_name);
			if (!strcmp(argv[2], (p_cmd+ctr)->cmd_name)) {
				flg_find = 1;
				if ((p_cmd+ctr)->call_back) {
					if (argc == 4) {
						ret = (p_cmd+ctr)->call_back(argv[3]);
					} else {
						char *p_void;
						ret = (p_cmd+ctr)->call_back(p_void);
					}
				}
				break; 
			}
		}
	}
	if (!flg_find) {
		printf("input command is invalid\n");
		print_usage();
	}
	return 0; 
}
int main(int argc, char **argv)
{
	if (argc < 2) {
		print_usage();
	} else {
		parse_cmd(argc, argv); 
	}
	return 0; 
}
