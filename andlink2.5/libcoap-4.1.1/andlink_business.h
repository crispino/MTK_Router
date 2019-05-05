#ifndef ANDLINK_BUSINESS_H
#define ANDLINK_BUSINESS_H

#ifdef	__cplusplus
extern "C" {
#endif

char* get_ApConfigInfo(int nProtocol, const char *id);

char* get_UplinkStatus(int nProtocol, const char *id);

char* get_STAInfo(int nProtocol, const char *id);

char* get_WiFIStats(int nProtocol, const char *id);



#ifdef __cplusplus
}
#endif


#endif


