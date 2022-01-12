      SUBROUTINE set_param_WW(prob_ang,prob_mass,rmas,rgam,
     $                    amreg2_mas,amreg2_ang,itype,i_flav)
!     *************************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      SAVE
      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
      DIMENSION prob_ang(0:m_step,n_ang)
      DIMENSION rmas(n_m_res),rgam(n_m_res),prob_mass(n_mass)
      DIMENSION i_flav(4)
      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF 
      COMMON / WEKIN2 / AMAW,GAMMW,GMU,ALPHAW 
!      DATA init /0/

!         BR(1)  <== ud
!         BR(2)  <== cd
!         BR(3)  <== us
!         BR(4)  <== cs
!         BR(5)  <== ub
!         BR(6)  <== cb
!         BR(7)  <== e
!         BR(8)  <== mu
!         BR(9)  <== tau

!      IF(init.EQ.0) THEN
!        init=1
! generalities
      ambeam=amel
      amreg2_mas=    ambeam**2/10
      amreg2_ang=    ambeam**3/10
      rmas(1)=amaw
      rmas(2)=amaz
      rgam(1)=gammw
      rgam(2)=gammz

      DO itp=0,m_step
        prob_ang(itp,6)=0d0  !(log t)**2/t
        prob_ang(itp,7)=0d0  !(log u)**2/u
        prob_ang(itp,8)=0d0  !1/t**2
        prob_ang(itp,9)=0d0  !1/u**2
      ENDDO
      prob_mass(5)=0d0

!      ENDIF

!     =======================================================
      IF(abs(i_flav(1)).NE.11 .AND. abs(i_flav(2)).NE.11 .AND. 
     @   abs(i_flav(3)).NE.11 .AND. abs(i_flav(4)).NE.11 ) THEN 
!     ==============
!  1-1 type channels
!     =======================================================
        DO itp=0,m_step
          prob_ang(itp,1)=3/10d0     !flat
          prob_ang(itp,2)=3/10d0     !1/t
          prob_ang(itp,3)=3/10d0     !1/u
          prob_ang(itp,4)=.5d0/10d0  !ln t/t
          prob_ang(itp,5)=.5d0/10d0  !ln u/u
        ENDDO
        IF(itype.eq.10)THEN
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=2/16d0  !1/s
          prob_mass(3)=2/16d0  !ln s/s
          prob_mass(4)=7/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=3/16d0  !Z
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=3/10d0     !flat
            prob_ang(itp,2)=3d0/10d0     !1/t
            prob_ang(itp,3)=3d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=6d0/10d0
            prob_ang(itp,2)=1.5d0/10d0
            prob_ang(itp,3)=1.5d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=.5d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=6d0/10d0
            prob_ang(itp,2)=1.5d0/10d0
            prob_ang(itp,3)=1.5d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=.5d0/10d0
            ENDIF
          ENDDO        
        ELSEIF(itype.eq.13)THEN
          prob_mass(1)=2d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=3/16d0  !ln s/s
          prob_mass(4)=0/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=8/16d0  !Z
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=3/10d0     !flat
            prob_ang(itp,2)=3d0/10d0     !1/t
            prob_ang(itp,3)=3d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=6d0/10d0
            prob_ang(itp,2)=1.5d0/10d0
            prob_ang(itp,3)=1.5d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=.5d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=6d0/10d0
            prob_ang(itp,2)=1.5d0/10d0
            prob_ang(itp,3)=1.5d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=.5d0/10d0
            ENDIF
          ENDDO        
        ELSEIF(itype.eq.38. or. itype.eq.45)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=2d0/10d0
            prob_ang(itp,3)=2d0/10d0
            prob_ang(itp,4)=2d0/10d0
            prob_ang(itp,5)=2d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=2d0/10d0
            prob_ang(itp,3)=2d0/10d0
            prob_ang(itp,4)=2d0/10d0
            prob_ang(itp,5)=2d0/10d0
            ENDIF
          ENDDO        
          prob_mass(1)=4d0/16d0  !flat
          prob_mass(2)=6d0/16d0  !1/s
          prob_mass(3)=2d0/16d0  !ln s/s
          prob_mass(4)=0d0/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=4d0/16d0  !Z
        ELSE
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=3/16d0  !1/s
          prob_mass(3)=2/16d0  !ln s/s
          prob_mass(4)=3/16d0  !1/sma-s
          prob_mass(6)=3/16d0  !W
          prob_mass(7)=3/16d0  !Z
        ENDIF
!     =======================================================
      ELSEIF( (abs(i_flav(1)).EQ.11 .AND. abs(i_flav(2)).NE.11 
     @   .AND. abs(i_flav(3)).NE.11 .AND. abs(i_flav(4)).NE.11)
     @      ) THEN 
!     ===============
!  7-1, type channels
!     =======================================================
        DO itp=0,m_step
          prob_ang(itp,1)=3/10d0     !flat
          prob_ang(itp,2)=2d0/10d0     !1/t
          prob_ang(itp,3)=2d0/10d0     !1/u
          prob_ang(itp,4)=1.5d0/10d0  !ln t/t
          prob_ang(itp,5)=1.5d0/10d0  !ln u/u
        ENDDO
        IF(itype.eq.9)THEN
          prob_mass(1)=4/16d0  !flat
          prob_mass(2)=4/16d0  !1/s
          prob_mass(3)=4/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=3/16d0  !Z
        ELSEIF(itype.eq.17)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=3.5d0/10d0
            prob_ang(itp,3)=0.5d0/10d0
            prob_ang(itp,4)=3.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=3.5d0/10d0
            prob_ang(itp,3)=0.5d0/10d0
            prob_ang(itp,4)=3.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            ENDIF        
          ENDDO
          prob_mass(1)=7d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=1d0/16d0  !ln s/s
          prob_mass(4)=2d0/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=3d0/16d0  !Z
        ELSEIF(itype.eq.18)THEN
          prob_mass(1)=2d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=2/16d0  !ln s/s
          prob_mass(4)=.5d0/16d0  !1/sma-s
          prob_mass(6)=6d0/16d0  !W
          prob_mass(7)=2.5/16d0  !Z
        ELSEIF(itype.eq.43 .or. itype.eq.55)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=4/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=1d0/10d0  !ln u/u
            ENDIF
          ENDDO
          prob_mass(1)=5d0/16d0  !flat
          prob_mass(2)=.5d0/16d0  !1/s
          prob_mass(3)=0d0/16d0  !ln s/s
          prob_mass(4)=1d0/16d0  !1/sma-s
          prob_mass(6)=6d0/16d0  !W
          prob_mass(7)=3.5d0/16d0  !Z
        ELSE
          DO itp=0,m_step
            prob_ang(itp,1)=3/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
          ENDDO
          prob_mass(1)=3/16d0  !flat
          prob_mass(2)=2/16d0  !1/s
          prob_mass(3)=2/16d0  !ln s/s
          prob_mass(4)=4/16d0  !1/sma-s
          prob_mass(6)=3/16d0  !W
          prob_mass(7)=2/16d0  !Z
        ENDIF
      ELSEIF( (abs(i_flav(1)).NE.11 .AND. abs(i_flav(2)).NE.11
     @   .AND. abs(i_flav(3)).NE.11 .AND. abs(i_flav(4)).EQ.11)
     @      ) THEN 
!     =======================================================
!  1-7 type channels
        DO itp=0,m_step
          prob_ang(itp,1)=3/10d0     !flat
          prob_ang(itp,2)=2d0/10d0     !1/t
          prob_ang(itp,3)=2d0/10d0     !1/u
          prob_ang(itp,4)=1.5d0/10d0  !ln t/t
          prob_ang(itp,5)=1.5d0/10d0  !ln u/u
        ENDDO
        IF(itype.eq.12)THEN
          prob_mass(1)=4/16d0  !flat
          prob_mass(2)=4/16d0  !1/s
          prob_mass(3)=4/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=3/16d0  !Z
        ELSEIF(itype.eq.34)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t  
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=3.5d0/10d0
            prob_ang(itp,3)=0.5d0/10d0
            prob_ang(itp,4)=3.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=0.5d0/10d0
            prob_ang(itp,3)=3.5d0/10d0
            prob_ang(itp,4)=0.5d0/10d0
            prob_ang(itp,5)=3.5d0/10d0
            ENDIF        
          ENDDO
          prob_mass(1)=5d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=1d0/16d0  !ln s/s
          prob_mass(4)=2d0/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=5d0/16d0  !Z
        ELSEIF(itype.eq.33)THEN
          prob_mass(1)=2d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=2/16d0  !ln s/s
          prob_mass(4)=.5d0/16d0  !1/sma-s
          prob_mass(6)=6d0/16d0  !W
          prob_mass(7)=2.5/16d0  !Z
        ELSEIF(itype.eq.56 .or. itype.eq.44)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=4/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=1d0/10d0  !ln u/u
            ENDIF
          ENDDO
          prob_mass(1)=6d0/16d0  !flat
          prob_mass(2)=.5d0/16d0  !1/s
          prob_mass(3)=0d0/16d0  !ln s/s
          prob_mass(4)=1d0/16d0  !1/sma-s
          prob_mass(6)=6d0/16d0  !W
          prob_mass(7)=2.5d0/16d0  !Z
        ELSE
          DO itp=0,m_step
            prob_ang(itp,1)=3/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=3/16d0  !1/s
          prob_mass(3)=2/16d0  !ln s/s
          prob_mass(4)=4/16d0  !1/sma-s
          prob_mass(6)=3/16d0  !W
          prob_mass(7)=2/16d0  !Z
        ENDIF
      ELSEIF(abs(i_flav(1)).EQ.11 .AND. abs(i_flav(2)).NE.11 .AND. 
     @   abs(i_flav(3)).NE.11 .AND. abs(i_flav(4)).EQ.11 ) THEN 
!     =======================================================
!  7-7 type channels
!  22.05.97  re-optimized 10,13,34,38,42,54,61
        DO itp=0,m_step
          prob_ang(itp,1)=3/10d0     !flat
          prob_ang(itp,2)=2d0/10d0     !1/t
          prob_ang(itp,3)=2d0/10d0     !1/u
          prob_ang(itp,4)=1.5d0/10d0  !ln t/t
          prob_ang(itp,5)=1.5d0/10d0  !ln u/u
        ENDDO
        IF(itype.eq.13)THEN
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=.5d0/16d0  !1/s
          prob_mass(3)=3.5d0/16d0  !ln s/s
          prob_mass(4)=8/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=2/16d0  !Z
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=3d0/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=8d0/10d0
            prob_ang(itp,2)=.5d0/10d0
            prob_ang(itp,3)=.5d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=.5d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=8d0/10d0
            prob_ang(itp,2)=.5d0/10d0
            prob_ang(itp,3)=.5d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=.5d0/10d0
            ENDIF
          ENDDO        
        ELSEIF(itype.eq.54)THEN
          prob_mass(1)=1.5d0/16d0  !flat
          prob_mass(2)=4d0/16d0  !1/s
          prob_mass(3)=5d0/16d0  !ln s/s
          prob_mass(4)=3d0/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=2.5/16d0  !Z
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2d0/10d0     !flat
            prob_ang(itp,2)=6.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=.5d0/10d0
            prob_ang(itp,3)=1.5d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=5.5d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=6d0/10d0
            prob_ang(itp,2)=1.5d0/10d0
            prob_ang(itp,3)=1.5d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=.5d0/10d0
            ENDIF
          ENDDO        
        ELSEIF(itype.eq.10)THEN
          prob_mass(1)=2d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=3/16d0  !ln s/s
          prob_mass(4)=0/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=8/16d0  !Z
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2d0/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=6.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=6.5d0/10d0
            prob_ang(itp,3)=.5d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=.5d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7d0/10d0
            prob_ang(itp,2)=1d0/10d0
            prob_ang(itp,3)=1d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=.5d0/10d0
            ENDIF
          ENDDO        
        ELSEIF(itype.eq.57 .or. itype.eq.21 
     $        .or. itype.eq.45)THEN
          DO itp=0,m_step
            prob_ang(itp,1)=3/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
          ENDDO
          prob_mass(1)=5/16d0  !flat
          prob_mass(2)=1/16d0  !1/s
          prob_mass(3)=6/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=3/16d0  !Z
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2d0/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=6.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=6.5d0/10d0
            prob_ang(itp,3)=.5d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=.5d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=1d0/10d0
            prob_ang(itp,3)=1d0/10d0
            prob_ang(itp,4)=0d0/10d0
            prob_ang(itp,5)=6d0/10d0
            ENDIF
          ENDDO        
        ELSEIF(itype.eq.33)THEN
          DO itp=0,m_step
            prob_ang(itp,1)=3/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
          ENDDO
          prob_mass(1)=5/16d0  !flat
          prob_mass(2)=1/16d0  !1/s
          prob_mass(3)=6/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=3/16d0  !Z
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=6.5d0/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=2d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=6.5d0/10d0
            prob_ang(itp,3)=.5d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=.5d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=6d0/10d0
            prob_ang(itp,2)=1d0/10d0
            prob_ang(itp,3)=1d0/10d0
            prob_ang(itp,4)=0d0/10d0
            prob_ang(itp,5)=2d0/10d0
            ENDIF
          ENDDO        
!!!        ELSEIF(itype.eq.38. or. itype.eq.45)THEN
        ELSEIF(itype.eq.38)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=4.5d0/10d0
            prob_ang(itp,3)=.5d0/10d0
            prob_ang(itp,4)=2.5d0/10d0
            prob_ang(itp,5)=.5d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=6d0/10d0
            prob_ang(itp,2)=2d0/10d0
            prob_ang(itp,3)=2d0/10d0
            prob_ang(itp,4)=0d0/10d0
            prob_ang(itp,5)=0d0/10d0
            ENDIF
          ENDDO        
          prob_mass(1)=4d0/16d0  !flat
          prob_mass(2)=2d0/16d0  !1/s
          prob_mass(3)=0d0/16d0  !ln s/s
          prob_mass(4)=6d0/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=4d0/16d0  !Z
        ELSEIF(itype.eq.9)THEN
          prob_mass(1)=4/16d0  !flat
          prob_mass(2)=4/16d0  !1/s
          prob_mass(3)=4/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=3/16d0  !Z
        ELSEIF(itype.eq.17)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=3.5d0/10d0
            prob_ang(itp,3)=3.5d0/10d0
            prob_ang(itp,4)=.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=3.5d0/10d0
            prob_ang(itp,3)=0.5d0/10d0
            prob_ang(itp,4)=3.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            ENDIF        
          ENDDO
          prob_mass(1)=7d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=1d0/16d0  !ln s/s
          prob_mass(4)=2d0/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=3d0/16d0  !Z
        ELSEIF(itype.eq.18)THEN
          prob_mass(1)=2d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=2/16d0  !ln s/s
          prob_mass(4)=.5d0/16d0  !1/sma-s
          prob_mass(6)=6d0/16d0  !W
          prob_mass(7)=2.5/16d0  !Z
        ELSEIF(itype.eq.43 .or. itype.eq.55)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=4/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=1d0/10d0  !ln u/u
            ENDIF
          ENDDO
          prob_mass(1)=6d0/16d0  !flat
          prob_mass(2)=.5d0/16d0  !1/s
          prob_mass(3)=0d0/16d0  !ln s/s
          prob_mass(4)=1d0/16d0  !1/sma-s
          prob_mass(6)=6d0/16d0  !W
          prob_mass(7)=2.5d0/16d0  !Z
        ELSEIF(itype.eq.12)THEN
          prob_mass(1)=4/16d0  !flat
          prob_mass(2)=4/16d0  !1/s
          prob_mass(3)=4/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=3/16d0  !Z
        ELSEIF(itype.eq.34)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=6/10d0     !flat
            prob_ang(itp,2)=1.5d0/10d0     !1/t  
            prob_ang(itp,3)=1.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=3.5d0/10d0
            prob_ang(itp,3)=3.5d0/10d0
            prob_ang(itp,4)=0.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=.5d0/10d0
            prob_ang(itp,3)=1.5d0/10d0
            prob_ang(itp,4)=0.5d0/10d0
            prob_ang(itp,5)=5.5d0/10d0
            ENDIF        
          ENDDO
          prob_mass(1)=7d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=1d0/16d0  !ln s/s
          prob_mass(4)=2d0/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=3d0/16d0  !Z
        ELSEIF(itype.eq.33)THEN
          prob_mass(1)=2d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=2/16d0  !ln s/s
          prob_mass(4)=.5d0/16d0  !1/sma-s
          prob_mass(6)=6d0/16d0  !W
          prob_mass(7)=2.5/16d0  !Z
        ELSEIF(itype.eq.56 .or. itype.eq.44)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=4/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=1d0/10d0  !ln u/u
            ENDIF
          ENDDO
          prob_mass(1)=6d0/16d0  !flat
          prob_mass(2)=.5d0/16d0  !1/s
          prob_mass(3)=0d0/16d0  !ln s/s
          prob_mass(4)=1d0/16d0  !1/sma-s
          prob_mass(6)=6d0/16d0  !W
          prob_mass(7)=2.5d0/16d0  !Z
        ELSEIF(itype.eq.61)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=3.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=3.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat !!!!!!!
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ENDIF
          ENDDO
          prob_mass(1)=4/16d0  !flat
          prob_mass(2)=1/16d0  !1/s
          prob_mass(3)=0/16d0  !ln s/s
          prob_mass(4)=5/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=6/16d0  !Z
        ELSEIF(itype.eq.42)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=6.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=6.5d0/10d0     !1/u   !!!!!
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=4/10d0     !flat 
            prob_ang(itp,2)=4d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ENDIF
          ENDDO
          prob_mass(1)=4/16d0  !flat
          prob_mass(2)=5/16d0  !1/s
          prob_mass(3)=0/16d0  !ln s/s
          prob_mass(4)=5/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=2/16d0  !Z
        ELSE
          DO itp=0,m_step
            prob_ang(itp,1)=3/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
          ENDDO
          prob_mass(1)=3/16d0  !flat
          prob_mass(2)=3/16d0  !1/s
          prob_mass(3)=2/16d0  !ln s/s
          prob_mass(4)=3/16d0  !1/sma-s
          prob_mass(6)=3/16d0  !W
          prob_mass(7)=2/16d0  !Z
        ENDIF
      ELSE
!     ====
        WRITE(6,*)'set_param_WW=> not set for i_flav:',i_flav
        STOP
      ENDIF
!     =====
!!!!!!!!!!!!!!!!!!!! short circuit !!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!! short circuit !!!!!!!!!!!!!!!!!!!!!!!!
c          DO itp=0,m_step
c            prob_ang(itp,1)=3/10d0     !flat
c            prob_ang(itp,2)=2d0/10d0     !1/t
c            prob_ang(itp,3)=2d0/10d0     !1/u
c            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
c            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
c          ENDDO
c          prob_mass(1)=6/16d0  !flat
c          prob_mass(2)=1/16d0  !1/s
c          prob_mass(3)=6/16d0  !ln s/s
c          prob_mass(4)=1/16d0  !1/sma-s
c          prob_mass(6)=0/16d0  !W
c          prob_mass(7)=1/16d0  !Z
!!!!!!!!!!!!!!!!!!!! short circuit !!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!! short circuit !!!!!!!!!!!!!!!!!!!!!!!!
!!! checks on probabilities !!!
      icheck=1
      IF(icheck.EQ.1) THEN
        DO i=0,m_step
          pang=0d0
          DO j=1,n_ang
            pang=pang+prob_ang(i,j)
          ENDDO
          IF(abs(pang-1).gt. 1d-14) THEN
            WRITE(6,*)
     @  'set_param_WW=> wrong ang. prob.: itype,step,sum=',itype,j,pang
            STOP
          ENDIF
        ENDDO
        pang=0d0
        DO i=1,n_mass
          pang=pang+prob_mass(i)
        ENDDO
        IF(abs(pang-1).gt. 1d-14) THEN
          WRITE(6,*)
     @      'set_param_WW=> wrong mass prob.: itype,sum=',itype,pang
          STOP
        ENDIF
      ENDIF

      END
