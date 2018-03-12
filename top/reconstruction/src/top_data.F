!-----------------------------------------------------------------------
! TOP counter for Belle: interface to TOP_DATA
! M. Staric, mar-2009, sept-2011, mar-2017
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
! mar-2017: change in some commons
! feb-2018: change in some commons
!-----------------------------------------------------------------------

      SUBROUTINE DATA_CLEAR
      
      IMPLICIT NONE
#include "TOP_DATA.fi"
      DATA N_DATA /0/

      N_DATA=0
      END


      SUBROUTINE DATA_PUT(MDN,ICH,TIME,TERR,KK)

      IMPLICIT NONE
      INTEGER MDN               ! module number (moduleID - 1)
      INTEGER ICH               ! channel number (pixelID - 1)
      REAL TIME                 ! time [ns]
      REAL TERR                 ! time uncertainty [ns]
      INTEGER KK                ! return status: index or <=0 (failure)
#include "TOP_DATA.fi"
#include "TOP_TDC.fi"
#include "TOP_GEO.fi"
#include "TOP_PIK.fi"
#include "TOP_MASK.fi"

      INTEGER K

      K=N_DATA+1
      IF(K.GT.NSIZ_DATA) THEN
         KK=0
         RETURN
      ENDIF
      IF(MDN.LT.0.OR.MDN.GE.NSIZ_MOD) THEN
         KK=-1
         RETURN
      ENDIF
      IF(ICH.LT.0.OR.ICH.GE.NUM_CHA) THEN
         KK=-2
         RETURN
      ENDIF
      IF(.NOT.CH_MASK(ICH,MDN)) THEN
         KK=-3
         RETURN
      ENDIF
      DATA_MOD(K)=MDN
      DATA_ICH(K)=ICH
      DATA_TIME(K)=TIME
      DATA_TERR(K)=TERR
      N_DATA=K
      KK=K

      END


      INTEGER FUNCTION DATA_GETNUM()

      IMPLICIT NONE
#include "TOP_DATA.fi"
      
      DATA_GETNUM=N_DATA
      END


      SUBROUTINE DATA_GET(K,MDN,ICH,TIME,TERR)

      IMPLICIT NONE
      INTEGER K                 ! index (row#) in TOP_DATA [in]
      INTEGER MDN               ! module ID number
      INTEGER ICH               ! channel ID number
      REAL TIME                 ! time [ns]
      REAL TERR                 ! time uncertainty [ns]
#include "TOP_DATA.fi"

      IF(K.GT.0.AND.K.LE.N_DATA) THEN
         MDN=DATA_MOD(K)
         ICH=DATA_ICH(K)
         TIME=DATA_TIME(K)
         TERR=DATA_TERR(K)
      ELSE
         MDN=-1
         ICH=-1
         TIME=0
         TERR=0
      ENDIF
      END



      
