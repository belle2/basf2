!----------------------------------------------------------------------
! TOP counter for Belle: initialization/interface of TOP_MASK
! M. Staric, mar-2017, 
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
!----------------------------------------------------------------------

      SUBROUTINE SET_CHANNEL_MASK(MDN, ICH, FLAG)

      IMPLICIT NONE
      INTEGER MDN               ! module ID
      INTEGER ICH               ! channel (e.g. pixelID-1)
      INTEGER FLAG              ! status flag (0 = bad, otherwise good)

#include "TOP_GEO.fi"
#include "TOP_PIK.fi"
#include "TOP_MASK.fi"
      integer nsiz
      PARAMETER (NSIZ=NUM_CHA*NSIZ_MOD)
      DATA CH_MASK /NSIZ*.TRUE./
      DATA CH_EFFI /NSIZ*1.0/
      DATA MIN_EFFI /0.1/
      
      IF(MDN.LT.0.OR.MDN.GE.NSIZ_MOD) RETURN
      IF(ICH.LT.0.OR.ICH.GE.NUM_CHA) RETURN
      CH_MASK(ICH,MDN)=FLAG.NE.0
      
      END


      SUBROUTINE SET_CHANNEL_EFFI(MDN, ICH, EFFI)

      IMPLICIT NONE
      INTEGER MDN               ! module ID
      INTEGER ICH               ! channel (e.g. pixelID-1)
      REAL EFFI                 ! relative efficiency

#include "TOP_GEO.fi"
#include "TOP_PIK.fi"
#include "TOP_MASK.fi"
      
      IF(MDN.LT.0.OR.MDN.GE.NSIZ_MOD) RETURN
      IF(ICH.LT.0.OR.ICH.GE.NUM_CHA) RETURN
      CH_EFFI(ICH,MDN)=MAX(EFFI,MIN_EFFI)
      
      END


      
