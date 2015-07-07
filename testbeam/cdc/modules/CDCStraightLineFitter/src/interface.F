!-----------------------------------------------------------------------
! interface to straight line in 3D
! M. Staric, mar-2015
!
!-----------------------------------------------------------------------


      SUBROUTINE TEMA_CLEAR

      IMPLICIT NONE
#include "TEMA_PAR.fi"
      DATA Z_CENTER /64.0/ ! cm
#include "TEMA_HITS.fi"
#include "TEMA_RSEG.fi"

      INTEGER I

!     TEMA_HITS
      DO I=1,NHIT_SIZ
         NHIT(I)=0
      ENDDO
      NHIT_ITR=0
      NHIT_OTR=0

!     TEMA_RSEG
      NSEG=0
      DO I=0,MAXSEG
         IP_SEG(I)=0
      ENDDO

      END


      SUBROUTINE TEMA_SET_RSEG(R,DIR)

      IMPLICIT NONE
      REAL R(3)                 ! track seed: point
      REAL DIR(3)               ! track seed: momentum

#include "TEMA_RSEG.fi"
#include "TEMA_HITS.fi"

      IF(DIR(3).EQ.0) THEN
         PRINT*,'tema_set_rseg_: PZ.EQ.0, no fit possible'
         RETURN
      ENDIF

      NSEG=1
      NHIT(1)=0
      FIT_SEG(NSEG)=.TRUE.      ! fit this segment
      SEG_R(1,NSEG)=R(1)
      SEG_R(2,NSEG)=R(2)
      SEG_R(3,NSEG)=R(3)
      SEG_S(1,NSEG)=DIR(1)/DIR(3)
      SEG_S(2,NSEG)=DIR(2)/DIR(3)
      SEG_S(3,NSEG)=1.

      END


      SUBROUTINE TEMA_ADD_HITS(R_FW, R_BK, DRIFT, SIG)

      IMPLICIT NONE
      REAL R_FW(3)              ! wire forward position
      REAL R_BK(3)              ! wire backward position
      REAL DRIFT                ! drift length
      REAL SIG                  ! sigma of drift length
      INTEGER LAYER             ! layer code (see TEMA_LAY.fi)

#include "TEMA_HITS.fi"
#include "TEMA_RSEG.fi"

      INTEGER K, I
      REAL ALFA

      K=NHIT(1)+1
      IF(K.GT.MAXHIT) THEN
         PRINT*,'tema_add_hits_: no space for new hit'
         RETURN
      ENDIF

      ZHIT(K)=R_BK(3)-(R_FW(3)-R_BK(3))/(R_FW(2)-R_BK(2))*R_BK(2)
      XHIT(K)=R_BK(1)-(R_FW(1)-R_BK(1))/(R_FW(2)-R_BK(2))*R_BK(2)
      QHIT(K)=DRIFT
      SHIT(1,K)=0.5             ! todo: according to cell size
      SHIT(2,K)=SIG
      IF(R_BK(2).LT.R_FW(2)) THEN
         YHIT(1,K)=R_BK(2)
         YHIT(2,K)=R_FW(2)
      ELSE
         YHIT(1,K)=R_FW(2)
         YHIT(2,K)=R_BK(2)
      ENDIF
      IF(R_FW(2).EQ.R_BK(2)) THEN
         PRINT*,'tema_add_hits_: wire length is zero (bug?)'
         RETURN
      ENDIF
      ALFA=ATAN((R_FW(1)-R_BK(1))/(R_FW(2)-R_BK(2)))
      C_ALFA(K)=COS(ALFA)
      S_ALFA(K)=SIN(ALFA)

      DO I=1,3
         WIRE_R0(I,K)=R_BK(I)
         WIRE_S(I,K)=R_FW(I)-R_BK(I)
      ENDDO
      CALL TEMA_UNIT_VECT(WIRE_S(1,K),WIRE_LEN(K))

      INDIK(K)=K
      IP_SEG(NSEG)=K
      NHIT(1)=K

      END


      LOGICAL FUNCTION TEMA_GET_STATUS()

      IMPLICIT NONE
#include "TEMA_RSEG.fi"
      INTEGER ISEG

      ISEG=1
      IF(ISEG.GT.0.AND.ISEG.LE.NSEG) THEN
         TEMA_GET_STATUS=FIT_SEG(ISEG)
      ELSE
         TEMA_GET_STATUS=.FALSE.
      ENDIF
      
      END


      REAL FUNCTION TEMA_GET_CHI2()

      IMPLICIT NONE
#include "TEMA_RSEG.fi"
      INTEGER ISEG

      ISEG=1
      IF(ISEG.GT.0.AND.ISEG.LE.NSEG) THEN
         TEMA_GET_CHI2=SEG_HI2(ISEG)
      ELSE
         TEMA_GET_CHI2=0
      ENDIF
      
      END


      INTEGER FUNCTION TEMA_GET_NDF()

      IMPLICIT NONE
#include "TEMA_RSEG.fi"
      INTEGER ISEG

      ISEG=1
      IF(ISEG.GT.0.AND.ISEG.LE.NSEG) THEN
         TEMA_GET_NDF=NDF_SEG(ISEG)
      ELSE
         TEMA_GET_NDF=0
      ENDIF
      
      END


      SUBROUTINE TEMA_GET_DIR(DIR)

      IMPLICIT NONE
      REAL DIR(3)
#include "TEMA_RSEG.fi"
      REAL S
      INTEGER ISEG

      ISEG=1
      IF(ISEG.GT.0.AND.ISEG.LE.NSEG) THEN
         S=SQRT(SEG_S(1,ISEG)**2+SEG_S(2,ISEG)**2+SEG_S(3,ISEG)**2)
         DIR(1)=SEG_S(1,ISEG)/S
         DIR(2)=SEG_S(2,ISEG)/S
         DIR(3)=SEG_S(3,ISEG)/S
      ENDIF
      
      END


      SUBROUTINE TEMA_GET_RF(R)

      IMPLICIT NONE
      REAL R(3)
#include "TEMA_RSEG.fi"
      INTEGER ISEG

      ISEG=1
      IF(ISEG.GT.0.AND.ISEG.LE.NSEG) THEN
         R(1)=SEG_RF(1,ISEG)
         R(2)=SEG_RF(2,ISEG)
         R(3)=SEG_RF(3,ISEG)
      ENDIF
      
      END


      SUBROUTINE TEMA_GET_RE(R)

      IMPLICIT NONE
      REAL R(3)
#include "TEMA_RSEG.fi"
      INTEGER ISEG

      ISEG=1
      IF(ISEG.GT.0.AND.ISEG.LE.NSEG) THEN
         R(1)=SEG_RE(1,ISEG)
         R(2)=SEG_RE(2,ISEG)
         R(3)=SEG_RE(3,ISEG)
      ENDIF
      
      END




