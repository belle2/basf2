!-----------------------------------------------------------------------
! TOP counter for Belle: initialization/interface of TOP_PAR, TOP_CONS
! M. Staric, mar-2009, 
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
! mar-2017: change in some commons
!-----------------------------------------------------------------------

      SUBROUTINE SET_TOP_PAR(BKG_PER_QBAR,SCALE_N0)
      
      IMPLICIT NONE
      REAL BKG_PER_QBAR         ! background/qbar
      REAL SCALE_N0             ! N0 scale factor
#include "TOP_CONS.fi"
      DATA C0 /29.9792458/      ! speed of light (cm/ns)
      DATA PI /3.14159274/
#include "TOP_PAR.fi"
      DATA PAR_INI /.FALSE./
      DATA D_E /0.1/            ! step for differentiation (eV)
#include "TOP_QEFF.fi"

      REAL S
      REAL PHASE_INDEX, GROUP_INDEX, QE_NZERO

      PI=4.*ATAN(1.)
      IF(.NOT.QE_INIT) THEN
         PRINT*,'*** QEFF data not loaded'
         RETURN
      ENDIF
      CALL QE_STAT(0.,10.,E_MEAN,SIG_E,S)
      SIG_E=SQRT(SIG_E)
      CALL SET_REFIND_PAR(1.0)
      NUM_BGR=MAX(BKG_PER_QBAR,0.1) ! should be >0 for numerical stability
      N_ZERO=QE_NZERO()*SCALE_N0
      PAR_INI=.TRUE.

      END


      SUBROUTINE GET_TOP_PAR(REFIND, GRUIND)

      IMPLICIT NONE
      REAL REFIND, GRUIND
#include "TOP_PAR.fi"

      REFIND=REF_IND
      GRUIND=GRU_IND

      END


      SUBROUTINE SET_REFIND_PAR(BETA)

      IMPLICIT NONE
      REAL BETA                 ! particle beta
      
#include "TOP_PAR.fi"
      
      REAL PHASE_INDEX, GROUP_INDEX, GET_MEAN_E

      E_MEAN=GET_MEAN_E(BETA)
      REF_IND=PHASE_INDEX(E_MEAN)
      GRU_IND=GROUP_INDEX(E_MEAN)
      REF_IND1=PHASE_INDEX(E_MEAN+D_E)
      GRU_IND1=GROUP_INDEX(E_MEAN+D_E)
      REL_DNDE=(REF_IND1-REF_IND)/D_E/REF_IND
      REL_DNGDE=(GRU_IND1-GRU_IND)/D_E/GRU_IND

      END


      REAL FUNCTION GET_MEAN_E(BETA)

      IMPLICIT NONE
      REAL BETA                 ! particle beta
#include "TOP_PAR.fi"
#include "TOP_QEFF.fi"
      REAL E, P, S, EM
      INTEGER I
      REAL PHASE_INDEX

      S=0
      EM=0
      DO I=1,MAX_NT
         E=QE_E(I)
         P=MAX(QE(I)*(1-(1/BETA/PHASE_INDEX(E))**2),0.)
         S=S+P
         EM=EM+P*E
      ENDDO
      IF(S.GT.0) THEN
         EM=EM/S
      ELSE
         EM=E_MEAN
      ENDIF
      GET_MEAN_E=EM
      
      END
      
