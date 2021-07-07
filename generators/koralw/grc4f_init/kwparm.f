      SUBROUTINE AMPdummy
! ******************************** 
! dummy routine, to outsmart some smart linker
! ********************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      CALL
     $   KWPAR2(XAMAW,XAMAZ,XGAMMW,XGAMMZ,XSINW2)
      END
