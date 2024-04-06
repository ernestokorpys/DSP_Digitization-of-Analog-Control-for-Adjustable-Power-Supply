#include "p30f4011.h"
#define FCY  1500000	 		      // 6MHz Crystal Frequency (Fosc/4 = Fcy)
#define MILLISEC FCY/10000            // 1 Sec delay constant
void DelayNmSec(unsigned int N)
{
   unsigned int j;
   while(N--)
        for(j=0;j < MILLISEC;j++);
   return;
}
