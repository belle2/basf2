      SUBROUTINE TOHEP
      DOUBLE PRECISION  QEFF1(4),QEFF2(4),SPHUM(4),SPHOT(100,4)
      DOUBLE PRECISION Q1(4),Q2(4),P1(4),P2(4),P3(4),P4(4)
      INTEGER NPHOT
      COMMON / MOMSET / QEFF1,QEFF2,SPHUM,SPHOT,NPHOT
      COMMON / MOMDEC / Q1,Q2,P1,P2,P3,P4
      DOUBLE PRECISION ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2
      INTEGER IDE,IDF
      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF
      COMMON /TAUPOS/ NP1,NP2                
      COMMON /PHOACT/ IFPHOT
      COMMON /TRALID/ idtra
      INTEGER IFLAV(4)
      REAL*4    POL0(4),POL1(4),POL2(4)
      DOUBLE PRECISION  AMDEC(4)
      COMMON / DECAYS / IFLAV, AMDEC
C
      REAL*4 APH(4),XPB1(4),XPB2(4),AQF1(4),AQF2(4),XAMD(4),AM
      REAL*4 XP1(4),XP2(4),XP3(4),XP4(4)

! to switch tau polarization OFF in taus from W- (W+) decays set 
!     DATA  POL1 /0.0,0.0,0.0,0.0/
!     DATA  POL2 /0.0,0.0,0.0,0.0/
! note that taus from Z decays are unpolarized, 10/7/98

      DATA  POL0 /0.0,0.0, 0.0,0.0/
      DATA  POL1 /0.0,0.0,-1.0,0.0/
      DATA  POL2 /0.0,0.0,-1.0,0.0/


      SAVE
C
      DO K=1,4
       AQF1(K)=Q1(K)
       AQF2(K)=Q2(K)
       XP1(K) =P1(K) 
       XP2(K) =P2(K) 
       XP3(K) =P3(K) 
       XP4(K) =P4(K) 
C
       XAMD(K)=AMDEC(K)
      ENDDO

C initial state (1,2)
      AM=AMEL
! e- (0 0 +1 1)
      XPB1(1) = 0d0
      XPB1(2) = 0d0
      XPB1(3) = dsqrt(ene**2 -am**2) !e- (00+11)
      XPB1(4) = ene
! e+ (0 0 -1 1)
      XPB2(1) = 0d0
      XPB2(2) = 0d0
      XPB2(3) =-xpb1(3) !e+ (00-11)
      XPB2(4) = ene

      CALL FILHEP(1,3, 11,0,0,0,0,XPB1,AM,.TRUE.)
      CALL FILHEP(2,3,-11,0,0,0,0,XPB2,AM,.TRUE.)
C primary final state W-W+ (3,4)
      IF (IFLAV(1).eq.-iflav(2)) then
       idwm= 23
       idwp= 23
      else
       idwm=-24
       idwp= 24       
      endif
      AM=sqrt(Q1(4)**2-Q1(3)**2-Q1(2)**2-Q1(1)**2)
      CALL FILHEP(3,2,idwm,1,2,0,0,AQF1,AM,.TRUE.)
      AM=sqrt(Q2(4)**2-Q2(3)**2-Q2(2)**2-Q2(1)**2)
      CALL FILHEP(4,2,idwp,1,2,0,0,AQF2,AM,.TRUE.)
C radiative photons (5 ... 4+NPHOT) (PDG-code for gamma is 22)
      IF(NPHOT.NE.0) THEN
        IP=0
        DO I=1,NPHOT
          DO J=1,4
            APH(J)=SPHOT(I,J)
          END DO
          IF (APH(4).GT.0.0) THEN
            IP=IP+1
            CALL FILHEP(4+IP,1,22,1,2,0,0,APH,0.0,.TRUE.)
          END IF
       END DO
      END IF
C decay products W- (5,6)+NPHOT
      CALL FILHEP(0,1,IFLAV(1),3,3,0,0,XP1,XAMD(1),.TRUE.)
      CALL FILHEP(0,1,IFLAV(2),3,3,0,0,XP2,XAMD(2),.TRUE.)
C decay products W+ (7,8)+NPHOT
      CALL FILHEP(0,1,IFLAV(3),4,4,0,0,XP3,XAMD(3),.TRUE.)
      CALL FILHEP(0,1,IFLAV(4),4,4,0,0,XP4,XAMD(4),.TRUE.)
C

C tau decays:
      if (abs(iflav(1)).eq.15.and.abs(iflav(2)).eq.16) then
          KTO=2
          idtra=1
!          write(*,*) 'id=',idtra
          NP2=5+NPHOT
          CALL DEXAY(KTO,POL2)
 !         write(*,*) '==='
      endif
      if (abs(iflav(4)).eq.15.and.abs(iflav(3)).eq.16) then
          KTO=1
          idtra=4
          NP1=8+NPHOT
          CALL DEXAY(KTO,POL1)
      endif
      if (abs(iflav(1)).eq.15.and.abs(iflav(2)).eq.15) then
          KTO=2
          idtra=1
          NP2=5+NPHOT
          CALL DEXAY(KTO,POL0)
          KTO=1
          idtra=2
          NP1=6+NPHOT
          CALL DEXAY(KTO,POL0)
      endif
      if (abs(iflav(4)).eq.15.and.abs(iflav(3)).eq.15) then
          KTO=2
          idtra=3
          NP2=7+NPHOT
          CALL DEXAY(KTO,POL0)
          KTO=1
          idtra=4
          NP1=8+NPHOT
          CALL DEXAY(KTO,POL0)
      endif
C radiate photons for  leptonic W decays.
        IF (IFPHOT.EQ.1) THEN
          if (abs(iflav(1)).gt.10) CALL PHOTOS(3)
          if (abs(iflav(3)).gt.10) CALL PHOTOS(4)
        ENDIF
      END 
