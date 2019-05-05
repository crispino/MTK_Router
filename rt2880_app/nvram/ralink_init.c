#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include "nvram.h"

#include <linux/autoconf.h>

#define DEFAULT_FLASH_ZONE_NAME "2860"

int set_usage(char *aout)
{
#ifndef CONFIG_CC_OPTIMIZE_FOR_SIZE
	int i, num;

	printf("Usage example: \n");
	for (i = 0; i < getNvramNum(); i++){
		printf("\t%s %s ", aout, getNvramName(i));
		printf("lan_ipaddr 1.2.3.4\n");
	}
#endif
	return -1;
}

int ra_nv_set(int argc,char **argv)
{
	int index, rc;
	char *fz, *key, *value;

	if (argc == 1 || argc > 5)
		return set_usage(argv[0]);

	if (argc == 2) {
		fz = DEFAULT_FLASH_ZONE_NAME;
		key = argv[1];
		value = "";
	} else if (argc == 3) {
		fz = DEFAULT_FLASH_ZONE_NAME;
		key = argv[1];
		value = argv[2];
	} else {
		fz = argv[1];
		key = argv[2];
		value = argv[3];
	}

	if ((index = getNvramIndex(fz)) == -1) {
		printf("%s: Error: \"%s\" flash zone not existed\n", argv[0], fz);
		return set_usage(argv[0]);
	}

	nvram_init(index);
	rc = nvram_set(index, key, value);
	nvram_close(index);
	return rc;
}

int get_usage(char *aout)
{
#ifndef CONFIG_CC_OPTIMIZE_FOR_SIZE
	int i;

	printf("Usage: \n");
	for (i = 0; i < getNvramNum(); i++){
		printf("\t%s %s ", aout, getNvramName(i));
		printf("lan_ipaddr\n");
	}
#endif
	return -1;
}

int ra_nv_get(int argc, char *argv[])
{
	char *fz;
	char *key;
	char *value;

	int index;

	if (argc != 3 && argc != 2)
		return get_usage(argv[0]);

	if (argc == 2) {
		fz = DEFAULT_FLASH_ZONE_NAME;
		key = argv[1];
	} else {
		fz = argv[1];
		key = argv[2];
	}

	if ((index = getNvramIndex(fz)) == -1) {
		printf("%s: Error: \"%s\" flash zone not existed\n", argv[0], fz);
		return get_usage(argv[0]);
	}

	nvram_init(index);
	printf("%s\n", nvram_bufget(index, key));
	nvram_close(index);
	return 0;
}

void usage(char *cmd)
{
#ifndef CONFIG_CC_OPTIMIZE_FOR_SIZE
	printf("Usage:\n");
	printf("  %s <command> [<platform>] [<file>]\n\n", cmd);
	printf("command:\n");
	printf("  rt2860_nvram_show - display rt2860 values in nvram\n");
#if defined (CONFIG_RTDEV) || \
	defined (CONFIG_RT2561_AP) || defined (CONFIG_RT2561_AP_MODULE) || \
	defined (CONFIG_RTDEV_PLC)
	printf("  rtdev_nvram_show   - display 2nd ralink device values in nvram\n");
#endif
#ifdef CONFIG_DUAL_IMAGE
	printf("  uboot_nvram_show - display uboot parameter values\n");
#endif
#if defined CONFIG_EXTEND_NVRAM
	printf("  config2_nvram_show   - display configuration in 2nd nvram\n");
#endif
	printf("  show    - display values in nvram for <platform>\n");
	printf("  gen     - generate config file from nvram for <platform>\n");
	printf("  renew   - replace nvram values for <platform> with <file>\n");
	printf("  clear	  - clear all entries in nvram for <platform>\n");
	printf("platform:\n");
	printf("  2860    - rt2860\n");
#if defined (CONFIG_RTDEV) || \
	defined (CONFIG_RT2561_AP) || defined (CONFIG_RT2561_AP_MODULE) || \
	defined (CONFIG_RTDEV_PLC)
	printf("  rtdev    - 2nd ralink device\n");
#endif
#ifdef CONFIG_DUAL_IMAGE
	printf("  uboot    - uboot parameter\n");
#endif
#if defined CONFIG_EXTEND_NVRAM
	printf("  config2    - 2nd nvram\n");
#endif
	printf("file:\n");
	printf("          - file name for renew command\n");
#endif
	exit(0);
}

int main(int argc, char *argv[])
{
	char *cmd;

	if (argc < 2)
		usage(argv[0]);

	//call nvram_get or nvram_set
	if ((cmd = strrchr(argv[0], '/')) != NULL)
		cmd++;
	else
		cmd = argv[0];
	if (!strncmp(cmd, "nvram_get", 10))
		return ra_nv_get(argc, argv);
	else if (!strncmp(cmd, "nvram_set", 10))
		return ra_nv_set(argc, argv);

	if (argc == 2) {
		if (!strncmp(argv[1], "rt2860_nvram_show", 18))
			nvram_show(RT2860_NVRAM);
#if defined (CONFIG_RTDEV) || \
	defined (CONFIG_RT2561_AP) || defined (CONFIG_RT2561_AP_MODULE) || \
	defined (CONFIG_RTDEV_PLC)
		else if (!strncmp(argv[1], "rtdev_nvram_show", 17))
			nvram_show(RTDEV_NVRAM);
#endif
#ifdef CONFIG_DUAL_IMAGE
		else if (!strncmp(argv[1], "uboot_nvram_show", 17))
			nvram_show(UBOOT_NVRAM);
#endif
#if 1 //def CONFIG_RT2860V2_STA_WPA_SUPPLICANT
		else if (!strncmp(argv[1], "cert_nvram_show", 16))
			nvram_show(CERT_NVRAM);
#endif

#ifdef WAPI_SUPPORT
		else if (!strncmp(argv[1], "wapi_nvram_show", 16))
			nvram_show(WAPI_NVRAM);
#endif
#if defined CONFIG_EXTEND_NVRAM
		else if (!strncmp(argv[1], "config2_nvram_show", 19))
			nvram_show(CONFIG2_NVRAM);
		else if (!strncmp(argv[1], "tr069cert_nvram_show", 21))
			nvram_show(TR069CERT_NVRAM);
#endif
		else
			usage(argv[0]);
	} else if (argc == 3) {
		/* TODO: <cmd> gen 2860ap */
		if (!strncasecmp(argv[1], "gen", 4) ||
		    !strncasecmp(argv[1], "make_wireless_config", 21)) {
			if (!strncmp(argv[2], "2860", 5) ||
			    !strncasecmp(argv[2], "rt2860", 7)) //b-compatible
				gen_config(RT2860_NVRAM);
#if defined (CONFIG_RTDEV) || \
	defined (CONFIG_RT2561_AP) || defined (CONFIG_RT2561_AP_MODULE) || \
	defined (CONFIG_RTDEV_PLC)
			else if (!strncasecmp(argv[2], "rtdev", 6))
				gen_config(RTDEV_NVRAM);
#endif
#ifdef CONFIG_DUAL_IMAGE
			else if (!strncasecmp(argv[2], "uboot", 6))
				printf("No support of gen command of uboot parameter.\n");
#endif
#ifdef CONFIG_RT2860V2_STA_WPA_SUPPLICANT
			else if (!strncmp(argv[2], "cert", 5))
				gen_cert(CERT_NVRAM);
#endif


#ifdef WAPI_SUPPORT
			else if (!strncmp(argv[2], "wapi", 5))
				gen_cert(WAPI_NVRAM);
#endif
#if defined CONFIG_EXTEND_NVRAM
      else if (!strncmp(argv[2], "tr069_ca_cert", 14))
				gen_cert_tr069("tr069_ca");
		  else if (!strncmp(argv[2], "tr069_cc_cert", 14))
				gen_cert_tr069("tr069_cc");
	#endif
			else
				usage(argv[0]);
		} else if (!strncasecmp(argv[1], "show", 5)) {
			if (!strncmp(argv[2], "2860", 5) ||
			    !strncasecmp(argv[2], "rt2860", 7)) //b-compatible
				nvram_show(RT2860_NVRAM);
#if defined (CONFIG_RTDEV) || \
	defined (CONFIG_RT2561_AP) || defined (CONFIG_RT2561_AP_MODULE) || \
	defined (CONFIG_RTDEV_PLC)
			else if (!strncasecmp(argv[2], "rtdev", 6))
				nvram_show(RTDEV_NVRAM);
#endif
#ifdef CONFIG_DUAL_IMAGE
			else if (!strncasecmp(argv[2], "uboot", 6))
				nvram_show(UBOOT_NVRAM);
#endif
#ifdef CONFIG_RT2860V2_STA_WPA_SUPPLICANT
			else if (!strncasecmp(argv[2], "cert", 5))
				nvram_show(CERT_NVRAM);
#endif
#ifdef WAPI_SUPPORT
			else if (!strncasecmp(argv[2], "wapi", 5))
				nvram_show(WAPI_NVRAM);
#endif
#if defined CONFIG_EXTEND_NVRAM
			else if (!strncasecmp(argv[2], "config2", 8))
				nvram_show(CONFIG2_NVRAM);
			else if (!strncasecmp(argv[2], "tr069cert", 10))
				nvram_show(TR069CERT_NVRAM);
#endif
			else
				usage(argv[0]);
		} else if(!strncasecmp(argv[1], "clear", 6)) {
			if (!strncmp(argv[2], "2860", 5) || 
			    !strncasecmp(argv[2], "rt2860", 7)) //b-compatible
				nvram_clear(RT2860_NVRAM);
#if defined (CONFIG_RTDEV) || \
	defined (CONFIG_RT2561_AP) || defined (CONFIG_RT2561_AP_MODULE) || \
	defined (CONFIG_RTDEV_PLC)
			else if (!strncasecmp(argv[2], "rtdev", 6))
				nvram_clear(RTDEV_NVRAM);
#endif
#ifdef CONFIG_DUAL_IMAGE
			else if (!strncasecmp(argv[2], "uboot", 6))
				nvram_clear(UBOOT_NVRAM);
#endif
#ifdef CONFIG_RT2860V2_STA_WPA_SUPPLICANT
			else if (!strncasecmp(argv[2], "cert", 5))
				nvram_clear(CERT_NVRAM);
#endif
#ifdef WAPI_SUPPORT
			else if (!strncasecmp(argv[2], "wapi", 5))
				nvram_clear(WAPI_NVRAM);
#endif
#if defined CONFIG_EXTEND_NVRAM
			else if (!strncasecmp(argv[2], "config2", 8))
				nvram_clear(CONFIG2_NVRAM);
			else if (!strncasecmp(argv[2], "tr069cert", 10))
				nvram_clear(TR069CERT_NVRAM);
#endif
			else
				usage(argv[0]);
		} else
			usage(argv[0]);
	} else if (argc == 4) {
		if (!strncasecmp(argv[1], "renew", 6)) {
			if (!strncmp(argv[2], "2860", 5) ||
			    !strncasecmp(argv[2], "rt2860", 7)) //b-compatible
				renew_nvram(RT2860_NVRAM, argv[3]);
#if defined (CONFIG_RTDEV) || \
	defined (CONFIG_RT2561_AP) || defined (CONFIG_RT2561_AP_MODULE) || \
	defined (CONFIG_RTDEV_PLC)
			else if (!strncasecmp(argv[2], "rtdev", 6))
				renew_nvram(RTDEV_NVRAM, argv[3]);
#endif
#ifdef CONFIG_DUAL_IMAGE
			else if (!strncasecmp(argv[2], "uboot", 6))
				printf("No support of renew command of uboot parameter.\n");
#endif
#if defined CONFIG_EXTEND_NVRAM
			else if (!strncasecmp(argv[2], "config2", 8))
				renew_nvram(CONFIG2_NVRAM, argv[3]);
#endif
		} else
			usage(argv[0]);
	} else
		usage(argv[0]);
	return 0;
}

int gen_cert(int mode)
{
	char cmd[4120];
	const char *file_name;
	
	nvram_init(mode);
#ifdef CONFIG_RT2860V2_STA_WPA_SUPPLICANT
	if (mode == CERT_NVRAM) {
		file_name = nvram_bufget(CERT_NVRAM, "CACLCertFile");
		if (strlen(file_name) > 0)
		{
			memset(cmd, 0, 4120);
			sprintf(cmd, "nvram_get cert CACLCert > %s", file_name);
			system(cmd);
		}
		file_name = nvram_bufget(CERT_NVRAM, "KeyCertFile");
		if (strlen(file_name) > 0)
		{
			memset(cmd, 0, 4120);
			sprintf(cmd, "nvram_get cert KeyCert > %s", file_name);
			system(cmd);
		}
	}
#endif
#ifdef WAPI_SUPPORT
	if (mode == WAPI_NVRAM) {
		file_name = nvram_bufget(WAPI_NVRAM, "ASCertFile");
		if (strlen(file_name) > 0)
		{
			memset(cmd, 0, 4120);
			sprintf(cmd, "nvram_get wapi ASCert > %s", file_name);
			system(cmd);
		}
		file_name = nvram_bufget(WAPI_NVRAM, "UserCertFile");
		if (strlen(file_name) > 0)
		{
			memset(cmd, 0, 4096);
			sprintf(cmd, "nvram_get wapi UserCert > %s", file_name);
			system(cmd);
		}
	}
#endif
}
#if defined CONFIG_EXTEND_NVRAM
int gen_cert_tr069(char *cert_type)
{
	char cmd[4120];
	const char *file_name;
	
   nvram_init(RT2860_NVRAM);
   if(strcmp(cert_type, "tr069_ca")==0){
		file_name = nvram_bufget(CONFIG2_NVRAM, "TR_CA_CERT_FILE");
		if (strlen(file_name) > 0)
		{
			memset(cmd, 0, 2048);
			sprintf(cmd, "nvram_get tr069cert TR069_CA > %s", file_name);
			system(cmd);
		}
	}else {
		
		file_name = nvram_bufget(CONFIG2_NVRAM, "TR_CLIENT_CERT_FILE");
		if (strlen(file_name) > 0)
		{
			memset(cmd, 0, 4096);
			sprintf(cmd, "nvram_get tr069cert TR069_CC > %s", file_name);
			system(cmd);
		}
	}
}
#endif
int gen_config(int mode)
{
	FILE *fp = NULL;
	int  i, ssid_num = 1;
	char tx_rate[32], wmm_enable[32];

	nvram_init(mode);

	/*
	nvram_bufset(mode, "SystemName", "RalinkAP");
	nvram_bufset(mode, "ModuleName", "RT2860");
	nvram_commit(mode);
	*/

	system("mkdir -p /etc/Wireless/RT2860");
	if (mode == RT2860_NVRAM) {
		fp = fopen("/etc/Wireless/RT2860/RT2860.dat", "w+");
	} else if (mode == RTDEV_NVRAM) {
#if defined (CONFIG_RTDEV)
		system("mkdir -p /etc/Wireless/iNIC");
		fp = fopen("/etc/Wireless/iNIC/iNIC_ap.dat", "w+");
#elif defined (CONFIG_RT2561_AP) || defined (CONFIG_RT2561_AP_MODULE)
		system("mkdir -p /etc/Wireless/RT2561");
		fp = fopen("/etc/Wireless/RT2561/RT2561.dat", "w+");
#elif defined (CONFIG_RTDEV_PLC)
		system("mkdir -p /etc/PLC");
		fp = fopen("/etc/PLC/plc.dat", "w+");
#endif
	} else
		return 0;
	if (NULL == fp){
		printf(" fopen failed.\n");
		return 0;
	}
	fprintf(fp, "#The word of \"Default\" must not be removed\n");
	fprintf(fp, "Default\n");

#define FPRINT_NUM(x) fprintf(fp, #x"=%d\n", atoi(nvram_bufget(mode, #x)));
#define FPRINT_STR(x) fprintf(fp, #x"=%s\n", nvram_bufget(mode, #x));

	if ((RT2860_NVRAM == mode) 
#if defined (CONFIG_RTDEV)
		|| (RTDEV_NVRAM == mode)
#endif
		) {
		FPRINT_NUM(CountryRegion);
		FPRINT_NUM(CountryRegionABand);
		FPRINT_STR(CountryCode);
		FPRINT_NUM(BssidNum);
		ssid_num = atoi(nvram_get(mode, "BssidNum"));
		if (ssid_num > 16)
			ssid_num = 16;
		else if (ssid_num < 0)
			ssid_num = 0;

		FPRINT_STR(SSID1);
		FPRINT_STR(SSID2);
		FPRINT_STR(SSID3);
		FPRINT_STR(SSID4);
		FPRINT_STR(SSID5);
		FPRINT_STR(SSID6);
		FPRINT_STR(SSID7);
		FPRINT_STR(SSID8);
#if defined (CONFIG_16MBSSID_MODE)
		FPRINT_STR(SSID9);
		FPRINT_STR(SSID10);
		FPRINT_STR(SSID11);
		FPRINT_STR(SSID12);
		FPRINT_STR(SSID13);
		FPRINT_STR(SSID14);
		FPRINT_STR(SSID15);
		FPRINT_STR(SSID16);
#endif

		FPRINT_NUM(WirelessMode);
		FPRINT_STR(FixedTxMode);

		//TxRate(FixedRate)
		bzero(tx_rate, sizeof(tx_rate));
		for (i = 0; i < ssid_num; i++)
		{
			snprintf(tx_rate+strlen(tx_rate), 1, "%d",
					atoi(nvram_bufget(mode, "TxRate")));
			snprintf(tx_rate+strlen(tx_rate), 1, "%c", ';');
		}
		//tx_rate[strlen(tx_rate) - 1] = '\0';
		fprintf(fp, "TxRate=%s\n", tx_rate);

		FPRINT_NUM(Channel);
		FPRINT_NUM(BasicRate);
		FPRINT_NUM(BeaconPeriod);
		FPRINT_NUM(DtimPeriod);
		FPRINT_NUM(TxPower);
		FPRINT_NUM(DisableOLBC);
		FPRINT_NUM(BGProtection);
		fprintf(fp, "TxAntenna=\n");
		fprintf(fp, "RxAntenna=\n");
		FPRINT_NUM(TxPreamble);
		FPRINT_NUM(RTSThreshold  );
		FPRINT_NUM(FragThreshold  );
		FPRINT_NUM(TxBurst);
		FPRINT_NUM(PktAggregate);
		FPRINT_NUM(AutoProvisionEn);
		FPRINT_NUM(FreqDelta);
		fprintf(fp, "TurboRate=0\n");

#if defined (CONFIG_RT2860V2_AP_VIDEO_TURBINE) || defined (CONFIG_RT2860V2_STA_VIDEO_TURBINE)
		FPRINT_NUM(VideoTurbine);
#endif

		//WmmCapable
		bzero(wmm_enable, sizeof(wmm_enable));
		for (i = 0; i < ssid_num; i++)
		{
			snprintf(wmm_enable+strlen(wmm_enable), 1, "%d",
					atoi(nvram_bufget(mode, "WmmCapable")));
			snprintf(wmm_enable+strlen(wmm_enable), 1, "%c", ';');
		}
		//wmm_enable[strlen(wmm_enable) - 1] = '\0';
		fprintf(fp, "WmmCapable=%s\n", wmm_enable);

		FPRINT_STR(APAifsn);
		FPRINT_STR(APCwmin);
		FPRINT_STR(APCwmax);
		FPRINT_STR(APTxop);
		FPRINT_STR(APACM);
		FPRINT_STR(BSSAifsn);
		FPRINT_STR(BSSCwmin);
		FPRINT_STR(BSSCwmax);
		FPRINT_STR(BSSTxop);
		FPRINT_STR(BSSACM);
		FPRINT_STR(AckPolicy);
		FPRINT_STR(APSDCapable);
		FPRINT_STR(DLSCapable);
		FPRINT_STR(NoForwarding);
		FPRINT_NUM(NoForwardingBTNBSSID);
		FPRINT_STR(HideSSID);
		FPRINT_NUM(ShortSlot);
		FPRINT_NUM(AutoChannelSelect);

		FPRINT_STR(IEEE8021X);
		FPRINT_NUM(IEEE80211H);
		FPRINT_NUM(CarrierDetect);

		FPRINT_NUM(ITxBfEn);
		FPRINT_STR(PreAntSwitch);
		FPRINT_NUM(PhyRateLimit);
		FPRINT_NUM(DebugFlags);
		FPRINT_NUM(ETxBfEnCond);
		FPRINT_NUM(ITxBfTimeout);
		FPRINT_NUM(ETxBfTimeout);
		FPRINT_NUM(ETxBfNoncompress);
		FPRINT_NUM(ETxBfIncapable);
		FPRINT_NUM(FineAGC);
		FPRINT_NUM(StreamMode);
		FPRINT_STR(StreamModeMac0);
		FPRINT_STR(StreamModeMac1);
		FPRINT_STR(StreamModeMac2);
		FPRINT_STR(StreamModeMac3);


		FPRINT_NUM(CSPeriod);
		FPRINT_STR(RDRegion);
		FPRINT_NUM(StationKeepAlive);
		FPRINT_NUM(DfsLowerLimit);
		FPRINT_NUM(DfsUpperLimit);
		FPRINT_NUM(DfsOutdoor);
		FPRINT_NUM(SymRoundFromCfg);
                FPRINT_NUM(BusyIdleFromCfg);
                FPRINT_NUM(DfsRssiHighFromCfg);
                FPRINT_NUM(DfsRssiLowFromCfg);
		FPRINT_NUM(DFSParamFromConfig);
		FPRINT_STR(FCCParamCh0);
		FPRINT_STR(FCCParamCh1);
		FPRINT_STR(FCCParamCh2);
		FPRINT_STR(FCCParamCh3);
		FPRINT_STR(CEParamCh0);
		FPRINT_STR(CEParamCh1);
		FPRINT_STR(CEParamCh2);
		FPRINT_STR(CEParamCh3);
		FPRINT_STR(JAPParamCh0);
		FPRINT_STR(JAPParamCh1);
		FPRINT_STR(JAPParamCh2);
		FPRINT_STR(JAPParamCh3);
		FPRINT_STR(JAPW53ParamCh0);
		FPRINT_STR(JAPW53ParamCh1);
		FPRINT_STR(JAPW53ParamCh2);
		FPRINT_STR(JAPW53ParamCh3);
		FPRINT_NUM(FixDfsLimit);
		FPRINT_NUM(LongPulseRadarTh);
		FPRINT_NUM(AvgRssiReq);
		FPRINT_NUM(DFS_R66);
		FPRINT_STR(BlockCh);

		FPRINT_NUM(GreenAP);
		FPRINT_STR(PreAuth);
		FPRINT_STR(AuthMode);
		FPRINT_STR(EncrypType);
		/*kurtis: WAPI*/
		FPRINT_STR(WapiPsk1);
		FPRINT_STR(WapiPsk2);
		FPRINT_STR(WapiPsk3);
		FPRINT_STR(WapiPsk4);
		FPRINT_STR(WapiPsk5);
		FPRINT_STR(WapiPsk6);
		FPRINT_STR(WapiPsk7);
		FPRINT_STR(WapiPsk8);
#if defined (CONFIG_16MBSSID_MODE)
		FPRINT_STR(WapiPsk9);
		FPRINT_STR(WapiPsk10);
		FPRINT_STR(WapiPsk11);
		FPRINT_STR(WapiPsk12);
		FPRINT_STR(WapiPsk13);
		FPRINT_STR(WapiPsk14);
		FPRINT_STR(WapiPsk15);
		FPRINT_STR(WapiPsk16);
#endif

		FPRINT_STR(WapiPskType);
		FPRINT_STR(Wapiifname);
		FPRINT_STR(WapiAsCertPath);
		FPRINT_STR(WapiUserCertPath);
		FPRINT_STR(WapiAsIpAddr);
		FPRINT_STR(WapiAsPort);
 
		FPRINT_NUM(BssidNum);

		FPRINT_STR(RekeyMethod);
		FPRINT_NUM(RekeyInterval);
		FPRINT_STR(PMKCachePeriod);

		FPRINT_NUM(MeshAutoLink);
		FPRINT_STR(MeshAuthMode);
		FPRINT_STR(MeshEncrypType);
		FPRINT_NUM(MeshDefaultkey);
		FPRINT_STR(MeshWEPKEY);
		FPRINT_STR(MeshWPAKEY);
		FPRINT_STR(MeshId);

		//WPAPSK
		FPRINT_STR(WPAPSK1);
		FPRINT_STR(WPAPSK2);
		FPRINT_STR(WPAPSK3);
		FPRINT_STR(WPAPSK4);
		FPRINT_STR(WPAPSK5);
		FPRINT_STR(WPAPSK6);
		FPRINT_STR(WPAPSK7);
		FPRINT_STR(WPAPSK8);
#if defined (CONFIG_16MBSSID_MODE)
		FPRINT_STR(WPAPSK9);
		FPRINT_STR(WPAPSK10);
		FPRINT_STR(WPAPSK11);
		FPRINT_STR(WPAPSK12);
		FPRINT_STR(WPAPSK13);
		FPRINT_STR(WPAPSK14);
		FPRINT_STR(WPAPSK15);
		FPRINT_STR(WPAPSK16);
#endif

		FPRINT_STR(DefaultKeyID);
		FPRINT_STR(Key1Type);
		FPRINT_STR(Key1Str1);
		FPRINT_STR(Key1Str2);
		FPRINT_STR(Key1Str3);
		FPRINT_STR(Key1Str4);
		FPRINT_STR(Key1Str5);
		FPRINT_STR(Key1Str6);
		FPRINT_STR(Key1Str7);
		FPRINT_STR(Key1Str8);
#if defined (CONFIG_16MBSSID_MODE)
		FPRINT_STR(Key1Str9);
		FPRINT_STR(Key1Str10);
		FPRINT_STR(Key1Str11);
		FPRINT_STR(Key1Str12);
		FPRINT_STR(Key1Str13);
		FPRINT_STR(Key1Str14);
		FPRINT_STR(Key1Str15);
		FPRINT_STR(Key1Str16);
#endif

		FPRINT_STR(Key2Type);
		FPRINT_STR(Key2Str1);
		FPRINT_STR(Key2Str2);
		FPRINT_STR(Key2Str3);
		FPRINT_STR(Key2Str4);
		FPRINT_STR(Key2Str5);
		FPRINT_STR(Key2Str6);
		FPRINT_STR(Key2Str7);
		FPRINT_STR(Key2Str8);
#if defined (CONFIG_16MBSSID_MODE)
		FPRINT_STR(Key2Str9);
		FPRINT_STR(Key2Str10);
		FPRINT_STR(Key2Str11);
		FPRINT_STR(Key2Str12);
		FPRINT_STR(Key2Str13);
		FPRINT_STR(Key2Str14);
		FPRINT_STR(Key2Str15);
		FPRINT_STR(Key2Str16);
#endif
		FPRINT_STR(Key3Type);
		FPRINT_STR(Key3Str1);
		FPRINT_STR(Key3Str2);
		FPRINT_STR(Key3Str3);
		FPRINT_STR(Key3Str4);
		FPRINT_STR(Key3Str5);
		FPRINT_STR(Key3Str6);
		FPRINT_STR(Key3Str7);
		FPRINT_STR(Key3Str8);
#if defined (CONFIG_16MBSSID_MODE)
		FPRINT_STR(Key3Str9);
		FPRINT_STR(Key3Str10);
		FPRINT_STR(Key3Str11);
		FPRINT_STR(Key3Str12);
		FPRINT_STR(Key3Str13);
		FPRINT_STR(Key3Str14);
		FPRINT_STR(Key3Str15);
		FPRINT_STR(Key3Str16);
#endif
		FPRINT_STR(Key4Type);
		FPRINT_STR(Key4Str1);
		FPRINT_STR(Key4Str2);
		FPRINT_STR(Key4Str3);
		FPRINT_STR(Key4Str4);
		FPRINT_STR(Key4Str5);
		FPRINT_STR(Key4Str6);
		FPRINT_STR(Key4Str7);
		FPRINT_STR(Key4Str8);
#if defined (CONFIG_16MBSSID_MODE)
		FPRINT_STR(Key4Str9);
		FPRINT_STR(Key4Str10);
		FPRINT_STR(Key4Str11);
		FPRINT_STR(Key4Str12);
		FPRINT_STR(Key4Str13);
		FPRINT_STR(Key4Str14);
		FPRINT_STR(Key4Str15);
		FPRINT_STR(Key4Str16);
#endif
		FPRINT_NUM(HSCounter);

		FPRINT_NUM(HT_HTC);
		FPRINT_NUM(HT_RDG);
		FPRINT_NUM(HT_LinkAdapt);
		FPRINT_NUM(HT_OpMode);
		FPRINT_NUM(HT_MpduDensity);
		FPRINT_NUM(HT_EXTCHA);
		FPRINT_NUM(HT_BW);
		FPRINT_NUM(HT_AutoBA);
		FPRINT_NUM(HT_BADecline);
		FPRINT_NUM(HT_AMSDU);
		FPRINT_NUM(HT_BAWinSize);
		FPRINT_NUM(HT_GI);
		FPRINT_NUM(HT_STBC);
		FPRINT_STR(HT_MCS);
		FPRINT_NUM(HT_TxStream);
		FPRINT_NUM(HT_RxStream);
		FPRINT_NUM(HT_PROTECT);
		FPRINT_NUM(HT_DisallowTKIP);
		FPRINT_NUM(HT_BSSCoexistence);
		FPRINT_NUM(HT_LDPC);
		FPRINT_NUM(GreenAP);
		FPRINT_NUM(VHT_BW);
		FPRINT_NUM(VHT_STBC);
		FPRINT_NUM(VHT_SGI);
		FPRINT_NUM(VHT_BW_SIGNAL);
		FPRINT_NUM(VHT_LDPC);

		FPRINT_NUM(WscConfMode);

		//WscConfStatus
		if (atoi(nvram_bufget(mode, "WscConfigured")) == 0)
			fprintf(fp, "WscConfStatus=%d\n", 1);
		else
			fprintf(fp, "WscConfStatus=%d\n", 2);
		if (strcmp(nvram_bufget(mode, "WscVendorPinCode"), "") != 0)
			FPRINT_STR(WscVendorPinCode);
		FPRINT_NUM(WCNTest);
#if defined CONFIG_UNIQUE_WPS
		FPRINT_STR(WSC_UUID_Str1);
		FPRINT_STR(WSC_UUID_E1);
#endif

		FPRINT_NUM(AccessPolicy0);
		FPRINT_STR(AccessControlList0);
		FPRINT_NUM(AccessPolicy1);
		FPRINT_STR(AccessControlList1);
		FPRINT_NUM(AccessPolicy2);
		FPRINT_STR(AccessControlList2);
		FPRINT_NUM(AccessPolicy3);
		FPRINT_STR(AccessControlList3);
		FPRINT_NUM(AccessPolicy4);
		FPRINT_STR(AccessControlList4);
		FPRINT_NUM(AccessPolicy5);
		FPRINT_STR(AccessControlList5);
		FPRINT_NUM(AccessPolicy6);
		FPRINT_STR(AccessControlList6);
		FPRINT_NUM(AccessPolicy7);
		FPRINT_STR(AccessControlList7);
#if defined (CONFIG_16MBSSID_MODE)
		FPRINT_NUM(AccessPolicy8);
		FPRINT_STR(AccessControlList8);
		FPRINT_NUM(AccessPolicy9);
		FPRINT_STR(AccessControlList9);
		FPRINT_NUM(AccessPolicy10);
		FPRINT_STR(AccessControlList10);
		FPRINT_NUM(AccessPolicy11);
		FPRINT_STR(AccessControlList11);
		FPRINT_NUM(AccessPolicy12);
		FPRINT_STR(AccessControlList12);
		FPRINT_NUM(AccessPolicy13);
		FPRINT_STR(AccessControlList13);
		FPRINT_NUM(AccessPolicy14);
		FPRINT_STR(AccessControlList14);
		FPRINT_NUM(AccessPolicy15);
		FPRINT_STR(AccessControlList15);
#endif

		FPRINT_NUM(WdsEnable);
		FPRINT_STR(WdsPhyMode);
		FPRINT_STR(WdsEncrypType);
		FPRINT_STR(WdsList);
		FPRINT_STR(Wds0Key);
		FPRINT_STR(Wds1Key);
		FPRINT_STR(Wds2Key);
		FPRINT_STR(Wds3Key);
		FPRINT_STR(RADIUS_Server);
		FPRINT_STR(RADIUS_Port);
		FPRINT_STR(RADIUS_Key1);
		FPRINT_STR(RADIUS_Key2);
		FPRINT_STR(RADIUS_Key3);
		FPRINT_STR(RADIUS_Key4);
		FPRINT_STR(RADIUS_Key5);
		FPRINT_STR(RADIUS_Key6);
		FPRINT_STR(RADIUS_Key7);
		FPRINT_STR(RADIUS_Key8);
#if defined (CONFIG_16MBSSID_MODE)
		FPRINT_STR(RADIUS_Key9);
		FPRINT_STR(RADIUS_Key10);
		FPRINT_STR(RADIUS_Key11);
		FPRINT_STR(RADIUS_Key12);
		FPRINT_STR(RADIUS_Key13);
		FPRINT_STR(RADIUS_Key14);
		FPRINT_STR(RADIUS_Key15);
		FPRINT_STR(RADIUS_Key16);
#endif
		FPRINT_STR(RADIUS_Acct_Server);
		FPRINT_NUM(RADIUS_Acct_Port);
		FPRINT_STR(RADIUS_Acct_Key);
		FPRINT_STR(own_ip_addr);
		FPRINT_STR(Ethifname);
		FPRINT_STR(EAPifname);
		FPRINT_STR(PreAuthifname);
		FPRINT_NUM(session_timeout_interval);
		FPRINT_NUM(idle_timeout_interval);
		FPRINT_NUM(WiFiTest);
		FPRINT_NUM(TGnWifiTest);

		//AP Client parameters
		FPRINT_NUM(ApCliEnable);
		FPRINT_STR(ApCliSsid);
		FPRINT_STR(ApCliBssid);
		FPRINT_STR(ApCliAuthMode);
		FPRINT_STR(ApCliEncrypType);
		FPRINT_STR(ApCliWPAPSK);
		FPRINT_NUM(ApCliDefaultKeyID);
		FPRINT_NUM(ApCliKey1Type);
		FPRINT_STR(ApCliKey1Str);
		FPRINT_NUM(ApCliKey2Type);
		FPRINT_STR(ApCliKey2Str);
		FPRINT_NUM(ApCliKey3Type);
		FPRINT_STR(ApCliKey3Str);
		FPRINT_NUM(ApCliKey4Type);
		FPRINT_STR(ApCliKey4Str);
		//FPRINT_NUM(TempComp);
		FPRINT_NUM(EfuseBufferMode);
		FPRINT_NUM(E2pAccessMode);

		//Radio On/Off
		if (atoi(nvram_bufget(mode, "RadioOff")) == 1)
			fprintf(fp, "RadioOn=0\n");
		else
			fprintf(fp, "RadioOn=1\n");

		FPRINT_NUM(BW_Enable);
		FPRINT_NUM(BW_Root);
		FPRINT_STR(BW_Priority);
		FPRINT_STR(BW_Guarantee_Rate);
		FPRINT_STR(BW_Maximum_Rate);
		/*
		 * There are no SSID/WPAPSK/Key1Str/Key2Str/Key3Str/Key4Str anymore since driver1.5 , but 
		 * STA WPS still need these entries to show the WPS result(That is the only way i know to get WPAPSK key) and
		 * so we create empty entries here.   --YY
		 */
		fprintf(fp, "SSID=\nWPAPSK=\nKey1Str=\nKey2Str=\nKey3Str=\nKey4Str=\n");
	}
#if defined (CONFIG_RT2561_AP) || defined (CONFIG_RT2561_AP_MODULE)
	if (RTDEV_NVRAM == mode) {
		FPRINT_NUM(CountryRegion);
		FPRINT_NUM(CountryRegionABand);
		FPRINT_STR(CountryCode);
		FPRINT_NUM(BssidNum);
		ssid_num = atoi(nvram_get(mode, "BssidNum"));
		FPRINT_STR(SSID);
		FPRINT_NUM(WirelessMode);
		//TxRate(FixedRate)
		bzero(tx_rate, sizeof(char)*12);
		for (i = 0; i < ssid_num; i++)
		{
			sprintf(tx_rate+strlen(tx_rate), "%d",
			atoi(nvram_bufget(mode, "TxRate")));
			sprintf(tx_rate+strlen(tx_rate), "%c", ';');
		}
		tx_rate[strlen(tx_rate) - 1] = '\0';
		fprintf(fp, "TxRate=%s\n", tx_rate);

		FPRINT_NUM(Channel);
		FPRINT_NUM(BasicRate);
		FPRINT_NUM(BeaconPeriod);
		FPRINT_NUM(DtimPeriod);
		FPRINT_NUM(TxPower);
		FPRINT_NUM(DisableOLBC);
		FPRINT_NUM(BGProtection);
		fprintf(fp, "TxAntenna=\n");
		fprintf(fp, "RxAntenna=\n");
		FPRINT_NUM(TxPreamble);
		FPRINT_NUM(RTSThreshold  );
		FPRINT_NUM(FragThreshold  );
		FPRINT_NUM(TxBurst);
		FPRINT_NUM(PktAggregate);
		fprintf(fp, "TurboRate=0\n");

		//WmmCapable
		bzero(wmm_enable, sizeof(char)*8);
		for (i = 0; i < ssid_num; i++)
		{
			sprintf(wmm_enable+strlen(wmm_enable), "%d",
			atoi(nvram_bufget(mode, "WmmCapable")));
			sprintf(wmm_enable+strlen(wmm_enable), "%c", ';');
		}
		wmm_enable[strlen(wmm_enable) - 1] = '\0';
		fprintf(fp, "WmmCapable=%s\n", wmm_enable);

		FPRINT_STR(APAifsn);
		FPRINT_STR(APCwmin);
		FPRINT_STR(APCwmax);
		FPRINT_STR(APTxop);
		FPRINT_STR(APACM);
		FPRINT_STR(BSSAifsn);
		FPRINT_STR(BSSCwmin);
		FPRINT_STR(BSSCwmax);
		FPRINT_STR(BSSTxop);
		FPRINT_STR(BSSACM);
		FPRINT_STR(AckPolicy);
		FPRINT_STR(APSDCapable);
		FPRINT_STR(DLSCapable);
		FPRINT_STR(NoForwarding);
		FPRINT_NUM(NoForwardingBTNBSSID);
		FPRINT_STR(HideSSID);
		FPRINT_NUM(ShortSlot);
		FPRINT_NUM(AutoChannelSelect);
		FPRINT_NUM(MaxTxPowerLevel);
		FPRINT_STR(IEEE8021X);
		FPRINT_NUM(IEEE80211H);
		FPRINT_NUM(CSPeriod);
		FPRINT_STR(RDRegion);
		FPRINT_STR(PreAuth);
		FPRINT_STR(AuthMode);
		FPRINT_STR(EncrypType);
	    /*kurtis: WAPI*/
		FPRINT_STR(WapiPsk1);
		FPRINT_STR(WapiPskType);
		FPRINT_STR(Wapiifname);
		FPRINT_STR(WapiAsCertPath);
		FPRINT_STR(WapiUserCertPath);
		FPRINT_STR(WapiAsIpAddr);
		FPRINT_STR(WapiAsPort);
 	
		FPRINT_NUM(RekeyInterval);
		FPRINT_STR(RekeyMethod);
		FPRINT_STR(PMKCachePeriod);
		FPRINT_STR(WPAPSK);
		FPRINT_STR(DefaultKeyID);
		FPRINT_STR(Key1Type);
		FPRINT_STR(Key1Str);
		FPRINT_STR(Key2Type);
		FPRINT_STR(Key2Str);
		FPRINT_STR(Key3Type);
		FPRINT_STR(Key3Str);
		FPRINT_STR(Key4Type);
		FPRINT_STR(Key4Str);
		FPRINT_NUM(HSCounter);
		FPRINT_NUM(AccessPolicy0);
		FPRINT_STR(AccessControlList0);
		FPRINT_NUM(AccessPolicy1);
		FPRINT_STR(AccessControlList1);
		FPRINT_NUM(AccessPolicy2);
		FPRINT_STR(AccessControlList2);
		FPRINT_NUM(AccessPolicy3);
		FPRINT_STR(AccessControlList3);
		FPRINT_NUM(WdsEnable);
		FPRINT_STR(WdsPhyMode);
		FPRINT_STR(WdsEncrypType);
		FPRINT_STR(WdsList);
		FPRINT_STR(WdsKey);
		FPRINT_STR(RADIUS_Server);
		FPRINT_STR(RADIUS_Port);
		FPRINT_STR(RADIUS_Key);
		FPRINT_STR(own_ip_addr);
		FPRINT_STR(Ethifname);
		//AP Client parameters
		FPRINT_NUM(ApCliEnable);
		FPRINT_STR(ApCliSsid);
		FPRINT_STR(ApCliBssid);
		FPRINT_STR(ApCliAuthMode);
		FPRINT_STR(ApCliEncrypType);
		FPRINT_STR(ApCliWPAPSK);
		FPRINT_NUM(ApCliDefaultKeyID);
		FPRINT_NUM(ApCliKey1Type);
		FPRINT_STR(ApCliKey1Str);
		FPRINT_NUM(ApCliKey2Type);
		FPRINT_STR(ApCliKey2Str);
		FPRINT_NUM(ApCliKey3Type);
		FPRINT_STR(ApCliKey3Str);
		FPRINT_NUM(ApCliKey4Type);
		FPRINT_STR(ApCliKey4Str);
		FPRINT_NUM(WscConfMode);
		//WscConfStatus
		if (atoi(nvram_bufget(mode, "WscConfigured")) == 0)
			fprintf(fp, "WscConfStatus=%d\n", 1);
		else
			fprintf(fp, "WscConfStatus=%d\n", 2);
	}
#elif defined (CONFIG_RTDEV_PLC)
	if (RTDEV_NVRAM == mode) {
		FPRINT_STR(MacAddress);
		FPRINT_STR(NPW);
		FPRINT_STR(DPW);
		FPRINT_NUM(BackupCCo);
		FPRINT_NUM(CCo);
		FPRINT_NUM(UnAssoCCo);
		FPRINT_NUM(STA);
		FPRINT_NUM(UnAssoSTA);
	}
#endif

	nvram_close(mode);
	fclose(fp);
	return 0;
}

int renew_nvram(int mode, char *fname)
{
	FILE *fp;
#define BUFSZ 1024
	unsigned char buf[BUFSZ], *p;
	unsigned char wan_mac[32];
	int found = 0, need_commit = 0;

	if (NULL == (fp = fopen(fname, "ro"))) {
		perror("fopen");
		return -1;
	}

	//find "Default" first
	while (NULL != fgets(buf, BUFSZ, fp)) {
		if (buf[0] == '\n' || buf[0] == '#')
			continue;
		if (!strncmp(buf, "Default\n", 8)) {
			found = 1;
			break;
		}
	}
	if (!found) {
		printf("file format error!\n");
		fclose(fp);
		return -1;
	}

	nvram_init(mode);
	while (NULL != fgets(buf, BUFSZ, fp)) {
		if (buf[0] == '\n' || buf[0] == '#')
			continue;
		if (NULL == (p = strchr(buf, '='))) {
			if (need_commit)
				nvram_commit(mode);
			printf("%s file format error!\n", fname);
			fclose(fp);
			return -1;
		}
		buf[strlen(buf) - 1] = '\0'; //remove carriage return
		*p++ = '\0'; //seperate the string
		//printf("bufset %d '%s'='%s'\n", mode, buf, p);
		nvram_bufset(mode, buf, p);
		need_commit = 1;
	}

	//Get wan port mac address, please refer to eeprom format doc
	//0x30000=user configure, 0x32000=rt2860 parameters, 0x40000=RF parameter
	flash_read_mac(buf);
	sprintf(wan_mac,"%0X:%0X:%0X:%0X:%0X:%0X\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
	nvram_bufset(RT2860_NVRAM, "WAN_MAC_ADDR", wan_mac);

	need_commit = 1;

	if (need_commit)
		nvram_commit(mode);

	nvram_close(mode);
	fclose(fp);
	return 0;
}

int nvram_show(int mode)
{
	nvram_init(mode);
	nvram_buflist(mode);
	nvram_close(index);
#if 0
	char *buffer, *p;
	int rc;
	int crc;
	unsigned int len = 0x4000;

	nvram_init(mode);
	len = getNvramBlockSize(mode);

	if ((mode = getNvramIndex(fz)) == -1) {
		printf("%s: Error: \"%s\" flash zone not existed\n", argv[0], fz);
		return get_usage(argv[0]);
	}

	buffer = malloc(len);
	if (buffer == NULL) {
		fprintf(stderr, "nvram_show: Can not allocate memory!\n");
		return -1;
	}
	if (mode == CONFIG2_NVRAM)
		flash_read("Config2", buffer, getNvramOffset(mode), len);
	else
		flash_read("Config", buffer, getNvramOffset(mode), len);
	memcpy(&crc, buffer, 4);

	fprintf(stderr, "crc = %x\n", crc);
	p = buffer + 4;
	while (*p != '\0') {
		printf("%s\n", p);
		p += strlen(p) + 1;
	}

	free(buffer);
#endif
	return 0;
}


