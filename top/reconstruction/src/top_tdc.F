!-----------------------------------------------------------------------
! TOP counter for Belle: initialization of/interface to TOP_TDC
! M. Staric, mar-2009, sept-2013
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
! mar-2017: change in some commons
!-----------------------------------------------------------------------

      SUBROUTINE SET_TDC(NBIT,CHWID,OFFSET)

      IMPLICIT NONE
      INTEGER NBIT              ! number of bits for TDC word
      REAL CHWID                ! channel width [ns]
      REAL OFFSET               ! offset [ns]
#include "TOP_TDC.fi"
#include "TOP_PIK.fi"
      DATA TDC_INI /.FALSE./
      DATA CFD_DT /0.5/         ! MC: CFD pile-up time [ns]
      DATA CFD_DELT /5./        ! MC: CFD double pulse resolution [ns]

      TDC_RANGE=2**NBIT
      TDC_DT=CHWID
      TDC_T0=OFFSET
      TIME_RANGE=TDC_RANGE*TDC_DT
      T_MIN=-TDC_T0
      T_MAX=TIME_RANGE-TDC_T0
      TDC_INI=.TRUE.

      END


      SUBROUTINE SET_TIME_WINDOW(TMIN, TMAX)
      
      IMPLICIT NONE
      REAL TMIN                 ! time window lower edge
      REAL TMAX                 ! time window upper edge
#include "TOP_TDC.fi"
#include "TOP_PIK.fi"
      
      IF(TMAX.GT.TMIN) THEN
         T_MIN=TMIN
         T_MAX=TMAX
      ELSE
         T_MIN=-TDC_T0
         T_MAX=TIME_RANGE-TDC_T0
      ENDIF

      END


      SUBROUTINE SET_CFD(DT, DELT)
      
      IMPLICIT NONE
      REAL DT                   ! CFD pile-up time [ns]
      REAL DELT                 ! CFD double pulse resolution [ns]
#include "TOP_TDC.fi"

      CFD_DT=DT
      CFD_DELT=DELT

      END



