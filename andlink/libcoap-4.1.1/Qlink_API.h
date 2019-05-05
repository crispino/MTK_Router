#ifndef QLINK_API_C_H_
#define QLINK_API_C_H_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if __cplusplus
extern "C"
{
#endif
typedef int (*onReceiveResponse)(unsigned char* body, int len);
const char* Qlink_GetVersion();

//返回隐藏wifi的ssid
const char* Qlink_GetHiddenSSID();

//获取隐藏SSID的密码
const char* Qlink_GetHiddenSSIDPWD();

// 返回网关的IP地址
const char* Qlink_FindGatewayIP();

/*加入隐藏ssid后(ssid:CMCC-QLINK pwd:sD5fH%9k),调用此方法*/
int Qlink_notifyGatewayJoinBoot(const char* deviceId, const char* deviceType, const char* ipAddress);

//消息接收服务接口
int Qlink_StartCoapServer();

//设置上网通道的回调
void Qlink_setReciveInternetChannelCallback(onReceiveResponse _callback);

//应答网关的请求 
int Qlink_ackQlinkNetinfo(const char* deviceId);

//发送原始的coap请求接口
int Qlink_SendCaopRequest(const char* coapurl, const char *option, const char *data, char ** response, int *len);
#if __cplusplus
}
#endif

#endif /*QLINK_API_C_H_*/
