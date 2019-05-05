#ifndef CLIENT_FLOW_H
#define CLIENT_FLOW_H

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>


#define MAX_NUMBER_OF_CLIENT 64
#define STAT_INTERVAL	5

#define GET_STAT_CLIENTS "web 2860 wifi getStatClients"

typedef struct t_FlowEntry {
	char 					 ipAddr[16];
	unsigned long            lastRxRate;  //�ϴεĽ���ʵʱ����
	unsigned long            lastTxRate; //�ϴεķ���ʵʱ����
	unsigned long            RxRate;     //��ǰ�Ľ���ʵʱ����
	unsigned long            TxRate;	//��ǰ�ķ���ʵʱ����
} recFlowEntry;

typedef struct t_FlowClients {
	int               Num;
	recFlowEntry      Entry[MAX_NUMBER_OF_CLIENT];
} recFlowClients;

int getCfgValue(const char *cmd, char *cfgValue, size_t len);
void saveToFile(const recFlowClients *prFlowClients);
int getNthValue(int index, char *value, char delimit, char *result, int len);
int getFlowData(recFlowClients *prFlowClients);
void statisFlows(recFlowClients *prFlowClients);

#endif
