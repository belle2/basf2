!-----------------------------------------------------------------------
! TOP counter for Belle: initialization of/interface to TOP_TDC
! M. Staric, mar-2009
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
!-----------------------------------------------------------------------

      SUBROUTINE SET_TDC(NBIT,CHWID,OFFSET)

      IMPLICIT NONE
      INTEGER NBIT              ! number of bits for TDC word
      REAL CHWID                ! channel width [ns]
      REAL OFFSET               ! offset [ns]
#include "TOP_TDC.fi"
      DATA TDC_INI /.FALSE./
      DATA CFD_DT /0.5/         ! MC: CFD pile-up time [ns]
      DATA CFD_DELT /5./        ! MC: CFD double pulse resolution [ns]

      TDC_RANGE=2**NBIT
      TDC_DT=CHWID
      TDC_T0=OFFSET
      TIME_RANGE=TDC_RANGE*TDC_DT
      TDC_INI=.TRUE.

      END


