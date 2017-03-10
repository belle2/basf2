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
      DATA C0 /29.9798458/      ! speed of light (cm/ns)
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
      REF_IND=PHASE_INDEX(E_MEAN)
      GRU_IND=GROUP_INDEX(E_MEAN)
      REF_IND1=PHASE_INDEX(E_MEAN+D_E)
      GRU_IND1=GROUP_INDEX(E_MEAN+D_E)
      REL_DNDE=(REF_IND1-REF_IND)/D_E/REF_IND
      REL_DNGDE=(GRU_IND1-GRU_IND)/D_E/GRU_IND
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


