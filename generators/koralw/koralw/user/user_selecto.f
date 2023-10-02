      SUBROUTINE user_selecto(p1,p2,p3,p4,p5,p6,wt)
* #################################################
* #        mask on phase space regions            #
* #         to be modified by the user            #
* #################################################
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION p1(4),p2(4),p3(4),p4(4),p5(4),p6(4)
      COMMON / decays / IFlav(4), amdec(4) 
      SAVE / decays /

      wt=1d0

      END

      SUBROUTINE user_selecto_canonical(pmu,pnu,pu,pd,p5,p6,wt)
* #########################################################
* #        mask on phase space regions                    #
* # Canonical Cuts as in LEP2 Workshop, YR 9601 v1 p. 243 #
* #  unclear for energy limits for massive fermions       #
* #########################################################
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION pmu(4),pnu(4),pu(4),pd(4),p5(4),p6(4)
      DIMENSION pbeam(4),pbeam2(4)
      COMMON / decays / IFlav(4), amdec(4) 
      SAVE / decays /
      DATA PBEAM /0D0,0D0,-1D0,1D0/
      DATA PBEAM2 /0D0,0D0,1D0,1D0/

C lepton visibility
      EMINLE=1d0    ! GeV
      AMINLE=10d0   ! degrees
C jet visibility
      EMINJE=3d0    ! GeV
      AMINJE=0d0    ! degrees
C photon visibility
      EMINGA=0.1d0  ! GeV
      AMINGA=1d0   ! degrees
C jet lepton separation 
      XLEJE= 5d0    ! degrees
C two jet mas 
      XTWOJE= 5d0   !GeV
C photon gluing angle
      AGLUE = 5d0   ! degrees     

      imu=iflav(1)
      inu=iflav(2)
      iu=iflav(3)
      id=iflav(4)

      wt=0d0
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!! m.s. cuts for all 4-f, YR vol 1, page 243.
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
       XMASA2=xtwoje**2
ccc       write(6,*) xtwoje,eminle,eminje,xleje,aminle
       IF(abs(imu).lt.10 .and. abs(iu).lt.10) THEN !4q 
         IF(PMU(4).LT.EMINJE) goto 34
         IF(PNU(4).LT.EMINJE) goto 34
         IF(PU(4).LT.EMINJE) goto 34
         IF(PD(4).LT.EMINJE) goto 34

         IF(selecto_xmas2(PMU,PNU).LT.xmasa2) goto 34
         IF(selecto_xmas2(PMU,PU).LT.xmasa2) goto 34
         IF(selecto_xmas2(PMU,PD).LT.xmasa2) goto 34
         IF(selecto_xmas2(PNU,PU).LT.xmasa2) goto 34
         IF(selecto_xmas2(PNU,PD).LT.xmasa2) goto 34
         IF(selecto_xmas2(PU,PD).LT.xmasa2) goto 34
       ELSEIF(abs(imu).ge.10 .and. abs(iu).ge.10) THEN !4l

c         write(6,*) '4l entered'
c         write(6,*) EMINLE,AMINLE,xleje
c         write(6,*) pmu,pnu,pu,pd

! flag neutrinos 
         ju=1
         jd=1
         jmu=1
         jnu=1

         if(abs(imu).eq.12 .or.abs(imu).eq.14 .or.abs(imu).eq.16) jmu=0 
         if(abs(inu).eq.12 .or.abs(inu).eq.14 .or.abs(inu).eq.16) jnu=0 
         if(abs(iu).eq.12 .or.abs(iu).eq.14 .or.abs(iu).eq.16) ju=0 
         if(abs(id).eq.12 .or.abs(id).eq.14 .or.abs(id).eq.16) jd=0 

         IF(PMU(4).LT.EMINLE .and. jmu.ne.0) goto 34
         IF(PNU(4).LT.EMINLE .and. jnu.ne.0) goto 34
         IF(PU(4).LT.EMINLE .and. ju.ne.0) goto 34
         IF(PD(4).LT.EMINLE .and. jd.ne.0) goto 34

         acol=90d0
         if(jmu.ne.0) ACOL=selecto_xacoln(PMU,PBEAM,3)
         IF(ACOL.LT.AMINLE.OR.ACOL.GT.(180D0-AMINLE)) goto 34

         acol=90d0
         if(jnu.ne.0) ACOL=selecto_xacoln(PNU,PBEAM,3)
         IF(ACOL.LT.AMINLE.OR.ACOL.GT.(180D0-AMINLE)) goto 34

         acol=90d0
         if(ju.ne.0) ACOL=selecto_xacoln(PU,PBEAM,3)
         IF(ACOL.LT.AMINLE.OR.ACOL.GT.(180D0-AMINLE))goto 34

         acol=90d0
         if(jd.ne.0) ACOL=selecto_xacoln(PD,PBEAM,3)
         IF(ACOL.LT.AMINLE.OR.ACOL.GT.(180D0-AMINLE))goto 34

         acol=90d0
         if(jmu.ne.0 .and. jnu.ne.0) ACOL=selecto_xacoln(PMU,PNU,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(jmu.ne.0 .and. ju.ne.0) ACOL=selecto_xacoln(PMU,PU,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(jmu.ne.0 .and. jd.ne.0) ACOL=selecto_xacoln(PMU,PD,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(jnu.ne.0 .and. ju.ne.0) ACOL=selecto_xacoln(PNU,PU,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(jnu.ne.0 .and. jd.ne.0) ACOL=selecto_xacoln(PNU,PD,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(jd.ne.0 .and. ju.ne.0) ACOL=selecto_xacoln(PU,PD,3)
         IF(ACOL.LT.XLEJE) goto 34

       ELSEIF(abs(imu).ge.10 .and. abs(iu).lt.10) THEN !2l2q 

cc         write(6,*) '2l2q entered'
cc         write(6,*) pmu,pnu,pu,pd
! flag neutrinos
         jmu=1
         jnu=1

         if(abs(imu).eq.12 .or.abs(imu).eq.14 .or.abs(imu).eq.16) jmu=0 
         if(abs(inu).eq.12 .or.abs(inu).eq.14 .or.abs(inu).eq.16) jnu=0 

         IF(PMU(4).LT.EMINLE .and. jmu.ne.0) goto 34
         IF(PNU(4).LT.EMINLE .and. jnu.ne.0) goto 34
         IF(PU(4).LT.EMINJE) goto 34
         IF(PD(4).LT.EMINJE) goto 34

cc         write(6,*) '2l2q energy'

         acol=90d0
         if(jmu.ne.0) ACOL=selecto_xacoln(PMU,PBEAM,3)
         IF(ACOL.LT.AMINLE.OR.ACOL.GT.(180D0-AMINLE)) goto 34

         acol=90d0
         if(jnu.ne.0) ACOL=selecto_xacoln(PNU,PBEAM,3)
         IF(ACOL.LT.AMINLE.OR.ACOL.GT.(180D0-AMINLE)) goto 34
!!!!!!! for checks only >>>>>>>
!!!!!!! for checks only <<<<<<<

cc         write(6,*) '2l2q beams'

         acol=90d0
         if(jmu.ne.0 .and. jnu.ne.0) ACOL=selecto_xacoln(PMU,PNU,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(jmu.ne.0) ACOL=selecto_xacoln(PMU,PU,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(jmu.ne.0) ACOL=selecto_xacoln(PMU,PD,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(jnu.ne.0) ACOL=selecto_xacoln(PNU,PU,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(jnu.ne.0) ACOL=selecto_xacoln(PNU,PD,3)
         IF(ACOL.LT.XLEJE) goto 34
!!!!!!! for checks only >>>>>>>
c         ACOL=selecto_xacoln(PU,PD,3)
c         IF(ACOL.LT.XLEJE) goto 34
!!!!!!! for checks only <<<<<<<

cc         write(6,*) '2l2q acol'
         IF(selecto_xmas2(PU,PD).LT.xmasa2) goto 34
cc         write(6,*) '2l2q mass'
       ELSEIF(abs(imu).lt.10 .and. abs(iu).ge.10) THEN !2q2l 

! flag neutrinos 
         ju=1
         jd=1

         if(abs(iu).eq.12 .or.abs(iu).eq.14 .or.abs(iu).eq.16) ju=0 
         if(abs(id).eq.12 .or.abs(id).eq.14 .or.abs(id).eq.16) jd=0 

         IF(PMU(4).LT.EMINJE) goto 34
         IF(PNU(4).LT.EMINJE) goto 34
         IF(PU(4).LT.EMINLE .and. ju.ne.0) goto 34
         IF(PD(4).LT.EMINLE .and. jd.ne.0) goto 34


         acol=90d0
         if(ju.ne.0) ACOL=selecto_xacoln(PU,PBEAM,3)
         IF(ACOL.LT.AMINLE.OR.ACOL.GT.(180D0-AMINLE)) goto 34

         acol=90d0
         if(jd.ne.0) ACOL=selecto_xacoln(PD,PBEAM,3)
         IF(ACOL.LT.AMINLE.OR.ACOL.GT.(180D0-AMINLE)) goto 34


         acol=90d0
         if(ju.ne.0) ACOL=selecto_xacoln(PMU,PU,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(jd.ne.0) ACOL=selecto_xacoln(PMU,PD,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(ju.ne.0) ACOL=selecto_xacoln(PNU,PU,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(jd.ne.0) ACOL=selecto_xacoln(PNU,PD,3)
         IF(ACOL.LT.XLEJE) goto 34

         acol=90d0
         if(jd.ne.0 .and. ju.ne.0) ACOL=selecto_xacoln(PU,PD,3)
         IF(ACOL.LT.XLEJE) goto 34

         IF(selecto_xmas2(PMU,PNU).LT.xmasa2) goto 34
       ELSE
         WRITE(6,*)'shop95=>wrong id-s'
         STOP
       ENDIF

       wt=1d0 ! all cuts passed, event accepted
 34    continue

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!! m.s. end
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

      END


      FUNCTION selecto_XMAS2(PU,PD)
C     ********************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION PU(4),PD(4)
      selecto_XMAS2=(PU(4)+PD(4))**2-(PU(3)+PD(3))**2
     $    -(PU(2)+PD(2))**2-(PU(1)+PD(1))**2
      END

      FUNCTION selecto_XACOLN(X,Y,N)
C     ********************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DOUBLE PRECISION    X(*),Y(*)
      DIMENSION X1(4),Y1(4)
      DATA PI /3.1415926535897932D0/
      S=0.D0
      X2=0.D0
      Y2=0.D0
      DO 9  I=1,N
      X1(I)=X(I)
    9 Y1(I)=Y(I)
      DO 10 I=1,N
      S=S+X1(I)*Y1(I)
      X2=X2+X1(I)**2
   10 Y2=Y2+Y1(I)**2
      selecto_XACOLN=ACOS(S/SQRT(X2*Y2))*180.D0/PI
      RETURN
      END
