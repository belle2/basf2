      SUBROUTINE set_param_ZZ(prob_ang,prob_mass,rmas,rgam,
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

!         BR(1)  <== u
!         BR(2)  <== d
!         BR(3)  <== s
!         BR(4)  <== c
!         BR(5)  <== b
!         BR(6)  <== e
!         BR(7)  <== mu
!         BR(8)  <== tau
!         BR(9)  <== ve
!         BR(10)  <== vmu
!         BR(11)  <== vtau

!      IF(init.EQ.0) THEN
!        init=1
! generalities
      ambeam=amel
      amreg2_mas=    ambeam**2/10
      amreg2_ang=    ambeam**3
      rmas(1)=amaw
      rmas(2)=amaz
      rgam(1)=gammw
      rgam(2)=gammz

      itpe_loc=itype
      IF( (abs(i_flav(1)).NE.11 .AND. abs(i_flav(2)).NE.11
     @   .AND. abs(i_flav(3)).EQ.11 .AND. abs(i_flav(4)).EQ.11)
     @      ) THEN 
        itpe_loc=flip_flop(itype)
      ELSE
        itpe_loc=itype
      ENDIF

      DO itp=0,m_step
        prob_ang(itp,6)=0d0  !log**2t/t
        prob_ang(itp,7)=0d0  !log**2u/u
        prob_ang(itp,8)=0d0  !1/t**2
        prob_ang(itp,9)=0d0  !1/u**2
      ENDDO
      prob_mass(5)=0d0

!      ENDIF

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
        IF(itpe_loc.eq.8)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=4.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=4.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=8/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=0d0/10d0  !ln t/t
            prob_ang(itp,5)=0d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=5/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=0/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=8/16d0  !Z
        ELSEIF(itpe_loc.eq.11)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=6.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=5.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=5/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=0/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=8/16d0  !Z
        ELSEIF(itpe_loc.eq.10)THEN
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=2/16d0  !1/s
          prob_mass(3)=2/16d0  !ln s/s
          prob_mass(4)=5/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=5/16d0  !Z
          IF( i_flav(1).EQ.1.AND.i_flav(3).EQ.4 .OR. !  ccdd
     @        i_flav(1).EQ.4.AND.i_flav(3).EQ.1 .OR. 
     @        i_flav(1).EQ.2.AND.i_flav(3).EQ.3 .OR. !  uuss
     @        i_flav(1).EQ.3.AND.i_flav(3).EQ.2 .OR. 
     @        i_flav(1).EQ.2.AND.i_flav(3).EQ.5 .OR. !  uubb
     @        i_flav(1).EQ.5.AND.i_flav(3).EQ.2 .OR. 
     @        i_flav(1).EQ.4.AND.i_flav(3).EQ.5 .OR. !  ccbb
     @        i_flav(1).EQ.5.AND.i_flav(3).EQ.4 ) THEN
c 2/20/98 fix for uuss... WW resonance
            prob_mass(1)=2/16d0  !flat
            prob_mass(2)=2/16d0  !1/s
            prob_mass(3)=1/16d0  !ln s/s
            prob_mass(4)=1/16d0  !1/sma-s
            prob_mass(6)=8/16d0  !W
            prob_mass(7)=2/16d0  !Z
          ENDIF
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
        ELSEIF(itpe_loc.eq.13)THEN
          prob_mass(1)=2d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=3/16d0  !ln s/s
          prob_mass(4)=0/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=8/16d0  !Z
          IF( i_flav(1).EQ.1.AND.i_flav(3).EQ.4 .OR. !  ccdd
     @        i_flav(1).EQ.4.AND.i_flav(3).EQ.1 .OR. 
     @        i_flav(1).EQ.2.AND.i_flav(3).EQ.3 .OR. !  uuss
     @        i_flav(1).EQ.3.AND.i_flav(3).EQ.2 .OR. 
     @        i_flav(1).EQ.2.AND.i_flav(3).EQ.5 .OR. !  uubb
     @        i_flav(1).EQ.5.AND.i_flav(3).EQ.2 .OR. 
     @        i_flav(1).EQ.4.AND.i_flav(3).EQ.5 .OR. !  ccbb
     @        i_flav(1).EQ.5.AND.i_flav(3).EQ.4 ) THEN
c 2/20/98 fix for uuss... WW resonance
            prob_mass(1)=2d0/16d0  !flat
            prob_mass(2)=2d0/16d0  !1/s
            prob_mass(3)=2/16d0  !ln s/s
            prob_mass(4)=0/16d0  !1/sma-s
            prob_mass(6)=8/16d0  !W
            prob_mass(7)=2/16d0  !Z
          ENDIF
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
        ELSEIF(itpe_loc.eq.38)THEN
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
        ELSEIF(itpe_loc.eq.43 .or. itpe_loc.eq.56)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=4/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=4/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=4.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=1d0/10d0  !ln u/u
            prob_ang(itp,8)=1d0/10d0  !1/t**2
            prob_ang(itp,9)=1d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2d0/16d0  !flat
          prob_mass(2)=5d0/16d0  !1/s
          prob_mass(3)=4d0/16d0  !ln s/s
          prob_mass(4)=1d0/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=4d0/16d0  !Z
        ELSEIF(itpe_loc.eq.40. or. itpe_loc.eq.59)THEN
      amreg2_mas=    ambeam**2/10
      amreg2_ang=    ambeam**3*50
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=6d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=6/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=8/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=3.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.22 .or. itpe_loc.eq.29)THEN
      amreg2_mas=    ambeam**2/10
      amreg2_ang=    ambeam**3*50
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=6/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=4d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=6/10d0     !flat
            prob_ang(itp,2)=1.5d0/10d0     !1/t
            prob_ang(itp,3)=1.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=8/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=3.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.52 .or. itpe_loc.eq.47)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=7.5d0/10d0     !1/u
            prob_ang(itp,4)=0d0/10d0  !ln t/t
            prob_ang(itp,5)=0d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=1.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=5.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=5/16d0  !1/s
          prob_mass(3)=4/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=4d0/16d0  !Z
        ELSEIF(itpe_loc.eq.34 .or. itpe_loc.eq.17)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=5/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=3d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1d0/10d0  !ln u/u
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=6.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=6/10d0     !flat
            prob_ang(itp,2)=1.5d0/10d0     !1/t
            prob_ang(itp,3)=1.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=8/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=4d0/16d0  !Z
        ELSEIF(itpe_loc.eq.55 .or. itpe_loc.eq.44)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=6/10d0     !flat
            prob_ang(itp,2)=1.5d0/10d0     !1/t
            prob_ang(itp,3)=1.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=1.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=5.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=5/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ENDIF
          ENDDO
          prob_mass(1)=2d0/16d0  !flat
          prob_mass(2)=4d0/16d0  !1/s
          prob_mass(3)=4d0/16d0  !ln s/s
          prob_mass(4)=1d0/16d0  !1/sma-s
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=4.5d0/16d0  !Z
        ELSE
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=5/16d0  !1/s
          prob_mass(3)=4/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=4/16d0  !Z
        ENDIF
!     =======================================================
      ELSEIF( (abs(i_flav(1)).EQ.11 .AND. abs(i_flav(2)).EQ.11 
     @   .AND. (abs(i_flav(3)).EQ.14 .OR. abs(i_flav(3)).EQ.16))
     @    .OR.
     @   ((abs(i_flav(1)).EQ.14 .OR. abs(i_flav(1)).EQ.16)
     @   .AND. abs(i_flav(3)).EQ.11 .AND. abs(i_flav(4)).EQ.11)
     @      ) THEN 
!     ===============
!  6-10, type channels
!     =======================================================
        DO itp=0,m_step
          prob_ang(itp,1)=3/10d0     !flat
          prob_ang(itp,2)=2d0/10d0     !1/t
          prob_ang(itp,3)=2d0/10d0     !1/u
          prob_ang(itp,4)=1.5d0/10d0  !ln t/t
          prob_ang(itp,5)=1.5d0/10d0  !ln u/u
        ENDDO
        IF(itpe_loc.eq.9)THEN
          prob_mass(1)=4/16d0  !flat
          prob_mass(2)=4/16d0  !1/s
          prob_mass(3)=3/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=4/16d0  !Z
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=3.5d0/10d0
            prob_ang(itp,3)=0.5d0/10d0
            prob_ang(itp,4)=3.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=3.5d0/10d0
            prob_ang(itp,3)=0.5d0/10d0
            prob_ang(itp,4)=3.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF        
          ENDDO
        ELSEIF(itpe_loc.eq.50)THEN
! unused
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=2.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=4d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=4d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=4/16d0  !1/s
          prob_mass(3)=2/16d0  !ln s/s
          prob_mass(4)=4/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=4d0/16d0  !Z
        ELSEIF(itpe_loc.eq.21)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=5.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=5d0/10d0     !1/t
            prob_ang(itp,3)=0.5d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=3.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=8/16d0  !1/s
          prob_mass(3)=2/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=8d0/16d0  !Z
        ELSEIF(itpe_loc.eq.42.or.itpe_loc.eq.50)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=6.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=2d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=4d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=4d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=1.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=7/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=2/16d0  !1/sma-s
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=3.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.60.or.itpe_loc.eq.33)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=4.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=5.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=8/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=3.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.57)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=4.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=5.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=4.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=5/16d0  !1/s
          prob_mass(3)=5/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=2.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.17)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=1.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=2d0/10d0  !1/t**2
            prob_ang(itp,9)=2d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=1.5d0/10d0
            prob_ang(itp,3)=0.5d0/10d0
            prob_ang(itp,4)=1.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            prob_ang(itp,8)=2d0/10d0  !1/t**2
            prob_ang(itp,9)=2d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=1.5d0/10d0
            prob_ang(itp,3)=0.5d0/10d0
            prob_ang(itp,4)=1.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            prob_ang(itp,8)=2d0/10d0  !1/t**2
            prob_ang(itp,9)=2d0/10d0  !1/u**2
            ENDIF        
          ENDDO
          prob_mass(1)=7d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=1d0/16d0  !ln s/s
          prob_mass(4)=2d0/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=3d0/16d0  !Z
        ELSEIF(itpe_loc.eq.18)THEN
          prob_mass(1)=2d0/16d0  !flat
          prob_mass(2)=3d0/16d0  !1/s
          prob_mass(3)=3/16d0  !ln s/s
          prob_mass(4)=.5d0/16d0  !1/sma-s
          prob_mass(6)=5d0/16d0  !W
          prob_mass(7)=2.5/16d0  !Z
        ELSEIF(itpe_loc.eq.43)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=2.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=4.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=4.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
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
          prob_mass(6)=5d0/16d0  !W
          prob_mass(7)=4.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.55 .or. itpe_loc.eq.31)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=6.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=3.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=3.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            ENDIF
          ENDDO
          prob_mass(1)=6d0/16d0  !flat
          prob_mass(2)=.5d0/16d0  !1/s
          prob_mass(3)=0d0/16d0  !ln s/s
          prob_mass(4)=1d0/16d0  !1/sma-s
          prob_mass(6)=6d0/16d0  !W
          prob_mass(7)=2.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.34)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3d0/10d0  !ln u/u
            prob_ang(itp,6)=0d0/10d0  !ln2 t/t
            prob_ang(itp,7)=2d0/10d0  !ln2 u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=0.5d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            prob_ang(itp,6)=0d0/10d0  !ln2 t/t
            prob_ang(itp,7)=0d0/10d0  !ln2 u/u
            prob_ang(itp,8)=4.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=5/10d0     !flat
            prob_ang(itp,2)=1.5d0/10d0     !1/t
            prob_ang(itp,3)=1.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,6)=0d0/10d0  !ln2 t/t
            prob_ang(itp,7)=0d0/10d0  !ln2 u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=9/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=3d0/16d0  !Z
        ELSEIF(itpe_loc.eq.36)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,6)=0d0/10d0  !ln2 t/t
            prob_ang(itp,7)=0d0/10d0  !ln2 u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            prob_ang(itp,6)=5d0/10d0  !ln2 t/t
            prob_ang(itp,7)=0d0/10d0  !ln2 u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=4.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,6)=0d0/10d0  !ln2 t/t
            prob_ang(itp,7)=0d0/10d0  !ln2 u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=9/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=3d0/16d0  !Z
        ELSEIF(itpe_loc.eq.25)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=2.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=4.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=4.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=10/16d0  !1/s
          prob_mass(3)=.5/16d0  !ln s/s
          prob_mass(4)=.5/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=3d0/16d0  !Z
        ELSEIF(itpe_loc.eq.37)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            prob_ang(itp,8)=2.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=2/16d0  !1/s
          prob_mass(3)=8/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=2.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.52)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=7.5d0/10d0     !1/u
            prob_ang(itp,4)=0d0/10d0  !ln t/t
            prob_ang(itp,5)=0d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=2.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=3.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=5/16d0  !flat
          prob_mass(2)=4/16d0  !1/s
          prob_mass(3)=4/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=0d0/16d0  !W
          prob_mass(7)=2d0/16d0  !Z
        ELSEIF(itpe_loc.eq.22)THEN
      amreg2_mas=    ambeam**2/10
      amreg2_ang=    ambeam**3*50
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=4d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=4d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=2d0/10d0  !ln u/u
            prob_ang(itp,8)=0.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=9/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=2.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.40)THEN
      amreg2_mas=    ambeam**2/10
      amreg2_ang=    ambeam**3*50
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=6d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=3d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=2.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=9/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=2.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.45)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=3d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=3d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=6.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=9/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=2.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.10)THEN
      amreg2_mas=    ambeam**2/10
      amreg2_ang=    ambeam**3*50
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=3.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=9/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=2.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.35
     @         .or.itpe_loc.eq.20)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=4.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=3.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=9/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=2.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.8)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=4.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=4.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=8/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=0d0/10d0  !ln t/t
            prob_ang(itp,5)=0d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=5/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=0/16d0  !1/sma-s
          prob_mass(6)=0/16d0  !W
          prob_mass(7)=8/16d0  !Z
        ELSEIF(itpe_loc.eq.11)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=6.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=5.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=5/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=2/16d0  !1/sma-s
          prob_mass(6)=1/16d0  !W
          prob_mass(7)=5/16d0  !Z
        ELSE
          DO itp=0,m_step
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=5/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=2/16d0  !1/sma-s
          prob_mass(6)=1/16d0  !W
          prob_mass(7)=5/16d0  !Z
        ENDIF
!!!!!!!!!!!!!!!!! FIXED !!!!!!!!!!!!!!!!!!!!!!
        prob_mass(1)=2/16d0  !flat
        prob_mass(2)=5/16d0  !1/s
        prob_mass(3)=0/16d0  !ln s/s
        prob_mass(4)=0/16d0  !1/sma-s
        prob_mass(6)=0d0/16d0  !W
        prob_mass(7)=9d0/16d0  !Z
!!!!!!!!!!!!!!!!! FIXED !!!!!!!!!!!!!!!!!!!!!!
!     =======================================================
      ELSEIF( (abs(i_flav(1)).EQ.11 .AND. abs(i_flav(2)).EQ.11 
     @   .AND. abs(i_flav(3)).NE.11 .AND. abs(i_flav(4)).NE.11)
!     @      ) THEN 
!      ELSEIF( 
     @    .OR.
     @   (abs(i_flav(1)).NE.11 .AND. abs(i_flav(2)).NE.11
     @   .AND. abs(i_flav(3)).EQ.11 .AND. abs(i_flav(4)).EQ.11)
!     @      ) THEN 
!      ELSEIF(
     @    .OR.
     @   (abs(i_flav(1)).EQ.11 .AND. abs(i_flav(2)).EQ.11 .AND. 
     @   abs(i_flav(3)).EQ.11 .AND. abs(i_flav(4)).EQ.11 ) 
     @      ) THEN 
!     ===============
!  6-1, type channels
!     =======================================================
!  6-6, type channels
!     =======================================================
        amreg2_ang=    ambeam**3/50
        amreg2_mas=    ambeam**2/10
        prob_mass(1)=2/16d0  !flat
        prob_mass(2)=8/16d0  !1/s
        prob_mass(3)=1/16d0  !ln s/s
        prob_mass(4)=1/16d0  !1/sma-s
        prob_mass(5)=0/16d0  !1/s**2
        prob_mass(6)=.5d0/16d0  !W
        prob_mass(7)=3.5d0/16d0  !Z
        DO itp=0,m_step
          prob_ang(itp,1)=3/10d0     !flat
          prob_ang(itp,2)=2d0/10d0     !1/t
          prob_ang(itp,3)=2d0/10d0     !1/u
          prob_ang(itp,4)=1.5d0/10d0  !ln t/t
          prob_ang(itp,5)=1.5d0/10d0  !ln u/u
        ENDDO
        IF(itpe_loc.eq.9)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=3.5d0/10d0
            prob_ang(itp,3)=0.5d0/10d0
            prob_ang(itp,4)=3.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=3.5d0/10d0
            prob_ang(itp,3)=0.5d0/10d0
            prob_ang(itp,4)=3.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF        
          ENDDO
        ELSEIF(itpe_loc.eq.11)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=3.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=1d0/10d0  !1/t**2
            prob_ang(itp,9)=1d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=4/16d0  !1/s
          prob_mass(3)=.5/16d0  !ln s/s
          prob_mass(4)=.5/16d0  !1/sma-s
          prob_mass(5)=5/16d0  !1/s**2
          prob_mass(6)=.5/16d0  !W
          prob_mass(7)=3.5/16d0  !Z
        ELSEIF(itpe_loc.eq.8)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=5/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=5/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=2.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=2/16d0  !flat
          prob_mass(2)=4/16d0  !1/s
          prob_mass(3)=.5/16d0  !ln s/s
          prob_mass(4)=.5/16d0  !1/sma-s
          prob_mass(5)=5/16d0  !1/s**2
          prob_mass(6)=.5/16d0  !W
          prob_mass(7)=3.5/16d0  !Z
        ELSEIF(itpe_loc.eq.49.or.itpe_loc.eq.46.or.itpe_loc.eq.23)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=5/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=4/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.50)THEN
! unused
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=2.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=4d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=4d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.21)THEN
c      amreg2_ang=    ambeam**3/10
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=5d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=5d0/10d0     !1/t
            prob_ang(itp,3)=0.5d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=4/10d0     !flat
            prob_ang(itp,2)=2.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
          prob_mass(1)=4/16d0  !flat
          prob_mass(2)=6/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(5)=0/16d0  !1/s**2
          prob_mass(6)=.5d0/16d0  !W
          prob_mass(7)=3.5d0/16d0  !Z
        ELSEIF(itpe_loc.eq.42.or.itpe_loc.eq.50)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=6.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=2d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=4d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=4d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=1.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.60.or.itpe_loc.eq.33)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=4.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=5.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.57)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=4.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=5.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=4.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.17)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=1.5d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=2d0/10d0  !1/t**2
            prob_ang(itp,9)=2d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1)THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=1.5d0/10d0
            prob_ang(itp,3)=0.5d0/10d0
            prob_ang(itp,4)=1.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            prob_ang(itp,8)=2d0/10d0  !1/t**2
            prob_ang(itp,9)=2d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2d0/10d0
            prob_ang(itp,2)=1.5d0/10d0
            prob_ang(itp,3)=0.5d0/10d0
            prob_ang(itp,4)=1.5d0/10d0
            prob_ang(itp,5)=0.5d0/10d0
            prob_ang(itp,8)=2d0/10d0  !1/t**2
            prob_ang(itp,9)=2d0/10d0  !1/u**2
            ENDIF        
          ENDDO
        ELSEIF(itpe_loc.eq.18)THEN
        ELSEIF(itpe_loc.eq.43 .or. itpe_loc.eq.55)THEN
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
        ELSEIF(itpe_loc.eq.34)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=0.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3d0/10d0  !ln u/u
            prob_ang(itp,6)=0d0/10d0  !ln2 t/t
            prob_ang(itp,7)=2d0/10d0  !ln2 u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=0.5d0/10d0     !1/t
            prob_ang(itp,3)=0.5d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            prob_ang(itp,6)=0d0/10d0  !ln2 t/t
            prob_ang(itp,7)=0d0/10d0  !ln2 u/u
            prob_ang(itp,8)=4.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2.5d0/10d0     !1/t
            prob_ang(itp,3)=2.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,6)=1d0/10d0  !ln2 t/t
            prob_ang(itp,7)=.5d0/10d0  !ln2 u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.36)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,6)=0d0/10d0  !ln2 t/t
            prob_ang(itp,7)=0d0/10d0  !ln2 u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            prob_ang(itp,6)=5d0/10d0  !ln2 t/t
            prob_ang(itp,7)=0d0/10d0  !ln2 u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=4.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,6)=0d0/10d0  !ln2 t/t
            prob_ang(itp,7)=0d0/10d0  !ln2 u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.25)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=2.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=4.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            prob_ang(itp,8)=5d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=4/10d0     !flat
            prob_ang(itp,2)=2.5d0/10d0     !1/t
            prob_ang(itp,3)=2.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.37)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            prob_ang(itp,8)=2.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.52)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=7.5d0/10d0     !1/u
            prob_ang(itp,4)=0d0/10d0  !ln t/t
            prob_ang(itp,5)=0d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=2.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=3.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=3.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=3.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.22)THEN
c      amreg2_ang=    ambeam**3*50
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=4d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=4d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=2d0/10d0  !ln u/u
            prob_ang(itp,8)=0.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.40)THEN
c      amreg2_ang=    ambeam**3*50
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=6d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=3d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=3.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1d0/10d0     !1/t
            prob_ang(itp,3)=1d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.45)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=3d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=3d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=6.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=0.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=7/10d0     !flat
            prob_ang(itp,2)=1.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.10)THEN
c      amreg2_ang=    ambeam**3*50
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=1d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=3.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=.5d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=.5d0/10d0  !ln t/t
            prob_ang(itp,5)=5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSEIF(itpe_loc.eq.35
     @         .or.itpe_loc.eq.20)THEN
          DO itp=0,m_step
            IF(itp.EQ.0) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=4.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ELSEIF(itp.EQ.1) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2.5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=3.5d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=0.5d0/10d0  !1/u**2
            ELSEIF(itp.EQ.2) THEN
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=5d0/10d0     !1/t
            prob_ang(itp,3)=.5d0/10d0     !1/u
            prob_ang(itp,4)=2d0/10d0  !ln t/t
            prob_ang(itp,5)=.5d0/10d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
            ENDIF
          ENDDO
        ELSE
          DO itp=0,m_step
            prob_ang(itp,1)=2/10d0     !flat
            prob_ang(itp,2)=2d0/10d0     !1/t
            prob_ang(itp,3)=2d0/10d0     !1/u
            prob_ang(itp,4)=1.5d0/10d0  !ln t/t
            prob_ang(itp,5)=1.5d0/10d0  !ln u/u
            prob_ang(itp,8)=.5d0/10d0  !1/t**2
            prob_ang(itp,9)=.5d0/10d0  !1/u**2
          ENDDO
        ENDIF
      ELSE
!     ====
        WRITE(6,*)'set_param_ZZ=> not set for i_flav:',i_flav
      ENDIF
!     =====
!!!!!!!!!!!!!!!!!!!! short circuit !!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!! short circuit !!!!!!!!!!!!!!!!!!!!!!!!
c          DO itp=0,m_step
c            prob_ang(itp,1)=2/10d0     !flat
c            prob_ang(itp,2)=1d0/10d0     !1/t
c            prob_ang(itp,3)=1d0/10d0     !1/u
c            prob_ang(itp,4)=2.5d0/10d0  !ln t/t
c            prob_ang(itp,5)=2.5d0/10d0  !ln u/u
c            prob_ang(itp,8)=.5d0/10d0  !1/t**2
c            prob_ang(itp,9)=.5d0/10d0  !1/u**2
c          ENDDO
c          prob_mass(1)=2/16d0  !flat
c          prob_mass(2)=2/16d0  !1/s
c          prob_mass(3)=5/16d0  !ln s/s
c          prob_mass(4)=3/16d0  !1/sma-s
c          prob_mass(6)=2/16d0  !W
c          prob_mass(7)=2/16d0  !Z
!!!!!!!!!!!!!!!!!!!! short circuit !!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!! short circuit !!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!! flat distr. for Mariusz begin !!!!!!!!!!!!!!!!!!!!
        IF(itpe_loc.eq.2)THEN
          DO itp=0,m_step
            prob_ang(itp,1)=8/10d0     !flat
            prob_ang(itp,2)=1/10d0     !1/t
            prob_ang(itp,3)=1/10d0     !1/u
            prob_ang(itp,4)=0d0  !ln t/t
            prob_ang(itp,5)=0d0  !ln u/u
            prob_ang(itp,6)=0d0  !ln t/t
            prob_ang(itp,7)=0d0  !ln u/u
            prob_ang(itp,8)=0d0/10d0  !1/t**2
            prob_ang(itp,9)=0d0/10d0  !1/u**2
          ENDDO
          prob_mass(1)=8/16d0  !flat
          prob_mass(2)=2/16d0  !1/s
          prob_mass(3)=1/16d0  !ln s/s
          prob_mass(4)=1/16d0  !1/sma-s
          prob_mass(5)=0/16d0  !1/s**2
          prob_mass(6)=2d0/16d0  !W
          prob_mass(7)=2d0/16d0  !Z
        ENDIF
!!!!!!!!!!!!! flat distr. for Mariusz end !!!!!!!!!!!!!!!!!!!!
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
     @  'set_param_ZZ=> wrong ang. prob.: itpe_loc,step,sum='
     @    ,itpe_loc,j,pang
            STOP
          ENDIF
        ENDDO
        pang=0d0
        DO i=1,n_mass
          pang=pang+prob_mass(i)
        ENDDO
        IF(abs(pang-1).gt. 1d-14) THEN
          WRITE(6,*)
     @      'set_param_ZZ=> wrong mass prob.: itpe_loc,sum='
     @        ,itpe_loc,pang
          STOP
        ENDIF
      ENDIF

      END
