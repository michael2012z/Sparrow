#ifndef _STRING_H_
#define _STRING_H_

void *memcpy(void *dest, const void *src, unsigned int count);
int memcmp(const void *cs, const void *ct, unsigned int count);
void *memset(void *s, int c, unsigned int count);
unsigned int strlen(const char *s);
int strcmp(const char *cs, const char *ct);
char *skip_spaces(const char *str);
char *strim(char *s);
int atoi(const char *str);

#endif
