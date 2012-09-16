#include <string.h>

/**
 * memcpy - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * You should not use this function to access IO space, use memcpy_toio()
 * or memcpy_fromio() instead.
 */
void *memcpy(void *dest, const void *src, unsigned int count)
{
	char *tmp = dest;
	const char *s = src;

	while (count--)
		*tmp++ = *s++;
	return dest;
}

/**
 * memcmp - Compare two areas of memory
 * @cs: One area of memory
 * @ct: Another area of memory
 * @count: The size of the area.
 */
int memcmp(const void *cs, const void *ct, unsigned int count)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}

void *memset(void *s, int c, unsigned int count)
{
	char *xs = s;
	while (count--)
		*xs++ = c;
	return s;
}


int strcmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;

	while (1) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

unsigned int strlen(const char *s)
{
	const char *sc = s;

	while (*sc != '\0')
		sc++;
	return sc - s;
}



int atoi(const char *str) {
  int minus = 0;
  int i = 0, n = 0;

  while(' ' == str[i])
	i++;

  if ('-' == str[i])
	minus = 1;

  if (('-' == str[i]) || ('+' == str[i]))
	i++;

  for (n = 0; ((str[i] >= '0') && (str[i] <= '9')); i++)
	n = n*10 + (str[i] - '0');

  return n;
}
