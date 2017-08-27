
#include <kernel.h>


int k_strlen(const char* str)
{
	int len = 0;
	while (*str != '\0') {
		len++;
		str++;
	} // optional way: while (*str++ != '\0') ++len;
	  // although ++ priority higher than dereference, since it is a post++
	  // it will *str first and then str++

	return (len);
}

void* k_memcpy(void* dst, const void* src, int len)
{
	char* ch_src = (char*)src;
	char* ch_dst = (char*)dst;
	for(int i = 0; i < len; i++) {
		*ch_dst++ = *ch_src++;	
	}
	return (dst);
}

// return value < 0 if b1 < b2
int k_memcmp(const void* b1, const void* b2, int len)
{
	unsigned char* ch_b1 = (unsigned char*)b1; //unsigned char might range from 0 to 255
	unsigned char* ch_b2 = (unsigned char*)b2; //signed char might range from -128 to 127
	for (int i = 0; i < len; i++) {
		int diff = *ch_b1++ - *ch_b2++;
		if (diff != 0) return (diff);
	}
	return (0);
}

void k_memset(const void* b, char fill, int len)
{
	unsigned char* ch_b = (unsigned char*)b;
	for (int i = 0; i < len; i++) {
		*ch_b++ = fill;
	}
	// no need to return (b);
}

