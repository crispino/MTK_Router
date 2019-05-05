#ifndef ZLINK_UTILS_H
#define ZLINK_UTILS_H

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#define ZLINK_LOGFILE	"/dev/console"

/** 执行系统指令 */
void zlink_doSystem(char *fmt, ...);

/** 取字符串的子串 */
void zlink_getSplitValue(char *record, char *split, int index, char *result);

char *zlink_setNthValue(int index, char *old_values, char *new_value);

int zlink_getNthValue(int index, char *value, char delimit, char *result, int len);

/** 字符串替换操作**/
char* zlink_strReplace(const char *original, const char *pattern,  const char *replacement);

/**Trim头部和尾部的无效字符**/
void zlink_trimLeft(char *s, char c);
void zlink_trimRight(char *s, char c);
void zlink_trim(char *s, char c);

#endif

