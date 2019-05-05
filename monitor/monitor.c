#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <linux/wireless.h>
#include "../../lib/libnvram/nvram.h"
#include "clientFlow.h"

#define STATICSFILE "/proc/net/dev"

long int old_tx=0;
long int old_rx=0;
int upLinkSpeed = 0;
int downLinkSpeed = 0;
int flag_ismod=0;

#define CMDLEN		256
#define IPADDRESS_FORM_DHCP  "/var/mac-ip"
#define GET_FLAG "/var/get_flow_flag"
#define LOGFILE	"/dev/console"
#define INTERAL_TIME 10

static char cmd[CMDLEN];

static void do_system(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsprintf(cmd, fmt, ap);
	va_end(ap);
	sprintf(cmd, "%s 1>%s 2>&1", cmd, LOGFILE);
	system(cmd);

	return;
}

char *get_ifname(void)
{
	char *opMode = NULL;
	static char *ifname = "br0";
	opMode = nvram_get(RT2860_NVRAM, "OperationMode");
	char *cm= nvram_bufget(RT2860_NVRAM, "wanConnectionMode");
	if (!strncmp(cm, "PPPOE", 6) || !strncmp(cm, "L2TP", 5) || !strncmp(cm, "PPTP", 5)
#ifdef CONFIG_USER_3G
			|| !strncmp(cm, "3G", 3)
#endif
		) {
		ifname = "ppp0";
	}
	else 
	{
		if (strlen(opMode))
		{
			if (!strcmp(opMode, "0")) //bridge
				ifname = "br0";
			else if (!strcmp(opMode, "1")) //gateway. do something, get UPLINK TYPE
				ifname = "eth2.2";
			else if (!strcmp(opMode, "3"))
				ifname = "apcli0";
			else
				ifname = "br0";
		}
	}

	return ifname;
}

#define STATIC_LEN 9
static void getStaticData(const char *input, char *output)
{
	int txLen = strlen(input);
	if (txLen > STATIC_LEN) {
		strncpy(output, input + (txLen - STATIC_LEN),  STATIC_LEN);
	} else {
		strncpy(output, input,  STATIC_LEN);
	}

	return;
}

static void statics(void)
{

  char buf[512]={0};
  char ifaces[24]={0};
  char rxBytes[24]={0};
  char rxPackets[24]={0};
  char rxErrs[24]={0};
  char rxDrops[24]={0};
  char rxFifo[24]={0};
  char rxFrame[24]={0};
  char rxCompressed[24]={0};
  char rxMulticast[24]={0};

  char txBytes[24]={0};
  char txPackets[24]={0};
  char txErrs[24]={0};
  char txDrops[24]={0};
  char txFifo[24]={0};
  char txFrame[24]={0};
  char txCompressed[24]={0};
  char txMulticast[24]={0};
  const char *vlanEnble, *vlanID;
  vlanEnble = nvram_bufget(RT2860_NVRAM, "VLANEnable");
  vlanID = nvram_bufget(RT2860_NVRAM, "VLANID");
  char if_name[64] = {0};
  char realTxBytes[STATIC_LEN + 1]= {0};
  char realRxBytes[STATIC_LEN + 1] = {0};

  if(0==strncmp(vlanEnble,"1",2))
  {
	 	char if_name_buff[64] = {0};
		sprintf(if_name_buff,"eth2.2.%s",vlanID);
		strcpy(if_name,if_name_buff);
  }
  else
  {
		strcpy(if_name, get_ifname());
  }

  FILE* fp=NULL;

  if(access(STATICSFILE,0)==0)
   {
    fp=fopen(STATICSFILE,"r");
	if (fp == NULL) return;

	while(fgets(buf,512,fp))
	{
	      if(strstr(buf,if_name))
          {
             sscanf(buf,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",ifaces,rxBytes,rxPackets,rxErrs,rxDrops,rxFifo,rxFrame,rxCompressed,\
                 rxMulticast,txBytes,txPackets,txErrs,txDrops,txFifo,txFrame,txCompressed,txMulticast);
          }
	}
    fclose(fp);
  }

	memset(realTxBytes, 0, sizeof(realTxBytes));
	memset(realRxBytes, 0, sizeof(realRxBytes));
	getStaticData(txBytes, realTxBytes);
	getStaticData(rxBytes, realRxBytes);
	
	upLinkSpeed = (atol(realTxBytes) - old_tx)/10; //Bps /10s
	downLinkSpeed = (atol(realRxBytes) - old_rx)/10;//Bps /10s

	old_tx = atol(realTxBytes);
	old_rx = atol(realRxBytes);

	//trim invalid data
	if (upLinkSpeed < 0 || downLinkSpeed <0) return;
	
	//Save upLinkSpeed/downLinkSpeed to /tmp/speed.txt
	sprintf(cmd,"echo %d,%d > /tmp/speed.txt",upLinkSpeed,downLinkSpeed);
	system(cmd);

	return;
}

static int internetIsOK(char *url, int datasize, int count)
{
	FILE* fp = NULL;
	char buf[128]={0};
	char cmd[256]={0};
	char txcount[10]={0};
	char rxcount[10]={0};
	char loss[10]={0};
	int flag = 0; //0: internet disconnected; 1: internet connected
	int loop = 0;

	do
	{
		loop++;
		sprintf(cmd,"ping -s %d -c %d %s> /tmp/pingtest",datasize,count,url);
//		sprintf(cmd,"ping.sh %s %d %d",url,datasize,count);
		system(cmd);

		if(access("/tmp/pingtest",0)==0)
		{
			fp = fopen("/tmp/pingtest","r");
			if(fp)
			{
				while(fgets(buf,128,fp))
				{
					if(strstr(buf,"transmitted"))
					{
					  sscanf(buf,"%s %s %s %s %s %s %s %s %s",txcount,buf,buf,rxcount,buf,buf,loss,buf,buf);

					  if(atoi(rxcount) > 0)//internet is ok
						{
							flag = 1;
							break;
						}
					}
				}
				fclose(fp);
			}
			
			system("rm /tmp/pingtest");
		}
	}while(flag == 0 && loop < 1); //Check only once

	return flag;
}

static int isforkexist(char* pidname)
{
	FILE* fp; 
	int count=-1; 
	char buf[8]; 
	char command[150]; 
	sprintf(command, "ps -ef | grep %s | grep -v grep | wc -l",pidname); 
 
	if((fp = popen(command,"r")) == NULL) 
		printf("popen error \n"); 
 
	if( (fgets(buf,8,fp))!= NULL ) 
	{
		count = atoi(buf); 
		
		if(count  == 0) 
			printf("udhcpd not found\n"); 
		else
			printf("process :tdv1 total is %d\n",count); 
	} 
	pclose(fp); 
	
	return count;

}


static void internetCheck()
{
	char  *wifionoff_24G= NULL;
	char  *wifionoff_5G= NULL;
	char* opmode=NULL;
	int flag = 0;
	
	opmode = nvram_bufget(RT2860_NVRAM, "OperationMode");
	wifionoff_24G = nvram_bufget(RT2860_NVRAM, "WiFiOff");
    wifionoff_5G = nvram_bufget(RTDEV_NVRAM, "WiFiOff");

	//Double check internet
	flag = internetIsOK("www.163.com", 56, 1);
	if (flag ==0) flag = internetIsOK("www.baidu.com", 56, 1);

	if(1 == flag)		
		system("echo 1 > /tmp/internetstatus");
	else		
		system("echo 0 > /tmp/internetstatus");

//#if defined (CONFIG_PRODUCT_331AA) || defined (CONFIG_PRODUCT_339AA)
#if 1
	if(atoi(opmode) != 1)
		{
			if(flag == 0 || network_check(flag))
				system("dhcpdiscover&");

			if(flag == 1 || network_check(flag) == 0 )
				nvram_set(RT2860_NVRAM, "dhcpserverok","0");

			if(atoi(opmode) != 1 && isforkexist("udhcpd")>=1 && flag == 1)
				system("killall udhcpd &");
		}
#endif

if(access("/tmp/led_control",0)!=0)
{
#if defined (CONFIG_PRODUCT_340AA) || defined (CONFIG_PRODUCT_341AA) \
	||defined (CONFIG_PRODUCT_340AA_JS) || defined (CONFIG_PRODUCT_341AA_JS) 
	if(flag == 1) {
			if(atoi(wifionoff_24G) == 0)
			system("gpio 2");
			else
				system("gpio a");
#if defined (CONFIG_RTDEV)
			if(atoi(wifionoff_5G) == 0)
			system("gpio 6");
			else
				system("gpio b");
#endif
	} else  {
		if(strncmp(opmode,"1",1) == 0) {
			if(atoi(wifionoff_24G) == 0)
			system("gpio 3");
			else
				system("gpio a");
#if defined (CONFIG_RTDEV)
			if(atoi(wifionoff_5G) == 0)
			system("gpio 7");
			else
				system("gpio b");
#endif
		} else {
			if(atoi(wifionoff_24G) == 0) 
				system("gpio 2");
		else
				system("gpio a");

#if defined (CONFIG_RTDEV)
			if(atoi(wifionoff_5G) == 0)
				system("gpio 6");
			else
				system("gpio b");
#endif
			}
		}
#endif
#if defined (CONFIG_PRODUCT_331AA) || defined (CONFIG_PRODUCT_339AA)\
	||defined (CONFIG_PRODUCT_331AA_JS) || defined (CONFIG_PRODUCT_339AA_JS)
		system("reg s 0xb0000060 | reg w 4 0x1&"); //初始化331 LED模式
		if(atoi(wifionoff_24G) == 0)
			system("gpio 2");
		else
			system("gpio 3");
#endif
}
}

static void portStatus(void)
{
	FILE* fp = NULL;
	char buf[256]={0};
	char cmd[256]={0};
	int port0 = 0;
	int port1 = 0;
	int port2 = 0;
	int port3 = 0;

	system("mii_mgr -g -p 0 -r 1 > /tmp/phy");
	system("mii_mgr -g -p 1 -r 1 >> /tmp/phy");
	system("mii_mgr -g -p 2 -r 1 >> /tmp/phy");
	system("mii_mgr -g -p 3 -r 1 >> /tmp/phy");

	if(access("/tmp/phy",0)==0)
		{
		fp = fopen("/tmp/phy","r");

		if(fp)
			{
			while(fgets(buf,128,fp))
				{
					if(strstr(buf,"phy[0].reg[1]"))
					{
						if(strstr(buf,"786d"))
							port0 = 1;
						else
							port0 = 0;
					}

					if(strstr(buf,"phy[1].reg[1]"))
					{
						if(strstr(buf,"786d"))
							port1 = 1;
						else
							port1 = 0;
					}

					if(strstr(buf,"phy[2].reg[1]"))
					{
						if(strstr(buf,"786d"))
							port2 = 1;
						else
							port2 = 0;
					}

					if(strstr(buf,"phy[3].reg[1]"))
					{
						if(strstr(buf,"786d"))
							port3 = 1;
						else
							port3 = 0;
					}
				}
				fclose(fp);
			}
			system("rm /tmp/phy");
		}

	sprintf(cmd,"echo %d,%d,%d,%d > /tmp/portStatus",port0,port1,port2,port3);
	system(cmd);
	
}


static void repeaterSuccess(void)
{
	char* value=NULL;
	char buf[256]={0};
	FILE* fp=NULL;
	char* ApcliEn2=NULL;
	int nConnected = 0;
	
	ApcliEn2 = nvram_bufget(RT2860_NVRAM, "ApCliEnable");
	value = nvram_bufget(RT2860_NVRAM, "OperationMode");
	printf("ApCliEnable:%s, OperationMode:%s", ApcliEn2, value);
	if(strncmp(value,"3", 1)==0)
	{
		if(strncmp(ApcliEn2,"1", 1) == 0)
		{
			system("iwconfig apcli0 > /tmp/apcli0Status");

			if(access("/tmp/apcli0Status",0) == 0)
			{
				fp = fopen("/tmp/apcli0Status","r");
				if (fp == NULL) return;

				while(fgets(buf,256,fp))
				{
					if(strstr(buf,"Access Point"))
					{
						if(!strstr(buf,"Not-Associated")) {
							nConnected = 1;
							break;
						}
					}
				}
				
				fclose(fp);
			}
		}
			
#if defined (CONFIG_RTDEV)
			char* ApcliEn5=NULL;
			ApcliEn5 = nvram_bufget(RTDEV_NVRAM, "ApCliEnable");
				
			if(strncmp(ApcliEn5,"1", 1) == 0)
			{
				system("iwconfig apclii0 > /tmp/apcli0Status");
				if(access("/tmp/apcli0Status",0) == 0)
				{
					fp = fopen("/tmp/apcli0Status","r");
					if (fp == NULL) return;
									
					memset(buf, 0, sizeof(buf));
					while(fgets(buf,256,fp))
					{
						if(strstr(buf,"Access Point"))
						{
							if(!strstr(buf,"Not-Associated")) {
								nConnected = 1;
								break;
							}
						}
					}
					
					fclose(fp);
				}	
			}
				
#endif
			if (nConnected) {
				nvram_set(RT2860_NVRAM, "apcliConnStatus","Connected");
			} else {
				nvram_set(RT2860_NVRAM, "apcliConnStatus","Disconnect");
			}
		}
}

int getPidByName(char *proc_name)
{
	char cmd_buf[128] = {0};
	char buf[64] = {0};

	int pid = -1;

	snprintf(cmd_buf, sizeof(cmd_buf), "ps -ef | grep %s | grep -v grep | grep -v ps", proc_name);

	FILE *pp = popen(cmd_buf, "r");
	if (pp)
	{
		while(fgets(buf, sizeof(buf), pp))
		{
			if (0 != strlen(buf) && strstr(buf, proc_name))
			{
				/* 3701 admin	  1720 S	ont_cloud */
				pid = atoi(buf);
				break;
			}
		}

		pclose(pp);
	}

	return pid;
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

void process_deamon(void)
{
	int opmode = 0;
//#ifdef CONFIG_USER_ONT_CLOUD
	if (-1 == getPidByName("ont_cloud"))
	{
		/* Gateway */
		if (atoi(nvram_bufget(RT2860_NVRAM, "OperationMode")) == 1)
			do_system("ont_cloud &");
	}
//#endif

//#ifdef CONFIG_USER_ANDLINK
	if (-1 == getPidByName("andlink"))
	{
		/* Bridge and repeater */
		opmode = atoi(nvram_bufget(RT2860_NVRAM, "OperationMode"));
		
		if(atoi(nvram_bufget(RT2860_NVRAM, "andlink_switch")) == 1)  // andlnk switch on
		{
			if (((opmode == 0) && init_network()) ||opmode == 3)
				{
					do_system("andlink &");
				}
/*			
			if (opmode == 0)
				{
					if(init_network() == 0 && flag_ismod == 0)
						{
							flag_ismod=1;
							system("rmmod statfilter.ko");
//							system("rmmod domain_redirect.ko");
							system("rmmod dhcpfilter.ko");
						}
					else if(init_network() == 1 && flag_ismod == 1)
						{
							flag_ismod=0;
							system("insmod /lib/modules/2.6.36\+/kernel/net/statfilter/statfilter.ko");
//							system("insmod /lib/modules/2.6.36\+/kernel/net/domain_redirect/domain_redirect.ko");
							system("insmod /lib/modules/2.6.36\+/kernel/net/dhcpfilter/dhcpfilter.ko");
						}
				}
*/
		}
	}
//#endif

	return ;
}

#define RT_OID_SYNC_RT61                            0x0D010750
#define RT_OID_WSC_QUERY_STATUS                     ((RT_OID_SYNC_RT61 + 0x01) & 0xffff)
#define RT_PRIV_IOCTL				(SIOCIWFIRSTPRIV + 0x0E)
#define SIOCDEVPRIVATE				0x8BE0
#define SIOCIWFIRSTPRIV				SIOCDEVPRIVATE

static void getWscStatus(char *interface)
{
	int socket_id;
	struct iwreq wrq;
	int data = 0;
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(wrq.ifr_name, interface);
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags = RT_OID_WSC_QUERY_STATUS;
	if( ioctl(socket_id, RT_PRIV_IOCTL, &wrq) == -1)
		printf("ioctl error");
	close(socket_id);

//	printf("status:%d\n",data);
	
	if(data == 34)
		{
			system("echo wps success > /tmp/wpsok&");
		}
}

int network_check(int flag)
{
	int ret=0,i=0;
	char gwip[36]={0};

		do
		{
		if (get_popen_str("cat /etc/resolv.conf", "nameserver ", gwip, sizeof(gwip)))
			{
				while(gwip[i] != '\n')
					{
						i++;
					}
				gwip[i]='\0';
			
				if(strncmp("8.8.8.8", gwip, strlen("8.8.8.8")))
					{// check if ping gwip is ok?
					if(flag != 1)
						{
							if(internetIsOK(gwip, 56, 1) == 0)
								ret++;
							else
								ret=0;
						}
					}
				else
					ret++;
			}
			sleep(6);
		}while(ret>=1 && ret <=2);
	return ret;
}

int main()
{
	FILE *fp = NULL;
	char cmd[64]={0};
	recFlowClients rFlowClients;
	int count = 0;
#if defined (CONFIG_PRODUCT_340AA) || defined (CONFIG_PRODUCT_341AA)
	system("gpio 1"); //turn off wps led
#endif	

//nvram_init(RT2860_NVRAM);

	memset(&rFlowClients, 0, sizeof(recFlowClients));
	//register statfilter.ko
	system("insmod /lib/modules/2.6.36\+/kernel/net/statfilter/statfilter.ko");
	system("insmod /lib/modules/2.6.36\+/kernel/net/domain_redirect/domain_redirect.ko");
	system("insmod /lib/modules/2.6.36\+/kernel/net/dhcpfilter/dhcpfilter.ko");
	//Router Mode remove dhcpfilter.ko
	char *operationMode = nvram_bufget(RT2860_NVRAM, "OperationMode");
	if (operationMode != NULL && operationMode[0] == '1') {
		system("rmmod dhcpfilter.ko");
	}

	while(1)
		{
			if(count%5 == 0)
			{
				//下挂设备流量统计
				printf("1.statisFlows \n");
				statisFlows(&rFlowClients);
			}
			
			if(count%10 == 0)
			{
				//检测中继是否成功
				printf("2.repeaterSuccess \n");
				repeaterSuccess();
				
				//获取wan eth2.2 的接口的统计
				printf("3.statics \n");
			    statics();

				//进程看门狗
				printf("4.process_deamon \n");
				process_deamon();
			}

			if(count%15 == 0) 
			{
				count=0;
				
			    //检测是否连上internet
				printf("5.internetCheck \n");
				internetCheck();

				//检测网口的物理连接
				printf("6.portStatus \n");
				portStatus();
				
				printf("7.getWscStatus \n");
				getWscStatus("ra0");
#if defined (CONFIG_RTDEV)
				getWscStatus("rai0");
#endif
			}

			sleep(1);
			count++;
		}
	
    return 0;
}
