/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************
 *
 * $Id: //WIFI_SOC/release/SDK_4_1_0_0/source/user/rt2880_app/get/get.c#1 $
 */

#include <stdio.h>             
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/autoconf.h>



void usage(char *cmd)
{
	printf("Usage for example: %s SN \n", cmd);
	printf("       %s usb \n", cmd);

}

void get_mac()
{
	FILE *fp;
	FILE *fp2;
	char str[5];
	char str2[18];
	fp = fopen("/tmp/getmac", "w+");

/////////////////// LAN  //////////////////////////

	
	system("iwpriv ra0 e2p 28 > /tmp/e2p");
	fp2 = fopen("/tmp/e2p", "r+");
	fseek(fp2, -7, SEEK_END);
	fgets(str, 5, fp2);
	fclose(fp2);
	str2[0] = str[2];
	str2[1] = str[3];
	str2[2] = ':';
	str2[3] = str[0];
	str2[4] = str[1];
	str2[5] = ':';
	
	system("iwpriv ra0 e2p 2A > /tmp/e2p");
	fp2 = fopen("/tmp/e2p", "r+");
	fseek(fp2, -7, SEEK_END);
	fgets(str, 5, fp2);	
	fclose(fp2);
	str2[6] = str[2];
	str2[7] = str[3];
	str2[8] = ':';
	str2[9] = str[0];
	str2[10] = str[1];
	str2[11] = ':';	
	
	system("iwpriv ra0 e2p 2C > /tmp/e2p");
	fp2 = fopen("/tmp/e2p", "r+");
	fseek(fp2, -7, SEEK_END);
	fgets(str, 5, fp2);	
	fclose(fp2);
	str2[12] = str[2];
	str2[13] = str[3];
	str2[14] = ':';
	str2[15] = str[0];
	str2[16] = str[1];
	str2[17] = '\0';
	fprintf(fp,"echo \"LAN MAC: %s\"\n", str2);
	
	
	/////////////////  WAN  ///////////////////////////
	/*	board hn1800 no WAN MAC
	
	system("iwpriv ra0 e2p 2E > /tmp/e2p");
	fp2 = fopen("/tmp/e2p", "r+");
	fseek(fp2, -7, SEEK_END);
	fgets(str, 5, fp2);
	fclose(fp2);
	str2[0] = str[2];
	str2[1] = str[3];
	str2[2] = ':';
	str2[3] = str[0];
	str2[4] = str[1];
	str2[5] = ':';
	
	system("iwpriv ra0 e2p 30 > /tmp/e2p");
	fp2 = fopen("/tmp/e2p", "r+");
	fseek(fp2, -7, SEEK_END);
	fgets(str, 5, fp2);	
	fclose(fp2);
	str2[6] = str[2];
	str2[7] = str[3];
	str2[8] = ':';
	str2[9] = str[0];
	str2[10] = str[1];
	str2[11] = ':';	
	
	system("iwpriv ra0 e2p 32 > /tmp/e2p");
	fp2 = fopen("/tmp/e2p", "r+");
	fseek(fp2, -7, SEEK_END);
	fgets(str, 5, fp2);	
	fclose(fp2);
	str2[12] = str[2];
	str2[13] = str[3];
	str2[14] = ':';
	str2[15] = str[0];
	str2[16] = str[1];
	str2[17] = '\0';	
	fprintf(fp,"echo \"WAN MAC: %s\"\n", str2);
	*/
//////////////////  WIFI 2.4G  //////////////////////////////	

	system("iwpriv ra0 e2p 4 > /tmp/e2p");
	fp2 = fopen("/tmp/e2p", "r+");
	fseek(fp2, -7, SEEK_END);
	fgets(str, 5, fp2);
	fclose(fp2);
	str2[0] = str[2];
	str2[1] = str[3];
	str2[2] = ':';
	str2[3] = str[0];
	str2[4] = str[1];
	str2[5] = ':';
	
	system("iwpriv ra0 e2p 6 > /tmp/e2p");
	fp2 = fopen("/tmp/e2p", "r+");
	fseek(fp2, -7, SEEK_END);
	fgets(str, 5, fp2);	
	fclose(fp2);
	str2[6] = str[2];
	str2[7] = str[3];
	str2[8] = ':';
	str2[9] = str[0];
	str2[10] = str[1];
	str2[11] = ':';	
	
	system("iwpriv ra0 e2p 8 > /tmp/e2p");
	fp2 = fopen("/tmp/e2p", "r+");
	fseek(fp2, -7, SEEK_END);
	fgets(str, 5, fp2);	
	fclose(fp2);
	str2[12] = str[2];
	str2[13] = str[3];
	str2[14] = ':';
	str2[15] = str[0];
	str2[16] = str[1];
	str2[17] = '\0';	
	fprintf(fp,"echo \"WIFI 2.4G MAC: %s\"\n", str2);
	
////////////////////  WIFI 5G  ////////////////////////////////
#if defined (CONFIG_PRODUCT_341AA)
	system("iwpriv rai0 e2p 4 > /tmp/e2p");
	fp2 = fopen("/tmp/e2p", "r+");
	fseek(fp2, -7, SEEK_END);
	fgets(str, 5, fp2);
	fclose(fp2);
	str2[0] = str[2];
	str2[1] = str[3];
	str2[2] = ':';
	str2[3] = str[0];
	str2[4] = str[1];
	str2[5] = ':';
	
	system("iwpriv rai0 e2p 6 > /tmp/e2p");
	fp2 = fopen("/tmp/e2p", "r+");
	fseek(fp2, -7, SEEK_END);
	fgets(str, 5, fp2);	
	fclose(fp2);
	str2[6] = str[2];
	str2[7] = str[3];
	str2[8] = ':';
	str2[9] = str[0];
	str2[10] = str[1];
	str2[11] = ':';	
	
	system("iwpriv rai0 e2p 8 > /tmp/e2p");
	fp2 = fopen("/tmp/e2p", "r+");
	fseek(fp2, -7, SEEK_END);
	fgets(str, 5, fp2);	
	fclose(fp2);
	str2[12] = str[2];
	str2[13] = str[3];
	str2[14] = ':';
	str2[15] = str[0];
	str2[16] = str[1];
	str2[17] = '\0';	
	fprintf(fp,"echo \"WIFI5G MAC: %s\"\n", str2);
#endif	
	fclose(fp);
	system("sh /tmp/getmac");
}
//Modified start by Brcue,20161228
#if 0
void get_sn()
{
	system("sn_get snall");
}
#else
//e2p addr [0x1e0~0x1fe]

void get_sn()
	{
		FILE *fp;
		FILE *fp2;
		char str[5];
		char tmp[5];
		char str2[18]={0},sn[32]={0},sn_tmp[32]={0};
		int val=0;
		unsigned char sbuf[100]={0};
		int addr=0x1e0,count=0;

		do
			{
			sprintf(sbuf,"iwpriv ra0 e2p %x > /tmp/e2p\n",addr);
			system(sbuf);
			fp2 = fopen("/tmp/e2p", "r+");
			fseek(fp2, -7, SEEK_END);
			fgets(str, 5, fp2);
			fclose(fp2);
			str2[count+0] = str[0];
			str2[count+1] = str[1];
			str2[count+2] = str[2];
			str2[count+3] = str[3];
	
			if(str[0] >= 'A')
				val = ((str[0] - 'A') + 10) * 16;
			else
				val = (str[0] - '0') * 16;

			if(str[1] >= 'A')
				val += ((str[1] - 'A') + 10);
			else
				val += (str[1] - '0');

			sprintf(sn_tmp,"%c",val);

			strcat(sn,sn_tmp);
			
			if(str[2] >= 'A')
				val = ((str[2] - 'A') + 10) * 16;
			else
				val = (str[2] - '0') * 16;

			if(str[3] >= 'A')
				val += ((str[3] - 'A') + 10);
			else
				val += (str[3] - '0');

			sprintf(sn_tmp,"%c",val);
			
			strcat(sn,sn_tmp);
			//printf("sn tmp:%s\n",sn);
			addr+=2;
			count+=4;
		}while(addr<=0x1fe && str[0]!='F' && str[2]!='F');

	 	printf("SN:%s\n",sn);
	}

//Modified end
#endif

#define WEBPWD_ADDR_START	0x1f0
#define WEBPWD_ADDR_END		0x1f5
void get_webpwd(void)
{
	FILE *fp;
	FILE *fp2;
	char str[5];
	char tmp[5];
	char str2[18]={0},sn[32]={0},sn_tmp[32]={0};
	int val=0;
	unsigned char sbuf[100]={0};
	int addr=WEBPWD_ADDR_START,count=0;

	do
	{
		sprintf(sbuf,"iwpriv ra0 e2p %x > /tmp/e2p\n",addr);
		system(sbuf);
		fp2 = fopen("/tmp/e2p", "r+");
		fseek(fp2, -7, SEEK_END);
		fgets(str, 5, fp2);
		fclose(fp2);
		str2[count+0] = str[0];
		str2[count+1] = str[1];
		str2[count+2] = str[2];
		str2[count+3] = str[3];

		if(str[0] >= 'A')
		val = ((str[0] - 'A') + 10) * 16;
		else
		val = (str[0] - '0') * 16;

		if(str[1] >= 'A')
		val += ((str[1] - 'A') + 10);
		else
		val += (str[1] - '0');

		sprintf(sn_tmp,"%c",val);

		strcat(sn,sn_tmp);

		if(str[2] >= 'A')
		val = ((str[2] - 'A') + 10) * 16;
		else
		val = (str[2] - '0') * 16;

		if(str[3] >= 'A')
		val += ((str[3] - 'A') + 10);
		else
		val += (str[3] - '0');

		sprintf(sn_tmp,"%c",val);

		strcat(sn,sn_tmp);
		//printf("sn tmp:%s\n",sn);
		addr+=2;
		count+=4;
	}while (addr < WEBPWD_ADDR_END && str[0]!='F' && str[2]!='F');

	/* Max length of webpwd: 5 */
	sn[5] = '\0';

	printf("webpwd: %s\n",sn);

	return ;
}

void get_wpapsk1(void)
{
	FILE *fp;
	FILE *fp2;
	char str[5];
	char tmp[5];
	char str2[18]={0},sn[32]={0},sn_tmp[32]={0};
	int val=0;
	unsigned char sbuf[100]={0};
	int addr=0x1d0,count=0;

	do
	{
		sprintf(sbuf,"iwpriv ra0 e2p %x > /tmp/e2p\n",addr);
		system(sbuf);
		fp2 = fopen("/tmp/e2p", "r+");
		fseek(fp2, -7, SEEK_END);
		fgets(str, 5, fp2);
		fclose(fp2);
		str2[count+0] = str[0];
		str2[count+1] = str[1];
		str2[count+2] = str[2];
		str2[count+3] = str[3];

		if(str[0] >= 'A')
		val = ((str[0] - 'A') + 10) * 16;
		else
		val = (str[0] - '0') * 16;

		if(str[1] >= 'A')
		val += ((str[1] - 'A') + 10);
		else
		val += (str[1] - '0');

		sprintf(sn_tmp,"%c",val);

		strcat(sn,sn_tmp);

		if(str[2] >= 'A')
		val = ((str[2] - 'A') + 10) * 16;
		else
		val = (str[2] - '0') * 16;

		if(str[3] >= 'A')
		val += ((str[3] - 'A') + 10);
		else
		val += (str[3] - '0');

		sprintf(sn_tmp,"%c",val);

		strcat(sn,sn_tmp);
		//printf("sn tmp:%s\n",sn);
		addr+=2;
		count+=4;
	}while (addr < 0x1d8 && str[0]!='F' && str[2]!='F');

	printf("WPAPSK1: %s\n",sn);

	return ;
}

void get_usb()
{
	int fd;
	fd = open("/sys/devices/platform/rt3xxx-ehci/usb1/1-1/", O_RDONLY);
	//fd = open("/dev/sda4", O_RDONLY);	
	if(fd < 0)
	{
		printf("error, no usb device found\n");
	}
	else
	{
		printf("ok, USB device is plugged\n");
	}			
	close(fd);
}

void get_config_mode()
{
	FILE *fp;
	fp = fopen("/tmp/getmode", "w+");
	//fprintf(fp, "telnetdoff=`nvram_get 2860 TelnetdOff`\n");
	fprintf(fp, "telnetdoff=`nvram get 2860 factory TelnetdOff`\n");
	fprintf(fp, "if [ \"$telnetdoff\" == \"0\" ]; then\n");
	fprintf(fp, "echo \"1-->default mode\"\n");
	fprintf(fp, "else\n");
	fprintf(fp, "echo \"0-->factory mode\"\n");
	fprintf(fp, "fi\n");
	fclose(fp);
	system("sh /tmp/getmode");

}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		usage(argv[0]);
		return 0;
	}

	else if(strcmp(argv[1], "mac") == 0)
	{
		get_mac();
	}
	else if(strcmp(argv[1], "SN") == 0)
	{
		get_sn();
	}
	else if(strcmp(argv[1], "wpapsk1") == 0)
	{
		get_wpapsk1();
	}
	else if(strcmp(argv[1], "webpwd") == 0)
	{
		get_webpwd();
	}
	else if(strcmp(argv[1], "usb") == 0)
	{
		get_usb();
	}
	else if(strcmp(argv[1], "config_mode") == 0)
	{
		get_config_mode();
	}
	else
	{
		usage(argv[0]);
	}	
			
	return 0;
}

