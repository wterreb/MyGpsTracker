#include <stdlib.h>
//#include "ftoa_engine.h"
#include "dtoa_conv.h"
//#include "sectionname.h"

/* ???  Is 'width' a signed value?
   dtostrf.S comment:
  If precision is < 0, the string is left adjusted with leading spaces.
  If precision is > 0, the string is right adjusted with trailing spaces.
   dtostrf.S code:
  'p_width' is a register for left/right adjustment
   avr-libc manual:
  nothing about this
   So, for compatibilty 'width' is signed value to left/right adjust.
 */


char *
dtostrf (double val, signed char width, unsigned char prec, char *sout)
{
    unsigned char flags;

    /* DTOA_UPPER: for compatibility with avr-libc <= 1.4 with NaNs */
    flags = width < 0 ? DTOA_LEFT | DTOA_UPPER : DTOA_UPPER;
    dtoa_prf (val, sout, abs(width), prec, flags);
    return sout;
}
