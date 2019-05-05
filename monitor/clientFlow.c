#include "clientFlow.h"

int getCfgValue(const char *cmd, char *cfgValue, size_t len)
{
    FILE* fp; 
    if((fp = popen(cmd, "r")) == NULL) {
        printf("getCfgValue:popen failed.");
        return -1;
    }
	
	fgets(cfgValue, len, fp);
	
	pclose(fp);
	return 0;
}

void saveToFile(const recFlowClients *prFlowClients)
{
	int i = 0;
	char cmd[4096]= {0};
	char tmp[64] = {0};	
	recFlowEntry *pe = NULL;

	strcpy(cmd, "echo \"");
	
	for (i = 0; i < prFlowClients->Num; i++) {
		pe = &(prFlowClients->Entry[i]);
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%s,%lu,%lu;", pe->ipAddr, pe->RxRate, pe->TxRate);
		strcat(cmd, tmp);
	}

	memset(tmp, 0, sizeof(tmp));
	strcpy(tmp, "\" > /etc_ro/client_flow_stats.txt");
	strcat(cmd, tmp);
	
	system(cmd);
	return;
}

int getNthValue(int index, char *value, char delimit, char *result, int len)
{
	int i=0, result_len=0;
	char *begin, *end;

	if(!value || !result || !len)
		return -1;

	begin = value;
	end = strchr(begin, delimit);

	while(i<index && end){
		begin = end+1;
		end = strchr(begin, delimit);
		i++;
	}

	//no delimit
	if(!end){
		if(i == index){
			end = begin + strlen(begin);
			result_len = (len-1) < (end-begin) ? (len-1) : (end-begin);
		}else
			return -1;
	}else
		result_len = (len-1) < (end-begin)? (len-1) : (end-begin);

	memcpy(result, begin, result_len );
	*(result+ result_len ) = '\0';

	return 0;
}


int getFlowData(recFlowClients *prFlowClients)
{
	//printf("begin getFlowData\n");

	char sStatClients[4096] = {0};
	char rec[64] = {0};
	char ipAddr[16] = {0};
	char in[12] = {0};
	char out[12] = {0};
	int i, j, flag;
	unsigned long inValue, outValue;

	//Get kernel statistic data
	int ret = getCfgValue(GET_STAT_CLIENTS, sStatClients, sizeof(sStatClients) - 1);
	if (ret < 0) return -1;

	/* data format: ip,in,out;ip,in,out; */
	i = inValue = outValue = 0;
	while((getNthValue(i++, sStatClients, ';', rec, sizeof(rec)) != -1) && strlen(rec)) {

		//1. ipaddr
		if((getNthValue(0, rec, ',', ipAddr, sizeof(ipAddr)) == -1)){
			continue;
		}

		//2. in
		if((getNthValue(1, rec, ',', in, sizeof(in)) == -1)){
			continue;
		}
		inValue = strtoul(in, NULL, 10);

		//3. out
		if((getNthValue(2, rec, ',', out, sizeof(out)) == -1)){
			continue;
		}
		outValue = strtoul(out, NULL, 10);

		flag = -1;
		recFlowEntry *pe = prFlowClients->Entry;
		for (j = 0;  j < prFlowClients->Num; j++) {
			if (!strcmp(pe[j].ipAddr, ipAddr)) {
				if (inValue > 0 && (inValue - pe[j].lastRxRate) >= 0) {
					pe[j].RxRate = (inValue - pe[j].lastRxRate) / STAT_INTERVAL;
					pe[j].lastRxRate = inValue;
				}

				if (outValue > 0 && (outValue - pe[j].lastTxRate) >= 0) {
					pe[j].TxRate = (outValue - pe[j].lastTxRate) / STAT_INTERVAL;
					pe[j].lastTxRate = outValue;
				}
				
				break;
			}

			//找到一个数据为0的节点
			if (pe[j].RxRate == 0 && pe[j].TxRate ==0) flag = j;
		}

		if (j == prFlowClients->Num) {
			//Not match ip address, and total num is less max clients number，
			//add the ip to the stat client list.
			if (j < MAX_NUMBER_OF_CLIENT) {
				strcpy(pe[j].ipAddr, ipAddr);
				pe[j].RxRate = inValue / STAT_INTERVAL;
				pe[j].lastRxRate = inValue;
				pe[j].TxRate = outValue / STAT_INTERVAL;
				pe[j].lastTxRate = outValue;
				
				prFlowClients->Num += 1;
				
			} else {
			
			    //Not match ip address, find a invalid node, then replace it with the new one.
			    if (flag >= 0) {
					strcpy(pe[flag].ipAddr, ipAddr);
					pe[flag].RxRate = inValue / STAT_INTERVAL;
					pe[flag].lastRxRate = inValue;
					pe[flag].TxRate = outValue / STAT_INTERVAL;
					pe[flag].lastTxRate = outValue;
			    }
			}
		}

	}

	return 0;
}


void statisFlows(recFlowClients *prFlowClients)
{
	//printf("begin statisFlows\n");
	int ret = getFlowData(prFlowClients);
	if (ret < 0) return;
	
	saveToFile(prFlowClients);
	return;
}

