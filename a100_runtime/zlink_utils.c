#include "zlink_utils.h"

void zlink_doSystem(char *fmt, ...)
{
    char cmd[256] = {0};
	va_list ap;
	va_start(ap, fmt);
	vsprintf(cmd, fmt, ap);
	va_end(ap);
	sprintf(cmd, "%s 1>%s 2>&1", cmd, ZLINK_LOGFILE);
	system(cmd);

	return;
}

char *zlink_setNthValue(int index, char *old_values, char *new_value)
{
	int i;
	char *p, *q;
	static char ret[2048];
	char buf[8][256];

	memset(ret, 0, 2048);
	for (i = 0; i < 8; i++)
		memset(buf[i], 0, 256);

	//copy original values
	for ( i = 0, p = old_values, q = strchr(p, ';');
			i < 8 && q != NULL;
			i++, p = q + 1, q = strchr(p, ';')) {
		
		strncpy(buf[i], p, q - p);
	}
	strcpy(buf[i], p); //the last one

	//replace buf[index] with new_value
	strncpy(buf[index], new_value, 256);

	//calculate maximum index
	index = (i > index)? i : index;

	//concatenate into a single string delimited by semicolons
	strcat(ret, buf[0]);
	for (i = 1; i <= index; i++) {
		strncat(ret, ";", 2);
		strncat(ret, buf[i], 256);
	}

	return ret;
}

void zlink_getSplitValue(char *record, char *split, int index, char *result)
{
    int nPos = 0;
    char *pTmp = NULL;
    char *pTok= NULL;
    pTmp = record;
    pTok = strtok(pTmp, split);
    while(NULL != pTok)
    {
         if (nPos + 1 == index) {
			strcpy(result, pTok);
			break;
		 }
         nPos++;
         pTok = strtok(NULL, split);	 
    }
    
    return;
}


int zlink_getNthValue(int index, char *value, char delimit, char *result, int len)
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


/* 字符串替换操作**/
char *zlink_strReplace(const char *original, const char *pattern,  const char *replacement) 
{
	//input check
	if (original == NULL || pattern == NULL || replacement == NULL) return NULL;
	
	size_t const replen = strlen(replacement);
	size_t const patlen = strlen(pattern);
	size_t const orilen = strlen(original);

	size_t patcnt = 0;
	const char * oriptr;
	const char * patloc;

	// find how many times the pattern occurs in the original string
	for (oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen) {
		patcnt++;
	}

	// allocate memory for the new string
	size_t const retlen = orilen + patcnt * (replen - patlen);
	char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

	if (returned != NULL) {
		// copy the original string, 
		// replacing all the instances of the pattern
		char * retptr = returned;
		for (oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)
		{
			size_t const skplen = patloc - oriptr;
			// copy the section until the occurence of the pattern
			strncpy(retptr, oriptr, skplen);
			retptr += skplen;
			// copy the replacement 
			strncpy(retptr, replacement, replen);
			retptr += replen;
		}
		// copy the rest of the string.
		strcpy(retptr, oriptr);
	}
	
	return returned;
}

/** 截去左面的空格 **/
void zlink_trimLeft(char *s, char c)
{
    int i = 0, j = 0;
	
     //入参检查
    if(s == NULL || !strlen(s)) return;
	
    //从头开始查找字符c, 查找到则移动下标
    while (s[i] == c && s[i] != '\0') i++;
	
    //向左移动数组
    while (s[i] != '\0') s[j++] = s[i++];
	
    s[j] = '\0';
}

/** 截去右边的空格 **/
void zlink_trimRight(char *s, char c)
{
    int pos;
	
    //入参检查
    if(s == NULL || !strlen(s)) return;
	
    pos = strlen(s) - 1;
	
    //从尾部开始查找字符c
    while(s[pos] == c) {
		
        s[pos--] = '\0';
        if(pos < 0) break;
    }
}

/** 去除左右的空格 **/
void zlink_trim(char *s, char c)
{
    zlink_trimLeft(s, c);
    zlink_trimRight(s, c);
}

