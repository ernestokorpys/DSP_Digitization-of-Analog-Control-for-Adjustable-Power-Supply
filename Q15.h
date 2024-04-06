/***************************************************/
/* Macros para la definicion de operaciones en Q15 */
/***************************************************/

#define _Q15 short
#define FloatToFix(B) ((int)((-B)*(float)(1<<15)))
#define FixToFloat(b) ((float)(-b)/(float)(1<<15))
#define AddFix(a,b) ((a)+(b))
#define SubFix(a,b) ((a)-(b))
#define DivFix(a,b) (((a)<<15/(b))
#define MulFix(a,b) (short)(((long)(a)*(long)(b))>>15)
