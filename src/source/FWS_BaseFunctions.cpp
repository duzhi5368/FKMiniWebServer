#include "../include/FWS_BaseFunctions.h"
//------------------------------------------------------------------------------
unsigned int hex2uint32(const char *p)
{
	register char c;
	register unsigned int i = 0;
	for (c = *p;;) {
		if (c >= 'A' && c <= 'F')
			c -= 7;
		else if (c >= 'a' && c <= 'f')
			c -= 39;
		else if (c<'0' || c>'9')
			break;
		i = (i << 4) | (c & 0xF);
		c = *(++p);
	}
	return i;
}