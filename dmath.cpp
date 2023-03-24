#include "dmath.h"
#include <assert.h>

// Magic numbers from https://users.ece.cmu.edu/~koopman/lfsr/
uint32_t lfsr_tap(uint32_t size)
{
	switch (size)
	{
	case 2: return 0x3;
	case 3: return 0x6;
	case 4: return 0xC;
	case 5: return 0x14;
	case 6: return 0x30;
	case 7: return 0x60;
	case 8: return 0xB4;
	case 9: return 0x110;
	case 10: return 0x240;
	case 11: return 0x500;
	case 12: return 0xE08;
	case 13: return 0x1C80;
	case 14: return 0x3802;
	case 15: return 0x6000;
	case 16: return 0xB400;
	case 17: return 0x10004;
	case 18: return 0x20013;
	case 19: return 0x40013;
	case 20: return 0x80004;
	case 21: return 0x100002;
	case 22: return 0x200001;
	case 23: return 0x400010;
	case 24: return 0x80000D;
	case 25: return 0x1000004;
	case 26: return 0x2000023;
	case 27: return 0x4000013;
	case 28: return 0x8000004;
	case 29: return 0x10000002;
	case 30: return 0x20000029;
	case 31: return 0x40000004;
	case 32: return 0x80000057;
	default: assert(false); break;
	}
	return 0;
}
