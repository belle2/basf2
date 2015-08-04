/*IT TRAPS FLOATING POINT EXCEPTIONS*/
/*written by Fulvio*/
#include <fpu_control.h>
#ifdef _FPU_SETCW
void __attribute__ ((constructor)) trapfpe() 
          {
	    int cw;
	    cw=_FPU_DEFAULT
	        & 
	      ~(  _FPU_MASK_IM  | _FPU_MASK_ZM  | _FPU_MASK_OM );
            _FPU_SETCW (cw);
          }
#else
void __attribute__ ((constructor)) trapfpe() 
          {
            (void) __setfpucw (_FPU_DEFAULT &
                               ~(_FPU_MASK_IM | _FPU_MASK_ZM | _FPU_MASK_OM));
          }
#endif

