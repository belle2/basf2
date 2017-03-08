!-----------------------------------------------------------------------
! TOP counter for Belle: interface to TOP_DATA
! M. Staric, mar-2009, sept-2011, mar-2017
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
!-----------------------------------------------------------------------

      SUBROUTINE DATA_CLEAR

      IMPLICIT NONE
#include "TOP_DATA.fi"
      DATA N_DATA /0/

      N_RESTORE=N_DATA
      N_DATA=0
      END


      SUBROUTINE DATA_RESTORE

      IMPLICIT NONE
#include "TOP_DATA.fi"

      N_DATA=N_RESTORE
      END


      SUBROUTINE DATA_PUT(MDN,ICH,TDC,TIME,KK)

      IMPLICIT NONE
      INTEGER MDN               ! module ID number
      INTEGER ICH               ! channel ID number
      INTEGER TDC               ! TDC time (digitized)
      REAL TIME                 ! time in [ns] converted from TDC and subtracted t0
      INTEGER KK                ! return status: index or 0 (failure)
#include "TOP_DATA.fi"
#include "TOP_TDC.fi"
#include "TOP_GEO.fi"
#include "TOP_PIK.fi"
#include "TOP_MASK.fi"

      INTEGER K

      KK=0
      K=N_DATA+1
      IF(K.GT.NSIZ_DATA) THEN
         PRINT*,'TOP_DATA: no space for new digitized data'
         RETURN
      ENDIF
      IF(MDN.LT.0.OR.MDN.GE.NSIZ_MOD) RETURN
      IF(ICH.LT.0.OR.ICH.GE.NUM_CHA) RETURN
      IF(.NOT.CH_MASK(ICH,MDN)) RETURN
c      IF(TDC.LT.0.OR.TDC.GE.TDC_RANGE) RETURN
      DATA_MOD(K)=MDN
      DATA_ICH(K)=ICH
      DATA_TDC(K)=TDC
      DATA_TIME(K)= TIME        !(TDC+0.5)*TDC_DT-TDC_T0+T0
      N_DATA=K
      KK=K

      END


      INTEGER FUNCTION DATA_GETNUM()

      IMPLICIT NONE
#include "TOP_DATA.fi"
      
      DATA_GETNUM=N_DATA
      END


      SUBROUTINE DATA_GET(K,MDN,ICH,TDC)

      IMPLICIT NONE
      INTEGER K                 ! index (row#) in TOP_DATA [in]
      INTEGER MDN               ! module ID number
      INTEGER ICH               ! channel ID number
      INTEGER TDC               ! TDC time (digitized)
#include "TOP_DATA.fi"

      IF(K.GT.0.AND.K.LE.N_DATA) THEN
         MDN=DATA_MOD(K)
         ICH=DATA_ICH(K)
         TDC=DATA_TDC(K)
      ELSE
         MDN=-1
         ICH=-1
         TDC=-1
      ENDIF
      END



      
