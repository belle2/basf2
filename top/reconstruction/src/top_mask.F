!----------------------------------------------------------------------
! TOP counter for Belle: initialization/interface of TOP_MASK
! M. Staric, mar-2017, 
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
! mar-2017: change in some commons
! feb-2018: change in some commons
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


      
      SUBROUTINE PRINT_CHANNEL_MASK

      IMPLICIT NONE

#include "TOP_GEO.fi"
#include "TOP_PIK.fi"
#include "TOP_MASK.fi"
      INTEGER MDN, ICH, ACTIVE, NROW, I, K
      REAL P
      CHARACTER*64 TXT

      PRINT*,'Channel masks (pixel row vs. column)'
      PRINT*,'===================================='
      
      DO MDN=0,NUM_MOD-1
         ACTIVE=0
         DO ICH=0,NUM_CHA-1
            IF(CH_MASK(ICH,MDN)) ACTIVE=ACTIVE+1
         ENDDO
         P=ACTIVE*100./NUM_CHA
         PRINT '(A,I3,A,F6.1,A)','Slot',MDN+1,': ',P,'% active'
         NROW=NUM_CHA/64
         DO K=NROW,1,-1
            DO I=1,64
               ICH=I-1+(K-1)*64
               IF(CH_MASK(ICH,MDN)) THEN
                  TXT(I:I)='1'
               ELSE
                  TXT(I:I)='0'
               ENDIF
            ENDDO
            PRINT '(I2,A,A)',K,'|',TXT
         ENDDO
         DO I=1,64
            TXT(I:I)='-'
         ENDDO
         PRINT '(A,A)','  +',TXT
         DO I=1,64
            WRITE(TXT(I:I),'(I1)') I/10
         ENDDO
         PRINT '(A,A)','   ',TXT
         DO I=1,64
            WRITE(TXT(I:I),'(I1)') MOD(I,10)
         ENDDO
         PRINT '(A,A)','   ',TXT
         PRINT*
      ENDDO
      
      END

      
