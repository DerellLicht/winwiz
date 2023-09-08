//************************************************************************************
//  This is a test function to analyse random attribute-scrambling algorithms
//  for the Wizard's Castle program.
//  
//  This function is one of the optional operations that may be invoked when
//  a chest is opened.  The original design attempted to add up the existing
//  attribute values and randomly re-distribute them, but that is difficult to
//  do in a truly balanced fashion.
//  
//  Instead, this modified routine (from 09/08/23) randomly generates new attributes
//  from the theoretical maximum values.  
//  It may generate a total amount which is more or less than the original total, 
//  but it has advantage of being a truly random distribution.
//************************************************************************************
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>

typedef unsigned short        u16 ;
typedef unsigned int          uint ;
//**************************************************************************
//  we *have* to user our own rand/srand functions, so we'll get
//  the same result on every execution
//**************************************************************************
#define  RAND_MAX2   (0x7FFF)
static unsigned long holdrand = 0 ;

//lint -esym(714, rand2)
//lint -esym(759, rand2)
//lint -esym(765, rand2)
u16 rand2(void)
{
   return((u16) ((holdrand = holdrand * 214013UL + 2531011UL) >> 16) & 0x7FFFUL); 
}

//lint -esym(714, srand2)
//lint -esym(759, srand2)
//lint -esym(765, srand2)
void srand2(unsigned long seed)
{
   holdrand = seed;
}

//**************************************************************************
//  generate random number between 0 and n-1
//**************************************************************************
// In Numerical Recipes in C: The Art of Scientific Computing (William H. 
// Press, Brian P. Flannery, Saul A. Teukolsky, William T. Vetterling;
// New York: Cambridge University Press, 1990 (1st ed, p. 207)),      
// the following comments are made:                                   
//                                                                    
// "If you want to generate a random integer between 1 and 10,        
// you should always do it by                                         
//                                                                    
//    j=1+(int) (10.0*rand()/(RAND_MAX+1.0));                         
//                                                                    
// and never by anything resembling                                   
//                                                                    
//    j=1+((int) (1000000.0*rand()) % 10);                            
//                                                                    
// (which uses lower-order bits)."                                    
//**************************************************************************
//lint -esym(714, random_int)
//lint -esym(759, random_int)
//lint -esym(765, random_int)
unsigned random_int(unsigned n)
{
   //  Note that this *may* overflow if n > 0xFFFF
   return (unsigned) ((unsigned) rand2() * n / (unsigned) (RAND_MAX2 + 1)) ;
}

//*************************************************************
// This function is still not optimum; 
// It still favours STR over DEX over INT.
// str= 3, dex= 9, int= 3
// str=13, dex= 2, int= 0
// str= 8, dex= 3, int= 4
// str=13, dex= 2, int= 0
// str= 3, dex=11, int= 1
// str= 9, dex= 4, int= 2
// str= 4, dex= 7, int= 4
// str= 3, dex=12, int= 0
//*************************************************************
#define  NEW_METHOD  1

void scramble(unsigned str, unsigned dex, unsigned iq)
{
   unsigned j=0, k=0, l=0, attr_temp = str + dex + iq ;
   
   //  09/01/23  Try this instead:
   //  str = random(100)
   //  dex = random(100)
   //  iq  = random(100)
   //  sum = str + dex + iq
   //  Use these to translate to general percentages,
   //  then translate to existing attr total

#ifdef  NEW_METHOD
   // j = random_int(100);
   // k = random_int(100);
   // l = random_int(100);
   // j = random_int(attr_temp);
   // k = random_int(attr_temp);
   // l = random_int(attr_temp);
   j = 1 + random_int(18);
   k = 1 + random_int(18);
   l = 1 + random_int(18);
   unsigned total = j + k + l ;
   printf("init: %u %u %u: %u\n", j, k, l, total);
   //  final decision: just run with the preceding...
   
#define  SINGLE_ATTR_CLIP  1   

#ifdef SINGLE_ATTR_CLIP
   //  this clips to single-attribute max value,
   //  but total is set to 18, which is not desired...
   uint single_attr = attr_temp ;
   double dstr = (double) (j * single_attr) / (double) total ;
   double ddex = (double) (k * single_attr) / (double) total ;
   double diq  = (double) (l * single_attr) / (double) total ;
#else   
   //  this gives us reasonable percent of total, 
   //  but does not clip to single-attribute value
   double dstr = (double) (j * attr_temp) / (double) total ;
   double ddex = (double) (k * attr_temp) / (double) total ;
   double diq  = (double) (l * attr_temp) / (double) total ;
#endif   
   double dtotal = dstr + ddex + diq ;
   printf("doubles: %.2f %.2f %.2f = %.2f \n", dstr, ddex, diq, dtotal);
   
   str = 1 + (uint) (dstr + 0.5) ;
   dex = 1 + (uint) (ddex + 0.5) ;
   iq  = 1 + (uint) (diq  + 0.5) ;
   uint out_total = str + dex + iq ;
   
   printf("str=%2u, dex=%2u, int=%2u, pct: %u/%u/%u, attr_temp: %u, pcttotal: %u, outtotal: %u\n", 
      str, dex, iq, j, k, l, attr_temp, total, out_total) ;
#else
   j = (1 + random(min(attr_temp, 18))) ;
   attr_temp -= j ;
   if (attr_temp == 0)  goto skipping ;
   k = (1 + random(min(attr_temp, 18))) ;
   attr_temp -= k ;
   if (attr_temp == 0)  goto skipping ;
   l = (1 + random(min(attr_temp, 18))) ;
   attr_temp -= l ;
skipping:
   // printf("init str=%u, dex=%u, int=%u, j=%u, k=%u, l=%u, temp=%u\n", 
   //    str, dex, iq, j, k, l, attr_temp) ;

   //  distribute remaining points evenly across all stats
   while (attr_temp != 0) {
      unsigned touched = 0 ;
      if (j < 18) {
         j++ ;
         touched++ ;
         if (--attr_temp == 0)
            continue;
      }
      if (k < 18) {
         k++ ;
         touched++ ;
         if (--attr_temp == 0)
            continue;
      }
      if (l < 18) {
         l++ ;
         touched++ ;
         if (--attr_temp == 0)
            continue;
      }
      if (!touched) {
         printf("no touch: str=%u, dex=%u, int=%u\n", j, k, l) ;
         break;
      }
   }
   printf("str=%2u, dex=%2u, int=%2u\n", j, k, l) ;
#endif
}

//*************************************************************
int main(void)
{
   srand2(time(NULL));
   while (1) {
      scramble(18, 18, 18) ;
      int chr = getch() ;
      if (chr != 32) {
         break ;
      }
   }
   // return 0;
}
